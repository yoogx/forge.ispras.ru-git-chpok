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

#include <config.h>

#include <types.h>
#include <errno.h>

#include <core/time.h>
#include <core/uaccess.h>
#include <core/sched.h>

#include <asp/entries.h> /* jet_on_tick() declaration. */

void jet_on_tick(void)
{
    pok_sched_on_time_changed();
}

/**
 * Get the current ticks value, store it in
 * \a clk_val
 * Returns EOK
 * Need the GETTICK service (POK_NEEDS_GETTICKS maccro)
 */
jet_ret_t   pok_clock_gettime (clockid_t clk_id, pok_time_t* __user val)
{
   pok_time_t* __kuser k_val = jet_user_to_kernel_typed(val);
   if(!k_val) return EFAULT;
   
   switch(clk_id) {
      case CLOCK_REALTIME:
         *k_val = jet_system_time();
         break;
      default:
         *k_val = -1; // Not supported
         return EINVAL;
   }

   return EOK;
}

jet_ret_t   jet_time(time_t* __user val)
{
   time_t* __kuser k_val = jet_user_to_kernel_typed(val);
   if(!k_val) return EFAULT;

   *k_val =  jet_calendar_time();

   return EOK;
}

#ifdef POK_NEEDS_TIME_SHIFT

// Returns the current system time in nanoseconds.
pok_time_t jet_system_time(void) {
    return ja_system_time();
}

// Returns the calendar time in seconds since the Epoch.
time_t jet_calendar_time(void) {
    return ja_calendar_time();
}

#endif // POK_NEEDS_TIME_SHIFT
