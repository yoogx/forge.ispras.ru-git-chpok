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

#include <asp/stack.h>
#include <asp/cswitch.h>
#include <assert.h>

struct jet_context* ja_context_init(jet_stack_t sp, void (*entry)(void))
{
    assert(0);
}

void ja_context_switch (struct jet_context** old_sp, struct jet_context* new_sp)
{
    assert(0);
}

void ja_context_jump(struct jet_context* new_sp)
{
    assert(0);
}

void ja_context_restart(jet_stack_t sp, void (*entry)(void))
{
    assert(0);
}

void ja_context_restart_and_save(jet_stack_t sp, void (*entry)(void),
        struct jet_context** new_context_p)
{
    assert(0);
}
