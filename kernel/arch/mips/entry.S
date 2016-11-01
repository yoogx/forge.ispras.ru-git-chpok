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

    SPRG0  - 
    DESAVE - kernel stack pointer
    SPRG2  - scratch register
    SPRG3  - scratch register
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
//rerwite           add $r7, $r3, $sp
   //rerwite        addi $r7, $r7, -FRAME_SIZE
   //rerwite        li $r8, 0
//rerwite        stwux $r8, $sp, $r7
         MTC0($sp, CP0_DESAVE)
        // Prepare user stack, STATUS and EPC for eret
  //rerwite         addi $sp, $r6, -16
    //rerwite       MTC0($r5, $14)        
    //rerwite       /*Do $r9 = CP0_STATUS_CU1 | CP0_STATUS_FR | CP0_STATUS_IE*/
    //rerwite       li   $r9, CP0_STATUS_CU1                              
    //rerwite       addi $r9, CP0_STATUS_FR                            
    //rerwite       addi $r9, CP0_STATUS_IE 
/*
 *EE	Enable/disable external interrupts  
 *
 *IP  Exception prefix. The setting of this bit specifies whether an exception vector offset
 *    is prepended with Fs or 0s. In the following description, nnnnn is the offset of the
 *    exception vector. See Table 6-2.
 *    0 Exceptions are vectored to the physical address 0x000n_nnnn in 32-bit
 *    implementations and 0x0000_0000_000n_nnnn in 64-bit implementations.
 *    1 Exceptions are vectored to the physical address 0xFFFn_nnnn in 32-bit
 *    implementations and 0x0000_0000_FFFn_nnnn in 64-bit implementations.
 *    In most systems, IP is set to 1 during system initialization, and then cleared to 0
 *    when initialization is complete.
 *FP  Floating-point available
 *    0 The processor prevents dispatch of floating-point instructions, including
 *    floating-point loads, stores, and moves.
 *    1 The processor can execute floating-point instructions.
 *PR  Privilege level
 *    0 The processor can execute both user- and supervisor-level instructions.
 *    1 The processor can only execute user-level instructions.
 */
  //rerwite         MTC0($r9, CP0_STATUS)

        // Nullify other registers (0, 2-28, 30-31)
        move $0,  $zero
        move $2,  $zero
        move $3,  $zero
        move $4,  $zero
        move $5,  $zero
        move $6,  $zero
        move $7,  $zero
        move $8,  $zero
        move $9,  $zero
        move $10, $zero
        move $11, $zero
        move $12, $zero
        move $13, $zero
        move $14, $zero
        move $15, $zero
        move $16, $zero
        move $17, $zero
        move $18, $zero
        move $19, $zero
        move $20, $zero
        move $21, $zero
        move $22, $zero
        move $23, $zero
        move $24, $zero
        move $25, $zero
        move $26, $zero
        move $27, $zero
        move $28, $zero
        move $30, $zero
        move $31, $zero

        mfc0 $k0, CP0_EPC
        ERET_AND_NOP
        JUMP_TO_REG($k0)

/*
 * Store registers to interrupt frame.
 * This is GDB-unaware version.
 *
 * Before the call:
 *
 * - r1 is adjusted to the beginning of the interrupt frame.
 * - cr is already stored to the frame. not need it in MIPS
 * - r3 contains old r1.
 *
 * After the call:
 * - srr0, srr1, r3, r4 are stored outside of the macro
 * - sprg3 contains old r3.
 * - sprg2 contains old r4.
 */


.macro SAVE_AT                                                   
                 .set    push                                    
                 .set    noat                                    
                 sw   $1, OFFSETOF_jet_interrupt_context_r1($sp)   
                 .set    pop                                     
.endm
                                                                 
