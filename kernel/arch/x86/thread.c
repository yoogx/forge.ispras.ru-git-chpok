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

#include <asp/arch.h>

#include <libc.h>
#include <asp/cswitch.h>

#include "thread.h"

/* This function starts kernel thread and is defined in cswitch.S.*/
void ja_kernel_thread(void);

struct jet_context* ja_context_init (jet_stack_t sp, void (*entry)(void))
{
    struct jet_context* ctx = (struct jet_context*)(sp - sizeof(*ctx));

    memset (ctx, 0, sizeof (*ctx));
    /*
    * TODO: Current implementation of context switching requires
    * Interrupt Enabled flag to be specified when context for the switch
    * is created.
    * 
    * Currently we use hardcoded guess, that flag for the switched context
    * should be same as current flag.
    * 
    * In the future Interrupt Enabled flag should be same as one for
    * context we switched from.
    */
    ctx->eflags  = ja_preempt_enabled()? 1 << 9 : 0;
    // Registers below are filled in accordance to ja_kernel_thread().
    ctx->ebp = 0;
    ctx->ebx = (uint32_t)entry;
    ctx->ret = (uint32_t)ja_kernel_thread;

    return ctx;
}
