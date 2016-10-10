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
1:      JUMP_TO_ADDR(1b) 

#define FRAME_SIZE SIZEOF_jet_interrupt_context

        .globl ja_user_space_jump
ja_user_space_jump:
        /* r3 - kernel stack for return,
         * r4 - user space id,
         * r5 - entry point
         * r6 - user stack
         */
        // Prepare kernel stack for return
        add $r7, $r3, $r1
        addi $r7, $r7, -FRAME_SIZE
        li $r8, 0
 *       stwux $r8, $r1, $r7
  *      mtsprg 1, $r1
        // Prepare user stack, srr1 and srr0 for rfi
        addi $r1, $r6, -16
 *       mtsrr0 $r5
 *       lis $r9, (MSR_EE | MSR_IP | MSR_PR | MSR_FP)@ha
  *      addi $r9, $r9, (MSR_EE | MSR_IP | MSR_PR | MSR_FP)@l
  *      mtsrr1 $r9

        // Nullify other registers (0, 2-7, 9-31)
        move $r0, $r8
        move $r2, $r8
        move $r3, $r8
        move $r4, $r8
        move $r5, $r8
        move $r6, $r8
        move $r7, $r8
        move $r9, $r8
        move $r10, $r8
        move $r11, $r8
        move $r12, $r8
        move $r13, $r8
        move $r14, $r8
        move $r15, $r8
        move $r16, $r8
        move $r17, $r8
        move $r18, $r8
        move $r19, $r8
        move $r20, $r8
        move $r21, $r8
        move $r22, $r8
        move $r23, $r8
        move $r24, $r8
        move $r25, $r8
        move $r26, $r8
        move $r27, $r8
        move $r28, $r8
        move $r29, $r8
        move $r30, $r8
        move $r31, $r8

        ERET_AND_NOP_AND_NOP
        JUMP_TO_REG($ra)

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
  *      mflr    $r4
        sw     $r4,OFFSETOF_jet_interrupt_context_lr($r1)
 *       mfctr   $r4
        sw     $r4,OFFSETOF_jet_interrupt_context_ctr($r1)
  *      mfxer   $r4
        sw      $4,OFFSETOF_jet_interrupt_context_xer($r1)
        sw      $0,OFFSETOF_jet_interrupt_context_0($r1)
        sw      $3,OFFSETOF_jet_interrupt_context_1($r1)
        sw      $2,OFFSETOF_jet_interrupt_context_2($r1)
        sw      $5,OFFSETOF_jet_interrupt_context_5($r1)
        sw      $6,OFFSETOF_jet_interrupt_context_6($r1)
        sw      $7,OFFSETOF_jet_interrupt_context_7($r1)
        sw      $8,OFFSETOF_jet_interrupt_context_8($r1)
        sw      $9,OFFSETOF_jet_interrupt_context_9($r1)
        sw      $10,OFFSETOF_jet_interrupt_context_10($r1)
        sw      $11,OFFSETOF_jet_interrupt_context_11($r1)
        sw      $12,OFFSETOF_jet_interrupt_context_12($r1)
.endm

#ifndef POK_NEEDS_GDB
#define SAVE_REGS_COMMON SAVE_REGS_COMMON_BASE
#else
.macro SAVE_REGS_COMMON
        SAVE_REGS_COMMON_BASE
        /* For GDB store non-volatile registers too. */
        sw      $13,OFFSETOF_jet_interrupt_context_13($r1)
        sw      $14,OFFSETOF_jet_interrupt_context_14($r1)
        sw      $15,OFFSETOF_jet_interrupt_context_15($r1)
        sw      $16,OFFSETOF_jet_interrupt_context_16($r1)
        sw      $17,OFFSETOF_jet_interrupt_context_17($r1)
        sw      $18,OFFSETOF_jet_interrupt_context_18($r1)
        sw      $19,OFFSETOF_jet_interrupt_context_19($r1)
        sw      $20,OFFSETOF_jet_interrupt_context_20($r1)
        sw      $21,OFFSETOF_jet_interrupt_context_21($r1)
        sw      $22,OFFSETOF_jet_interrupt_context_22($r1)
        sw      $23,OFFSETOF_jet_interrupt_context_23($r1)
        sw      $24,OFFSETOF_jet_interrupt_context_24($r1)
        sw      $25,OFFSETOF_jet_interrupt_context_25($r1)
        sw      $26,OFFSETOF_jet_interrupt_context_26($r1)
        sw      $27,OFFSETOF_jet_interrupt_context_27($r1)
        sw      $28,OFFSETOF_jet_interrupt_context_28($r1)
        sw      $29,OFFSETOF_jet_interrupt_context_29($r1)
        sw      $30,OFFSETOF_jet_interrupt_context_30($r1)
        sw      $31,OFFSETOF_jet_interrupt_context_31($r1)
        /*
         * Also store pointer to the interrupt context.
         *
         * TODO: This is needed only for user space context, which always
         * at the beginning of the stack.
         */
       * lis $r5, global_thread_stack@ha
     *   addi $r5, $r5, global_thread_stack@l
        sw $r1, 0(r5)
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
        lw      $12,OFFSETOF_jet_interrupt_context_12($r1)
        lw      $11,OFFSETOF_jet_interrupt_context_11($r1)
        lw      $10,OFFSETOF_jet_interrupt_context_10($r1)
        lw      $9,OFFSETOF_jet_interrupt_context_9($r1)
        lw      $8,OFFSETOF_jet_interrupt_context_8($r1)
        lw      $7,OFFSETOF_jet_interrupt_context_7($r1)
        lw      $6,OFFSETOF_jet_interrupt_context_6($r1)
        lw      $5,OFFSETOF_jet_interrupt_context_5($r1)
        lw      $2,OFFSETOF_jet_interrupt_context_2($r1)
        lw      $3,OFFSETOF_jet_interrupt_context_1($r1)
        lw      $0,OFFSETOF_jet_interrupt_context_0($r1)
        lw      $4,OFFSETOF_jet_interrupt_context_xer($r1)
    *    mtxer   $r4
        lw      $r4,OFFSETOF_jet_interrupt_context_ctr($r1)
      *  mtctr   $r4
        lw      $r4,OFFSETOF_jet_interrupt_context_lr($r1)
     *   mtlr    $r4
