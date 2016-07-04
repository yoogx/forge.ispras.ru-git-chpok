/*
 * Institute for System Programming of the Russian Academy of Sciences
 * Copyright (C) 2016 ISPRAS
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, Version 3.
 *
 * This program is distributed in the hope # that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License version 3 for more details.
 *
 * This file also incorporates work covered by POK License.
 * Copyright (c) 2007-2009 POK team
 */

#include <config.h>

#ifdef POK_NEEDS_ARINC653_BUFFER
#include <arinc653/types.h>
#include <arinc653/buffer.h>
#include <types.h>
#include <core/buffer.h>
#include <utils.h>


#include <core/partition.h>

#define MAP_ERROR(from, to) case (from): *RETURN_CODE = (to); break
#define MAP_ERROR_DEFAULT(to) default: *RETURN_CODE = (to); break
 
void CREATE_BUFFER ( 
       /*in */ BUFFER_NAME_TYPE         BUFFER_NAME, 
       /*in */ MESSAGE_SIZE_TYPE        MAX_MESSAGE_SIZE, 
       /*in */ MESSAGE_RANGE_TYPE       MAX_NB_MESSAGE, 
       /*in */ QUEUING_DISCIPLINE_TYPE  QUEUING_DISCIPLINE, 
       /*out*/ BUFFER_ID_TYPE           *BUFFER_ID, 
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE )
{
   strtoupper(BUFFER_NAME);
   pok_ret_t                  core_ret;
   pok_buffer_id_t            core_id;
   pok_queuing_discipline_t   core_discipline;
   
   switch (QUEUING_DISCIPLINE)
   {
     case PRIORITY:
         core_discipline = POK_QUEUING_DISCIPLINE_PRIORITY;
         break;

     case FIFO:
         core_discipline = POK_QUEUING_DISCIPLINE_FIFO;
         break;

      default:
         *RETURN_CODE = INVALID_PARAM;
         return;
   }

   if (MAX_MESSAGE_SIZE <= 0) {
      *RETURN_CODE = INVALID_PARAM;
      return;
   }
   if (MAX_NB_MESSAGE <= 0) {
      // XXX the spec vaguely says "out of range"
      *RETURN_CODE = INVALID_PARAM;
      return;
   }
   if (MAX_NB_MESSAGE > INT32_MAX / MAX_MESSAGE_SIZE) {
       // overflow
       *RETURN_CODE = INVALID_PARAM;
       return;
   }

   core_ret = pok_buffer_create (BUFFER_NAME, MAX_MESSAGE_SIZE, MAX_NB_MESSAGE, core_discipline, &core_id);
   
   *BUFFER_ID = core_id + 1;

   switch (core_ret) {
      MAP_ERROR(POK_ERRNO_OK, NO_ERROR);
      MAP_ERROR(POK_ERRNO_EXISTS, NO_ACTION);
      MAP_ERROR(POK_ERRNO_EINVAL, INVALID_CONFIG);
      MAP_ERROR(POK_ERRNO_PARTITION_MODE, INVALID_MODE);
      MAP_ERROR_DEFAULT(INVALID_CONFIG); // random error status, should never happen 
   }
}
 
void SEND_BUFFER ( 
       /*in */ BUFFER_ID_TYPE           BUFFER_ID, 
       /*in */ MESSAGE_ADDR_TYPE        MESSAGE_ADDR,       /* by reference */ 
       /*in */ MESSAGE_SIZE_TYPE        LENGTH, 
       /*in */ SYSTEM_TIME_TYPE         TIME_OUT, 
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE )
{
   if (BUFFER_ID == 0) {
      *RETURN_CODE = INVALID_PARAM;
      return;
   }

   pok_ret_t core_ret;
   pok_time_t ms = TIME_OUT < 0 ? INFINITE_TIME_VALUE : arinc_time_to_ms(TIME_OUT);
   core_ret = pok_buffer_send (BUFFER_ID - 1, MESSAGE_ADDR, LENGTH, &ms);

   switch (core_ret) {
      MAP_ERROR(POK_ERRNO_OK, NO_ERROR);
      MAP_ERROR(POK_ERRNO_EINVAL, INVALID_PARAM);
      MAP_ERROR(POK_ERRNO_FULL, NOT_AVAILABLE);
      MAP_ERROR(POK_ERRNO_MODE, INVALID_MODE);
      MAP_ERROR(POK_ERRNO_TIMEOUT, TIMED_OUT);
      MAP_ERROR_DEFAULT(INVALID_PARAM); // random error status, should never happen 
   }
}
 
void RECEIVE_BUFFER ( 
       /*in */ BUFFER_ID_TYPE           BUFFER_ID, 
       /*in */ SYSTEM_TIME_TYPE         TIME_OUT, 
       /*out*/ MESSAGE_ADDR_TYPE        MESSAGE_ADDR, 
       /*out*/ MESSAGE_SIZE_TYPE        *LENGTH, 
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE )
{
   if (BUFFER_ID == 0) {
      *RETURN_CODE = INVALID_PARAM;
      return;
   }

   pok_ret_t core_ret;
   pok_message_size_t core_size;
   pok_time_t ms = TIME_OUT < 0 ? INFINITE_TIME_VALUE : arinc_time_to_ms(TIME_OUT);
   
   core_ret = pok_buffer_receive (BUFFER_ID - 1, &ms, MESSAGE_ADDR, &core_size);
   *LENGTH = (APEX_INTEGER) core_size;

   switch (core_ret) {
      MAP_ERROR(POK_ERRNO_OK, NO_ERROR);
      MAP_ERROR(POK_ERRNO_EINVAL, INVALID_PARAM);
      MAP_ERROR(POK_ERRNO_EMPTY, NOT_AVAILABLE);
      MAP_ERROR(POK_ERRNO_MODE, INVALID_MODE);
      MAP_ERROR(POK_ERRNO_TIMEOUT, TIMED_OUT);
      MAP_ERROR_DEFAULT(INVALID_PARAM); // random error status, should never happen 
   }
}
 
void GET_BUFFER_ID ( 
       /*in */ BUFFER_NAME_TYPE         BUFFER_NAME, 
       /*out*/ BUFFER_ID_TYPE           *BUFFER_ID, 
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE )
{
   strtoupper(BUFFER_NAME);
   pok_buffer_id_t id;
   pok_ret_t core_ret = pok_buffer_get_id(BUFFER_NAME, &id);

   switch (core_ret) {
      MAP_ERROR(POK_ERRNO_OK, NO_ERROR);
      MAP_ERROR(POK_ERRNO_EINVAL, INVALID_CONFIG);
      MAP_ERROR_DEFAULT(INVALID_CONFIG); // random error status, should never happen 
   }
   *BUFFER_ID = id + 1;
}
 
void GET_BUFFER_STATUS ( 
       /*in */ BUFFER_ID_TYPE           BUFFER_ID, 
       /*out*/ BUFFER_STATUS_TYPE       *BUFFER_STATUS, 
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE )
{
   pok_buffer_status_t status;
    
   if (BUFFER_ID == 0) {
      *RETURN_CODE = INVALID_PARAM;
      return;
   }

   *RETURN_CODE = pok_buffer_status(BUFFER_ID - 1, &status);
    
   BUFFER_STATUS->NB_MESSAGE = status.nb_message;
   BUFFER_STATUS->MAX_NB_MESSAGE = status.max_nb_message;
   BUFFER_STATUS->MAX_MESSAGE_SIZE = status.max_message_size;
   BUFFER_STATUS->WAITING_PROCESSES = status.waiting_processes;
}
 
#endif 
