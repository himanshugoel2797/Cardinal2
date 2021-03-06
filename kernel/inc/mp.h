// Copyright (c) 2022 Himanshu Goel
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT


#ifndef CARDINAL_SYSMP_H
#define CARDINAL_SYSMP_H

#include "kerndefs.h"

int mp_tls_setup();

int mp_tls_alloc(int bytes);

TLS void* mp_tls_get(int off);

int mp_corecount(void);

int mp_platform_getstatesize(void);

void mp_platform_getstate(void* buf);

void mp_platform_setstate(void* buf);

void mp_platform_getdefaultstate(void *buf, void *stackpointer, void *instr_ptr, void *args0, void *args1);

#endif