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

/*
 * This should be defined in kernel's deployment.c
 */
extern const pok_error_hm_partition_t * const pok_partition_hm_tables[];

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

static void take_fixed_action(pok_error_action_t action)
{
    // TODO in case of restart, fill in restart reason (to be inspected later)

    switch (action) {
        case POK_ERROR_ACTION_IGNORE:
            // TODO not all kinds of errors can be ignored just like that
            return;
        case POK_ERROR_ACTION_IDLE:
            pok_partition_set_mode_current(POK_PARTITION_MODE_IDLE);
            assert(FALSE && "this's supposed to be unreachable");
            break;
        case POK_ERROR_ACTION_COLD_START:
            pok_partition_set_mode_current(POK_PARTITION_MODE_INIT_COLD);
            assert(FALSE && "this's supposed to be unreachable");
            break;
        case POK_ERROR_ACTION_WARM_START:
            pok_partition_set_mode_current(POK_PARTITION_MODE_INIT_WARM);
            assert(FALSE && "this's supposed to be unreachable");
            break;
        default:
            assert(FALSE && "invalid HM action");
    }
}

pok_ret_t pok_error_raise_thread(
        pok_error_kind_t error, 
        pok_thread_id_t thread_id, 
        const char *message,
        size_t msg_size)
{
    /*
     * This might be either process or partition level error,
     * which depends on HM table.
     *
     * So, we need to consult the table first.
     */
    const pok_error_hm_partition_t *entry;
    for (entry = pok_partition_hm_tables[POK_SCHED_CURRENT_PARTITION]; entry->kind != POK_ERROR_KIND_INVALID; entry++) {
        if (entry->kind == error) {
            break;
        }
    }
    if (entry->kind == POK_ERROR_KIND_INVALID) {
        // couldn't find error in the table, oops.
        // XXX what should we do? restart partition, or what?
        assert(FALSE && "missing error code in partition HM table");
    }

    if (entry->level == POK_ERROR_LEVEL_PARTITION || 
        !POK_CURRENT_PARTITION.thread_error_created || 
        POK_CURRENT_PARTITION.thread_error == thread_id)
    {
        // take fixed action
        take_fixed_action(entry->action);
        // TODO if action is ignore, sometimes we need to do something else (to prevent raising it again)
    } else {
        // otherwise, pass it to the error handler process

        // TODO add real error queue
        pok_error_status_t* status = &POK_CURRENT_PARTITION.error_status;
        assert(status->error_kind == POK_ERROR_KIND_INVALID);

        status->error_kind = entry->target_error_code;
        status->failed_thread = thread_id;
        if (msg_size > 0) {
            memcpy(status->msg, message, msg_size);
        }
        status->msg_size = msg_size;
        status->failed_addr = (uintptr_t) 0; // TODO somehow find interrupt frame and extract EIP from there
        
        // reset it's stack and other stuff
        pok_error_enable();
    }

    return POK_ERRNO_OK;
}


void pok_error_raise_partition (pok_partition_id_t partition, pok_error_kind_t error)
{
    (void) partition;
    (void) error;

    // consult partition error table
    const pok_error_hm_partition_t *entry;
    for (entry = pok_partition_hm_tables[POK_SCHED_CURRENT_PARTITION]; entry->kind != POK_ERROR_KIND_INVALID; entry++) {
        if (entry->kind == error) {
            break;
        }
    }
    if (entry->kind == POK_ERROR_KIND_INVALID) {
        // couldn't find error in the table, oops.
        // XXX what should we do? restart partition, or what?
        assert(FALSE && "missing error code in partition HM table");
    }

    assert(entry->level == POK_ERROR_LEVEL_PARTITION);

    take_fixed_action(entry->action);
}

void pok_error_raise_kernel(pok_error_kind_t error)
{
    (void) error;

    // TODO hmmm

    pok_fatal("KERNEL ERROR!!1");
    
    return;
}

pok_ret_t pok_error_raise_application_error (const char* msg, size_t msg_size)
{
    if (msg_size > POK_ERROR_MAX_MSG_SIZE) {
        return POK_ERRNO_EINVAL;
    }

    pok_ret_t ret = pok_error_raise_thread(
            POK_ERROR_KIND_APPLICATION_ERROR, 
            POK_SCHED_CURRENT_THREAD,
            msg,
            msg_size
    );

    if (ret == POK_ERRNO_OK) {
        // call scheduler, which will switch to error handler
        // FIXME if error is ignored by partition HM table, we shouldn't actually do that
        pok_sched();
    }

    return ret;
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

#endif

