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

#include <libc/string.h>      /* For strcmp */

#include <errno.h>            /* For POK_ERRNO_... maccros */

#include <core/partition.h>
#include <core/thread.h>
#include <core/error.h>
#include <utils.h>
#include <types.h>

#define POK_EVENT_NAME_EQ(x, y) (strncmp((x), (y), POK_EVENT_MAX_NAME_LENGTH) == 0)

#define MAP_ERROR(from, to) case (from): *RETURN_CODE = (to); break
#define MAP_ERROR_DEFAULT(to) default: *RETURN_CODE = (to); break

bool_t pok_arinc653_events_initialized = 0;

static void init_events() 
{
   size_t i;
   for (i = 0; i < POK_CONFIG_ARINC653_NB_EVENTS; i++) {
      pok_arinc653_events_layers[i].ready = 0;
   }
   pok_arinc653_events_initialized = 1;
}

#define CHECK_EVENTS_INIT if (!pok_arinc653_events_initialized) init_events();

void CREATE_EVENT (EVENT_NAME_TYPE EVENT_NAME,
                   EVENT_ID_TYPE *EVENT_ID,
                   RETURN_CODE_TYPE *RETURN_CODE)
{
	new_toupper(EVENT_NAME);
   pok_event_id_t    core_id;
   pok_ret_t         core_ret;

   *RETURN_CODE = INVALID_CONFIG;

   CHECK_EVENTS_INIT

#ifdef POK_NEEDS_PARTITIONS
   pok_partition_mode_t operating_mode;
   pok_current_partition_get_operating_mode(&operating_mode);
   if (operating_mode == POK_PARTITION_MODE_NORMAL) {
      *RETURN_CODE = INVALID_MODE;
      return;
   }
#endif 

   // try to find existing one
   size_t i;
   for (i = 0; i < POK_CONFIG_ARINC653_NB_EVENTS; i++) {
	  if (pok_arinc653_events_layers[i].ready && 
          POK_EVENT_NAME_EQ(EVENT_NAME, pok_arinc653_events_layers[i].name)) 
      {
         *RETURN_CODE = NO_ACTION;
         return;
      }
   }

   // otherwise, create a new one
   for (i = 0; i < POK_CONFIG_ARINC653_NB_EVENTS; i++) {
      if (!pok_arinc653_events_layers[i].ready) {
         // found a free one

         // note: ARINC events never use "notify", only "broadcast"
         // it means that queueing discipline is irrelevant here
         core_ret = pok_event_create(&core_id, POK_QUEUEING_DISCIPLINE_PRIORITY);
         if (core_ret != POK_ERRNO_OK) {
            // XXX figure out exact cause of the error
            return;
         }

         pok_arinc653_event_layer_t *evt = &pok_arinc653_events_layers[i];

         evt->ready = TRUE;
         evt->core_id = core_id;
         evt->is_up = FALSE;
         strncpy(evt->name, EVENT_NAME, POK_EVENT_MAX_NAME_LENGTH);

         *EVENT_ID = i + 1;
         *RETURN_CODE = NO_ERROR;
         return;
      }
   }
   
   // out of events
   *RETURN_CODE = INVALID_CONFIG;
   return;
}

void SET_EVENT (EVENT_ID_TYPE EVENT_ID,
                RETURN_CODE_TYPE *RETURN_CODE)
{
   pok_ret_t core_ret;

   *RETURN_CODE = INVALID_PARAM;

   CHECK_EVENTS_INIT

   if (EVENT_ID <= 0 || EVENT_ID >= POK_CONFIG_ARINC653_NB_EVENTS) {
      *RETURN_CODE = INVALID_PARAM;
      return;
   }

   size_t events_layer_idx = EVENT_ID - 1;
   pok_arinc653_event_layer_t *evt = &pok_arinc653_events_layers[events_layer_idx];

   if (!evt->ready) {
      *RETURN_CODE = INVALID_PARAM;
      return;
   }

   evt->is_up = TRUE;

   core_ret = pok_event_broadcast(evt->core_id);
   if (core_ret == POK_ERRNO_OK) {
      // TODO yield only if someone's waiting
      pok_thread_yield(); 

      *RETURN_CODE = NO_ERROR;
      return;
   } else {
      *RETURN_CODE = INVALID_PARAM;
      return;
   }
}

