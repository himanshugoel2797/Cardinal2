/**
 * Copyright (c) 2022 Himanshu Goel
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */


#include "stdint.h"
#include "stddef.h"
#include "stdbool.h"
#include "stdlib.h"
#include "string.h"
#include "kerndefs.h"

#include "vmem.h"
#include "acpi/acpi_tables.h"
#include "acpi/madt.h"
#include "interrupts/interrupts.h"

typedef struct {
    uint32_t id;
    uint32_t global_intr_base;
    uint32_t volatile *base_addr;
} ioapic_t;

static ioapic_t *ioapics;
static int ioapic_cnt;

static void ioapic_write(int idx, uint32_t off, uint32_t val) {
    ioapics[idx].base_addr[0] = off;
    ioapics[idx].base_addr[4] = val;
}

static uint32_t ioapic_read(int idx, uint32_t off) {
    ioapics[idx].base_addr[0] = off;
    return ioapics[idx].base_addr[4];
}

static void ioapic_map(uint32_t idx, uint32_t irq_pin, uint32_t irq, bool active_low, bool level_trigger) {

    //configure this override
    const uint32_t low_index = 0x10 + irq_pin*2;
    const uint32_t high_index = 0x10 + irq_pin*2 + 1;

    uint32_t high = ioapic_read(idx, high_index);
    high &= ~0xff000000;
    //high |= (0xff000000);
    //bsp is the destination
    high |= (interrupt_get_cpuidx() << 24);
    ioapic_write(idx, high_index, high);

    uint32_t low = ioapic_read(idx, low_index);

    // set the polarity
    low &= ~(1<<13);
    low |= ((active_low & 1) << 13);

    low &= ~(1<<15);
    low |= ((level_trigger & 1) << 15);

    // set delivery vector
    low &= ~0xff;
    low |= irq & 0xff;

    // set to fixed destination mode
    low &= ~(1<<11);
    low |= (0 << 11);

    // set to fixed delivery mode
    low &= ~0x700;
    low |= 0 << 8;


    // unmask the interrupt
    low &= ~(1<<16);

    ioapic_write(idx, low_index, low);
}

static void ioapic_setmask(uint32_t idx, uint32_t irq_pin, bool mask) {
    //configure this override
    const uint32_t low_index = 0x10 + irq_pin*2;
    uint32_t low = ioapic_read(idx, low_index);

    // unmask the interrupt
    low &= ~(1<<16);
    if(mask)
        low |= (1 << 16);

    ioapic_write(idx, low_index, low);
}


void interrupt_mapinterrupt(uint32_t line, int irq, bool active_low, bool level_trig) {
    int ioapic_idx = 0;
    uint32_t ioapic_close_intr_base = 0;

    for(int i = 0; i < ioapic_cnt; i++)
        if(ioapics[i].global_intr_base < line && ioapics[i].global_intr_base > ioapic_close_intr_base) {
            ioapic_close_intr_base = ioapics[i].global_intr_base;
            ioapic_idx = i;
        }

    ioapic_map(ioapic_idx, line, irq, active_low, level_trig);
}

void interrupt_setmask(uint32_t line, bool mask) {
    int ioapic_idx = 0;
    uint32_t ioapic_close_intr_base = 0;

    for(int i = 0; i < ioapic_cnt; i++)
        if(ioapics[i].global_intr_base < line && ioapics[i].global_intr_base > ioapic_close_intr_base) {
            ioapic_close_intr_base = ioapics[i].global_intr_base;
            ioapic_idx = i;
        }

    ioapic_setmask(ioapic_idx, line, mask);
}

int ioapic_init() {
    //Read the MADT table
    MADT *madt = acpi_find(MADT_SIG);
    if (madt == NULL)
        return -1;

    uint32_t madt_len = madt->h.Length - 8 - sizeof(ACPISDTHeader);
    uint32_t lapic_cnt = 0;
    uint32_t ioapic_cnt = 0;
    uint32_t isaovr_cnt = 0;

    for (uint32_t i = 0; i < madt_len;)
    {
        MADT_EntryHeader *hdr = (MADT_EntryHeader*)&madt->entries[i];
        switch(hdr->type)
        {
            case MADT_LAPIC_ENTRY_TYPE:
                lapic_cnt++;
                break;
            case MADT_IOAPIC_ENTRY_TYPE:
                ioapic_cnt++;
                break;
            case MADT_ISAOVR_ENTRY_TYPE:
                isaovr_cnt++;
                break;
        }
        i += hdr->entry_size;
        if (hdr->entry_size == 0)
            i += 8;
    }

    ioapics = malloc(sizeof(ioapic_t) * ioapic_cnt);

    uint32_t ioapic_idx = 0;
    for (uint32_t off = 0; off < madt_len;)
    {
        MADT_EntryHeader *hdr = (MADT_EntryHeader*)&madt->entries[off];
        if (hdr->type == MADT_IOAPIC_ENTRY_TYPE)
        {
            MADT_EntryIOAPIC *ioapic = (MADT_EntryIOAPIC*)&madt->entries[off];

            ioapics[ioapic_idx].id = (uint32_t)ioapic->id;
            ioapics[ioapic_idx].base_addr = (uint32_t volatile *)vmem_phystovirt(ioapic->base_addr, KiB(8), vmem_flags_uncached);
            ioapics[ioapic_idx].global_intr_base = (uint32_t)ioapic->global_sys_int_base;

            uint64_t available_redirs = ((ioapic_read(ioapic_idx, 0x01) >> 16) & 0xff) + 1;
            for (uint32_t i = 0; i < available_redirs; i++)
            {
                //Configure this entry as normal
                ioapic_map(ioapic_idx, i, i + ioapic->global_sys_int_base + 0x20, false, false);
                ioapic_setmask(ioapic_idx, i, true);
            }

            ioapic_idx++;
        }
        off += hdr->entry_size;
        if (hdr->entry_size == 0)
            off += 8;
    }
    
    //Configure the detected overrides
    for (uint32_t off = 0; off < madt_len;)
    {
        MADT_EntryHeader *hdr = (MADT_EntryHeader*)&madt->entries[off];
        if (hdr->type == MADT_ISAOVR_ENTRY_TYPE)
        {
            MADT_EntryISAOVR *isaovr = (MADT_EntryISAOVR*)&madt->entries[off];
            for (ioapic_idx = 0; ioapic_idx < ioapic_cnt; ioapic_idx++)
            {
                uint64_t available_redirs = ((ioapic_read(ioapic_idx, 0x01) >> 16) & 0xff) + 1;
                if (ioapics[ioapic_idx].global_intr_base <= isaovr->global_sys_int && ioapics[ioapic_idx].global_intr_base + available_redirs > isaovr->global_sys_int)
                {
                    uint32_t irq_pin = isaovr->global_sys_int - ioapics[ioapic_idx].global_intr_base;
                    ioapic_map(ioapic_idx, irq_pin, isaovr->irq_src + 0x20, isaovr->flags & 2 /*active_low*/, isaovr->flags & 8 /*level_trigger*/);
                    ioapic_setmask(ioapic_idx, irq_pin, true);
                    break;
                }
            }
        }
        off += hdr->entry_size;
        if (hdr->entry_size == 0)
            off += 8;
    }

    return 0;
}