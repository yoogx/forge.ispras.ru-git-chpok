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

/**
 * \file    core/lockobj.c
 * \brief   Provides functionnalities for locking functions (mutexes, semaphores and so on)
 * \author  Julien Delange
 *
 * This file contains the implementation code for mutexes, conditions and
 * semaphores. This is implemented in the same file since the functionnalities
 * does not differ so much.
 */


#if defined (POK_NEEDS_LOCKOBJECTS) || defined (POK_NEEDS_PORTS_QUEUEING) || defined (POK_NEEDS_PORTS_SAMPLING)

#include <arch.h>
#include <errno.h>
#include <types.h>
#include <core/sched.h>
#include <core/time.h>
#include <core/partition.h>
#include <core/thread.h>
#include <core/lockobj.h>
#include <libc.h>

#include <assert.h>

pok_lockobj_t           pok_partitions_lockobjs[POK_CONFIG_NB_LOCKOBJECTS+1];

static void push_thread(pok_lockobj_queue_t **list, pok_lockobj_queue_t *entry)
{
    while (*list != NULL && (*list)->priority >= entry->priority) {
        list = &((**list).next);
    }
    entry->next = *list;
    *list = entry;
}

static pok_thread_id_t pop_thread(pok_lockobj_queue_t **list)
{
    if (*list != NULL) {
        pok_thread_id_t res = (**list).thread;
        *list = (**list).next;
        return res;
    }
    return IDLE_THREAD;
}

static void remove_thread(pok_lockobj_queue_t **list, pok_lockobj_queue_t *entry)
{
    while (*list != NULL && *list != entry) {
        list = &((**list).next);
    }

    if (*list == NULL) return;

    *list = (**list).next;
}

static uint8_t get_thread_priority(
    pok_thread_id_t id, 
    pok_queueing_discipline_t discipline)
{
    if (discipline == POK_QUEUEING_DISCIPLINE_FIFO) {
        return 0;
    } else if (discipline == POK_QUEUEING_DISCIPLINE_PRIORITY) {
        return pok_threads[id].priority;
    }

    assert(0);
}

/**
 * Init the array of lockobjects
 */
pok_ret_t pok_lockobj_init ()
{
#if POK_CONFIG_NB_LOCKOBJECTS > 0
   uint8_t i;

#ifdef POK_NEEDS_PARTITIONS
#ifdef POK_NEEDS_ERROR_HANDLING
   uint32_t total_lockobjects;

   total_lockobjects = 0;

   for ( i = 0 ; i < POK_CONFIG_NB_PARTITIONS ; i++)
   {
      total_lockobjects = total_lockobjects + pok_partitions[i].nlockobjs;
   }
   
   if (total_lockobjects != POK_CONFIG_NB_LOCKOBJECTS)
   {
      pok_kernel_error (POK_ERROR_KIND_KERNEL_CONFIG);
   }
#endif
#endif

   for ( i = 0 ; i < POK_CONFIG_NB_LOCKOBJECTS ; i++)
   {

      pok_partitions_lockobjs[i].initialized = FALSE;
   }
#endif
   return POK_ERRNO_OK;
}


pok_ret_t pok_lockobj_create (pok_lockobj_t* obj, const pok_lockobj_attr_t* attr)
{
   /* Check the kind of the locjobj, must have a declared kind
    * If not, of course, we reject the creation.
    */
   if ((attr->kind != POK_LOCKOBJ_KIND_MUTEX) && (attr->kind != POK_LOCKOBJ_KIND_SEMAPHORE) && (attr->kind != POK_LOCKOBJ_KIND_EVENT))
   {
      return POK_ERRNO_LOCKOBJ_KIND;
   }

   if (attr->kind == POK_LOCKOBJ_KIND_EVENT) {
       switch (attr->queueing_policy) {
          case POK_QUEUEING_DISCIPLINE_FIFO:
          case POK_QUEUEING_DISCIPLINE_PRIORITY:
             break;
          default:
             return POK_ERRNO_LOCKOBJ_POLICY;
       }
   }
 
   obj->queueing_policy = attr->queueing_policy;
   obj->kind            = attr->kind;
   obj->initialized     = TRUE;
   obj->spin            = 0;
   obj->is_locked       = FALSE;
   
   obj->waiting_thread_list = NULL;
   obj->event_waiting_thread_list = NULL;

   if (attr->kind == POK_LOCKOBJ_KIND_SEMAPHORE)
   {
      obj->current_value = attr->initial_value;
      obj->max_value     = attr->max_value;
      
      if (obj->current_value == 0)
      {
         obj->is_locked = TRUE;
      }
   }

   return POK_ERRNO_OK;
}

