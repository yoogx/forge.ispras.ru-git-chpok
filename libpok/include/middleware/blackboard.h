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


#ifndef  __POK_USER_BLACKBOARD_H__
#define __POK_USER_BLACKBOARD_H__

#include <config.h>

#ifdef POK_NEEDS_MIDDLEWARE
#ifdef POK_NEEDS_BLACKBOARDS

// must be at least MAX_NAME_LENGTH of ARINC653
#define POK_BLACKBOARD_MAX_NAME_LENGTH 30
#define POK_BLACKBOARD_NAME_EQ(x, y) (strncmp((x), (y), POK_BLACKBOARD_MAX_NAME_LENGTH) == 0)

#include <types.h>
#include <errno.h>

typedef struct
{
   pok_size_t          size;
   pok_size_t          current_message_size;
   pok_bool_t          empty;
   pok_range_t         waiting_processes;
   pok_size_t          index;
   pok_bool_t          ready;
   pok_event_id_t      lock;
   char                name[POK_BLACKBOARD_MAX_NAME_LENGTH];
}pok_blackboard_t;

typedef struct
{
   pok_port_size_t      msg_size;
   pok_bool_t           empty;
   pok_range_t          waiting_processes;
}pok_blackboard_status_t;


pok_ret_t pok_blackboard_create (char*                   name,
                                 const pok_size_t        msg_size,
                                 pok_blackboard_id_t*    id);

pok_ret_t pok_blackboard_read (const pok_blackboard_id_t      id,
                               const int64_t                  timeout,
                               void*                          data,
                               pok_port_size_t*               len);

pok_ret_t pok_blackboard_display (const pok_blackboard_id_t       id,
                                  const void*                     message,
                                  const pok_port_size_t           len);

pok_ret_t pok_blackboard_clear (const pok_blackboard_id_t               id);

pok_ret_t pok_blackboard_id     (char*                            name,
                                 pok_blackboard_id_t*             id);

pok_ret_t pok_blackboard_status (const pok_blackboard_id_t        id,
                                 pok_blackboard_status_t*         status);

extern pok_blackboard_t pok_blackboards[];
extern char pok_blackboards_data[]; 

#endif
#endif
#endif

