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

#include <asp/stack.h>
#include <asp/cswitch.h>
#include <assert.h>

struct jet_context
{
    uint32_t user_sp;
    uint32_t user_lr;
    uint32_t r4;
    uint32_t r5;
    uint32_t r6;
    uint32_t r7;
    uint32_t r8;
    uint32_t r9;
    uint32_t r10;
    uint32_t r11;
    uint32_t r12;

    uint32_t entry;//lr
} __attribute__((aligned(8)));


struct jet_context* ja_context_init(jet_stack_t sp, void (*entry)(void))
{
    struct jet_context *ctx = (struct jet_context *)(sp - sizeof(*ctx));
    memset(ctx, 0, sizeof(*ctx));
    ctx->entry = entry;
}

void ja_context_restart_and_save(jet_stack_t sp, void (*entry)(void),
        struct jet_context** new_context_p)
{
    *new_context_p = 1; //something not NULL
    ja_context_restart(sp, entry);
}
