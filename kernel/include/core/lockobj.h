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
 *  Copyright (C) 2014 Maxim Malkov, ISPRAS <malkov@ispras.ru> 
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


#ifndef __POK_KERNEL_LOCKOBJ_H__
#define __POK_KERNEL_LOCKOBJ_H__

#include <config.h>

#include <types.h>
#include <arch.h>

#ifndef POK_CONFIG_NB_LOCKOBJECTS
#define POK_CONFIG_NB_LOCKOBJECTS 0
#endif

/*
 * In POK, all locking objects are implemented in the same way. It avoids
 * code duplication and save the environnement because we will consume less
 * space on your disk and will save trees because stupid people that prints
 * the code will consume less paper.
 * Moreoever, if all lockobjects share the same code, it will be easy to
 * certify and verify the behavior of all.
 */

/*
 * Half-mutex, half-semaphore, half-condition variable.
 *
 * It operates in one of the following modes
 * (as specified by pok_lockobj_kind_t):
 *  - Mutex 
 *  - Semaphore (also fairly traditional)
 *  - Event (a mix of mutex and condition variable, also known as "monitor")
 */


typedef enum {
   POK_LOCKOBJ_KIND_MUTEX = 1,
   POK_LOCKOBJ_KIND_SEMAPHORE = 2,
   POK_LOCKOBJ_KIND_EVENT = 3
} pok_lockobj_kind_t;

/* All kind of lock objects we have in the kernel */

typedef struct {
    pok_lockobj_kind_t         kind;
    pok_queueing_discipline_t  queueing_policy;  
    pok_sem_value_t            initial_value;
    pok_sem_value_t            max_value;
} pok_lockobj_attr_t;

typedef struct pok_lockobj_queue_t {
    pok_thread_id_t             thread;
    uint8_t                     priority;
    struct pok_lockobj_queue_t  *next;
} pok_lockobj_queue_t;

typedef struct
{
   /* 
    * If false, lock has never been created.
    * The rest of its members are undefined
    */
   bool_t                     initialized;

   pok_spinlock_t             spin;
   pok_spinlock_t             eventspin;

   /* Is the mutex locked ? */
   bool_t                     is_locked;
   
   /*
    * Threads waiting to acquire lock.
    */
   pok_lockobj_queue_t        *waiting_thread_list; 

   /*
    * Threads waiting on condition variable 
    * (if lock is operating as POK_LOCKOBJ_KIND_EVENT).
    */
   pok_lockobj_queue_t        *event_waiting_thread_list;

   /*
    * Queueing discipline associated with both
    * mutex and event queues.
    *
    * XXX Perhaps it's a little inflexible.
    */
   pok_queueing_discipline_t  queueing_policy;

   pok_lockobj_kind_t         kind;
   
   
   /* Used by semaphore part of this lock object */
   pok_sem_value_t            current_value;
   pok_sem_value_t            max_value;
} pok_lockobj_t;


typedef enum {
   LOCKOBJ_LOCK_REGULAR = 1,
   LOCKOBJ_LOCK_TIMED   = 2
} pok_lockobj_lock_kind_t;

typedef enum
{
   LOCKOBJ_OPERATION_LOCK = 1,
   LOCKOBJ_OPERATION_UNLOCK = 2,
   LOCKOBJ_OPERATION_WAIT = 3,
   LOCKOBJ_OPERATION_SIGNAL = 4,
   LOCKOBJ_OPERATION_BROADCAST = 5,
   LOCKOBJ_OPERATION_TRYLOCK = 6,
   LOCKOBJ_OPERATION_SIGNAL_THREAD = 7,
}pok_lockobj_operation_t;

typedef struct
{
   pok_lockobj_operation_t    operation;
   pok_lockobj_kind_t         obj_kind;
   pok_lockobj_lock_kind_t    lock_kind;
   union {
      uint64_t                time;
      pok_thread_id_t         thread; // used by SIGNAL_THREAD only
   };
}pok_lockobj_lockattr_t;

// in user space it's called pok_sem_status_t
// layout is the same
typedef struct {
    uint32_t current_value;
    uint32_t maximum_value;
    uint32_t waiting_processes; 
} pok_lockobj_status_t; 

pok_ret_t pok_lockobj_create (pok_lockobj_t* obj, const pok_lockobj_attr_t* attr);
pok_ret_t pok_lockobj_init ();
pok_ret_t pok_lockobj_partition_create (pok_lockobj_id_t* id, const pok_lockobj_attr_t* attr);
pok_ret_t pok_lockobj_lock (pok_lockobj_t* obj, const pok_lockobj_lockattr_t* attr);
pok_ret_t pok_lockobj_lock3 (pok_lockobj_t* obj, const pok_lockobj_lockattr_t* attr, bool_t noblock);
pok_ret_t pok_lockobj_unlock (pok_lockobj_t* obj, const pok_lockobj_lockattr_t* attr);
pok_ret_t pok_lockobj_eventwait (pok_lockobj_t* obj, const uint64_t timeout);
pok_ret_t pok_lockobj_eventsignal (pok_lockobj_t* obj);
pok_ret_t pok_lockobj_eventsignal_thread(pok_lockobj_t *obj, pok_thread_id_t thread);
pok_ret_t pok_lockobj_eventbroadcast (pok_lockobj_t* obj);
pok_ret_t pok_lockobj_partition_wrapper (const pok_lockobj_id_t id, const pok_lockobj_lockattr_t* attr);
pok_ret_t pok_lockobj_partition_status (const pok_lockobj_id_t id, pok_lockobj_status_t* attr);

#endif
