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
    
    uint16_t message_stride;
    
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
    pok_time_t timeout);

pok_ret_t pok_buffer_receive(pok_buffer_id_t* id,
    pok_time_t timeout,
    void* __user data,
    pok_message_size_t* __user length);

pok_ret_t pok_buffer_get_id(char* __user name,
    pok_buffer_id_t* __user id);

/**************************** Blackboard ******************************/
typedef struct {
    char name[MAX_NAME_LENGTH];
    
    pok_message_size_t max_message_size;
    
    pok_message_t* message;
    
    pok_thread_wq_t waiters;
} pok_blackboard_t;

//TODO: methods

/**************************** Semaphore *******************************/
typedef struct {
    char name[MAX_NAME_LENGTH];
    
    pok_sem_value_t value;
    
    pok_queuing_discipline_t discipline;
    
    pok_thread_wq_t waiters;
} pok_semaphore_t;

//TODO: methods


/******************************* Event ********************************/
typedef struct {
    char name[MAX_NAME_LENGTH];
    
    pok_sem_value_t value;
    
    pok_thread_wq_t waiters;
} pok_event_t;

//TODO: methods


#endif /* __POK_INTRA_ARINC_H__ */
