/**
 * Copyright (c) 2022 Himanshu Goel
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */


#include "stddef.h"
#include "stdint.h"
#include "string.h"
#include "stdbool.h"
#include "stdlib.h"
#include "kerndefs.h"
#include "vmem.h"
#include "bootinfo.h"
#include "acpi/acpi_tables.h"
#include "acpi/madt.h"
#include "acpi/fadt.h"
#include "acpi/hpet.h"
#include "acpi/mcfg.h"

static RSDPDescriptor20 *rsdp;

bool acpi_validate_csum(ACPISDTHeader *header)
{
    uint8_t sum = 0;
    for (uint32_t i = 0; i < header->Length; i++)
    {
        sum += ((char *)header)[i];
    }

    return sum == 0;
}

void *acpi_find(const char *table_name)
{
    if (rsdp == NULL){
        bootinfo_t *bootinfo = (bootinfo_t *)bootinfo_get();
        rsdp = malloc(sizeof(RSDPDescriptor20));
        memcpy(rsdp, (void*)bootinfo->acpi_rsdp_addr, sizeof(RSDPDescriptor20));
    }

    if (rsdp->firstPart.Revision != ACPI_VERSION_1 && rsdp->XsdtAddress)
    {
        XSDT *xsdt = (XSDT *)vmem_phystovirt((intptr_t)rsdp->XsdtAddress, MiB(2), vmem_flags_cachewriteback);
        if (!acpi_validate_csum((ACPISDTHeader *)xsdt))
            return (void *)-1;

        int entries = XSDT_GET_POINTER_COUNT((xsdt->h));

        for (int i = 0; i < entries; i++)
        {
            if (xsdt->PointerToOtherSDT[i] == 0)
                continue;
            ACPISDTHeader *h = (ACPISDTHeader *)vmem_phystovirt((intptr_t)xsdt->PointerToOtherSDT[i], MiB(2), vmem_flags_cachewriteback);
            char tmp_table_name[5];
            tmp_table_name[4] = 0;
            tmp_table_name[0] = h->Signature[0];
            tmp_table_name[1] = h->Signature[1];
            tmp_table_name[2] = h->Signature[2];
            tmp_table_name[3] = h->Signature[3];
            if (!memcmp(h->Signature, table_name, 4) && acpi_validate_csum(h))
            {
                return (void *)h;
            }
        }
    }
    else if ((rsdp->firstPart.Revision == ACPI_VERSION_1) | (!rsdp->XsdtAddress))
    {
        RSDT *rsdt = (RSDT *)vmem_phystovirt((intptr_t)rsdp->firstPart.RsdtAddress, MiB(2), vmem_flags_cachewriteback);
        if (!acpi_validate_csum((ACPISDTHeader *)rsdt))
            return NULL;

        int entries = RSDT_GET_POINTER_COUNT((rsdt->h));

        for (int i = 0; i < entries; i++)
        {
            ACPISDTHeader *h = (ACPISDTHeader *)vmem_phystovirt((intptr_t)rsdt->PointerToOtherSDT[i], MiB(2), vmem_flags_cachewriteback);
            if (!memcmp(h->Signature, table_name, 4) && acpi_validate_csum(h))
            {
                return (void *)h;
            }
        }
    }

    return NULL;
}