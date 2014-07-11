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
 * Created by julien on Mon Jan 19 10:51:40 2009 
 */

#ifdef POK_NEEDS_ERROR_HANDLING

#include <types.h>
#include <arch.h>
#include <core/thread.h>
#include <core/error.h>
#include <core/debug.h>
#include <core/partition.h>

#include <assert.h>
#include <libc.h>

pok_ret_t pok_error_thread_create (uint32_t stack_size, void* entry)
{
   pok_thread_id_t      tid;
   pok_thread_attr_t    attr;
   pok_ret_t            ret;

   if (POK_CURRENT_PARTITION.mode == POK_PARTITION_MODE_NORMAL) {
      // it's too late
      return POK_ERRNO_MODE;
   }

   if (POK_CURRENT_PARTITION.thread_error_created) {
      // already created
      return POK_ERRNO_EXISTS;
   }

   attr.priority = POK_THREAD_MAX_PRIORITY + 1; 
   attr.entry = entry;
   attr.period = -1;
   attr.deadline = DEADLINE_SOFT; // not that it matters, but we still have to specify something
   attr.time_capacity = -1;
   attr.stack_size = stack_size;
   
   ret = pok_partition_thread_create (&tid, &attr, POK_SCHED_CURRENT_PARTITION);
   
   if (ret == POK_ERRNO_OK) {
      POK_CURRENT_PARTITION.thread_error_created = TRUE;
      POK_CURRENT_PARTITION.thread_error = tid;
      POK_CURRENT_PARTITION.error_status.error_kind = POK_ERROR_KIND_INVALID;

      pok_threads[tid].state = POK_STATE_STOPPED;
   } 

   return (ret);
}

/*
 * Resets the context of the error handler
 * and marks it as runnable.
 *
 * Assumes it's already created and is not
 * handling some other error right now.
 *
 */
static void pok_error_enable(void)
{
    assert(POK_CURRENT_PARTITION.thread_error_created);

    pok_thread_t *thread = &pok_threads[POK_CURRENT_PARTITION.thread_error];

    assert(thread->state == POK_STATE_STOPPED);

    // TODO this code repeats in pok_thread_delayed_start
    // I guess I should refactor it somewhere

    thread->sp = thread->initial_sp;
    pok_space_context_restart(
        thread->sp,
        thread->partition,
        (uintptr_t) thread->entry,
        thread->init_stack_addr,
        0xdead,
        0xbeaf
    );

    // XXX hack hack hack - force context switch instead of returning from interrupt
    thread->force_restart = TRUE;

    thread->state  = POK_STATE_RUNNABLE;
}

void pok_error_declare (const uint8_t error)
{
    pok_error_declare2(error, POK_SCHED_CURRENT_THREAD);
}

void pok_error_declare2(uint8_t error, pok_thread_id_t thread_id)
{
    /**
     * Ok, the error handler process is created inside the partition
     * so we declare the error in a appropriate structure.
     */

    // TODO special case: error is raised by error handler

    if (POK_CURRENT_PARTITION.thread_error_created) {
        /*
         * We can't handle more than one error at a time.
         */
        assert(POK_CURRENT_PARTITION.error_status.error_kind == POK_ERROR_KIND_INVALID);

        POK_CURRENT_PARTITION.error_status.error_kind    = error;
        POK_CURRENT_PARTITION.error_status.failed_thread = thread_id;
        POK_CURRENT_PARTITION.error_status.msg_size      = 0;
        /*
         * FIXME: Add failed address and so on.
         */

        pok_error_enable();   
    } else {
        // XXX probably not quite correct
        pok_partition_error (POK_SCHED_CURRENT_PARTITION, error);
    }
}

void pok_error_ignore ()
{
   /* Do nothing at this time */
}


#ifndef POK_USE_GENERATED_KERNEL_ERROR_HANDLER
void pok_kernel_error (uint32_t error)
{
#ifdef POK_NEEDS_DEBUG
   printf ("[KERNEL] [WARNING] Error %d was raised by the kernel but no error recovery was set\n", error);
#else
   (void) error;
#endif /* POK_NEEDS_DEBUG */
   return;
}
#endif

