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
#include <libc/string.h>
#include <core/event.h>
#include <middleware/blackboard.h>

static pok_size_t pok_blackboards_data_index = 0;

pok_ret_t pok_blackboard_create (char*                             name, 
                                 const pok_port_size_t             msg_size, 
                                 pok_blackboard_id_t*              id)
{
   pok_ret_t   ret;
   uint8_t     n;

   if ((int) msg_size <= 0) {
       return POK_ERRNO_SIZE;
   }
   
   // XXX global blackboard create lock?

   // try to find existing blackboard
   for (n=0;  n < POK_CONFIG_NB_BLACKBOARDS ; n++) {
      if (pok_blackboards[n].ready && POK_BLACKBOARD_NAME_EQ(pok_blackboards[n].name, name)) {
         return POK_ERRNO_READY;
      }
   }
   
   // TODO ensure that we still have free space

   // create a new one
   for (n=0 ; n < POK_CONFIG_NB_BLACKBOARDS ; n++)
   {
      if (!pok_blackboards[n].ready) {
         ret = pok_event_create (&pok_blackboards[n].lock, POK_QUEUEING_DISCIPLINE_PRIORITY);

         if (ret != POK_ERRNO_OK)
         {
            return ret;
         }

         pok_blackboards[n].ready               = TRUE;
         pok_blackboards[n].empty               = TRUE;
         pok_blackboards[n].index               = pok_blackboards_data_index;
         pok_blackboards[n].waiting_processes   = 0;
         pok_blackboards[n].size                = msg_size;
         *id                                    = n;
         pok_blackboards_data_index             = pok_blackboards_data_index + msg_size;
         strncpy(pok_blackboards[n].name, name, POK_BLACKBOARD_MAX_NAME_LENGTH);
         return POK_ERRNO_OK;
      }
   }

   return POK_ERRNO_EINVAL;
}

#endif /* POK_NEEDS_BLACKBOARDS */
#endif /* POK_NEEDS_MIDDLEWARE */
