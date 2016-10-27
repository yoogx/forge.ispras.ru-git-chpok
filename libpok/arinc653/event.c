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

#include <config.h>

#ifdef POK_NEEDS_ARINC653_EVENT
#include <arinc653/types.h>
#include <arinc653/event.h>

#include <core/event.h>       /* For core function */

#include <errno.h>            /* For POK_ERRNO_... maccros */

#include <core/event.h>
#include <utils.h>

#define MAP_ERROR(from, to) case (from): *RETURN_CODE = (to); break
#define MAP_ERROR_DEFAULT(to) default: *RETURN_CODE = (to); break

void CREATE_EVENT (EVENT_NAME_TYPE EVENT_NAME,
                   EVENT_ID_TYPE *EVENT_ID,
                   RETURN_CODE_TYPE *RETURN_CODE)
{

   pok_event_id_t    core_id;
   pok_ret_t         core_ret;

   core_ret = pok_event_create(EVENT_NAME, &core_id);

   switch (core_ret) {
      MAP_ERROR(POK_ERRNO_OK, NO_ERROR);
      MAP_ERROR(POK_ERRNO_PARTITION_MODE, INVALID_MODE);
      MAP_ERROR(POK_ERRNO_EXISTS, NO_ACTION);
      MAP_ERROR(POK_ERRNO_TOOMANY, INVALID_CONFIG);
      MAP_ERROR_DEFAULT(INVALID_CONFIG); // random error status, should never happen 
   }

   if(core_ret == POK_ERRNO_OK)
   {
      *EVENT_ID = core_id + 1;
   }

   return;
}

void SET_EVENT (EVENT_ID_TYPE EVENT_ID,
                RETURN_CODE_TYPE *RETURN_CODE)
{
   pok_ret_t core_ret;

   core_ret = pok_event_set(EVENT_ID - 1);

   switch (core_ret) {
      MAP_ERROR(POK_ERRNO_OK, NO_ERROR);
      MAP_ERROR(POK_ERRNO_UNAVAILABLE, INVALID_PARAM);
      MAP_ERROR_DEFAULT(INVALID_CONFIG); // random error status, should never happen 
   }

   return;
}

void RESET_EVENT (EVENT_ID_TYPE EVENT_ID,
                  RETURN_CODE_TYPE *RETURN_CODE)
{
   pok_ret_t core_ret;

   core_ret = pok_event_reset(EVENT_ID - 1);

   switch (core_ret) {
      MAP_ERROR(POK_ERRNO_OK, NO_ERROR);
      MAP_ERROR(POK_ERRNO_UNAVAILABLE, INVALID_PARAM);
      MAP_ERROR_DEFAULT(INVALID_CONFIG); // random error status, should never happen 
   }

   return;
}

void WAIT_EVENT (EVENT_ID_TYPE EVENT_ID,
                 SYSTEM_TIME_TYPE TIME_OUT,
                 RETURN_CODE_TYPE *RETURN_CODE)
{
   pok_ret_t core_ret;
   pok_time_t ms = TIME_OUT < 0 ? INFINITE_TIME_VALUE : arinc_time_to_ms(TIME_OUT);
   
   core_ret = pok_event_wait(EVENT_ID - 1, &ms);

   switch (core_ret) {
      MAP_ERROR(POK_ERRNO_OK, NO_ERROR);
      MAP_ERROR(POK_ERRNO_UNAVAILABLE, INVALID_PARAM);
      MAP_ERROR(POK_ERRNO_EMPTY, NOT_AVAILABLE);
      MAP_ERROR(POK_ERRNO_MODE, INVALID_MODE);
      MAP_ERROR(POK_ERRNO_TIMEOUT, TIMED_OUT);
      MAP_ERROR_DEFAULT(INVALID_CONFIG); // random error status, should never happen 
   }

   return;
}

void GET_EVENT_ID (EVENT_NAME_TYPE EVENT_NAME,
                   EVENT_ID_TYPE *EVENT_ID,
                   RETURN_CODE_TYPE *RETURN_CODE)
{
   pok_event_id_t id;
   pok_ret_t core_ret = pok_event_id(EVENT_NAME, &id);

   switch (core_ret) {
      MAP_ERROR(POK_ERRNO_OK, NO_ERROR);
      MAP_ERROR(POK_ERRNO_EINVAL, INVALID_CONFIG);
      MAP_ERROR_DEFAULT(INVALID_CONFIG); // random error status, should never happen 
   }
   *EVENT_ID = id + 1;
}

void GET_EVENT_STATUS (EVENT_ID_TYPE EVENT_ID,
                       EVENT_STATUS_TYPE *EVENT_STATUS,
                       RETURN_CODE_TYPE *RETURN_CODE)
{
   pok_event_status_t core_status;
   pok_ret_t core_ret = pok_event_status(EVENT_ID - 1, &core_status);

   switch (core_ret) {
      MAP_ERROR(POK_ERRNO_OK, NO_ERROR);
      MAP_ERROR(POK_ERRNO_UNAVAILABLE, INVALID_PARAM);
      MAP_ERROR_DEFAULT(INVALID_CONFIG); // random error status, should never happen 
   }

   if(core_ret == POK_ERRNO_OK)
   {
      EVENT_STATUS->EVENT_STATE = core_status.is_up ? UP : DOWN;
      EVENT_STATUS->WAITING_PROCESSES = core_status.waiting_processes;
   }
}

#endif
