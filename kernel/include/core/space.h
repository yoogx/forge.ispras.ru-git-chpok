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

#ifndef __JET_SPACE_H__
#define __JET_SPACE_H__

#include <asp/space.h>
#include <config.h>

#define jet_ustack_get_alignment() ja_ustack_get_alignment()

#define pok_space_switch ja_space_switch
#define pok_space_get_current() ja_space_get_current()

#define jet_user_space_jump ja_user_space_jump

/*
 * Revoke rw access from kernel space to memory for all
 * user spaces for debug purposes.
 * 
 * Initially global access is granted.
 * 
 * Should be called only once with global preemption disabled.
 */
void jet_uspace_revoke_access(void);

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
void jet_uspace_grant_access_debug(void);
void jet_uspace_revoke_access_debug(void);

#endif /* POK_NEEDS_GDB */

struct _pok_partition;

/* 
 * Revoke rw access from kernel space to memory local to given
 * partition.
 * 
 * Initially local access is granted for all user spaces.
 * 
 * Functions can be called only for toggling access, that is
 * _revoke_ can be called only when access is granted.
 * 
 * PRE: partition should have non-zero space_id.
 */
void jet_uspace_revoke_access_local(struct _pok_partition* part);

/* Grant access to the local partition's memory, if it was revoked before. */
void jet_uspace_reset_access_local(struct _pok_partition* part);

#endif /* __JET_ASP_SPACE_H__ */
