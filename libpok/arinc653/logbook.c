/*
 * Institute for System Programming of the Russian Academy of Sciences
 * Copyright (C) 2017 ISPRAS
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

#include <config.h>

#include "logbook.h"
#include <arinc653/types.h>
#include <types.h>
#include <utils.h>                  // Для ALIGN
#include "arinc_config.h"           // Для arinc_config_nlogbooks
#include <kernel_shared_data.h>     // Для статуса раздела
#include "arinc_alloc.h"            // Для функций и переменных выделения места


#include <string.h>                 // Для strncasecmp

static unsigned nlogbooks_used = 0;

static inline LOGBOOK_ID_TYPE index_to_id(unsigned index)
{
    return index + 1;
}

static inline unsigned id_to_index(LOGBOOK_ID_TYPE id)
{
    return id - 1;
}

/* Find logbook by name (in UPPERCASE). Returns NULL if not found. */
static struct arinc_logbook* find_logbook(const char* name)
{
    printf("Current number of logbooks is %d\n", arinc_config_nlogbooks);   // Todelete
    
    for(unsigned i = 0; i < arinc_config_nlogbooks; i++)
    {
        if(strncasecmp(arinc_logbooks[i].logbook_name, name, MAX_NAME_LENGTH) == 0)
        {
            return &arinc_logbooks[i];
        }
   }

   return NULL;
}

/* Возвращает индекс сообщения в буфере, соответствующего указанному смещению. */
static MESSAGE_RANGE_TYPE
buffer_message_index(struct arinc_logbook* logbook, MESSAGE_RANGE_TYPE offset)
{
    MESSAGE_RANGE_TYPE res = logbook->buffer_base_offset + offset;

    if (res >= logbook->max_nb_in_progress_messages) 
        res -= logbook->max_nb_in_progress_messages;

    return res;
}

/* Возвращает индекс сообщения в NVM, соответствующего указанному смещению. */
static MESSAGE_RANGE_TYPE
nvm_message_index(struct arinc_logbook* logbook, MESSAGE_RANGE_TYPE offset)
{
    MESSAGE_RANGE_TYPE res = logbook->nvm_base_offset + offset;

    if (res >= logbook->max_nb_logged_messages) 
        res -= logbook->max_nb_logged_messages;

    return res;
}

/* Возвращает указатель на сообщение по данному индексу в in-progress-буфере*/
static char*
buffer_message_at(struct arinc_logbook* logbook, MESSAGE_RANGE_TYPE index)
{
   return logbook->buffer_messages + (logbook->message_stride * index);
}

/* Возвращает указатель на сообщение по данному индексу в nvm*/
static char*
nvm_message_at(struct arinc_logbook* logbook, MESSAGE_RANGE_TYPE index)
{
   return logbook->nvm_messages + (logbook->message_stride * index);
}

static int get_entry(struct arinc_logbook* logbook, MESSAGE_RANGE_TYPE message_entry, 
                        WRITE_STATUS_TYPE *write_status, MESSAGE_SIZE_TYPE *LENGTH, 
                        MESSAGE_ADDR_TYPE MESSAGE_ADDR)
{
    MESSAGE_RANGE_TYPE message_index;
    
    if (message_entry < logbook->nb_in_progress_messages)
    {
        *write_status = IN_PROGRESS;
        message_index = logbook->buffer_base_offset + message_entry;
        if (message_index > + logbook->nb_in_progress_messages)
            message_index -= logbook->nb_in_progress_messages;
            
        *LENGTH = logbook->buffer_messages_size[message_index];
        char* message_src = buffer_message_at(logbook, message_index);
        memcpy(MESSAGE_ADDR, message_src, *LENGTH);
        return 0;

    }
    else if (message_entry < logbook->nb_logged_messages + logbook->nb_in_progress_messages)
    {
        *write_status = COMPLETE;
        message_index = logbook->nvm_base_offset + message_entry - logbook->nb_in_progress_messages;
        if (message_index > + logbook->nb_logged_messages)
            message_index -= logbook->nb_logged_messages;
            
        *LENGTH = logbook->nvm_messages_size[message_index];
        char* message_src = nvm_message_at(logbook, message_index);
        memcpy(MESSAGE_ADDR, message_src, *LENGTH);
        return 0;
        
        
    }
    else
    {
        *write_status = ABORTED;
        *LENGTH = 0;
        return -1;
    }
}

