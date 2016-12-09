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

#ifdef POK_NEEDS_GETTICK
/**
 * Get the current ticks value, store it in
 * \a clk_val
 * Returns POK_ERRNO_OK
 * Need the GETTICK service (POK_NEEDS_GETTICKS maccro)
 */
pok_ret_t   pok_clock_gettime (clockid_t clk_id, pok_time_t* __user val)
{
   pok_time_t* __kuser k_val = jet_user_to_kernel_typed(val);
   if(!k_val) return POK_ERRNO_EFAULT;
   
   switch(clk_id) {
      case CLOCK_REALTIME:
         *k_val = jet_system_time();
         break;
      default:
         *k_val = -1; // Not supported
         return POK_ERRNO_EINVAL;
   }

   return POK_ERRNO_OK;
}
#endif

pok_ret_t   jet_time(time_t* __user val)
{
   time_t* __kuser k_val = jet_user_to_kernel_typed(val);
   if(!k_val) return POK_ERRNO_EFAULT;

   *k_val =  jet_calendar_time();

   return POK_ERRNO_OK;
}