.endm

#ifndef POK_NEEDS_GDB
#define RESTORE_REGS_COMMON RESTORE_REGS_COMMON_BASE
#else
.macro RESTORE_REGS_COMMON
        /* For GDB restore non-volatile registers too, they could be changed. */
        lw      $31,OFFSETOF_jet_interrupt_context_31($r1)
        lw      $30,OFFSETOF_jet_interrupt_context_30($r1)
        lw      $29,OFFSETOF_jet_interrupt_context_29($r1)
        lw      $28,OFFSETOF_jet_interrupt_context_28($r1)
        lw      $27,OFFSETOF_jet_interrupt_context_27($r1)
        lw      $26,OFFSETOF_jet_interrupt_context_26($r1)
        lw      $25,OFFSETOF_jet_interrupt_context_25($r1)
        lw      $24,OFFSETOF_jet_interrupt_context_24($r1)
        lw      $23,OFFSETOF_jet_interrupt_context_23($r1)
        lw      $22,OFFSETOF_jet_interrupt_context_22($r1)
        lw      $21,OFFSETOF_jet_interrupt_context_21($r1)
        lw      $20,OFFSETOF_jet_interrupt_context_20($r1)
        lw      $19,OFFSETOF_jet_interrupt_context_19($r1)
        lw      $18,OFFSETOF_jet_interrupt_context_18($r1)
        lw      $17,OFFSETOF_jet_interrupt_context_17($r1)
        lw      $16,OFFSETOF_jet_interrupt_context_16($r1)
        lw      $15,OFFSETOF_jet_interrupt_context_15($r1)
        lw      $14,OFFSETOF_jet_interrupt_context_14($r1)
        lw      $13,OFFSETOF_jet_interrupt_context_13($r1)
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
        lw $r4, OFFSETOF_jet_interrupt_context_srr1($r1)
      *  andi.   $r4,$r4,MSR_PR
     *   beq     1f
        /* Return to user - need to set SPRG1. */
        lw $r4, OFFSETOF_jet_interrupt_context_cr($r1) // Restore everything else from interrupt frame.
     *   mtcr $r4
     *   mtsprg 1, $r1
      *  b 2f
1:
        /* Return to kernel. */
        lw $r4, OFFSETOF_jet_interrupt_context_cr($r1) // Restore everything else from interrupt frame.
  *      mtcr $r4
2:
        move $r1, $r3
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
    *    mfcr    $r3
        /* Enable floating point bit in msr */
    *    mfmsr $r4
     *   ori $r4,$r4,MSR_FP
     *   mtmsr $r4
        /* Check whether kernel stack is usable. */
     *   mfsprg $r4, 1
     *   andi.   $r4, $r4, 1 /* Valid stack is always 16-byte aligned. */
      *  beq     1f
        /* Come from kernel and kernel stack is ready */
     *   stwu $r1, -FRAME_SIZE-JET_STACK_RED_ZONE($r1)
        sw $r3, OFFSETOF_jet_interrupt_context_cr($r1)
        /* Store lr to previous backchain. */
     *   mflr $r3
        sw $r3, FRAME_SIZE + JET_STACK_RED_ZONE + OFFSETOF_jet_stack_frame_lr($r1)
     *   addi $r3, $r1, FRAME_SIZE + JET_STACK_RED_ZONE
     *   b       2f
