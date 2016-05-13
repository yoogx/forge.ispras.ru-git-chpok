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

/**
 * \file semaphore.c
 * \brief Provides ARINC653 API functionnalities for semaphore management.
 */

#include <config.h>

#ifdef POK_NEEDS_ARINC653_SEMAPHORE

#include <types.h>
#include <libc/string.h>
#include <arinc653/types.h>
#include <arinc653/semaphore.h>
#include <core/semaphore.h>
#include <core/partition.h>
#include <core/thread.h>
#include <core/error.h>
#include <utils.h>

#define MAP_ERROR(from, to) case (from): *RETURN_CODE = (to); break
#define MAP_ERROR_DEFAULT(to) default: *RETURN_CODE = (to); break

void CREATE_SEMAPHORE (SEMAPHORE_NAME_TYPE SEMAPHORE_NAME,
                       SEMAPHORE_VALUE_TYPE CURRENT_VALUE,
                       SEMAPHORE_VALUE_TYPE MAXIMUM_VALUE,
                       QUEUING_DISCIPLINE_TYPE QUEUING_DISCIPLINE,
                       SEMAPHORE_ID_TYPE *SEMAPHORE_ID,
                       RETURN_CODE_TYPE *RETURN_CODE )
{
   strtoupper(SEMAPHORE_NAME);
   pok_sem_id_t      sem_id;
   pok_ret_t         core_ret;
   pok_queuing_discipline_t core_discipline;

   switch (QUEUING_DISCIPLINE) {
      case FIFO: core_discipline = POK_QUEUING_DISCIPLINE_FIFO; break;
      case PRIORITY: core_discipline = POK_QUEUING_DISCIPLINE_PRIORITY; break;
      default: 
         *RETURN_CODE = INVALID_PARAM;
         return; 
   }
   
   core_ret = pok_semaphore_create(SEMAPHORE_NAME,
      CURRENT_VALUE, MAXIMUM_VALUE,
      core_discipline, &sem_id);
   
   switch (core_ret) {
      MAP_ERROR(POK_ERRNO_OK, NO_ERROR);
      MAP_ERROR(POK_ERRNO_PARTITION_MODE, INVALID_MODE);
      MAP_ERROR(POK_ERRNO_EXISTS, NO_ACTION);
      MAP_ERROR(POK_ERRNO_EINVAL, INVALID_PARAM);
      MAP_ERROR(POK_ERRNO_TOOMANY, INVALID_CONFIG);
      MAP_ERROR_DEFAULT(INVALID_CONFIG); // random error status, should never happen 
   }
   
   if(core_ret == POK_ERRNO_OK)
      *SEMAPHORE_ID = sem_id + 1;

   
   return;
}

void WAIT_SEMAPHORE (SEMAPHORE_ID_TYPE SEMAPHORE_ID,
                     SYSTEM_TIME_TYPE TIME_OUT,
                     RETURN_CODE_TYPE *RETURN_CODE )
{
   pok_ret_t core_ret;

   pok_time_t ms = TIME_OUT < 0? INFINITE_TIME_VALUE: arinc_time_to_ms(TIME_OUT);

   core_ret = pok_semaphore_wait(SEMAPHORE_ID - 1, &ms);
   
   switch (core_ret) {
      MAP_ERROR(POK_ERRNO_OK, NO_ERROR);
      MAP_ERROR(POK_ERRNO_MODE, INVALID_MODE);
      MAP_ERROR(POK_ERRNO_UNAVAILABLE, INVALID_PARAM);
      MAP_ERROR(POK_ERRNO_EMPTY, NOT_AVAILABLE);
      MAP_ERROR(POK_ERRNO_TIMEOUT, TIMED_OUT);
      MAP_ERROR_DEFAULT(INVALID_CONFIG); // random error status, should never happen 
   }
}

void SIGNAL_SEMAPHORE (SEMAPHORE_ID_TYPE SEMAPHORE_ID,
                       RETURN_CODE_TYPE *RETURN_CODE )
{
   pok_ret_t core_ret;
   
   core_ret = pok_semaphore_signal(SEMAPHORE_ID - 1);
   
   switch (core_ret) {
      MAP_ERROR(POK_ERRNO_OK, NO_ERROR);
      MAP_ERROR(POK_ERRNO_EINVAL, NO_ACTION);
      MAP_ERROR(POK_ERRNO_UNAVAILABLE, INVALID_PARAM);
      MAP_ERROR_DEFAULT(INVALID_CONFIG); // random error status, should never happen 
   }
}

void GET_SEMAPHORE_ID (SEMAPHORE_NAME_TYPE SEMAPHORE_NAME,
                       SEMAPHORE_ID_TYPE *SEMAPHORE_ID,
                       RETURN_CODE_TYPE *RETURN_CODE )
{
   strtoupper(SEMAPHORE_NAME);
   pok_sem_id_t id;
   pok_ret_t core_ret = pok_semaphore_id(SEMAPHORE_NAME, &id);

   switch (core_ret) {
      MAP_ERROR(POK_ERRNO_OK, NO_ERROR);
      MAP_ERROR(POK_ERRNO_UNAVAILABLE, INVALID_CONFIG);
      MAP_ERROR_DEFAULT(INVALID_CONFIG); // random error status, should never happen 
   }
   *SEMAPHORE_ID = id + 1;
}

void GET_SEMAPHORE_STATUS (SEMAPHORE_ID_TYPE SEMAPHORE_ID,
                           SEMAPHORE_STATUS_TYPE *SEMAPHORE_STATUS,
                           RETURN_CODE_TYPE *RETURN_CODE )
{
   pok_semaphore_status_t status;
   pok_ret_t core_ret = pok_semaphore_status(SEMAPHORE_ID - 1, &status);
   
   if (core_ret != POK_ERRNO_OK) {
      // shouldn't happen
      *RETURN_CODE = INVALID_PARAM;
      return;
   }

   SEMAPHORE_STATUS->CURRENT_VALUE = status.current_value;
   SEMAPHORE_STATUS->MAXIMUM_VALUE = status.maximum_value;
   SEMAPHORE_STATUS->WAITING_PROCESSES = status.waiting_processes;

   *RETURN_CODE = NO_ERROR;
}

#endif
