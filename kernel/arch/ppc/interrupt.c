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
#include <core/sched.h>
#include <core/error.h>
#include <core/error_arinc.h>
#include <libc.h>
#include <bsp_common.h>
#include "reg.h"

#include "space.h"
#include "timer.h"
#include "syscalls.h"
#include "fpscr.h"
#include "esr.h"

#include "thread.h"

void pok_int_critical_input(uintptr_t ea) {
    (void) ea;
    pok_fatal("Critical input interrupt"); 
}

void pok_int_machine_check(uintptr_t ea) {
    (void) ea;
    pok_fatal("Machine check interrupt"); 
}

void pok_int_data_storage(volatile_context_t *vctx, uintptr_t dear, unsigned long esr) {
    pok_arch_handle_page_fault(vctx, dear, esr, PF_DATA_STORAGE);
}

void pok_int_inst_storage(volatile_context_t *vctx, uintptr_t dear, unsigned long esr) {
    pok_arch_handle_page_fault(vctx, dear, esr, PF_INST_STORAGE);
}

void pok_int_ext_interrupt(uintptr_t ea) {
    (void) ea;
    pok_fatal("External interrupt");
}

void pok_int_alignment(uintptr_t ea) {
    (void) ea;
    pok_fatal("Alignment interrupt");
}

int k=0;

extern void * pok_trap_addr;
void write_on_screen();

void pok_int_program(struct regs * ea) {
    printf("%s: ea->fpscr: 0x%lx\n", __func__, ea->fpscr);
	printf("%s: ea->xer: 0x%lx\n", __func__, ea->xer);

    if (((ea->fpscr & FPSCR_ZE) && (ea->fpscr & FPSCR_ZX)) ||
		((ea->fpscr & FPSCR_OE) && (ea->fpscr & FPSCR_OX)) ||
		((ea->fpscr & FPSCR_UE) && (ea->fpscr & FPSCR_UX)) ||
		((ea->fpscr & FPSCR_VE) && (ea->fpscr & FPSCR_VX)) ||
		((ea->fpscr & FPSCR_XE) && (ea->fpscr & FPSCR_XX)))
	{
		printf("%s: numeric exception!\n", __func__);
        pok_raise_error(POK_ERROR_KIND_NUMERIC_ERROR, 1, NULL);
		// Step over the instruction that caused exception
        // so that CPU won't retry it
		ea->srr0 += 4;
		return;
	}

//// pok_trap_addr = address of pok_trap in entry.S
#ifdef DEBUG_GDB
    printf("    Pok_int_program interrupt\n");
    int DBCR0 = mfspr(SPRN_DBCR0);
    printf("DBCR0 = 0x%x\n", DBCR0);
    printf("DBSR = %lx\n", mfspr(SPRN_DBSR));
    printf("DAC1 = %lx\n", mfspr(SPRN_DAC1));
    printf("DAC2 = %lx\n", mfspr(SPRN_DAC1));
    printf("srr0 = 0x%lx\n", ea->srr0);
    printf("instr = 0x%lx\n", *(uint32_t *)ea->srr0);
#endif

    if (ea->srr0 == (unsigned) (& pok_trap_addr)) {
        k++;
#ifdef DEBUG_GDB
        printf("Reason: SIGINT\n");
#endif
        handle_exception(17, ea); 
    } else {
#ifdef DEBUG_GDB
        printf("Reason: Breakpoint\n");
#endif
        handle_exception(3, ea); 
    }

    if (k == 1) {
/*
 * it was a trap from gdb.c (in gdb.c function)
 */ 
        ea->srr0 += 4;
#ifdef DEBUG_GDB
        printf("Change SRR0");
#endif
    }
    k=0;

#ifdef DEBUG_GDB
    printf("srr0 = 0x%lx\n", ea->srr0);
    printf("instr = 0x%lx\n", *(uint32_t *)ea->srr0);
    //~ asm volatile("isync");
    printf("instr = 0x%lx\n", *(uint32_t *)(ea->srr0));
    DBCR0 = mfspr(SPRN_DBCR0);
    printf("DBCR0 = 0x%x\n", DBCR0);
    printf("\n          Exit from handle exception\n");
#endif
    
//~ asm volatile("acbi");  
    
////    pok_fatal("Program interrupt");
}

void pok_int_fp_unavail(uintptr_t ea) {
    (void) ea;
    pok_fatal("FP unavailable interrupt");
}

unsigned long pok_int_system_call(uintptr_t ea, unsigned long arg1, unsigned long arg2, unsigned long arg3, unsigned long arg4, unsigned long arg5, unsigned long arg6) {
    (void) ea;
    return pok_arch_sc_int(arg1, arg2, arg3, arg4, arg5, arg6);
}

void pok_int_decrementer(uintptr_t ea) {
    (void) ea;
    pok_arch_decr_int();
}

void pok_int_interval_timer(uintptr_t ea) {
    (void) ea;
    pok_fatal("Interval timer interrupt");
}

void pok_int_watchdog(uintptr_t ea) {
    (void) ea;
    pok_fatal("Watchdog interrupt");
}

void pok_int_data_tlb_miss(volatile_context_t *vctx, uintptr_t dear, unsigned long esr) {
    pok_arch_handle_page_fault(vctx, dear, esr, PF_DATA_TLB_MISS);
}

void pok_int_inst_tlb_miss(volatile_context_t *vctx, uintptr_t dear, unsigned long esr) {
    pok_arch_handle_page_fault(vctx, dear, esr, PF_INST_TLB_MISS);
}

void pok_int_debug(struct regs * ea) {
#ifdef DEBUG_GDB
    printf("    DEBUG EVENT!\n");
    printf("DBSR = %lx\n", mfspr(SPRN_DBSR));
    printf("ea = 0x%lx\n", (uint32_t) ea);
    int DBCR0 = mfspr(SPRN_DBCR0);
    printf("DBCR0 = 0x%x\n", DBCR0);
    printf("DAC1 = %lx\n", mfspr(SPRN_DAC1));
    printf("DAC2 = %lx\n", mfspr(SPRN_DAC2));
    printf("srr0 = 0x%lx\n", ea->srr0);
    printf("srr1 = 0x%lx\n", ea->srr1);
    printf("Reason: Watchpoint\n");   
#endif
    handle_exception(1, ea); 
#ifdef DEBUG_GDB
    printf("instr = 0x%lx\n", *(uint32_t *)ea->srr0);
    DBCR0 = mfspr(SPRN_DBCR0);
    printf("DBCR0 = 0x%x\n", DBCR0);
    asm volatile("dcbst 0, %0; sync; icbi 0,%0; sync; isync" : : "r" ((char *) ea->srr0));
    printf("Exit from debug event\n");    
    //~ k = 1;
    //~ pok_fatal("Debug interrupt");
#endif
}
