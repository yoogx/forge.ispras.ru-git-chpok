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
 *
 */

#include <libc.h>
#include <assert.h>
#include <asp/cons.h>
#include <asp/entries.h>

#include <libc.h>
#include <bsp/ioports.h>

// The base address for UART.
#define UART1_BASE 0x2020000
#define UART2_BASE 0x21E8000

/* Register definitions */
#define URXD 0x0  /* Receiver Register */
#define UTXD 0x40 /* Transmitter Register */
#define UCR1  0x80 /* Control Register 1 */
#define UCR2  0x84 /* Control Register 2 */
#define UCR3  0x88 /* Control Register 3 */
#define UCR4  0x8c /* Control Register 4 */
#define UFCR  0x90 /* FIFO Control Register */
#define USR1  0x94 /* Status Register 1 */
#define USR2  0x98 /* Status Register 2 */
#define UESC  0x9c /* Escape Character Register */
#define UTIM  0xa0 /* Escape Timer Register */
#define UBIR  0xa4 /* BRM Incremental Register */
#define UBMR  0xa8 /* BRM Modulator Register */
#define UBRC  0xac /* Baud Rate Count Register */
#define ONEMS 0xb0 /* One Millisecond Register */
#define UTS   0xb4 /* Test Register */
#define UMCR  0xb8 /* RS-485 Mode Control Register */

/*
 * Uart constants from linux_kernel/drivers/tty/serial/imx.c
 *  Author: Sascha Hauer <sascha@saschahauer.de>
 *  Copyright (C) 2004 Pengutronix
 *
 *  Copyright (C) 2009 emlix GmbH
 *  Author: Fabian Godehardt (added IrDA support for iMX)
 */

