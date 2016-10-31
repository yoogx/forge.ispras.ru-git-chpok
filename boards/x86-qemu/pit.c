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


#include <errno.h>
#include <core/time.h>
#include <core/sched.h>
#include <ioports.h>

#include "pic.h"

#include "pit.h"

#include <bsp/bsp.h>

#define OSCILLATOR_RATE 1193180 /** The oscillation rate of x86 clock */
#define PIT_BASE 0x40

void ja_bsp_process_timer(interrupt_frame* frame)
{
   (void) frame;
   pok_pic_eoi (PIT_IRQ);
   CLOCK_HANDLER;
}

void pok_x86_qemu_timer_init(void)
{
   uint16_t pit_freq;

   pit_freq = POK_TIMER_FREQUENCY;

   outb (PIT_BASE + 3, 0x34); /* Channel0, rate generator, Set LSB then MSB */
   outb (PIT_BASE, (OSCILLATOR_RATE / pit_freq) & 0xff);
   outb (PIT_BASE, ((OSCILLATOR_RATE / pit_freq) >> 8) & 0xff);

   pok_pic_unmask (PIT_IRQ);
}


