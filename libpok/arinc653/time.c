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

#include <arinc653/types.h>
#include <arinc653/time.h>

#include <core/thread.h>
#include <core/time.h>
#include <types.h>

#include "map_error.h"

void TIMED_WAIT (SYSTEM_TIME_TYPE delay_time, RETURN_CODE_TYPE *RETURN_CODE)
{
   if (delay_time < 0) {
       // ARINC doesn't allow infinite sleep
       *RETURN_CODE = INVALID_PARAM;
       return;
   }

   jet_ret_t core_ret;
   core_ret = pok_thread_sleep(&delay_time);
    
   MAP_ERROR_BEGIN(core_ret)
      MAP_ERROR(EOK, NO_ERROR);
      MAP_ERROR(ETIMEDOUT, NO_ERROR);
      MAP_ERROR(JET_CANCELLED, NO_ERROR);
      MAP_ERROR(JET_INVALID_MODE, INVALID_MODE);
      // EFAULT is impossible
      MAP_ERROR_CANCELLED();
      MAP_ERROR_DEFAULT();
   MAP_ERROR_END()
}

void PERIODIC_WAIT (RETURN_CODE_TYPE *RETURN_CODE)
{
   jet_ret_t core_ret;
   core_ret = pok_thread_period ();

   MAP_ERROR_BEGIN(core_ret)
      MAP_ERROR(EOK, NO_ERROR);
      MAP_ERROR(JET_INVALID_MODE, INVALID_MODE);
      MAP_ERROR_DEFAULT();
   MAP_ERROR_END()
}

void GET_TIME (SYSTEM_TIME_TYPE *system_time, RETURN_CODE_TYPE *RETURN_CODE)
{
   *system_time = pok_time_get();
   *RETURN_CODE = NO_ERROR;
}

void REPLENISH (SYSTEM_TIME_TYPE budget_time, RETURN_CODE_TYPE *RETURN_CODE)
{
    jet_ret_t core_ret = pok_thread_replenish(&budget_time);

    MAP_ERROR_BEGIN(core_ret)
        MAP_ERROR(EOK, NO_ERROR);
        MAP_ERROR(JET_NOACTION, NO_ACTION);
        MAP_ERROR(JET_INVALID_MODE, INVALID_MODE);
        // EFAULT is impossible
        MAP_ERROR_DEFAULT();
    MAP_ERROR_END()
}