// Debug function: todelete in the future
void list_of_logbooks()
{
    printf("LIST_OF_LOGBOOKS:\n");
    unsigned i = 0;
    for (i = 0; i < arinc_config_nlogbooks; i++)
    {
        printf("Logbook №%d: \n", i+1);
        printf("Name: %s\n", arinc_logbooks[i].logbook_name);
        printf("Message size: %d\n", arinc_logbooks[i].max_message_size);
        printf("max_nb_logged_messages: %d\n", arinc_logbooks[i].max_nb_logged_messages);
        printf("max_nb_in_progress_messages: %d\n", arinc_logbooks[i].max_nb_in_progress_messages);
    }
}

void CREATE_LOGBOOK(
    LOGBOOK_NAME_TYPE     LOGBOOK_NAME,
    MESSAGE_SIZE_TYPE     MAX_MESSAGE_SIZE,
    MESSAGE_RANGE_TYPE    MAX_NB_LOGGED_MESSAGES,
    MESSAGE_RANGE_TYPE    MAX_NB_IN_PROGRESS_MESSAGES,
    LOGBOOK_ID_TYPE       *LOGBOOK_ID,
    RETURN_CODE_TYPE      *RETURN_CODE)
{
    // Check whether it is enough resources for creating new logbook
    if (nlogbooks_used >= arinc_config_nlogbooks)
    {
        *RETURN_CODE = INVALID_CONFIG;
        return;
    }

    struct arinc_logbook* logbook = find_logbook(LOGBOOK_NAME);

    // Check whether it is existing logbook with this name in configuration
    if (logbook == NULL)
    {
        *RETURN_CODE = INVALID_CONFIG;
        printf("There is no this logbook in configuration!\n");   // Todelete
        return;
    }
        
    // Check whether this logbook was already created
    if (logbook->is_created == TRUE)
    {
        *RETURN_CODE = NO_ACTION;
        return;
    }
        
    // Check an accordance of arguments to the configuration and ranges
    if (logbook->max_message_size != MAX_MESSAGE_SIZE || 
        logbook->max_message_size < 0)
    {
        *RETURN_CODE = INVALID_CONFIG;
        return;
    }
    
    if (logbook->max_nb_logged_messages != MAX_NB_LOGGED_MESSAGES ||
        logbook->max_nb_logged_messages < 0)
    {
        *RETURN_CODE = INVALID_CONFIG;
        return;
    }
        
    if (logbook->max_nb_in_progress_messages != MAX_NB_IN_PROGRESS_MESSAGES ||
        logbook->max_nb_in_progress_messages < 0)
    {
        *RETURN_CODE = INVALID_CONFIG;
        return;
    }
        
    if(kshd->partition_mode == POK_PARTITION_MODE_NORMAL) {
        // Cannot create buffer in NORMAL mode
        *RETURN_CODE = INVALID_MODE;
        return;
    }
        
    // Save current state of memory
    arinc_allocator_state astate = arinc_allocator_get_state();
    
    // Why int in alignof???
    // Is it connected to type of MESSAGE_RANGE_TYPE?
    // MESSAGE_RANGE_TYPE <- APEX_INTEGER <- LONG?????

    logbook->message_stride = ALIGN(logbook->max_message_size, __alignof__(int));  
    logbook->buffer_messages = arinc_alloc(logbook->message_stride * logbook->max_nb_in_progress_messages, __alignof__(int));
    if (logbook->buffer_messages == NULL)
    {
        arinc_allocator_reset_state(astate);
        *RETURN_CODE = INVALID_CONFIG;
        return;
    }
        
    logbook->buffer_messages_size = arinc_alloc(logbook->max_nb_in_progress_messages * sizeof(*logbook->buffer_messages_size),
      __alignof__(*logbook->buffer_messages_size));
      
    if (logbook->buffer_messages_size == NULL)
    {
        arinc_allocator_reset_state(astate);
        *RETURN_CODE = INVALID_CONFIG;
        return;
    }
        
    logbook->nvm_messages = arinc_alloc(logbook->message_stride * logbook->max_nb_logged_messages, __alignof__(int));
    if (logbook->nvm_messages == NULL) 
    {
        arinc_allocator_reset_state(astate);
        *RETURN_CODE = INVALID_CONFIG;
        return;
    }    
        
    logbook->nvm_messages_size = arinc_alloc(logbook->max_nb_logged_messages * sizeof(*logbook->nvm_messages_size),
      __alignof__(*logbook->nvm_messages_size));
    if (logbook->nvm_messages_size == NULL)
    {
        arinc_allocator_reset_state(astate);
        *RETURN_CODE = INVALID_CONFIG;
        return;
    }
        
    logbook->nvm_messages_status = arinc_alloc(logbook->max_nb_logged_messages * sizeof(*logbook->nvm_messages_status),
      __alignof__(*logbook->nvm_messages_status));
    if (logbook->nvm_messages_status == NULL)
    {
        arinc_allocator_reset_state(astate);
        *RETURN_CODE = INVALID_CONFIG;
        return;
    }
        
    logbook->nb_logged_messages = 0;
    logbook->nb_in_progress_messages = 0;
    logbook->nb_aborted_messages = 0;
    
    logbook->is_created = TRUE;

    *LOGBOOK_ID = index_to_id(nlogbooks_used);
    nlogbooks_used++;
    
    *RETURN_CODE = NO_ERROR;
}

