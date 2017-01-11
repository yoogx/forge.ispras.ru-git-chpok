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

#include <core/space.h>
#include <core/partition.h>
#include <assert.h>
#include <asp/arch.h>

static int global_access_counter = 1;

/*
 * Grant or revoke rw access from kernel space to memory for all
 * user spaces.
 * 
 * Initially global access is granted.
 * 
 * Functions can be called only for toggling access, that is
 * _grant_ can be called only when access is not granted and
 * _revoke_ can be called only when access is granted.
 */
void jet_uspace_revoke_access(void)
{
    assert(global_access_counter == 1);
    assert(!ja_preempt_enabled());
    ja_uspace_revoke_access();
    global_access_counter = 0;
}

#ifdef POK_NEEDS_GDB

/*
 * Grant or revoke rw access from kernel space to memory for all
 * user spaces for debug purposes.
 * 
 * These functions operates on *counter*: _grant_ increment this counter,
 * and _revoke_ decrement it. Debug access is allowed only when counter
 * is positive.
 * 
 * These functions may be used for organize section of code,
 * where access to all memory is allowed. None of other jet_uspace_*
 * functions are allowed in this section.
 */
void jet_uspace_grant_access_debug(void)
{
    assert(!ja_preempt_enabled());
    if(++global_access_counter == 0)
        ja_uspace_grant_access();
}
void jet_uspace_revoke_access_debug(void)
{
    assert(global_access_counter > 0);
    assert(!ja_preempt_enabled());
    if(--global_access_counter == 1)
        ja_uspace_revoke_access();
}

#endif /* POK_NEEDS_GDB */

/* 
 * Revoke rw access from kernel space to memory local to given
 * partition.
 * 
 * Initially local access is granted for all user spaces.
 * 
 * Function can be called only for toggling access, that is
 * _revoke_ can be called only when access is granted.
 * 
 * PRE: partition should have non-zero space_id.
 */
void jet_uspace_revoke_access_local(struct _pok_partition* part)
{
    pok_bool_t is_preempt_enabled = ja_preempt_enabled();
    if(is_preempt_enabled)
        ja_preempt_disable();

    assert(part->space_id != 0);
    assert(part->is_local_access_granted);

    part->is_local_access_granted = FALSE;

    ja_uspace_revoke_access_local(part->space_id);

    if(is_preempt_enabled)
        ja_preempt_enable();
}

void jet_uspace_reset_access_local(struct _pok_partition* part)
{
    pok_bool_t is_preempt_enabled = ja_preempt_enabled();
    if(is_preempt_enabled)
        ja_preempt_disable();

    if(!part->is_local_access_granted) {
        part->is_local_access_granted = TRUE;
        ja_uspace_grant_access_local(part->space_id);
    }

    if(is_preempt_enabled)
        ja_preempt_enable();
}
