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

#include "msr.h"

#ifdef JET_PPC_CONFIG_SPE
#define SAVE_FP(num)
#define LOAD_FP(num)
#else
#define SAVE_FP(num) stfd %f##num,(8*num)(%r3)
#define LOAD_FP(num) lfd %f##num,(8*num)(%r3)
#endif

.macro ENABLE_FP
    /* Enable floating point bit in msr.
     *
     * NOTE: uses %r4.
     */
    mfmsr %r4
    oris %r4,%r4, (MSR_ENABLE_FP)@h
    ori %r4,%r4, (MSR_ENABLE_FP)@l
    mtmsr %r4
.endm

.macro DISABLE_FP
    /*
     * Disable floating point bit in msr.
     *
     * NOTE: Assume fp being enabled previously.
     *
     * NOTE: uses %r4.
     */
    mfmsr %r4
    subis %r4, %r4, (MSR_ENABLE_FP)@h
    subi %r4, %r4, (MSR_ENABLE_FP)@l
    mtmsr %r4
.endm

    .rodata
    .align 4 # 4 words = 16 bytes
zero_fp:
    .quad 0


    .text

    .globl ja_fp_init
ja_fp_init:

    ENABLE_FP

#ifdef JET_PPC_CONFIG_SPE

    li %r4, 0 //zero low 32bit of r4

    /* 'evmergelo X,A,X' makes X=A|X
     * i.e. rewrite high 32 bit of X by low 32 bit of A.
     * So in next instruction we zero high32bit of all GPRs
     */
    evmergelo %r0 , %r4, %r0
    evmergelo %r1 , %r4, %r1
    evmergelo %r2 , %r4, %r2
    evmergelo %r3 , %r4, %r3
    evmergelo %r4 , %r4, %r4
    evmergelo %r5 , %r4, %r5
    evmergelo %r6 , %r4, %r6
    evmergelo %r7 , %r4, %r7
    evmergelo %r8 , %r4, %r8
    evmergelo %r9 , %r4, %r9
    evmergelo %r10, %r4, %r10
    evmergelo %r11, %r4, %r11
    evmergelo %r12, %r4, %r12
    evmergelo %r13, %r4, %r13
    evmergelo %r14, %r4, %r14
    evmergelo %r15, %r4, %r15
    evmergelo %r16, %r4, %r16
    evmergelo %r17, %r4, %r17
    evmergelo %r18, %r4, %r18
    evmergelo %r19, %r4, %r19
    evmergelo %r20, %r4, %r20
    evmergelo %r21, %r4, %r21
    evmergelo %r22, %r4, %r22
    evmergelo %r23, %r4, %r23
    evmergelo %r24, %r4, %r24
    evmergelo %r25, %r4, %r25
    evmergelo %r26, %r4, %r26
    evmergelo %r27, %r4, %r27
    evmergelo %r28, %r4, %r28
    evmergelo %r29, %r4, %r29
    evmergelo %r30, %r4, %r30
    evmergelo %r31, %r4, %r31

#else
    lis %r3, zero_fp@ha
    addi %r3, %r3, zero_fp@l

    lfd %f0, 0(%r3)

    # Copy other registers from f0
    fmr %f1, %f0
    fmr %f2, %f0
    fmr %f3, %f0
    fmr %f4, %f0
    fmr %f5, %f0
    fmr %f6, %f0
    fmr %f7, %f0
    fmr %f8, %f0
    fmr %f9, %f0
    fmr %f10, %f0
    fmr %f11, %f0
    fmr %f12, %f0
    fmr %f13, %f0
    fmr %f14, %f0
    fmr %f15, %f0
    fmr %f16, %f0
    fmr %f17, %f0
    fmr %f18, %f0
    fmr %f19, %f0
    fmr %f20, %f0
    fmr %f21, %f0
    fmr %f22, %f0
    fmr %f23, %f0
    fmr %f24, %f0
    fmr %f25, %f0
    fmr %f26, %f0
    fmr %f27, %f0
    fmr %f28, %f0
    fmr %f29, %f0
    fmr %f30, %f0
    fmr %f31, %f0
#endif
    DISABLE_FP

    blr

    .size ja_fp_init, . - ja_fp_init

    .globl ja_fp_save
ja_fp_save:
    /* r3 - pointer to 'struct jet_fp_store'. */

    ENABLE_FP

    SAVE_FP(0)
    SAVE_FP(1)
    SAVE_FP(2)
    SAVE_FP(3)
    SAVE_FP(4)
    SAVE_FP(5)
    SAVE_FP(6)
    SAVE_FP(7)
    SAVE_FP(8)
    SAVE_FP(9)
    SAVE_FP(10)
    SAVE_FP(11)
    SAVE_FP(12)
    SAVE_FP(13)
    SAVE_FP(14)
    SAVE_FP(15)
    SAVE_FP(16)
    SAVE_FP(17)
    SAVE_FP(18)
    SAVE_FP(19)
    SAVE_FP(20)
    SAVE_FP(21)
    SAVE_FP(22)
    SAVE_FP(23)
    SAVE_FP(24)
    SAVE_FP(25)
    SAVE_FP(26)
    SAVE_FP(27)
    SAVE_FP(28)
    SAVE_FP(29)
    SAVE_FP(30)
    SAVE_FP(31)

    //TODO: Store FPSCR

    DISABLE_FP

    blr

    .size ja_fp_save, . - ja_fp_save

    .globl ja_fp_restore
ja_fp_restore:
    /* r3 - pointer to 'struct jet_fp_store'. */

    ENABLE_FP

    LOAD_FP(0)
    LOAD_FP(1)
    LOAD_FP(2)
    LOAD_FP(3)
    LOAD_FP(4)
    LOAD_FP(5)
    LOAD_FP(6)
    LOAD_FP(7)
    LOAD_FP(8)
    LOAD_FP(9)
    LOAD_FP(10)
    LOAD_FP(11)
    LOAD_FP(12)
    LOAD_FP(13)
    LOAD_FP(14)
    LOAD_FP(15)
    LOAD_FP(16)
    LOAD_FP(17)
    LOAD_FP(18)
    LOAD_FP(19)
    LOAD_FP(20)
    LOAD_FP(21)
    LOAD_FP(22)
    LOAD_FP(23)
    LOAD_FP(24)
    LOAD_FP(25)
    LOAD_FP(26)
    LOAD_FP(27)
    LOAD_FP(28)
    LOAD_FP(29)
    LOAD_FP(30)
    LOAD_FP(31)

    //TODO: Restore FPSCR

    DISABLE_FP

    blr

    .size ja_fp_restore, . - ja_fp_restore
