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
#include <asp/entries.h>
#include <assert.h>

void mmu_enable(void); //FIXME DELETEME
//FIXME
void timer_init(void);

void jet_arch_init(void)
{
    jet_console_init_all ();

    printf("Hello world\n");
    timer_init();

    mmu_enable();
    printf("Hello MMU world\n");
}

void ja_preempt_disable (void)
{
    //assert(0);
}

void ja_preempt_enable (void)
{
    //assert(0);
}

pok_bool_t ja_preempt_enabled(void)
{
    //assert(0);
    return 0;
}

void ja_inf_loop(void)
{
    while (1);
}

void ja_cpu_reset(void)
{
    assert(0);
}
