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
#include "reg.h"

#include "asm_offsets_interrupt_context.h"
#include "asm_offsets_context.h"
#include "asm_offsets_stack_frame.h"

/*
    SPRG (software use special-purpose register) designations:

    SPRG0 - 
    SPRG1 - kernel stack pointer
    SPRG2 - scratch register
    SPRG3 - scratch register
*/
        
        .section ".start", "ax"

base = .
        
reserved_0000:
        /* fails */
1:      b 1b 

#define FRAME_SIZE SIZEOF_jet_interrupt_context

        .globl ja_user_space_jump
ja_user_space_jump:
        /* r3 - kernel stack for return,
         * r4 - user space id,
         * r5 - entry point
         * r6 - user stack
         */
        // Prepare kernel stack for return
        sub %r7, %r3, %r1
        subi %r7, %r7, FRAME_SIZE
        li %r8, 0
        stwux %r8, %r1, %r7
        mtsprg 1, %r1
        // Prepare user stack, srr1 and srr0 for rfi
        subi %r1, %r6, 12
        mtsrr0 %r5
        lis %r9, (MSR_EE | MSR_IP | MSR_PR | MSR_FP)@ha
        addi %r9, %r9, (MSR_EE | MSR_IP | MSR_PR | MSR_FP)@l
        mtsrr1 %r9

        // Nullify other registers (0, 2-7, 9-31)
        mr %r0, %r8
        mr %r2, %r8
        mr %r3, %r8
        mr %r4, %r8
        mr %r5, %r8
        mr %r6, %r8
        mr %r7, %r8
        mr %r9, %r8
        mr %r10, %r8
        mr %r11, %r8
        mr %r12, %r8
        mr %r13, %r8
        mr %r14, %r8
        mr %r15, %r8
        mr %r16, %r8
        mr %r17, %r8
        mr %r18, %r8
        mr %r19, %r8
        mr %r20, %r8
        mr %r21, %r8
        mr %r22, %r8
        mr %r23, %r8
        mr %r24, %r8
        mr %r25, %r8
        mr %r26, %r8
        mr %r27, %r8
        mr %r28, %r8
        mr %r29, %r8
        mr %r30, %r8
        mr %r31, %r8

        rfi

/*
 * Store registers to interrupt frame.
 * This is GDB-unaware version.
 *
 * Before the call:
 *
 * - r1 is adjusted to the beginning of the interrupt frame.
 * - cr is already stored to the frame.
 * - r3 contains old r1.
 *
 * After the call:
 * - srr0, srr1, r3, r4 are stored outside of the macro
 * - sprg3 contains old r3.
 * - sprg2 contains old r4.
 */
.macro SAVE_REGS_COMMON_BASE
        mflr    %r4
        stw     %r4,OFFSETOF_jet_interrupt_context_lr(%r1)
        mfctr   %r4
        stw     %r4,OFFSETOF_jet_interrupt_context_ctr(%r1)
        mfxer   %r4
        stw     %r4,OFFSETOF_jet_interrupt_context_xer(%r1)
        stw     %r0,OFFSETOF_jet_interrupt_context_r0(%r1)
        stw     %r3,OFFSETOF_jet_interrupt_context_r1(%r1)
        stw     %r2,OFFSETOF_jet_interrupt_context_r2(%r1)
        stw     %r5,OFFSETOF_jet_interrupt_context_r5(%r1)
        stw     %r6,OFFSETOF_jet_interrupt_context_r6(%r1)
        stw     %r7,OFFSETOF_jet_interrupt_context_r7(%r1)
        stw     %r8,OFFSETOF_jet_interrupt_context_r8(%r1)
        stw     %r9,OFFSETOF_jet_interrupt_context_r9(%r1)
        stw     %r10,OFFSETOF_jet_interrupt_context_r10(%r1)
        stw     %r11,OFFSETOF_jet_interrupt_context_r11(%r1)
        stw     %r12,OFFSETOF_jet_interrupt_context_r12(%r1)
.endm

