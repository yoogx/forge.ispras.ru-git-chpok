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

#ifndef __JET_ASP_CSWITCH_H__
#define __JET_ASP_CSWITCH_H__

#include <asp/stack.h>

/* Opaque (kernel thread) context for switch to. */
struct jet_context;

/**
 * Initialize `context` on the given stack.
 * 
 * Return stack pointer, which can be used by pok_context_switch() to
 * jump into given entry with given stack.
 * 
 * Never returns NULL.
 */
struct jet_context* ja_context_init(jet_stack_t sp, void (*entry)(void));

/**
 * Switch to context, stored in @new_sp.
 * 
 * Pointer (non-NULL) to the current context will stored in @old_sp.
 */
void ja_context_switch (struct jet_context** old_sp, struct jet_context* new_sp);

/**
 * Jump to context, stored in @new_sp.
 * 
 * Current context will be lost.
 */
void ja_context_jump(struct jet_context* new_sp);

/**
 * Jump to given entry with given stack.
 * 
 * Mainly used for restart current context.
 */
void ja_context_restart(jet_stack_t sp, void (*entry)(void));

/**
 * Jump to given entry with given stack.
 * 
 * Mainly used for restart current context.
 * 
 * Value, pointed by `new_context_p` will be set to the stack pointer,
 * "as if" pok_context_switch() has been called.
 * 
 * This is useful for change special NULL value of context pointer to
 * something else.
 */
void ja_context_restart_and_save(jet_stack_t sp, void (*entry)(void),
        struct jet_context** new_context_p);


#endif /* __JET_ASP_CSWITCH_H__ */