#ifdef POK_NEEDS_LOCKOBJECTS
pok_ret_t pok_lockobj_partition_create (pok_lockobj_id_t* id, const pok_lockobj_attr_t* attr)
{
   uint8_t pid;
   uint8_t mid;
   pok_ret_t ret;
   uint8_t lower_bound = 0;
   uint8_t upper_bound = 0;
   bool_t  found = FALSE;

   if (  (POK_CURRENT_PARTITION.mode != POK_PARTITION_MODE_INIT_COLD) &&
         (POK_CURRENT_PARTITION.mode != POK_PARTITION_MODE_INIT_WARM))
   {
      return POK_ERRNO_MODE;
   }
   
   pid = POK_SCHED_CURRENT_PARTITION;

   lower_bound = pok_partitions[pid].lockobj_index_low;
   upper_bound = pok_partitions[pid].lockobj_index_high;

   /*
    * Find a lockobject for the partition
    */
   mid = lower_bound;
   while (mid < upper_bound)
   {
      if (pok_partitions_lockobjs[mid].initialized == FALSE)
      {
         found = TRUE; /* Yeeepeee, we found a free lockobj for this partition */
         break;
      }
      mid++;
   }

   if (found == FALSE)
   {
      return POK_ERRNO_LOCKOBJ_UNAVAILABLE;
   }

   *id = mid;

   ret = pok_lockobj_create (&pok_partitions_lockobjs[mid], attr);

   if (ret != POK_ERRNO_OK)
   {
      return ret;
   }

   return POK_ERRNO_OK;
}
#endif

pok_ret_t pok_lockobj_eventwait (pok_lockobj_t* obj, const uint64_t timeout)
{
   pok_ret_t ret;

   if (obj->initialized == FALSE) {
      return POK_ERRNO_LOCKOBJ_NOTREADY;
   }

   if (obj->kind != POK_LOCKOBJ_KIND_EVENT) {
      return POK_ERRNO_EINVAL;
   }
   
   SPIN_LOCK (obj->eventspin);

   if (POK_CURRENT_PARTITION.lock_level > 0) {
      // thread would block on itself
      SPIN_UNLOCK (obj->eventspin);
      return POK_ERRNO_MODE;
   }
   
   // release mutex
   if (pok_lockobj_unlock (obj, NULL)) {
      SPIN_UNLOCK (obj->eventspin);
      return POK_ERRNO_UNAVAILABLE;
   }

   {
       // add process to the event list
       pok_lockobj_queue_t entry;
       entry.thread = POK_SCHED_CURRENT_THREAD;
       entry.priority = get_thread_priority(entry.thread, obj->queueing_policy);

       push_thread(&obj->event_waiting_thread_list, &entry); 

       if (timeout > 0) {
          pok_sched_lock_current_thread_timed (timeout);
       } else {
          pok_sched_lock_current_thread ();
       } 

       SPIN_UNLOCK (obj->eventspin);
       pok_sched (); // <-- sleep here
       SPIN_LOCK (obj->eventspin);

       // remove process from the event list
       remove_thread(&obj->event_waiting_thread_list, &entry);
   }

   // reacquire mutex (might also sleep)
   ret = pok_lockobj_lock3 (obj, NULL, FALSE);

   if (ret != POK_ERRNO_OK) {
      SPIN_UNLOCK (obj->eventspin);
      return ret;
   }

   /* Here, we come back after we wait*/
   if ((timeout != 0 ) && (POK_GETTICK() >= timeout)) {
      ret = POK_ERRNO_TIMEOUT;
   } else {
      ret = POK_ERRNO_OK;
   }

   SPIN_UNLOCK (obj->eventspin);

   return ret;
}

