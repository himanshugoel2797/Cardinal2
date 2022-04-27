/**
 * Copyright (c) 2022 Himanshu Goel
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */

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