void RESET_EVENT (EVENT_ID_TYPE EVENT_ID,
                  RETURN_CODE_TYPE *RETURN_CODE)
{
   if (EVENT_ID <= 0 || EVENT_ID >= POK_CONFIG_ARINC653_NB_EVENTS)
   {
      *RETURN_CODE = INVALID_PARAM;
      return;
   }
   
   size_t events_layer_idx = EVENT_ID - 1;
   pok_arinc653_event_layer_t *evt = &pok_arinc653_events_layers[events_layer_idx];

   if (!evt->ready)
   {
      *RETURN_CODE = INVALID_PARAM;
      return;
   }

   evt->is_up = FALSE;

   *RETURN_CODE = NO_ERROR;
   return;
}

void WAIT_EVENT (EVENT_ID_TYPE EVENT_ID,
                 SYSTEM_TIME_TYPE TIME_OUT,
                 RETURN_CODE_TYPE *RETURN_CODE)
{
   pok_ret_t core_ret;

   *RETURN_CODE = INVALID_PARAM;

   CHECK_EVENTS_INIT

   if (EVENT_ID <= 0 || EVENT_ID >= POK_CONFIG_ARINC653_NB_EVENTS)
   {
      *RETURN_CODE = INVALID_PARAM;
      return;
   }

   // XXX this code smells of race conditions
   
   size_t events_layer_idx = EVENT_ID - 1;
   pok_arinc653_event_layer_t *evt = &pok_arinc653_events_layers[events_layer_idx];

   if (!evt->ready)
   {
      *RETURN_CODE = INVALID_PARAM;
      return;
   }

   if (evt->is_up) {
      *RETURN_CODE = NO_ERROR;
      return;
   }

   if (TIME_OUT == 0) {
      // don't wait
      *RETURN_CODE = NOT_AVAILABLE;
      return;
   } 

   if (pok_current_partition_preemption_disabled() || pok_error_is_handler() == POK_ERRNO_OK) {
      *RETURN_CODE = INVALID_MODE;
      return;
   }

   int64_t delay_ms = arinc_time_to_ms(TIME_OUT);
   core_ret = pok_event_wait (pok_arinc653_events_layers[events_layer_idx].core_id, delay_ms);

   switch (core_ret)
   {
      case POK_ERRNO_OK:
         *RETURN_CODE = NO_ERROR;
         break;

      case POK_ERRNO_TIMEOUT:
         *RETURN_CODE = TIMED_OUT;
         break;

      default:
         *RETURN_CODE = INVALID_PARAM;
         break;
   }
}

void GET_EVENT_ID (EVENT_NAME_TYPE EVENT_NAME,
                   EVENT_ID_TYPE *EVENT_ID,
                   RETURN_CODE_TYPE *RETURN_CODE)
{
   new_toupper(EVENT_NAME);
   size_t i;

   *RETURN_CODE = INVALID_CONFIG;

   CHECK_EVENTS_INIT

   for (i = 0 ; i < POK_CONFIG_ARINC653_NB_EVENTS ; i++)
   {
      if (POK_EVENT_NAME_EQ(pok_arinc653_events_layers[i].name, EVENT_NAME))
      {
         *EVENT_ID = i + 1;
         *RETURN_CODE = NO_ERROR;
         return;
      }
   }
}

void GET_EVENT_STATUS (EVENT_ID_TYPE EVENT_ID,
                       EVENT_STATUS_TYPE *EVENT_STATUS,
                       RETURN_CODE_TYPE *RETURN_CODE)
{
   if (EVENT_ID <= 0 || EVENT_ID >= POK_CONFIG_ARINC653_NB_EVENTS)
   {
      *RETURN_CODE = INVALID_PARAM;
      return;
   }
   
   size_t events_layer_idx = EVENT_ID - 1;
   pok_arinc653_event_layer_t *evt = &pok_arinc653_events_layers[events_layer_idx];

   if (!pok_arinc653_events_layers[events_layer_idx].ready)
   {
      *RETURN_CODE = INVALID_PARAM;
      return;
   }
   
   // TODO
   EVENT_STATUS->EVENT_STATE = evt->is_up ? UP : DOWN;
   EVENT_STATUS->WAITING_PROCESSES = 0;

   *RETURN_CODE = NO_ERROR;
}

#endif