#ifdef POK_NEEDS_PARTITIONS
#ifndef POK_USE_GENERATED_PARTITION_ERROR_HANDLER
void pok_partition_error (uint8_t partition, uint32_t error)
{
#ifdef POK_NEEDS_DEBUG
   printf ("[KERNEL] [WARNING] Error %d was raised by partition %d but no error recovery was set\n", error, partition);
#else
   (void) partition;
   (void) error;
#endif /* POK_NEEDS_DEBUG */
   return;
}
#endif /* POK_USE_GENERATED_PARTITION_ERROR_HANDLER */
#endif /* POK_NEEDS_PARTITIONS */


#ifndef POK_USE_GENERATED_KERNEL_ERROR_CALLBACK
void pok_error_kernel_callback ()
{
#ifdef POK_NEEDS_DEBUG
   printf ("[KERNEL] [WARNING] Kernel calls callback function but nothing was defined by the user\n");
   printf ("[KERNEL] [WARNING] You MUST define the pok_error_partition_callback function\n");
#endif /* POK_NEEDS_DEBUG */
   return;
}
#endif /* POK_USE_GENERATED_KERNEL_ERROR_CALLBACK */


#ifdef POK_NEEDS_PARTITIONS
#ifndef POK_USE_GENERATED_PARTITION_ERROR_CALLBACK
void pok_error_partition_callback (uint32_t partition)
{
#ifdef POK_NEEDS_DEBUG
   printf ("[KERNEL] [WARNING] Partition %d calls callback function but nothing was defined by the user\n", partition);
   printf ("[KERNEL] [WARNING] You MUST define the pok_error_partition_callback function\n");
#else
   (void) partition;
#endif
   return;
}
#endif /* POK_USE_GENERATED_PARTITION_ERROR_CALLBACK */


void pok_error_raise_application_error (char* msg, uint32_t msg_size)
{
    if (msg_size > POK_ERROR_MAX_MSG_SIZE) {
      msg_size = POK_ERROR_MAX_MSG_SIZE;
    }
   
    // TODO special case: error is raised by error handler

    if (POK_CURRENT_PARTITION.thread_error_created) {
        /*
         * We can't handle more than one error at a time.
         */
        assert(POK_CURRENT_PARTITION.error_status.error_kind == POK_ERROR_KIND_INVALID);
        pok_error_status_t* status;
        status                  = &pok_partitions[pok_current_partition].error_status;
        status->error_kind      = POK_ERROR_KIND_APPLICATION_ERROR;
        status->failed_thread   = POK_SCHED_CURRENT_THREAD;
        status->msg_size        = msg_size;

        memcpy(status->msg, msg, msg_size); 

        pok_error_enable();

        // since this function is called from user space
        // directly, switch to error handler here:
        pok_sched();
    } else {
        // XXX probably not quite correct
        pok_partition_error(POK_SCHED_CURRENT_PARTITION, POK_ERROR_KIND_APPLICATION_ERROR);
    }
}

pok_ret_t pok_error_get (pok_error_status_t* status)
{
   if (!pok_thread_is_error_handling(&POK_CURRENT_THREAD)) {
      return POK_ERRNO_THREAD;
   }

   if (POK_CURRENT_PARTITION.error_status.error_kind != POK_ERROR_KIND_INVALID)
   {

      status->error_kind       = POK_CURRENT_PARTITION.error_status.error_kind;
      status->failed_thread    = POK_CURRENT_PARTITION.error_status.failed_thread;
      status->failed_addr      = POK_CURRENT_PARTITION.error_status.failed_addr;
      status->msg_size         = POK_CURRENT_PARTITION.error_status.msg_size;
      memcpy (status->msg, POK_CURRENT_PARTITION.error_status.msg, POK_CURRENT_PARTITION.error_status.msg_size);

      // ARINC errors are supposed to be in FIFO queue
      // so we kinda emulate an 1-element error queue
      POK_CURRENT_PARTITION.error_status.error_kind = POK_ERROR_KIND_INVALID;

      return POK_ERRNO_OK;
   }
   else
   {
      return POK_ERRNO_UNAVAILABLE;
   }
}

pok_ret_t pok_error_is_handler(void)
{
    if (POK_CURRENT_PARTITION.thread_error_created && 
        POK_SCHED_CURRENT_THREAD == POK_CURRENT_PARTITION.thread_error)
    {
        return POK_ERRNO_OK;
    } else {
        return POK_ERRNO_UNAVAILABLE;
    }
}

#endif /* POK_NEEDS_PARTITIONS */

#endif

