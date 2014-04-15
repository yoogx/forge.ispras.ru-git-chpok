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


#ifdef POK_NEEDS_ARINC653_SAMPLING

#include <types.h>
#include <middleware/port.h>
#include <arinc653/types.h>
#include <arinc653/sampling.h>
#include <arinc653/partition.h>
#include <core/thread.h>
#include <utils.h>

#define MAP_ERROR(from, to) case (from): *RETURN_CODE = (to); break
#define MAP_ERROR_DEFAULT(to) default: *RETURN_CODE = (to); break

void CREATE_SAMPLING_PORT (
			 /*in */ SAMPLING_PORT_NAME_TYPE    SAMPLING_PORT_NAME,
			 /*in */ MESSAGE_SIZE_TYPE          MAX_MESSAGE_SIZE,
			 /*in */ PORT_DIRECTION_TYPE        PORT_DIRECTION,
			 /*in */ SYSTEM_TIME_TYPE           REFRESH_PERIOD,
			 /*out*/ SAMPLING_PORT_ID_TYPE      *SAMPLING_PORT_ID,
			 /*out*/ RETURN_CODE_TYPE           *RETURN_CODE )
{
	 pok_port_direction_t core_direction;
	 pok_port_id_t        core_id;
	 pok_ret_t            core_ret;
	 pok_thread_attr_t     attr;
	 uint32_t							process_id;

	 pok_thread_id(&process_id);
	 core_ret = pok_thread_status (process_id, &attr);
	 if (attr.state == NORMAL)
		 {
			 *RETURN_CODE = INVALID_MODE;
			 return ;
		 }
	 if (MAX_MESSAGE_SIZE <= 0 || REFRESH_PERIOD < 0)
		 {
			 *RETURN_CODE = INVALID_CONFIG;
			 return ;
		 }

	 switch (PORT_DIRECTION)
	 {
			case SOURCE:
				 core_direction = POK_PORT_DIRECTION_OUT;
				 break;

			case DESTINATION:
				 core_direction = POK_PORT_DIRECTION_IN;
				 break;

			default:
				 *RETURN_CODE = INVALID_CONFIG;
				 return;
	 }

         if (REFRESH_PERIOD < 0) {
             *RETURN_CODE = INVALID_CONFIG;
             return;
         }

         int64_t refresh_ms = arinc_time_to_ms(REFRESH_PERIOD);
    
         if (refresh_ms < 0 || refresh_ms > UINT32_MAX) {
            *RETURN_CODE = INVALID_CONFIG;
         }

	 core_ret = pok_port_sampling_create (SAMPLING_PORT_NAME, MAX_MESSAGE_SIZE, core_direction, (uint32_t) refresh_ms, &core_id);

	 *SAMPLING_PORT_ID = core_id + 1;

   switch (core_ret) {
      MAP_ERROR(POK_ERRNO_OK, NO_ERROR);
      MAP_ERROR(POK_ERRNO_EXISTS, NO_ACTION);
      MAP_ERROR(POK_ERRNO_MODE, INVALID_MODE);
      MAP_ERROR_DEFAULT(INVALID_CONFIG); // random error status, should never happen 
   }
}

void WRITE_SAMPLING_MESSAGE (
			 /*in */ SAMPLING_PORT_ID_TYPE      SAMPLING_PORT_ID,
			 /*in */ MESSAGE_ADDR_TYPE          MESSAGE_ADDR,     /* by reference */
			 /*in */ MESSAGE_SIZE_TYPE          LENGTH,
			 /*out*/ RETURN_CODE_TYPE           *RETURN_CODE )
{
    pok_ret_t core_ret;

    if (LENGTH <= 0) {
        *RETURN_CODE = INVALID_PARAM;
	return;
    }
    if (SAMPLING_PORT_ID == 0) {
        *RETURN_CODE = INVALID_PARAM;
        return;
    }
    core_ret = pok_port_sampling_write (SAMPLING_PORT_ID - 1, MESSAGE_ADDR, LENGTH);
    switch (core_ret) {
      MAP_ERROR(POK_ERRNO_OK, NO_ERROR);
      MAP_ERROR(POK_ERRNO_PORT, INVALID_PARAM); // port doesn't exists
      MAP_ERROR(POK_ERRNO_EINVAL, INVALID_CONFIG); // incorrect length
      MAP_ERROR(POK_ERRNO_DIRECTION, INVALID_MODE); 
      MAP_ERROR_DEFAULT(INVALID_CONFIG); // random error status, should never happen 
    }
}