#ifndef POK_NEEDS_GDB
#define SAVE_REGS_COMMON SAVE_REGS_COMMON_BASE
#else
.macro SAVE_REGS_COMMON
        SAVE_REGS_COMMON_BASE
        /* For GDB store non-volatile registers too. */
        stw     %r13,OFFSETOF_jet_interrupt_context_r13(%r1)
        stw     %r14,OFFSETOF_jet_interrupt_context_r14(%r1)
        stw     %r15,OFFSETOF_jet_interrupt_context_r15(%r1)
        stw     %r16,OFFSETOF_jet_interrupt_context_r16(%r1)
        stw     %r17,OFFSETOF_jet_interrupt_context_r17(%r1)
        stw     %r18,OFFSETOF_jet_interrupt_context_r18(%r1)
        stw     %r19,OFFSETOF_jet_interrupt_context_r19(%r1)
        stw     %r20,OFFSETOF_jet_interrupt_context_r20(%r1)
        stw     %r21,OFFSETOF_jet_interrupt_context_r21(%r1)
        stw     %r22,OFFSETOF_jet_interrupt_context_r22(%r1)
        stw     %r23,OFFSETOF_jet_interrupt_context_r23(%r1)
        stw     %r24,OFFSETOF_jet_interrupt_context_r24(%r1)
        stw     %r25,OFFSETOF_jet_interrupt_context_r25(%r1)
        stw     %r26,OFFSETOF_jet_interrupt_context_r26(%r1)
        stw     %r27,OFFSETOF_jet_interrupt_context_r27(%r1)
        stw     %r28,OFFSETOF_jet_interrupt_context_r28(%r1)
        stw     %r29,OFFSETOF_jet_interrupt_context_r29(%r1)
        stw     %r30,OFFSETOF_jet_interrupt_context_r30(%r1)
        stw     %r31,OFFSETOF_jet_interrupt_context_r31(%r1)
        /*
         * Also store pointer to the interrupt context.
         *
         * TODO: This is needed only for user space context, which always
         * at the beginning of the stack.
         */
        lis %r5, global_thread_stack@ha
        addi %r5, %r5, global_thread_stack@l
        stw %r1, 0(r5)
.endm
#endif /* POK_NEEDS_GDB */

/*
 * Restore registers from interrupt frame.
 * This is GDB-unaware version.
 *
 * Before the call:
 *
 * srr0, srr1 are already restored to their final location.
 * r3, r4 are already restored into temporary locations.
 *
 * After the call:
 *
 * - r1 remains pointed to the beginning of the interrupt frame.
 * - r3 contains restored r1
 * - cr is not restored yet.
 */
.macro RESTORE_REGS_COMMON_BASE
        lwz     %r12,OFFSETOF_jet_interrupt_context_r12(%r1)
        lwz     %r11,OFFSETOF_jet_interrupt_context_r11(%r1)
        lwz     %r10,OFFSETOF_jet_interrupt_context_r10(%r1)
        lwz     %r9,OFFSETOF_jet_interrupt_context_r9(%r1)
        lwz     %r8,OFFSETOF_jet_interrupt_context_r8(%r1)
        lwz     %r7,OFFSETOF_jet_interrupt_context_r7(%r1)
        lwz     %r6,OFFSETOF_jet_interrupt_context_r6(%r1)
        lwz     %r5,OFFSETOF_jet_interrupt_context_r5(%r1)
        lwz     %r2,OFFSETOF_jet_interrupt_context_r2(%r1)
        lwz     %r3,OFFSETOF_jet_interrupt_context_r1(%r1)
        lwz     %r0,OFFSETOF_jet_interrupt_context_r0(%r1)
        lwz     %r4,OFFSETOF_jet_interrupt_context_xer(%r1)
        mtxer   %r4
        lwz     %r4,OFFSETOF_jet_interrupt_context_ctr(%r1)
        mtctr   %r4
        lwz     %r4,OFFSETOF_jet_interrupt_context_lr(%r1)
        mtlr    %r4
.endm

