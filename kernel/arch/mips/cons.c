/*
 *                               POK header
 * 
 * The following file is a part of the POK project. Any modification should
 * made according to the POK licence. You CANNOT use this file or a part of
 * this file is this part of a file for your own project
 *
 * For more information on the POK licence, please see our LICENCE FILE
 *
 * Please follow the coding guidelines described in doc/CODING_GUIDELINES
 *
 *                                      Copyright (c) 2007-2009 POK team 
 *
 * Created by julien on Thu Jan 15 23:34:13 2009 
 */

#include <config.h>

#include <errno.h>

#include <ioports.h>
#include <libc.h>
#include <core/debug.h>
#include <asp/cons.h>
#include "bsp/bsp.h"

#if defined (POK_NEEDS_CONSOLE) || defined (POK_NEEDS_DEBUG) || defined (POK_NEEDS_INSTRUMENTATION) || defined (POK_NEEDS_COVERAGE_INFOS)

#define NS16550_REG_THR 0
#define NS16550_REG_LSR 5

#define UART_LSR_THR    0x60


static void ns16550_writeb(int offset, int value, int flag)
{
    if (flag == 0){
	    outb(pok_bsp.ccsrbar_base + pok_bsp.serial0_regs_offset + offset, value);
    } else {
	    outb(pok_bsp.ccsrbar_base + pok_bsp.serial1_regs_offset + offset, value);
    }
}

static int ns16550_readb(int offset, int flag)
{
    if (flag == 0) {
        return inb(pok_bsp.ccsrbar_base + pok_bsp.serial0_regs_offset + offset);
    }
    return inb(pok_bsp.ccsrbar_base + pok_bsp.serial1_regs_offset + offset);
}


static void write_serial(char a, int flag)
{
   while ((ns16550_readb(NS16550_REG_LSR, flag) & UART_LSR_THR) != UART_LSR_THR)
     ;

   ns16550_writeb(NS16550_REG_THR, a, flag);
}

#define UART_LSR_DR   0x01
#define UART_LSR_RFE  0x80
	
// TODO: revisit.
static int read_serial(int flag)
{
    int data;
    if (!(ns16550_readb(NS16550_REG_LSR, flag) & UART_LSR_DR)) return -1;
    data=ns16550_readb(NS16550_REG_THR, flag);
    if ( !(ns16550_readb(NS16550_REG_LSR, flag) & UART_LSR_RFE) )
	    return data;
    return -1;
}

static size_t iostream_write_common(const char* s, size_t length, int flag)
{
    // Write only single character
    char c = *s;
    if (c != '\n')
	write_serial(c, flag);
    else {
	write_serial('\r', flag);
	write_serial('\n', flag);
    }
    return 1;
}

static size_t iostream_write_main(const char* s, size_t length)
{
    return iostream_write_common(s, length, 0);
}

static size_t iostream_write_debug(const char* s, size_t length)
{
    return iostream_write_common(s, length, 1);
}

static size_t iostream_read_common(char* s, size_t length, int flag)
{
    int data = read_serial(flag);
    // Read only single character
    if(data == -1)
    {
	return 0;
    }
    s[0] = (char)data;
    return 1;
}

static size_t iostream_read_main(char* s, size_t length)
{
    return iostream_read_common(s, length, 0);
}
static size_t iostream_read_debug(char* s, size_t length)
{
    return iostream_read_common(s, length, 1);
}

struct jet_iostream mips_stream_main =
{
    .write = &iostream_write_main,
    .read  = &iostream_read_main
};
struct jet_iostream mips_stream_debug =
{
    .write = &iostream_write_debug,
    .read  = &iostream_read_debug
};

struct jet_iostream* ja_stream_default_read = &mips_stream_main;
struct jet_iostream* ja_stream_default_write = &mips_stream_main;
struct jet_iostream* ja_stream_default_read_debug = &mips_stream_debug;
struct jet_iostream* ja_stream_default_write_debug = &mips_stream_debug;

#else

struct jet_iostream* ja_stream_default_read = NULL;
struct jet_iostream* ja_stream_default_write = NULL;
struct jet_iostream* ja_stream_default_read_debug = NULL;
struct jet_iostream* ja_stream_default_write_debug = NULL;

#endif
