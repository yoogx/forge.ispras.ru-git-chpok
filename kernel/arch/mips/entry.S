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
mips_tlb_exeption_asm:
        mfc0    $k0, CP0_CAUSE
        andi    $k0, 0x7f
        li      $k1, 0x8
        beq     $k1, $k0, mips_first_exeption_handler_asm  /*TLBL*/
        li      $k1, 0xc  
        beq     $k1, $k0, mips_first_exeption_handler_asm  /*TLBS*/
reserved_0000:        
        /* fails */
1:      JUMP_TO_ADDR(1b) 

    . = base + 0x180
        la  $k0, mips_first_exeption_handler_asm
        jr  $k0
        
        
    . = base + 0x10000
        save1: .word  0
        save2: .word  0

#define FRAME_SIZE SIZEOF_jet_interrupt_context

        .globl ja_user_space_jump
ja_user_space_jump:
        /* a0 - kernel stack for return,
         * a1 - user space id,
         * a2 - entry point
         * a3 - user stack
         */
        // Prepare kernel stack for return
         subu   $8, $a0, $sp
         addi   $8, $8, -FRAME_SIZE
         li     $9, 0
        //equivalent stwux $9, $sp, $8 in powerpc 
         sw     $9, ($sp)
         addu   $sp, $sp, $8
         MTC0($sp, CP0_DESAVE)
        // Prepare user stack, STATUS and EPC for eret
         addi   $sp, $a3, -16
         MTC0($a2, $14)        
        /*Do CP0_STATUS_SETTINGS = CP0_STATUS_CU1 | CP0_STATUS_FR | CP0_STATUS_IE*/
         li     $10, CP0_STATUS_SETTINGS                              

        MTC0($10, CP0_STATUS)

        // Nullify other registers (0, 2-28, 30-31)
        move    $0,  $zero
        move    $2,  $zero
        move    $3,  $zero
        move    $4,  $zero
        move    $5,  $zero
        move    $6,  $zero
        move    $7,  $zero
        move    $8,  $zero
        move    $9,  $zero
        move    $10, $zero
        move    $11, $zero
        move    $12, $zero
        move    $13, $zero
        move    $14, $zero
        move    $15, $zero
        move    $16, $zero
        move    $17, $zero
        move    $18, $zero
        move    $19, $zero
        move    $20, $zero
        move    $21, $zero
        move    $22, $zero
        move    $23, $zero
        move    $24, $zero
        move    $25, $zero
        move    $26, $zero
        move    $27, $zero
        move    $28, $zero
        move    $30, $zero
        move    $31, $zero

        mfc0    $k0, CP0_EPC
        ERET_AND_NOP
        JUMP_TO_REG($k0)

/*
 * Store registers to interrupt frame.
 * This is GDB-unaware version.
 *
 * Before the call:
 *
 * - sp is adjusted to the beginning of the interrupt frame.
 * - cr is already stored to the frame. not need it in MIPS
 * - a0 contains old sp.
 *
 * After the call:
 * - EPC, STATUS, a0, a1 are stored outside of the macro
 * - save2 contains old a0.
 * - save1 contains old a1.
 */


.macro SAVE_AT                                                   
                 .set    push                                    
                 .set    noat                                    
                 sw     $1, OFFSETOF_jet_interrupt_context_r1($sp)   
                 .set    pop                                     
.endm
                                                                 
.macro SAVE_TEMP                                                 
                 mfhi   $3                                       
                 sw     $10, OFFSETOF_jet_interrupt_context_r10($sp) 
                 sw     $11, OFFSETOF_jet_interrupt_context_r11($sp) 
                 sw     $12, OFFSETOF_jet_interrupt_context_r12($sp) 
                 sw     $3,  OFFSETOF_jet_interrupt_context_hi($sp)   
                 mflo   $3                                       
                 sw     $13, OFFSETOF_jet_interrupt_context_r13($sp) 
                 sw     $14, OFFSETOF_jet_interrupt_context_r14($sp) 
                 sw     $15, OFFSETOF_jet_interrupt_context_r15($sp) 
                 sw     $24, OFFSETOF_jet_interrupt_context_r24($sp) 
                 sw     $3,  OFFSETOF_jet_interrupt_context_lo($sp) 
                 mfc1   $3,  CP1_FCCR
                 sw     $3,  OFFSETOF_jet_interrupt_context_FCCR($sp)  
                 mfc0   $3,  CP0_BadVAddr
                 sw     $3,  OFFSETOF_jet_interrupt_context_BadVAddr($sp) 