pok_ret_t pok_lockobj_eventsignal (pok_lockobj_t* obj)
{
   SPIN_LOCK(obj->eventspin);

   // wake up a single waiting process
   pok_thread_id_t thread_id = pop_thread(&obj->event_waiting_thread_list);

   // wake up a single waiting process
   if (thread_id != IDLE_THREAD) {
      pok_sched_unlock_thread(thread_id);
   }

   SPIN_UNLOCK(obj->eventspin);

   if (thread_id != IDLE_THREAD) {
       return POK_ERRNO_OK;
   } else {
       return POK_ERRNO_NOTFOUND;
   }
}

pok_ret_t pok_lockobj_eventbroadcast (pok_lockobj_t* obj)
{
   SPIN_LOCK(obj->eventspin);

   {
       pok_lockobj_queue_t *queue = obj->event_waiting_thread_list;
       while (queue) {
          pok_sched_unlock_thread(queue->thread);
          queue = queue->next;
       }
   }
   obj->event_waiting_thread_list = NULL;

   SPIN_UNLOCK(obj->eventspin);   
   return POK_ERRNO_OK;
}

pok_ret_t pok_lockobj_lock(pok_lockobj_t* obj, const pok_lockobj_lockattr_t* attr) {
    return pok_lockobj_lock3(obj, attr, FALSE);
}

pok_ret_t pok_lockobj_lock3 (pok_lockobj_t* obj, const pok_lockobj_lockattr_t* attr, bool_t noblock)
{
   uint64_t timeout = 0;

   if (obj->initialized == FALSE) {
      return POK_ERRNO_LOCKOBJ_NOTREADY;
   }
   
   SPIN_LOCK (obj->spin);

   if ( (obj->is_locked == FALSE ) /*&& (obj->thread_state[POK_SCHED_CURRENT_THREAD] == LOCKOBJ_STATE_UNLOCK )*/)
   {
      obj->is_locked = TRUE;
      SPIN_UNLOCK (obj->spin);
   }
   else
   {
      if (noblock){
         SPIN_UNLOCK (obj->spin);
         return POK_ERRNO_TIMEOUT;
      }

      if (POK_CURRENT_PARTITION.lock_level > 0) {
         // thread would block on itself
         SPIN_UNLOCK (obj->spin);
         return POK_ERRNO_MODE;
      }

      /*
       * attr->time corresponds to the timeout for the waiting object
       */
      if ((attr != NULL) && (attr->time > 0)) {
         timeout = attr->time;
      }
      
      pok_lockobj_queue_t entry;
      entry.thread = POK_SCHED_CURRENT_THREAD;
      entry.priority = get_thread_priority(entry.thread, obj->queueing_policy);

      while ( (obj->is_locked == TRUE ) /*|| (obj->thread_state[POK_SCHED_CURRENT_THREAD] == LOCKOBJ_STATE_LOCK)*/) 
      {
         // XXX does it mean we push it multiple times (in while loop)?
         push_thread(&obj->waiting_thread_list, &entry);

         if (timeout > 0)
         {
            pok_sched_lock_current_thread_timed (timeout);
            if (POK_GETTICK() >= timeout)
            {
               remove_thread(&obj->waiting_thread_list, &entry);
               SPIN_UNLOCK (obj->spin);
               return POK_ERRNO_TIMEOUT;
            }
         }
         else
         {
            pok_sched_lock_current_thread ();
         }
         
         SPIN_UNLOCK (obj->spin);
         pok_sched(); // <-- sleep here 
         SPIN_LOCK (obj->spin);
    
         // XXX?
         remove_thread(&obj->waiting_thread_list, &entry);
      }
      
      // sometimes it's already too late, so check timeout again
      if (timeout > 0 && POK_GETTICK() >= timeout) {
        // XXX is it still in the list?
        remove_thread(&obj->waiting_thread_list, &entry);
        SPIN_UNLOCK (obj->spin);
        return POK_ERRNO_TIMEOUT;
      }
      
      switch (obj->kind)
      {
         case POK_LOCKOBJ_KIND_SEMAPHORE:
         {
            obj->current_value--;
            if (obj->current_value == 0)
            {
               obj->is_locked = TRUE;
            }
            break;
         }
         
         case POK_LOCKOBJ_KIND_MUTEX:
         case POK_LOCKOBJ_KIND_EVENT:
         {
            obj->is_locked = TRUE;
            break;
         }
         
      }
      if (!noblock) 
      {
        pok_sched_unlock_thread (POK_SCHED_CURRENT_THREAD);
      }
   }

   SPIN_UNLOCK (obj->spin);

   return POK_ERRNO_OK;
}

