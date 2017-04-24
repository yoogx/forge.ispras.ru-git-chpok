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
 *
 * This file also incorporates work covered by POK License.
 * Copyright (c) 2007-2009 POK team
 */

#include "cp0.h"
#include "asm_offsets_fp_store.h" 

#define SAVE_two_FP(num0, num1) sdc1 $f##num0,(OFFSETOF_jet_fp_store_big_fp_regs + 8*num0)($a0); \
                     mfc1 $k0, $f##num1; sw $k0,(OFFSETOF_jet_fp_store_small_fp_regs + 4*num1)($a0)
#define LOAD_two_FP(num0, num1) ldc1 $f##num0,(OFFSETOF_jet_fp_store_big_fp_regs + 8*num0)($a0); \
                     mtc1 $k0, $f##num1; lw $k0,(OFFSETOF_jet_fp_store_small_fp_regs + 4*num1)($a0)

#define SAVE_FIR     mfc1 $k0, CP1_FIR;     sw  $k0, OFFSETOF_jet_fp_store_fp_fir($a0);
#define SAVE_FCONFIG mfc1 $k0, CP1_FCONFIG; sw  $k0, OFFSETOF_jet_fp_store_fp_fconfig($a0);
#define SAVE_FCSR    mfc1 $k0, CP1_FCSR;    sw  $k0, OFFSETOF_jet_fp_store_fp_fcsr($a0);
#define LOAD_FIR     lw  $k0, OFFSETOF_jet_fp_store_fp_fir($a0);        MTC1($k0 , CP1_FIR);
#define LOAD_FCONFIG lw  $k0, OFFSETOF_jet_fp_store_fp_fconfig($a0);    MTC1($k0 , CP1_FCONFIG);
#define LOAD_FCSR    lw  $k0, OFFSETOF_jet_fp_store_fp_fcsr($a0);       MTC1($k0 , CP1_FCSR);
.macro ENABLE_FP
    /* Enable floating point bit in msr. Note: uses %r4. */
    mfc0    $k0, CP0_STATUS
    li      $k1, CP0_STATUS_CU1
    or      $k0, $k0, $k1
    MTC0($k0, CP0_STATUS)
    
.endm

.macro DISABLE_FP
    /* 
     * Disable floating point bit in msr.
     * 
     * NOTE: Assume fp being enabled previously.
     */
    mfc0    $k0, CP0_STATUS
    li      $k1, CP0_STATUS_CU1
    or      $k0, $k0, $k1
    xor     $k0, $k0, $k1
    MTC0($k0, CP0_STATUS)
.endm
 
 
    //.rodata
    .align 4 # 4 words = 16 bytes

zero_fp:
    .quad 0
    

    .text


.set oddspreg
    
    .globl ja_fp_init
ja_fp_init:
    ENABLE_FP
    # clear all registers    
    ldc1  $f0, zero_fp
    mtc1  $zero, $f1
    ldc1  $f2, zero_fp
    mtc1  $zero, $f3
    ldc1  $f4, zero_fp
    mtc1  $zero, $f5
    ldc1  $f6, zero_fp
    mtc1  $zero, $f7
    ldc1  $f8, zero_fp
    mtc1  $zero, $f9
    ldc1  $f10, zero_fp
    mtc1  $zero, $f11
    ldc1  $f12, zero_fp
    mtc1  $zero, $f13
    ldc1  $f14, zero_fp
    mtc1  $zero, $f15
    ldc1  $f16, zero_fp
    mtc1  $zero, $f17
    ldc1  $f18, zero_fp
    mtc1  $zero, $f19
    ldc1  $f20, zero_fp
    mtc1  $zero, $f21
    ldc1  $f22, zero_fp
    mtc1  $zero, $f23
    ldc1  $f24, zero_fp
    mtc1  $zero, $f25
    ldc1  $f26, zero_fp
    mtc1  $zero, $f27
    ldc1  $f28, zero_fp
    mtc1  $zero, $f29
    ldc1  $f30, zero_fp
    mtc1  $zero, $f31

    DISABLE_FP

    JUMP_TO_REG($ra)

    .size ja_fp_init, . - ja_fp_init

    .globl ja_fp_save
ja_fp_save:
    /* $a0 - pointer to 'struct jet_fp_store'. */

    ENABLE_FP
    

    SAVE_two_FP(0, 1)
    SAVE_two_FP(2, 3)
    SAVE_two_FP(4, 5)
    SAVE_two_FP(6, 7)
    SAVE_two_FP(8, 9)
    SAVE_two_FP(10, 11)
    SAVE_two_FP(12, 13)
    SAVE_two_FP(14, 15)
    SAVE_two_FP(16, 17)
    SAVE_two_FP(18, 19)
    SAVE_two_FP(20, 21)
    SAVE_two_FP(22, 23)
    SAVE_two_FP(24, 25)
    SAVE_two_FP(26, 27)
    SAVE_two_FP(28, 29)
    SAVE_two_FP(30, 31)
   
    SAVE_FIR
    SAVE_FCONFIG
    SAVE_FCSR
            
    DISABLE_FP
        
    JUMP_TO_REG($ra)

    .size ja_fp_save, . - ja_fp_save

    .globl ja_fp_restore
ja_fp_restore:
    /* $a0 - pointer to 'struct jet_fp_store'. */

    ENABLE_FP

    LOAD_FIR
    LOAD_FCONFIG
    LOAD_FCSR
    
    LOAD_two_FP(0, 1)
    LOAD_two_FP(2, 3)
    LOAD_two_FP(4, 5)
    LOAD_two_FP(6, 7)
    LOAD_two_FP(8, 9)
    LOAD_two_FP(10, 11)
    LOAD_two_FP(12, 13)
    LOAD_two_FP(14, 15)
    LOAD_two_FP(16, 17)
    LOAD_two_FP(18, 19)
    LOAD_two_FP(20, 21)
    LOAD_two_FP(22, 23)
    LOAD_two_FP(24, 25)
    LOAD_two_FP(26, 27)
    LOAD_two_FP(28, 29)
    LOAD_two_FP(30, 31)
        
    DISABLE_FP
        
    JUMP_TO_REG($ra)

    .size ja_fp_restore, . - ja_fp_restore
