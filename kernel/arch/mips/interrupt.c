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
#include "msr.h"

#include "space.h"
#include "timer.h"
#include "syscalls.h"
#include "interrupt_context.h"

#ifdef POK_NEEDS_GDB
#include "gdb.h"
int k=0;

extern void * pok_trap_addr;
#endif

void pok_int_ri(struct jet_interrupt_context* ea) {
    (void) ea;
    printf("DEBUG: EPC    = 0x%x\n", ea->EPC);
    printf("DEBUG: Cause  = 0x%x\n", ea->CAUSE);
    printf("DEBUG: Status = 0x%x\n", ea->STATUS);
    pok_fatal("Unrealized instruction"); 
}

void pok_int_alignment(struct jet_interrupt_context* vctx, uintptr_t dear, unsigned long esr)
{		
    (void) vctx;
    (void) dear;
    (void) esr;
    pok_fatal("Alignment interrupt");	
}

void write_on_screen();


void pok_int_fp_unavail(struct jet_interrupt_context* ea) {
    (void) ea;
    pok_fatal("FP unavailable interrupt");
}

void pok_int_none(struct jet_interrupt_context* ea) {
    (void) ea;
    printf("DEBUG: EPC   = 0x%x\n", ea->EPC);
    printf("DEBUG: Cause = 0x%x\n", ea->CAUSE);
    pok_fatal("Unknown interrupt :(");
}

void pok_int_overflow(struct jet_interrupt_context* ea) {
    (void) ea;
    printf("DEBUG: EPC   = 0x%x\n", ea->EPC);
    printf("DEBUG: Cause = 0x%x\n", ea->CAUSE);
    while (1 == 1){
    }
    pok_fatal("OVF Arithmetic overflow");
}

void pok_int_addrl(struct jet_interrupt_context* ea) {
    (void) ea;
    printf("DEBUG: EPC   = 0x%x\n", ea->EPC);
    printf("DEBUG: Cause = 0x%x\n", ea->CAUSE);
    pok_fatal("ADDRL Load from an illegal address");
}

void pok_int_addrs(struct jet_interrupt_context* ea) {
    (void) ea;
    printf("DEBUG: EPC   = 0x%x\n", ea->EPC);
    printf("DEBUG: Cause = 0x%x\n", ea->CAUSE);
    pok_fatal("ADDRS Store to an illegal address");
}

void pok_int_ibus(struct jet_interrupt_context* ea) {
    (void) ea;
    printf("DEBUG: EPC   = 0x%x\n", ea->EPC);
    printf("DEBUG: Cause = 0x%x\n", ea->CAUSE);
    pok_fatal("IBUS Bus error on instruction fetch");
}

void pok_int_dbus(struct jet_interrupt_context* ea) {
    (void) ea;
    printf("DEBUG: EPC   = 0x%x\n", ea->EPC);
    printf("DEBUG: Cause = 0x%x\n", ea->CAUSE);
    pok_fatal("DBUS Bus error on data reference");
}

unsigned long pok_int_system_call(struct jet_interrupt_context* ea,
    unsigned long arg1, unsigned long arg2, unsigned long arg3, unsigned long arg4, unsigned long arg5, unsigned long arg6) {
    //~ printf("DEBUG: SYCALL %d!\n", (int) arg1);
    //~ printf("DEBUG: EPC   = 0x%x\n", ea->EPC);
    //~ printf("DEBUG: Cause = 0x%x\n", ea->CAUSE);
    //~ printf("DEBUG: RA    = 0x%x\n", ea->r31);
    (void) ea;
    return pok_arch_sc_int(arg1, arg2, arg3, arg4, arg5, arg6);
}

void pok_int_decrementer(struct jet_interrupt_context* ea) {
    //~ printf("DECREMENTER: Count = 0x%lx\n", mfc0(CP0_COUNT));
    //~ printf("DECREMENTER: sizeof = 0x%x\n", sizeof(struct jet_interrupt_context));
    (void) ea;
    pok_arch_decr_int();
}

void pok_int_data_tlb_miss(struct jet_interrupt_context* vctx, uintptr_t dear, unsigned long esr) {
    printf("DEBUG: EPC    = 0x%x\n", vctx->EPC);
    printf("DEBUG: Cause  = 0x%x\n", vctx->CAUSE);
    printf("DEBUG: Status = 0x%x\n", vctx->STATUS);
    printf("TLB error\n");
    pok_arch_handle_page_fault(vctx, dear, esr, PF_DATA_TLB_MISS);
}

void pok_int_inst_tlb_miss(struct jet_interrupt_context* vctx, uintptr_t dear, unsigned long esr) {
    printf("DEBUG: EPC    = 0x%x\n", vctx->EPC);
    printf("DEBUG: Cause  = 0x%x\n", vctx->CAUSE);
    printf("DEBUG: Status = 0x%x\n", vctx->STATUS);
    printf("TLB error\n");
    pok_arch_handle_page_fault(vctx, dear, esr, PF_INST_TLB_MISS);
}

void pok_int_debug(struct jet_interrupt_context* ea) {
    printf("DEBUG: EPC    = 0x%x\n", ea->EPC);
    printf("DEBUG: Cause  = 0x%x\n", ea->CAUSE);
    printf("DEBUG: Status = 0x%x\n", ea->STATUS);
    pok_fatal("BKPT break instruction executed");
}
