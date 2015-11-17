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

#define SAVE_FP(num) stfd %f##num,(80+8*num)(%r1)
#define LOAD_FP(num) lfd %f##num,(80+8*num)(%r1)

//96 = 0x60 = align_up(80 + 8(for f0 reg))
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
        mfsrr0 %r3; /* EA of exception */


failed: b failed

        . = base + 0x100
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
        EXCEPTION_PROLOGUE 
        bl pok_int_critical_input
        b pok_arch_rfi

    START_EXCEPTION(pok_int_machine_check)
        EXCEPTION_PROLOGUE
        bl pok_int_machine_check
        b pok_arch_rfi

    START_EXCEPTION(pok_int_data_storage)
        EXCEPTION_PROLOGUE
        
        mfspr %r4, SPRN_DEAR // DEAR - faulting address
        mfspr %r5, SPRN_ESR // ESR - exception syndrome

        bl pok_int_data_storage
        b  pok_arch_rfi

    START_EXCEPTION(pok_int_inst_storage)
        EXCEPTION_PROLOGUE
        
        mfspr %r4, SPRN_DEAR // DEAR - faulting address
        mfspr %r5, SPRN_ESR // ESR - exception syndrome

        bl pok_int_inst_storage
        b  pok_arch_rfi

    START_EXCEPTION(pok_int_ext_interrupt)
        EXCEPTION_PROLOGUE

        bl pok_int_ext_interrupt
        b pok_arch_rfi

    START_EXCEPTION(pok_int_alignment)
        EXCEPTION_PROLOGUE

        bl pok_int_alignment
        b pok_arch_rfi

    START_EXCEPTION(pok_int_program)
        EXCEPTION_PROLOGUE

        bl pok_int_program
        b pok_arch_rfi

    START_EXCEPTION(pok_int_fp_unavail)
        EXCEPTION_PROLOGUE

        bl pok_int_fp_unavail
        b pok_arch_rfi

    START_EXCEPTION(pok_int_system_call)
		mtsprg  2,%r4
		lis		%r4, 2048
		cmpw	%r3, %r4
		bne		sc_cont
		rfi
		
		sc_cont:
		mfsprg  %r4, 2
        EXCEPTION_PROLOGUE

        /* load system call arguments back from the stack */
        lwz     %r4,20(%r1)
        lwz     %r5,24(%r1)
        lwz     %r6,28(%r1)
        lwz     %r7,32(%r1)
        lwz     %r8,36(%r1)
        lwz     %r9,40(%r1)

        bl      pok_int_system_call 
        stw     %r3,20(%r1)
        
        b       pok_arch_rfi       

    START_EXCEPTION(pok_int_decrementer)
        EXCEPTION_PROLOGUE

        bl      pok_int_decrementer 
        b       pok_arch_rfi

    START_EXCEPTION(pok_int_interval_timer)
        EXCEPTION_PROLOGUE

        bl      pok_int_interval_timer
        b       pok_arch_rfi

    START_EXCEPTION(pok_int_watchdog)
        EXCEPTION_PROLOGUE

        bl      pok_int_watchdog
        b       pok_arch_rfi

    START_EXCEPTION(pok_int_data_tlb_miss)
        EXCEPTION_PROLOGUE
        
        mfspr %r4, SPRN_DEAR // DEAR - faulting address
        mfspr %r5, SPRN_ESR // ESR - exception syndrome

        bl      pok_int_data_tlb_miss
        b       pok_arch_rfi

    START_EXCEPTION(pok_int_inst_tlb_miss)
        EXCEPTION_PROLOGUE

        mfspr %r4, SPRN_DEAR // DEAR - faulting address
        mfspr %r5, SPRN_ESR // ESR - exception syndrome

        bl      pok_int_inst_tlb_miss
        b       pok_arch_rfi
        
    START_EXCEPTION(pok_int_debug)
        EXCEPTION_PROLOGUE

        bl      pok_int_debug
        b       pok_arch_rfi

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
