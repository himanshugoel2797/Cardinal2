/**
 * Copyright (c) 2022 Himanshu Goel
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */

#include "stdint.h"
#include "stdbool.h"
#include "stddef.h"
#include "kerndefs.h"
#include "cpuid.h"
#include "mp.h"
#include "interrupts/interrupts.h"
#include "vmem.h"

#define ICW4_8086 0x01    /* 8086/88 (MCS-80/85) mode */
#define ICW1_ICW4 0x01    /* ICW4 (not) needed */
#define ICW1_INIT 0x10 /* Initialization - required! */

#define APIC_ID (0x20)
#define APIC_TPR (0x80)
#define APIC_PPR (0xA0)
#define APIC_EOI (0xB0)
#define APIC_LDR (0xD0)
#define APIC_DFR (0xE0)
#define APIC_SVR (0xF0)
#define APIC_ISR (0x100)
#define APIC_ICR_xAPIC_LO (0x300)
#define APIC_ICR_xAPIC_HI (0x310)
#define APIC_ICR_x2APIC (0x300)

#define APIC_TIMER (0x320)
#define APIC_ICoR (0x380)
#define APIC_CCoR (0x390)
#define APIC_DCR (0x3E0)

#define MSI_ADDR (0xFEEFF00C)
#define MSI_VEC (lvl, active_low, vector) (((lvl & 1) << 15) | ((~active_low & 1) << 14) | 0x100 /*lowest priority*/ | (vector & 0xff))

typedef struct {
    uint32_t *base_addr;
    uint32_t id;
    bool x2apic_mode;
} tls_apic_t;

static TLS tls_apic_t *apic = NULL;

uint64_t apic_read(uint32_t off) {
    if(apic->x2apic_mode) {
        uint64_t val = rdmsr(0x800 + off / 16);
        return val;
    }
    return apic->base_addr[off / sizeof(uint32_t)];
}

void apic_write(uint32_t off, uint64_t val) {
    if(apic->x2apic_mode) {

        if(off == APIC_EOI) {
            wrmsr(0x800 + off / 16, val);
            return;
        }
        uint64_t tmp_Val = rdmsr(0x800 + off / 16);
        wrmsr(0x800 + off / 16, (tmp_Val & ~0xffffffff) | val);
    } else
        apic->base_addr[off / sizeof(uint32_t)] = (uint32_t)val;
}

int pic_fini() {
    //Disable the PIC
    outb(0x20, ICW1_INIT+ICW1_ICW4); // starts the initialization sequence (in cascade mode)
    outb(0xA0, ICW1_INIT+ICW1_ICW4);
    outb(0x21, 32);           // ICW2: Master PIC vector offset
    outb(0xA1, 40);           // ICW2: Slave PIC vector offset
    outb(0x21, 4);                 // ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
    outb(0xA1, 2);                 // ICW3: tell Slave PIC its cascade identity (0000 0010)

    outb(0x21, ICW4_8086);
    outb(0xA1, ICW4_8086);

    outb(0x21, 0xFF);
    outb(0xA1, 0xFF); //disable all interrupts from the PIC

    return 0;
}

int apic_init() {

    if(apic == NULL)
        apic = (TLS tls_apic_t*)mp_tls_get(mp_tls_alloc(sizeof(tls_apic_t)));


    uint64_t apic_base_reg = rdmsr(IA32_APIC_BASE);
    apic->base_addr = (uint32_t*)vmem_phystovirt(apic_base_reg & ~0xfff, KiB(8), vmem_flags_uncached);
    apic_base_reg |= (1 << 11); //Enable the apic
    {
        uint32_t a, b, c, d;
        cpuid_request(1, 0, &a, &b, &c, &d);
        bool x2apic_sup = (c >> 21) & 1;

        apic->x2apic_mode = x2apic_sup;
        if(x2apic_sup) {
            apic_base_reg |= (1 << 10);   //Enable x2apic mode

        } else {

        }
    }
    if(!apic->x2apic_mode) apic_write(APIC_DFR, 0xf0000000);    //Setup cluster destination mode
    wrmsr(IA32_APIC_BASE, apic_base_reg);

    //Read the id after enabling the apic so x2apic handling works correctly
    apic->id = apic_read(APIC_ID);
    if(!apic->x2apic_mode)apic->id = apic->id >> 24;

    apic_write(APIC_TPR, 0);
    apic_write(APIC_SVR, (1 << 8) | 0xFF);

    //TODO: Implement IPI function

    return 0;
}

