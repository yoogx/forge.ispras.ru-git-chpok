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

/* 
 * Context for store on interrupt.
 * 
 * This is '_noconfig' version of the header.
 * 
 * Do not use it directly, include interrupt_context.h instead.
 */

#ifndef __JET_MIPS_INTERRUPT_CONTEXT_NOCONFIG_H__
#define __JET_MIPS_INTERRUPT_CONTEXT_NOCONFIG_H__

#include <stdint.h>

#include "stack_frame.h"

/* 
 * Context stored on the stack for return back with pok_arch_rfi().
 * 
 * Normally, only volatile and special registers are stored here.
 * But for GDB we store all registers.
 * 
 * This is a complete first stack frame.
 */
uint32_t * expection_handlers[16];

struct jet_interrupt_context
{
  //~ struct jet_stack_frame stack_frame;
  uint32_t arg0;
  uint32_t arg1;
  uint32_t arg2;
  uint32_t arg3;
  uint32_t arg4;
  uint32_t arg5;
  uint32_t arg6;
  
  uint32_t r0;
  uint32_t r1;
  uint32_t r2;
  uint32_t r3;
  uint32_t r4;
  uint32_t r5;
  uint32_t r6;
  uint32_t r7;
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
  uint32_t r29;
  uint32_t r30;
  uint32_t r31;
  uint32_t lo;
  uint32_t hi;
  uint32_t EPC;
  uint32_t FCCR;
  uint32_t BadVAddr;
  uint32_t STATUS;
  uint32_t CAUSE;
#ifdef POK_NEEDS_GDB
  uint32_t FIR;
  uint32_t FCSR;
#endif /* POK_NEEDS_GDB */
  uint32_t a0;
  uint32_t a1;
  uint32_t a2;
  uint32_t a3;
} __attribute__((aligned(16)));

#endif /* __JET_MIPS_INTERRUPT_CONTEXT_NOCONFIG_H__ */
