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
#ifdef POK_NEEDS_BUFFERS

#include <errno.h>
#include <types.h>
#include <core/time.h>
#include <core/event.h>
#include <core/thread.h>
#include <libc/string.h>
#include <middleware/buffer.h>
#include <utils.h>

extern pok_buffer_t    pok_buffers[POK_CONFIG_NB_BUFFERS];
extern char            pok_buffers_data[1024];

pok_ret_t pok_buffer_receive (const pok_buffer_id_t                id, 
                              const int64_t                       timeout, 
                              void*                                data, 
                              pok_port_size_t*                     len)
{
   pok_ret_t ret;

   if (id >= POK_CONFIG_NB_BUFFERS)
   {
      return POK_ERRNO_EINVAL;
   }

   if (pok_buffers[id].ready == FALSE)
   {
      return POK_ERRNO_EINVAL;
   }

   if (data == NULL)
   {
      return POK_ERRNO_EINVAL;
   }
   
   int64_t delay_ms = arinc_time_to_ms(timeout);

   pok_event_lock (pok_buffers[id].lock); 

   // TODO handle case of spontaneous wakeup
   while (pok_buffers[id].empty == TRUE)
   {
      if (delay_ms == 0)
      {
         pok_event_unlock (pok_buffers[id].lock);
         return POK_ERRNO_EMPTY;
      }
      else
      {
         // ARINC's INFINITE_TIME_VALUE (-1) translates to 0 timeout of pok
         pok_buffers[id].waiting_processes++;
         ret = pok_event_wait (pok_buffers[id].lock, delay_ms < 0 ? 0 : delay_ms);
         pok_buffers[id].waiting_processes--;
         if (ret != POK_ERRNO_OK)
         {
            pok_event_unlock (pok_buffers[id].lock);
            return ret;
         }
      }
   }

   size_t offset = pok_buffers[id].index + pok_buffers[id].off_b;
   pok_port_size_t msglen = *(pok_port_size_t *) &pok_buffers_data[offset];
   offset += sizeof(msglen);
   memcpy (data, &pok_buffers_data[offset], msglen);
   pok_buffers[id].off_b = (pok_buffers[id].off_b + pok_buffers[id].msgsize + sizeof(pok_port_size_t)) % pok_buffers[id].size;

   if (pok_buffers[id].off_b == pok_buffers[id].off_e)
   {
      pok_buffers[id].empty = TRUE;
   }

   pok_bool_t gotta_yield = pok_buffers[id].full && pok_buffers[id].waiting_processes > 0;

   pok_buffers[id].full = FALSE;

   *len = msglen;

   pok_event_unlock (pok_buffers[id].lock);

   pok_event_broadcast (pok_buffers[id].lock);

   if (gotta_yield) {
      pok_thread_yield();
   }

   return POK_ERRNO_OK;
}

#endif /* POK_NEEDS_BUFFERS */
#endif
