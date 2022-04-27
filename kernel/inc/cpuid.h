// Copyright (c) 2022 Himanshu Goel
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CARDINAL_CPUID_H
#define CARDINAL_CPUID_H

#include "stdint.h"

typedef enum
{
    CPUID_FEAT_ECX_SSE3 = 1u << 0,
    CPUID_FEAT_ECX_PCLMUL = 1u << 1,
    CPUID_FEAT_ECX_DTES64 = 1u << 2,
    CPUID_FEAT_ECX_MONITOR = 1u << 3,
    CPUID_FEAT_ECX_DS_CPL = 1u << 4,
    CPUID_FEAT_ECX_VMX = 1u << 5,
    CPUID_FEAT_ECX_SMX = 1u << 6,
    CPUID_FEAT_ECX_EST = 1u << 7,
    CPUID_FEAT_ECX_TM2 = 1u << 8,
    CPUID_FEAT_ECX_SSSE3 = 1u << 9,
    CPUID_FEAT_ECX_CID = 1u << 10,
    CPUID_FEAT_ECX_FMA = 1u << 12,
    CPUID_FEAT_ECX_CX16 = 1u << 13,
    CPUID_FEAT_ECX_ETPRD = 1u << 14,
    CPUID_FEAT_ECX_PDCM = 1u << 15,
    CPUID_FEAT_ECX_DCA = 1u << 18,
    CPUID_FEAT_ECX_SSE4_1 = 1u << 19,
    CPUID_FEAT_ECX_SSE4_2 = 1u << 20,
    CPUID_FEAT_ECX_x2APIC = 1u << 21,
    CPUID_FEAT_ECX_MOVBE = 1u << 22,
    CPUID_FEAT_ECX_POPCNT = 1u << 23,
    CPUID_FEAT_ECX_AES = 1u << 25,
    CPUID_FEAT_ECX_XSAVE = 1u << 26,
    CPUID_FEAT_ECX_OSXSAVE = 1u << 27,
    CPUID_FEAT_ECX_AVX = 1u << 28,

    CPUID_FEAT_EDX_FPU = 1u << 0,
    CPUID_FEAT_EDX_VME = 1u << 1,
    CPUID_FEAT_EDX_DE = 1u << 2,
    CPUID_FEAT_EDX_PSE = 1u << 3,
    CPUID_FEAT_EDX_TSC = 1u << 4,
    CPUID_FEAT_EDX_MSR = 1u << 5,
    CPUID_FEAT_EDX_PAE = 1u << 6,
    CPUID_FEAT_EDX_MCE = 1u << 7,
    CPUID_FEAT_EDX_CX8 = 1u << 8,
    CPUID_FEAT_EDX_APIC = 1u << 9,
    CPUID_FEAT_EDX_SEP = 1u << 11,
    CPUID_FEAT_EDX_MTRR = 1u << 12,
    CPUID_FEAT_EDX_PGE = 1u << 13,
    CPUID_FEAT_EDX_MCA = 1u << 14,
    CPUID_FEAT_EDX_CMOV = 1u << 15,
    CPUID_FEAT_EDX_PAT = 1u << 16,
    CPUID_FEAT_EDX_PSE36 = 1u << 17,
    CPUID_FEAT_EDX_PSN = 1u << 18,
    CPUID_FEAT_EDX_CLF = 1u << 19,
    CPUID_FEAT_EDX_DTES = 1u << 21,
    CPUID_FEAT_EDX_ACPI = 1u << 22,
    CPUID_FEAT_EDX_MMX = 1u << 23,
    CPUID_FEAT_EDX_FXSR = 1u << 24,
    CPUID_FEAT_EDX_SSE = 1u << 25,
    CPUID_FEAT_EDX_SSE2 = 1u << 26,
    CPUID_FEAT_EDX_SS = 1u << 27,
    CPUID_FEAT_EDX_HTT = 1u << 28,
    CPUID_FEAT_EDX_TM1 = 1u << 29,
    CPUID_FEAT_EDX_IA64 = 1u << 30,
    CPUID_FEAT_EDX_PBE = 1u << 31
} CPUID_FEAT;

typedef enum
{
    CPUID_EAX = 0,
    CPUID_EBX,
    CPUID_ECX,
    CPUID_EDX
} CPUID_REG;

typedef enum
{
    CPUID_ECX_IGNORE = 0,
    CPUID_EAX_FIRST_PAGE = 1
} CPUID_REQUESTS;

typedef enum
{
    CPUID_VENDOR_INTEL = 0,
    CPUID_VENDOR_AMD,
} CPUID_VENDORS;

void cpuid_request(uint32_t page, uint32_t idx, uint32_t *a, uint32_t *b, uint32_t *c, uint32_t *d);
void cpuid_manufacturer(int *manufacturer, uint32_t *stepping, uint32_t *model, uint32_t *family, uint32_t *proc_type);

#endif