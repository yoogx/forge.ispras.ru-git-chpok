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
static inline uint32_t cpsr_get()
{
    uint32_t r;
    asm volatile("mrs %0, cpsr" : "=r" (r));
    return r;
}

static inline void cpsr_set(uint32_t r)
{
    asm volatile("msr cpsr, %0" : : "r" (r));
}


static inline uint32_t sctlr_get(void)
{
    uint32_t rval;
    asm volatile("mrc p15, 0, %0, c1, c0, 0"
            : "=r" (rval)
            :: "memory");
    return rval;
}

static inline void sctlr_set(uint32_t val)
{
    asm volatile("mcr p15, 0, %0, c1, c0, 0"
            : : "r" (val)
            : "memory");
}

static inline void ttbr0_set(uint32_t val)
{
    asm volatile("mcr p15, 0, %0, c2, c0, 0"
            : : "r" (val)
            : "memory");
}

static inline uint32_t dfar_get(void)
{
    uint32_t rval;
    asm volatile("MRC p15, 0, %0, c6, c0, 0"
            : "=r" (rval)
            :: "memory");
    return rval;
}

static inline uint32_t dfsr_get(void)
{
    uint32_t rval;
    asm volatile("MRC p15, 0, %0, c5, c0, 0"
            : "=r" (rval)
            :: "memory");
    return rval;
}

static inline uint32_t ifar_get(void)
{
    uint32_t rval;
    asm volatile("MRC p15, 0, %0, c6, c0, 2"
            : "=r" (rval)
            :: "memory");
    return rval;
}

static inline uint32_t ifsr_get(void)
{
    uint32_t rval;
    asm volatile("MRC p15, 0, %0, c5, c0, 1"
            : "=r" (rval)
            :: "memory");
    return rval;
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

#define SCTLR_M (1<<0) //enable MMU
#define SCTLR_V (1<<13) //vector table is high

#endif //__ARM_REGS_H__
