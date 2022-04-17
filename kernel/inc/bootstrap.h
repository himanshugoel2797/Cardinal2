// Copyright (c) 2022 Himanshu Goel
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CARDINAL2_BOOTSTRAP_H
#define CARDINAL2_BOOTSTRAP_H
#include "stdint.h"

void* bootstrap_malloc(size_t size);
void bootstrap_free(void* ptr, size_t size);

#endif //CARDINAL2_BOOTSTRAP_H