1:      /* Come from user or interrupted handler hasn't adjust stack yet. */
      *  mfsprg $r4, 1
      *  mtcr $r3 /* Restore cr, so possible highlevel interrupt will see the same as we.*/
        /* Store cr before updating stack pointer! It can be overwritting by highlevel interrupt... but to the same value(see above). */
        sw $r3, OFFSETOF_jet_interrupt_context_cr($r4)
        move $r3, $r1
        move $r1, $r4 /* Back chain is already set to 0. */
        /* Mark kernel stack as ready for highlevel interrupts. */
        li $r4, 1
     *   mtsprg 1, $r4
2:      SAVE_REGS_COMMON
.endm

/* Return from non-critical interrupt. */
        .globl pok_arch_rfi
pok_arch_rfi:
        /* r4 <-> sprg2, r3 <-> sprg3 */
        lw     $r4,OFFSETOF_jet_interrupt_context_4($r1)
     *   mtsprg  2,$r4
        lw     $r3,OFFSETOF_jet_interrupt_context_3($r1)
     *   mtsprg  3,$r3

        /* srr0->srr0, srr1->srr1. */
        lw     $r4,OFFSETOF_jet_interrupt_context_srr1($r1)
     *   mtsrr1  $r4
        lw     $r3,OFFSETOF_jet_interrupt_context_srr0($r1)
     *   mtsrr0  $r3

        EXCEPTION_EPILOGUE_COMMON

     *   mfsprg $r3, 3
      *  mfsprg $r4, 2

        ERET_AND_NOP


/* Exception prologue for non-critical interrupts. */
.macro EXCEPTION_PROLOGUE
        /* r3 <-> sprg3, r4 <-> sprg2*/
      *  mtsprg  3,$r3
      *  mtsprg  2,$r4
        EXCEPTION_PROLOGUE_COMMON

        /* srr0<-srr0, srr1<-srr1. */
       * mfsrr0  $r3
        sw     $r3,OFFSETOF_jet_interrupt_context_srr0($r1)
     *   mfsrr1  $r4
        sw     $r4,OFFSETOF_jet_interrupt_context_srr1($r1)

     *   mfsprg  $r4,2
        sw     $r4,OFFSETOF_jet_interrupt_context_4($r1)
      *  mfsprg  $r3,3
        sw     $r3,OFFSETOF_jet_interrupt_context_3($r1)
.endm
        
        
/* Return from debug interrupt. */
        .globl pok_arch_rfdi_for_debug
pok_arch_rfdi_for_debug:
        /* r4 <-> sprg2, r3 <-> sprg3. TODO: other temporary locations should be used. */
        lw      $r4,OFFSETOF_jet_interrupt_context_4($r1)
    *    mtsprg  2,$r4
        lw      $r3,OFFSETOF_jet_interrupt_context_3($r1)
    *    mtsprg  2,$r3

        /* srr0->dsrr0, srr1->dsrr1. */
        lw      $r4,OFFSETOF_jet_interrupt_context_srr1($r1)
      *  mtspr   SPRN_DSRR1, $r4
        lw      $r3,OFFSETOF_jet_interrupt_context_srr0($r1)
      *  mtspr   SPRN_DSRR0, $r3

        EXCEPTION_EPILOGUE_COMMON

     *   mfsprg  $r4, 2
      *  mfsprg  $r3, 3

     *   rfdi

.macro EXCEPTION_PROLOGUE_DEBUG
        /* r3 <-> sprg3, r4 <-> sprg2. TODO: use different temporaries. */
     *   mtsprg  3,$r3
     *   mtsprg  2,$r4
        EXCEPTION_PROLOGUE_COMMON

        /* srr0<-dsrr0, srr1<-dsrr1. */
     *   mtspr   SPRN_DSRR0, $r3
        sw      $r3,OFFSETOF_jet_interrupt_context_srr0($r1)
      *  mtspr   SPRN_DSRR0, $r4
        sw      $r4,OFFSETOF_jet_interrupt_context_srr1($r1)

      *  mfsprg  $r3,3
        sw      $r3,OFFSETOF_jet_interrupt_context_3($r1)
     *   mfsprg  $r4,2
        sw      $r4,OFFSETOF_jet_interrupt_context_4($r1)
.endm

        . = base + 0x300
        .global pok_trap_addr
pok_trap_addr:
        .global pok_trap
pok_trap:
        break
      *  blr




failed: b failed


        . = base + 0x350
        .globl _pok_reset
system_reset_0100:
_pok_reset:
        /*write r3(devtree addr) to devtree_address global variable*/
      *  lis $r1, (devtree_address)@h
      *  ori $r1,$r1,(devtree_address)@l
        sw  $r3, 0(r1)

        /* Setup stack */
      *  lis $r1,(pok_stack_end-4)@h
       * ori $r1,$r1,(pok_stack_end-4)@l

        /* Mark kernel stack as ready for interrupts. */
        li $r4, 1
      *  mtsprg 1, $r4


        JUMP_AND_LINK(pok_setup_interrupts)

        /* Clear back chain */
      *  xor $r0,$r0,$r0
        sw  $r0,4($r1)

        /* TODO: copy .sdata, clear .bss */
        /* Copy .data */
