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

/* Generic Interrupt Controller (GIC) */

#define SCU_BASE 0xa00000 //Snoop Control Unit address
#define GIC_CPU_BASE  (SCU_BASE + 0x100) //GIC 'CPU Interface' registers
#define GIC_DIST_BASE (SCU_BASE + 0x1000) //GIC 'Distributor' registers

#define GICD_CTLR (GIC_DIST_BASE + 0x0)

#define GICD_ISENABLER0 (GIC_DIST_BASE + 0x100)
#define GICD_ITARGETSR0 (GIC_DIST_BASE + 0x800)

#define GICC_CTLR (GIC_CPU_BASE + 0x0)
#define GICC_PMR (GIC_CPU_BASE + 0x4) // interrupt priority mask
#define GICC_IAR (GIC_CPU_BASE + 0xC) // interrupt acknoledge register (read only)
#define GICC_EOIR (GIC_CPU_BASE + 0x10) // end of interrupt register (write only)

#define PRIORITY_MASK 128

#define CTLR_EN (0x1)

#include <libc.h>
#include <arch/ioports.h>
#include "irq.h"
#include <core/debug.h>
#include "timer.h"

void irq_handle()
{
    int interrupt_id = ioread32(GICC_IAR);
    if (interrupt_id == IRQ_GPT) {
        iowrite32(GICC_EOIR, interrupt_id);
        timer_handle_interrupt();
    } else {
        pok_fatal("Unknown interrupt");
    }
}

static inline void set_bits(uintptr_t reg, uint32_t bits)
{
    iowrite32(reg, ioread32(reg)|bits);
}

void irq_enable_interrupt(unsigned irq)
{
    uint32_t isenablern = GICD_ISENABLER0 + 4*(irq/32);
    set_bits(isenablern, 1<<(irq%32)); //enable 87 irq

    uint32_t itargetsrn = GICD_ITARGETSR0 + 4*(irq/4);
    set_bits(itargetsrn, 0b00000001<<(irq%4)); //route 87 irq to cpu interface 0

    //default priority val = 0 (max)
}

void irq_init(void)
{
    printf("%s\n", __func__);

    iowrite32(GICD_CTLR, CTLR_EN);
    iowrite32(GICC_CTLR, CTLR_EN);
    iowrite32(GICC_PMR, PRIORITY_MASK);
}
