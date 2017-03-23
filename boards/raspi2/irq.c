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

#include <libc.h>
#include <bsp/ioports.h>
#include "irq.h"
#include <core/debug.h>
#include "timer.h"

/* BCM2836 (raspberry pi 2) local interrupt controller */

#define CORE0_TIMER_IRQCNTL 0x40000040
#define CORE1_TIMER_IRQCNTL 0x40000044
#define CORE2_TIMER_IRQCNTL 0x40000048
#define CORE3_TIMER_IRQCNTL 0x4000004C

#define TIMER0_IRQ 0x01
#define TIMER1_IRQ 0x02
#define TIMER2_IRQ 0x04
#define TIMER3_IRQ 0x08

#define IRQCNTL_ALL_IRQ (TIMER0_IRQ|TIMER1_IRQ|TIMER2_IRQ|TIMER3_IRQ)


void irq_handle()
{
    timer_handle_interrupt();
}

void irq_init(void)
{
    iowrite32(CORE0_TIMER_IRQCNTL, IRQCNTL_ALL_IRQ);
}
