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

#include <asp/space.h>
#include <assert.h>

void ja_space_layout_get(jet_space_id space_id,
    struct jet_space_layout* space_layout)
{
    assert(0);
}


void* ja_space_get_heap(jet_space_id space_id)
{
    assert(0);
}

struct jet_kernel_shared_data* __kuser ja_space_shared_data(jet_space_id space_id)
{
    assert(0);
}

/**
 * Jump to the user space.
 * 
 * Kernel stack passed as 'sp' will be used in interrupts/syscalls.
 */
void ja_user_space_jump(
    jet_stack_t stack_kernel,
    jet_space_id space_id, /* Actually, unused (should already be set with ja_space_switch). */
    void (__user * entry_user)(void),
    jet_ustack_t stack_user)
{
    assert(0);
}

void   ja_space_switch (jet_space_id new_space_id)
{
    assert(0);
}

jet_space_id ja_space_get_current (void)
{
    assert(0);
}

void ja_ustack_init (jet_space_id space_id)
{
    assert(0);
}

jet_ustack_t ja_ustack_alloc (jet_space_id space_id, size_t stack_size)
{
    assert(0);
}

struct jet_fp_store* ja_alloc_fp_store(void)
{
    assert(0);
}

void ja_fp_save(struct jet_fp_store* fp_store)
{
    assert(0);
}

void ja_fp_restore(struct jet_fp_store* fp_store)
{
    assert(0);
}

void ja_fp_init(void)
{
    assert(0);
}
