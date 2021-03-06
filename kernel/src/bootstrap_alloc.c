/**
 * Copyright (c) 2022 Himanshu Goel
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */

#include "stddef.h"
#include "stdint.h"
#include "string.h"

#include "local_spinlock.h"

#define BOOTSTRAP_ALLOC_AREA_SIZE (MiB(128))

static ALIGNED(KiB(4)) uint8_t bootstrap_alloc_area[BOOTSTRAP_ALLOC_AREA_SIZE];
static uint64_t bootstrap_alloc_pos = 0;
static int bootstrap_alloc_lock = 0;

void *bootstrap_malloc(size_t s)
{

    void *mem = NULL;

    if (s > 0)
    {

        if (s % 16 != 0) // Require all allocations to be 16-byte aligned
            s = ((s >> 4) + 1) << 4;

        local_spinlock_lock(&bootstrap_alloc_lock);
        if (bootstrap_alloc_pos + s < BOOTSTRAP_ALLOC_AREA_SIZE)
        {
            mem = &bootstrap_alloc_area[bootstrap_alloc_pos];
            bootstrap_alloc_pos += s;
        }
        local_spinlock_unlock(&bootstrap_alloc_lock);
    }
    return mem;
}

void bootstrap_free(void *mem, size_t s)
{
    // If another allocation has not been made yet, we can free the memory
    if (mem == NULL)
        return;

    if (s == 0)
        return;

    if (s % 16 != 0) // Require all allocations to be 16-byte aligned
        s = ((s >> 4) + 1) << 4;

    local_spinlock_lock(&bootstrap_alloc_lock);
    if (bootstrap_alloc_pos > s &&
        &bootstrap_alloc_area[bootstrap_alloc_pos - s] == (uint8_t *)mem)
        bootstrap_alloc_pos -= s;

    local_spinlock_unlock(&bootstrap_alloc_lock);
}