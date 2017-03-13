/*
 * Institute for System Programming of the Russian Academy of Sciences
 * Copyright (C) 2017 ISPRAS
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

#include <core/syscall.h>
#include <core/uaccess.h>

pok_ret_t syscall_handler(
        pok_syscall_id_t syscall_id,
        pok_syscall_args_t* __user args)
{
    pok_syscall_args_t* syscall_args = jet_user_to_kernel_typed(args);

    if(syscall_args == NULL) {
        return POK_ERRNO_EINVAL;
    } else {
        return pok_core_syscall (syscall_id, syscall_args);
    }
}
