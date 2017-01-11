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

#ifndef __JET_ASP_SPACE_H__
#define __JET_ASP_SPACE_H__

#include <types.h>
#include <asp/cswitch.h>
#include <common.h>
#include <errno.h>
#include <uapi/kernel_shared_data.h>

/* 
 * Arch header should define:
 * 
 * type 'jet_ustack_t' - pointer to allocated user stack.
 *   Value 0 should be assignable and comparable to given type, and
 *   never corresponds to allocated stack.
 */
#include <arch/space.h>

/* 
 * Identificator for the (memory) space.
 * 
 * Value 0 corresponds to kernel space.
 * Non-zero values correspond to user spaces.
 */
typedef uint8_t jet_space_id;

/*
 * Grant or revoke rw access from kernel space to memory for all
 * user spaces.
 * 
 * Initially global access is granted.
 * 
 * Implementation may assume that access is always toggled, that is
 * _grant_ is called only when access is not granted and
 * _revoke_ is called only when access is granted.
 * 
 * Called with global preemption disabled.
 */
void ja_uspace_grant_access(void);
void ja_uspace_revoke_access(void);

/* 
 * Grant or revoke rw access from kernel space to memory local to given
 * user space.
 * 
 * Initially local access is granted for all user spaces.
 * 
 * Implementation may assume that access is always toggled, that is
 * _grant_ is called only when access is not granted and
 * _revoke_ is called only when access is granted.
 * 
 * Called with global(!) preemption disabled.
 * 
 * PRE: space_id is not 0.
 * 
 * NOTE: Local access isn't affected by global switch; e.g.
 * ja_uspace_revoke_access() doesn't cancel effect of
 * ja_uspace_grant_access_local() for particular space.
 */
void ja_uspace_grant_access_local(jet_space_id space_id);
void ja_uspace_revoke_access_local(jet_space_id space_id);

/**
 * Jump to the user space.
 * 
 * Kernel stack passed as 'stack_kernel' will be used in interrupts/syscalls.
 */
void ja_user_space_jump(
    jet_stack_t stack_kernel,
    jet_space_id space_id, /* Actually, unused (should already be set with ja_space_switch). */
    void (__user * entry_user)(void),
    uintptr_t stack_user);

/**
 * Switch to given space.
 */
void   ja_space_switch (jet_space_id new_space_id);

/*
 * Return id of current space.
 */
jet_space_id ja_space_get_current (void);

/* Return alignment suitable for user space stack. */
size_t ja_ustack_get_alignment(void);

/*
 * Place for store floating point registers.
 * 
 * Kernel code doesn't use floating point operations,
 * so floating point registers needs to be stored only when switching
 * to other user space thread.
 */
struct jet_fp_store;

/* 
 * Allocate place for store floating point registers.
 * 
 * May be called only during OS init.
 */
struct jet_fp_store* ja_alloc_fp_store(void);

/* Save floating point registers into given place. */
void ja_fp_save(struct jet_fp_store* fp_store);

/* Restore floating point registers into given place. */
void ja_fp_restore(struct jet_fp_store* fp_store);

/* Initialize floating point registers with zero. */
void ja_fp_init(void);


#endif /* __JET_ASP_SPACE_H__ */
