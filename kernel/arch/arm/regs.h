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

#ifndef __ARM_REGS_H__
#define __ARM_REGS_H__

static inline uint32_t cpsrget()
{
    uint32_t r;
    asm("mrs %0, cpsr" : "=r" (r));
    return r;
}

static inline void cpsrset(uint32_t r)
{
    asm("msr cpsr, %0" : : "r" (r));
}

#define CPSR_IRQ (1<<7)

#endif //__ARM_REGS_H__
