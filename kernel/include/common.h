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
 *
 * This file also incorporates work covered by POK License.
 * Copyright (c) 2007-2009 POK team
 */

#ifndef __POK_COMMON_H__
#define __POK_COMMON_H__

#include <compiler.h>
#include <stdint.h>

/*
 * Mark pointer as pointer to user space.
 * 
 * Pointers of such types should be dereferenced only with special function
 * (see uaccess.h).
 * 
 * Some checkers can set this mark to something, which *actually*
 * prohibits incorrect usage of marked pointers.
 * 
 * Usage example:
 * 
 *    void a(char* __user buf);
 */
#define __user

/*
 * Kernel-space address of user memory area.
 * 
 * The address can be accessed only while being in corresponded space.
 */
#define __kuser

/*
 * Kernel-space address used for remote access.
 *
 * The address can only be used in copy_to_client()/copy_from_client() calls.
 */
#define __remote


/**
 * Casts pointer to member of structure to pointer to structure itself.
 * 
 * @ptr: the pointer to the member
 * @type: the type of the container struct
 * @member: name of the member within the container.
 * 
 * Implementation is taken from Linux kernel.
 */
#define container_of(ptr, type, member) ({                   \
    const typeof( ((type *)0)->member)* __mptr = (ptr);      \
    (type *)( (char*) __mptr - offsetof(type, member) ); })

/*
 * Return minimal value, which is greater-or-equal than given value
 * and has corresponded alignment.
 * 
 * @align should be constant and be power of 2.
 */
static inline unsigned long ALIGN_VAL(unsigned long val, unsigned long align)
{
    return (val + align - 1) & (~(align - 1));
}


/* return a*b/c */
static inline uint64_t muldiv64(uint64_t a, uint32_t b, uint32_t c)
{
    uint32_t a_h, a_l, res_h, res_l;
    uint64_t prod_l, prod_h;

    a_l = (uint32_t) a;
    a_h = (uint32_t)(a >> 32);
    //Now (a_h<<32 + a_l) = a

    prod_l = (uint64_t)a_l * (uint64_t)b;
    prod_h = (uint64_t)a_h * (uint64_t)b;
    prod_h += (prod_l >> 32);
    prod_l = prod_l & 0xffffffff;
    //Now (prod_h<<32 + prod_l) = a*b

    res_h = prod_h / c;
    res_l = (((prod_h % c) << 32) + prod_l) / c;
    //Now (res_h<<32 + res_l) = a*b/c

    return ((uint64_t)res_h<<32) + res_l;
}

#endif /* !__POK_COMMON_H__ */
