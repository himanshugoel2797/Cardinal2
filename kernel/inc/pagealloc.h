// Copyright (c) 2022 Himanshu Goel
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CARDINAL_PHYSMEM_H
#define CARDINAL_PHYSMEM_H

#include "stdint.h"

typedef enum {
    physmem_alloc_flags_reclaimable = (1 << 0),
    physmem_alloc_flags_data = (1 << 1),
    physmem_alloc_flags_instr = (1 << 2),
    physmem_alloc_flags_pagetable = (1 << 3),
    physmem_alloc_flags_zero = (1 << 4),
    physmem_alloc_flags_32bit = (1 << 5),
} physmem_alloc_flags_t;

int pagealloc_init();

uintptr_t pagealloc_alloc(int domain, int color, physmem_alloc_flags_t flags, uint64_t size);

void pagealloc_free(uintptr_t addr, uint64_t size);

#endif