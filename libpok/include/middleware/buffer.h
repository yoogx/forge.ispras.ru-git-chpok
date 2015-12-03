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


#ifndef __POK_USER_BUFFER_H__
#define __POK_USER_BUFFER_H__

#include <config.h>

#ifdef POK_NEEDS_MIDDLEWARE
#ifdef POK_NEEDS_BUFFERS

#include <types.h>
#include <errno.h>

// must be at least MAX_NAME_LENGTH of ARINC653 (which is 30)
#define POK_BUFFER_MAX_NAME_LENGTH 30

/* 
 * This essentially mirrors ARINC-653 BUFFER_STATUS type.
 */
typedef struct {
   pok_range_t          nb_messages;
   pok_range_t          max_messages;
   pok_size_t           message_size;
   pok_range_t          waiting_processes;
} pok_buffer_status_t;

typedef struct pok_buffer_wait_list_t
{
    struct pok_buffer_wait_list_t *next;

    pok_thread_id_t thread;
    int priority;
    int64_t timeout;
    pok_ret_t result;
    
    union {
        struct {
            const char *data_ptr;
            pok_port_size_t data_size;
        } sending;

        struct {
            char *data_ptr;
            pok_port_size_t *data_size_ptr;
        } receiving;
    };
} pok_buffer_wait_list_t;

typedef struct
{
   /* Flag indicating that buffer has been created.
    * 
    * If it's not, all other members are undefined.
    */
   pok_bool_t                   ready;

   /* 
    * Pointer to buffer (where messages are stored) 
    * As of now, it's allocated from 
    * static pok_buffers_data[POK_CONFIG_BUFFER_DATA_SIZE],
    * but it cab be changed to be allocated elsewhere.
    */
   void                         *buffer;

   /*
    * Maximum message size (as given when buffer was created).
    */
   pok_port_size_t              msg_size;

   /*
    * Index of the first message (that is going to be read next).
    */
   pok_port_size_t              head_index;

   /*
    * Number of messages currently stored in the buffer.
    */
   pok_port_size_t              number_of_messages;          

   /*
    * Maximum number of messages that can be queued in the buffer.
    */
   pok_port_size_t              max_number_of_messages;       

   /*
    * Queueing discipline. We need one here to properly
    * sort processes in the wait list.
    */
   pok_queueing_discipline_t    discipline;

   /*
    * The synchronization object, obviously.
    */
   pok_event_id_t               lock;

   /*
    * The wait list. 
    * When buffer is empty, processes attempting to receive message go here.
    * Likewise, when it's full, writing processes go here.
    *
    * Note that since buffer can't be full and empty at the same time,
    * we can use a single list.
    *
    * Wait list entries are allocated on the stack of 
    * the corresponding waiting process.
    */
   pok_buffer_wait_list_t       *wait_list;

   /*
    * Buffer name, as specified when it was created.
    */
   char                         name[POK_BUFFER_MAX_NAME_LENGTH];
} pok_buffer_t;

// actual allocated data structures (as global static variables)
extern pok_buffer_t pok_buffers[];

extern char pok_buffers_data[]; 

pok_ret_t pok_buffer_create(
        const char                      *name, 
        pok_port_size_t                 num_messages, 
        pok_port_size_t                 msg_size, 
        pok_queueing_discipline_t       discipline,
        pok_buffer_id_t                 *id);

pok_ret_t pok_buffer_receive(
        pok_buffer_id_t                 id, 
        const int64_t                   timeout, 
        void                            *data, 
        pok_port_size_t                 *len);

pok_ret_t pok_buffer_send(
        pok_buffer_id_t                 id, 
        const void*                     data, 
        pok_port_size_t                 len, 
        int64_t                         timeout);

pok_ret_t pok_buffer_status(
        pok_buffer_id_t                 id,
        pok_buffer_status_t             *status);

pok_ret_t pok_buffer_id(
        const char                      *name,
        pok_buffer_id_t                 *id);

#endif
#endif

#endif