int interrupt_get_cpuidx(void) {
    return apic->id;
}

void interrupt_sendeoi(int irq) {
    int byte_off = (irq / 32) * 16;
    int bit_off = irq % 32;

    if(apic_read(APIC_ISR + byte_off) & (1 << bit_off)) {
        apic_write(APIC_EOI, 0);
    }
}

void interrupt_sendipi(int cpu, int vector, ipi_delivery_mode_t delivery_mode) {
    uint64_t ipi_msg = 0;
    ipi_msg |= (vector & 0xff);
    ipi_msg |= (delivery_mode & 0x7) << 8;
    ipi_msg |= (1 << 14);

    if(apic->x2apic_mode) {
        ipi_msg |= ((uint64_t)cpu << 32);

        apic_write(APIC_ICR_x2APIC, ipi_msg);
    } else {
        ipi_msg |= ((uint64_t)cpu << 56);

        apic_write(APIC_ICR_xAPIC_HI, (uint32_t)(ipi_msg >> 32));
        apic_write(APIC_ICR_xAPIC_LO, (uint32_t)ipi_msg);
    }
}

static int intrpt_num = 0;

//configure timer for tsc deadline
int local_apic_timer_init(bool tsc_mode, void (*handler)(int), bool ap) {

    if(!ap) {
        intrpt_num = 50;   //Allocate a low priority interrupt
        interrupt_allocate(1, interrupt_flags_exclusive, &intrpt_num);
        interrupt_registerhandler(intrpt_num, handler);
    }

    uint64_t v = apic_read(APIC_TIMER);
    v = v & ~0x007100ff;
    v |= (intrpt_num & 0xff);

    if(tsc_mode) {
        v |= (2 << 17); //Set TSC-Deadline mode
    } else {
        v |= (1 << 17); //Set Periodic mode

        uint32_t a, b, c, d;
        cpuid_request(0x16, 0, &a, &b, &c, &d);
        uint32_t apic_rate = c & 0xffff;

        cpuid_request(0x15, 0, &a, &b, &c, &d);
        uint32_t ratio_denom = a;
        uint32_t ratio_numer = b;
        uint32_t clock_freq = c;
        uint32_t tsc_rate = 0;
        if (ratio_denom != 0) {
            tsc_rate = (ratio_numer * clock_freq) / ratio_denom;
        }

        int manufacturer;
        uint32_t stepping, model, family, proc_type;
        cpuid_manufacturer(&manufacturer, &stepping, &model, &family, &proc_type);

        switch (manufacturer)
        {
            //TODO: Detect QEMU and handle separately
            case CPUID_VENDOR_AMD:
            {
                //Default to 100MHz
                if (apic_rate == 0)
                    apic_rate = 100;
            }
            break;
            case CPUID_VENDOR_INTEL:
            {
                if (apic_rate == 0)
                    switch (model)
                    {
                    //Nehalem
                    case 0x1a:
                    case 0x1e:
                    case 0x1f:
                    case 0x2e:

                    //Westmere
                    case 0x25:
                    case 0x2c:
                    case 0x2f: //CPUID holds, else must callibrate
                        break;

                    case 0x2a: //Sandy Bridge
                    case 0x2d: //Sandy Bridge EP
                    case 0x3a: //Ivy Bridge
                    case 0x3e: //Ivy Bridge EP
                    case 0x3c: //Haswell DT
                    case 0x3f: //Haswell MB
                    case 0x45: //Haswell ULT
                    case 0x46: //Haswell ULX
                    case 0x3d: //Broadwell
                    case 0x47: //Broadwell H
                    case 0x56: //Broadwell EP
                    case 0x4f: //Broadwell EX
                        //MSR_PLATFORM_INFO gives the apic rate
                        apic_rate = ((rdmsr(0xce) >> 8) & 0xFF) * 100;
                        break;

                    case 0x4e: //Skylake Y/U
                    case 0x5e: //Skylake H/S
                    case 0x55: //Skylake E

                    case 0x8e: //Kabylake Y/U
                    case 0x9e: //Kabylake H/S
                        apic_rate = 24;
                        break;
                    }
            }
            break;
        }
        //tick every 0.05ms
        uint64_t apic_freq = (apic_rate * 1000 * 1000) / 20000;
        apic_write(APIC_ICoR, apic_freq);
    }

    apic_write(APIC_TIMER, v);
    return intrpt_num;
}