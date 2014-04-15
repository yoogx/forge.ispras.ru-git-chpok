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
#include <libc/string.h>
#include <middleware/buffer.h>
#include <utils.h>

extern pok_buffer_t    pok_buffers[POK_CONFIG_NB_BUFFERS];
pok_size_t             pok_buffers_data_index = 0;


pok_ret_t pok_buffer_create (char*                                 name, 
                             const pok_port_size_t                 num_messages, 
                             const pok_port_size_t                 msg_size, 
                             const pok_queueing_discipline_t       discipline,
                             pok_buffer_id_t*                      id)
{
   uint8_t     n;
   pok_ret_t   ret;

   // leave some space to save message length
   const pok_port_size_t size = num_messages * (msg_size + sizeof(pok_port_size_t));

   if (  size > INT32_MAX - pok_buffers_data_index ||
         pok_buffers_data_index + size >= 1024) {
      return POK_ERRNO_EINVAL;
   }

   // try to find existing buffer
   for (n=0 ; n < POK_CONFIG_NB_BUFFERS ; n++)
   {
      if (pok_buffers[n].ready && POK_BUFFER_NAME_EQ(name, pok_buffers[n].name)) {
         return POK_ERRNO_READY;
      }
   }

   // create a new one
   for (n=0 ; n < POK_CONFIG_NB_BUFFERS ; n++)
   {
      if (pok_buffers[n].ready == FALSE) {
         ret = pok_event_create (&pok_buffers[n].lock);

         if (ret != POK_ERRNO_OK)
         {
            return ret;
         }

         pok_buffers[n].index                = pok_buffers_data_index;
         pok_buffers[n].ready                = TRUE;
         pok_buffers[n].empty                = TRUE;
         pok_buffers[n].size                 = size;
         pok_buffers[n].msgsize              = msg_size;
         pok_buffers[n].waiting_processes    = 0;
         pok_buffers[n].off_e                = 0;
         pok_buffers[n].off_b                = 0;
         pok_buffers[n].discipline           = discipline;
         strncpy(pok_buffers[n].name, name, POK_BUFFER_MAX_NAME_LENGTH);

         pok_buffers_data_index              = pok_buffers_data_index + size;

         *id = n;

         return POK_ERRNO_OK;
      }

   }

   return POK_ERRNO_EINVAL;
}

#endif /* POK_NEEDS_BUFFERS */
#endif /* POK_NEEDS_MIDDLEWARE */
