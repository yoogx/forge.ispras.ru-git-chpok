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

#include <gdb.h>
/* Fill 'registers' array according to 'ea'. */
void gdb_set_regs(const struct regs* ea, uint32_t* registers)
{
    registers[r0] = ea->r0;    /* zero */   /* Всегда содержит нуль, запись в регистр игнорируется */
    registers[r1] = ea->r1;    /* at */     /* Временные данные ассемблера (Assembler Temporaries) */
    registers[r2] = ea->r2;    /* v0 */     /* Регистр результата (Value Holder) */
    registers[r3] = ea->r3;    /* v1 */     /* Регистр результата (Value Holder) */
    registers[r4] = ea->r4;    /* a0 */     /* Регистр для передаваемого аргумента (Argument) */
    registers[r5] = ea->r5;    /* a1 */     /* Регистр для передаваемого аргумента (Argument) */
    registers[r6] = ea->r6;    /* a2 */     /* Регистр для передаваемого аргумента (Argument) */
    registers[r7] = ea->r7;    /* a3 */     /* Регистр для передаваемого аргумента (Argument) */
    registers[r8] = ea->r8;    /* t0 */     /* Временные данные подпрограммы (Temporary) */
    registers[r9] = ea->r9;    /* t1 */     /* Временные данные подпрограммы (Temporary) */
    registers[r10] = ea->r10;  /* t2 */     /* Временные данные подпрограммы (Temporary) */
    registers[r11] = ea->r11;  /* t3 */     /* Временные данные подпрограммы (Temporary) */
    registers[r12] = ea->r12;  /* t4 */     /* Временные данные подпрограммы (Temporary) */
    registers[r13] = ea->r13;  /* t5 */     /* Временные данные подпрограммы (Temporary) */
    registers[r14] = ea->r14;  /* t6 */     /* Временные данные подпрограммы (Temporary) */
    registers[r15] = ea->r15;  /* t7 */     /* Временные данные подпрограммы (Temporary) */
    registers[r16] = ea->r16;  /* s0 */     /* Регистр сохраняемый подпрограммой (Saved Register) */
    registers[r17] = ea->r17;  /* s1 */     /* Регистр сохраняемый подпрограммой (Saved Register) */
    registers[r18] = ea->r18;  /* s2 */     /* Регистр сохраняемый подпрограммой (Saved Register) */
    registers[r19] = ea->r19;  /* s3 */     /* Регистр сохраняемый подпрограммой (Saved Register) */
    registers[r20] = ea->r20;  /* s4 */     /* Регистр сохраняемый подпрограммой (Saved Register) */
    registers[r21] = ea->r21;  /* s5 */     /* Регистр сохраняемый подпрограммой (Saved Register) */
    registers[r22] = ea->r22;  /* s6 */     /* Регистр сохраняемый подпрограммой (Saved Register) */
    registers[r23] = ea->r23;  /* s7 */     /* Регистр сохраняемый подпрограммой (Saved Register) */
    registers[r24] = ea->r24;  /* t8 */     /* Временные данные подпрограммы (Temporary) */
    registers[r25] = ea->r25;  /* t9 */     /* Временные данные подпрограммы (Temporary) */
    registers[r26] = ea->r26;  /* k0 */     /* Регистр ядра ОС (Kernel Register) */
    registers[r27] = ea->r27;  /* k1 */     /* Регистр ядра ОС (Kernel Register) */
    registers[r28] = ea->r28;  /* gp */     /* Указатель на секцию глобальных переменных (Global Pointer) */
    registers[r29] = ea->r29;  /* sp */     /* Указатель на вершину стека (Stack Pointer) */
    registers[r30] = ea->r30;  /* s8,fp */  /* Указатель кадра (Frame Pointer) */
    registers[r31] = ea->r31;  /* ra */     /* Регистр адреса возврата из подпрограммы (Return Address) */

}

/* Fill 'ea' array according to 'registers'. */
void gdb_get_regs(struct regs* ea, const uint32_t* registers)
{
    //TODO
}

