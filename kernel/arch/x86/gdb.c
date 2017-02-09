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

#include <config.h>

#ifdef POK_NEEDS_GDB

#include <asp/gdb.h>
/* Fill 'registers' array according to 'ea'. */
void gdb_set_regs(const struct jet_interrupt_context* ea, uint32_t* registers)
{
    registers[EAX] = ea->eax;
    registers[ECX] = ea->ecx;
    registers[EDX] = ea->edx;
    registers[EBX] = ea->ebx;
    registers[ESP] = ea->__esp;
    registers[EBP] = ea->ebp;
    registers[ESI] = ea->esi;
    registers[EDI] = ea->edi;
    registers[PC] = ea->eip;
    registers[PS] = ea->eflags;
    registers[CS] = ea->cs;
    registers[SS] = ea->ss;
    registers[DS] = ea->ds;
    registers[ES] = ea->es;
    registers[FS] = -1;
    registers[GS] = -1;
}

/* Fill 'ea' array according to 'registers'. */
void gdb_get_regs(struct jet_interrupt_context* ea, const uint32_t* registers)
{
    //TODO
}

#endif /* POK_NEEDS_GDB */
