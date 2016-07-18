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

/* FP_START must be equal to offsetof(volatile_context_t, fp0) */
#define FP_START 80
#define SAVE_FP(num) stfd %f##num,(FP_START + 8*num)(%r1)
#define LOAD_FP(num) lfd %f##num,(FP_START + 8*num)(%r1)

/* FRAME_SIZE must be equal to offsetof(volatile_context_t, back_chain) */
#define FRAME_SIZE 96
        /* r4 is available, r1 is set, r3 contains cr,
           sprg3 contains r3 and sprg2 contains r2.  */

#define  SAVE_AT  sw  $1, PT_R(1)(sp)

#define PT_R0     24
#define PT_R1     28
#define PT_R2     32
#define PT_R3     36
#define PT_R4     40
#define PT_R5     44
#define PT_R6     48
#define PT_R7     52
#define PT_R8     56
#define PT_R9     60
#define PT_R10    64
#define PT_R11    68
#define PT_R12    72
#define PT_R13    76
#define PT_R14    80
#define PT_R15    84
#define PT_R16    88
#define PT_R17    92
#define PT_R18    96
#define PT_R19    100
#define PT_R20    104
#define PT_R21    108
#define PT_R22    112
#define PT_R23    116
#define PT_R24    120
#define PT_R25    124
#define PT_R26    128
#define PT_R27    132
#define PT_R28    136
#define PT_R29    140
#define PT_R30    144
#define PT_R31    148
#define PT_LO     152
#define PT_HI     156
#define PT_EPC    160
#define PT_BVADDR 164
#define PT_STATUS 168
#define PT_CAUSE  172
#define PT_SIZE   176     
        
 #define SAVE_AT                                          \
                 sw      $1, PT_R1(sp)
 
 #define SAVE_TEMP                                        \
                 mfhi    v1;                              \
                 sw      $8, PT_R8(sp);                   \
                 sw      $9, PT_R9(sp);                   \
                 sw      v1, PT_HI(sp);                   \
                 mflo    v1;                              \
                 sw      $10,PT_R10(sp);                  \
                 sw      $11, PT_R11(sp);                 \
                 sw      v1,  PT_LO(sp);                  \
                 sw      $12, PT_R12(sp);                 \
                 sw      $13, PT_R13(sp);                 \
                 sw      $14, PT_R14(sp);                 \
                 sw      $15, PT_R15(sp);                 \
                 sw      $24, PT_R24(sp)
 
 #define SAVE_STATIC                                      \
                 sw      $16, PT_R16(sp);                 \
                 sw      $17, PT_R17(sp);                 \
                 sw      $18, PT_R18(sp);                 \
                 sw      $19, PT_R19(sp);                 \
                 sw      $20, PT_R20(sp);                 \
                 sw      $21, PT_R21(sp);                 \
                 sw      $22, PT_R22(sp);                 \
                 sw      $23, PT_R23(sp);                 \
                 sw      $30, PT_R30(sp)
 #define SAVE_SOME                                        \
                 mfc0    k0, CP0_STATUS;                  \
                 sll     k0, 3;     /* extract cu0 bit */ \
                 bltz    k0, 8f;                          \
                  move   k1, sp;                          \
                 /* Called from user mode, new stack. */  \
                 lui     k1, %hi(kernelsp);               \
                 lw      k1, %lo(kernelsp)(k1);           \
 8:                                                       \
                 move    k0, sp;                          \
                 subu    sp, k1, PT_SIZE;                 \
                 sw      k0, PT_R29(sp);                  \
                 sw      $3, PT_R3(sp);                   \
                 sw      $0, PT_R0(sp);                   \
                 mfc0    v1, CP0_STATUS;                  \
                 sw      $2, PT_R2(sp);                   \
                 sw      v1, PT_STATUS(sp);               \
                 sw      $4, PT_R4(sp);                   \
                 mfc0    v1, CP0_CAUSE;                   \
                 sw      $5, PT_R5(sp);                   \
                 sw      v1, PT_CAUSE(sp);                \
                 sw      $6, PT_R6(sp);                   \
                 mfc0    v1, CP0_EPC;                     \
                 sw      $7, PT_R7(sp);                   \
                 sw      v1, PT_EPC(sp);                  \
                 sw      $25, PT_R25(sp);                 \
                 sw      $28, PT_R28(sp);                 \
                 sw      $31, PT_R31(sp);                 \
                 ori     $28, sp, 0x1fff;                 \??
                 xori    $28, 0x1fff;                     \??



