/**
 * Copyright (c) 2022 Himanshu Goel
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */

#include "stdint.h"
#include "stdbool.h"
#include "kerndefs.h"
#include "stdlib.h"

//malloc and free hooks variables
static void *(*__malloc_hook)(size_t) = NULL;
static void (*__free_hook)(void *) = NULL;

void update_malloc_free_hooks(void *(*malloc_hook)(size_t), void (*free_hook)(void *))
{
    __malloc_hook = malloc_hook;
    __free_hook = free_hook;
}

void* malloc(size_t size)
{
    if (__malloc_hook)
        return __malloc_hook(size);
    else
        return NULL;
}

void free(void *ptr)
{
    if (__free_hook)
        __free_hook(ptr);
}

char *WEAK itoa(int val, char *dst, int base) {
    char *iter = dst;
    int len = 0, start = 0;
    uint32_t tmp = (uint32_t)val;

    if (base == 0)
        return NULL;

    if (dst == NULL)
        return NULL;

    if (val == 0) {
        dst[0] = '0';
        dst[1] = 0;
        return dst;
    }

    if (val < 0 && base == 10)
        tmp = (uint32_t)-val;

    while (tmp != 0) {
        char v = tmp % base;
        char c = 0;

        if (v <= 9)
            c = '0' + v;
        else if (v <= 35)
            c = 'a' + (v - 10);

        *(iter++) = c;
        *iter = 0;
        len++;

        tmp = tmp / base;
    }

    if (val < 0 && base == 10) {
        *(iter++) = '-';
        *iter = 0;
        len++;
    }

    while (start < len) {
        char t = dst[start];
        dst[start] = dst[len - 1];
        dst[len - 1] = t;

        len--;
        start++;
    }

    return dst;
}

char* WEAK ltoa(long long val, char *dst, int base) {
    char *iter = dst;
    int len = 0, start = 0;
    uint64_t tmp = (uint64_t)val;

    if (base == 0)
        return NULL;

    if (dst == NULL)
        return NULL;

    if (val == 0) {
        dst[0] = '0';
        dst[1] = 0;
        return dst;
    }

    if (val < 0 && base == 10)
        tmp = (uint64_t)-val;

    while (tmp != 0) {
        char v = tmp % base;
        char c = 0;

        if (v <= 9)
            c = '0' + v;
        else if (v <= 35)
            c = 'a' + (v - 10);

        *(iter++) = c;
        *iter = 0;
        len++;

        tmp = tmp / base;
    }

    if (val < 0 && base == 10) {
        *(iter++) = '-';
        *iter = 0;
        len++;
    }

    while (start < len) {
        char t = dst[start];
        dst[start] = dst[len - 1];
        dst[len - 1] = t;

        len--;
        start++;
    }

    return dst;
}

int WEAK atoi(const char * ptr, int base) {

    if(base == 16) {
        int val = 0;
        int digit = 0;
        while(true) {
            if(*ptr >= '0' && *ptr <= '9') {
                val |= (*ptr - '0');
            } else if(*ptr >= 'a' && *ptr <= 'f') {
                val |= (*ptr - 'a') + 10;
            } else if(*ptr >= 'A' && *ptr <= 'F') {
                val |= (*ptr - 'A') + 10;
            } else {
                val = val >> 4;
                return val;
            }

            ptr++;
            val = val << 4;
        }
    } else
        return -1;
}