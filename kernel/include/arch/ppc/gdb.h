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

#ifndef __POK_ARCH_GDB_H__

#define NUMREGS_FP 32
enum fp_regnames {
f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13,
f14, f15, f16, f17, f18, f19, f20, f21, f22, f23, f24, f25, f26, f27, f28, f29,
f30, f31    
};

#define NUMREGS 38
enum regnames {
r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13,
r14, r15, r16, r17, r18, r19, r20, r21, r22, r23, r24, r25, r26, r27, r28, r29,
r30, r31, pc, msr, cr, lr, ctr, xer 
};

/* Fill 'registers' array according to 'ea'. */
void gdb_set_regs(const struct regs* ea, uint32_t* registers);

/* Fill 'ea' array according to 'registers'. */
void gdb_get_regs(struct regs* ea, const uint32_t* registers);

#define __POK_ARCH_GDB_H__
#endif /* __POK_ARCH_GDB_H__ */