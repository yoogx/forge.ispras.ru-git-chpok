/*  
 *  Copyright (C) 2015 Maxim Malkov, ISPRAS <malkov@ispras.ru> 
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
#include <libc.h>
#include <bsp.h>

#include "space.h"
#include "timer.h"
#include "syscalls.h"
#include "fpscr.h"
#include "esr.h"


void pok_int_critical_input(uintptr_t ea) {
    (void) ea;
    pok_fatal("Critical input interrupt"); 
}

void pok_int_machine_check(uintptr_t ea) {
    (void) ea;
    pok_fatal("Machine check interrupt"); 
}

void pok_int_data_storage(uintptr_t ea, uintptr_t dear, unsigned long esr) {
    (void) ea;
    if (dear == (uintptr_t) NULL) {
		POK_ERROR_CURRENT_THREAD(POK_ERROR_KIND_MEMORY_VIOLATION);
		return;
	}
    pok_arch_handle_page_fault(dear, esr);
}

void pok_int_inst_storage(uintptr_t ea, uintptr_t dear, unsigned long esr) {
    (void) ea;
    if (ea == (uintptr_t) NULL) {
		POK_ERROR_CURRENT_THREAD(POK_ERROR_KIND_MEMORY_VIOLATION);
		return;
	}
    pok_arch_handle_page_fault(dear, esr);
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
extern void * pok_trap;

void pok_int_program(struct regs * ea, unsigned long esr) {
	printf("%s: ea->fpscr: 0x%lx\n", __func__, ea->fpscr);
	printf("%s: ea->xer: 0x%lx\n", __func__, ea->xer);
	printf("%s: esr: 0x%lx\n", __func__, esr);
	
	if ((esr & ESR_PIL) || (esr & ESR_PTR)) {
		printf("%s: esr: illegal instruction exception\n", __func__);
		POK_ERROR_CURRENT_THREAD(POK_ERROR_KIND_ILLEGAL_REQUEST);
		return;
	}
	
	if (((ea->fpscr & FPSCR_ZE) && (ea->fpscr & FPSCR_ZX)) ||
		((ea->fpscr & FPSCR_OE) && (ea->fpscr & FPSCR_OX)) ||
		((ea->fpscr & FPSCR_UE) && (ea->fpscr & FPSCR_UX)) ||
		((ea->fpscr & FPSCR_VE) && (ea->fpscr & FPSCR_VX)) ||
		((ea->fpscr & FPSCR_XE) && (ea->fpscr & FPSCR_XX)))
	{
		printf("%s: numeric exception!\n", __func__);
		POK_ERROR_CURRENT_THREAD(POK_ERROR_KIND_NUMERIC_ERROR);
		/* Step over the instruction that caused exception
		 * so that CPU won't retry it
		 */
		ea->srr0 += 4;
		return;
	}

//// pok_trap_addr = address of pok_trap in entry.S

    if (ea->srr0 == (unsigned) (& pok_trap_addr)){
        k++;
        handle_exception(17, ea);
    } else {
        handle_exception(3, ea);
    }

    printf("\n          Exit from handle exception\n");

    if (k == 1){
/*
 * it was a trap from gdb.c (in gdb.c function)
 */ 
        ea->srr0+=4;
    }    
    k=0;
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

void pok_int_data_tlb_miss(uintptr_t ea, uintptr_t dear, unsigned long esr) {
    (void) ea;
    pok_arch_handle_page_fault(dear, esr);
}

void pok_int_inst_tlb_miss(uintptr_t ea, uintptr_t dear, unsigned long esr) {
    (void) ea;
    pok_arch_handle_page_fault(dear, esr);
}

void pok_int_debug(uintptr_t ea) {
    (void) ea;
    pok_fatal("Debug interrupt");
}
