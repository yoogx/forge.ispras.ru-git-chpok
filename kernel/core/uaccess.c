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

#include <uaccess.h>
#include <assert.h>
#include <libc.h>

#ifndef NDEBUG
void __copy_from_user(void* to, const void* __user from, size_t n)
{
    assert(check_access_read(from, n));
    memcpy(to, from, n);
}

void __copy_to_user(void* __user to, const void* from, size_t n)
{
    assert(check_access_write(to, n));
    memcpy(to, from, n);
}

void __copy_user(void* __user to, const void* __user from, size_t n)
{
    memcpy(to, from, n);
}
#endif /* NDEBUG */

pok_bool_t copy_from_user(void* to, const void* __user from, size_t n)
{
    if(!check_access_read(from, n)) return FALSE;
    memcpy(to, from, n);
    return TRUE;
}

static inline pok_bool_t copy_to_user(void* __user to, const void* from, size_t n)
{
    if(!check_access_write(to, n)) return FALSE;
    memcpy(to, from, n);
    return TRUE;
}

/* 
 * Copy name to the user space.
 * 
 * Destination buffer should be checked before.
 */
void pok_copy_name_to_user(void* __user to, const char* name)
{
    // How many bytes to copy.
    size_t n = strnlen(name, MAX_NAME_LENGTH);
    if(n != MAX_NAME_LENGTH) n++; // null-byte should be copied too.
    
    __copy_to_user(to, name, n);
}