.macro SAVE_TEMP                                                 
                 mfhi $3                                       
                 sw   $10, OFFSETOF_jet_interrupt_context_r10($sp) 
                 sw   $11, OFFSETOF_jet_interrupt_context_r11($sp) 
                 sw   $12, OFFSETOF_jet_interrupt_context_r12($sp) 
                 sw   $3,  OFFSETOF_jet_interrupt_context_HI($sp)   
                 mflo $3                                       
                 sw   $13, OFFSETOF_jet_interrupt_context_r13($sp) 
                 sw   $14, OFFSETOF_jet_interrupt_context_r14($sp) 
                 sw   $15, OFFSETOF_jet_interrupt_context_r15($sp) 
                 sw   $24, OFFSETOF_jet_interrupt_context_r24($sp) 
                 sw   $3,  OFFSETOF_jet_interrupt_context_LO($sp)   
.endm
                                                                 
                                                                 
.macro SAVE_STATIC                                               
                 sw   $16, OFFSETOF_jet_interrupt_context_r16($sp) 
                 sw   $17, OFFSETOF_jet_interrupt_context_r17($sp) 
                 sw   $18, OFFSETOF_jet_interrupt_context_r18($sp) 
                 sw   $19, OFFSETOF_jet_interrupt_context_r19($sp) 
                 sw   $20, OFFSETOF_jet_interrupt_context_r20($sp) 
                 sw   $21, OFFSETOF_jet_interrupt_context_r21($sp) 
                 sw   $22, OFFSETOF_jet_interrupt_context_r22($sp) 
                 sw   $23, OFFSETOF_jet_interrupt_context_r23($sp) 
                 sw   $30, OFFSETOF_jet_interrupt_context_r30($sp) 
.endm
                                        

.macro SAVE_SOME                                                  
                 .set    push                                     
                 .set    noat                                     
                 move $k0, $sp                                   
                 sw   $k0, OFFSETOF_jet_interrupt_context_r29($sp)   
                 sw   $3,  OFFSETOF_jet_interrupt_context_r3($sp)    
                 /*                                               
                  * We must  save $0, because                     
                  * the FPU emulator and gdb remote debug stub    
                  * need it to operate correctly                  
                  */                                              
                 sw   $0,  OFFSETOF_jet_interrupt_context_r0($sp)    
                 mfc0 $3,  CP0_STATUS                           
                 sw   $2,  OFFSETOF_jet_interrupt_context_r2($sp)    
                 sw   $3,  OFFSETOF_jet_interrupt_context_STATUS($sp)
                 sw   $4,  OFFSETOF_jet_interrupt_context_r4($sp)    
                 mfc0 $3,  CP0_CAUSE                            
                 sw   $5,  OFFSETOF_jet_interrupt_context_r5($sp)    
                 sw   $3,  OFFSETOF_jet_interrupt_context_CAUSE($sp) 
                 sw   $6,  OFFSETOF_jet_interrupt_context_r6($sp)    
                 mfc0 $3,  CP0_EPC                              
                 sw   $7,  OFFSETOF_jet_interrupt_context_r7($sp)    
                 sw   $8,  OFFSETOF_jet_interrupt_context_r8($sp)    
                 sw   $9,  OFFSETOF_jet_interrupt_context_r9($sp)    
                 sw   $3,  OFFSETOF_jet_interrupt_context_EPC($sp)   
                 sw   $25, OFFSETOF_jet_interrupt_context_r25($sp)  
                 sw   $28, OFFSETOF_jet_interrupt_context_r28($sp)  
                 sw   $31, OFFSETOF_jet_interrupt_context_r31($sp)  
                 .set    pop                                      
.endm
                                                                  
                                                                  
.macro SAVE_ALL                      
                 SAVE_SOME           
                 SAVE_AT             
                 SAVE_TEMP           
                 SAVE_STATIC         
.endm
                                        

.macro RESTORE_AT                                                 
                 .set    push                                     
                 .set    noat                                     
                 lw   $1,  OFFSETOF_jet_interrupt_context_r1($sp)   
                 .set    pop                                      