#define RESTORE_AT                                       \
                lw      $1,  PT_R1(sp);                  \
 
#define RESTORE_SP                                       \
                lw      sp,  PT_R29(sp)

#define RESTORE_TEMP                                     \
                lw      $24, PT_LO(sp);                  \
                lw      $8, PT_R8(sp);                   \
                lw      $9, PT_R9(sp);                   \
                mtlo    $24;                             \
                lw      $24, PT_HI(sp);                  \
                lw      $10,PT_R10(sp);                  \
                lw      $11, PT_R11(sp);                 \
                mthi    $24;                             \
                lw      $12, PT_R12(sp);                 \
                lw      $13, PT_R13(sp);                 \
                lw      $14, PT_R14(sp);                 \
                lw      $15, PT_R15(sp);                 \
                lw      $24, PT_R24(sp)

#define RESTORE_STATIC                                   \
                lw      $16, PT_R16(sp);                 \
                lw      $17, PT_R17(sp);                 \
                lw      $18, PT_R18(sp);                 \
                lw      $19, PT_R19(sp);                 \
                lw      $20, PT_R20(sp);                 \
                lw      $21, PT_R21(sp);                 \
                lw      $22, PT_R22(sp);                 \
                lw      $23, PT_R23(sp);                 \
                lw      $30, PT_R30(sp)

#define RESTORE_SOME                                     \
                mfc0    t0, CP0_STATUS;                  \
                ori     t0, 0x1f;                        \
                xori    t0, 0x1f;                        \
                mtc0    t0, CP0_STATUS;                  \
                li      v1, 0xff00;                      \
                and     t0, v1;                          \
                lw      v0, PT_STATUS(sp);               \
                nor     v1, $0, v1;                      \
                and     v0, v1;                          \
                or      v0, t0;                          \
                mtc0    v0, CP0_STATUS;                  \
                lw      v1, PT_EPC(sp);                  \
                mtc0    v1, CP0_EPC;                     \
                lw      $31, PT_R31(sp);                 \
                lw      $28, PT_R28(sp);                 \
                lw      $25, PT_R25(sp);                 \
                lw      $7,  PT_R7(sp);                  \
                lw      $6,  PT_R6(sp);                  \
                lw      $5,  PT_R5(sp);                  \
                lw      $4,  PT_R4(sp);                  \
                lw      $3,  PT_R3(sp);                  \
                lw      $2,  PT_R2(sp)

#define RESTORE_ALL                                      \
                RESTORE_SOME;                            \
                RESTORE_AT;                              \
                RESTORE_TEMP;                            \
                RESTORE_STATIC;                          \
                RESTORE_SP

        /* Restore registers */
        .globl pok_arch_eret
pok_arch_eret:
        RESTORE_ALL
        eret



#define NESTED(symbol, framesize, rpc)                  \
                .globl  symbol;                         \
                .align  2;                              \
                .type   symbol,@function;               \
                .ent    symbol,0;                       \
symbol:         .frame  sp, framesize, rpc
    
    
    
