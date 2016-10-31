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

#ifndef __JA_X86_INTERRUPT_FRAME
#define __JA_X86_INTERRUPT_FRAME

#include <types.h>

typedef struct jet_interrupt_context
{
  uint32_t es;
  uint32_t ds;
  // These registers are ordered for pusha/popa
  uint32_t edi;
  uint32_t esi;
  uint32_t ebp;
  uint32_t __esp;
  uint32_t ebx;
  uint32_t edx;
  uint32_t ecx;
  uint32_t eax;

  /* These are pushed by interrupt */
  uint32_t error;	/* Error code or padding */
  uint32_t eip;
  uint32_t cs;
  uint32_t eflags;

  /* Only pushed with privilege switch */
  /* (Check cs content to have original CPL) */
  uint32_t esp;
  uint32_t ss;
} interrupt_frame;

#endif /* __JA_X86_INTERRUPT_FRAME */
