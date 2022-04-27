/**
 * Copyright (c) 2022 Himanshu Goel
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */

#include "stddef.h"
#include "stdint.h"
#include "cpuid.h"

void cpuid_request(uint32_t page, uint32_t idx, uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx)
{

    uint32_t ax = page, bx = *ebx, cx = idx, dx = *edx;

    __asm__ volatile("cpuid\n\t"
                     : "=a"(ax), "=b"(bx), "=c"(cx), "=d"(dx)
                     : "a"(ax), "c"(cx));

    *eax = ax;
    *ebx = bx;
    *ecx = cx;
    *edx = dx;
}

void cpuid_manufacturer(int *manufacturer, uint32_t *stepping, uint32_t *model, uint32_t *family, uint32_t *proc_type)
{
    uint32_t eax, ebx, ecx, edx;
    cpuid_request(1, 0, &eax, &ebx, &ecx, &edx);
    if (stepping != NULL) *stepping = eax & 0x0F;
    if (model != NULL) *model = (eax & 0xF0) >> 4;
    if (family != NULL) *family = (eax & 0xF00) >> 8;
    if (proc_type != NULL) *proc_type = (eax & 0xF000) >> 12;

    cpuid_request(0, 0, &eax, &ebx, &ecx, &edx);
    char proc_str[13];
    proc_str[12] = 0;
    memcpy(proc_str, &ebx, sizeof(ebx));
    memcpy(proc_str + 4, &edx, sizeof(edx));
    memcpy(proc_str + 8, &ecx, sizeof(ecx));

    if (manufacturer != NULL)
        if (strcmp(proc_str, "GenuineIntel") == 0)
            *manufacturer = CPUID_VENDOR_INTEL;
        else if (strcmp(proc_str, "AuthenticAMD") == 0)
            *manufacturer = CPUID_VENDOR_AMD;
}