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

void pok_int_program(uintptr_t ea) {
    (void) ea;
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
    printf("\n\n\n\n\n\n DEBUG INTERRUPT \n\n\n\n\n\n\n");

    pok_fatal("Debug interrupt");
}
