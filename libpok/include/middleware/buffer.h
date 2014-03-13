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

#ifdef POK_NEEDS_MIDDLEWARE
#ifdef POK_NEEDS_BUFFERS

#define POK_BUFFER_DISCIPLINE_FIFO 1
#define POK_BUFFER_DISCIPLINE_PRIORITY 2

// must be at least MAX_NAME_LENGTH of ARINC653
#define POK_BUFFER_MAX_NAME_LENGTH 30
#define POK_BUFFER_NAME_EQ(x, y) (strncmp((x), (y), POK_BUFFER_MAX_NAME_LENGTH) == 0)

#include <types.h>
#include <errno.h>

#include <core/lockobj.h>

typedef struct
{
   pok_bool_t                 ready;
   pok_bool_t                 empty;
   pok_bool_t                 full;
   pok_size_t                 size;
   pok_size_t                 index;
   pok_port_size_t            off_b;
   pok_port_size_t            off_e;
   pok_port_size_t            msgsize;
   pok_range_t                waiting_processes;
   pok_queueing_discipline_t  discipline;
   pok_event_id_t             lock;
   char                       name[POK_BUFFER_MAX_NAME_LENGTH];
}pok_buffer_t;

typedef struct
{
   pok_range_t          nb_messages;
   pok_range_t          max_messages;
   pok_size_t           message_size;
   pok_range_t          waiting_processes;
}pok_buffer_status_t;


pok_ret_t pok_buffer_create (char*                                 name, 
                             const pok_port_size_t                 num_messages, 
                             const pok_port_size_t                 msg_size, 
                             const pok_queueing_discipline_t       discipline,
                             pok_buffer_id_t*                      id);

pok_ret_t pok_buffer_receive (const pok_buffer_id_t                id, 
                              const int64_t                       timeout, 
                              void*                                data, 
                              pok_port_size_t*                     len);

pok_ret_t pok_buffer_send (const pok_buffer_id_t              id, 
                           const void*                        data, 
                           const pok_port_size_t              len, 
                           const int64_t                     timeout);

pok_ret_t pok_buffer_status (const pok_buffer_id_t            id,
                                   pok_buffer_status_t*       status);

pok_ret_t pok_buffer_id (char*                                     name,
                         pok_buffer_id_t*                          id);

#endif
#endif

#endif
