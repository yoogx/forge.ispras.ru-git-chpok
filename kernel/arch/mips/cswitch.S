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
/*
 * blr == JUMP_TO_REG($ra) (= $31)
 * 
 * stw = sw
 * lwz = lw
 */
#include "asm_offsets_stack_frame.h"
#include "asm_offsets_context.h"
#include "msr.h"

#define FRAME_SIZE SIZEOF_jet_context
        .text

/* Restore context from current stack. */
CONTEXT_RESTORE:
        .set    noat 
        lw     $30, OFFSETOF_jet_context_r30($sp)
        lw     $28, OFFSETOF_jet_context_r28($sp)
        lw     $27, OFFSETOF_jet_context_r27($sp)
        lw     $26, OFFSETOF_jet_context_r26($sp)
        lw     $25, OFFSETOF_jet_context_r25($sp)
        lw     $24, OFFSETOF_jet_context_r24($sp)
        lw     $23, OFFSETOF_jet_context_r23($sp)
        lw     $22, OFFSETOF_jet_context_r22($sp)
        lw     $21, OFFSETOF_jet_context_r21($sp)
        lw     $20, OFFSETOF_jet_context_r20($sp)
        lw     $19, OFFSETOF_jet_context_r19($sp)
        lw     $18, OFFSETOF_jet_context_r18($sp)
        lw     $17, OFFSETOF_jet_context_r17($sp)
        lw     $16, OFFSETOF_jet_context_r16($sp)
        lw     $15, OFFSETOF_jet_context_r15($sp)
        lw     $14, OFFSETOF_jet_context_r14($sp)
        lw     $13, OFFSETOF_jet_context_r13($sp)
        lw     $12, OFFSETOF_jet_context_r12($sp)
        lw     $11, OFFSETOF_jet_context_r11($sp)
        lw     $10, OFFSETOF_jet_context_r10($sp)
        lw     $9,  OFFSETOF_jet_context_r9($sp)
        lw     $8,  OFFSETOF_jet_context_r8($sp)
        lw     $1,  OFFSETOF_jet_context_lo($sp)
        MTlo($1)
        lw     $1,  OFFSETOF_jet_context_hi($sp)
        MThi($1)
        lw     $1,  OFFSETOF_jet_context_FCCR($sp)
        MTC1($1,  CP1_FCCR)
        lw     $sp, OFFSETOF_jet_stack_frame_back_chain($sp)
        lw     $ra, OFFSETOF_jet_stack_frame_lr($sp)
        JUMP_TO_REG($ra)

        .globl ja_context_switch
ja_context_switch:
        /* $4: *old_sp, $5: new_sp */
        /* Form stack frame */
        sw     $sp, -FRAME_SIZE($sp)
        /*mflr   $1*/
        sw     $ra,FRAME_SIZE + OFFSETOF_jet_stack_frame_lr($sp)
        /* Save registers.  */
        mfc1   $1,  CP1_FCCR
        sw     $1,  OFFSETOF_jet_context_FCCR($sp)
        mfhi   $1                               
        sw     $1,  OFFSETOF_jet_context_hi($sp) 
        mflo   $1            
        sw     $1,  OFFSETOF_jet_context_lo($sp)
        sw     $8,  OFFSETOF_jet_context_r8($sp)
        sw     $9,  OFFSETOF_jet_context_r9($sp)
        sw     $10, OFFSETOF_jet_context_r10($sp)
        sw     $11, OFFSETOF_jet_context_r11($sp)
        sw     $12, OFFSETOF_jet_context_r12($sp)
        sw     $13, OFFSETOF_jet_context_r13($sp)
        sw     $14, OFFSETOF_jet_context_r14($sp)
        sw     $15, OFFSETOF_jet_context_r15($sp)
        sw     $16, OFFSETOF_jet_context_r16($sp)
        sw     $17, OFFSETOF_jet_context_r17($sp)
        sw     $18, OFFSETOF_jet_context_r18($sp)
        sw     $19, OFFSETOF_jet_context_r19($sp)
        sw     $20, OFFSETOF_jet_context_r20($sp)
        sw     $21, OFFSETOF_jet_context_r21($sp)
        sw     $22, OFFSETOF_jet_context_r22($sp)
        sw     $23, OFFSETOF_jet_context_r23($sp)
        sw     $24, OFFSETOF_jet_context_r24($sp)
        sw     $25, OFFSETOF_jet_context_r25($sp)
        sw     $26, OFFSETOF_jet_context_r26($sp)
        sw     $27, OFFSETOF_jet_context_r27($sp)
        sw     $28, OFFSETOF_jet_context_r28($sp)
        sw     $30, OFFSETOF_jet_context_r30($sp)
        /* Switch stacks. */
        sw     $sp, 0($4)
        move   $sp, $5
        JUMP_TO_ADDR(CONTEXT_RESTORE)
        .size ja_context_switch, . - ja_context_switch

        .globl ja_context_jump
ja_context_jump:
        /* $4: new_sp */
        move  $sp, $4
        JUMP_TO_ADDR(CONTEXT_RESTORE)
        .size ja_context_jump, . - ja_context_jump

        .globl ja_context_restart
ja_context_restart:
        /* $4: stack, $5: entry */
        /* Calculate in $6 relative stack offset for stwux: */
        add   $6, $4, $sp
        addi  $6, $6, -SIZEOF_jet_stack_frame_null
        li    $7, 0 /* 0 should be stored into first back chain*/
        /* stwux $7, $sp, $6 */
        add   $sp, $sp, $6
        sw    $7, ($sp)
        /*mtlr  $5*/
        move  $ra, $5
        /* Nullify other registers (2-6, 8-28, 30). */
        move  $2, $7
        move  $3, $7
        move  $4, $7
        move  $5, $7
        move  $6, $7
        move  $8, $7
        move  $9, $7
        move  $10, $7
        move  $11, $7
        move  $12, $7
        move  $13, $7
        move  $14, $7
        move  $15, $7
        move  $16, $7
        move  $17, $7
        move  $18, $7
        move  $19, $7
        move  $20, $7
        move  $21, $7
        move  $22, $7
        move  $23, $7
        move  $24, $7
        move  $25, $7
        move  $26, $7
        move  $27, $7
        move  $28, $7
        move  $30, $7
        
        
        JUMP_TO_REG($ra)
        .size ja_context_restart, . - ja_context_restart

        .globl ja_context_restart_and_save
ja_context_restart_and_save:
        /* $4: stack, $5: entry, $6 - address to save resulted context */
        addi  $7,  $4, SIZEOF_jet_stack_frame_null
        sw    $7,  0($6)
        JUMP_TO_ADDR(ja_context_restart)
        .size ja_context_restart_and_save, . - ja_context_restart_and_save
