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

#include <arch/uaccess.h>

/* Check that user space can read area specified. */
pok_bool_t ja_check_access_read(const void* __user addr, size_t size);
/* Check that user space can write area specified. */
pok_bool_t ja_check_access_write(void* __user addr, size_t size);

#ifndef JET_ARCH_DECLARE_USER_TO_KERNEL
/* 
 * Return address which can be directly accessed by the kernel for 
 * r/w from/to user space area.
 * 
 * User address should be checked before by 'ja_check_access_write'.
 * 
 * If arch defines macro 'JET_ARCH_DECLARE_USER_TO_KERNEL', it should
 * provide definition with same usage.
 */
void* ja_user_to_kernel(void* __user addr);
/* 
 * Return address which can be directly accessed by the kernel for 
 * read from user space area.
 * 
 * User address should be checked before by 'ja_check_access_read'.
 * 
 * If arch defines macro 'JET_ARCH_DECLARE_USER_TO_KERNEL', it should
 * provide definition with same usage.
 */
const void* ja_user_to_kernel_ro(const void* __user addr);
#endif /* JET_ARCH_DECLARE_USER_TO_KERNEL */

#endif /* __JET_ASP_UACCESS_H__ */
