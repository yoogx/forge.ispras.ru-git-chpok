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

#include "cpu.h"

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


//Coprocessor Access Control Register
static inline uint32_t cpacr_get()
{
    uint32_t r;
    asm volatile("MRC p15, 0, %0, c1, c0, 2" : "=r" (r));
    return r;
}

static inline void cpacr_set(uint32_t val)
{
    asm volatile("MCR p15, 0, %0, c1, c0, 2" : : "r" (val));
}

#ifdef ARMv7_SECURE_EXTENSION

//Non-Secure Access Control Register,
static inline uint32_t nsacr_get()
{
    uint32_t r;
    asm volatile("MRC p15, 0, %0, c1, c1, 2" : "=r" (r));
    return r;
}

static inline void nsacr_set(uint32_t val)
{
    asm volatile("MCR p15, 0, %0, c1, c1, 2" : : "r" (val));
}

//Secure Configuration Register
static inline uint32_t scr_get()
{
    uint32_t r;
    asm volatile("MRC p15, 0, %0, c1, c1, 0" : "=r" (r));
    return r;
}
#endif




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

//Main ID Register
static inline uint32_t midr_get(void)
{
    uint32_t rval;
    asm volatile("MRC p15, 0, %0, c0, c0, 0"
            : "=r" (rval)
            :: "memory");
    return rval;
}

 
#endif // __ASSEMBLER__

#define CPSR_IRQ    (1<<7) //disable IRQ
#define CPSR_FIQ    (1<<6) //disable FIQ
#define CPSR_ABORT  (1<<6) //disable async ABORT

#define CPSR_MODE_USR 0x10 //User
#define CPSR_MODE_FIQ 0x11 //Fast IRQ
#define CPSR_MODE_IRQ 0x12 // IRQ
#define CPSR_MODE_SVC 0x13 // Supervisor
#define CPSR_MODE_MON 0x16 // Monitor
#define CPSR_MODE_ABT 0x17 // Abort
#define CPSR_MODE_HYP 0x1a // Hyp
#define CPSR_MODE_UND 0x1b // Undef
#define CPSR_MODE_SYS 0x1f // System

#define CPSR_MODE_MASK 0x1f

#define KERNEL_ENTRY_MODE (CPSR_MODE_SVC | CPSR_IRQ | CPSR_FIQ)

#define CPACR_CP0(x)  (x<<0)
#define CPACR_CP1(x)  (x<<2)
#define CPACR_CP2(x)  (x<<4)
#define CPACR_CP3(x)  (x<<6)
#define CPACR_CP4(x)  (x<<8)
#define CPACR_CP5(x)  (x<<10)
#define CPACR_CP6(x)  (x<<12)
#define CPACR_CP7(x)  (x<<14)
#define CPACR_CP8(x)  (x<<16)
#define CPACR_CP9(x)  (x<<18)
#define CPACR_CP10(x) (x<<20)
#define CPACR_CP11(x) (x<<22)
#define CPACR_CP12(x) (x<<24)
#define CPACR_CP13(x) (x<<26)

#define CPACR_ACCESS_DENIED   0b00
#define CPACR_ACCESS_PL1_ONLY 0b01
#define CPACR_ACCESS_FULL     0b11

#define CPACR_TRCDIS (1<<28)
#define CPACR_D32DIS (1<<30)
#define CPACR_ASEDIS (1<<31)

#define NSACR_CP0  (1<<0 )
#define NSACR_CP1  (1<<1 )
#define NSACR_CP2  (1<<2 )
#define NSACR_CP3  (1<<3 )
#define NSACR_CP4  (1<<4 )
#define NSACR_CP5  (1<<5 )
#define NSACR_CP6  (1<<6 )
#define NSACR_CP7  (1<<7 )
#define NSACR_CP8  (1<<8 )
#define NSACR_CP9  (1<<9 )
#define NSACR_CP10 (1<<10)
#define NSACR_CP11 (1<<11)
#define NSACR_CP12 (1<<12)
#define NSACR_CP13 (1<<13)

#define SCTLR_M (1<<0) //enable MMU
#define SCTLR_V (1<<13) //vector table is high

#define SCR_NS (1<<0) // non-secure bit

#endif //__ARM_REGS_H__
