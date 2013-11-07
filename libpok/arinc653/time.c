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


#ifdef POK_NEEDS_ARINC653_TIME

#include <arinc653/types.h>
#include <arinc653/time.h>

#include <core/thread.h>
#include <core/time.h>
#include <types.h>


void TIMED_WAIT (SYSTEM_TIME_TYPE delay_time, RETURN_CODE_TYPE *return_code)
{
   // delay_time is in ns
   // since we need to sleep AT LEAST specified time,
   // round ms value up
   // FIXME leaving out (uint32_t) cast makes compiler generate 64-bit division
   //       which requires a library function
   //       which isn't implemented
   uint32_t delay_ms = (uint32_t) delay_time / 1000;
   if ((uint32_t) delay_time % 1000) delay_ms += 1;

   pok_syscall2(POK_SYSCALL_THREAD_SLEEP, delay_ms, 0);
   *return_code = NO_ERROR;
}

void PERIODIC_WAIT (RETURN_CODE_TYPE *return_code)
{
   pok_ret_t core_ret;
   core_ret = pok_thread_period ();
   *return_code = core_ret;
}

void GET_TIME (SYSTEM_TIME_TYPE *system_time, RETURN_CODE_TYPE *return_code)
{
   uint64_t time;
   pok_time_get(&time);
   *system_time = time * 1000;
   *return_code = NO_ERROR;
}

#ifndef POK_CONFIG_OPTIMIZE_FOR_GENERATED_CODE
void REPLENISH (SYSTEM_TIME_TYPE budget_time, RETURN_CODE_TYPE *return_code)
{
   (void) budget_time;
   *return_code = NOT_AVAILABLE;
}
#endif

#endif