.endm
                                                                  
                                                                  
.macro RESTORE_TEMP                                               
                 lw   $24, OFFSETOF_jet_interrupt_context_LO($sp)   
                 MTlo($24)                                        
                 lw   $24, OFFSETOF_jet_interrupt_context_HI($sp)   
                 MTHi($24)                                        
                 lw   $8,  OFFSETOF_jet_interrupt_context_r8($sp)    
                 lw   $9,  OFFSETOF_jet_interrupt_context_r9($sp)    
                 lw   $10, OFFSETOF_jet_interrupt_context_r10($sp)  
                 lw   $11, OFFSETOF_jet_interrupt_context_r11($sp)  
                 lw   $12, OFFSETOF_jet_interrupt_context_r12($sp)  
                 lw   $13, OFFSETOF_jet_interrupt_context_r13($sp)  
                 lw   $14, OFFSETOF_jet_interrupt_context_r14($sp)  
                 lw   $15, OFFSETOF_jet_interrupt_context_r15($sp)  
                 lw   $24, OFFSETOF_jet_interrupt_context_r24($sp)  
.endm
                                                                  
.macro RESTORE_STATIC                                             
                 lw   $16, OFFSETOF_jet_interrupt_context_r16($sp)  
                 lw   $17, OFFSETOF_jet_interrupt_context_r17($sp)  
                 lw   $18, OFFSETOF_jet_interrupt_context_r18($sp)  
                 lw   $19, OFFSETOF_jet_interrupt_context_r19($sp)  
                 lw   $20, OFFSETOF_jet_interrupt_context_r20($sp)  
                 lw   $21, OFFSETOF_jet_interrupt_context_r21($sp)  
                 lw   $22, OFFSETOF_jet_interrupt_context_r22($sp)  
                 lw   $23, OFFSETOF_jet_interrupt_context_r23($sp)  
                 lw   $30, OFFSETOF_jet_interrupt_context_r30($sp)  
.endm
                                        
                                        
.macro RESTORE_SOME                                               
                 .set    push                                     
                 .set    noat                                     
                 /*Do $k0 = CP0_STATUS_CU1 | CP0_STATUS_FR | CP0_STATUS_IE*/
                 li   $k0, CP0_STATUS_CU1                              
                 addi $k0, CP0_STATUS_FR                            
                 addi $k0, CP0_STATUS_IE                                           
                 
                 lw   $2,  OFFSETOF_jet_interrupt_context_STATUS($sp)                                
                 add  $2, $2, $k0
                 MTC0($2,  CP0_STATUS)                             
                 lw   $3,  OFFSETOF_jet_interrupt_context_EPC($sp)   
                 MTC0($3,  CP0_EPC)                                
                 lw   $31, OFFSETOF_jet_interrupt_context_r31($sp)  
                 lw   $28, OFFSETOF_jet_interrupt_context_r28($sp)  
                 lw   $25, OFFSETOF_jet_interrupt_context_r25($sp)  
                 lw   $8,  OFFSETOF_jet_interrupt_context_r8($sp)    
                 lw   $9,  OFFSETOF_jet_interrupt_context_r9($sp)    
                 lw   $7,  OFFSETOF_jet_interrupt_context_r7($sp)   
                 lw   $6,  OFFSETOF_jet_interrupt_context_r6($sp)   
                 lw   $5,  OFFSETOF_jet_interrupt_context_r5($sp)   
                 lw   $4,  OFFSETOF_jet_interrupt_context_r4($sp)   
                 lw   $3,  OFFSETOF_jet_interrupt_context_r3($sp)   
                 lw   $2,  OFFSETOF_jet_interrupt_context_r2($sp)   
                 .set    pop                                      
.endm

 
.macro RESTORE_ALL                
                 RESTORE_TEMP     
                 RESTORE_STATIC   
                 RESTORE_AT       
                 RESTORE_SOME     
.endm
 
 
.macro SAVE_REGS_COMMON_BASE
    SAVE_ALL
.endm

#ifndef POK_NEEDS_GDB
#define SAVE_REGS_COMMON SAVE_REGS_COMMON_BASE
#else
.macro SAVE_REGS_COMMON
        SAVE_REGS_COMMON_BASE
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
    RESTORE_ALL
.endm

