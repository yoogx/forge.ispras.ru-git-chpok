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
 * Created by julien on Thu Jan 15 23:34:13 2009 
 */


#ifdef POK_NEEDS_ARINC653_BUFFER
#include <arinc653/types.h>
#include <arinc653/buffer.h>
#include <types.h>
#include <middleware/port.h>
#include <middleware/buffer.h>

#include <core/partition.h>

#define MAP_ERROR(from, to) case (from): *RETURN_CODE = (to); break
 
void CREATE_BUFFER ( 
       /*in */ BUFFER_NAME_TYPE         BUFFER_NAME, 
       /*in */ MESSAGE_SIZE_TYPE        MAX_MESSAGE_SIZE, 
       /*in */ MESSAGE_RANGE_TYPE       MAX_NB_MESSAGE, 
       /*in */ QUEUING_DISCIPLINE_TYPE  QUEUING_DISCIPLINE, 
       /*out*/ BUFFER_ID_TYPE           *BUFFER_ID, 
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE )
{
   pok_ret_t                  core_ret;
   pok_buffer_id_t            core_id;
   pok_queueing_discipline_t  core_discipline;
   
#ifdef POK_NEEDS_PARTITIONS
   pok_partition_mode_t operating_mode;
   pok_current_partition_get_operating_mode(&operating_mode);
   if (operating_mode == POK_PARTITION_MODE_NORMAL) {
      *RETURN_CODE = INVALID_MODE;
      return;
   }
#endif 

   switch (QUEUING_DISCIPLINE)
   {
     case PRIORITY:
         core_discipline = POK_PORT_QUEUEING_DISCIPLINE_PRIORITY;
         break;

     case FIFO:
         core_discipline = POK_PORT_QUEUEING_DISCIPLINE_FIFO;
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
   if (MAX_NB_MESSAGE > 0xFFFFFF / MAX_MESSAGE_SIZE) {
       // overflow
       // XXX unfortunately, limits.h isn't available yet
       *RETURN_CODE = INVALID_PARAM;
       return;
   }

   core_ret = pok_buffer_create (BUFFER_NAME, MAX_MESSAGE_SIZE * MAX_NB_MESSAGE, MAX_MESSAGE_SIZE, core_discipline, &core_id);
   
   *BUFFER_ID = core_id + 1;

   switch (core_ret) {
      MAP_ERROR(POK_ERRNO_OK, NO_ERROR);
      MAP_ERROR(POK_ERRNO_READY, NO_ACTION);
      MAP_ERROR(POK_ERRNO_EINVAL, INVALID_CONFIG);
      default: *RETURN_CODE = INVALID_CONFIG; // random error status, should never happen 
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
   core_ret = pok_buffer_send (BUFFER_ID - 1, MESSAGE_ADDR, LENGTH, TIME_OUT);

   switch (core_ret) {
      MAP_ERROR(POK_ERRNO_OK, NO_ERROR);
      MAP_ERROR(POK_ERRNO_EINVAL, INVALID_PARAM);
      MAP_ERROR(POK_ERRNO_FULL, NOT_AVAILABLE);
      default: *RETURN_CODE = INVALID_PARAM; // random error status, should never happen
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
   pok_port_size_t core_size;
   core_ret = pok_buffer_receive (BUFFER_ID - 1, TIME_OUT, MESSAGE_ADDR, &core_size);
   *LENGTH = (APEX_INTEGER) core_size;
   switch (core_ret) {
      MAP_ERROR(POK_ERRNO_OK, NO_ERROR);
      MAP_ERROR(POK_ERRNO_EINVAL, INVALID_PARAM);
      MAP_ERROR(POK_ERRNO_EMPTY, NOT_AVAILABLE);
      default: *RETURN_CODE = INVALID_PARAM; // random error status, should never happen
   }
}
 
void GET_BUFFER_ID ( 
       /*in */ BUFFER_NAME_TYPE         BUFFER_NAME, 
       /*out*/ BUFFER_ID_TYPE           *BUFFER_ID, 
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE )
{
   pok_buffer_id_t id;
   *RETURN_CODE = pok_buffer_id(BUFFER_NAME, &id);
   *BUFFER_ID = id + 1;
}
 
void GET_BUFFER_STATUS ( 
       /*in */ BUFFER_ID_TYPE           BUFFER_ID, 
       /*out*/ BUFFER_STATUS_TYPE       *BUFFER_STATUS, 
       /*out*/ RETURN_CODE_TYPE         *RETURN_CODE )
{
   pok_buffer_status_t status;
   *RETURN_CODE = pok_buffer_status(BUFFER_ID - 1, &status);
    
   BUFFER_STATUS->NB_MESSAGE = status.nb_messages;
   BUFFER_STATUS->MAX_NB_MESSAGE = status.max_messages;
   BUFFER_STATUS->MAX_MESSAGE_SIZE = status.message_size;
   BUFFER_STATUS->WAITING_PROCESSES = status.waiting_processes;
}
 
#endif 
