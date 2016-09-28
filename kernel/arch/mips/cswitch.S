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
blr == jr $ra (= $31)

stw = sw
lwz = lw
*/
#include "asm_offsets_stack_frame.h"
#include "asm_offsets_context.h"

#define FRAME_SIZE SIZEOF_jet_context
        .text

/* Restore context from current stack. */
CONTEXT_RESTORE:
        lw      $30,OFFSETOF_jet_context_30($29)
        lw      $28,OFFSETOF_jet_context_28($29)
        lw      $27,OFFSETOF_jet_context_27($29)
        lw      $26,OFFSETOF_jet_context_26($29)
        lw      $25,OFFSETOF_jet_context_25($29)
        lw      $24,OFFSETOF_jet_context_24($29)
        lw      $23,OFFSETOF_jet_context_23($29)
        lw      $22,OFFSETOF_jet_context_22($29)
        lw      $21,OFFSETOF_jet_context_21($29)
        lw      $20,OFFSETOF_jet_context_20($29)
        lw      $19,OFFSETOF_jet_context_19($29)
        lw      $18,OFFSETOF_jet_context_18($29)
        lw      $17,OFFSETOF_jet_context_17($29)
        lw      $16,OFFSETOF_jet_context_16($29)
        lw      $15,OFFSETOF_jet_context_15($29)
        lw      $14,OFFSETOF_jet_context_14($29)
        lw      $13,OFFSETOF_jet_context_13($29)
        lw      $12,OFFSETOF_jet_context_12($29)
        lw      $11,OFFSETOF_jet_context_11($29)
        lw      $10,OFFSETOF_jet_context_10($29)
        lw      $9,OFFSETOF_jet_context_9($29)
        lw      $8,OFFSETOF_jet_context_8($29)
        lw      $1,OFFSETOF_jet_context_lo($29)
        mtlo    $1
        lw      $1,OFFSETOF_jet_context_hi($20)
        mthi    $1
        lw      $1,OFFSETOF_jet_context_FCCR($29)
        mtc1    $1,$24
        lw      $29,OFFSETOF_jet_stack_frame_back_chain($29)
        lw      $31,OFFSETOF_jet_stack_frame_lr($29)
        jr      $31

        .globl ja_context_switch
ja_context_switch:
        /* $4: *old_sp, $5: new_sp */
        /* Form stack frame */
        sw      $29,-FRAME_SIZE($29)
/*      mflr    $1*/
        sw      $31,FRAME_SIZE + OFFSETOF_jet_stack_frame_lr($29)
        /* Save registers.  */
        mfc1    $1, $24 /* FCCR */
        sw      $1,OFFSETOF_jet_context_FCCR($29)
        mfhi    $1                             
        sw      $1,OFFSETOF_jet_context_hi($20) 
        mflo    $1            
        sw      $1,OFFSETOF_jet_context_lo($20)
        sw      $8,OFFSETOF_jet_context_8($29)
        sw      $9,OFFSETOF_jet_context_9($29)
        sw      $10,OFFSETOF_jet_context_10($29)
        sw      $11,OFFSETOF_jet_context_11($29)
        sw      $12,OFFSETOF_jet_context_12($29)
        sw      $13,OFFSETOF_jet_context_13($29)
        sw      $14,OFFSETOF_jet_context_14($29)
        sw      $15,OFFSETOF_jet_context_15($29)
        sw      $16,OFFSETOF_jet_context_16($29)
        sw      $17,OFFSETOF_jet_context_17($29)
        sw      $18,OFFSETOF_jet_context_18($29)
        sw      $19,OFFSETOF_jet_context_19($29)
        sw      $20,OFFSETOF_jet_context_20($29)
        sw      $21,OFFSETOF_jet_context_21($29)
        sw      $22,OFFSETOF_jet_context_22($29)
        sw      $23,OFFSETOF_jet_context_23($29)
        sw      $24,OFFSETOF_jet_context_24($29)
        sw      $25,OFFSETOF_jet_context_25($29)
        sw      $26,OFFSETOF_jet_context_26($29)
        sw      $27,OFFSETOF_jet_context_27($29)
        sw      $28,OFFSETOF_jet_context_28($29)
        sw      $30,OFFSETOF_jet_context_30($29)
        /* Switch stacks. */
        sw      $29,0($4)
        move    $29, $5
        j CONTEXT_RESTORE
        .size ja_context_switch, . - ja_context_switch

        .globl ja_context_jump
ja_context_jump:
        /* $4: new_sp */
        move      $29, $4
        j CONTEXT_RESTORE
        .size ja_context_jump, . - ja_context_jump

        .globl ja_context_restart
ja_context_restart:
        /* $4: stack, $5: entry */
        /* Calculate in $6 relative stack offset for stwux: */
        add   $6, $4, $29
        addi  $6, $6, SIZEOF_jet_stack_frame_null
        li    $7, 0 /* 0 should be stored into first back chain*/
        /* stwux $7, $29, $6 */
        add   $29, $29, $6
        sw    $7, ($29)
        /*mtlr  $5*/
        move  $31, $5
        /* Nullify other registers (2-6, 8-29, 30). */
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
        
        
        jr $31
        .size ja_context_restart, . - ja_context_restart

        .globl ja_context_restart_and_save
ja_context_restart_and_save:
        /* $4: stack, $5: entry, $6 - address to save resulted context */
        addi  $7, $4, SIZEOF_jet_stack_frame_null
        sw    $7, 0($6)
        j ja_context_restart
        .size ja_context_restart_and_save, . - ja_context_restart_and_save
