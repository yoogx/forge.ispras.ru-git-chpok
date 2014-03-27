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

#include <core/dependencies.h>

#ifdef POK_NEEDS_ARINC653_QUEUEING

#include <types.h>
#include <middleware/port.h>
#include <arinc653/types.h>
#include <arinc653/queueing.h>

#define MAP_ERROR(from, to) case (from): *RETURN_CODE = (to); break
#define MAP_ERROR_DEFAULT(to) default: *RETURN_CODE = (to); break

void CREATE_QUEUING_PORT (
      /*in */ QUEUING_PORT_NAME_TYPE    QUEUING_PORT_NAME,
      /*in */ MESSAGE_SIZE_TYPE         MAX_MESSAGE_SIZE,
      /*in */ MESSAGE_RANGE_TYPE        MAX_NB_MESSAGE,
      /*in */ PORT_DIRECTION_TYPE       PORT_DIRECTION,
      /*in */ QUEUING_DISCIPLINE_TYPE   QUEUING_DISCIPLINE,
      /*out*/ QUEUING_PORT_ID_TYPE      *QUEUING_PORT_ID,
      /*out*/ RETURN_CODE_TYPE          *RETURN_CODE)
{
   pok_ret_t                        core_ret;
   pok_port_id_t                    core_id;
   pok_port_queueing_create_arg_t   arg;

   arg.name = QUEUING_PORT_NAME;
   arg.message_size = MAX_MESSAGE_SIZE;
   arg.max_nb_message = MAX_NB_MESSAGE;

   switch (QUEUING_DISCIPLINE)
   {
     case PRIORITY:
         arg.discipline = POK_PORT_QUEUEING_DISCIPLINE_PRIORITY;
         break;

     case FIFO:
         arg.discipline = POK_PORT_QUEUEING_DISCIPLINE_FIFO;
         break;

      default:
         *RETURN_CODE = INVALID_PARAM;
         return;
   }

   switch (PORT_DIRECTION)
   {
      case SOURCE:
         arg.direction = POK_PORT_DIRECTION_OUT;
         break;
      
      case DESTINATION:
         arg.direction = POK_PORT_DIRECTION_IN;
         break;

      default:
         *RETURN_CODE = INVALID_PARAM;
         return;
   }

   core_ret = pok_port_queueing_create(&arg, &core_id);

   *QUEUING_PORT_ID = core_id + 1;

   switch (core_ret) {
      MAP_ERROR(POK_ERRNO_OK, NO_ERROR);
      MAP_ERROR(POK_ERRNO_EXISTS, NO_ACTION);
      MAP_ERROR_DEFAULT(INVALID_CONFIG);
   }
}

void SEND_QUEUING_MESSAGE (
      /*in */ QUEUING_PORT_ID_TYPE      QUEUING_PORT_ID,
      /*in */ MESSAGE_ADDR_TYPE         MESSAGE_ADDR,       /* by reference */
      /*in */ MESSAGE_SIZE_TYPE         LENGTH,
      /*in */ SYSTEM_TIME_TYPE          TIME_OUT,
      /*out*/ RETURN_CODE_TYPE          *RETURN_CODE)
{
   pok_ret_t core_ret;

   if (QUEUING_PORT_ID == 0) {
       *RETURN_CODE = INVALID_PARAM;
       return;
   }

   core_ret = pok_port_queueing_send (QUEUING_PORT_ID - 1, MESSAGE_ADDR, LENGTH, TIME_OUT);

   *RETURN_CODE = core_ret;
}

void RECEIVE_QUEUING_MESSAGE (
      /*in */ QUEUING_PORT_ID_TYPE      QUEUING_PORT_ID,
      /*in */ SYSTEM_TIME_TYPE          TIME_OUT,
      /*out*/ MESSAGE_ADDR_TYPE         MESSAGE_ADDR,
      /*out*/ MESSAGE_SIZE_TYPE         *LENGTH,
      /*out*/ RETURN_CODE_TYPE          *RETURN_CODE )
{
   pok_ret_t core_ret;

   if (QUEUING_PORT_ID == 0) {
       *RETURN_CODE = INVALID_PARAM;
       return;
   }

   core_ret = pok_port_queueing_receive (QUEUING_PORT_ID - 1, TIME_OUT, *LENGTH, MESSAGE_ADDR, (pok_port_size_t*)LENGTH);

   if(core_ret == POK_ERRNO_EMPTY) core_ret = NOT_AVAILABLE;
   *RETURN_CODE = core_ret;
}

void GET_QUEUING_PORT_ID (
      /*in */ QUEUING_PORT_NAME_TYPE    QUEUING_PORT_NAME,
      /*out*/ QUEUING_PORT_ID_TYPE      *QUEUING_PORT_ID,
      /*out*/ RETURN_CODE_TYPE          *RETURN_CODE)
{
   pok_ret_t core_ret;
   // TODO this function isn't supposed to work

   core_ret = pok_port_queueing_id(QUEUING_PORT_NAME, *QUEUING_PORT_ID);
   *RETURN_CODE = core_ret;
}

void GET_QUEUING_PORT_STATUS (
      /*in */ QUEUING_PORT_ID_TYPE      QUEUING_PORT_ID,
      /*out*/ QUEUING_PORT_STATUS_TYPE *QUEUING_PORT_STATUS,
      /*out*/ RETURN_CODE_TYPE          *RETURN_CODE)
{
  pok_ret_t core_ret;
  pok_port_queueing_status_t status;
  
    if (QUEUING_PORT_ID == 0) {
       *RETURN_CODE = INVALID_PARAM;
       return;
    }

   core_ret = pok_port_queueing_status(QUEUING_PORT_ID - 1, &status);
   if (core_ret == POK_ERRNO_OK) {
       QUEUING_PORT_STATUS->NB_MESSAGE = status.nb_message;
       QUEUING_PORT_STATUS->MAX_NB_MESSAGE = status.max_nb_message;
       QUEUING_PORT_STATUS->MAX_MESSAGE_SIZE = status.max_message_size;
       switch (status.direction) {
         case POK_PORT_DIRECTION_OUT:
          QUEUING_PORT_STATUS->PORT_DIRECTION = SOURCE;
          break;
         case POK_PORT_DIRECTION_IN:
          QUEUING_PORT_STATUS->PORT_DIRECTION = DESTINATION;
          break;
         default:
          break; // XXX assert(0)
       }
       QUEUING_PORT_STATUS->WAITING_PROCESSES = status.waiting_processes;
       *RETURN_CODE = NO_ERROR;
   } else {
       *RETURN_CODE = INVALID_PARAM;
   }
}

void CLEAR_QUEUING_PORT (
      /*in */ QUEUING_PORT_ID_TYPE      QUEUING_PORT_ID,
      /*out*/ RETURN_CODE_TYPE          *return_code)
{
  (void) QUEUING_PORT_ID;
  *return_code = NOT_AVAILABLE;
}

#endif
