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

/* Basic stack frame */

#ifndef __JET_MIPS_FRAME_H__
#define __JET_MIPS_FRAME_H__

#include <stdint.h>

/* 
 * Final part of any stack frame.
 * 
 * When describe concrete stack frame, include this struct at the 
 * beginning of actual struct and force resulted struct to be 16-bytes
 * aligned.
 */
struct jet_stack_frame
{
    uint32_t back_chain;
    uint32_t lr;
};

/* Null frame, usually the first one on the stack. */
struct jet_stack_frame_null
{
    struct jet_stack_frame stack_frame;
}__attribute__((aligned(16)));

/* Link stack frames at initialization. */
static inline void jet_stack_frame_link(
    struct jet_stack_frame* parent_frame,
    struct jet_stack_frame* child_frame,
    void (*entry)(void))
{
    parent_frame->lr = (uint32_t) entry;
    child_frame->back_chain = (uint32_t) parent_frame;
}

#endif /* __JET_MIPS_FRAME_H__ */
