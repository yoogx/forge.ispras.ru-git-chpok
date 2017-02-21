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
/*
For interrupt ID m, when DIV and MOD are the integer division and modulo operations:
• the corresponding GICD_ISENABLER number, n, is given by n = m DIV 32
• the offset of the required GICD_ISENABLER is (0x100 + (4*n))
• the bit number of the required Set-enable bit in this register is m MOD 32.
 */


#define GICC_CTLR (GIC_CPU_BASE + 0x0)
#define GICC_PMR (GIC_CPU_BASE + 0x4)

#define CTLR_EN (0x1)

#include <libc.h>
#include <bsp/ioports.h>

#include <bsp/regs.h>


static void set_bits(uintptr_t reg, uint32_t mask)
{
    iowrite32(reg, ioread32(reg)|mask);
}

void irq_init(void)
{
    irq_enable();
    printf("%s\n", __func__);

    iowrite32(GICD_CTLR, CTLR_EN);
    iowrite32(GICC_CTLR, CTLR_EN);

    iowrite32(GICC_PMR, 128);

    //default priority val = 0 (max)
    uint32_t isenablern = GICD_ISENABLER0 + 4*(87/32);
    set_bits(isenablern, 1<<(87%32)); //enable 87 irq

    uint32_t itargetsrn = GICD_ITARGETSR0 + 4*(87/4);
    set_bits(itargetsrn, 0b00000001<<(87%4)); //route 87 irq to cpu interface 0

}