void WRITE_LOGBOOK(
    LOGBOOK_ID_TYPE      LOGBOOK_ID,
    MESSAGE_ADDR_TYPE    MESSAGE_ADDR,
    MESSAGE_SIZE_TYPE    LENGTH,
    RETURN_CODE_TYPE     *RETURN_CODE)
{
    unsigned index = id_to_index(LOGBOOK_ID);
    if (index >= nlogbooks_used) 
    {
        // Incorrect logbook identificator
        printf("Incorrect identificator\n"); // Todelete
        *RETURN_CODE = INVALID_PARAM;
        return;
    }
    
    struct arinc_logbook *logbook = &arinc_logbooks[index];
    
    if (LENGTH <= 0 || LENGTH > logbook->max_message_size) 
    {
        // LENGTH is non-positive or too big.
        printf("Incorrect LENGTH\n"); // Todelete
        *RETURN_CODE = INVALID_PARAM;
        return;
    }
    
    if (logbook->nb_in_progress_messages == logbook->max_nb_in_progress_messages)
    {
        // The underlying NVM is busy
        *RETURN_CODE = NOT_AVAILABLE;
        return;
    }
    
    MESSAGE_RANGE_TYPE message_index = buffer_message_index(logbook, logbook->nb_in_progress_messages);
    char* message_dest = buffer_message_at(logbook, message_index);
    
    memcpy(message_dest, MESSAGE_ADDR, LENGTH);
    logbook->buffer_messages_size[message_index] = LENGTH;
    logbook->nb_in_progress_messages++;


    // What should do with transmition to the NVM?
    
    /*
    message_index = nvm_message_index(logbook, logbook->nb_logged_messages);
    char* message_dest = nvm_message_at(logbook, message_index);
    
    memcpy(message_dest, MESSAGE_ADDR, LENGTH);
    logbook->nvm_messages_size[message_index] = LENGTH;
    logbook->nb_logged_messages++;
    */
   
	*RETURN_CODE = NO_ERROR;
}

void OVERWRITE_LOGBOOK(
    LOGBOOK_ID_TYPE      LOGBOOK_ID,
    MESSAGE_ADDR_TYPE    MESSAGE_ADDR,    
    MESSAGE_SIZE_TYPE    LENGTH,
    RETURN_CODE_TYPE    *RETURN_CODE)
{

    unsigned index = id_to_index(LOGBOOK_ID);
    if (index >= nlogbooks_used) 
    {
        // Incorrect logbook identificator
        printf("Incorrect identificator\n"); // Todelete
        *RETURN_CODE = INVALID_PARAM;
        return;
    }
    
    struct arinc_logbook *logbook = &arinc_logbooks[index];

    if (logbook->max_nb_in_progress_messages < 2)
    {
        *RETURN_CODE = INVALID_CONFIG;
        return;
    }
    
    if (LENGTH <= 0 || LENGTH > logbook->max_message_size) 
    {
        // LENGTH is non-positive or too big.
        printf("Incorrect LENGTH\n"); // Todelete
        *RETURN_CODE = INVALID_PARAM;
        return;
    }
    
    
    if (logbook->max_nb_in_progress_messages == logbook->nb_in_progress_messages)
    {
        // overwrite most recent message (index = 0, actually that is the last message in 
        // used data structure)
        
        // Actually this branch of if is similar to the next, so it is possible to unite them
        MESSAGE_RANGE_TYPE message_index = buffer_message_index(logbook, logbook->nb_in_progress_messages);
        char* message_dest = buffer_message_at(logbook, message_index);
    
        memcpy(message_dest, MESSAGE_ADDR, LENGTH);
        logbook->buffer_messages_size[message_index] = LENGTH;
        logbook->nb_in_progress_messages++; 
        
    }
    else
    {
        // similar to WRITE_LOGBOOK
        MESSAGE_RANGE_TYPE message_index = buffer_message_index(logbook, logbook->nb_in_progress_messages);
        char* message_dest = buffer_message_at(logbook, message_index);
    
        memcpy(message_dest, MESSAGE_ADDR, LENGTH);
        logbook->buffer_messages_size[message_index] = LENGTH;
        logbook->nb_in_progress_messages++;
    }

	*RETURN_CODE = NO_ERROR;
}

