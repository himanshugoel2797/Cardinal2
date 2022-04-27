// Copyright (c) 2022 Himanshu Goel
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef STDLIB_H
#define STDLIB_H

#include "stddef.h"
#include "stdint.h"

#ifndef NULL
#define NULL ((void *)0)
#endif

void update_malloc_free_hooks(void *(*malloc_hook)(size_t), void (*free_hook)(void *));

void *malloc(size_t size);

void free(void *ptr);

char *itoa(int val, char *dst, int base);

char *ltoa(long long val, char *dst, int base);

int atoi(const char * ptr, int base);

#endif