#ifndef POK_NEEDS_GDB
#define RESTORE_REGS_COMMON RESTORE_REGS_COMMON_BASE
#else
.macro RESTORE_REGS_COMMON
        /* For GDB restore non-volatile registers too, they could be changed. */
        lwz     %r31,OFFSETOF_jet_interrupt_context_r31(%r1)
        lwz     %r30,OFFSETOF_jet_interrupt_context_r30(%r1)
        lwz     %r29,OFFSETOF_jet_interrupt_context_r29(%r1)
        lwz     %r28,OFFSETOF_jet_interrupt_context_r28(%r1)
        lwz     %r27,OFFSETOF_jet_interrupt_context_r27(%r1)
        lwz     %r26,OFFSETOF_jet_interrupt_context_r26(%r1)
        lwz     %r25,OFFSETOF_jet_interrupt_context_r25(%r1)
        lwz     %r24,OFFSETOF_jet_interrupt_context_r24(%r1)
        lwz     %r23,OFFSETOF_jet_interrupt_context_r23(%r1)
        lwz     %r22,OFFSETOF_jet_interrupt_context_r22(%r1)
        lwz     %r21,OFFSETOF_jet_interrupt_context_r21(%r1)
        lwz     %r20,OFFSETOF_jet_interrupt_context_r20(%r1)
        lwz     %r19,OFFSETOF_jet_interrupt_context_r19(%r1)
        lwz     %r18,OFFSETOF_jet_interrupt_context_r18(%r1)
        lwz     %r17,OFFSETOF_jet_interrupt_context_r17(%r1)
        lwz     %r16,OFFSETOF_jet_interrupt_context_r16(%r1)
        lwz     %r15,OFFSETOF_jet_interrupt_context_r15(%r1)
        lwz     %r14,OFFSETOF_jet_interrupt_context_r14(%r1)
        lwz     %r13,OFFSETOF_jet_interrupt_context_r13(%r1)
        RESTORE_REGS_COMMON_BASE
.endm
#endif /* POK_NEEDS_GDB */

/*
 * Common part of interrupt epilogue.
 *
 * Prepare everything except srr0, srr1, r3, r4.
 */
.macro EXCEPTION_EPILOGUE_COMMON
        RESTORE_REGS_COMMON
        /* Determine whether we return to user space. */
        lwz %r4, OFFSETOF_jet_interrupt_context_srr1(%r1)
        andi.   %r4,%r4,MSR_PR
        beq     1f
        /* Return to user - need to set SPRG1. */
        lwz %r4, OFFSETOF_jet_interrupt_context_cr(%r1) // Restore everything else from interrupt frame.
        mtcr %r4
        mtsprg 1, %r1
        b 2f
1:
        /* Return to kernel. */
        lwz %r4, OFFSETOF_jet_interrupt_context_cr(%r1) // Restore everything else from interrupt frame.
        mtcr %r4
2:
        mr %r1, %r3
.endm

/*
 * Common exception prologue.
 *
 * Before the call:
 *     r3 and r4 are stored to some temporary locations before.
 *
 * After the call:
 *     r1 points to interrupt frame, where r3, r4, srr0 and srr1 should be stored.
 */
.macro EXCEPTION_PROLOGUE_COMMON
        mfcr    %r3
        /* Enable floating point bit in msr */
        mfmsr %r4
        ori %r4,%r4,MSR_FP
        mtmsr %r4
        /* Check whether kernel stack is usable. */
        mfsprg %r4, 1
        andi.   %r4, %r4, 1 /* Valid stack is always 16-byte aligned. */
        beq     1f
        /* Come from kernel and kernel stack is ready */
        stwu %r1, -FRAME_SIZE-JET_STACK_RED_ZONE(%r1)
        stw %r3, OFFSETOF_jet_interrupt_context_cr(%r1)
        /* Store lr to previous backchain. */
        mflr %r3
        stw %r3, FRAME_SIZE + JET_STACK_RED_ZONE + OFFSETOF_jet_stack_frame_lr(%r1)
        addi %r3, %r1, FRAME_SIZE + JET_STACK_RED_ZONE
        b       2f
