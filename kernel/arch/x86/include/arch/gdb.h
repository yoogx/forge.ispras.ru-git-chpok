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

#ifndef __JET_X86_GDB_H__
#define __JET_X86_GDB_H__

#define NUMREGS 16
enum regnames {
EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI,
           pc /* also known as eip */,
           PS /* also known as eflags */,
           CS, SS, DS, ES, FS, GS
};

#define ja_GDB_INSTR_SIZE 1

/*
 * This provides definition of 'struct jet_interrupt_context' for
 * arch-independent code.
 * 
 * TODO: should be removed when no longer needed.
 */
#include <../arch/x86/interrupt.h>


#endif /* __JET_X86_GDB_H__ */