void READ_LOGBOOK(
    LOGBOOK_ID_TYPE       LOGBOOK_ID,
    MESSAGE_RANGE_TYPE    LOGBOOK_ENTRY,
    MESSAGE_ADDR_TYPE     MESSAGE_ADDR,
    MESSAGE_SIZE_TYPE     *LENGTH,
    WRITE_STATUS_TYPE     *WRITE_STATUS,
    RETURN_CODE_TYPE      *RETURN_CODE)
{
    unsigned index = id_to_index(LOGBOOK_ID);
    if (index >= nlogbooks_used) 
    {
        // Incorrect logbook identificator
        printf("Incorrect identificator\n"); // Todelete
        *RETURN_CODE = INVALID_PARAM;
        return;
    }
    
    struct arinc_logbook *logbook = &arinc_logbooks[index];

    
    if (LOGBOOK_ENTRY >= logbook->max_nb_logged_messages)
    {
        *RETURN_CODE = INVALID_PARAM;
        return;
    }
    
    int status = get_entry(logbook, LOGBOOK_ENTRY, WRITE_STATUS, LENGTH, MESSAGE_ADDR);
    if (status == -1)
    {
        *RETURN_CODE = NO_ACTION;
        return;
    }
    *RETURN_CODE = NO_ERROR;
}

void CLEAR_LOGBOOK(
    LOGBOOK_ID_TYPE       LOGBOOK_ID,
    RETURN_CODE_TYPE      *RETURN_CODE)
{
    
    /*
    Как это делается в blackboard-ах:
    
        void CLEAR_BLACKBOARD (
           /*in  BLACKBOARD_ID_TYPE       BLACKBOARD_ID,
           /*out RETURN_CODE_TYPE         *RETURN_CODE )
    {
        unsigned index = id_to_index(BLACKBOARD_ID);
       if (index >= nblackboards_used) {
          // Incorrect blackboard identificator.
          *RETURN_CODE = INVALID_PARAM;
          return;
       }

       struct arinc_blackboard* blackboard = &arinc_blackboards[index];

       msection_enter(&blackboard->section);
       blackboard->message_size = 0;
       msection_leave(&blackboard->section);

       *RETURN_CODE = NO_ERROR;
    }
    */
    
	*RETURN_CODE = NO_ERROR;
}

void GET_LOGBOOK_ID(
    LOGBOOK_NAME_TYPE     LOGBOOK_NAME,
    LOGBOOK_ID_TYPE       *LOGBOOK_ID,
    RETURN_CODE_TYPE      *RETURN_CODE)
{
    struct arinc_logbook* logbook = find_logbook(LOGBOOK_NAME);
    if (logbook == NULL)
    {
        *RETURN_CODE = INVALID_CONFIG;
    }
    
	*LOGBOOK_ID = index_to_id(logbook - arinc_logbooks);
	*RETURN_CODE = NO_ERROR;
}

void GET_LOGBOOK_STATUS(
    LOGBOOK_ID_TYPE        LOGBOOK_ID,
    LOGBOOK_STATUS_TYPE    *LOGBOOK_STATUS,
    RETURN_CODE_TYPE       *RETURN_CODE)
{
    
    unsigned index = id_to_index(LOGBOOK_ID);
    if (index >= nlogbooks_used) 
    {
        // Incorrect logbook identificator.
        *RETURN_CODE = INVALID_PARAM;
        return;
    }
    
    struct arinc_logbook *logbook = &arinc_logbooks[index];
    
    // MUTEX_ENTER
    // NEED TO ADD!

	LOGBOOK_STATUS->MAX_MESSAGE_SIZE = logbook->max_message_size;
	LOGBOOK_STATUS->MAX_NB_LOGGED_MESSAGES = logbook->max_nb_logged_messages;
	LOGBOOK_STATUS->MAX_NB_IN_PROGRESS_MESSAGES = logbook->max_nb_in_progress_messages;
	LOGBOOK_STATUS->NB_LOGGED_MESSAGES = logbook->nb_logged_messages;
	LOGBOOK_STATUS->NB_IN_PROGRESS_MESSAGES = logbook->nb_in_progress_messages;
	LOGBOOK_STATUS->NB_ABORTED_MESSAGES = logbook->nb_aborted_messages;
	*RETURN_CODE = NO_ERROR;
    
    // MUTEX_LEAVE
    // NEED TO ADD!
}
