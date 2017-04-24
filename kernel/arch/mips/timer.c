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

#include <errno.h>
#include "bsp/bsp.h"
#include <core/time.h>
#include <core/sched.h>
#include <core/debug.h>
#include "reg.h"
#include "cp0.h"
#include "timer.h"
#include <cons.h>
#include <asp/entries.h>

/* First value of decrementer.  */
static uint64_t time_first;


/* Last time when decr was set.  */
static uint64_t time_last;

/* Decrementer optimal value.  */
static uint32_t time_inter;

/*
 * Hardcoded calendar time at the beginning of the OS loading.
 * 
 * Could be obtained on https://www.timeanddate.com/.
 */
static time_t base_calendar_time = 1480330081; // On 28.11.2016


static uint64_t get_timebase(void)
{
    uint64_t count = mfc0(CP0_COUNT) + time_last;
    return count;
}

/* Compute new value for the decrementer.  If the value is in the future,
   sets the decrementer else returns an error.  */
static int set_decrementer(void)
{
  uint64_t time_new = time_last + time_inter;
  uint64_t time_cur = get_timebase();
  int32_t delta = time_new - time_cur;
  time_last = time_new;
  //~ printf("time_new = %lld; time_cur = %lld; delta = %d;\n", time_new, time_cur, delta);
  if (delta < 0){
    // that delta already expired
    /*Clear time count, because we saved it in time_last*/
    mtc0(CP0_COUNT, 0x0);
    return EINVAL;
  }
  else{
    mtc0(CP0_COMPARE, delta);
    //~ printf("TIMER: STATUS = 0x%lx\n", mfsr());
    //~ printf("TIMER: mfc0(CP0_COUNT) before = 0x%lx\n", mfc0(CP0_COUNT));
    //~ printf("TIMER: mfc0(CP0_COMPARE) = 0x%lx\n", mfc0(CP0_COMPARE));
    mtc0(CP0_COUNT, 0x0);
    return EOK;
  }  //~ printf("TIMER: mfc0(CP0_COUNT) after = 0x%lx\n", mfc0(CP0_COUNT));
}

/* Called by the interrupt handled.  */
void pok_arch_decr_int (void)
{
  int err;
  do
  {
    err = set_decrementer();
  } while (err != EOK);

  jet_on_tick();
}

void ja_time_init (void)
{
  time_inter = pok_bsp.timebase_freq / POK_TIMER_FREQUENCY;
  printf("Timer interval: %u\n", time_inter);
  time_last = 0;
  time_first = time_last = get_timebase ();
  /*Clear time count, because we saved it in time_last*/
  mtc0(CP0_COUNT, 0x0);
  set_decrementer();
  
  mtsr((mfsr() | CP0_STATUS_IM7)); // enable decrementer
}

pok_time_t ja_system_time(void)
{
  return ((get_timebase() - time_first) / time_inter) * 1000000;
}

time_t ja_calendar_time(void)
{
  return base_calendar_time + (time_t)(ja_system_time() / 1000000000);
}
