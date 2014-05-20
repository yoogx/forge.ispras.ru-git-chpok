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


#ifndef __POK_LIBPOK_LOCKOBJ_H__
#define __POK_LIBPOK_LOCKOBJ_H__

#include <types.h>

typedef enum
{
   POK_LOCKOBJ_KIND_MUTEX = 1,
   POK_LOCKOBJ_KIND_SEMAPHORE = 2,
   POK_LOCKOBJ_KIND_EVENT = 3
}pok_lockobj_kind_t;


typedef struct
{
   pok_lockobj_kind_t            kind;
   pok_queueing_discipline_t     queueing_policy;  
   pok_sem_value_t               initial_value;
   pok_sem_value_t               max_value;
}pok_lockobj_attr_t;

typedef enum
{
   LOCKOBJ_LOCK_REGULAR = 1,
   LOCKOBJ_LOCK_TIMED   = 2
}pok_lockobj_lock_kind_t;

typedef enum
{
   LOCKOBJ_OPERATION_LOCK = 1,
   LOCKOBJ_OPERATION_UNLOCK = 2,
   LOCKOBJ_OPERATION_WAIT = 3,
   LOCKOBJ_OPERATION_SIGNAL = 4,
   LOCKOBJ_OPERATION_BROADCAST = 5,
   LOCKOBJ_OPERATION_TRYLOCK = 6,
}pok_lockobj_operation_t;

typedef struct
{
   pok_lockobj_operation_t    operation;
   pok_lockobj_kind_t         obj_kind;
   pok_lockobj_lock_kind_t    lock_kind;
   uint64_t                   time;
}pok_lockobj_lockattr_t;


#endif
