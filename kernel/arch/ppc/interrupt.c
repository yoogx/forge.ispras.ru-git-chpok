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
#include <libc.h>


#include "space.h"
#include "timer.h"
#include "syscalls.h"




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
    pok_arch_handle_page_fault(dear, esr);
}

void pok_int_inst_storage(uintptr_t ea, uintptr_t dear, unsigned long esr) {
    (void) ea;
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

void pok_int_program(struct regs * ea) {

////printf("ea = 0x%lx\n", ea);

////    (void) ea;
    k++;
    printf("addr = 0x%lx\n",(uint32_t) ea);
    printf("offset1 = 0x%lx\n",ea->r1);
    printf("offset2 = 0x%lx\n",ea->offset2);
    printf("cr = 0x%lx\n",ea->cr);
    printf("r0 = 0x%lx\n",ea->r0);
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
    printf("r13 = 0x%lx\n",ea->r13);
    printf("ctr = 0x%lx\n",ea->ctr);
    printf("xer = 0x%lx\n",ea->xer);
    printf("srr0 or pc = 0x%lx\n",ea->srr0); 
    printf("srr1 = 0x%lx\n",ea->srr1);
    printf("offset3 = 0x%lx\n",ea->offset3);
    printf("offset4 = 0x%lx\n",ea->offset4);
    printf("offset5 = 0x%lx\n",ea->offset5);
    printf("offset6 = 0x%lx\n",ea->offset6);
    printf("offset7 = 0x%lx\n",ea->offset7);
    printf("lr = 0x%lx\n",ea->lr);
    printf("offset8 = 0x%lx\n",ea->offset8);
    printf("offset9 = 0x%lx\n",ea->offset9);
    printf("offset10 = 0x%lx\n",ea->offset10);
    printf("offset11 = 0x%lx\n",ea->offset11);
    printf("offset12 = 0x%lx\n",ea->offset12);
    printf("offset13 = 0x%lx\n",ea->offset13);
    printf("offset14 = 0x%lx\n",ea->offset14);
    printf("offset15 = 0x%lx\n",ea->offset15);

    handle_exception(17,ea); 

    while (k != 1){
        
    }
    asm("trap");

    pok_fatal("Program interrupt");
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
    pok_fatal("Inteval timer interrupt");
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
