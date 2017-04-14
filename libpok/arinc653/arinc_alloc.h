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

#ifndef __LIBJET_ARINC_ALLOC_H__
#define __LIBJET_ARINC_ALLOC_H__

/* Allocator of memory for ARINC needs. */

#include <config.h>
#include <types.h>

/* Initializa ARINC allocator. */
void arinc_allocator_init(void);

/* 
 * Allocate memory of given size and alignment.
 * 
 * Return NULL if insufficient memory.
 */
void* arinc_alloc(size_t size, size_t alignment);

/*
 * State of the arinc allocator.
 */
typedef char* arinc_allocator_state;

/*
 * Return current state of the allocator.
 */
arinc_allocator_state arinc_allocator_get_state(void);

/*
 * Restore state of the arinc allocator.
 * State should be previously obtained with arinc_allocator_get_state().
 */
void arinc_allocator_reset_state(arinc_allocator_state state);

#endif /* __LIBJET_ARINC_ALLOC_H__ */
