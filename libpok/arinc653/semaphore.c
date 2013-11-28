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

/**
 * \file semaphore.c
 * \brief Provides ARINC653 API functionnalities for semaphore management.
 */

#ifdef POK_NEEDS_ARINC653_SEMAPHORE

#include <types.h>
#include <libc/string.h>
#include <arinc653/types.h>
#include <arinc653/semaphore.h>
#include <core/semaphore.h>
#include <core/partition.h>
#include <core/thread.h>

// must be at least MAX_NAME_LENGTH of ARINC653
#define POK_SEM_MAX_NAME_LENGTH 30
#define POK_SEM_NAME_EQ(x, y) (strncmp((x), (y), POK_SEM_MAX_NAME_LENGTH) == 0)

#define MAP_ERROR(from, to) case (from): *RETURN_CODE = (to); break
#define MAP_ERROR_DEFAULT(to) default: *RETURN_CODE = (to); break

typedef struct
{
   pok_bool_t        ready;
   pok_sem_id_t      core_id;
   char              name[POK_SEM_MAX_NAME_LENGTH];
} pok_arinc653_semaphore_layer_t;

pok_arinc653_semaphore_layer_t         pok_arinc653_semaphores_layers[POK_CONFIG_ARINC653_NB_SEMAPHORES];

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

   RETURN_CODE_TYPE  return_code_name;
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
      // XXX lockobj doesn't seem to care about locking policy anyway...
      case FIFO: discipline = 0; break;
      case PRIORITY: discipline = 1; break;
      default: 
         *RETURN_CODE = INVALID_PARAM;
         return; 
   }
   for (i = 0; i < POK_CONFIG_ARINC653_NB_SEMAPHORES; i++) {
      if (!pok_arinc653_semaphores_layers[i].ready) {

         core_ret = pok_sem_create(&sem_id, CURRENT_VALUE, MAXIMUM_VALUE, QUEUING_DISCIPLINE);
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
         *SEMAPHORE_ID = i;
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

   if (SEMAPHORE_ID >= POK_CONFIG_ARINC653_NB_SEMAPHORES)
   {
      *RETURN_CODE = INVALID_PARAM;
      return;
   }
   
   if (!pok_arinc653_semaphores_layers[SEMAPHORE_ID].ready) 
   {
      *RETURN_CODE = INVALID_PARAM;
      return;
   }

   if (TIME_OUT == 0) {
     // check semaphore without blocking
     core_ret = pok_sem_trywait(pok_arinc653_semaphores_layers[SEMAPHORE_ID].core_id);
     switch (core_ret) {
        MAP_ERROR(POK_ERRNO_OK, NO_ERROR);
        MAP_ERROR(POK_ERRNO_TIMEOUT, NOT_AVAILABLE);
        MAP_ERROR_DEFAULT(INVALID_PARAM);
     }
   } else {
      uint64_t delay_ms;
      if (TIME_OUT < 0) {
         delay_ms = 0;
      } else {
         delay_ms = (uint32_t) TIME_OUT / 1000000;
         if ((uint32_t) TIME_OUT % 1000000) delay_ms++;
      }
      core_ret = pok_sem_wait (pok_arinc653_semaphores_layers[SEMAPHORE_ID].core_id, delay_ms);

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

   if (SEMAPHORE_ID >= POK_CONFIG_ARINC653_NB_SEMAPHORES)
   {
      *RETURN_CODE = INVALID_PARAM;
      return;
   }

   if (!pok_arinc653_semaphores_layers[SEMAPHORE_ID].ready) 
   {
      *RETURN_CODE = INVALID_PARAM;
      return;
   }

   core_ret = pok_sem_signal (pok_arinc653_semaphores_layers[SEMAPHORE_ID].core_id);

   if (core_ret == POK_ERRNO_OK)
   {
      *RETURN_CODE = NO_ERROR;
      // XXX should yield only if there're waiting processes
      pok_thread_yield();
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
         *SEMAPHORE_ID = i;
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

   if (SEMAPHORE_ID >= POK_CONFIG_ARINC653_NB_SEMAPHORES) {
      *RETURN_CODE = INVALID_PARAM;
      return;
   }
   if (!pok_arinc653_semaphores_layers[SEMAPHORE_ID].ready) {
      *RETURN_CODE = INVALID_PARAM;
      return;
   }

   pok_sem_status_t status;
   pok_ret_t core_ret = pok_sem_status(pok_arinc653_semaphores_layers[SEMAPHORE_ID].core_id, &status);
   
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