#ifndef POK_NEEDS_GDB
#define RESTORE_REGS_COMMON RESTORE_REGS_COMMON_BASE
#else
.macro RESTORE_REGS_COMMON
        RESTORE_REGS_COMMON_BASE
.endm
#endif /* POK_NEEDS_GDB */

/*
 * Common part of interrupt epilogue.
 *
 * Prepare everything except EPC, CP0_STATUS, r3, r4.
 */
.macro EXCEPTION_EPILOGUE_COMMON
        RESTORE_REGS_COMMON
        /* Determine whether we return to user space. */
   //rerwite        lw $r4, OFFSETOF_jet_interrupt_context_STATUS($sp)
     //rerwite   andi.   $r4,$r4,MSR_PR
    //rerwite    beq     1f
        /* Return to user - need to set SPRG1. */
    //rerwite    lw $r4, OFFSETOF_jet_interrupt_context_cr($sp) Restore everything else from interrupt frame.
    //rerwite    mtcr $r4 //Don't need it in MIPS
        MTC0($sp, CP0_DESAVE)
     //rerwite   b 2f
1:
        /* Return to kernel. */
 //rerwite       lw $r4, OFFSETOF_jet_interrupt_context_cr($sp) Restore everything else from interrupt frame.
 //rerwite       mtcr $r4 //Don't need it in MIPS
2:
   //rerwite        move $sp, $r3
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
   //rerwite     mfcr    $r3
        /* Enable floating point bit in status */
        mfc0 $k0, CP0_STATUS
        ori $k0, $k0, STATUS_CU1
        MTC0($k0, CP0_STATUS)
        /* Check whether kernel stack is usable. */
    //rerwite    mfc0    $r4, CP0_DESAVE
    //rerwite    andi.   $r4, $r4, 1 /* Valid stack is always 16-byte aligned. */
     //rerwite   beq     1f
        /* Come from kernel and kernel stack is ready */
    //rerwite    stwu $sp, -FRAME_SIZE-JET_STACK_RED_ZONE($sp)
    //rerwite       sw $r3, OFFSETOF_jet_interrupt_context_cr($sp)
        /* Store lr to previous backchain. */
    //rerwite    mflr $r3
    //rerwite       sw $r3, FRAME_SIZE + JET_STACK_RED_ZONE + OFFSETOF_jet_stack_frame_lr($sp)
    //rerwite    addi $r3, $sp, FRAME_SIZE + JET_STACK_RED_ZONE
    //rerwite    b       2f
1:      /* Come from user or interrupted handler hasn't adjust stack yet. */
     //rerwite   mfc0    $r4, CP0_DESAVE
     //rerwite   mtcr $r3 /* Restore cr, so possible highlevel interrupt will see the same as we.*/
        /* Store cr before updating stack pointer! It can be overwritting by highlevel interrupt... but to the same value(see above). */
    //rerwite       sw $r3, OFFSETOF_jet_interrupt_context_cr($r4)
    //rerwite       move $r3, $sp
    //rerwite       move $sp, $r4 /* Back chain is already set to 0. */
        /* Mark kernel stack as ready for highlevel interrupts. */
    //rerwite       li $r4, 1
    //rerwite    MTC0($r4, CP0_DESAVE)
2:      SAVE_REGS_COMMON
.endm

/* Return from non-critical interrupt. */
        .globl pok_arch_rfi
pok_arch_rfi:
        /* r4 <-> sprg2, r3 <-> sprg3 */
    //rerwite      lw     $r4,OFFSETOF_jet_interrupt_context_4($sp)
    //rerwite    mtsprg  2, $r4
     //rerwite      lw     $r3,OFFSETOF_jet_interrupt_context_3($sp)
    //rerwite    mtsprg  3, $r3

    //TODO можно убирать:
        /* srr0->srr0, srr1->srr1. */
    //rerwite       lw     $r4, OFFSETOF_jet_interrupt_context_STATUS($sp)
    //rerwite       MTC0($r4, CP0_STATUS)
    //rerwite       lw     $r3, OFFSETOF_jet_interrupt_context_EPC($sp)
    //rerwite       MTC0($r3, CP0_EPC)

        EXCEPTION_EPILOGUE_COMMON

    //rerwite    mfsprg $r3, 3
     //rerwite   mfsprg $r4, 2
        
    /*TODO: Condition*/
        b 1b
    
