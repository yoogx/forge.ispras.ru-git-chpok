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
 *
 * This file also incorporates work covered by POK License.
 * Copyright (c) 2007-2009 POK team
 */

#ifndef __POK_ARCH_H__
#define __POK_ARCH_H__

#include <types.h>
#include <errno.h>
#include <bsp_common.h>

// TODO: Where should be that definition?
#define KERNEL_STACK_SIZE_DEFAULT 8192

/* 
 * TODO: Actually, this is architecture-dependent structure which
 * is used only in arch-specific code.
 * 
 * The only reason it is included here: `spaces` array should be
 * defined in the deployment.c (kernel).
 */
struct pok_space
{
    uintptr_t     phys_base;
    size_t        size;
};

extern struct pok_space spaces[];



/**
 * Function that initializes architecture concerns.
 */
pok_ret_t   pok_arch_init (void);

/**
 * Disable interruptions
 */
pok_ret_t   pok_arch_preempt_disable (void);

/**
 * Enable interruptions
 */
pok_ret_t   pok_arch_preempt_enable (void);

/**
 * Returns true if interrupts are enabled
 */
pok_bool_t pok_arch_preempt_enabled(void);

/**
 * Function that do nothing. Useful for the idle task for example.
 */
pok_ret_t   pok_arch_idle (void);

/**
 * Register an event (for example, an interruption)
 */
pok_ret_t   pok_arch_event_register (uint8_t vector, void (*handler)(void));

/**
 *  Disable interrupts and loop
 */
void pok_arch_inf_loop();

void pok_trap();

/*
 * reset cpu
 */
void pok_arch_cpu_reset();


void pok_arch_cpu_second_run(void (*entry)(void* private), void* private);

#include <asp/cswitch.h>

#define pok_context_init ja_context_init
#define pok_context_switch ja_context_switch

/**
 * DEV: It should be simple uint32_t eventually.
 * 
 * Some arch-dependent jump-like operations are implemented using
 * context switch, which requires 2 stacks. So, we prepare additional,
 * private stack for these operations.
 */
struct dStack
{
    uint32_t stacks[2];
    int index;
};

// => pok_stack_alloc
static inline void pok_dstack_alloc(struct dStack* d, uint32_t stack_size)
{
    d->stacks[0] = pok_stack_alloc(stack_size);
    d->stacks[1] = pok_stack_alloc(stack_size);
    d->index = 0;
}

/* Extract "normal" stack. */
static inline uint32_t pok_dstack_get_stack(struct dStack* d)
{
    return d->stacks[d->index];
}

/**
 * Jump to context, stored in @new_sp.
 * 
 * Current context will be lost.
 */
static inline void pok_context_jump(uint32_t new_sp)
{
    uint32_t fake_sp;
    pok_context_switch(&fake_sp, new_sp);
}

/**
 * Jump to given entry with given stack.
 * 
 * Mainly used for restart current context.
 * 
 * Value, pointed by `new_sp_p` will be set to the stack pointer,
 * "as if" pok_context_switch() has been called.
 * This is for prevent caller from forgetting to change value pointed
 * by `new_sp_p` from 0, which means "restart requires", to something else.
 * 
 */
static inline void pok_context_restart(struct dStack* d,
        void (*entry)(void),
        uint32_t* new_sp_p)
{
    int index = d->index;
    int index_other = index ^ 1;
    d->index = index_other;
    *new_sp_p = pok_context_init(d->stacks[index_other], entry);
    pok_context_jump(*new_sp_p);
}

#include <asp/space.h>

#define pok_create_space ja_space_create
#define pok_space_base_vaddr ja_space_base_vaddr
#define pok_space_context_init ja_space_context_init

#define pok_space_switch ja_space_switch
#define pok_space_get_current() ja_space_get_current();

#define pok_thread_stack_addr ja_thread_stack_addr

/**
 * Jump to the user space.
 * 
 * Current kernel stack will be used in interrupts/syscalls.
 */
static inline void pok_context_user_jump (
        struct dStack* d,
        uint8_t space_id, /* Actually, unused (should already be set with pok_space_switch). */
        uint32_t entry_rel,
        uint32_t stack_rel,
        uint32_t arg1,
        uint32_t arg2)
{
        int index_other = d->index ^ 1;
        d->index = index_other;
        
        uint32_t sp = pok_space_context_init(d->stacks[index_other],
                space_id,
                entry_rel,
                stack_rel,
                arg1,
                arg2);
                
        pok_context_jump(sp);
}

#define pok_arch_load_partition ja_load_partition

#include <asp/spinlock.h>

#endif /* !__POK_ARCH_H__ */
