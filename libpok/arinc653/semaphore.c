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
 */

/**
 * \file semaphore.c
 * \brief Provides ARINC653 API functionnalities for semaphore management.
 */

#include <config.h>

#include "semaphore.h"

#include <arinc653/types.h>
#include <arinc653/semaphore.h>
#include <utils.h>

#include <kernel_shared_data.h>
#include <core/assert_os.h>

#include <string.h>
#include "arinc_config.h"
#include "arinc_process_queue.h"

#include "map_error.h"

static unsigned nsemaphores_used = 0;
static inline SEMAPHORE_ID_TYPE index_to_id(unsigned index)
{
    // Avoid 0 value.
    return index + 1;
}

static inline unsigned id_to_index(SEMAPHORE_ID_TYPE id)
{
    return id - 1;
}

/* Find semaphore by name (in UPPERCASE). Returns NULL if not found. */
static struct arinc_semaphore* find_semaphore(const char* name)
{
   for(unsigned i = 0; i < nsemaphores_used; i++)
   {
      struct arinc_semaphore* semaphore = &arinc_semaphores[i];
      if(strncasecmp(semaphore->semaphore_name, name, MAX_NAME_LENGTH) == 0)
         return semaphore;
   }

   return NULL;
}

void CREATE_SEMAPHORE (SEMAPHORE_NAME_TYPE SEMAPHORE_NAME,
                       SEMAPHORE_VALUE_TYPE CURRENT_VALUE,
                       SEMAPHORE_VALUE_TYPE MAXIMUM_VALUE,
                       QUEUING_DISCIPLINE_TYPE QUEUING_DISCIPLINE,
                       SEMAPHORE_ID_TYPE *SEMAPHORE_ID,
                       RETURN_CODE_TYPE *RETURN_CODE )
{
   if(kshd->partition_mode == POK_PARTITION_MODE_NORMAL) {
      // Cannot create semaphore in NORMAL mode
      *RETURN_CODE = INVALID_MODE;
      return;
   }

   if(find_semaphore(SEMAPHORE_NAME) != NULL) {
      // Semaphore with given name already exists.
      *RETURN_CODE = NO_ACTION;
      return;
   }

   if(nsemaphores_used == arinc_config_nsemaphores) {
      // Too many semaphores
      *RETURN_CODE = INVALID_CONFIG;
      return;
   }

   if(MAXIMUM_VALUE < 0 || MAXIMUM_VALUE > MAX_SEMAPHORE_VALUE) {
      // Maximum value is negative or too high.
      // DEV: Current implementation SIGNAL_SEMAPHORE allows to signal semaphore with maximum value, if there are waiters.
      *RETURN_CODE = INVALID_PARAM;
      return;
   }

   if(CURRENT_VALUE < 0 || CURRENT_VALUE > MAXIMUM_VALUE) {
      // Current value is negative or too high.
      *RETURN_CODE = INVALID_PARAM;
      return;
   }

   if((QUEUING_DISCIPLINE != FIFO) && (QUEUING_DISCIPLINE != PRIORITY)) {
      // Incorrect discipline value.
      *RETURN_CODE = INVALID_PARAM;
      return;
   }

   struct arinc_semaphore* semaphore = &arinc_semaphores[nsemaphores_used];

   memcpy(semaphore->semaphore_name, SEMAPHORE_NAME, MAX_NAME_LENGTH);
   semaphore->maximum_value = MAXIMUM_VALUE;
   semaphore->current_value = CURRENT_VALUE;
   semaphore->discipline = QUEUING_DISCIPLINE;

   msection_init(&semaphore->section);
   msection_wq_init(&semaphore->process_queue);

   *SEMAPHORE_ID = index_to_id(nsemaphores_used);

   nsemaphores_used++;

   *RETURN_CODE = NO_ERROR;
}

