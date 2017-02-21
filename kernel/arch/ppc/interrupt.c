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
 */

/*
 * This file contains entry points to interrupt handlers.
 * They're called from the assembly contained in entry.S.
 *
 */

#include <types.h>
#include <errno.h> 
#include <core/debug.h>
#include <libc.h>
#include "reg.h"

#include "space.h"
#include "timer.h"
#include "syscalls.h"
#include "interrupt_context.h"


#ifdef POK_NEEDS_GDB
#include "gdb.h"
int k=0;

extern void * pok_trap_addr;
#endif

void pok_int_critical_input(struct jet_interrupt_context* ea) {
    (void) ea;
    pok_fatal("Critical input interrupt"); 
}

void pok_int_machine_check(struct jet_interrupt_context* ea) {
    (void) ea;
    pok_fatal("Machine check interrupt"); 
}

void pok_int_data_storage(struct jet_interrupt_context *vctx, uintptr_t dear, unsigned long esr) {
    pok_arch_handle_page_fault(vctx, dear, esr, PF_DATA_STORAGE);
}

void pok_int_inst_storage(struct jet_interrupt_context* vctx, uintptr_t dear, unsigned long esr) {
    pok_arch_handle_page_fault(vctx, dear, esr, PF_INST_STORAGE);
}

void pok_int_ext_interrupt(struct jet_interrupt_context* ea) {
    (void) ea;
    pok_fatal("External interrupt");
}

void pok_int_alignment(struct jet_interrupt_context* vctx, uintptr_t dear, unsigned long esr)
{
    printf("dear = %p\n", (void *)dear);
    printf("esr  = 0x%lx\n", esr);
    printf("cr   = 0x%lx\n", vctx->cr);
    printf("r0   = 0x%lx\n", vctx->r0);
    printf("r1   = 0x%lx\n", vctx->r1);
    printf("r2   = 0x%lx\n", vctx->r2);
    printf("r3   = 0x%lx\n", vctx->r3);
    printf("r4   = 0x%lx\n", vctx->r4);
    printf("r5   = 0x%lx\n", vctx->r5);
    printf("r6   = 0x%lx\n", vctx->r6);
    printf("r7   = 0x%lx\n", vctx->r7);
    printf("r8   = 0x%lx\n", vctx->r8);
    printf("r9   = 0x%lx\n", vctx->r9);
    printf("r10  = 0x%lx\n", vctx->r10);
    printf("r11  = 0x%lx\n", vctx->r11);
    printf("r12  = 0x%lx\n", vctx->r12);
    printf("ctr  = 0x%lx\n", vctx->ctr);
    printf("xer  = 0x%lx\n", vctx->xer);
    printf("srr0 = 0x%lx\n", vctx->srr0);
    printf("srr1 = 0x%lx\n", vctx->srr1);
    printf("lr   = 0x%lx\n", vctx->lr);
    pok_fatal("Alignment interrupt");
}

void pok_int_spe(struct jet_interrupt_context* vctx)
{
    printf("cr   = 0x%lx\n", vctx->cr);
    printf("r0   = 0x%lx\n", vctx->r0);
    printf("r1   = 0x%lx\n", vctx->r1);
    printf("r2   = 0x%lx\n", vctx->r2);
    printf("r3   = 0x%lx\n", vctx->r3);
    printf("r4   = 0x%lx\n", vctx->r4);
    printf("r5   = 0x%lx\n", vctx->r5);
    printf("r6   = 0x%lx\n", vctx->r6);
    printf("r7   = 0x%lx\n", vctx->r7);
    printf("r8   = 0x%lx\n", vctx->r8);
    printf("r9   = 0x%lx\n", vctx->r9);
    printf("r10  = 0x%lx\n", vctx->r10);
    printf("r11  = 0x%lx\n", vctx->r11);
    printf("r12  = 0x%lx\n", vctx->r12);
    printf("ctr  = 0x%lx\n", vctx->ctr);
    printf("xer  = 0x%lx\n", vctx->xer);
    printf("srr0 = 0x%lx\n", vctx->srr0);
    printf("srr1 = 0x%lx\n", vctx->srr1);
    printf("lr   = 0x%lx\n", vctx->lr);
    pok_fatal("SPE interrupt\n");
}


void write_on_screen();


