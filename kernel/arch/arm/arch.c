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

#include <asp/arch.h>
#include <assert.h>

void ja_preempt_disable (void)
{
    assert(0);
}

void ja_preempt_enable (void)
{
    assert(0);
}

pok_bool_t ja_preempt_enabled(void)
{
    assert(0);
    return 0;
}

void ja_inf_loop(void)
{
    assert(0);
}

void ja_cpu_reset(void)
{
    assert(0);
}

#include <asp/bsp_common.h>

pok_ret_t pok_bsp_get_info(void * __user addr)
{
    assert(0);
    return POK_ERRNO_OK;
}
