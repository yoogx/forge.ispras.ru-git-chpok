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

#include "asm_offsets_stack_frame.h"
#include "asm_offsets_context.h"

#define FRAME_SIZE SIZEOF_jet_context
        .text

/* Restore context from current stack. */
CONTEXT_RESTORE:
        lwz     %r31,OFFSETOF_jet_context_r31(%r1)
        lwz     %r30,OFFSETOF_jet_context_r30(%r1)
        lwz     %r29,OFFSETOF_jet_context_r29(%r1)
        lwz     %r28,OFFSETOF_jet_context_r28(%r1)
        lwz     %r27,OFFSETOF_jet_context_r27(%r1)
        lwz     %r26,OFFSETOF_jet_context_r26(%r1)
        lwz     %r25,OFFSETOF_jet_context_r25(%r1)
        lwz     %r24,OFFSETOF_jet_context_r24(%r1)
        lwz     %r23,OFFSETOF_jet_context_r23(%r1)
        lwz     %r22,OFFSETOF_jet_context_r22(%r1)
        lwz     %r21,OFFSETOF_jet_context_r21(%r1)
        lwz     %r20,OFFSETOF_jet_context_r20(%r1)
        lwz     %r19,OFFSETOF_jet_context_r19(%r1)
        lwz     %r18,OFFSETOF_jet_context_r18(%r1)
        lwz     %r17,OFFSETOF_jet_context_r17(%r1)
        lwz     %r16,OFFSETOF_jet_context_r16(%r1)
        lwz     %r15,OFFSETOF_jet_context_r15(%r1)
        lwz     %r14,OFFSETOF_jet_context_r14(%r1)
        lwz     %r13,OFFSETOF_jet_context_r13(%r1)
        lwz     %r2,OFFSETOF_jet_context_r2(%r1)
        lwz     %r0,OFFSETOF_jet_context_cr(%r1)
        mtcr    %r0
        lwz     %r1,OFFSETOF_jet_stack_frame_back_chain(%r1)
        lwz     %r0,OFFSETOF_jet_stack_frame_lr(%r1)
        mtlr    %r0
        blr

        .globl ja_context_switch
ja_context_switch:
        /* r3: *old_sp, r4: new_sp */

        /* Form stack frame */
        stwu    %r1,-FRAME_SIZE(%r1)
        mflr    %r0
        stw     %r0,FRAME_SIZE + OFFSETOF_jet_stack_frame_lr(%r1)
        /* Save registers.  */
        mfcr    %r0
        stw     %r0,OFFSETOF_jet_context_cr(%r1)
        stw     %r2,OFFSETOF_jet_context_r2(%r1)
        stw     %r13,OFFSETOF_jet_context_r13(%r1)
        stw     %r14,OFFSETOF_jet_context_r14(%r1)
        stw     %r15,OFFSETOF_jet_context_r15(%r1)
        stw     %r16,OFFSETOF_jet_context_r16(%r1)
        stw     %r17,OFFSETOF_jet_context_r17(%r1)
        stw     %r18,OFFSETOF_jet_context_r18(%r1)
        stw     %r19,OFFSETOF_jet_context_r19(%r1)
        stw     %r20,OFFSETOF_jet_context_r20(%r1)
        stw     %r21,OFFSETOF_jet_context_r21(%r1)
        stw     %r22,OFFSETOF_jet_context_r22(%r1)
        stw     %r23,OFFSETOF_jet_context_r23(%r1)
        stw     %r24,OFFSETOF_jet_context_r24(%r1)
        stw     %r25,OFFSETOF_jet_context_r25(%r1)
        stw     %r26,OFFSETOF_jet_context_r26(%r1)
        stw     %r27,OFFSETOF_jet_context_r27(%r1)
        stw     %r28,OFFSETOF_jet_context_r28(%r1)
        stw     %r29,OFFSETOF_jet_context_r29(%r1)
        stw     %r30,OFFSETOF_jet_context_r30(%r1)
        stw     %r31,OFFSETOF_jet_context_r31(%r1)

        /* Switch stacks. */
        stw     %r1,0(%r3)
        mr      %r1, %r4

        b CONTEXT_RESTORE
        .size ja_context_switch, . - ja_context_switch

        .globl ja_context_jump
ja_context_jump:
        /* r3: new_sp */
        mr      %r1, %r3
        b CONTEXT_RESTORE
        .size ja_context_jump, . - ja_context_jump

        .globl ja_context_restart
ja_context_restart:
        /* r3: stack, r4: entry */
        /* Calculate in r5 relative stack offset for stwux: */
        sub %r5, %r3, %r1
        subi %r5, %r5, SIZEOF_jet_stack_frame_null
        li %r6, 0 /* 0 should be stored into first back chain*/
        stwux %r6, %r1, %r5
        mtlr %r4
        /* Nullify other registers (0, 2-5, 7-31). */
        mr %r0, %r6
        mr %r2, %r6
        mr %r3, %r6
        mr %r4, %r6
        mr %r5, %r6
        mr %r7, %r6
        mr %r8, %r6
        mr %r9, %r6
        mr %r10, %r6
        mr %r11, %r6
        mr %r12, %r6
        mr %r13, %r6
        mr %r14, %r6
        mr %r15, %r6
        mr %r16, %r6
        mr %r17, %r6
        mr %r18, %r6
        mr %r19, %r6
        mr %r20, %r6
        mr %r21, %r6
        mr %r22, %r6
        mr %r23, %r6
        mr %r24, %r6
        mr %r25, %r6
        mr %r26, %r6
        mr %r27, %r6
        mr %r28, %r6
        mr %r29, %r6
        mr %r30, %r6
        mr %r31, %r6
        
        blr
        .size ja_context_restart, . - ja_context_restart

        .globl ja_context_restart_and_save
ja_context_restart_and_save:
        /* r3: stack, r4: entry, r5 - address to save resulted context */
        subi %r6, %r3, SIZEOF_jet_stack_frame_null
        stw %r6, 0(%r5)
        b ja_context_restart
        .size ja_context_restart_and_save, . - ja_context_restart_and_save