/*
    *    lis $r3,(__data_start-4)@h
    *    ori $r3,$r3,(__data_start-4)@l
     *   lis $r4,(__data_load-4)@h
     *   ori $r4,$r4,(__data_load-4)@l
     *   lis $r5,__data_end@h
     *   ori $r5,$r5,__data_end@l
     *   cmplw $r3,$r5
     *   bge 2f
1:   *   lwzu $r6,4($r4)
     *   stwu $r6,4($r3)
     *   cmplw $r3,$r5
     *   blt 1b
2:
*/
        /* Call C functions */
        JUMP_AND_LINK(pok_arch_init)

        JUMP_AND_LINK(jet_boot) /* shouldn't return */
1:      JUMP_TO_ADDR(1b)

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
        move $r3, $r1
        JUMP_AND_LINK(pok_int_critical_input)
        JUMP_TO_ADDR(pok_arch_rfi)

    /* TODO: shouldn't machine check interrupt level should be used there? */
    START_EXCEPTION(pok_int_machine_check)
        EXCEPTION_PROLOGUE
        move $r3, $r1
        JUMP_AND_LINK(pok_int_machine_check)
        JUMP_TO_ADDR(pok_arch_rfi)

    START_EXCEPTION(pok_int_data_storage)
        EXCEPTION_PROLOGUE
        move $r3, $r1

      *  mfspr $r4, SPRN_DEAR // DEAR - faulting address
      *  mfspr $r5, SPRN_ESR // ESR - exception syndrome

        JUMP_AND_LINK(pok_int_data_storage)
        JUMP_TO_ADDR(pok_arch_rfi)

    START_EXCEPTION(pok_int_inst_storage)
        EXCEPTION_PROLOGUE
        move $r3, $r1
    *    mfspr $r4, SPRN_DEAR // DEAR - faulting address
     *   mfspr $r5, SPRN_ESR // ESR - exception syndrome

        JUMP_AND_LINK(pok_int_inst_storage)
        JUMP_TO_ADDR(pok_arch_rfi)

    START_EXCEPTION(pok_int_ext_interrupt)
        EXCEPTION_PROLOGUE
        move $r3, $r1
        JUMP_AND_LINK(pok_int_ext_interrupt)
        JUMP_TO_ADDR(pok_arch_rfi)

    START_EXCEPTION(pok_int_alignment)
        EXCEPTION_PROLOGUE
        move $r3, $r1

        JUMP_AND_LINK(pok_int_alignment)
        JUMP_TO_ADDR(pok_arch_rfi)

    START_EXCEPTION(pok_int_program)
        EXCEPTION_PROLOGUE
        move $r3, $r1

        JUMP_AND_LINK(pok_int_program)
        JUMP_TO_ADDR(pok_arch_rfi)

    START_EXCEPTION(pok_int_fp_unavail)
        EXCEPTION_PROLOGUE
        move $r3, $r1

        JUMP_AND_LINK(pok_int_fp_unavail)
        JUMP_TO_ADDR(pok_arch_rfi)

    START_EXCEPTION(pok_int_system_call)
        EXCEPTION_PROLOGUE
        move $r3, $r1
        /* load system call arguments back from the interrupt frame */
        lw      $r4,OFFSETOF_jet_interrupt_context_3($r1)
        lw      $r5,OFFSETOF_jet_interrupt_context_4($r1)
        lw      $r6,OFFSETOF_jet_interrupt_context_5($r1)
        lw      $r7,OFFSETOF_jet_interrupt_context_6($r1)
        lw      $r8,OFFSETOF_jet_interrupt_context_7($r1)
        lw      $r9,OFFSETOF_jet_interrupt_context_8($r1)

        JUMP_AND_LINK(pok_int_system_call) 
        sw      $r3,OFFSETOF_jet_interrupt_context_3($r1)

        JUMP_TO_ADDR(pok_arch_rfi)

    START_EXCEPTION(pok_int_decrementer)
        EXCEPTION_PROLOGUE
        move $r3, $r1

        JUMP_AND_LINK(pok_int_decrementer) 
        JUMP_TO_ADDR(pok_arch_rfi)

    START_EXCEPTION(pok_int_interval_timer)
        EXCEPTION_PROLOGUE
        move $r3, $r1

        JUMP_AND_LINK(pok_int_interval_timer)
        JUMP_TO_ADDR(pok_arch_rfi)

    START_EXCEPTION(pok_int_watchdog)
        EXCEPTION_PROLOGUE
        move $r3, $r1

        JUMP_AND_LINK(pok_int_watchdog)
        JUMP_TO_ADDR(pok_arch_rfi)

    START_EXCEPTION(pok_int_data_tlb_miss)
        EXCEPTION_PROLOGUE
        move $r3, $r1
   *     mfspr $r4, SPRN_DEAR // DEAR - faulting address
   *     mfspr $r5, SPRN_ESR // ESR - exception syndrome

        JUMP_AND_LINK(pok_int_data_tlb_miss)
        JUMP_TO_ADDR(pok_arch_rfi)

    START_EXCEPTION(pok_int_inst_tlb_miss)
        EXCEPTION_PROLOGUE
        move $r3, $r1
     *   mfspr $r4, SPRN_DEAR // DEAR - faulting address
     *   mfspr $r5, SPRN_ESR // ESR - exception syndrome

        JUMP_AND_LINK(pok_int_inst_tlb_miss)
        JUMP_TO_ADDR(pok_arch_rfi)
        
    START_EXCEPTION(pok_int_debug)
        EXCEPTION_PROLOGUE_DEBUG

        JUMP_AND_LINK(pok_int_debug)
        JUMP_TO_ADDR(pok_arch_rfdi_for_debug)