/*
 * the return sequence from the exception handler for the case of an
 * external exception (interrupt)
 */
1:      mfc0    $k0, CP0_EPC                  /* get EPC in $k0 */
        ERET_AND_NOP                          /* return from exception */
        JUMP_TO_REG($k0)                      /* replace PC with the return address */
/*
 * the return sequence from the exception handler for the case of a
 * trap (including a syscall).
 */
2:      mfc0    $k0, CP0_EPC                  /* get EPC in $k0 */
        addiu   $k0, 4                        /* make sure it points to next instruction */
        ERET_AND_NOP                          /* return from exception */
        JUMP_TO_REG($k0)                      /* replace PC with the return address */

/* Exception prologue for non-critical interrupts. */
.macro EXCEPTION_PROLOGUE
        /* r3 <-> sprg3, r4 <-> sprg2*/
     //rerwite   mtsprg  3, $r3
     //rerwite   mtsprg  2, $r4
        EXCEPTION_PROLOGUE_COMMON

    //TODO можно убирать так как это сохранение есть в EXEPTION_PROLOGUE_COMMON
        /* srr0<-srr0, srr1<-srr1. */
//rerwite           mfc0   $r3, CP0_EPC
//rerwite           sw     $r3, OFFSETOF_jet_interrupt_context_EPC($sp)
//rerwite           mfc0   $r4, CP0_STATUS
//rerwite           sw     $r4, OFFSETOF_jet_interrupt_context_STATUS($sp)

    //rerwite    mfsprg  $r4, s2
  //rerwite         sw     $r4, OFFSETOF_jet_interrupt_context_4($sp)
     //rerwite   mfsprg  $r3, 3
   //rerwite        sw     $r3, OFFSETOF_jet_interrupt_context_3($sp)
.endm
        
        
/* Return from debug interrupt. */
        .globl pok_arch_rfdi_for_debug
pok_arch_rfdi_for_debug:
        /* $5 <-> sprg2, $4 <-> sprg3. TODO: other temporary locations should be used. */
   //rerwite        lw      $5, OFFSETOF_jet_interrupt_context_4($sp)
   //rerwite     mtsprg  2, $5
    //rerwite       lw      $3, OFFSETOF_jet_interrupt_context_3($sp)
   //rerwite     mtsprg  2, $4

    //TODO нужно оставить, так как эти регистры мы не трогаем в загрузке
        /* EPC->DEPC, CAUSE->DEBUG. */
    //rerwite       lw      $5, OFFSETOF_jet_interrupt_context_CAUSE($sp)
    //rerwite       MTC0($5, CP0_DEBUG)
    //rerwite       lw      $4, OFFSETOF_jet_interrupt_context_EPC($sp)
    //rerwite       MTC0($4, CP0_DEPC)

        EXCEPTION_EPILOGUE_COMMON

    //rerwite    mfsprg  $5, 2
     //rerwite   mfsprg  $4, 3

        mfc0   $k0, CP0_DEPC
        DERET_AND_NOP
        JUMP_TO_REG($k0)        

.macro EXCEPTION_PROLOGUE_DEBUG
        /* r3 <-> sprg3, r4 <-> sprg2. TODO: use different temporaries. */
    //rerwite    mtsprg  3, $r3
    //rerwite    mtsprg  2, $r4
        EXCEPTION_PROLOGUE_COMMON

    //TODO нужно оставить, так как эти регистры мы не трогаем в сохранении
        /* EPC<-DEPC, CAUSE<-DEBUG. */
   //rerwite        mfc0    $r3, CP0_DEPC
   //rerwite        sw      $r3, OFFSETOF_jet_interrupt_context_EPC($sp)
   //rerwite        mfc0    $r4, CP0_DEBUG
    //rerwite       sw      $r4, OFFSETOF_jet_interrupt_context_CAUSE($sp)

     //rerwite   mfsprg  $r3, 3
    //rerwite       sw      $r3, OFFSETOF_jet_interrupt_context_3($sp)
    //rerwite    mfsprg  $r4, 2
    //rerwite       sw      $r4, OFFSETOF_jet_interrupt_context_4($sp)
