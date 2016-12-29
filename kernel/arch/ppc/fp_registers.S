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
#include "asm_offsets_fp_store.h"

#ifdef JET_PPC_CONFIG_SPE
/* We save and load only high 32bit of GPRs.
   Low part of some GPRs are are already saved, low part of
   others will be saved, and here we should not worry about it */
#define SAVE_FP(num) evstwwe %r##num,(OFFSETOF_jet_fp_store_fp_regs + 4*num)(%r3)
#define LOAD_FP(num) lwz %r4, (OFFSETOF_jet_fp_store_fp_regs + 4*num)(%r3); evmergelo %r##num, %r4, %r##num

 /* save/load SPEFSCR */
#define SAVE_FPSCR mfspefscr %r4; stw %r4,OFFSETOF_jet_fp_store_fp_fpscr(%r3)
#define LOAD_FPSCR lwz %r4, OFFSETOF_jet_fp_store_fp_fpscr(%r3); mtspefscr %r4;

/* copy rewrite high 32bit of r_num by highpart of r0 (which is zero) */
#define COPY_VAL_FROM_FP0_TO(num) evmergehilo %r##num , %r0, %r##num

#else

#define COPY_VAL_FROM_FP0_TO(num) fmr %f##num, %f0
#define SAVE_FP(num) stfd %f##num,(OFFSETOF_jet_fp_store_fp_regs + 8*num)(%r3)
#define LOAD_FP(num) lfd %f##num,(OFFSETOF_jet_fp_store_fp_regs + 8*num)(%r3)

#define SAVE_FPSCR mffs %f0; stfd %f0, OFFSETOF_jet_fp_store_fp_fpscr(%r3)
#define LOAD_FPSCR lfd %f0, OFFSETOF_jet_fp_store_fp_fpscr(%r3); mtfsf 0xff, %f0
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
    /* load zero to high 32bit of r0 */
    li %r4, 0x0 //zero low 32bit of r4
    evmergelo %r0, %r4, %r0 //zero high 32bit of r0

    /* zero SPEFSCR */
    mtspefscr %r4
#else
    /* load zero to %f0 */
    lis %r3, zero_fp@ha
    addi %r3, %r3, zero_fp@l
    lfd %f0, 0(%r3)

    /* zero FPSCR */
    lfd %f0, 0(%r3)
    mtfsf 0xff, %f0
#endif

    /* copy val from FP0 (r0 if SPE and f0 if not) to others */
    COPY_VAL_FROM_FP0_TO(1)
    COPY_VAL_FROM_FP0_TO(2)
    COPY_VAL_FROM_FP0_TO(3)
    COPY_VAL_FROM_FP0_TO(4)
    COPY_VAL_FROM_FP0_TO(5)
    COPY_VAL_FROM_FP0_TO(6)
    COPY_VAL_FROM_FP0_TO(7)
    COPY_VAL_FROM_FP0_TO(8)
    COPY_VAL_FROM_FP0_TO(9)
    COPY_VAL_FROM_FP0_TO(10)
    COPY_VAL_FROM_FP0_TO(11)
    COPY_VAL_FROM_FP0_TO(12)
    COPY_VAL_FROM_FP0_TO(13)
    COPY_VAL_FROM_FP0_TO(14)
    COPY_VAL_FROM_FP0_TO(15)
    COPY_VAL_FROM_FP0_TO(16)
    COPY_VAL_FROM_FP0_TO(17)
    COPY_VAL_FROM_FP0_TO(18)
    COPY_VAL_FROM_FP0_TO(19)
    COPY_VAL_FROM_FP0_TO(20)
    COPY_VAL_FROM_FP0_TO(21)
    COPY_VAL_FROM_FP0_TO(22)
    COPY_VAL_FROM_FP0_TO(23)
    COPY_VAL_FROM_FP0_TO(24)
    COPY_VAL_FROM_FP0_TO(25)
    COPY_VAL_FROM_FP0_TO(26)
    COPY_VAL_FROM_FP0_TO(27)
    COPY_VAL_FROM_FP0_TO(28)
    COPY_VAL_FROM_FP0_TO(29)
    COPY_VAL_FROM_FP0_TO(30)
    COPY_VAL_FROM_FP0_TO(31)


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

    /* save floating point status and control (FPSCR if SPE, SPEFSCR if not) register
     * NOTE: save_fpscr usese FP0, so we must save fp0 before
     */
    SAVE_FPSCR

    DISABLE_FP

    blr

    .size ja_fp_save, . - ja_fp_save

    .globl ja_fp_restore
ja_fp_restore:
    /* r3 - pointer to 'struct jet_fp_store'. */

    ENABLE_FP

    /* Load floating point status and control (FPSCR if SPE, SPEFSCR if not) register.
     * We must load FPSCR before FP0 because loading FPSCR uses FP0
     */
    LOAD_FPSCR

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


    DISABLE_FP

    blr

    .size ja_fp_restore, . - ja_fp_restore
