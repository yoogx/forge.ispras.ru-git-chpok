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

#if POK_NEEDS_GDB

#include <gdb.h>
/* Fill 'registers' array according to 'ea'. */
void gdb_set_regs(const struct jet_interrupt_context* ea, uint32_t* registers)
{
    if(ea == NULL)
    {
        memset(registers, 0, NUMREGS * sizeof(uint32_t));
        return;
    }

    registers[r0] = ea->r0;
    registers[r1] = ea->r1;
    registers[r2] = ea->r2;
    registers[r3] = ea->r3;
    registers[r4] = ea->r4;
    registers[r5] = ea->r5;
    registers[r6] = ea->r6;
    registers[r7] = ea->r7;
    registers[r8] = ea->r8;
    registers[r9] = ea->r9;
    registers[r10] = ea->r10;
    registers[r11] = ea->r11;
    registers[r12] = ea->r12;
    registers[r13] = ea->r13;
    registers[r14] = ea->r14;
    registers[r15] = ea->r15;
    registers[r16] = ea->r16;
    registers[r17] = ea->r17;
    registers[r18] = ea->r18;
    registers[r19] = ea->r19;
    registers[r20] = ea->r20;
    registers[r21] = ea->r21;
    registers[r22] = ea->r22;
    registers[r23] = ea->r23;
    registers[r24] = ea->r24;
    registers[r25] = ea->r25;
    registers[r26] = ea->r26;
    registers[r27] = ea->r27;
    registers[r28] = ea->r28;
    registers[r29] = ea->r29;
    registers[r30] = ea->r30;
    registers[r31] = ea->r31;
    registers[ctr] = ea->ctr;
    registers[xer] = ea->xer;
    registers[pc] = ea->srr0;
    registers[msr] = ea->srr1;
    registers[lr] = ea->lr;
    registers[cr] = ea->cr;
}

/* Fill 'ea' array according to 'registers'. */
void gdb_get_regs(struct jet_interrupt_context* ea, const uint32_t* registers)
{
    (void) ea;
    (void) registers;
    //TODO
}

#endif /* POK_NEEDS_GDB */
