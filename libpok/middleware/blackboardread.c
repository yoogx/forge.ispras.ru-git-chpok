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

#include <core/dependencies.h>

#ifdef POK_NEEDS_MIDDLEWARE
#ifdef POK_NEEDS_BLACKBOARDS
#include <errno.h>
#include <types.h>
#include <core/partition.h>
#include <core/event.h>
#include <libc/string.h>
#include <core/error.h>
#include <middleware/blackboard.h>
#include <utils.h>
#include <core/syscall.h>

pok_ret_t pok_blackboard_read (const pok_blackboard_id_t   id, 
                               const int64_t               timeout,
                               void*                       data,
                               pok_port_size_t*            len)
{
   if (id >= POK_CONFIG_NB_BLACKBOARDS)
   {
      return POK_ERRNO_EINVAL;
   }

   if (data == NULL)
   {
      return POK_ERRNO_EINVAL;
   }

   if (pok_blackboards[id].ready != TRUE)
   {
      return POK_ERRNO_EINVAL;
   }

   int64_t delay_ms = arinc_time_to_ms(timeout);
   pok_ret_t ret;
   if (pok_event_lock(pok_blackboards[id].lock) != POK_ERRNO_OK) {
        return POK_ERRNO_EINVAL;
   }

   if (pok_blackboards[id].empty) {
      if (delay_ms == 0) {
         pok_event_unlock(pok_blackboards[id].lock);
         return POK_ERRNO_EMPTY;
      } else {
         if (pok_current_partition_preemption_disabled() || pok_error_is_handler() == POK_ERRNO_OK) {
            pok_event_unlock(pok_blackboards[id].lock);
            return POK_ERRNO_MODE;
         }

         // ARINC's INFINITE_TIME_VALUE (-1) translates to 0 timeout of pok
         pok_blackboards[id].waiting_processes++;
         ret = pok_event_wait (pok_blackboards[id].lock, delay_ms < 0 ? 0 : delay_ms);
         pok_blackboards[id].waiting_processes--;
         if (ret != POK_ERRNO_OK)
         {
            pok_event_unlock (pok_blackboards[id].lock);
            return ret;
         }
      }
   }

   *len = pok_blackboards[id].current_message_size;
   memcpy (data, &pok_blackboards_data[pok_blackboards[id].index], *len);

   if (pok_event_unlock(pok_blackboards[id].lock) != POK_ERRNO_OK) {
        return POK_ERRNO_EINVAL;
   }

   return POK_ERRNO_OK;
}

#endif
#endif
