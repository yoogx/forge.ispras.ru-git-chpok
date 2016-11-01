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

#ifndef __JET_MIPS_ASM_OFFSETS_INTERRUPT_CONTEXT_H__
#define __JET_MIPS_ASM_OFFSETS_INTERRUPT_CONTEXT_H__

#include <config.h>
#ifdef POK_NEEDS_GDB
#include "asm_offsets_interrupt_context_gdb_on.h"
#else /* POK_NEEDS_GDB */
#include "asm_offsets_interrupt_context_gdb_off.h"
#endif

#define JET_STACK_RED_ZONE 128

#endif /* __JET_MIPS_ASM_OFFSETS_CONTEXT_H__ */
