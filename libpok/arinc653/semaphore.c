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

/**
 * \file semaphore.c
 * \brief Provides ARINC653 API functionnalities for semaphore management.
 */

#include <config.h>

#ifdef POK_NEEDS_ARINC653_SEMAPHORE

#include <types.h>
#include <libc/string.h>
#include <arinc653/types.h>
#include <arinc653/semaphore.h>
#include <core/semaphore.h>
#include <core/partition.h>
#include <core/thread.h>
#include <core/error.h>
#include <utils.h>

#define POK_SEM_NAME_EQ(x, y) (strncmp((x), (y), POK_SEM_MAX_NAME_LENGTH) == 0)

#define MAP_ERROR(from, to) case (from): *RETURN_CODE = (to); break
#define MAP_ERROR_DEFAULT(to) default: *RETURN_CODE = (to); break

pok_arinc653_semaphore_layer_t *pok_arinc653_semaphores_layers;

pok_bool_t           pok_arinc653_semaphores_initialized = 0;

static void init_semaphores(void)
{
   size_t i;
   for (i = 0; i < POK_CONFIG_ARINC653_NB_SEMAPHORES; i++) {
      pok_arinc653_semaphores_layers[i].ready = 0;
   }
   pok_arinc653_semaphores_initialized = 1;
}

#define CHECK_SEM_INIT if (!pok_arinc653_semaphores_initialized) init_semaphores();

void CREATE_SEMAPHORE (SEMAPHORE_NAME_TYPE SEMAPHORE_NAME,
                       SEMAPHORE_VALUE_TYPE CURRENT_VALUE,
                       SEMAPHORE_VALUE_TYPE MAXIMUM_VALUE,
                       QUEUING_DISCIPLINE_TYPE QUEUING_DISCIPLINE,
                       SEMAPHORE_ID_TYPE *SEMAPHORE_ID,
                       RETURN_CODE_TYPE *RETURN_CODE )
{
   pok_sem_id_t      sem_id;
   pok_ret_t         core_ret;

   CHECK_SEM_INIT;

   *RETURN_CODE = INVALID_CONFIG;

   // XXX global creation lock?
   //

#ifdef POK_NEEDS_PARTITIONS
   pok_partition_mode_t operating_mode;
   pok_current_partition_get_operating_mode(&operating_mode);
   if (operating_mode == POK_PARTITION_MODE_NORMAL) {
      *RETURN_CODE = INVALID_MODE;
      return;
   }
#endif 

   // try to find existing one
   size_t i;
   for (i = 0; i < POK_CONFIG_ARINC653_NB_SEMAPHORES; i++) {
      if (pok_arinc653_semaphores_layers[i].ready && 
          POK_SEM_NAME_EQ(SEMAPHORE_NAME, pok_arinc653_semaphores_layers[i].name)) 
      {
         *RETURN_CODE = NO_ACTION;
         return;
      }
   }

   // create a new one
   // ...but first, check the parameters
   if (CURRENT_VALUE < 0 || 
       MAXIMUM_VALUE < 0 ||
       CURRENT_VALUE > MAXIMUM_VALUE) 
   {
      *RETURN_CODE = INVALID_PARAM;
      return;
   }
   pok_queueing_discipline_t discipline;
   switch (QUEUING_DISCIPLINE) {
      case FIFO: discipline = POK_QUEUEING_DISCIPLINE_FIFO; break;
      case PRIORITY: discipline = POK_QUEUEING_DISCIPLINE_PRIORITY; break;
      default: 
         *RETURN_CODE = INVALID_PARAM;
         return; 
   }
   for (i = 0; i < POK_CONFIG_ARINC653_NB_SEMAPHORES; i++) {
      if (!pok_arinc653_semaphores_layers[i].ready) {

         core_ret = pok_sem_create(&sem_id, CURRENT_VALUE, MAXIMUM_VALUE, discipline);
         if (core_ret != POK_ERRNO_OK) {
            // XXX figure out exact cause of the error
            if (core_ret == POK_ERRNO_LOCKOBJ_UNAVAILABLE) {
                // out of underlying lock objects
                *RETURN_CODE = INVALID_CONFIG;
                return;
            }
            *RETURN_CODE = INVALID_PARAM;
            return;
         }

         pok_arinc653_semaphores_layers[i].ready = 1;
         pok_arinc653_semaphores_layers[i].core_id = sem_id;
         strncpy(pok_arinc653_semaphores_layers[i].name, SEMAPHORE_NAME, POK_SEM_MAX_NAME_LENGTH);
         *RETURN_CODE = NO_ERROR;
         *SEMAPHORE_ID = i + 1;
         return;
      }
   }

   // out of semaphores
   *RETURN_CODE = INVALID_CONFIG;
   return;
}