void WAIT_SEMAPHORE (SEMAPHORE_ID_TYPE SEMAPHORE_ID,
                     SYSTEM_TIME_TYPE TIME_OUT,
                     RETURN_CODE_TYPE *RETURN_CODE )
{
    unsigned index = id_to_index(SEMAPHORE_ID);
   if (index >= nsemaphores_used) {
      // Incorrect semaphore identificator.
      *RETURN_CODE = INVALID_PARAM;
      return;
   }

   struct arinc_semaphore* semaphore = &arinc_semaphores[index];

   msection_enter(&semaphore->section);

   if(semaphore->current_value > 0) {
      // Current value is positive.
      semaphore->current_value--;
      *RETURN_CODE = NO_ERROR;
   }

   else if(TIME_OUT == 0) {
      // Current value is 0 but waiting is not requested.
      *RETURN_CODE = NOT_AVAILABLE;
   }

   else {
      // Current value is 0 and waiting is *requested* by the caller.
      // (whether waiting is *allowed* will be checked by the kernel.)
      pok_thread_id_t t = kshd->current_thread_id;

      arinc_process_queue_add_common(&semaphore->process_queue, semaphore->discipline);

      jet_ret_t core_ret = msection_wait(&semaphore->section, TIME_OUT);

      MAP_ERROR_BEGIN(core_ret)
         MAP_ERROR(EOK, NO_ERROR);
         MAP_ERROR(JET_INVALID_MODE, INVALID_MODE);
         MAP_ERROR(ETIMEDOUT, TIMED_OUT);
         MAP_ERROR_CANCELLED();
         MAP_ERROR_DEFAULT();
      MAP_ERROR_END()

      switch(core_ret)
      {
      case EOK:
         // Current value is already decremented for us by the notifier.
         break;
      case JET_INVALID_MODE: // Waiting is not allowed
      case JET_CANCELLED: // Thread has been STOP()-ed or [IPPC] server thread has been cancelled.
      case ETIMEDOUT: // Timeout
         msection_wq_del(&semaphore->process_queue, t);
         break;
      default:
         assert_os(FALSE);
      }
   }

   msection_leave(&semaphore->section);
}

void SIGNAL_SEMAPHORE (SEMAPHORE_ID_TYPE SEMAPHORE_ID,
                       RETURN_CODE_TYPE *RETURN_CODE )
{
    unsigned index = id_to_index(SEMAPHORE_ID);
   if (index >= nsemaphores_used) {
      // Incorrect semaphore identificator.
      *RETURN_CODE = INVALID_PARAM;
      return;
   }

   struct arinc_semaphore* semaphore = &arinc_semaphores[index];

   msection_enter(&semaphore->section);

   if(msection_wq_notify(&semaphore->section,
      &semaphore->process_queue, FALSE) == EOK) {
      // There are waiters on semaphore. We have already awoken the first of them.
      /*
       * DEV: ARINC says to emit error if current_value == maximum_value.
       *
       * But we allow to signal such semaphore if someone waits on it.
       */
      pok_thread_id_t t_awoken = semaphore->process_queue.first;
      msection_wq_del(&semaphore->process_queue, t_awoken);
      *RETURN_CODE = NO_ERROR;
   }

   else if(semaphore->current_value == semaphore->maximum_value) {
      // Current value achive the maximum.
      *RETURN_CODE = NO_ACTION;
   }

   else {
      // No one waits on semaphore. Just increment its current value.
      semaphore->current_value ++;
      *RETURN_CODE = NO_ERROR;
   }

   msection_leave(&semaphore->section);
}

void GET_SEMAPHORE_ID (SEMAPHORE_NAME_TYPE SEMAPHORE_NAME,
                       SEMAPHORE_ID_TYPE *SEMAPHORE_ID,
                       RETURN_CODE_TYPE *RETURN_CODE )
{

   struct arinc_semaphore* semaphore = find_semaphore(SEMAPHORE_NAME);
   if(semaphore == NULL) {
      *RETURN_CODE = INVALID_CONFIG;
      return;
   }

   *SEMAPHORE_ID = index_to_id(semaphore - arinc_semaphores);
   *RETURN_CODE = NO_ERROR;
}

void GET_SEMAPHORE_STATUS (SEMAPHORE_ID_TYPE SEMAPHORE_ID,
                           SEMAPHORE_STATUS_TYPE *SEMAPHORE_STATUS,
                           RETURN_CODE_TYPE *RETURN_CODE )
{
    unsigned index = id_to_index(SEMAPHORE_ID);
   if (index >= nsemaphores_used) {
      // Incorrect semaphore identificator.
      *RETURN_CODE = INVALID_PARAM;
      return;
   }

   struct arinc_semaphore* semaphore = &arinc_semaphores[index];

   SEMAPHORE_STATUS->MAXIMUM_VALUE = semaphore->maximum_value;

   msection_enter(&semaphore->section);

   SEMAPHORE_STATUS->CURRENT_VALUE = semaphore->current_value;
   SEMAPHORE_STATUS->WAITING_PROCESSES = msection_wq_size(&semaphore->section,
      &semaphore->process_queue);

   msection_leave(&semaphore->section);

   *RETURN_CODE = NO_ERROR;
}
