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

#include <libc.h>
#include <assert.h>
#include <asp/cons.h>
#include <asp/entries.h>

#include <libc.h>
#include <arch/ioports.h>

enum
{
    // The GPIO registers base address.
    GPIO_BASE = 0x3F200000, // for raspi2 and raspi3

    // The offsets for reach register.

    // Controls actuation of pull up/down to ALL GPIO pins.
    GPPUD = (GPIO_BASE + 0x94),

    // Controls actuation of pull up/down for specific GPIO pin.
    GPPUDCLK0 = (GPIO_BASE + 0x98),

    // The base address for UART.
    UART0_BASE = 0x3F201000, // for raspi 2 and raspi3

    // The offsets for reach register for the UART.
    UART0_DR     = (UART0_BASE + 0x00),
    UART0_RSRECR = (UART0_BASE + 0x04),
    UART0_FR     = (UART0_BASE + 0x18),
    UART0_ILPR   = (UART0_BASE + 0x20),
    UART0_IBRD   = (UART0_BASE + 0x24),
    UART0_FBRD   = (UART0_BASE + 0x28),
    UART0_LCRH   = (UART0_BASE + 0x2C),
    UART0_CR     = (UART0_BASE + 0x30),
    UART0_IFLS   = (UART0_BASE + 0x34),
    UART0_IMSC   = (UART0_BASE + 0x38),
    UART0_RIS    = (UART0_BASE + 0x3C),
    UART0_MIS    = (UART0_BASE + 0x40),
    UART0_ICR    = (UART0_BASE + 0x44),
    UART0_DMACR  = (UART0_BASE + 0x48),
    UART0_ITCR   = (UART0_BASE + 0x80),
    UART0_ITIP   = (UART0_BASE + 0x84),
    UART0_ITOP   = (UART0_BASE + 0x88),
    UART0_TDR    = (UART0_BASE + 0x8C),
};

/* Loop <delay> times in a way that the compiler won't optimize away. */
static inline void delay(int32_t count)
{
    asm volatile("__delay_%=: subs %[count], %[count], #1; bne __delay_%=\n"
            : "=r"(count): [count]"0"(count) : "cc");
}

void uart_putc(uint8_t byte)
{
    // Wait for UART to become ready to transmit.
    while (ioread32(UART0_FR) & (1 << 5)) { }
    iowrite32(UART0_DR, byte);
}

uint8_t uart_getc()
{
    // Wait for UART to have recieved something.
    while (ioread32(UART0_FR) & (1 << 4)) { }
    return ioread32(UART0_DR);
}

void uart_init()
{
    /*
    // Disable UART0.
    iowrite32(UART0_CR, 0x00000000);
    // Setup the GPIO pin 14 && 15.

    // Disable pull up/down for all GPIO pins & delay for 150 cycles.
    iowrite32(GPPUD, 0x00000000);
    delay(150);

    // Disable pull up/down for pin 14,15 & delay for 150 cycles.
    iowrite32(GPPUDCLK0, (1 << 14) | (1 << 15));
    delay(150);

    // Write 0 to GPPUDCLK0 to make it take effect.
    iowrite32(GPPUDCLK0, 0x00000000);

    // Clear pending interrupts.
    iowrite32(UART0_ICR, 0x7FF);

    // Set integer & fractional part of baud rate.
    // Divider = UART_CLOCK/(16 * Baud)
    // Fraction part register = (Fractional part * 64) + 0.5
    // UART_CLOCK = 3000000; Baud = 115200.

    // Divider = 3000000 / (16 * 115200) = 1.627 = ~1.
    // Fractional part register = (.627 * 64) + 0.5 = 40.6 = ~40.
    iowrite32(UART0_IBRD, 1);
    iowrite32(UART0_FBRD, 40);

    // Enable FIFO & 8 bit data transmissio (1 stop bit, no parity).
    iowrite32(UART0_LCRH, (1 << 4) | (1 << 5) | (1 << 6));

    // Mask all interrupts.
    iowrite32(UART0_IMSC, (1 << 1) | (1 << 4) | (1 << 5) | (1 << 6) |
            (1 << 7) | (1 << 8) | (1 << 9) | (1 << 10));

    // Enable UART0, receive & transfer part of UART.
    iowrite32(UART0_CR, (1 << 0) | (1 << 8) | (1 << 9));
    */
}

static size_t iostream_write_common(const char* s, size_t length, int flag)
{
    (void) flag;
    char c = *s;
    if (c != '\n')
        uart_putc(c);
    else {
        uart_putc('\r');
        uart_putc('\n');
    }
    return 1;
}


static size_t iostream_read_common(char* s, size_t length, int flag)
{
    (void) flag;

    // Check if there is something
    if (ioread32(UART0_FR) & (1 << 4))
        return 0; //Empty

    //Read single character
    s[0] = ioread32(UART0_DR);
    return 1;
}

static void iostream_init_common(int flag)
{
    (void) flag;

    uart_init();
}

static size_t iostream_write_main(const char* s, size_t length)
{
    return iostream_write_common(s, length, 0);
}

static size_t iostream_read_main(char* s, size_t length)
{
    return iostream_read_common(s, length, 0);
}

static void iostream_init_main(void)
{
    iostream_init_common(0);
}

struct jet_iostream arm_stream_main =
{
    .write = &iostream_write_main,
    .read  = &iostream_read_main,
    .init = &iostream_init_main
};

struct jet_iostream* ja_stream_default_read = &arm_stream_main;
struct jet_iostream* ja_stream_default_write = &arm_stream_main;
struct jet_iostream* ja_stream_default_read_debug = NULL;
struct jet_iostream* ja_stream_default_write_debug = NULL;
