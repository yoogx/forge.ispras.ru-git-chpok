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

#ifndef __JET_PPC_UACCESS_H__
#define __JET_PPC_UACCESS_H__

#define JET_ARCH_DECLARE_USER_TO_KERNEL 1

#define ja_user_to_kernel(addr) ((void*)(addr))
#define ja_user_to_kernel_ro(addr) ((const void*)(addr))

#endif /* __JET_PPC_UACCESS_H__ */
