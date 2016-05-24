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

#include <arch/uaccess.h>

#define check_access_read ja_check_access_read
#define check_access_write ja_check_access_write

#include <libc.h>

/* 
 * Check that user space can read and write area specified.
 * 
 * TODO: Is this symlink needed?
 */
#define check_access_rw check_access_write

#ifndef NDEBUG
/* 
 * Copy from user memory to kernel one.
 * 
 * NOTE: Access check should be performed before.
 */
void __copy_from_user(void* to, const void* __user from, size_t n);
/* 
 * Copy from kernel memory to user one.
 * 
 * NOTE: Access check should be performed before.
 */
void __copy_to_user(void* __user to, const void* from, size_t n);
/*
 * Copy between areas in user space.
 * 
 * Both areas should be checked before.
 */
void __copy_user(void* __user to, const void* __user from, size_t n);

#else /* NDEBUG */
#define __copy_from_user(to, from, n) memcpy(to, from, n)
#define __copy_to_user(to, from, n) memcpy(to, from, n)
#define __copy_user(to, from, n) memcpy(to, from, n)
#endif /* NDEBUG */

/* Check that given *typed* user area is readable. */
#define check_user_read(ptr) check_access_read(ptr, sizeof(*ptr))
/* Check that given *typed* user area is writable. */
#define check_user_write(ptr) check_access_write(ptr, sizeof(*ptr))

/* 
 * Return value from *typed* user memory.
 * 
 * NOTE: Access check should be performed before.
 */
#define __get_user(ptr) ({typeof(*(ptr)) __val = *(ptr); __val; })

/* 
 * Return value of the field in user-space structure.
 * 
 * NOTE: Access check should be performed before.
 */
#define __get_user_f(ptr, field) ({typeof((ptr)->field) __val = (ptr)->field; __val; })

/* 
 * Put value to *typed* user memory.
 * 
 * NOTE: Access check should be performed before.
 */
#define __put_user(ptr, val) do {*(ptr) = (typeof(*ptr))(val); } while(0)

/* 
 * Set field of user-space structure.
 * 
 * NOTE: Access check should be performed before.
 */
#define __put_user_f(ptr, field, val) do {(ptr)->field = (typeof((ptr)->field))(val); } while(0)

/* 
 * Copy name to the user space.
 * 
 * Destination buffer should be checked before.
 */
void pok_copy_name_to_user(void* __user to, const char* name);

/*
 * Check whether given address may be executed.
 * 
 * TODO: Currently this is just replacement for POK_CHECK_VPTR_IN_PARTITION.
 * Probably, this check is completely unnesessary, because access to
 * given address is performed only from user space.
 * (Unlike to read/write access to user buffers, performed from kernel space).
 */
#define check_access_exec(addr) check_access_read(addr, sizeof(void*))


#endif /* __POK_UACCESS_H__ */
