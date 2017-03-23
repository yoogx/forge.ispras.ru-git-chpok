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

void irq_handle()
{
    /*
    int interrupt_id = ioread32(GICC_IAR);
    if (interrupt_id == IRQ_GPT) {
        iowrite32(GICC_EOIR, interrupt_id);
        timer_handle_interrupt();
    } else {
        pok_fatal("Unknown interrupt");
    }
    */
}

static inline void set_bits(uintptr_t reg, uint32_t bits)
{
    iowrite32(reg, ioread32(reg)|bits);
}

void irq_enable_interrupt(unsigned irq)
{
}

void irq_init(void)
{
    printf("%s\n", __func__);
}
