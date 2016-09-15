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

/* Stack for the (kernel) thread. */

#ifndef __JET_X86_STACK_H__
#define __JET_X86_STACK_H__

#include <stdint.h>

/*
 * Any stack returned by pok_stack_alloc() has two words *above* it
 * filled with zeros.
 * 
 * So, whenever %ebp register points to the beginning of the stack,
 * it is treated as the last stack frame.
 */
typedef uint32_t jet_stack_t;

#endif /* __JET_X86_STACK_H__ */