pok_ret_t pok_lockobj_unlock (pok_lockobj_t* obj, const pok_lockobj_lockattr_t* attr)
{
   pok_ret_t ret;

   (void) attr;         /* unused at this time */
   
   if (obj->initialized == FALSE)
   {
      return POK_ERRNO_LOCKOBJ_NOTREADY;
   }
   
   SPIN_LOCK (obj->spin);

   ret = POK_ERRNO_OK;
   switch (obj->kind)
   {
      case POK_LOCKOBJ_KIND_SEMAPHORE:
      {
         if (obj->current_value < obj->max_value) {
            obj->current_value++;
         } else {
            ret = POK_ERRNO_FULL;
         }
         obj->is_locked = FALSE;
         break;
      }
      
      case POK_LOCKOBJ_KIND_EVENT:
      case POK_LOCKOBJ_KIND_MUTEX:
      {
         obj->is_locked = FALSE;
         break;
      }
   }  
   
   // now, unlock next thread from the waiting list (if any)
   pok_thread_id_t next_waiting = pop_thread(&obj->waiting_thread_list);

   if (next_waiting != IDLE_THREAD) {
      pok_sched_unlock_thread (next_waiting);
      SPIN_UNLOCK (obj->spin);
      pok_sched();
   } else {
      SPIN_UNLOCK (obj->spin);
   }

   return ret;
}

#ifdef POK_NEEDS_LOCKOBJECTS
pok_ret_t pok_lockobj_partition_wrapper (const pok_lockobj_id_t id, const pok_lockobj_lockattr_t* attr)
{
   /* First, we check that the locked object belongs to the partition
    * If not, we return an error
    */
   if (id < pok_partitions[POK_SCHED_CURRENT_PARTITION].lockobj_index_low)
   {
      return POK_ERRNO_EINVAL;
   }
   
   if ( id >= pok_partitions[POK_SCHED_CURRENT_PARTITION].lockobj_index_high)
   {
      return POK_ERRNO_EINVAL;
   }

   if (pok_partitions_lockobjs[id].kind != attr->obj_kind)
   {
      return POK_ERRNO_EINVAL;
   }

   switch (attr->operation)
   {
      case LOCKOBJ_OPERATION_LOCK:
         return pok_lockobj_lock3 (&pok_partitions_lockobjs[id], attr, FALSE);

      case LOCKOBJ_OPERATION_TRYLOCK:
         return pok_lockobj_lock3(&pok_partitions_lockobjs[id], attr, TRUE);

      case LOCKOBJ_OPERATION_UNLOCK:
         return pok_lockobj_unlock (&pok_partitions_lockobjs[id], attr);

      case LOCKOBJ_OPERATION_WAIT:
         return pok_lockobj_eventwait (&pok_partitions_lockobjs[id], attr->time);

      case LOCKOBJ_OPERATION_SIGNAL:
         return pok_lockobj_eventsignal (&pok_partitions_lockobjs[id]);

      case LOCKOBJ_OPERATION_BROADCAST:
         return pok_lockobj_eventbroadcast (&pok_partitions_lockobjs[id]);

      default:
         return POK_ERRNO_EINVAL;
   }
}

pok_ret_t pok_lockobj_partition_status(pok_lockobj_id_t id, pok_lockobj_status_t *status)
{
   if (id < pok_partitions[POK_SCHED_CURRENT_PARTITION].lockobj_index_low)
   {
      return POK_ERRNO_EINVAL;
   }
   
   if ( id >= pok_partitions[POK_SCHED_CURRENT_PARTITION].lockobj_index_high)
   {
      return POK_ERRNO_EINVAL;
   }

   const pok_lockobj_t *obj = &pok_partitions_lockobjs[id];

   if (!obj->initialized) 
   {
      return POK_ERRNO_LOCKOBJ_NOTREADY;
   }
   
   status->current_value = obj->current_value;
   status->maximum_value = obj->max_value;
   status->waiting_processes = 0; // XXX

   return POK_ERRNO_OK;
}

#endif

#endif