.endm

        . = base + 0x300
        .global pok_trap_addr
pok_trap_addr:
        .global pok_trap
pok_trap:
        break
        JUMP_TO_REG($ra)




failed: b failed


        . = base + 0x350
        .globl _pok_reset
system_reset_0100:
_pok_reset:
        /*write $4(devtree addr) to devtree_address global variable*/
        //TODO fix conflict:
        //lis $sp, devtree_address

        sw  $4, 0($sp)

        /* Setup stack */
        //TODO fix conflict:
        //lis $sp, pok_stack_end-4

        /* Mark kernel stack as ready for interrupts. */
    //rerwite       li $r4, 1
     //rerwite   MTC0($r4, CP0_DESAVE)


        JUMP_AND_LINK(pok_setup_interrupts)

        /* Clear back chain */
     //rerwite   xor $r0, $r0, $r0
    //rerwite       sw  $r0, 4($sp)

        /* TODO: copy .sdata, clear .bss */
        /* Copy .data */
/*
 *         lis $r3, (__data_start-4)@h
 *         ori $r3, $r3, (__data_start-4)@l
 *         lis $r4, (__data_load-4)@h
 *         ori $r4, $r4, (__data_load-4)@l
 *         lis $r5, __data_end@h
 *         ori $r5, $r5, __data_end@l
 *         cmplw $r3, $r5
 *         bge 2f
 * 1:      lwzu $r6, 4($r4)
 *         stwu $r6, 4($r3)
 *         cmplw $r3, $r5
 *         blt 1b
 * 2:
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
        move $4, $sp
        JUMP_AND_LINK(pok_int_critical_input)
        JUMP_TO_ADDR(pok_arch_rfi)

    /* TODO: shouldn't machine check interrupt level should be used there? */
    START_EXCEPTION(pok_int_machine_check)
        EXCEPTION_PROLOGUE
        move $4, $sp
        JUMP_AND_LINK(pok_int_machine_check)
        JUMP_TO_ADDR(pok_arch_rfi)

    START_EXCEPTION(pok_int_data_storage)
        EXCEPTION_PROLOGUE
        move $4, $sp

        mfc0 $5, CP0_BadVAddr
        mfc0 $6, CP0_CAUSE
     //rerwite   mfspr $r4, SPRN_DEAR // DEAR - faulting address    DEAR = BadVAddr
     //rerwite   mfspr $r5, SPRN_ESR // ESR - exception syndrome    ESR  = Cause.ExcCode

        JUMP_AND_LINK(pok_int_data_storage)
        JUMP_TO_ADDR(pok_arch_rfi)

    START_EXCEPTION(pok_int_inst_storage)
        EXCEPTION_PROLOGUE
        move $4, $sp
        mfc0 $5, CP0_BadVAddr
        mfc0 $6, CP0_CAUSE
    //rerwite     mfspr $r4, SPRN_DEAR // DEAR - faulting address    DEAR = BadVAddr
    //rerwite    mfspr $r5, SPRN_ESR // ESR - exception syndrome    ESR  = Cause.ExcCode

        JUMP_AND_LINK(pok_int_inst_storage)
        JUMP_TO_ADDR(pok_arch_rfi)

    START_EXCEPTION(pok_int_ext_interrupt)
        EXCEPTION_PROLOGUE
        move $4, $sp
        JUMP_AND_LINK(pok_int_ext_interrupt)
        JUMP_TO_ADDR(pok_arch_rfi)

    START_EXCEPTION(pok_int_alignment)
        EXCEPTION_PROLOGUE
        move $4, $sp

        JUMP_AND_LINK(pok_int_alignment)
        JUMP_TO_ADDR(pok_arch_rfi)

    START_EXCEPTION(pok_int_program)
        EXCEPTION_PROLOGUE
        move $4, $sp

        JUMP_AND_LINK(pok_int_program)
        JUMP_TO_ADDR(pok_arch_rfi)

    START_EXCEPTION(pok_int_fp_unavail)
        EXCEPTION_PROLOGUE
        move $4, $sp

        JUMP_AND_LINK(pok_int_fp_unavail)
        JUMP_TO_ADDR(pok_arch_rfi)

    START_EXCEPTION(pok_int_system_call)
        EXCEPTION_PROLOGUE
        move $4, $sp
        /* load system call arguments back from the interrupt frame */
