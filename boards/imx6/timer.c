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

/* General Purpose Timer (GPT) registers */

#include <bsp/ioports.h>
#include <libc.h>
#include "irq.h"
#include <core/time.h>
#include <asp/time.h>
#include <assert.h>
#include <asp/entries.h>


//TODO: should we enable bits in CCM_CCGR1? to use this clock source?
#define CLOCK_SOURCE (GPT_CR_CLCKSRC_24MHz)
#define CLOCK_SOURCE_FREQ 0x1800000 //24 MHz

static uint32_t timer_interval; //number of timer ticks in 1/POK_TIMER_FREQUENCY sec.
static uint64_t intervals_count;

/* Two parts of system time */
volatile uint32_t system_time_low;
volatile uint32_t system_time_high;

/*
 * Hardcoded calendar time at the beginning of the OS loading.
 *
 * Could be obtained on https://www.timeanddate.com/.
 */
static time_t base_calendar_time = 1480330081; // On 28.11.2016

#define GIC_GPT_IRQ 87 //irq vector

#define GPT_CR   0x2098000 //GPT Control Register
#define GPT_PR   0x2098004 //GPT Prescaler Register
#define GPT_SR   0x2098008 //GPT Status Register
#define GPT_IR   0x209800C //GPT Interrupt Register
#define GPT_OCR1 0x2098010 //GPT Output Compare Register 1
#define GPT_OCR2 0x2098014 //GPT Output Compare Register 2
#define GPT_OCR3 0x2098018 //GPT Output Compare Register 3
#define GPT_ICR1 0x209801C //GPT Input Capture Register 1
#define GPT_ICR2 0x2098020 //GPT Input Capture Register 2
#define GPT_CNT  0x2098024 //GPT Counter Register

#define GPT_CR_EN (1<<0) //enable bit
#define GPT_CR_ENMOD (1<<1)
#define GPT_CR_FRR (1<<9) //Free-Run or Restart mode
#define GPT_CR_CLCKSRC(x) (x<<6) // clock source
#define GPT_CR_CLCKSRC_24MHz (0b111)
#define GPT_CR_SWR (1<<15) // software reset

#define GPT_IR_OF1IE (1<<0)

#define GPT_SR_OF1 (1<<0)

#define GPT_IR_DISABLE_ALL (0)

//static void unset_bits(uintptr_t reg, uint32_t bits)
//{
//    iowrite32(reg, ioread32(reg) & ~bits);
//}

static void set_bits(uintptr_t reg, uint32_t bits)
{
    iowrite32(reg, ioread32(reg)|bits);
}

static uint32_t abs_diff(uint32_t a, uint32_t b)
{
    if (a >= b)
        return a - b;
    else
        return b - a;
}

void set_timer(void)
{
    //We assume that there are at least 3 (or 2?) interrupts in 2**32 ticks. Therefore
    //between two sequential calls of set_timer can't pass more than 2**31 ticks.

    intervals_count += 1;
    uint32_t next = ((intervals_count * timer_interval)%0xffffffff);
    iowrite32(GPT_OCR1, next);
    uint32_t current = ioread32(GPT_CNT);

    if ( (next > current  &&  abs_diff(next,current) > 0x80000000) ||
         (next < current  &&  abs_diff(next,current) < 0x80000000)
         ) {
        // next already passed, so recount
        set_timer();
    }
}

void timer_handle_interrupt(void)
{
    //Reset OF1 bit in SR
    iowrite32(GPT_SR, GPT_SR_OF1);
    //printf("TIMER_INTERRUPT\n");
    set_timer();

    jet_on_tick();
}


void timer_init(void)
{
    printf("%s\n", __func__);
    iowrite32(GPT_CR, 0); // ensure that GPT is disabled

    // reset device (all register). And check that reset finished correctly
    //set_bits(GPT_CR, GPT_CR_SWR);
    //if (ioread32(GPT_CR) & GPT_CR_SWR)
    //    pok_fatal("Update your QEMU! It has a bug in GPT SWR\n");

    iowrite32(GPT_CR,
            GPT_CR_CLCKSRC(CLOCK_SOURCE) //Set clock source
            | GPT_CR_ENMOD //zero GPT counter after enabling
            | GPT_CR_FRR //free-run mode
            );

    // Enable interrupts on channel 1
    set_bits(GPT_IR, GPT_IR_OF1IE);

    // Enable GPT
    set_bits(GPT_CR, GPT_CR_EN);

    irq_enable_interrupt(IRQ_GPT);

    // update timer_interval according selected clock source
    timer_interval = CLOCK_SOURCE_FREQ / POK_TIMER_FREQUENCY;

    set_timer(); //set next timer interrupt
}


/* Return current system time. */
pok_time_t ja_system_time(void)
{
    return intervals_count*1000000000/POK_TIMER_FREQUENCY;
}

/* Return current calendar time (seconds since Epoch). */
time_t ja_calendar_time(void)
{
  return base_calendar_time + (time_t)(ja_system_time() / 1000000000);
}
