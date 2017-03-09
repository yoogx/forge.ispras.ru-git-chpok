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

#ifndef __ASSEMBLER__
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
#endif // __ASSEMBLER__

#define CPSR_IRQ (1<<7)

#define CPSR_MODE_USR 0x10 //User
#define CPSR_MODE_FIQ 0x11 //Fast IRQ
#define CPSR_MODE_IRQ 0x12 // IRQ
#define CPSR_MODE_SVC 0x13 // Supervisor
#define CPSR_MODE_MON 0x16 // Monitor
#define CPSR_MODE_ABT 0x17 // Abort
#define CPSR_MODE_HYP 0x1a // Hyp
#define CPSR_MODE_UND 0x1b // Undef
#define CPSR_MODE_SYS 0x1f // System

#endif //__ARM_REGS_H__
