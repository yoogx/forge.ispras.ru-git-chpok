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

#ifndef __JET_ASP_UACCESS_H__
#define __JET_ASP_UACCESS_H__

#include <types.h>
#include <common.h>
#include <asp/space.h>


/* 
 * Check that given address range doesn't belong to the kernel.
 * 
 * NOTE: Returning true *doesn't mean* that address belongs to the *user space*.
 */
pok_bool_t ja_access_ok(const void* __user addr, size_t size);

#endif /* __JET_ASP_UACCESS_H__ */
