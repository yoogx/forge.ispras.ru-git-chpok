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

void ja_uspace_grant_access(void)
{
    assert(0);
}

void ja_uspace_revoke_access(void)
{
    assert(0);
}

void ja_uspace_grant_access_local(jet_space_id space_id)
{
    assert(0);
}

void ja_uspace_revoke_access_local(jet_space_id space_id)
{
    assert(0);
}


/*
 * Jump to the user space.
 *
 * Kernel stack passed as 'sp' will be used in interrupts/syscalls.
 */
void ja_user_space_jump(
    jet_stack_t stack_kernel,
    jet_space_id space_id, /* Actually, unused (should already be set with ja_space_switch). */
    void (__user * entry_user)(void),
    uintptr_t stack_user)
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

size_t ja_ustack_get_alignment(void)
{
    return 8;
}
