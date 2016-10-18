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

#define SAVE_FP(num) stfd %f##num,(8*num)(%r3)
#define LOAD_FP(num) lfd %f##num,(8*num)(%r3)

.macro ENABLE_FP
    /* Enable floating point bit in msr. Note: uses %r4. */
    nop
.endm

.macro DISABLE_FP
    /* 
     * Disable floating point bit in msr.
     * 
     * NOTE: Assume fp being enabled previously.
     *
     * NOTE: uses %r4.
     */
    nop
.endm


zero_fp:
    .quad 0
    

    .text

    .globl ja_fp_init
ja_fp_init:
    ENABLE_FP
    

    DISABLE_FP

    eret

    .size ja_fp_init, . - ja_fp_init

    .globl ja_fp_save
ja_fp_save:
    /* r3 - pointer to 'struct jet_fp_store'. */

    ENABLE_FP
    
            
    DISABLE_FP
        
    eret

    .size ja_fp_save, . - ja_fp_save

    .globl ja_fp_restore
ja_fp_restore:
    /* r3 - pointer to 'struct jet_fp_store'. */

    ENABLE_FP
    
    
    //TODO: Restore FPSCR
        
    DISABLE_FP
        
    eret

    .size ja_fp_restore, . - ja_fp_restore
