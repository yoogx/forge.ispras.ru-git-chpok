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

#ifndef __JET_ASP_GDB_H__
#define __JET_ASP_GDB_H__

/*
 * Arch header should define NUMREGS constant which contains number of
 * general-purpose registers accessible by gdb.
 * 
 * Arch header should define correspondence between names of these
 * general-purpose registers and their index in registers array.
 * (TODO: which order of array? TODO: why arch-specific names are needed for common code?).
 * 
 * Arch header should define NUMREGS_FP constant which contains number
 * of floating-point registers accessible by gdb. (TODO: Currently only ppc defines it).
 * 
 * Arch header should define correspondence between names of these
 * floating-point registers and their index in registers array.
 * (TODO: which order of array? TODO: why arch-specific names are needed for common code?).
 * 
 * If POK_NEEDS_GDB is enabled, arch header should expose definition(!)
 * of struct jet_interrupt_context. (TODO: Move usage of this definition into arch code).
 */
#include <arch/gdb.h>

struct jet_interrupt_context;

/* 
 * Context from the last interrupt. Used for extract registers for
 * interrupted thread.
 * 
 * Defined in arch-independent code. Should be assigned by arch code.
 * 
 * TODO: Naming is bad.
 */
extern struct jet_interrupt_context* global_thread_stack;

/* 
 * Fill 'registers' array according to 'ea'.
 * 
 * If 'ea' is NULL, interpret it as thread have not been started yet.
 */
void gdb_set_regs(const struct jet_interrupt_context* ea, uint32_t* registers);

/* Fill 'ea' array according to 'registers'. */
void gdb_get_regs(struct jet_interrupt_context* ea, const uint32_t* registers);

void pok_trap(void);

#endif /* __JET_ASP_GDB_H__ */
