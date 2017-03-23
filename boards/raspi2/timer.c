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

#include <bsp/ioports.h>
#include <libc.h>
#include "irq.h"
#include <asp/time.h>
#include <assert.h>
#include <asp/entries.h>

/* The Generic Timer */
/* Timer has physical and virtual counters. AFAIU QEMU doesn't check
 * CNTHCTL/CNTKCTL registers, that protects access to physical counter. So I
 * will user virtual counter
 */

uint32_t frequency;
#define NANO 1000000000

/*
 * Hardcoded calendar time at the beginning of the OS loading.
 *
 * Could be obtained on https://www.timeanddate.com/.
 */
static time_t base_calendar_time = 1480330081; // On 28.11.2016

static uint64_t vcounter_get(void)
{
    uint32_t low, high;
    asm volatile ("mrrc p15, 0, %0, %1, c14": "=r"(low), "=r"(high)::"memory");
    return (((uint64_t)(high) << 32) + low);
}

static uint64_t frequency_get(void)
{
    uint32_t freq;
    asm volatile("mrc p15, 0, %0, c14, c0, 0": "=r"(freq)::"memory");
    return freq;
}

void timer_init(void)
{
    frequency = frequency_get();
    printf("Timer frequency = %ld Hz\n", frequency);

    uint32_t rem = NANO % frequency;
    if (rem != 0) {
        printf("WARNING: timer counting is not precise if with current frequency\n");
    }
}

/* Return current system time. */
pok_time_t ja_system_time(void)
{
    return (pok_time_t)(vcounter_get() * (NANO / frequency));
}

/* Return current calendar time (seconds since Epoch). */
time_t ja_calendar_time(void)
{
    return base_calendar_time + (time_t)(ja_system_time() / 1000000000);
}
