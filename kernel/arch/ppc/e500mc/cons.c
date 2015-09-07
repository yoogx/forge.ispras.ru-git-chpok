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
#include <core/cons.h>
#include "cons.h"

#if defined (POK_NEEDS_CONSOLE) || defined (POK_NEEDS_DEBUG) || defined (POK_NEEDS_INSTRUMENTATION) || defined (POK_NEEDS_COVERAGE_INFOS)

#define MPC8544_SERIAL0_REGS_OFFSET 0x4500ULL

#define NS16550_REG_THR 0
#define NS16550_REG_LSR 5

#define UART_LSR_THRE   0x20

static void ns16550_writeb(int offset, int value)
{
    outb(CCSRBAR_BASE + MPC8544_SERIAL0_REGS_OFFSET + offset, value);
}

static int ns16550_readb(int offset)
{
    return inb(CCSRBAR_BASE + MPC8544_SERIAL0_REGS_OFFSET + offset);
}

static void write_serial(char a)
{
   while ((ns16550_readb(NS16550_REG_LSR) & UART_LSR_THRE) == 0)
     ;

   ns16550_writeb(NS16550_REG_THR, a);
}

#define UART_LSR_DR   0x01
#define UART_LSR_RFE  0x80
	
int data_to_read() //return 0 if no data to read
		  
{
	if (!(ns16550_readb(NS16550_REG_LSR) & UART_LSR_DR))
		return 0;
	return 1;
}

int read_serial()
{
	int data;
	data=ns16550_readb(NS16550_REG_THR);
	if ( !(ns16550_readb(NS16550_REG_LSR) & UART_LSR_RFE) )
		return data;
	return -1;

}

pok_bool_t pok_cons_write (const char *s, size_t length)
{
    char c;
    for (; length > 0; length--) {
        c = *s++;
        if (c != '\n')
            write_serial(c);
        else {
            write_serial('\r');
            write_serial('\n');
        }
    }
   return 0;
}


int pok_cons_init (void)
{
    pok_print_init (write_serial, NULL);
    return 0;
}
#else
int pok_cons_init (void)
{
   return 0;
}
#endif