void WAIT_SEMAPHORE (SEMAPHORE_ID_TYPE SEMAPHORE_ID,
                     SYSTEM_TIME_TYPE TIME_OUT,
                     RETURN_CODE_TYPE *RETURN_CODE )
{
   pok_ret_t core_ret;

   CHECK_SEM_INIT;

   if (SEMAPHORE_ID == 0) {
      *RETURN_CODE = INVALID_PARAM;
      return;
   }

   size_t sem_layer_idx = SEMAPHORE_ID - 1;

   if (sem_layer_idx >= POK_CONFIG_ARINC653_NB_SEMAPHORES)
   {
      *RETURN_CODE = INVALID_PARAM;
      return;
   }
   
   if (!pok_arinc653_semaphores_layers[sem_layer_idx].ready) 
   {
      *RETURN_CODE = INVALID_PARAM;
      return;
   }

   pok_bool_t dont_block = 
       TIME_OUT == 0 ||
       pok_current_partition_preemption_disabled() || 
       pok_error_is_handler() == POK_ERRNO_OK;

   if (dont_block) {
     // check semaphore without blocking
     core_ret = pok_sem_trywait(pok_arinc653_semaphores_layers[sem_layer_idx].core_id);
     switch (core_ret) {
        MAP_ERROR(POK_ERRNO_OK, NO_ERROR);
        MAP_ERROR(POK_ERRNO_TIMEOUT, NOT_AVAILABLE);
        MAP_ERROR_DEFAULT(INVALID_PARAM);
     }

     if (dont_block && TIME_OUT != 0) {
        // attempted to perform blocking operation in either
        // error handler or when preemption is locked
        *RETURN_CODE = INVALID_MODE;
     }
   } else {
      int64_t delay_ms = arinc_time_to_ms(TIME_OUT);


      core_ret = pok_sem_wait (pok_arinc653_semaphores_layers[sem_layer_idx].core_id, delay_ms < 0 ? 0 : delay_ms);

      switch (core_ret) {
         MAP_ERROR(POK_ERRNO_OK, NO_ERROR);
         MAP_ERROR(POK_ERRNO_TIMEOUT, TIMED_OUT);
         MAP_ERROR(POK_ERRNO_MODE, INVALID_MODE);
         MAP_ERROR_DEFAULT(INVALID_PARAM);
      }
   }

}

void SIGNAL_SEMAPHORE (SEMAPHORE_ID_TYPE SEMAPHORE_ID,
                       RETURN_CODE_TYPE *RETURN_CODE )
{
   pok_ret_t core_ret;

   CHECK_SEM_INIT;

   if (SEMAPHORE_ID == 0) {
      *RETURN_CODE = INVALID_PARAM;
      return;
   }

   size_t sem_layer_idx = SEMAPHORE_ID - 1;

   if (sem_layer_idx >= POK_CONFIG_ARINC653_NB_SEMAPHORES)
   {
      *RETURN_CODE = INVALID_PARAM;
      return;
   }

   if (!pok_arinc653_semaphores_layers[sem_layer_idx].ready) 
   {
      *RETURN_CODE = INVALID_PARAM;
      return;
   }

   core_ret = pok_sem_signal (pok_arinc653_semaphores_layers[sem_layer_idx].core_id);

   if (core_ret == POK_ERRNO_OK || core_ret == POK_ERRNO_FULL)
   {
      *RETURN_CODE = NO_ERROR;
      if (core_ret == POK_ERRNO_FULL) {
         // semaphore's value is already at max
         *RETURN_CODE = NO_ACTION;
      }
   }
   else
   {
      *RETURN_CODE = INVALID_PARAM;
   }
}

void GET_SEMAPHORE_ID (SEMAPHORE_NAME_TYPE SEMAPHORE_NAME,
                       SEMAPHORE_ID_TYPE *SEMAPHORE_ID,
                       RETURN_CODE_TYPE *RETURN_CODE )
{
   size_t i;

   CHECK_SEM_INIT;

   *RETURN_CODE = INVALID_CONFIG;

   for (i = 0 ; i < POK_CONFIG_ARINC653_NB_SEMAPHORES ; i++)
   {
      if (POK_SEM_NAME_EQ(pok_arinc653_semaphores_layers[i].name, SEMAPHORE_NAME))
      {
         *SEMAPHORE_ID = i + 1;
         *RETURN_CODE = NO_ERROR;
         break;
      }
   }
}

void GET_SEMAPHORE_STATUS (SEMAPHORE_ID_TYPE SEMAPHORE_ID,
                           SEMAPHORE_STATUS_TYPE *SEMAPHORE_STATUS,
                           RETURN_CODE_TYPE *RETURN_CODE )
{
   CHECK_SEM_INIT;

   if (SEMAPHORE_ID == 0) {
      *RETURN_CODE = INVALID_PARAM;
      return;
   }

   size_t sem_layer_idx = SEMAPHORE_ID - 1;

   if (sem_layer_idx >= POK_CONFIG_ARINC653_NB_SEMAPHORES) {
      *RETURN_CODE = INVALID_PARAM;
      return;
   }
   if (!pok_arinc653_semaphores_layers[sem_layer_idx].ready) {
      *RETURN_CODE = INVALID_PARAM;
      return;
   }

   pok_sem_status_t status;
   pok_ret_t core_ret = pok_sem_status(pok_arinc653_semaphores_layers[sem_layer_idx].core_id, &status);
   
   if (core_ret != POK_ERRNO_OK) {
      // shouldn't happen
      *RETURN_CODE = INVALID_PARAM;
      return;
   }

   SEMAPHORE_STATUS->CURRENT_VALUE = status.current_value;
   SEMAPHORE_STATUS->MAXIMUM_VALUE = status.maximum_value;
   SEMAPHORE_STATUS->WAITING_PROCESSES = status.waiting_processes;

   *RETURN_CODE = NO_ERROR;
}

#endif
