/*
 * Institute for System Programming of the Russian Academy of Sciences
 * Copyright (C) 2017 ISPRAS
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

#include <arch/ioports.h>
#include <libc.h>
#include "irq.h"
#include <asp/time.h>
#include <core/time.h>
#include <assert.h>
#include <asp/entries.h>
#include <common.h>

/* The Generic Timer BCM2836 specific part */

#define CONTROL 0x40000000 // Control register
#define CORE_TIMER_PRESCALER 0x40000008


void bcm2836_init_timer(void)
{
    /*
     * Set the timer to source from the crystal clock (bit
     * 8 unset), and only increment by 1 instead of 2 (bit 9
     * unset).
     */
    iowrite32(CONTROL, 0);

    /*
     * Set the timer prescaler to 1:1 (timer freq = input freq *
     * 2**31 / prescaler)
     */
    iowrite32(CORE_TIMER_PRESCALER, 0x80000000);
}

