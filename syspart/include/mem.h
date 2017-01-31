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

#ifndef __POK_SYSNET_MEM_H__
#define __POK_SYSNET_MEM_H__

#include <core/syscall.h>

#define ALIGN_UP(addr,size) (((addr)+((size)-1))&(~((size)-1)))

static inline uintptr_t pok_virt_to_phys(void * virt) {
   //return pok_syscall1(POK_SYSCALL_MEM_VIRT_TO_PHYS, (uintptr_t) virt);
}

static inline void* pok_phys_to_virt(uintptr_t phys) {
   //return (void *) pok_syscall1(POK_SYSCALL_MEM_PHYS_TO_VIRT, phys);
}

#endif