.endm
                                                                 
                                                                 
.macro SAVE_STATIC                                               
                 sw     $16, OFFSETOF_jet_interrupt_context_r16($sp) 
                 sw     $17, OFFSETOF_jet_interrupt_context_r17($sp) 
                 sw     $18, OFFSETOF_jet_interrupt_context_r18($sp) 
                 sw     $19, OFFSETOF_jet_interrupt_context_r19($sp) 
                 sw     $20, OFFSETOF_jet_interrupt_context_r20($sp) 
                 sw     $21, OFFSETOF_jet_interrupt_context_r21($sp) 
                 sw     $22, OFFSETOF_jet_interrupt_context_r22($sp) 
                 sw     $23, OFFSETOF_jet_interrupt_context_r23($sp) 
                 sw     $30, OFFSETOF_jet_interrupt_context_r30($sp) 
.endm
                                        

.macro SAVE_SOME                                                  
                 .set    push                                     
                 .set    noat                                     
                 move   $k0, $sp                                   
                 sw     $k0, OFFSETOF_jet_interrupt_context_r29($sp)   
                 sw     $3,  OFFSETOF_jet_interrupt_context_r3($sp)    
                 /*                                               
                  * We must  save $0, because                     
                  * the FPU emulator and gdb remote debug stub    
                  * need it to operate correctly                  
                  */                                              
                 sw     $0,  OFFSETOF_jet_interrupt_context_r0($sp)    
                 sw     $2,  OFFSETOF_jet_interrupt_context_r2($sp)    
                 sw     $4,  OFFSETOF_jet_interrupt_context_r4($sp)    
                 mfc0   $3,  CP0_CAUSE                            
                 sw     $5,  OFFSETOF_jet_interrupt_context_r5($sp)    
                 sw     $3,  OFFSETOF_jet_interrupt_context_CAUSE($sp) 
                 sw     $6,  OFFSETOF_jet_interrupt_context_r6($sp)   
                 sw     $7,  OFFSETOF_jet_interrupt_context_r7($sp)    
                 sw     $8,  OFFSETOF_jet_interrupt_context_r8($sp)    
                 sw     $9,  OFFSETOF_jet_interrupt_context_r9($sp)    
                 sw     $25, OFFSETOF_jet_interrupt_context_r25($sp)  
                 sw     $28, OFFSETOF_jet_interrupt_context_r28($sp)  
                 sw     $31, OFFSETOF_jet_interrupt_context_r31($sp)  
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
                 lw     $1,  OFFSETOF_jet_interrupt_context_r1($sp)   
                 .set    pop                                      
.endm
                                                                  
                                                                  
.macro RESTORE_TEMP
                 lw     $24,  OFFSETOF_jet_interrupt_context_BadVAddr($sp) 
                 MTC0($24,  CP0_BadVAddr)
                 lw     $24,  OFFSETOF_jet_interrupt_context_FCCR($sp)  
                 MTC1($24,  CP1_FCCR)
                 lw     $24, OFFSETOF_jet_interrupt_context_lo($sp)   
                 MTlo($24)                                        
                 lw     $24, OFFSETOF_jet_interrupt_context_hi($sp)   
                 MThi($24)                                        
                 lw     $8,  OFFSETOF_jet_interrupt_context_r8($sp)    
                 lw     $9,  OFFSETOF_jet_interrupt_context_r9($sp)    
                 lw     $10, OFFSETOF_jet_interrupt_context_r10($sp)  
                 lw     $11, OFFSETOF_jet_interrupt_context_r11($sp)  
                 lw     $12, OFFSETOF_jet_interrupt_context_r12($sp)  
                 lw     $13, OFFSETOF_jet_interrupt_context_r13($sp)  
                 lw     $14, OFFSETOF_jet_interrupt_context_r14($sp)  
                 lw     $15, OFFSETOF_jet_interrupt_context_r15($sp)  
                 lw     $24, OFFSETOF_jet_interrupt_context_r24($sp)  
