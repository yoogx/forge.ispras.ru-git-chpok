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
 * This file also incorporates work covered by the following 
 * copyright and license notice:
 *
 *  Copyright (C) 2013-2014 Maxim Malkov, ISPRAS <malkov@ispras.ru> 
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Created by julien on Thu Jan 15 23:34:13 2009 
 */


#ifdef POK_NEEDS_ARINC653_TIME

#include <arinc653/types.h>
#include <arinc653/time.h>

#include <core/thread.h>
#include <core/time.h>
#include <types.h>
#include <utils.h>

#define MAP_ERROR(from, to) case (from): *return_code = (to); break
#define MAP_ERROR_DEFAULT(to) default: *return_code = (to); break

void TIMED_WAIT (SYSTEM_TIME_TYPE delay_time, RETURN_CODE_TYPE *return_code)
{
   int64_t delay_ms = arinc_time_to_ms(delay_time);
   if (delay_ms > INT32_MAX) {
       *return_code = INVALID_PARAM;
       return;
   }
   if (delay_ms < 0) {
       // arinc doesn't allow infinite sleep
       *return_code = INVALID_PARAM;
       return;
   }

   pok_ret_t core_ret;
   core_ret = pok_syscall2(POK_SYSCALL_THREAD_SLEEP, (int32_t) delay_ms, 0);
    
   switch (core_ret) {
      MAP_ERROR(POK_ERRNO_OK, NO_ERROR);
      MAP_ERROR(POK_ERRNO_MODE, INVALID_MODE);
      MAP_ERROR_DEFAULT(INVALID_PARAM);
   }
}

void PERIODIC_WAIT (RETURN_CODE_TYPE *return_code)
{
   pok_ret_t core_ret;
   core_ret = pok_thread_period ();
   switch (core_ret) {
      MAP_ERROR(POK_ERRNO_OK, NO_ERROR);
      MAP_ERROR(POK_ERRNO_MODE, INVALID_MODE);
      MAP_ERROR_DEFAULT(INVALID_PARAM);
   }
}

void GET_TIME (SYSTEM_TIME_TYPE *system_time, RETURN_CODE_TYPE *return_code)
{
   uint64_t time;
   pok_time_get(&time);
   *system_time = ms_to_arinc_time(time);
   *return_code = NO_ERROR;
}

void REPLENISH (SYSTEM_TIME_TYPE budget_time, RETURN_CODE_TYPE *return_code)
{
   (void) budget_time;
   *return_code = NOT_AVAILABLE;
}

#endif