void pok_int_program(struct jet_interrupt_context* ea) {

#ifdef POK_NEEDS_GDB
    printf_GDB("    Pok_int_program interrupt\n");
    printf_GDB("DBCR0 = 0x%lx\n", mfspr(SPRN_DBCR0));
    printf_GDB("DBSR = %lx\n", mfspr(SPRN_DBSR));
    printf_GDB("DAC1 = %lx\n", mfspr(SPRN_DAC1));
    printf_GDB("DAC2 = %lx\n", mfspr(SPRN_DAC1));
    printf_GDB("srr0 = 0x%lx\n", ea->srr0);
    printf_GDB("instr = 0x%lx\n", *(uint32_t *)ea->srr0);
/*
 * pok_trap_addr = address of pok_trap in entry.S
 */
    if (ea->srr0 == (unsigned) (& pok_trap_addr)){
        k++;
        printf_GDB("Reason: SIGINT\n");
        handle_exception(17,ea); 
    }else{
        printf_GDB("Reason: Breakpoint\n");
        handle_exception(3,ea); 
    }
    if (k == 1){
/*
 * it was a trap from gdb.c (in gdb.c function)
 */ 
        ea->srr0 += 4;
        printf_GDB("Change SRR0");
    }
    k=0;
    printf_GDB("srr0 = 0x%lx\n", ea->srr0);
    printf_GDB("instr = 0x%lx\n", *(uint32_t *)ea->srr0);
    //~ asm volatile("isync");
    printf_GDB("instr = 0x%lx\n", *(uint32_t *)(ea->srr0));
    printf_GDB("DBCR0 = 0x%lx\n", mfspr(SPRN_DBCR0));
    printf_GDB("\n          Exit from handle exception\n");
#else
    printf("\n\n            In pok_int_programm:\n");
    printf("addr = 0x%lx\n",(uint32_t) ea);
    printf("cr = 0x%lx\n",ea->cr);
    printf("ctr = 0x%lx\n",ea->ctr);
    printf("xer = 0x%lx\n",ea->xer);
    printf("srr0 or pc = 0x%lx\n",ea->srr0); 
    printf("srr1 = 0x%lx\n",ea->srr1);
    printf("r0 = 0x%lx\n",ea->r0);
    printf("r1 = 0x%lx\n",ea->r1);
    printf("r2 = 0x%lx\n",ea->r2);
    printf("r3 = 0x%lx\n",ea->r3);
    printf("r4 = 0x%lx\n",ea->r4);
    printf("r5 = 0x%lx\n",ea->r5);
    printf("r6 = 0x%lx\n",ea->r6);
    printf("r7 = 0x%lx\n",ea->r7);
    printf("r8 = 0x%lx\n",ea->r8);
    printf("r9 = 0x%lx\n",ea->r9);
    printf("r10 = 0x%lx\n",ea->r10);
    printf("r11 = 0x%lx\n",ea->r11);
    printf("r12 = 0x%lx\n",ea->r12);
    printf("lr = 0x%lx\n",ea->lr);
    pok_fatal("Program interrupt");
#endif

}

void pok_int_fp_unavail(struct jet_interrupt_context* ea) {
    (void) ea;
    pok_fatal("FP unavailable interrupt");
}

unsigned long pok_int_system_call(struct jet_interrupt_context* ea,
    unsigned long arg1, unsigned long arg2, unsigned long arg3, unsigned long arg4, unsigned long arg5, unsigned long arg6) {
    (void) ea;
    return pok_arch_sc_int(arg1, arg2, arg3, arg4, arg5, arg6);
}

void pok_int_decrementer(struct jet_interrupt_context* ea) {
    (void) ea;
    pok_arch_decr_int();
}

void pok_int_interval_timer(struct jet_interrupt_context* ea) {
    (void) ea;
    pok_fatal("Inteval timer interrupt");
}

void pok_int_watchdog(struct jet_interrupt_context* ea) {
    (void) ea;
    pok_fatal("Watchdog interrupt");
}

void pok_int_data_tlb_miss(struct jet_interrupt_context* vctx, uintptr_t dear, unsigned long esr) {
    pok_arch_handle_page_fault(vctx, dear, esr, PF_DATA_TLB_MISS);
}

void pok_int_inst_tlb_miss(struct jet_interrupt_context* vctx, uintptr_t dear, unsigned long esr) {
    pok_arch_handle_page_fault(vctx, dear, esr, PF_INST_TLB_MISS);
}

void pok_int_debug(struct jet_interrupt_context* ea) {

    printf_GDB("    DEBUG EVENT!\n");
    printf_GDB("DBSR = %lx\n", mfspr(SPRN_DBSR));
    printf_GDB("ea = 0x%lx\n", (uint32_t) ea);
    printf_GDB("DBCR0 = 0x%lx\n", mfspr(SPRN_DBCR0));
    printf_GDB("DAC1 = %lx\n", mfspr(SPRN_DAC1));
    printf_GDB("DAC2 = %lx\n", mfspr(SPRN_DAC2));
    printf_GDB("srr0 = 0x%lx\n", ea->srr0);
    printf_GDB("srr1 = 0x%lx\n", ea->srr1);
    printf_GDB("Reason: Watchpoint\n");   

    handle_exception(1, ea); 
    
    printf_GDB("instr = 0x%lx\n", *(uint32_t *)ea->srr0);
    printf_GDB("DBCR0 = 0x%lx\n", mfspr(SPRN_DBCR0));
    asm volatile("dcbst 0, %0; sync; icbi 0,%0; sync; isync" : : "r" ((char *) ea->srr0));
    printf_GDB("Exit from debug event\n"); 
}