.endm
                                                                  
.macro RESTORE_STATIC                                             
                 lw     $16, OFFSETOF_jet_interrupt_context_r16($sp)  
                 lw     $17, OFFSETOF_jet_interrupt_context_r17($sp)  
                 lw     $18, OFFSETOF_jet_interrupt_context_r18($sp)  
                 lw     $19, OFFSETOF_jet_interrupt_context_r19($sp)  
                 lw     $20, OFFSETOF_jet_interrupt_context_r20($sp)  
                 lw     $21, OFFSETOF_jet_interrupt_context_r21($sp)  
                 lw     $22, OFFSETOF_jet_interrupt_context_r22($sp)  
                 lw     $23, OFFSETOF_jet_interrupt_context_r23($sp)  
                 lw     $30, OFFSETOF_jet_interrupt_context_r30($sp)  
.endm
                                        
                                        
.macro RESTORE_SOME                                               
                 .set    push                                     
                 .set    noat                                     
                 lw     $31, OFFSETOF_jet_interrupt_context_r31($sp)  
                 lw     $28, OFFSETOF_jet_interrupt_context_r28($sp)  
                 lw     $25, OFFSETOF_jet_interrupt_context_r25($sp)  
                 lw     $8,  OFFSETOF_jet_interrupt_context_r8($sp)    
                 lw     $9,  OFFSETOF_jet_interrupt_context_r9($sp)    
                 lw     $7,  OFFSETOF_jet_interrupt_context_r7($sp)   
                 lw     $6,  OFFSETOF_jet_interrupt_context_r6($sp)   
                 lw     $3,  OFFSETOF_jet_interrupt_context_r3($sp)   
                 lw     $2,  OFFSETOF_jet_interrupt_context_r2($sp)   
                 lw     $a0, OFFSETOF_jet_interrupt_context_r29($sp)
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
 * EPC, STATUS are already restored to their final location.
 * a0, a1 are already restored into temporary locations.
 *
 * After the call:
 *
 * - sp remains pointed to the beginning of the interrupt frame.
 * - a0 contains restored sp
 * - cr is not restored yet. not need cr in MIPS
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
 * Prepare everything except EPC, STATUS, a0, a1.
 */
.macro EXCEPTION_EPILOGUE_COMMON
        RESTORE_REGS_COMMON
        /* Determine whether we return to user space. */
        lw      $a1, OFFSETOF_jet_interrupt_context_STATUS($sp)
        andi    $a1, $a1, CP0_STATUS_USER
        bgez    $a1, 1f
        /* Return to user - need to set DESAVE. */
    //rerwite    lw $a1, OFFSETOF_jet_interrupt_context_cr($sp) Restore everything else from interrupt frame.
    //rerwite    mtcr $a1 //Don't need it in MIPS
        MTC0($sp, CP0_DESAVE)
        b       2f
1:
        /* Return to kernel. */
 //rerwite       lw $a1, OFFSETOF_jet_interrupt_context_cr($sp) Restore everything else from interrupt frame.
 //rerwite       mtcr $a1 //Don't need it in MIPS
2:
        move    $sp, $a0
.endm

/*
 * Common exception prologue.
 *
 * Before the call:
 *     a0 and a1 are stored to some temporary locations before.
 *
 * After the call:
 *     sp points to interrupt frame, where a0, a1, STATUS and EPC should be stored.
 */
