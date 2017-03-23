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

#include <asp/arch.h>
#include <asp/time.h>
#include <asp/entries.h>
#include <assert.h>
#include <bsp/bsp.h>
#include "regs.h"
#include <arch/mmu.h>
#include "space.h"


void jet_arch_init(void)
{
    jet_console_init_all ();
    printf("Hello world \n");

    copy_vector_table(); //interrupt vec table
    space_init(); //user space init
    ja_bsp_init();

    //ja_preempt_enable();
    //int64_t time, time_old = 0;
    //while (1) {
    //    time = ja_system_time();
    //        //printf("%lld \n", time);
    //    if ((time - time_old) > 1000000000) {
    //        printf("%lld \n", time);
    //        time_old = time;
    //    }
    //}
}

void ja_preempt_disable (void)
{
    cpsr_set(cpsr_get() | (CPSR_IRQ));
}

void ja_preempt_enable (void)
{
    cpsr_set(cpsr_get() & ~(CPSR_IRQ));
}

pok_bool_t ja_preempt_enabled(void)
{
    return !(cpsr_get() & CPSR_IRQ);
}

void ja_inf_loop(void)
{
    while (1);
}

void ja_cpu_reset(void)
{
    assert(0);
}