/*
 *       lw      $r4,OFFSETOF_jet_interrupt_context_r3($sp)
 *       lw      $r5,OFFSETOF_jet_interrupt_context_r4($sp)                   //У нас только 4 аргумента передаваемые функции в регистрах лежат
 *       lw      $r6,OFFSETOF_jet_interrupt_context_r5($sp)
 *       lw      $r7,OFFSETOF_jet_interrupt_context_r6($sp)
 *       lw      $r8,OFFSETOF_jet_interrupt_context_r7($sp)
 *       lw      $r9,OFFSETOF_jet_interrupt_context_r8($sp)
 */
        JUMP_AND_LINK(pok_int_system_call) 
   //rewrite     sw      $r3,OFFSETOF_jet_interrupt_context_3($sp)

        JUMP_TO_ADDR(pok_arch_rfi)

    START_EXCEPTION(pok_int_decrementer)
        EXCEPTION_PROLOGUE
        move $4, $sp

        JUMP_AND_LINK(pok_int_decrementer) 
        JUMP_TO_ADDR(pok_arch_rfi)

    START_EXCEPTION(pok_int_interval_timer)
        EXCEPTION_PROLOGUE
        move $4, $sp

        JUMP_AND_LINK(pok_int_interval_timer)
        JUMP_TO_ADDR(pok_arch_rfi)

    START_EXCEPTION(pok_int_watchdog)
        EXCEPTION_PROLOGUE
        move $4, $sp

        JUMP_AND_LINK(pok_int_watchdog)
        JUMP_TO_ADDR(pok_arch_rfi)

    START_EXCEPTION(pok_int_data_tlb_miss)
        EXCEPTION_PROLOGUE
        move $4, $sp
        mfc0 $5, CP0_BadVAddr
        mfc0 $6, CP0_CAUSE
  //rerwite      mfspr $r4, SPRN_DEAR // DEAR - faulting address    DEAR = BadVAddr
  //rerwite      mfspr $r5, SPRN_ESR // ESR - exception syndrome    ESR  = Cause.ExcCode

        JUMP_AND_LINK(pok_int_data_tlb_miss)
        JUMP_TO_ADDR(pok_arch_rfi)

    START_EXCEPTION(pok_int_inst_tlb_miss)
        EXCEPTION_PROLOGUE
        move $4, $sp
        mfc0 $5, CP0_BadVAddr
        mfc0 $6, CP0_CAUSE
    //rerwite    mfspr $r4, SPRN_DEAR // DEAR - faulting address    DEAR = BadVAddr
    //rerwite    mfspr $r5, SPRN_ESR // ESR - exception syndrome    ESR  = Cause.ExcCode

        JUMP_AND_LINK(pok_int_inst_tlb_miss)
        JUMP_TO_ADDR(pok_arch_rfi)
        
    START_EXCEPTION(pok_int_debug)
        EXCEPTION_PROLOGUE_DEBUG

        JUMP_AND_LINK(pok_int_debug)
        JUMP_TO_ADDR(pok_arch_rfdi_for_debug)
 
    START_EXCEPTION(pok_int_none)
        EXCEPTION_PROLOGUE

        JUMP_AND_LINK(pok_int_debug)
        JUMP_TO_ADDR(pok_arch_rfdi_for_debug)


/*
 *  Adding addr of C part of exeption handler for each interrupt in array
 */