void READ_SAMPLING_MESSAGE (
			 /*in */ SAMPLING_PORT_ID_TYPE      SAMPLING_PORT_ID,
			 /*out*/ MESSAGE_ADDR_TYPE          MESSAGE_ADDR,
			 /*out*/ MESSAGE_SIZE_TYPE          *LENGTH,
			 /*out*/ VALIDITY_TYPE              *VALIDITY,
			 /*out*/ RETURN_CODE_TYPE           *RETURN_CODE )
{
    pok_ret_t core_ret;
    
    if (SAMPLING_PORT_ID == 0) {
        *RETURN_CODE = INVALID_PARAM;
        return;
    }

    core_ret = pok_port_sampling_read (SAMPLING_PORT_ID - 1, MESSAGE_ADDR, (pok_port_size_t*) LENGTH, (pok_bool_t*) VALIDITY);
    switch (core_ret) {
        MAP_ERROR(POK_ERRNO_OK, NO_ERROR);
        MAP_ERROR(POK_ERRNO_EMPTY, NO_ACTION);
        MAP_ERROR(POK_ERRNO_EINVAL, INVALID_PARAM);
        MAP_ERROR(POK_ERRNO_DIRECTION, INVALID_MODE); 
        MAP_ERROR_DEFAULT(INVALID_CONFIG); // random error status, should never happen 
    }
}

void GET_SAMPLING_PORT_ID (
			 /*in */ SAMPLING_PORT_NAME_TYPE    SAMPLING_PORT_NAME,
			 /*out*/ SAMPLING_PORT_ID_TYPE      *SAMPLING_PORT_ID,
			 /*out*/ RETURN_CODE_TYPE           *RETURN_CODE )
{
    pok_ret_t core_ret;
    pok_port_id_t id;

    core_ret = pok_port_sampling_id(SAMPLING_PORT_NAME, &id);
    *SAMPLING_PORT_ID = id + 1;
    switch (core_ret) {
        MAP_ERROR(POK_ERRNO_OK, NO_ERROR);
        MAP_ERROR_DEFAULT(INVALID_CONFIG);
    }
}

void GET_SAMPLING_PORT_STATUS (
			 /*in */ SAMPLING_PORT_ID_TYPE      SAMPLING_PORT_ID,
			 /*out*/ SAMPLING_PORT_STATUS_TYPE  *SAMPLING_PORT_STATUS,
			 /*out*/ RETURN_CODE_TYPE           *RETURN_CODE )
{
    pok_ret_t core_ret;
    pok_port_sampling_status_t status;

    if (SAMPLING_PORT_ID == 0) {
        *RETURN_CODE = INVALID_PARAM;
        return;
    }

    core_ret = pok_port_sampling_status(SAMPLING_PORT_ID - 1, &status);
    
    if (core_ret == POK_ERRNO_OK) {
        SAMPLING_PORT_STATUS->REFRESH_PERIOD = ms_to_arinc_time(status.refresh);
        SAMPLING_PORT_STATUS->MAX_MESSAGE_SIZE = status.size;
        SAMPLING_PORT_STATUS->PORT_DIRECTION = (status.direction == POK_PORT_DIRECTION_OUT) ? SOURCE : DESTINATION;
        SAMPLING_PORT_STATUS->LAST_MSG_VALIDITY = status.validity;
    }

    switch (core_ret) {
        MAP_ERROR(POK_ERRNO_OK, NO_ERROR);
        MAP_ERROR_DEFAULT(INVALID_PARAM);
    }
}

#endif