#define SAVE_REGS                                \
        SAVE_SOME;                                \   
        SAVE_AT;                                    \
        SAVE_TEMP;                                   \
        SAVE_STATIC;                                  \




        /* Establish new frame.  */              \
        mflr    %r4;                             \
        sw     %r4,FRAME_SIZE+4(%r1);  /* lr */ \
        mfctr   %r4;                             \
        sw     %r4,64(%r1);  /* ctr */          \
        mfxer   %r4;                             \
        sw     %r4,68(%r1);  /* xer */          \
        sw     %r3,8(%r1);   /* cr = v1 */           \
        sw     %r0,12(%r1);  /* r0 = zero*/           \
        sw     %r2,16(%r1);  /* r2 = v0*/           \
        mfsprg  %r0,3;                           \
        sw     %r0,20(%r1);  /* r3 */           \
        mfsprg  %r0,2;                           \
        sw     %r0,24(%r1);  /* r4 */           \
        sw     %r5,28(%r1);  /* r5 = a1*/           \
        sw     %r6,32(%r1);  /* r6 = a2*/           \
        sw     %r7,36(%r1);  /* r7 = a3*/           \
        sw     %r8,40(%r1);  /* r8 = t0*/           \
        sw     %r9,44(%r1);  /* r9 = t1*/           \
        sw     %r10,48(%r1); /* r10 = t2*/          \
        sw     %r11,52(%r1); /* r11 = t3*/          \
        sw     %r12,56(%r1); /* r12 = t4*/          \
        sw     %r13,60(%r1); /* r13 = t5*/          \
        SAVE_FP(0); /* f0 */                     \
        mfc0    %r3, 0x0C;                             \
        stw     %r3,72(%r1);  /* srr0 = cp = CPO_status ???????*/         \
        mfc1    %r4, 0x1F;                             \
        stw     %r4,76(%r1)  /* srr1 = msr = CP1_Status*/         
        
        
        
        /* Restore registers */
        .globl pok_arch_rfi
pok_arch_rfi:
        LOAD_FP(0)
        lwz     %r13,60(%r1)
        lwz     %r12,56(%r1)
        lwz     %r11,52(%r1)
        lwz     %r10,48(%r1)
        lwz     %r9,44(%r1)
        lwz     %r8,40(%r1)
        lwz     %r7,36(%r1)
        lwz     %r6,32(%r1)
        lwz     %r5,28(%r1)
        lwz     %r4,24(%r1)
        lwz     %r3,20(%r1)
        lwz     %r2,16(%r1)
        lwz     %r0,FRAME_SIZE+4(%r1)
        mtlr    %r0
        lwz     %r0,64(%r1)
        mtctr   %r0
        lwz     %r0,68(%r1)
        mtxer   %r0
        lwz     %r0,72(%r1)
        mtsrr0  %r0
        lwz     %r0,76(%r1)
        mtsrr1  %r0
        andi.   %r0,%r0,MSR_PR
        beq     1f
        /* If user mode, restore kernel stack and save it to SPRG1 */
        addi    %r0,%r1,FRAME_SIZE
        mtsprg  1,%r0
1:      lwz     %r0,8(%r1)
        mtcr    %r0
        lwz     %r0,12(%r1)
        lwz     %r1,0(%r1)
        rfi

#define EXCEPTION_PROLOGUE                      \
        mtsprg  3,%r3;                          \
        mtsprg  2,%r4;                          \
        mfcr    %r3;                            \
        /* Enable floating point bit in msr */  \
        mfmsr %r4;                              \
        ori %r4,%r4,MSR_FP;                     \
        mtmsr %r4;                              \
        mfsrr1  %r4;                            \
        andi.   %r4,%r4,MSR_PR;                 \
        bne     1f;                             \
        /* Already on kernel stack */           \
        stwu %r1, -FRAME_SIZE(%r1);             \
        b       2f;                             \
1:      /* User stack right now          */     \
        mr %r4, %r1;                            \
        mfsprg %r1, 1;                          \
        stwu %r4, -FRAME_SIZE(%r1);/*save it*/  \
2:      SAVE_REGS;                              \
        lis %r5, global_thread_stack@ha;      \
        addi %r5, %r5, global_thread_stack@l; \
        stw %r1, 0(r5);                         \
        mfsrr0 %r3; /* EA of exception */


#define SAVE_FP_FOR_INT(num) stfd %f##num,(152+8*num)(%r1)
#define LOAD_FP_FOR_INT(num) lfd %f##num,(152+8*num)(%r1)

#define FRAME_SIZE_FOR_INT 168 /*96+72*/
        /* r4 is available, r1 is set, r3 contains cr,
           sprg3 contains r3 and sprg2 contains r2.  */
