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
 * Return kernel address which can be used in the kernel for
 * r/w from/to given user space area.
 * 
 * If given area cannot be written by the user space, returns NULL.
 */
void* __kuser ja_user_to_kernel_space(void* __user addr, size_t size,
    jet_space_id space_id);

/* 
 * Return kernel address which can be used in the kernel for
 * read from given user space area.
 * 
 * If given area cannot be read by the user space, returns NULL.
 */
const void* __kuser ja_user_to_kernel_ro_space(const void* __user addr, size_t size,
    jet_space_id space_id);

/*
 * Return true if address *may* contain instruction, which can be
 * executed by the user in the given space.
 * 
 * Used mainly for additional checks.
 * Kernel doesn't perform r/w to given address.
 */
pok_bool_t ja_check_access_exec(void* __user addr, jet_space_id space_id);

#ifdef POK_NEEDS_GDB

/* 
 * Return address which can be used by GDB for
 * r/w from/to given kernel or user space area.
 * 
 * Access is garanteed only if switched to given space.
 * 
 * If given area cannot be written, returns NULL.
 */
void* ja_addr_to_gdb(void* addr, size_t size,
    jet_space_id space_id);

/* 
 * Return address which can be used by GDB for
 * read from given kernel or user space area.
 * 
 * Access is garanteed only if switched to given space.
 * 
 * If given area cannot be read by the user space, returns NULL.
 */
const void* ja_addr_to_gdb_ro(const void* addr, size_t size,
    jet_space_id space_id);

#endif /* POK_NEEDS_GDB */

#endif /* __JET_ASP_UACCESS_H__ */