.macro EXCEPTION_PROLOGUE_COMMON
   //rerwite     mfcr    $a0
        /* Enable floating point bit in status */
        mfc0    $k0, CP0_STATUS
        li      $k1, CP0_STATUS_CU1
        or      $k0, $k0, $k1
        MTC0($k0, CP0_STATUS)
        /* Check whether kernel stack is usable. */
        mfc0    $a1, CP0_DESAVE
        andi    $a1, $a1, 1 /* Valid stack is always 16-byte aligned. */
        bgez    $a1, 1f
        /* Come from kernel and kernel stack is ready */
        /*This is stwu ppc realization on MIPS*/
        sw      $sp, -FRAME_SIZE-JET_STACK_RED_ZONE($sp)
        addi    $sp, $sp, -FRAME_SIZE-JET_STACK_RED_ZONE 
      //rerwite  sw      $a0, OFFSETOF_jet_interrupt_context_cr($sp)
        /* Store ra to previous backchain. */
        move    $a0, $ra
        sw      $a0, FRAME_SIZE + JET_STACK_RED_ZONE + OFFSETOF_jet_stack_frame_ra($sp)
        addi    $a0, $sp, FRAME_SIZE + JET_STACK_RED_ZONE
        b       2f
1:      /* Come from user or interrupted handler hasn't adjust stack yet. */
        mfc0    $a1, CP0_DESAVE
     //rerwite   mtcr $a0 /* Restore cr, so possible highlevel interrupt will see the same as we.*/
        /* Store cr before updating stack pointer! It can be overwritting by highlevel interrupt... but to the same value(see above). */
    //rerwite       sw $a0, OFFSETOF_jet_interrupt_context_cr($r4)
    //rerwite       move $a0, $sp
        //move    $sp, $a1 /* Back chain is already set to 0. */
        /* Mark kernel stack as ready for highlevel interrupts. */
   
   
    ///WHY WE NEED THIS?!!?
    
        //li      $a1, 1
        //move      $a1, $a1
        //MTC0($a1, CP0_DESAVE)
        MTC0($sp, CP0_DESAVE)
2:      SAVE_REGS_COMMON
.endm

/* Return from non-critical interrupt. */
        .globl pok_arch_rfi
pok_arch_rfi:
        /* a1 <-> sprg2, a0 <-> sprg3 */

        lw      $a1,OFFSETOF_jet_interrupt_context_r5($sp)
        sw      $a1, (save1)
        lw      $a0,OFFSETOF_jet_interrupt_context_r4($sp)
        sw      $a0, (save2)




    //TODO можно убирать:
        /* EPC->EPC, STATUS->STATUS. */

//        /*Do CP0_STATUS_SETTINGS = CP0_STATUS_CU1 | CP0_STATUS_FR | CP0_STATUS_IE*/
//        li   $k0, CP0_STATUS_SETTINGS                                           
        lw     $a1, OFFSETOF_jet_interrupt_context_STATUS($sp)
//        add    $a1, $a1, $k0
        MTC0($a1, CP0_STATUS)
        lw     $a0, OFFSETOF_jet_interrupt_context_EPC($sp)
        MTC0($a0, CP0_EPC)
        




        EXCEPTION_EPILOGUE_COMMON
       
       
        lw      $a1, (save1)
        lw      $a0, (save2)
       
       
        /* Clear ERL and EXL bits*/
        mfc0    $k0, CP0_STATUS
        ori     $k0, $k0, 0x6
        xori    $k0, $k0, 0x6
        mtc0    $k0, CP0_STATUS 


        mfc0    $k0, CP0_CAUSE
        andi    $k0, 0x7f
        li      $k1, 0x20
        beq     $k1, $k0, 2f  /*Syscall*/
        li      $k1, 0x24  
        beq     $k1, $k0, 2f  /*Break*/
    
/*
 * the return sequence from the exception handler for the case of an
 * external exception (interrupt)
 */
1:      mfc0    $k0, CP0_EPC                  /* get EPC in $k0 */
        ERET_AND_NOP                          /* return from exception */
        JUMP_TO_REG($k0)                      /* replace PC with the return address, doesnt need it, because eret replace PC too*/
/*
 * the return sequence from the exception handler for the case of a
 * trap (including a syscall).
 */
2:      mfc0    $k0, CP0_EPC                  /* get EPC in $k0 */
        addiu   $k0, 4                        /* make sure it points to next instruction */
        mtc0    $k0, CP0_EPC
        ERET_AND_NOP                          /* return from exception */
        JUMP_TO_REG($k0)                      /* replace PC with the return address, doesnt need it, because eret replace PC too */

