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

#ifndef __JET_ASP_ALLOC_H__
#define __JET_ASP_ALLOC_H__

#include <types.h>

/* 
 * Allocate memory chunk of given size and with given alignment.
 * 
 * May be called only during OS initialization.
 * 
 * 'alignment' should be power of 2.
 * 
 * Note: Failure to allocate memory is treated as error
 * (that is function never returns NULL).
 */
void* ja_mem_alloc_aligned(size_t size, unsigned int alignment);

/* 
 * Returned alignment suitable for any type which fits into the
 * requested size.
 */
unsigned int ja_mem_get_alignment(size_t size);

#endif /* __JET_ASP_ALLOC_H__ */
