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
 * Created by julien on Thu Jan 15 23:34:13 2009 
 */

#ifndef __POK_SCHED_H__
#define __POK_SCHED_H__

#if defined (POK_NEEDS_SCHED) || defined (POK_NEEDS_THREADS)

#include <types.h>
#include <errno.h>
#include <core/schedvalues.h>

#ifdef POK_NEEDS_PARTITIONS
extern pok_partition_id_t pok_current_partition;
#define POK_SCHED_CURRENT_PARTITION pok_current_partition
#endif


extern pok_thread_id_t current_thread;
#define POK_SCHED_CURRENT_THREAD current_thread

// must match to libpok/include/core/thread.h
typedef enum
{
  POK_STATE_STOPPED = 0,
  POK_STATE_RUNNABLE = 1,
  POK_STATE_WAITING = 2,
  POK_STATE_LOCK = 3,
  POK_STATE_WAIT_NEXT_ACTIVATION = 4,
  POK_STATE_DELAYED_START = 5
} pok_state_t;

typedef struct {
    void (*initialize)(void);
    pok_thread_id_t (*elect_thread)(void);
    void (*enqueue_thread)(pok_thread_id_t thread_id);
} pok_scheduler_ops;

void pok_sched_init(void); /* Initialize scheduling stuff */

void pok_sched(void);      /* Main scheduling function, this function
                             * schedules everything
                             */

/* Context switch functions */
void pok_sched_context_switch(pok_thread_id_t);
void pok_partition_switch (void);

/*
 * Functions to lock threads
 */
void pok_sched_lock_thread(pok_thread_id_t);
void pok_sched_unlock_thread(pok_thread_id_t);
void pok_sched_lock_current_thread(void);
void pok_sched_lock_current_thread_timed(uint64_t time);

pok_ret_t pok_sched_end_period ();

#ifdef POK_NEEDS_PARTITIONS
void pok_sched_activate_error_thread(void);
#endif

uint32_t pok_sched_get_current(pok_thread_id_t *thread_id);

#endif /* POK_NEEDS.... */

#endif /* !__POK_SCHED_H__ */