#define SAVE_REGS_FOR_INT                               \
        /* Establish new frame.  */                     \
        mflr    %r4;                                    \
        stw     %r4,FRAME_SIZE_FOR_INT+4(%r1); /* lr */ \
        mfctr   %r4;                                    \
        stw     %r4,64(%r1);  /* ctr */                 \
        mfxer   %r4;                                    \
        stw     %r4,68(%r1);  /* xer */                 \
        stw     %r3,8(%r1);   /* cr */                  \
        stw     %r0,12(%r1);  /* r0 */                  \
        stw     %r2,16(%r1);  /* r2 */                  \
        mfsprg  %r0,3;                                  \
        stw     %r0,20(%r1);  /* r3 */                  \
        mfsprg  %r0,2;                                  \
        stw     %r0,24(%r1);  /* r4 */                  \
        stw     %r5,28(%r1);  /* r5 */                  \
        stw     %r6,32(%r1);  /* r6 */                  \
        stw     %r7,36(%r1);  /* r7 */                  \
        stw     %r8,40(%r1);  /* r8 */                  \
        stw     %r9,44(%r1);  /* r9 */                  \
        stw     %r10,48(%r1); /* r10 */                 \
        stw     %r11,52(%r1); /* r11 */                 \
        stw     %r12,56(%r1); /* r12 */                 \
        stw     %r13,60(%r1); /* r13 */                 \
        SAVE_FP_FOR_INT(0); /* f0 */                    \
        mfsrr0  %r3;                                    \
        stw     %r3,72(%r1);  /* srr0 */                \
        mfsrr1  %r4;                                    \
        stw     %r4,76(%r1);  /* srr1 */                \
        stw     %r14,80(%r1); /* r14 */                 \
        stw     %r15,84(%r1); /* r15 */                 \
        stw     %r16,88(%r1); /* r16 */                 \
        stw     %r17,92(%r1); /* r17 */                 \
        stw     %r18,96(%r1); /* r18 */                 \
        stw     %r19,100(%r1); /* r19 */                \
        stw     %r20,104(%r1); /* r20 */                \
        stw     %r21,108(%r1); /* r21 */                \
        stw     %r22,112(%r1); /* r22 */                \
        stw     %r23,116(%r1); /* r23 */                \
        stw     %r24,120(%r1); /* r24 */                \
        stw     %r25,124(%r1); /* r25 */                \
        stw     %r26,128(%r1); /* r26 */                \
        stw     %r27,132(%r1); /* r27 */                \
        stw     %r28,136(%r1); /* r28 */                \
        stw     %r29,140(%r1); /* r29 */                \
        stw     %r30,144(%r1); /* r30 */                \
        stw     %r31,148(%r1); /* r31 */
        

        
        
        /* Restore registers */
        .globl pok_arch_rfi_for_int
pok_arch_rfi_for_int:
        LOAD_FP_FOR_INT(0)
        lwz     %r31,148(%r1)         
        lwz     %r30,144(%r1)          
        lwz     %r29,140(%r1)          
        lwz     %r28,136(%r1)
        lwz     %r27,132(%r1)       
        lwz     %r26,128(%r1)          
        lwz     %r25,124(%r1)          
        lwz     %r24,120(%r1)          
        lwz     %r23,116(%r1)          
        lwz     %r22,112(%r1)          
        lwz     %r21,108(%r1)          
        lwz     %r20,104(%r1)          
        lwz     %r19,100(%r1)           
        lwz     %r18,96(%r1)           
        lwz     %r17,92(%r1)           
        lwz     %r16,88(%r1)          
        lwz     %r15,84(%r1)          
        lwz     %r14,80(%r1)           
        lwz     %r13,60(%r1)
        lwz     %r12,56(%r1)
        lwz     %r11,52(%r1)
        lwz     %r10,48(%r1)
        lwz     %r9,44(%r1)
        lwz     %r8,40(%r1)
        lwz     %r7,36(%r1)
        lwz     %r6,32(%r1)
        lwz     %r5,28(%r1)
        lwz     %r4,24(%r1)
        lwz     %r3,20(%r1)
        lwz     %r2,16(%r1)
        lwz     %r0,FRAME_SIZE_FOR_INT+4(%r1)
        mtlr    %r0
        lwz     %r0,64(%r1)
        mtctr   %r0
        lwz     %r0,68(%r1)
        mtxer   %r0
        lwz     %r0,72(%r1)
        mtsrr0  %r0
        lwz     %r0,76(%r1)
        mtsrr1  %r0
        andi.   %r0,%r0,MSR_PR
        beq     1f
        /* If user mode, restore kernel stack and save it to SPRG1 */
        addi    %r0,%r1,FRAME_SIZE_FOR_INT
        mtsprg  1,%r0
