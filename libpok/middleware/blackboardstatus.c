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

#include <middleware/blackboard.h>
#include <errno.h>
#include <types.h>
#include <libc/string.h>

pok_ret_t pok_blackboard_status (const pok_blackboard_id_t id,
                                 pok_blackboard_status_t*  status)
{
   if (id >= POK_CONFIG_NB_BLACKBOARDS)
   {
      return POK_ERRNO_EINVAL;
   }

   if (!pok_blackboards[id].ready) {
      return POK_ERRNO_EINVAL;
   }
   //printf("getting pok_blackboards[%u].size = %ld\n", id, pok_blackboards[id].size);
   status->waiting_processes = pok_blackboards[id].waiting_processes;
   status->msg_size          = pok_blackboards[id].size;
   status->empty             = pok_blackboards[id].empty;

   return POK_ERRNO_OK;
}

#endif /* POK_NEEDS_BLACKBOARDS */
#endif