/*
 *  Adding addr of C part of exeption handler for each interrupt in array
 */
#define SET_IVOR(vector_number, vector_label)           \               
                li      r26,vector_label##_asm @ l;             \               
                sw      r26, vector_number(expection_handlers)      \
                sync

pok_setup_interrupts:
        /*Switch on using EBASE addr for exeption handler*/
        mfc0   $k0, CPO_STATUS
        addi   $k0, $k0, ~CPO_STATUS_BEV
        MTC0($k0, CPO_STATUS)
        /*Switch off using the special interrupt vector*/ 
        mfc0   $k0, CPO_CAUSE
        addi   $k0, $k0, ~CPO_CAUSE_IV
        MTC0($k0, CPO_CAUSE)
        lis    $k0, mips_first_exeption_handler_asm@h // higher 16 bits
        MTC0_sel1(CP0_EBASE, $k0)
        

pok_int_critical_input)
pok_int_machine_check) 
pok_int_data_storage)  
pok_int_inst_storage)  
pok_int_ext_interrupt) 
pok_int_alignment)     
pok_int_program)       
pok_int_fp_unavail)    
pok_int_system_call)   
pok_int_decrementer)   
pok_int_interval_timer)
pok_int_watchdog)      
pok_int_data_tlb_miss) 
pok_int_inst_tlb_miss) 
pok_int_debug)         
pok_int_debug)         
pok_int_debug)      


        SET_IVOR(0, pok_int_decrementer)   /*Прерывание                    INT Interrupt               */
        SET_IVOR(1,                        /*Запись в защищенную страницу                              */
        SET_IVOR(2, pok_int_inst_tlb_miss) /*Исключение в TLB (при чтении данных                       */
                                           /*                            или при выборке инструкции)   */
        SET_IVOR(3, pok_int_data_tlb_miss) /*Исключение в TLB (при записи данных)                      */
        SET_IVOR(4,                        /*Неправильный адрес при чтении  ADDRL Load from an illegal address*/
        SET_IVOR(5,                        /*Неправильный адрес при записи  ADDRS Store to an illegal address                           */
        SET_IVOR(6,                        /*Ошибка шины при выборе команды IBUS Bus error on instruction fetch                           */
        SET_IVOR(7,                        /*Ошибка шины при чтении данных  DBUS Bus error on data reference                           */
        SET_IVOR(8, pok_int_system_call)   /*Команда SYSCALL                SYSCALL syscall instruction executed                           */
        SET_IVOR(9, pok_int_debug)         /*Команда BREAK                  BKPT break instruction executed                           */
        SET_IVOR(10,                       /*Нереализованная команда        RI Reserved instruction                           */
        SET_IVOR(11,                       /*Недоступный сопроцессор                                   */
        SET_IVOR(12,                       /*Целочисленное переполнение     OVF Arithmetic overflow                              */
        SET_IVOR(13, pok_int_debug)        /*Команды TRAP                                              */
        SET_IVOR(14, pok_int_fp_unavail) ????  /*Исключение от блока вещественной арифметики               */
        SET_IVOR(15,                       /*Исключение при доступе в накристальную память             */    



       * blr

mips_first_exeption_handler:
        .set  push
        mfc0  $k1, $CP0_CAUSE
        andi  $k1, $k1, 0x7c   	          /* read exception number */
        
        JUMP_TO_ADDR(mips_second_exception_handler) /* jump to real exception handler */
        nop        
        .set  pop

mips_second_exception_handler:
	SAVE_ALL                              /* save all needed registers */

	lw      $k0, exception_handlers($k1)  /* exception number is an offset in array */
	la      $ra, mips_restore_from_exception   /* return address for exit from exception */
	move    $a0, $sp                      /* Arg 0: saved regs. */
	JUMP_TO_REG($k0)                           /* Call C code. */
/*
 * the return sequence from the exception handler for the case of an
 * external exception (interrupt)
 */
    mfc0    $k0, $14                      /* get EPC in $k0 */
    rfe                                   /* return from exception */
    JUMP_TO_REG($k0)                      /* replace PC with the return address */
	nop
/*
 * the return sequence from the exception handler for the case of a
 * trap (including a syscall).
 */
    mfc0    $k0, $14                      /* get EPC in $k0 */
    addiu   $k0, 4                        /* make sure it points to next instruction */
    rfe                                   /* return from exception */
    JUMP_TO_REG($k0)                      /* replace PC with the return address */


mips_restore_from_exception:              /* label for exception return address */
	RESTORE_ALL                           /* restore all registers and return from exception */


typedef struct pt_regs {
	unsigned int reg[25];
	unsigned int gp; /* global pointer r28 */
	unsigned int sp; /* stack pointer r29 */
	unsigned int fp; /* frame pointer r30 */
	unsigned int ra; /* return address 31*/
	unsigned int lo;
	unsigned int hi;
	unsigned int cp0_status;
	unsigned int pc;
}pt_regs_t;



#define SAVE_ALL                    \
		LONG_ADDI $sp, -PT_SIZE     \
		SAVE_SOME                   \
		SAVE_AT                     \
		SAVE_TEMP                   \
		SAVE_STATIC                 \



        .section ".bss", "aw"
        .align 4 # 4 words = 16 bytes
pok_stack:
		.global pok_stack_end
        .space 8 * 1024
pok_stack_end:

pok_save_area:
        .space 4 * 8



#define SAVE_AT                                                     \
                 .set    push                                       \
                 .set    noat                                       \
                 sw  $1, OFFSETOF_jet_interrupt_context_R1(sp)      \
                 .set    pop                                        \
                                        
#define SAVE_TEMP                                                   \
                 mfhi    v1                                         \
                 sw  $10, OFFSETOF_jet_interrupt_context_R10(sp)    \
                 sw  $11, OFFSETOF_jet_interrupt_context_R11(sp)    \
                 sw  $12, OFFSETOF_jet_interrupt_context_R12(sp)    \
                 sw  v1, OFFSETOF_jet_interrupt_context_HI(sp)      \
                 mflo    v1                                         \
                 sw  $13, OFFSETOF_jet_interrupt_context_R13(sp)    \
                 sw  $14, OFFSETOF_jet_interrupt_context_R14(sp)    \
                 sw  $15, OFFSETOF_jet_interrupt_context_R15(sp)    \
                 sw  $24, OFFSETOF_jet_interrupt_context_R24(sp)    \
                 sw  v1, OFFSETOF_jet_interrupt_context_LO(sp)      \

 
#define SAVE_STATIC                                                 \
                 sw  $16, OFFSETOF_jet_interrupt_context_R16(sp)    \
                 sw  $17, OFFSETOF_jet_interrupt_context_R17(sp)    \
                 sw  $18, OFFSETOF_jet_interrupt_context_R18(sp)    \
                 sw  $19, OFFSETOF_jet_interrupt_context_R19(sp)    \
                 sw  $20, OFFSETOF_jet_interrupt_context_R20(sp)    \
                 sw  $21, OFFSETOF_jet_interrupt_context_R21(sp)    \
                 sw  $22, OFFSETOF_jet_interrupt_context_R22(sp)    \
                 sw  $23, OFFSETOF_jet_interrupt_context_R23(sp)    \
                 sw  $30, OFFSETOF_jet_interrupt_context_R30(sp)    \
                                        

#define SAVE_SOME                                                                               \
                 .set    push                                                                   \
                 .set    noat                                                                   \
                 .set    reorder                                                                \
                 mfc0    k0, CP0_STATUS                                                         \
                 sll     k0, 3           /* extract cu0 bit */                                  \
                 .set    noreorder                                                              \
                 bltz    k0, 8f                                                                 \
                  move   k1, sp                                                                 \
                 .set    reorder                                                                \
                 /* Called from user mode, new stack. */                                        \
                 get_saved_sp                                                                   \
 #ifndef CONFIG_CPU_DADDI_WORKAROUNDS                                                           \
 8:              move    k0, sp                                                                 \
                 subu sp, k1, FRAME_SIZE                                                        \
 #else                                                                                          \
                 .set    at=k0                                                                  \
 8:              subu k1, FRAME_SIZE                                                            \
                 .set    noat                                                                   \
                 move    k0, sp                                                                 \
                 move    sp, k1                                                                 \
 #endif                                                                                         \
                 sw  k0, OFFSETOF_jet_interrupt_context_R29(sp)                                 \
                 sw  $3, OFFSETOF_jet_interrupt_context_R3(sp)                                  \
                 /*                                                                             \
                  * We must  save $0, because                                                   \
                  * the FPU emulator and gdb remote debug stub                                  \
                  * need it to operate correctly                                                \
                  */                                                                            \
                 sw  $0, OFFSETOF_jet_interrupt_context_R0(sp)                                  \
                 mfc0    v1, CP0_STATUS                                                         \
                 sw  $2, OFFSETOF_jet_interrupt_context_R2(sp)                                  \
                 sw  v1, OFFSETOF_jet_interrupt_context_STATUS(sp)                              \
                 sw  $4, OFFSETOF_jet_interrupt_context_R4(sp)                                  \
                 mfc0    v1, CP0_CAUSE                                                          \
                 sw  $5, OFFSETOF_jet_interrupt_context_R5(sp)                                  \
                 sw  v1, OFFSETOF_jet_interrupt_context_CAUSE(sp)                               \
                 sw  $6, OFFSETOF_jet_interrupt_context_R6(sp)                                  \
                 mfc0    v1, CP0_EPC                                                            \
                 sw  $7, OFFSETOF_jet_interrupt_context_R7(sp)                                  \
 #ifdef CONFIG_64BIT                                                                            \
                 sw  $8, OFFSETOF_jet_interrupt_context_R8(sp)                                  \
                 sw  $9, OFFSETOF_jet_interrupt_context_R9(sp)                                  \
 #endif                                                                                         \
                 sw  v1, OFFSETOF_jet_interrupt_context_EPC(sp)                                 \
                 sw  $25, OFFSETOF_jet_interrupt_context_R25(sp)                                \
                 sw  $28, OFFSETOF_jet_interrupt_context_R28(sp)                                \
                 sw  $31, OFFSETOF_jet_interrupt_context_R31(sp)                                \
                 ori     $28, sp, _THREAD_MASK                                                  \
                 xori    $28, _THREAD_MASK                                                      \
                 .set    pop                                                                    \
                                                                    
                                                                    
#define SAVE_ALL                        \                           
                 SAVE_SOME              \                           
                 SAVE_AT                \
                 SAVE_TEMP              \
                 SAVE_STATIC            \
                                        

#define RESTORE_AT                                                  \
                 .set    push                                       \
                 .set    noat                                       \
                 lw  $1,  OFFSETOF_jet_interrupt_context_R1(sp)     \
                 .set    pop                                        \
                                        
    
#define RESTORE_TEMP                                                \
                 lw  $24, OFFSETOF_jet_interrupt_context_LO(sp)     \
                 MTlo($24)                                          \
                 lw  $24, OFFSETOF_jet_interrupt_context_HI(sp)     \
                 MTHi($24)                                          \
                 lw  $8, OFFSETOF_jet_interrupt_context_R8(sp)      \
                 lw  $9, OFFSETOF_jet_interrupt_context_R9(sp)      \
                 lw  $10, OFFSETOF_jet_interrupt_context_R10(sp)    \
                 lw  $11, OFFSETOF_jet_interrupt_context_R11(sp)    \
                 lw  $12, OFFSETOF_jet_interrupt_context_R12(sp)    \
                 lw  $13, OFFSETOF_jet_interrupt_context_R13(sp)    \
                 lw  $14, OFFSETOF_jet_interrupt_context_R14(sp)    \
                 lw  $15, OFFSETOF_jet_interrupt_context_R15(sp)    \
                 lw  $24, OFFSETOF_jet_interrupt_context_R24(sp)    \
 
#define RESTORE_STATIC                                              \
                 lw  $16, OFFSETOF_jet_interrupt_context_R16(sp)    \
                 lw  $17, OFFSETOF_jet_interrupt_context_R17(sp)    \
                 lw  $18, OFFSETOF_jet_interrupt_context_R18(sp)    \
                 lw  $19, OFFSETOF_jet_interrupt_context_R19(sp)    \
                 lw  $20, OFFSETOF_jet_interrupt_context_R20(sp)    \
                 lw  $21, OFFSETOF_jet_interrupt_context_R21(sp)    \
                 lw  $22, OFFSETOF_jet_interrupt_context_R22(sp)    \
                 lw  $23, OFFSETOF_jet_interrupt_context_R23(sp)    \
                 lw  $30, OFFSETOF_jet_interrupt_context_R30(sp)    \
                                        
                                        
#define RESTORE_SOME                                                                    \
                 .set    push                                                           \
                 .set    reorder                                                        \
                 .set    noat                                                           \
                 mfc0    a0, CP0_STATUS                                                 \
                 ori     a0, STATMASK                                                   \
                 xori    a0, STATMASK                                                   \
                 MTC0(a0, CP0_STATUS)                                                   \
                 li      v1, ST0_CU1 | ST0_FR | ST0_IM                                  \
                 and     a0, v1                                                         \
                 lw  v0, OFFSETOF_jet_interrupt_context_STATUS(sp)                      \
                 nor     v1, $0, v1                                                     \
                 and     v0, v1                                                         \
                 or      v0, a0                                                         \
                 MTC0(v0, CP0_STATUS)                                                   \
                 lw  v1, OFFSETOF_jet_interrupt_context_EPC(sp)                         \
                 MTC0(v1, CP0_EPC)                                                      \
                 lw  $31, OFFSETOF_jet_interrupt_context_R31(sp)                        \
                 lw  $28, OFFSETOF_jet_interrupt_context_R28(sp)                        \
                 lw  $25, OFFSETOF_jet_interrupt_context_R25(sp)                        \
 #ifdef CONFIG_64BIT                                                                    \
                 lw  $8, OFFSETOF_jet_interrupt_context_R8(sp)                          \
                 lw  $9, OFFSETOF_jet_interrupt_context_R9(sp)                          \
 #endif                                                                                 \
                 lw  $7,  OFFSETOF_jet_interrupt_context_R7(sp)                         \
                 lw  $6,  OFFSETOF_jet_interrupt_context_R6(sp)                         \
                 lw  $5,  OFFSETOF_jet_interrupt_context_R5(sp)                         \
                 lw  $4,  OFFSETOF_jet_interrupt_context_R4(sp)                         \
                 lw  $3,  OFFSETOF_jet_interrupt_context_R3(sp)                         \
                 lw  $2,  OFFSETOF_jet_interrupt_context_R2(sp)                         \
                 .set    pop                                                            \

 
#define RESTORE_SP_AND_RET                                          \
                 lw  sp, OFFSETOF_jet_interrupt_context_R29(sp)     \
                 .set    arch=r4000                                 \ ??
                 ERET_AND_NOP                                       \
                 .set    mips0                                      \ ??

 
 
#define RESTORE_SP                                              \
                 lw  sp, OFFSETOF_jet_interrupt_context_R29(sp) \

 
#define RESTORE_ALL                 \
                 RESTORE_TEMP       \
                 RESTORE_STATIC     \
                 RESTORE_AT         \
                 RESTORE_SOME       \
                 RESTORE_SP         \

 
#define RESTORE_ALL_AND_RET         \
             RESTORE_TEMP           \
             RESTORE_STATIC         \
             RESTORE_AT             \
             RESTORE_SOME           \
             RESTORE_SP_AND_RET     \
 
#define CP0_INDEX         $0
#define CP0_RANDOM        $1
#define CP0_ENTRYLO0      $2
#define CP0_ENTRYLO1      $3
#define CP0_CONF          $3
#define CP0_CONTEXT       $4
#define CP0_PAGEMASK      $5
#define CP0_WIRED         $6
#define CP0_INFO          $7
#define CP0_BADVADDR      $8
#define CP0_COUNT         $9
#define CP0_ENTRYHI       $10
#define CP0_COMPARE       $11
#define CP0_STATUS        $12
#define CP0_CAUSE         $13
#define CP0_EPC           $14
#define CP0_PRID          $15
#define CP0_CONFIG        $16
#define CP0_LLADDR        $17
#define CP0_WATCHLO       $18
#define CP0_WATCHHI       $19
#define CP0_XCONTEXT      $20
#define CP0_FRAMEMASK     $21
#define CP0_DIAGNOSTIC    $22
#define CP0_DEBUG         $23
#define CP0_DEPC          $24
#define CP0_PERFORMANCE   $25
#define CP0_ECC           $26
#define CP0_CACHEERR      $27
#define CP0_TAGLO         $28
#define CP0_TAGHI         $29
#define CP0_ERROREPC      $30
#define CP0_DESAVE        $31
#define PT_SIZE           176
#define JUMP_TO_REG(reg)   \
        jr  reg            \
        nop                \
        nop                \

#define JUMP_TO_ADDR(addr) \
        j   addr           \
        nop                \
        nop                \

#define JUMP_AND_LINK(addr) \
        jal   addr          \
        nop                 \
        nop                 \
        
#define ERET_AND_NOP      \
        eret              \
        nop               \
        nop               \

#define MTC1(arg1, arg2)  \
        mtc1 arg1, arg2   \
        nop               \
        nop               \

#define MTHi(arg1)        \
        mthi arg1         \
        nop               \
        nop               \
        
#define MTlo(arg1)        \
        mtlo arg1         \
        nop               \
        nop               \
        
#define MTC0(arg1, arg2)  \
        mtc0 arg1, arg2   \
        nop               \
        nop               \                



#define MTC0_sel1(arg1, arg2)  \
        mtc0 arg1, arg2, 1     \
        nop                    \
        nop                    \
