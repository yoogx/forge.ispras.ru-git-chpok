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

/// Arch template starts

#ifndef __POK_ARCH_UACCESS_H__
#define __POK_ARCH_UACCESS_H__

#include <types.h>
#include <common.h>

/* Check that user space can read area specified. */
pok_bool_t ja_check_access_read(const void* __user addr, size_t size);
/* Check that user space can write area specified. */
pok_bool_t ja_check_access_write(void* __user addr, size_t size);

#endif /* __POK_ARCH_UACCESS_H__ */

/// Arch template ends
