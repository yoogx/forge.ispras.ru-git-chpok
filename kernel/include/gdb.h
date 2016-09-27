/*
 * Institute for System Programming of the Russian Academy of Sciences
 * Copyright (C) 2016 ISPRAS
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, Version 3.
 *
 * This program is distributed in the hope # that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License version 3 for more details.
 */

#ifndef __POK_GDB_H__
#define __POK_GDB_H__

#include <libc.h>
#include <asp/gdb.h>

#define NUMREGBYTES (NUMREGS * 4)

/*
 * Whether we are in user space.
 * 
 * This variable is set when we jump/return to user space and cleared
 * on return from there.
 * 
 * This variable is checked in interrupts handlers for decide, whether
 * need to store 'global_thread_stack' localy.
 */
extern volatile pok_bool_t pok_in_user_space;

/* Callback function for append string(or other data) into
 * some stream.
 *
 * 'data' - data for append
 * 'len' - length of the data for append
 * 'cb_data' - opaque value, specific for concrete callback.
 */
typedef void (*print_cb_t)(const void* data, size_t len, void* cb_data);

#endif /* __POK_GDB_H__ */
