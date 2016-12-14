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
#include "msr.h"

#include "timer.h"
#include <cons.h>

/* Last time when decr was set.  */
static uint64_t time_last;

/* Decrementer optimal value.  */
static uint32_t time_inter;


static uint32_t something(uint32_t arg1, uint32_t arg2,uint32_t arg3,uint32_t arg4,uint32_t arg5,uint32_t arg6,uint32_t arg7)
{
  
  
  return (0xb) | arg1 | arg2 | arg3 | arg4 | arg5 | arg6 | arg7 ;
    
};

static uint64_t get_timebase(void)
{
    uint32_t count = mfc0(CP0_COUNT);
    uint32_t smth = something(0xdeadbeef, 0x11111111, 0x22222222, 0x33333333, 0x44444444, 0x55555555, 0x66666666);
    printf("%d", smth);
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
    uint32_t first = mfc0(CP0_COUNT);
    printf("TIMER: mfc0(CP0_COUNT) = 0x%x\n", first);

    //~ uint32_t second = mfc0(CP0_COUNT);
    //~ printf("mfc0(CP0_COUNT) = 0x%x\n", second);
    //~ mtc0(CP0_CAUSE, mfc0(CP0_CAUSE) | CP0_CAUSE_DC);
    //~ printf("second - first = 0x%x\n", second - first);
    //~ printf("mfc0(CP0_COMPARE) = 0x%lx\n", mfc0(CP0_COMPARE));
  if (delta < 0)
  {
    // that delta already expired
    return POK_ERRNO_EINVAL;
  }
  else
  {
    //~ mtc0(CP0_COMPARE, time_new);
    return POK_ERRNO_OK;
  }
}

/* Called by the interrupt handled.  */
void pok_arch_decr_int (void)
{
  int err;

  // FIXME: MIPS doesn't need to clear this bit
  // clear pending intrerrupt
  mtc0(CP0_CAUSE, 0x0);// mfc0(CP0_CAUSE) & (~CP0_CAUSE_IP7));

  do
  {
    err = set_decrementer();
    pok_tick_counter += 1;
  } while (err != POK_ERRNO_OK);


  pok_sched_on_time_changed ();
}

void ja_time_init (void)
{
  time_inter = pok_bsp.timebase_freq / POK_TIMER_FREQUENCY;
  printf("Timer interval: %u\n", time_inter);
  time_last = get_timebase ();
  set_decrementer();
  
  mtsr((mfsr() | CP0_STATUS_IM7)); // enable decrementer
}
