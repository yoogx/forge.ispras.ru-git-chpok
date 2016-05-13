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

/* Intrapartition communication for ARINC. */

#ifndef __POK_INTRA_ARINC_H__
#define __POK_INTRA_ARINC_H__

#include <types.h>
#include <message.h>
#include <core/thread.h>

/************************* Buffer *************************************/

typedef struct {
    char name[MAX_NAME_LENGTH];
    
    pok_message_size_t max_message_size;
    pok_message_range_t max_nb_message;
    pok_message_range_t nb_message;
    
    pok_message_size_t message_stride;
    
    char* messages;
    
    pok_message_range_t base_offset;
    
    pok_queuing_discipline_t discipline;
    
    pok_thread_wq_t waiters;
    
} pok_buffer_t;

typedef struct {
    pok_message_range_t nb_message;
    pok_message_range_t max_nb_message;
    pok_message_size_t max_message_size;
    pok_range_t waiting_processes;
} pok_buffer_status_t;

/* (Re)initialize buffer structure. */
void pok_buffer_reset(pok_buffer_t* buffer);

pok_ret_t pok_buffer_create(char* __user name,
    pok_message_size_t max_message_size,
    pok_message_range_t max_nb_message,
    pok_queuing_discipline_t discipline,
    pok_buffer_id_t* __user id);


pok_ret_t pok_buffer_send(pok_buffer_id_t id,
    const void* __user data,
    pok_message_size_t length,
    const pok_time_t* __user timeout);

pok_ret_t pok_buffer_receive(pok_buffer_id_t id,
    const pok_time_t* __user timeout,
    void* __user data,
    pok_message_size_t* __user length);

pok_ret_t pok_buffer_get_id(char* __user name,
    pok_buffer_id_t* __user id);

pok_ret_t pok_buffer_status(pok_buffer_id_t id,
    pok_buffer_status_t* __user status);

/**************************** Blackboard ******************************/
typedef struct {
    char name[MAX_NAME_LENGTH];
    
    pok_message_size_t max_message_size;
    pok_message_size_t message_stride;
    
    pok_message_t* message;
    
    pok_thread_wq_t waiters;
} pok_blackboard_t;

typedef struct
{
   pok_message_size_t   max_message_size;
   pok_bool_t           is_empty;
   pok_range_t          waiting_processes;
}pok_blackboard_status_t;


pok_ret_t pok_blackboard_create (const char* __user             name,
                                 const pok_message_size_t       max_message_size,
                                 pok_blackboard_id_t* __user    id);

pok_ret_t pok_blackboard_read (pok_blackboard_id_t          id,
                               const pok_time_t* __user     timeout,
                               void* __user                 data,
                               pok_message_size_t* __user   len);

pok_ret_t pok_blackboard_display (pok_blackboard_id_t   id,
                                  const void* __user    message,
                                  pok_message_size_t    len);

pok_ret_t pok_blackboard_clear (pok_blackboard_id_t id);

pok_ret_t pok_blackboard_id     (const char* __user             name,
                                 pok_blackboard_id_t* __user    id);

pok_ret_t pok_blackboard_status (pok_blackboard_id_t                id,
                                 pok_blackboard_status_t* __user    status);

/**************************** Semaphore *******************************/
typedef struct {
    char name[MAX_NAME_LENGTH];
    
    pok_sem_value_t value;
    pok_sem_value_t max_value;
    
    pok_queuing_discipline_t discipline;
    
    pok_thread_wq_t waiters;
} pok_semaphore_t;

typedef struct {
    pok_sem_value_t current_value;
    pok_sem_value_t maximum_value;
    pok_range_t waiting_processes;
} pok_semaphore_status_t;

pok_ret_t pok_semaphore_create(const char* __user name,
    pok_sem_value_t value,
    pok_sem_value_t max_value,
    pok_queuing_discipline_t discipline,
    pok_sem_id_t* __user id);

pok_ret_t pok_semaphore_wait(pok_sem_id_t id,
    const pok_time_t* __user timeout);


pok_ret_t pok_semaphore_signal(pok_sem_id_t id);

pok_ret_t pok_semaphore_id(const char* __user name,
    pok_sem_id_t* __user id);

pok_ret_t pok_semaphore_status(pok_sem_id_t id,
    pok_semaphore_status_t* __user status);

/******************************* Event ********************************/
typedef struct {
    char name[MAX_NAME_LENGTH];
    
    pok_bool_t is_up;
    
    pok_thread_wq_t waiters;
} pok_event_t;

typedef struct {
    pok_bool_t is_up;
    pok_range_t waiting_processes;
} pok_event_status_t;


pok_ret_t pok_event_create(const char* __user name,
    pok_event_id_t* __user id);

pok_ret_t pok_event_set(pok_event_id_t id);
pok_ret_t pok_event_reset(pok_event_id_t id);

pok_ret_t pok_event_wait(pok_event_id_t id,
    const pok_time_t* timeout);

pok_ret_t pok_event_id(const char* __user name,
    pok_event_id_t* __user id);

pok_ret_t pok_event_status(pok_event_id_t id,
    pok_event_status_t* __user status);


#endif /* __POK_INTRA_ARINC_H__ */
