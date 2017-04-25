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

#ifndef __JET_MIPS_CONTEXT_H__
#define __JET_MIPS_CONTEXT_H__

#include <stdint.h>

#include "stack_frame.h"

/* 
 * Context stored on the stack for return back via ja_context_switch().
 * 
 * Only non-volatile registers are stored here.
 */
struct jet_context
{
  uint32_t a0;
  uint32_t a1;
  uint32_t a2;
  uint32_t a3;
  uint32_t r1;
  uint32_t r2;
  uint32_t r3;
  uint32_t r6;
  uint32_t r7;
  
  uint32_t fccr;
  uint32_t hi;
  uint32_t lo;
  uint32_t r8;
  uint32_t r9;
  uint32_t r10;
  uint32_t r11;
  uint32_t r12;
  uint32_t r13;
  uint32_t r14;
  uint32_t r15;
  uint32_t r16;
  uint32_t r17;
  uint32_t r18;
  uint32_t r19;
  uint32_t r20;
  uint32_t r21;
  uint32_t r22;
  uint32_t r23;
  uint32_t r24;
  uint32_t r25;
  uint32_t r26;
  uint32_t r27;
  uint32_t r28;
  
  uint32_t r30;
  
  uint32_t ra;
  uint32_t sp;
  uint32_t aa0;
  uint32_t aa1;
  uint32_t aa2;
  uint32_t aa3;
} __attribute__((aligned(16)));

#endif /* __JET_MIPS_CONTEXT_H__ */
