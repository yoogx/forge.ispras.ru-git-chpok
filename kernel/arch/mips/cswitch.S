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

/* FP_START must be equal to offsetof(context_t, fp0) */
#define FP_START 140
#define SAVE_FP(num) swc1 $f##num,(140+8*num)(sp)
#define LOAD_FP(num) lwc1 $f##num,(140+8*num)(sp)
/* FRAME_SIZE must be equal to offsetof(context_t, back_chain) */
#define sp      $29 
#define a0      $4      /* argument registers */
#define a1      $5
#define a2      $6
#define a3      $7
#define FRAME_SIZE 404 /* 352 + 52*/
        .text
        .globl ja_context_switch
ja_context_switch:
        /* r3: *old_sp, r4: new_sp */
        .set    noat
        /* Save registers.  */
        sw    sp, -FRAME_SIZE(sp)                  
        
        
        /* SAVE LR REGISTER!!! FIXIT*/
        sw      $31,FRAME_SIZE+4(sp)
        
        sw      $1, 12(sp)
        mfc1    $1, $24 /* FCCR */
        sw      $1, 8(sp)
        
        mfhi    $1                             
        sw      $1, 136(sp) 
        mflo    $1            
        sw      $1, 140(sp) 
        
        sw      $2, 16(sp) 
        sw      $3, 20(sp) 
        sw      $4, 24(sp) 
        sw      $5, 28(sp) 
        sw      $6, 32(sp) 
        sw      $7, 36(sp) 
        sw      $8, 40(sp) 
        sw      $9, 44(sp) 
        sw      $10, 48(sp)
        sw      $11, 52(sp)
        sw      $12, 56(sp)
        sw      $13, 60(sp)
        sw      $14, 64(sp)
        sw      $15, 68(sp)
        sw      $16, 72(sp)
        sw      $17, 76(sp)
        sw      $18, 80(sp)
        sw      $19, 84(sp)
        sw      $20, 88(sp)
        sw      $21, 92(sp)
        sw      $22, 96(sp)
        sw      $23, 100(sp)
        sw      $24, 104(sp)
        sw      $25, 108(sp)
        sw      $26, 112(sp)
        sw      $27, 116(sp)
        sw      $28, 120(sp)
        sw      $29, 124(sp)
        sw      $30, 128(sp)
        sw      $31, 132(sp)
        SAVE_FP (0)
        SAVE_FP (1)
        SAVE_FP (2)
        SAVE_FP (3)
        SAVE_FP (4)
        SAVE_FP (5)
        SAVE_FP (6)
        SAVE_FP (7)
        SAVE_FP (8)
        SAVE_FP (9)
        SAVE_FP (10)
        SAVE_FP (11)
        SAVE_FP (12)
        SAVE_FP (13)
        SAVE_FP (14)
        SAVE_FP (15)
        SAVE_FP (16)
        SAVE_FP (17)
        SAVE_FP (18)
        SAVE_FP (19)
        SAVE_FP (20)
        SAVE_FP (21)
        SAVE_FP (22)
        SAVE_FP (23)
        SAVE_FP (24)
        SAVE_FP (25)
        SAVE_FP (26)
        SAVE_FP (27)
        SAVE_FP (28)
        SAVE_FP (29)
        SAVE_FP (30)
        SAVE_FP (31)
        
        
        /* Switch stack. */
        sw      sp, 0(a0)
        move      sp, a1       
        
        
        /* Restore registers.  */
        LOAD_FP (31)
        LOAD_FP (30)
        LOAD_FP (29)
        LOAD_FP (28)
        LOAD_FP (27)
        LOAD_FP (26)
        LOAD_FP (25)
        LOAD_FP (24)
        LOAD_FP (23)
        LOAD_FP (22)
        LOAD_FP (21)
        LOAD_FP (20)
        LOAD_FP (19)
        LOAD_FP (18)
        LOAD_FP (17)
        LOAD_FP (16)
        LOAD_FP (15)
        LOAD_FP (14)
        LOAD_FP (13)
        LOAD_FP (12)
        LOAD_FP (11)
        LOAD_FP (10)
        LOAD_FP (9)
        LOAD_FP (8)
        LOAD_FP (7)
        LOAD_FP (6)
        LOAD_FP (5)
        LOAD_FP (4)
        LOAD_FP (3)
        LOAD_FP (2)
        LOAD_FP (1)         
        LOAD_FP (0)

        lw      $31, 132(sp)
        lw      $30, 128(sp)
        lw      $29, 124(sp)
        lw      $28, 120(sp)
        lw      $27, 116(sp)
        lw      $26, 112(sp)
        lw      $25, 108(sp)
        lw      $24, 104(sp)
        lw      $23, 100(sp)
        lw      $22, 96(sp)
        lw      $21, 92(sp)
        lw      $20, 88(sp)
        lw      $19, 84(sp)
        lw      $18, 80(sp)
        lw      $17, 76(sp)
        lw      $16, 72(sp)
        lw      $15, 68(sp)
        lw      $14, 64(sp)
        lw      $13, 60(sp)
        lw      $12, 56(sp)
        lw      $11, 52(sp)
        lw      $10, 48(sp)
        lw      $9, 44(sp) 
        lw      $8, 40(sp) 
        lw      $7, 36(sp) 
        lw      $6, 32(sp) 
        lw      $5, 28(sp) 
        lw      $4, 24(sp) 
        lw      $3, 20(sp) 
        lw      $2, 16(sp) 
        
        lw      $1, 140(sp) 
        mtlo    $1            
        lw      $1, 136(sp) 
        mthi    $1                             
        
        lw      $1, 8(sp)
        mtc1    $1, $24 /* FCCR */
        
        lw      $1, 12(sp)
        
        /* Load LR REGISTER!!! FIXIT*/
        lw      $31, 4(sp)
        
        .set    at
        /*maybe it can be better*/
        jr      $31
        
        .size ja_context_switch, . - ja_context_switch