/* Exception prologue for non-critical interrupts. */
.macro EXCEPTION_PROLOGUE
        /* a0 <-> save2, a1 <-> save1*/
        sw      $a0, (save2)
        sw      $a1, (save1)
        EXCEPTION_PROLOGUE_COMMON

        /* EPC<-EPC, STATUS<-STATUS. */
        mfc0    $a0, CP0_EPC
        sw      $a0, OFFSETOF_jet_interrupt_context_EPC($sp)
        mfc0    $a1, CP0_STATUS
        sw      $a1, OFFSETOF_jet_interrupt_context_STATUS($sp)

        lw      $a1, (save1)
        sw      $a1, OFFSETOF_jet_interrupt_context_r5($sp)
        lw      $a0, (save2)
        sw      $a0, OFFSETOF_jet_interrupt_context_r4($sp)
.endm
        
        
/* Return from debug interrupt. */
        .globl pok_arch_rfdi_for_debug
pok_arch_rfdi_for_debug:
        /* $a1 <-> sprg2, $a0 <-> sprg3. TODO: other temporary locations should be used. */
        lw      $a1, OFFSETOF_jet_interrupt_context_r5($sp)
        sw      $a1, (save1)
        lw      $a0, OFFSETOF_jet_interrupt_context_r4($sp)
        sw      $a0, (save2)

        /* EPC->DEPC, CAUSE->DEBUG. */
        lw      $a1, OFFSETOF_jet_interrupt_context_CAUSE($sp)
        MTC0($a1, CP0_DEBUG)
        lw      $a0, OFFSETOF_jet_interrupt_context_EPC($sp)
        MTC0($a0, CP0_DEPC)

        EXCEPTION_EPILOGUE_COMMON

        lw      $a1, (save1)
        lw      $a0, (save2)

        /* Clear ERL and EXL bits*/
        mfc0    $k0, CP0_STATUS
        ori     $k0, $k0, 0x6
        xori    $k0, $k0, 0x6
        mtc0    $k0, CP0_STATUS 

        mfc0    $k0, CP0_DEPC                 /* get DEPC in $k0 */
        addiu   $k0, 4                        /* make sure it points to next instruction */
        mtc0    $k0, CP0_DEPC
        DERET_AND_NOP
        JUMP_TO_REG($k0)        

.macro EXCEPTION_PROLOGUE_DEBUG
        /* a0 <-> sprg3, a1 <-> sprg2. TODO: use different temporaries. */
        sw      $a0, (save2)
        sw      $a1, (save1)
        EXCEPTION_PROLOGUE_COMMON

        /* EPC<-DEPC, CAUSE<-DEBUG. */
        mfc0    $a0, CP0_DEPC
        sw      $a0, OFFSETOF_jet_interrupt_context_EPC($sp)
        mfc0    $a1, CP0_DEBUG
        sw      $a1, OFFSETOF_jet_interrupt_context_CAUSE($sp)

        lw      $a0, (save2)
        sw      $a0, OFFSETOF_jet_interrupt_context_r4($sp)
        lw      $a1, (save1)
        sw      $a1, OFFSETOF_jet_interrupt_context_r5($sp)
.endm

        . = base + 0x10400
        .global pok_trap_addr
pok_trap_addr:
        .global pok_trap
pok_trap:
        break
        JUMP_TO_REG($ra)




failed: b failed


        . = base + 0x10450
        .globl _pok_reset
system_reset_0100:
_pok_reset:
        /* Setup stack */
        la      $sp, pok_stack_end

        /* Mark kernel stack as ready for interrupts. */
        li      $a1, 1
        MTC0($a1, CP0_DESAVE)

        /* Switch on CP1 and CP0*/
        mfc0    $k0, CP0_STATUS
        li      $k1, CP0_STATUS_CU1
        or      $k0, $k0, $k1
        li      $k1, CP0_STATUS_CU0
        or      $k0, $k0, $k1
        li      $k1, CP0_STATUS_BEV
        or      $k0, $k0, $k1
        xor     $k0, $k0, $k1
        MTC0($k0, CP0_STATUS)

            
        JUMP_AND_LINK(pok_setup_interrupts)

        /* TODO: copy .sdata, clear .bss */
        /* Copy .data */

//not sure in this part of code

