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

#ifdef POK_NEEDS_TIME_SHIFT

/* Terms for this piece of code:
   - asptime -- physical time, time that is given by ja_system_time();
   - systime -- system time, time that is given by jet_system_time();
 */

// Overall shift for all already ended periods of waiting.
static pok_time_t total_already_finished_shift;

// ASP time of real stop for the last time. POK_TIME_INFINITY when not applicable.
static pok_time_t asptime_of_stopped;

// System time after which the system should stop its time automatically.
static pok_time_t systime_when_to_stop;

// TODO to get rid of situation of pretty complex invariant of asptime_of_stopped == POK_TIME_INFINITY <==> systime_when_to_stop != POK_TIME_INFINITY && vice versa.

#endif // POK_NEEDS_TIME_SHIFT

void jet_on_tick(void)
{
#ifdef POK_NEEDS_TIME_SHIFT
    pok_time_t curr_asptime = ja_system_time();

    if (systime_when_to_stop != POK_TIME_INFINITY && systime_when_to_stop >= curr_asptime - total_already_finished_shift) {
        asptime_of_stopped = curr_asptime;
        systime_when_to_stop = POK_TIME_INFINITY;
    }
    if (asptime_of_stopped != POK_TIME_INFINITY) {
        if (asptime_of_stopped <= curr_asptime) {
            switch_to_idle_partition();
            return; // This line must be unreachable, in fact.
        } else {
            // An error situation.
        }
    }
#endif // POK_NEEDS_TIME_SHIFT

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

void jet_suspend_systime_now() {
    jet_suspend_systime_at(jet_system_time());
    // It seems that this implementation works also nicely:
    //jet_suspend_systime_at(BEGINNING_OF_THE_TIME);
}

void jet_suspend_systime_at(pok_time_t systime) {
    // TODO to check whether the next stop is greater.
    systime_when_to_stop = systime;
}

void jet_resume_systime() {
    systime_when_to_stop = POK_TIME_INFINITY;
    total_already_finished_shift += (ja_system_time() - asptime_of_stopped);
    asptime_of_stopped = POK_TIME_INFINITY;
}

// Returns the current system time in nanoseconds.
pok_time_t jet_system_time(void) {
    return (asptime_of_stopped == POK_TIME_INFINITY ? ja_system_time() : asptime_of_stopped) - total_already_finished_shift;
}

// Returns the calendar time in seconds since the Epoch.
time_t jet_calendar_time(void) {
    return ja_calendar_time();
}

#endif // POK_NEEDS_TIME_SHIFT