1:      /* Come from user or interrupted handler hasn't adjust stack yet. */
        mfsprg %r4, 1
        mtcr %r3 /* Restore cr, so possible highlevel interrupt will see the same as we.*/
        /* Store cr before updating stack pointer! It can be overwritting by highlevel interrupt... but to the same value(see above). */
        stw %r3, OFFSETOF_jet_interrupt_context_cr(%r4)
        mr %r3, %r1
        mr %r1, %r4 /* Back chain is already set to 0. */
        /* Mark kernel stack as ready for highlevel interrupts. */
        li %r4, 1
        mtsprg 1, %r4
2:      SAVE_REGS_COMMON
.endm

/* Return from non-critical interrupt. */
        .globl pok_arch_rfi
pok_arch_rfi:
        /* r4 <-> sprg2, r3 <-> sprg3 */
        lwz     %r4,OFFSETOF_jet_interrupt_context_r4(%r1)
        mtsprg  2,%r4
        lwz     %r3,OFFSETOF_jet_interrupt_context_r3(%r1)
        mtsprg  3,%r3

        /* srr0->srr0, srr1->srr1. */
        lwz     %r4,OFFSETOF_jet_interrupt_context_srr1(%r1)
        mtsrr1  %r4
        lwz     %r3,OFFSETOF_jet_interrupt_context_srr0(%r1)
        mtsrr0  %r3

        EXCEPTION_EPILOGUE_COMMON

        mfsprg %r3, 3
        mfsprg %r4, 2

        rfi


/* Exception prologue for non-critical interrupts. */
.macro EXCEPTION_PROLOGUE
        /* r3 <-> sprg3, r4 <-> sprg2*/
        mtsprg  3,%r3
        mtsprg  2,%r4
        EXCEPTION_PROLOGUE_COMMON

        /* srr0<-srr0, srr1<-srr1. */
        mfsrr0  %r3
        stw     %r3,OFFSETOF_jet_interrupt_context_srr0(%r1)
        mfsrr1  %r4
        stw     %r4,OFFSETOF_jet_interrupt_context_srr1(%r1)

        mfsprg  %r4,2
        stw     %r4,OFFSETOF_jet_interrupt_context_r4(%r1)
        mfsprg  %r3,3
        stw     %r3,OFFSETOF_jet_interrupt_context_r3(%r1)
.endm
        
        
/* Return from debug interrupt. */
        .globl pok_arch_rfdi_for_debug
pok_arch_rfdi_for_debug:
        /* r4 <-> sprg2, r3 <-> sprg3. TODO: other temporary locations should be used. */
        lwz     %r4,OFFSETOF_jet_interrupt_context_r4(%r1)
        mtsprg  2,%r4
        lwz     %r3,OFFSETOF_jet_interrupt_context_r3(%r1)
        mtsprg  2,%r3

        /* srr0->dsrr0, srr1->dsrr1. */
        lwz     %r4,OFFSETOF_jet_interrupt_context_srr1(%r1)
        mtspr   SPRN_DSRR1, %r4
        lwz     %r3,OFFSETOF_jet_interrupt_context_srr0(%r1)
        mtspr   SPRN_DSRR0, %r3

        EXCEPTION_EPILOGUE_COMMON

        mfsprg  %r4, 2
        mfsprg  %r3, 3

        rfdi

.macro EXCEPTION_PROLOGUE_DEBUG
        /* r3 <-> sprg3, r4 <-> sprg2. TODO: use different temporaries. */
        mtsprg  3,%r3
        mtsprg  2,%r4
        EXCEPTION_PROLOGUE_COMMON

        /* srr0<-dsrr0, srr1<-dsrr1. */
        mtspr   SPRN_DSRR0, %r3
        stw     %r3,OFFSETOF_jet_interrupt_context_srr0(%r1)
        mtspr   SPRN_DSRR0, %r4
        stw     %r4,OFFSETOF_jet_interrupt_context_srr1(%r1)

        mfsprg  %r3,3
        stw     %r3,OFFSETOF_jet_interrupt_context_r3(%r1)
        mfsprg  %r4,2
        stw     %r4,OFFSETOF_jet_interrupt_context_r4(%r1)