//~       la       $a0, __data_start-4
//~       la       $a1, __data_load-4
//~       la       $a2, __data_end
//~        bgez     $a2-$a0, 2f
//~1:      /*equivalent lwzu in ppc*/
//~        lw       $a3, 4($a1)
//~        addi     $a1, 4
//~        /*equivalent stwu in ppc*/
//~        sw       $a3, 4($a0)
//~        addi     $a0, 4
//~        bltz     $a2-$a0, 1b
//~2:

        /* Call C functions */
        JUMP_AND_LINK(pok_arch_init)

        JUMP_AND_LINK(jet_boot) /* shouldn't return */
1:      JUMP_TO_ADDR(1b)

/* Interrupt vectors */
/* TODO handle kernel/user stack transition correctly */
/* TODO some interrupts, like "critical interrupt", use different
 *      set of save/restore registers, so they need
 *      a different prologue and rfi functions
 */


// Useful defines:
#define START_EXCEPTION(label)                                               \
        .align 5;                                                            \
label##_asm:

// Handlers themselves:

    /* TODO: shouldn't critical interrupt level should be used there? */
    START_EXCEPTION(pok_int_critical_input)
        EXCEPTION_PROLOGUE
        move    $a0, $sp
        JUMP_AND_LINK(pok_int_critical_input)
        JUMP_TO_ADDR(pok_arch_rfi)

    /* TODO: shouldn't machine check interrupt level should be used there? */
    START_EXCEPTION(pok_int_machine_check)
        EXCEPTION_PROLOGUE
        move    $a0, $sp
        JUMP_AND_LINK(pok_int_machine_check)
        JUMP_TO_ADDR(pok_arch_rfi)

    START_EXCEPTION(pok_int_data_storage)
        EXCEPTION_PROLOGUE
        move    $a0, $sp

        mfc0    $a1, CP0_BadVAddr
        mfc0    $a2, CP0_CAUSE

        JUMP_AND_LINK(pok_int_data_storage)
        JUMP_TO_ADDR(pok_arch_rfi)

    START_EXCEPTION(pok_int_inst_storage)
        EXCEPTION_PROLOGUE
        move    $a0, $sp
        mfc0    $a1, CP0_BadVAddr
        mfc0    $a2, CP0_CAUSE

        JUMP_AND_LINK(pok_int_inst_storage)
        JUMP_TO_ADDR(pok_arch_rfi)

    START_EXCEPTION(pok_int_ext_interrupt)
        EXCEPTION_PROLOGUE
        move    $a0, $sp
        JUMP_AND_LINK(pok_int_ext_interrupt)
        JUMP_TO_ADDR(pok_arch_rfi)

    START_EXCEPTION(pok_int_alignment)
        EXCEPTION_PROLOGUE
        move    $a0, $sp
        mfc0    $a1, CP0_BadVAddr
        mfc0    $a2, CP0_CAUSE

        JUMP_AND_LINK(pok_int_alignment)
        JUMP_TO_ADDR(pok_arch_rfi)

    START_EXCEPTION(pok_int_program)
        EXCEPTION_PROLOGUE
        move    $a0, $sp

        JUMP_AND_LINK(pok_int_program)
        JUMP_TO_ADDR(pok_arch_rfi)

    START_EXCEPTION(pok_int_fp_unavail)
        EXCEPTION_PROLOGUE
        move    $a0, $sp

        JUMP_AND_LINK(pok_int_fp_unavail)
        JUMP_TO_ADDR(pok_arch_rfi)

    START_EXCEPTION(pok_int_system_call)
        EXCEPTION_PROLOGUE
        move    $a0, $sp
        /* load system call arguments back from the interrupt frame */
        subu    $sp, 24
        lw      $a1,OFFSETOF_jet_interrupt_context_r3($sp)          
        lw      $a2,OFFSETOF_jet_interrupt_context_r4($sp)                   
        lw      $a3,OFFSETOF_jet_interrupt_context_r5($sp)
        /*send 5-7 arg to syscall*/
        lw      $k0,OFFSETOF_jet_interrupt_context_r29($sp)
        lw      $k1, 16($k0)
        sw      $k0, 16($sp)
        lw      $k1, 20($k0)
        sw      $k0, 20($sp)
        lw      $k1, 24($k0)
        sw      $k0, 24($sp)
        
        JUMP_AND_LINK(pok_int_system_call) 
        
        addi    $sp, 24
        sw      $v0,OFFSETOF_jet_interrupt_context_r2($sp)

        JUMP_TO_ADDR(pok_arch_rfi)

    START_EXCEPTION(pok_int_decrementer)
        EXCEPTION_PROLOGUE
        move    $a0, $sp

        JUMP_AND_LINK(pok_int_decrementer) 
        JUMP_TO_ADDR(pok_arch_rfi)

    START_EXCEPTION(pok_int_interval_timer)
        EXCEPTION_PROLOGUE
        move    $a0, $sp

        JUMP_AND_LINK(pok_int_interval_timer)
        JUMP_TO_ADDR(pok_arch_rfi)

    START_EXCEPTION(pok_int_watchdog)
        EXCEPTION_PROLOGUE
        move    $a0, $sp
        
    START_EXCEPTION(pok_int_ri)
        EXCEPTION_PROLOGUE
        move    $a0, $sp

        JUMP_AND_LINK(pok_int_ri)
        JUMP_TO_ADDR(pok_arch_rfi)

    START_EXCEPTION(pok_int_data_tlb_miss)
        EXCEPTION_PROLOGUE
        move    $a0, $sp
        mfc0    $a1, CP0_BadVAddr
        mfc0    $a2, CP0_CAUSE

        JUMP_AND_LINK(pok_int_data_tlb_miss)
        JUMP_TO_ADDR(pok_arch_rfi)

    START_EXCEPTION(pok_int_inst_tlb_miss)
        EXCEPTION_PROLOGUE
        move    $a0, $sp
        mfc0    $a1, CP0_BadVAddr
        mfc0    $a2, CP0_CAUSE

        JUMP_AND_LINK(pok_int_inst_tlb_miss)
        JUMP_TO_ADDR(pok_arch_rfi)
        
    START_EXCEPTION(pok_int_debug)
        //EXCEPTION_PROLOGUE_DEBUG
        EXCEPTION_PROLOGUE
        move    $a0, $sp

        JUMP_AND_LINK(pok_int_debug)
        //JUMP_TO_ADDR(pok_arch_rfdi_for_debug)
        JUMP_TO_ADDR(pok_arch_rfi)
 
    START_EXCEPTION(pok_int_none)
        EXCEPTION_PROLOGUE
        move    $a0, $sp

        JUMP_AND_LINK(pok_int_none)
        JUMP_TO_ADDR(pok_arch_rfi)

    START_EXCEPTION(pok_int_overflow)
        EXCEPTION_PROLOGUE
        move    $a0, $sp

        JUMP_AND_LINK(pok_int_overflow)
        JUMP_TO_ADDR(pok_arch_rfi)

    START_EXCEPTION(pok_int_addrl)
        EXCEPTION_PROLOGUE
        move    $a0, $sp

        JUMP_AND_LINK(pok_int_addrl)
        JUMP_TO_ADDR(pok_arch_rfi)

    START_EXCEPTION(pok_int_addrs)
        EXCEPTION_PROLOGUE
        move    $a0, $sp

        JUMP_AND_LINK(pok_int_addrs)
        JUMP_TO_ADDR(pok_arch_rfi)

    START_EXCEPTION(pok_int_ibus)
        EXCEPTION_PROLOGUE
        move    $a0, $sp

        JUMP_AND_LINK(pok_int_ibus)
        JUMP_TO_ADDR(pok_arch_rfi)

    START_EXCEPTION(pok_int_dbus)
        EXCEPTION_PROLOGUE
        move    $a0, $sp

        JUMP_AND_LINK(pok_int_dbus)
        JUMP_TO_ADDR(pok_arch_rfi)


