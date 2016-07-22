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


#include <asp/cswitch.h>

#endif /* !__POK_ARCH_H__ */
