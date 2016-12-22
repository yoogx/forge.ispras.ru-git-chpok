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

#include <asp/uaccess.h>
#include <assert.h>


void* __kuser ja_user_to_kernel_space(void* __user addr, size_t size,
    jet_space_id space_id)
{
    assert(0);
}

const void* __kuser ja_user_to_kernel_ro_space(const void* __user addr, size_t size,
    jet_space_id space_id)
{
    assert(0);
}

pok_bool_t ja_check_access_exec(void* __user addr, jet_space_id space_id)
{
    assert(0);
}
