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
#define SAVE_REGS                                \
        /* Establish new frame.  */              \
        mflr    %r4;                             \
        stw     %r4,FRAME_SIZE+4(%r1);  /* lr */ \
        mfctr   %r4;                             \
        stw     %r4,64(%r1);  /* ctr */          \
        mfxer   %r4;                             \
        stw     %r4,68(%r1);  /* xer */          \
        stw     %r3,8(%r1);   /* cr */           \
        stw     %r0,12(%r1);  /* r0 */           \
        stw     %r2,16(%r1);  /* r2 */           \
        mfsprg  %r0,3;                           \
        stw     %r0,20(%r1);  /* r3 */           \
        mfsprg  %r0,2;                           \
        stw     %r0,24(%r1);  /* r4 */           \
        stw     %r5,28(%r1);  /* r5 */           \
        stw     %r6,32(%r1);  /* r6 */           \
        stw     %r7,36(%r1);  /* r7 */           \
        stw     %r8,40(%r1);  /* r8 */           \
        stw     %r9,44(%r1);  /* r9 */           \
        stw     %r10,48(%r1); /* r10 */          \
        stw     %r11,52(%r1); /* r11 */          \
        stw     %r12,56(%r1); /* r12 */          \
        stw     %r13,60(%r1); /* r13 */          \
        SAVE_FP(0); /* f0 */                     \
        mfsrr0  %r3;                             \
        stw     %r3,72(%r1);  /* srr0 */         \
        mfsrr1  %r4;                             \
        stw     %r4,76(%r1)  /* srr1 */         
        
        
        
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
