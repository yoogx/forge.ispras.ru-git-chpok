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

#ifndef __JET_ALLOC_H__
#define __JET_ALLOC_H__

#include <asp/alloc.h>

/* 
 * Allocate memory chunk of given size.
 * 
 * Returned address should be suitably alignment for any type which fit
 * into the requested size.
 * 
 * May be called only during OS initialization.
 *
 * Note: Failure to allocate memory is treated as error
 * (that is function never returns NULL).
 */
void* jet_mem_alloc (size_t size);


#endif /* __JET_ALLOC_H__ */