1:      lwz     %r0,8(%r1)
        mtcr    %r0
        lwz     %r0,12(%r1)
        lwz     %r1,0(%r1)
        rfi

        /* Restore registers */
        .globl pok_arch_rfdi_for_debug
pok_arch_rfdi_for_debug:
        LOAD_FP_FOR_INT(0)
        lwz     %r31,148(%r1)         
        lwz     %r30,144(%r1)          
        lwz     %r29,140(%r1)          
        lwz     %r28,136(%r1)
        lwz     %r27,132(%r1)       
        lwz     %r26,128(%r1)          
        lwz     %r25,124(%r1)          
        lwz     %r24,120(%r1)          
        lwz     %r23,116(%r1)          
        lwz     %r22,112(%r1)          
        lwz     %r21,108(%r1)          
        lwz     %r20,104(%r1)          
        lwz     %r19,100(%r1)           
        lwz     %r18,96(%r1)           
        lwz     %r17,92(%r1)           
        lwz     %r16,88(%r1)          
        lwz     %r15,84(%r1)          
        lwz     %r14,80(%r1)           
        lwz     %r13,60(%r1)
        lwz     %r12,56(%r1)
        lwz     %r11,52(%r1)
        lwz     %r10,48(%r1)
        lwz     %r9,44(%r1)
        lwz     %r8,40(%r1)
        lwz     %r7,36(%r1)
        lwz     %r6,32(%r1)
        lwz     %r5,28(%r1)
        lwz     %r4,24(%r1)
        lwz     %r3,20(%r1)
        lwz     %r2,16(%r1)
        lwz     %r0,FRAME_SIZE_FOR_INT+4(%r1)
        mtlr    %r0
        lwz     %r0,64(%r1)
        mtctr   %r0
        lwz     %r0,68(%r1)
        mtxer   %r0
        lwz     %r0,72(%r1)
        mtspr  574,%r0
        lwz     %r0,76(%r1)
        mtspr  575,%r0
        andi.   %r0,%r0,MSR_PR
        beq     1f
        /* If user mode, restore kernel stack and save it to SPRG1 */
        addi    %r0,%r1,FRAME_SIZE_FOR_INT
        mtsprg  1,%r0
1:      lwz     %r0,8(%r1)
        mtcr    %r0
        lwz     %r0,12(%r1)
        lwz     %r1,0(%r1)
        rfdi

#define SAVE_REGS_FOR_DEBUG                             \
        /* Establish new frame.  */                     \
        mflr    %r4;                                    \
        stw     %r4,FRAME_SIZE_FOR_INT+4(%r1); /* lr */ \
        mfctr   %r4;                                    \
        stw     %r4,64(%r1);  /* ctr */                 \
        mfxer   %r4;                                    \
        stw     %r4,68(%r1);  /* xer */                 \
        stw     %r3,8(%r1);   /* cr */                  \
        stw     %r0,12(%r1);  /* r0 */                  \
        stw     %r2,16(%r1);  /* r2 */                  \
        mfsprg  %r0,3;                                  \
        stw     %r0,20(%r1);  /* r3 */                  \
        mfsprg  %r0,2;                                  \
        stw     %r0,24(%r1);  /* r4 */                  \
        stw     %r5,28(%r1);  /* r5 */                  \
        stw     %r6,32(%r1);  /* r6 */                  \
        stw     %r7,36(%r1);  /* r7 */                  \
        stw     %r8,40(%r1);  /* r8 */                  \
        stw     %r9,44(%r1);  /* r9 */                  \
        stw     %r10,48(%r1); /* r10 */                 \
        stw     %r11,52(%r1); /* r11 */                 \
        stw     %r12,56(%r1); /* r12 */                 \
        stw     %r13,60(%r1); /* r13 */                 \
        SAVE_FP_FOR_INT(0); /* f0 */                    \
        mfspr  %r3,574;                                    \
        stw     %r3,72(%r1);  /* dsrr0 */                \
        mfspr %r4,575;                                    \
        stw     %r4,76(%r1);  /* dsrr1 */                \
        stw     %r14,80(%r1); /* r14 */                 \
        stw     %r15,84(%r1); /* r15 */                 \
        stw     %r16,88(%r1); /* r16 */                 \
        stw     %r17,92(%r1); /* r17 */                 \
        stw     %r18,96(%r1); /* r18 */                 \
        stw     %r19,100(%r1); /* r19 */                \
        stw     %r20,104(%r1); /* r20 */                \
        stw     %r21,108(%r1); /* r21 */                \
        stw     %r22,112(%r1); /* r22 */                \
        stw     %r23,116(%r1); /* r23 */                \
        stw     %r24,120(%r1); /* r24 */                \
        stw     %r25,124(%r1); /* r25 */                \
        stw     %r26,128(%r1); /* r26 */                \
        stw     %r27,132(%r1); /* r27 */                \
        stw     %r28,136(%r1); /* r28 */                \
        stw     %r29,140(%r1); /* r29 */                \
        stw     %r30,144(%r1); /* r30 */                \
        stw     %r31,148(%r1); /* r31 */				\
        mffs	%f1;		   /* FPSCR */				\
        SAVE_FP_FOR_INT(1);    /* f1 */					\
        lwz		%r0,168(%r1);							\
        stw     %r0,152(%r1);  /* FPSCR */				\