.endm

        . = base + 0x300
        .global pok_trap_addr
pok_trap_addr:
        .global pok_trap
pok_trap:
        trap
        blr




failed: b failed


        . = base + 0x350
        .globl _pok_reset
system_reset_0100:
_pok_reset:
        /*write r3(devtree addr) to devtree_address global variable*/
        lis %r1, (devtree_address)@h
        ori %r1,%r1,(devtree_address)@l
        stw %r3, 0(r1)

        /* Setup stack */
        lis %r1,(pok_stack_end-4)@h
        ori %r1,%r1,(pok_stack_end-4)@l

        /* Mark kernel stack as ready for interrupts. */
        li %r4, 1
        mtsprg 1, %r4


        bl pok_setup_interrupts

        /* Clear back chain */
        xor %r0,%r0,%r0
        stw %r0,4(%r1)

        /* TODO: copy .sdata, clear .bss */
        /* Copy .data */
/*
        lis %r3,(__data_start-4)@h
        ori %r3,%r3,(__data_start-4)@l
        lis %r4,(__data_load-4)@h
        ori %r4,%r4,(__data_load-4)@l
        lis %r5,__data_end@h
        ori %r5,%r5,__data_end@l
        cmplw %r3,%r5
        bge 2f
1:      lwzu %r6,4(%r4)
        stwu %r6,4(%r3)
        cmplw %r3,%r5
        blt 1b
2:
*/
        /* Call C functions */
        bl pok_arch_init
        bl pok_bsp_init
        bl pok_bsp_time_init

        bl jet_boot /* shouldn't return */
1:      b  1b

/* Interrupt vectors */
/* TODO handle kernel / user stack transition correctly */
/* TODO some interrupts, like "critical interrupt", use different
        set of save/restore registers, so they need
        a different prologue and rfi functions
*/


// Useful defines:
#define START_EXCEPTION(label)                                               \
        .align 5;                                                            \
label##_asm:

// Handlers themselves:

    /* TODO: shouldn't critical interrupt level should be used there? */
    START_EXCEPTION(pok_int_critical_input)
        EXCEPTION_PROLOGUE
        mr %r3, %r1
        bl pok_int_critical_input
        b pok_arch_rfi

    /* TODO: shouldn't machine check interrupt level should be used there? */
    START_EXCEPTION(pok_int_machine_check)
        EXCEPTION_PROLOGUE
        mr %r3, %r1
        bl pok_int_machine_check
        b pok_arch_rfi

    START_EXCEPTION(pok_int_data_storage)
        EXCEPTION_PROLOGUE
        mr %r3, %r1

        mfspr %r4, SPRN_DEAR // DEAR - faulting address
        mfspr %r5, SPRN_ESR // ESR - exception syndrome

        bl pok_int_data_storage
        b  pok_arch_rfi

    START_EXCEPTION(pok_int_inst_storage)
        EXCEPTION_PROLOGUE
        mr %r3, %r1
        mfspr %r4, SPRN_DEAR // DEAR - faulting address
        mfspr %r5, SPRN_ESR // ESR - exception syndrome

        bl pok_int_inst_storage
        b  pok_arch_rfi

    START_EXCEPTION(pok_int_ext_interrupt)
        EXCEPTION_PROLOGUE
        mr %r3, %r1
        bl pok_int_ext_interrupt
        b pok_arch_rfi

    START_EXCEPTION(pok_int_alignment)
        EXCEPTION_PROLOGUE
        mr %r3, %r1

        bl pok_int_alignment
        b pok_arch_rfi

    START_EXCEPTION(pok_int_program)
        EXCEPTION_PROLOGUE
        mr %r3, %r1

        bl pok_int_program
        b pok_arch_rfi

    START_EXCEPTION(pok_int_fp_unavail)
        EXCEPTION_PROLOGUE
        mr %r3, %r1

        bl pok_int_fp_unavail
        b pok_arch_rfi

    START_EXCEPTION(pok_int_system_call)
        EXCEPTION_PROLOGUE
        mr %r3, %r1
        /* load system call arguments back from the interrupt frame */
        lwz     %r4,OFFSETOF_jet_interrupt_context_r3(%r1)
        lwz     %r5,OFFSETOF_jet_interrupt_context_r4(%r1)
        lwz     %r6,OFFSETOF_jet_interrupt_context_r5(%r1)
        lwz     %r7,OFFSETOF_jet_interrupt_context_r6(%r1)
        lwz     %r8,OFFSETOF_jet_interrupt_context_r7(%r1)
        lwz     %r9,OFFSETOF_jet_interrupt_context_r8(%r1)

        bl      pok_int_system_call 
        stw     %r3,OFFSETOF_jet_interrupt_context_r3(%r1)

        b       pok_arch_rfi

    START_EXCEPTION(pok_int_decrementer)
        EXCEPTION_PROLOGUE
        mr %r3, %r1

        bl      pok_int_decrementer 
        b       pok_arch_rfi

    START_EXCEPTION(pok_int_interval_timer)
        EXCEPTION_PROLOGUE
        mr %r3, %r1

        bl      pok_int_interval_timer
        b       pok_arch_rfi

    START_EXCEPTION(pok_int_watchdog)
        EXCEPTION_PROLOGUE
        mr %r3, %r1

        bl      pok_int_watchdog
        b       pok_arch_rfi

    START_EXCEPTION(pok_int_data_tlb_miss)
        EXCEPTION_PROLOGUE
        mr %r3, %r1
        mfspr %r4, SPRN_DEAR // DEAR - faulting address
        mfspr %r5, SPRN_ESR // ESR - exception syndrome

        bl      pok_int_data_tlb_miss
        b       pok_arch_rfi

    START_EXCEPTION(pok_int_inst_tlb_miss)
        EXCEPTION_PROLOGUE
        mr %r3, %r1
        mfspr %r4, SPRN_DEAR // DEAR - faulting address
        mfspr %r5, SPRN_ESR // ESR - exception syndrome

        bl      pok_int_inst_tlb_miss
        b       pok_arch_rfi
        
    START_EXCEPTION(pok_int_debug)
        EXCEPTION_PROLOGUE_DEBUG

        bl      pok_int_debug
        b       pok_arch_rfdi_for_debug

#define SET_IVOR(vector_number, vector_label)           \
                li      r26,vector_label##_asm @ l;             \
                mtspr   SPRN_IVOR##vector_number,r26;   \
                sync

pok_setup_interrupts:
        lis     %r3, pok_int_critical_input_asm@h // higher 16 bits
        mtspr   SPRN_IVPR, %r3

        SET_IVOR(0,  pok_int_critical_input)
        SET_IVOR(1,  pok_int_machine_check)
        SET_IVOR(2,  pok_int_data_storage)
        SET_IVOR(3,  pok_int_inst_storage)
        SET_IVOR(4,  pok_int_ext_interrupt)
        SET_IVOR(5,  pok_int_alignment)
        SET_IVOR(6,  pok_int_program)
        SET_IVOR(7,  pok_int_fp_unavail)
        SET_IVOR(8,  pok_int_system_call)
        SET_IVOR(10, pok_int_decrementer)
        SET_IVOR(11, pok_int_interval_timer)
        SET_IVOR(12, pok_int_watchdog)
        SET_IVOR(13, pok_int_data_tlb_miss)
        SET_IVOR(14, pok_int_inst_tlb_miss)
        SET_IVOR(15, pok_int_debug)

        blr

        .section ".bss", "aw"
pok_stack:
		.global pok_stack_end
        .space 8 * 1024
pok_stack_end:

pok_save_area:
        .space 4 * 8
