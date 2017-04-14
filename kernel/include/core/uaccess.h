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

/*
 * Access to user space from kernel space.
 */

#ifndef __POK_UACCESS_H__
#define __POK_UACCESS_H__

#include <types.h>

#include <asp/uaccess.h>
#include <core/partition.h>

/* 
 * Return kernel address which can be used in the kernel for 
 * r/w from/to current partition.
 * 
 * If given area cannot be written by the user space, returns NULL.
 */
void* __kuser jet_user_to_kernel(void* __user addr, size_t size);

/* 
 * Return kernel address which can be used in the kernel for 
 * read from current partition.
 * 
 * If given area cannot be read by the user space, returns NULL.
 */
const void* __kuser jet_user_to_kernel_ro(const void* __user addr, size_t size);

/*
 * Return true if address *may* contain instruction, which can be
 * executed by the user in the current partition.
 * 
 * Used mainly for additional checks.
 * Kernel doesn't perform r/w to given address.
 */
pok_bool_t jet_check_access_exec(void* __user addr);


/* Shortcat for jet_user_to_kernel for typed pointers. */
#define jet_user_to_kernel_typed(addr) jet_user_to_kernel(addr, sizeof(*addr))

/* Shortcat for jet_user_to_kernel_ro for typed pointers. */
#define jet_user_to_kernel_typed_ro(addr) jet_user_to_kernel_ro(addr, sizeof(*addr))


/* 
 * Copy NULL-terminated string from user space to the kernel.
 * 
 * Note, that string in user space may occupy less space than its maximum
 * length.
 * 
 * On success, return pointer to the 'dest'. On fail return NULL.
 */
void* kstrncpy(char* dest, const char* __user src, size_t n);

/* 
 * Copy name to the user space.
 * 
 * Destination buffer should be checked before.
 */
void pok_copy_name_to_user(char* __kuser to, const char* name);

#ifdef POK_NEEDS_GDB

/* 
 * Return address which can be used by GDB for
 * r/w from/to given kernel or user space area.
 * 
 * Access is garanteed only if switched to address space of given partition.
 * 
 * If given area cannot be written, returns NULL.
 */
void* jet_addr_to_gdb(void* addr, size_t size,
    pok_partition_t* part);

/* 
 * Return address which can be used by GDB for
 * read from given kernel or user space area.
 * 
 * Access is garanteed only if switched to address space of given partition.
 * 
 * If given area cannot be read by the user space, returns NULL.
 */
const void* jet_addr_to_gdb_ro(const void* addr, size_t size,
    pok_partition_t* part);

#endif /* POK_NEEDS_GDB */


#endif /* __POK_UACCESS_H__ */