#define EXCEPTION_PROLOGUE_FOR_DEBUG                    \
        mtsprg  3,%r3;                                  \
        mtsprg  2,%r4;                                  \
        mfcr    %r3;                                    \
        /* Enable floating point bit in msr */          \
        mfmsr %r4;                                      \
        ori %r4,%r4,MSR_FP;                             \
        mtmsr %r4;                                      \
        mfsrr1  %r4;                                    \
        andi.   %r4,%r4,MSR_PR;                         \
        bne     1f;                                     \
        /* Already on kernel stack */                   \
        stwu %r1, -FRAME_SIZE_FOR_INT(%r1);             \
        b       2f;                                     \
1:      /* User stack right now          */             \
        mr %r4, %r1;                                    \
        mfsprg %r1, 1;                                  \
        stwu %r4, -FRAME_SIZE_FOR_INT(%r1);/*save it*/  \
2:      SAVE_REGS_FOR_DEBUG;                            \
        lis %r5, global_thread_stack@ha;              \
        addi %r5, %r5, global_thread_stack@l;         \
        stw %r1, 0(r5);                                 \
        mr	%r3, %r1;

#define EXCEPTION_PROLOGUE_FOR_INT                      \
        mtsprg  3,%r3;                                  \
        mtsprg  2,%r4;                                  \
        mfcr    %r3;                                    \
        /* Enable floating point bit in msr */          \
        mfmsr %r4;                                      \
        ori %r4,%r4,MSR_FP;                             \
        mtmsr %r4;                                      \
        mfsrr1  %r4;                                    \
        andi.   %r4,%r4,MSR_PR;                         \
        bne     1f;                                     \
        /* Already on kernel stack */                   \
        stwu %r1, -FRAME_SIZE_FOR_INT(%r1);             \
        b       2f;                                     \
1:      /* User stack right now          */             \
        mr %r4, %r1;                                    \
        mfsprg %r1, 1;                                  \
        stwu %r4, -FRAME_SIZE_FOR_INT(%r1);/*save it*/  \
2:      SAVE_REGS_FOR_INT;                              \
        mfsrr0 %r3; /* EA of exception */               \
        mr	%r3, %r1;

#define EXCEPTION_PROLOGUE_FOR_GDB                      \
        mtsprg  3,%r3;                                  \
        mtsprg  2,%r4;                                  \
        mfcr    %r3;                                    \
        /* Enable floating point bit in msr */          \
        mfmsr %r4;                                      \
        ori %r4,%r4,MSR_FP;                             \
        mtmsr %r4;                                      \
        mfsrr1  %r4;                                    \
        andi.   %r4,%r4,MSR_PR;                         \
        bne     1f;                                     \
        /* Already on kernel stack */                   \
        stwu %r1, -FRAME_SIZE_FOR_INT(%r1);             \
        b       2f;                                     \
1:      /* User stack right now          */             \
        mr %r4, %r1;                                    \
        mfsprg %r1, 1;                                  \
        stwu %r4, -FRAME_SIZE_FOR_INT(%r1);/*save it*/  \
