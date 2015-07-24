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


#ifdef POK_NEEDS_MIDDLEWARE
#ifdef POK_NEEDS_BLACKBOARDS

#include <errno.h>
#include <types.h>
#include <libc/string.h>
#include <middleware/blackboard.h>
#include <core/event.h>

pok_ret_t pok_blackboard_clear (const pok_blackboard_id_t id)
{
   if (id >= POK_CONFIG_NB_BLACKBOARDS)
   {
      return POK_ERRNO_EINVAL;
   }

   if (!pok_blackboards[id].ready) {
      return POK_ERRNO_EINVAL;
   }

   if (pok_event_lock(pok_blackboards[id].lock) != POK_ERRNO_OK) {
        return POK_ERRNO_EINVAL;
   }

   pok_blackboards[id].empty = TRUE;

   if (pok_event_unlock(pok_blackboards[id].lock) != POK_ERRNO_OK) {
        return POK_ERRNO_EINVAL;
   }
   return POK_ERRNO_OK;
}

#endif /* POK_NEEDS_BLACKBOARDS */
#endif
