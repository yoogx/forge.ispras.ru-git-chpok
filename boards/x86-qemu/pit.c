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
#include <arch/ioports.h>

#include "pic.h"

#include "pit.h"

#include <bsp/bsp.h>

#include <asp/entries.h>

#define OSCILLATOR_RATE 1193180 /** The oscillation rate of x86 clock */
#define PIT_BASE 0x40

/* Two parts of system time, each one can be upated atomically. */
volatile uint32_t system_time_low;
volatile uint32_t system_time_high;

/*
 * Hardcoded calendar time at the beginning of the OS loading.
 * 
 * Could be obtained on https://www.timeanddate.com/.
 */
static time_t base_calendar_time = 1480330081; // On 28.11.2016


void ja_bsp_process_timer(interrupt_frame* frame)
{
   (void) frame;
   pok_pic_eoi (PIT_IRQ);

   uint32_t system_time_low_new = system_time_low + (1000000000 / POK_TIMER_FREQUENCY);
   if(system_time_low_new < (1000000000 / POK_TIMER_FREQUENCY)) {
      // Overflow of low part.
      system_time_high++;
   }

   system_time_low = system_time_low_new;

   jet_on_tick();
}

void pok_x86_qemu_timer_init(void)
{
   system_time_low = system_time_high = 0;

   uint16_t pit_freq;

   pit_freq = POK_TIMER_FREQUENCY;

   outb (PIT_BASE + 3, 0x34); /* Channel0, rate generator, Set LSB then MSB */
   outb (PIT_BASE, (OSCILLATOR_RATE / pit_freq) & 0xff);
   outb (PIT_BASE, ((OSCILLATOR_RATE / pit_freq) >> 8) & 0xff);

   pok_pic_unmask (PIT_IRQ);
}

pok_time_t ja_system_time(void)
{
   uint32_t low, high;
   uint32_t high1 = system_time_high;

   /*
    * Repeat reading parts of time while high part is updated during
    * that reading.
    */
   do {
      high = high1;
      low = system_time_low;
      high1 = system_time_high;
   } while(high != high1);

   return (((pok_time_t)(high) << 32) + low);
}

time_t ja_calendar_time(void)
{
   return base_calendar_time + (time_t)(ja_system_time() / 1000000000);
}
