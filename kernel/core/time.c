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

// Coefficient of pok_time_t (nanoseconds for now) to time_t (seconds for now).
#define POK_TIME_TO_TIME 1000000000

static inline time_t poktime_to_sec(pok_time_t pok_time) {
    return pok_time / POK_TIME_TO_TIME;
}

/* Terms for this piece of code:
   - asptime -- physical time, time that is given by ja_system_time();
   - systime -- system time, time that is given by jet_system_time();
 */

/*
   Overall shift for all already ended periods of waiting.

   That is, when stopped_now is false, this is the total shift value,
   when stopped_now is true, this is the total shift of all but the current periods of waiting.
 */
static pok_time_t total_already_finished_shift;

// Whether at this moment the system is idling (and thus shifting time) or not.
static pok_bool_t stopped_now;

/*
   When stopped_now is true, this is the value of ASP time of real stop for the last time; must not be POK_TIME_INFINITY.
   When stopped_now is false, this is the value of ASP time when system needs to stop or POK_TIME_INFINITY when it should not.
*/
static pok_time_t stop_asptime;

// TODO to initialize these values.

/*@
    global invariant time_shift_state_consistency:
        asptime_of_stop == POK_TIME_INFINITY ==> !stopped_now &&
        stopped_now ==> stop_asptime < ja_system_time();
 */

#endif // POK_NEEDS_TIME_SHIFT

void jet_on_tick(void)
{
#ifdef POK_NEEDS_TIME_SHIFT
    pok_time_t curr_asptime = ja_system_time();

    if (!stopped_now && stop_asptime != POK_TIME_INFINITY && stop_asptime >= curr_asptime) {
        // These two assignments must be done atomically, with no preemption.
        // Since, we are in `jet_on_tick', interruptions must be disabled, I hope.
        stopped_now = TRUE;
        stop_asptime = curr_asptime;
    }
    if (stopped_now) {
        switch_to_idle_partition();
        return; // This line must be unreachable, in fact.
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
    assert(!stopped_now);

    // In case of turned off asserts, the following code should not execute.
    // TODO to think whether this should be reported or kernel fail should be done instead.
    if (stopped_now) {
        stop_asptime = systime + total_already_finished_shift;
    }
}

void jet_resume_systime() {
    assert(stopped_now);

    // In case of turned off asserts, the following code should not execute.
    // TODO to think whether this should be reported or kernel fail should be done instead.
    if (!stopped_now) {
        total_already_finished_shift += (ja_system_time() - stop_asptime);
        // TODO These two assignments must be atomic (noone guarantees this for now), otherwise, invariant is not hold.
        stop_asptime = POK_TIME_INFINITY;
        stopped_now = FALSE;
    }
}

// Returns the current system time in nanoseconds.
pok_time_t jet_system_time(void) {
    return (!stopped_now ? ja_system_time() : stop_asptime) - total_already_finished_shift;
}

// Returns the calendar time in seconds since the Epoch.
time_t jet_calendar_time(void) {
    return !stopped_now
        ? ja_calendar_time() - poktime_to_sec(total_already_finished_shift)
        : poktime_to_sec(stop_asptime - total_already_finished_shift);
}

#endif // POK_NEEDS_TIME_SHIFT