2:      SAVE_REGS_FOR_INT;                              \
        lis %r5, global_thread_stack@ha;              \
        addi %r5, %r5, global_thread_stack@l;         \
        stw %r1, 0(r5);                                 \
        mfsrr0 %r3; /* EA of exception */               

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
        /* Call C function */
        bl pok_boot
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

    START_EXCEPTION(pok_int_critical_input)
        EXCEPTION_PROLOGUE_FOR_GDB
        bl pok_int_critical_input
        b pok_arch_rfi_for_int

    START_EXCEPTION(pok_int_machine_check)
        EXCEPTION_PROLOGUE_FOR_GDB
        bl pok_int_machine_check
        b pok_arch_rfi_for_int

    START_EXCEPTION(pok_int_data_storage)
        EXCEPTION_PROLOGUE_FOR_GDB
        mr %r3, %r1

        mfspr %r4, SPRN_DEAR // DEAR - faulting address
        mfspr %r5, SPRN_ESR // ESR - exception syndrome

        bl pok_int_data_storage
        b  pok_arch_rfi_for_int

    START_EXCEPTION(pok_int_inst_storage)
        EXCEPTION_PROLOGUE_FOR_GDB
        mr %r3, %r1
        mfspr %r4, SPRN_DEAR // DEAR - faulting address
        mfspr %r5, SPRN_ESR // ESR - exception syndrome

        bl pok_int_inst_storage
        b  pok_arch_rfi_for_int

    START_EXCEPTION(pok_int_ext_interrupt)
        EXCEPTION_PROLOGUE_FOR_GDB

        bl pok_int_ext_interrupt
        b pok_arch_rfi_for_int

    START_EXCEPTION(pok_int_alignment)
        EXCEPTION_PROLOGUE_FOR_GDB

        bl pok_int_alignment
        b pok_arch_rfi_for_int

    START_EXCEPTION(pok_int_program)
        EXCEPTION_PROLOGUE_FOR_INT

        bl pok_int_program
        b pok_arch_rfi_for_int

    START_EXCEPTION(pok_int_fp_unavail)
        EXCEPTION_PROLOGUE_FOR_GDB

        bl pok_int_fp_unavail
        b pok_arch_rfi_for_int

    START_EXCEPTION(pok_int_system_call)
        EXCEPTION_PROLOGUE_FOR_GDB

        /* load system call arguments back from the stack */
        lwz     %r4,20(%r1)
        lwz     %r5,24(%r1)
        lwz     %r6,28(%r1)
        lwz     %r7,32(%r1)
        lwz     %r8,36(%r1)
        lwz     %r9,40(%r1)

        bl      pok_int_system_call 
        stw     %r3,20(%r1)
        
        b       pok_arch_rfi_for_int       

    START_EXCEPTION(pok_int_decrementer)
        EXCEPTION_PROLOGUE_FOR_GDB

        bl      pok_int_decrementer 
        b       pok_arch_rfi_for_int

    START_EXCEPTION(pok_int_interval_timer)
        EXCEPTION_PROLOGUE_FOR_GDB

        bl      pok_int_interval_timer
        b       pok_arch_rfi_for_int

    START_EXCEPTION(pok_int_watchdog)
        EXCEPTION_PROLOGUE_FOR_GDB

        bl      pok_int_watchdog
        b       pok_arch_rfi_for_int

    START_EXCEPTION(pok_int_data_tlb_miss)
        EXCEPTION_PROLOGUE_FOR_GDB
        mr %r3, %r1
        mfspr %r4, SPRN_DEAR // DEAR - faulting address
        mfspr %r5, SPRN_ESR // ESR - exception syndrome

        bl      pok_int_data_tlb_miss
        b       pok_arch_rfi_for_int

    START_EXCEPTION(pok_int_inst_tlb_miss)
        EXCEPTION_PROLOGUE_FOR_GDB
        mr %r3, %r1
        mfspr %r4, SPRN_DEAR // DEAR - faulting address
        mfspr %r5, SPRN_ESR // ESR - exception syndrome

        bl      pok_int_inst_tlb_miss
        b       pok_arch_rfi_for_int
        
    START_EXCEPTION(pok_int_debug)
        EXCEPTION_PROLOGUE_FOR_DEBUG

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