/* UART Control Register Bit Fields.*/
#define URXD_DUMMY_READ      (1<<16)
#define URXD_CHARRDY         (1<<15)
#define URXD_ERR             (1<<14)
#define URXD_OVRRUN          (1<<13)
#define URXD_FRMERR          (1<<12)
#define URXD_BRK             (1<<11)
#define URXD_PRERR           (1<<10)
#define URXD_RX_DATA         (0xFF<<0)
#define UCR1_ADEN            (1<<15) /* Auto detect interrupt */
#define UCR1_ADBR            (1<<14) /* Auto detect baud rate */
#define UCR1_TRDYEN          (1<<13) /* Transmitter ready interrupt enable */
#define UCR1_IDEN            (1<<12) /* Idle condition interrupt */
#define UCR1_ICD_REG(x)      (((x) & 3) << 10) /* idle condition detect */
#define UCR1_RRDYEN          (1<<9)  /* Recv ready interrupt enable */
#define UCR1_RDMAEN          (1<<8)  /* Recv ready DMA enable */
#define UCR1_IREN            (1<<7)  /* Infrared interface enable */
#define UCR1_TXMPTYEN        (1<<6)  /* Transimitter empty interrupt enable */
#define UCR1_RTSDEN          (1<<5)  /* RTS delta interrupt enable */
#define UCR1_SNDBRK          (1<<4)  /* Send break */
#define UCR1_TDMAEN          (1<<3)  /* Transmitter ready DMA enable */
#define IMX1_UCR1_UARTCLKEN  (1<<2) /* UART clock enabled, i.mx1 only */
#define UCR1_ATDMAEN         (1<<2)  /* Aging DMA Timer Enable */
#define UCR1_DOZE            (1<<1)  /* Doze */
#define UCR1_UARTEN          (1<<0)  /* UART enabled */
#define UCR2_ESCI            (1<<15) /* Escape seq interrupt enable */
#define UCR2_IRTS            (1<<14) /* Ignore RTS pin */
#define UCR2_CTSC            (1<<13) /* CTS pin control */
#define UCR2_CTS             (1<<12) /* Clear to send */
#define UCR2_ESCEN           (1<<11) /* Escape enable */
#define UCR2_PREN            (1<<8)  /* Parity enable */
#define UCR2_PROE            (1<<7)  /* Parity odd/even */
#define UCR2_STPB            (1<<6)  /* Stop */
#define UCR2_WS              (1<<5)  /* Word size */
#define UCR2_RTSEN           (1<<4)  /* Request to send interrupt enable */
#define UCR2_ATEN            (1<<3)  /* Aging Timer Enable */
#define UCR2_TXEN            (1<<2)  /* Transmitter enabled */
#define UCR2_RXEN            (1<<1)  /* Receiver enabled */
#define UCR2_SRST            (1<<0)  /* SW reset */
#define UCR3_DTREN           (1<<13) /* DTR interrupt enable */
#define UCR3_PARERREN        (1<<12) /* Parity enable */
#define UCR3_FRAERREN        (1<<11) /* Frame error interrupt enable */
#define UCR3_DSR             (1<<10) /* Data set ready */
#define UCR3_DCD             (1<<9)  /* Data carrier detect */
#define UCR3_RI              (1<<8)  /* Ring indicator */
#define UCR3_ADNIMP          (1<<7)  /* Autobaud Detection Not Improved */
#define UCR3_RXDSEN          (1<<6)  /* Receive status interrupt enable */
#define UCR3_AIRINTEN        (1<<5)  /* Async IR wake interrupt enable */
#define UCR3_AWAKEN          (1<<4)  /* Async wake interrupt enable */
#define UCR3_RXDMUXSEL       (1<<2)  /* RXD Muxed Input Select */
#define UCR3_INVT            (1<<1)  /* Inverted Infrared transmission */
#define UCR3_BPEN            (1<<0)  /* Preset registers enable */
#define UCR4_CTSTL_SHF       10      /* CTS trigger level shift */
#define UCR4_CTSTL_MASK      0x3F    /* CTS trigger is 6 bits wide */
#define UCR4_INVR            (1<<9)  /* Inverted infrared reception */
#define UCR4_ENIRI           (1<<8)  /* Serial infrared interrupt enable */
#define UCR4_WKEN            (1<<7)  /* Wake interrupt enable */
#define UCR4_REF16           (1<<6)  /* Ref freq 16 MHz */
#define UCR4_IDDMAEN         (1<<6)  /* DMA IDLE Condition Detected */
#define UCR4_IRSC            (1<<5)  /* IR special case */
#define UCR4_TCEN            (1<<3)  /* Transmit complete interrupt enable */
#define UCR4_BKEN            (1<<2)  /* Break condition interrupt enable */
#define UCR4_OREN            (1<<1)  /* Receiver overrun interrupt enable */
#define UCR4_DREN            (1<<0)  /* Recv data ready interrupt enable */
#define UFCR_RXTL_SHF        0       /* Receiver trigger level shift */
#define UFCR_DCEDTE          (1<<6)  /* DCE/DTE mode select */
#define UFCR_RFDIV           (7<<7)  /* Reference freq divider mask */
#define UFCR_RFDIV_REG(x)    (((x) < 7 ? 6 - (x) : 6) << 7)
#define UFCR_TXTL_SHF        10      /* Transmitter trigger level shift */
#define USR1_PARITYERR       (1<<15) /* Parity error interrupt flag */
#define USR1_RTSS            (1<<14) /* RTS pin status */
#define USR1_TRDY            (1<<13) /* Transmitter ready interrupt/dma flag */
#define USR1_RTSD            (1<<12) /* RTS delta */
#define USR1_ESCF            (1<<11) /* Escape seq interrupt flag */
#define USR1_FRAMERR         (1<<10) /* Frame error interrupt flag */
#define USR1_RRDY            (1<<9)   /* Receiver ready interrupt/dma flag */
#define USR1_TIMEOUT         (1<<7)   /* Receive timeout interrupt status */
#define USR1_RXDS            (1<<6)  /* Receiver idle interrupt flag */
#define USR1_AIRINT          (1<<5)  /* Async IR wake interrupt flag */
#define USR1_AWAKE           (1<<4)  /* Aysnc wake interrupt flag */
#define USR2_ADET            (1<<15) /* Auto baud rate detect complete */
#define USR2_TXFE            (1<<14) /* Transmit buffer FIFO empty */
#define USR2_DTRF            (1<<13) /* DTR edge interrupt flag */
#define USR2_IDLE            (1<<12) /* Idle condition */
#define USR2_IRINT           (1<<8)  /* Serial infrared interrupt flag */
#define USR2_WAKE            (1<<7)  /* Wake */
#define USR2_RTSF            (1<<4)  /* RTS edge interrupt flag */
#define USR2_TXDC            (1<<3)  /* Transmitter complete */
#define USR2_BRCD            (1<<2)  /* Break condition */
#define USR2_ORE             (1<<1)   /* Overrun error */
#define USR2_RDR             (1<<0)   /* Recv data ready */
#define UTS_FRCPERR          (1<<13) /* Force parity error */
#define UTS_LOOP             (1<<12)  /* Loop tx and rx */
#define UTS_TXEMPTY          (1<<6)  /* TxFIFO empty */
#define UTS_RXEMPTY          (1<<5)  /* RxFIFO empty */
#define UTS_TXFULL           (1<<4)  /* TxFIFO full */
#define UTS_RXFULL           (1<<3)  /* RxFIFO full */
#define UTS_SOFTRST          (1<<0)  /* Software reset */

void uart_putc(uintptr_t uart_base, uint8_t byte)
{
    while (ioread32(uart_base + UTS) & UTS_TXFULL)
        {} //waiting while TxFIFI is full
    iowrite8(uart_base + UTXD, byte);
}


void uart_init(uintptr_t uart_base)
{
    iowrite32(uart_base + UCR1, UCR1_UARTEN); //Enable the UART.

    /* No HW flow control (CS,CTS) */
    iowrite32(uart_base + UCR2, UCR2_PREN|UCR2_WS|UCR2_RXEN|UCR2_TXEN);

    iowrite32(uart_base + UCR3, UCR3_RXDMUXSEL); //on imx6 should be allways enabled

    //TODO add HW control?
    //TODO add baud rate setting
}

static size_t iostream_write_common(const char* s, size_t length, int flag)
{
    (void) flag;
    char c = *s;
    if (c != '\n')
        uart_putc(UART1_BASE, c);
    else {
        uart_putc(UART1_BASE, '\r');
        uart_putc(UART1_BASE, '\n');
    }
    return 1;
}


static size_t iostream_read_common(char* s, size_t length, int flag)
{
    (void) flag;

    // Check if there is something
    if (ioread32(UART1_BASE + URXD) & URXD_CHARRDY) {

        //Read single character
        s[0] = ioread32(UART1_BASE + URXD) & 0xFF;
        return 1;
    } else {
        return 0; //Empty
    }

}

static void iostream_init_common(int flag)
{
    uart_init(UART1_BASE);
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
    return iostream_init_common(0);
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
