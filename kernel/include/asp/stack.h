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

#ifndef __JET_ASP_STACK_H__
#define __JET_ASP_STACK_H__

/* 
 * Arch header should define type 'jet_stack_t' as pointer to allocated
 * (kernel) stack.
 */
#include <arch/stack.h>

/**
 * Alloc (kernel) stack of specified size.
 * 
 * Return pointer to the head of the stack, that is when stack is assumed
 * to be empty.
 * 
 * May be called only during OS initialization.
 */
jet_stack_t pok_stack_alloc(uint32_t stack_size);

#endif /* __JET_ASP_STACK_H__ */
