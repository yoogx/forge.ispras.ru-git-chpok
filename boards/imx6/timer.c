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
#define GPT_CR_CLCKSRC_24MGH (0b111)
#define GPT_CR_SWR (1<<15) // software reset

#define GPT_IR_OF1IE (1<<0)

#define GPT_IR_DISABLE_ALL (0)


void unset_bits(uintptr_t reg, uint32_t bits)
{
    iowrite32(reg, ioread32(reg) & ~bits);
}

void set_bits(uintptr_t reg, uint32_t bits)
{
    iowrite32(reg, ioread32(reg)|bits);
}

/* Loop <delay> times in a way that the compiler won't optimize away. */
static inline void delay(int32_t count)
{
    asm volatile("__delay_%=: subs %[count], %[count], #1; bne __delay_%=\n"
            : "=r"(count): [count]"0"(count) : "cc");
}

void timer_init(void)
{
    //irq_init();
    // reset device
    {
        set_bits(GPT_CR, GPT_CR_SWR);

        /*
        printf("AAA\n");
        while (ioread32(GPT_CR)&GPT_CR_SWR)
            ;
        printf("OOOO\n");
        */
    }

    // and clear other bits in GPT_CR
    iowrite32(GPT_CR,
            GPT_CR_CLCKSRC(GPT_CR_CLCKSRC_24MGH)| //Change clock source TODO: enable bits in CCM_CCGR1 ?
            GPT_CR_ENMOD| //zero GPT counter after enabling
            GPT_CR_FRR //free-run mode
            );


    iowrite32(GPT_OCR1, 0x05000000);

    // Enable interrupt register on channel 1
    set_bits(GPT_IR, GPT_IR_OF1IE);

    // Enable GPT
    set_bits(GPT_CR, GPT_CR_EN);


    while (1) {
        printf("%lx %lx\n", ioread32(GPT_CNT), ioread32(GPT_SR));
        delay(0x10000000);

    }

}
