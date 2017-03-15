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

#include "msr.h"
#include "asm_offsets_fp_store.h" 

#define SAVE_FP(num) sdc1 $f##num,(OFFSETOF_jet_fp_store_fp_regs + 8*num)($a0)
#define LOAD_FP(num) ldc1 $f##num,(OFFSETOF_jet_fp_store_fp_regs + 8*num)($a0)
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

    .globl ja_fp_init
ja_fp_init:
    ENABLE_FP
    
    # clear all registers    
    mtc1  $zero, $f0
    mtc1  $zero, $f2
    mtc1  $zero, $f4
    mtc1  $zero, $f6
    mtc1  $zero, $f8
    mtc1  $zero, $f10
    mtc1  $zero, $f12
    mtc1  $zero, $f14
    mtc1  $zero, $f16
    mtc1  $zero, $f18
    mtc1  $zero, $f20
    mtc1  $zero, $f22
    mtc1  $zero, $f24
    mtc1  $zero, $f26
    mtc1  $zero, $f28
    mtc1  $zero, $f30

    DISABLE_FP

    JUMP_TO_REG($ra)

    .size ja_fp_init, . - ja_fp_init

    .globl ja_fp_save
ja_fp_save:
    /* $a0 - pointer to 'struct jet_fp_store'. */

    ENABLE_FP
    

    SAVE_FP(0)
    SAVE_FP(2)
    SAVE_FP(4)
    SAVE_FP(6)
    SAVE_FP(8)
    SAVE_FP(10)
    SAVE_FP(12)
    SAVE_FP(14)
    SAVE_FP(16)
    SAVE_FP(18)
    SAVE_FP(20)
    SAVE_FP(22)
    SAVE_FP(24)
    SAVE_FP(26)
    SAVE_FP(28)
    SAVE_FP(30)
   
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
    
    LOAD_FP(0)
    LOAD_FP(2)
    LOAD_FP(4)
    LOAD_FP(6)
    LOAD_FP(8)
    LOAD_FP(10)
    LOAD_FP(12)
    LOAD_FP(14)
    LOAD_FP(16)
    LOAD_FP(18)
    LOAD_FP(20)
    LOAD_FP(22)
    LOAD_FP(24)
    LOAD_FP(26)
    LOAD_FP(28)
    LOAD_FP(30)
        
    DISABLE_FP
        
    JUMP_TO_REG($ra)

    .size ja_fp_restore, . - ja_fp_restore
