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

        /* Switch stack. */
        stw     %r1,0(%r3)
        mr      %r1,%r4
        
        /* Restore registers.  */
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

        .size ja_context_switch, . - ja_context_switch
