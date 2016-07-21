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

/**
 * Return current partition id
 */
int ja_current_segment(void);

/**
 * Create TLB descriptor, which maps physical addresses in range
 * 
 * [addr;addr+size)
 * 
 * into user space.
 * 
 * Descriptor will be accessible via its identificator (@space_id).
 */
pok_ret_t   ja_space_create (uint8_t space_id, uintptr_t addr, size_t size);

/**
 * Return base virtual address for space mapping.
 * 
 * @addr is currently unused.
 * 
 */
uintptr_t	   ja_space_base_vaddr (uintptr_t addr);

/**
 * Jump to the user space.
 * 
 * Kernel stack passed as 'sp' will be used in interrupts/syscalls.
 */
void ja_user_space_jump(
        jet_stack_t sp,
        uint8_t space_id, /* Actually, unused (should already be set with pok_space_switch). */
        void (__user * entry_user)(void),
        uint32_t stack_user);

/**
 * Switch to given (user) space.
 * 
 * Space with index 0 is kernel space.
 */
pok_ret_t   ja_space_switch (uint8_t new_space_id);

uint8_t ja_space_get_current (void);

/**
 * Returns the stack address for the thread in a partition.
 *
 * @arg space_id indicates space for the partition that contains
 * the thread.
 * 
 * @arg stack_size indicates size of requested stack.
 *
 * @arg state should either
 * 
 *    - points to 0, which denotes first allocation request
 *                (all previous allocations are invalidated)
 *    - be value, passed to previous call to the function.
 *
 * On success function returns head to the stack and update value
 * pointed by @state.
 * 
 * On fail (e.g., insufficient space for requested stack) function
 * returns 0 and leave value pointed by @state unchanged.
 */
uint32_t    ja_thread_stack_addr   (uint8_t    space_id,
                                     uint32_t stack_size,
                                     uint32_t* state);

/*
 * Load given elf into given user space to given ARINC partition.
 * 
 * After the call @entry will be filled with address of start function.
 */
struct _pok_patition_arinc;
void ja_load_partition(struct _pok_patition_arinc* part,
        uint8_t elf_id,
        uint8_t space_id,
        uintptr_t *entry);

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