#define SET_IVOR(vector_number, vector_label)                       \
                li      $k0, vector_label##_asm;                    \
                sw      $k0, vector_number + expection_handlers ;   \
                sync


pok_setup_interrupts:
        /*Switch on using EBASE addr for exeption handler*/
        mfc0   $k0, CP0_STATUS
        li     $k1, CP0_STATUS_BEV
        or     $k0, $k0, $k1
        MTC0($k0, CP0_STATUS)
        /*Switch off using the special interrupt vector*/ 
        mfc0   $k0, CP0_CAUSE
        li     $k1, CP0_CAUSE_IV
        or     $k0, $k0, $k1
        xor    $k0, $k0, $k1
        MTC0($k0, CP0_CAUSE)
        li    $k0, mips_first_exeption_handler_asm
        MTC0_sel1($k0, CP0_EBASE)
        

/* 
 * pok_int_critical_input)
 * pok_int_machine_check) 
 * pok_int_data_storage)  
 * pok_int_inst_storage)  
 * pok_int_ext_interrupt) 
 * pok_int_alignment)     
 * pok_int_program)       
 * pok_int_fp_unavail)    
 * pok_int_system_call)   
 * pok_int_decrementer)   
 * pok_int_interval_timer)
 * pok_int_watchdog)      
 * pok_int_data_tlb_miss) 
 * pok_int_inst_tlb_miss) 
 * pok_int_debug)         
 * pok_int_debug)         
 * pok_int_debug)      
 */

        SET_IVOR(0,  pok_int_decrementer)   /*Прерывание                    INT Interrupt               */
        SET_IVOR(1,  pok_int_none)          /*Запись в защищенную страницу                              */
        SET_IVOR(2,  pok_int_inst_tlb_miss) /*Исключение в TLB (при чтении данных                       */
                                            /*                            или при выборке инструкции)   */
        SET_IVOR(3,  pok_int_data_tlb_miss) /*Исключение в TLB (при записи данных)                      */
        SET_IVOR(4,  pok_int_none)          /*Неправильный адрес при чтении  ADDRL Load from an illegal address   */
        SET_IVOR(5,  pok_int_none)          /*Неправильный адрес при записи  ADDRS Store to an illegal address    */
        SET_IVOR(6,  pok_int_none)          /*Ошибка шины при выборе команды IBUS Bus error on instruction fetch  */
        SET_IVOR(7,  pok_int_none)          /*Ошибка шины при чтении данных  DBUS Bus error on data reference     */
        SET_IVOR(8,  pok_int_system_call)   /*Команда SYSCALL                SYSCALL syscall instruction executed */
        SET_IVOR(9,  pok_int_debug)         /*Команда BREAK                  BKPT break instruction executed      */
        SET_IVOR(10, pok_int_none)         /*Нереализованная команда        RI Reserved instruction              */
        SET_IVOR(11, pok_int_none)         /*Недоступный сопроцессор                                   */
        SET_IVOR(12, pok_int_none)         /*Целочисленное переполнение     OVF Arithmetic overflow    */
        SET_IVOR(13, pok_int_debug)        /*Команды TRAP                                              */
        SET_IVOR(14, pok_int_fp_unavail) /*????*/  /*Исключение от блока вещественной арифметики       */
        SET_IVOR(15, pok_int_none)         /*Исключение при доступе в накристальную память             */    



        JUMP_TO_REG($ra)

/*This exeption handler is equal for each interrupt*/
mips_first_exeption_handler_asm:
        .set  push
        mfc0  $k1, CP0_CAUSE
        andi  $k1, $k1, 0x7c   	                   /* read exception number */
    	lw    $k0, exception_handlers($k1)         /* exception number is an offset in array */
        
    	JUMP_TO_REG($k0)                           /* Call asm code for this exeption. */
        .set  pop

        .section ".bss", "aw"
        .align 4 # 4 words = 16 bytes
pok_stack:
		.global pok_stack_end
        .space 8 * 1024
pok_stack_end:

pok_save_area:
        .space 4 * 8
