// Copyright (c) 2022 Himanshu Goel
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT


#ifndef CARDINAL2_DEBUG_H
#define CARDINAL2_DEBUG_H

int init_serial_debug();
int debug_shell(char (*input_stream)(), void (*output_stream)(char));

#endif