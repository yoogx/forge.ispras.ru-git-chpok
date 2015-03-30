#include "msr.h"
#include "reg.h"
#include "exceptions.h"

        .section ".start", "ax"

base = .
        
reserved_0000:
        /* fails */
1:      b 1b 
        
        /* Restore registers */
        .globl pok_arch_rfi
pok_arch_rfi:
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
        lwz     %r0,84(%r1)
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
        addi    %r0,%r1,FRAME_SIZE
        mtsprg  1,%r1
1:      lwz     %r0,8(%r1)
        mtcr    %r0
        lwz     %r0,12(%r1)
        lwz     %r1,0(%r1)
        rfi

failed: b failed

        . = base + 0x100
        .globl _pok_reset
system_reset_0100:
_pok_reset:
        /* Setup stack */
        lis %r1,(pok_stack_end-4)@h
        ori %r1,%r1,(pok_stack_end-4)@l

        bl pok_setup_interrupts

        /* Clear back chain */
        xor %r0,%r0,%r0
        stw %r0,4(%r1)

        /* TODO: copy .sdata, clear .bss */
        /* Copy .data */
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

        /* Call C function */
        bl pok_boot
1:      b  1b

/* Interrupt vectors */
/* TODO all interrupt vectors should handle their interrupts */
/* TODO handle kernel / user stack transition correctly */

    STUB_EXCEPTION(pok_int_critical_input)
    STUB_EXCEPTION(pok_int_machine_check)

    START_EXCEPTION(pok_int_data_storage)
        
        // save r4 and r3
        mtsprg  3,%r3
        mtsprg  2,%r4
        
        // save all of them
        stwu    %r1,-FRAME_SIZE(%r1)
        SAVE_REGS
        
        mfspr %r3, SPRN_DEAR // DEAR - faulting address
        mfspr %r4, SPRN_ESR // ESR - exception syndrome

        bl pok_arch_handle_page_fault
        b  pok_arch_rfi

    START_EXCEPTION(pok_int_inst_storage)
        b pok_int_data_storage

    STUB_EXCEPTION(pok_int_ext_interrupt)
    STUB_EXCEPTION(pok_int_alignment)
    STUB_EXCEPTION(pok_int_program)
    STUB_EXCEPTION(pok_int_fp_unavail)

    START_EXCEPTION(pok_int_system_call)
        mtsprg  3,%r3
        mtsprg  2,%r4
        mfcr    %r3
        mfsrr1  %r4
        andi.   %r4,%r4,MSR_PR
1:      beq     1b /* SC musn't appear in supervisor mode.  */

        /* Switch to kernel stack.  */
        mr      %r4,%r1
        mfsprg  %r1,1
        stwu    %r4,-FRAME_SIZE(%r1)

        SAVE_REGS
        /* load system call arguments back from the stack */
        lwz     %r3,20(%r1)
        lwz     %r4,24(%r1)
        lwz     %r5,28(%r1)
        lwz     %r6,32(%r1)
        lwz     %r7,36(%r1)
        lwz     %r8,40(%r1)

        /* Enable interrupt during syscalls. XXX or not? */
        #if 0
        mfmsr   %r0
        ori     %r0,%r0,MSR_EE
        mtmsr   %r0
        #endif

        bl      pok_arch_sc_int
        stw     %r3,20(%r1)
        
        b       pok_arch_rfi       

    STUB_EXCEPTION(pok_int_decrementer)
    STUB_EXCEPTION(pok_int_interval_timer)
    STUB_EXCEPTION(pok_int_watchdog)

    START_EXCEPTION(pok_int_data_tlb_miss)
        b pok_int_data_storage

    START_EXCEPTION(pok_int_inst_tlb_miss)
        b pok_int_inst_storage
        
    STUB_EXCEPTION(pok_int_debug)

#if 0
int_critical_input:
        b int_ext_interrupt

int_machine_check:
        b int_machine_check

int_data_storage:
        b int_data_storage

int_inst_storage:
        b int_inst_storage

int_ext_interrupt:
        /* PC saved in srr0,
           MSR saved in srr1.  */
        /* Can use sprg.  */
        mtsprg  3,%r3
        mtsprg  2,%r4
        mfcr    %r3
        mfsrr1  %r4
        andi.   %r4,%r4,MSR_PR
1:      bne     1b /* Need to switch to system stack */

        stwu    %r1,-FRAME_SIZE(%r1)
        
        SAVE_REGS

        bl      pok_arch_ext_interrupt
        b       pok_arch_rfi

int_alignment:
        b int_alignment

int_program:
        b int_program

int_fp_unavail:
        b int_fp_unavail

int_system_call:
        mtsprg  3,%r3
        mtsprg  2,%r4
        mfcr    %r3
        mfsrr1  %r4
        andi.   %r4,%r4,MSR_PR
1:      beq     1b /* SC musn't appear in supervisor mode.  */

        /* Switch to kernel stack.  */
        mr      %r4,%r1
        mfsprg  %r1,1
        stwu    %r4,-FRAME_SIZE(%r1)

        SAVE_REGS
        lwz     %r3,20(%r1)
        lwz     %r4,24(%r1)
        lwz     %r5,28(%r1)
        lwz     %r6,32(%r1)
        lwz     %r7,36(%r1)
        lwz     %r8,40(%r1)

        /* Enable interrupt during syscalls.  */
        mfmsr   %r0
        ori     %r0,%r0,MSR_EE
        mtmsr   %r0

        bl      pok_arch_sc_int
        stw     %r3,20(%r1)
        
        b       pok_arch_rfi

int_decrementer:
        /* PC saved in srr0,
           MSR saved in srr1.  */
        /* Can use sprg.  */
        mtsprg  3,%r3
        mtsprg  2,%r4
        mfcr    %r3
        mfsrr1  %r4
        andi.   %r4,%r4,MSR_PR
1:      bne     1b /* Need to switch to system stack */

        stwu    %r1,-FRAME_SIZE(%r1)
        
        SAVE_REGS

        bl      pok_arch_decr_int
        b       pok_arch_rfi

int_interval_timer:
        b int_interval_timer

int_watchdog:
        b int_watchdog

int_data_tlb_miss:
        b int_data_tlb_miss

int_inst_tlb_miss:
        b int_inst_tlb_miss

int_debug:
        b int_debug

#endif

pok_setup_interrupts:
        lis     %r3, pok_int_critical_input@h // higher 16 bits
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
        .space 8 * 1024
pok_stack_end:

pok_save_area:
        .space 4 * 8