/*
 *  Adding addr of C part of exeption handler for each interrupt in array
 */

#define  SET_INT(vector_number, vector_label)                       \
                la      $k0, vector_label##_asm;                    \
                sw      $k0, vector_number*4 + expection_handlers ;   \
                sync


pok_setup_interrupts:
        /*EBASE doenst support in this MIPS*/
        /*Switch on using EBASE addr for exeption handler*/
        //~mfc0   $k0, CP0_STATUS
        //~li     $k1, CP0_STATUS_BEV
        //~or     $k0, $k0, $k1
        //~MTC0($k0, CP0_STATUS)
        /*Switch off using the special interrupt vector*/ 
        mfc0    $k0, CP0_CAUSE
        li      $k1, CP0_CAUSE_IV
        or      $k0, $k0, $k1
        xor     $k0, $k0, $k1
        MTC0($k0, CP0_CAUSE)
        mfc0    $k0, CP0_STATUS
        li      $k1, CP0_STATUS_ALL_INT_ON
        or      $k0, $k0, $k1
        li      $k1, CP0_STATUS_IE
        or      $k0, $k0, $k1
        xor     $k0, $k0, $k1

        //~ori     $k0, $k0, CP0_STATUS_ERL
        //~xori     $k0, $k0, CP0_STATUS_ERL
        MTC0($k0, CP0_STATUS) 
        //~li    $k0, mips_first_exeption_handler_asm
        //~MTC0_sel1($k0, CP0_EBASE)
        

        SET_INT(0,  pok_int_decrementer)   /*Прерывание                    INT Interrupt               */
        SET_INT(1,  pok_int_none)          /*Запись в защищенную страницу                              */
                                           /*                            или при выборке инструкции)   */
        SET_INT(2,  pok_int_inst_tlb_miss) /*Исключение в TLB (при чтении данных                       */
        SET_INT(3,  pok_int_data_tlb_miss) /*Исключение в TLB (при записи данных)                      */
        SET_INT(4,  pok_int_addrl)          /*Неправильный адрес при чтении  ADDRL Load from an illegal address   */
        SET_INT(5,  pok_int_addrs)          /*Неправильный адрес при записи  ADDRS Store to an illegal address    */
        SET_INT(6,  pok_int_ibus)          /*Ошибка шины при выборе команды IBUS Bus error on instruction fetch  */
        SET_INT(7,  pok_int_dbus)          /*Ошибка шины при чтении данных  DBUS Bus error on data reference     */
        SET_INT(8,  pok_int_system_call)   /*Команда SYSCALL                SYSCALL syscall instruction executed */
        SET_INT(9,  pok_int_debug)         /*Команда BREAK                  BKPT break instruction executed      */
        SET_INT(10, pok_int_ri)           /*Нереализованная команда        RI Reserved instruction              */
        SET_INT(11, pok_int_none)         /*Недоступный сопроцессор                                   */
        SET_INT(12, pok_int_overflow)         /*Целочисленное переполнение     OVF Arithmetic overflow    */
        SET_INT(13, pok_int_debug)        /*Команды TRAP                                              */
        SET_INT(14, pok_int_fp_unavail) /*????*/  /*Исключение от блока вещественной арифметики       */
        SET_INT(15, pok_int_none)         /*Исключение при доступе в накристальную память             */    


        
        JUMP_TO_REG($ra)

        
        la  $k0, mips_first_exeption_handler_asm
        jr  $k0
/*This exeption handler is equal for each interrupt*/
mips_first_exeption_handler_asm:
        .set  push
        mfc0    $k1, CP0_CAUSE
        andi    $k1, $k1, 0x7c   	                   /* read exception number */
    	lw      $k0, expection_handlers($k1)         /* exception number is an offset in array */
        
    	JUMP_TO_REG($k0)                           /* Call asm code for this exeption. */
        .set  pop

        .globl dmfc0_asm


dmfc0_asm:
        .set mips64
        move    $k0, $zero
        li      $k0, 0x70000180
        mtc0    $k0, CP0_ENTRYHI
        dmfc0   $k0, CP0_ENTRYHI
        nop
        nop
        li      $k0, 0xa0000000
        addu    $ra, $k0, $ra
        JUMP_TO_REG($ra)
        .set mips32


        .section ".bss", "aw"
        .align 4 # 4 words = 16 bytes
pok_stack:
		.global pok_stack_end
        .space 8 * 1024
pok_stack_end:

pok_save_area:
        .space 4 * 8


