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
extern uint64_t pok_sched_next_deadline;
extern uint8_t pok_sched_current_slot;
#define POK_SCHED_CURRENT_PARTITION pok_current_partition

typedef enum
{
    POK_SLOT_SPARE = 0, // idle thread is always scheduled here
    POK_SLOT_PARTITION = 1,
#if defined(POK_NEEDS_NETWORKING)
    POK_SLOT_NETWORKING = 2,
#endif
} pok_sched_slot_type_t;

typedef struct
{
    pok_sched_slot_type_t type;

    uint64_t duration;
    uint64_t offset;

    union {
        struct {
            pok_partition_id_t id;
            pok_bool_t periodic_processing_start;
        } partition;
    };
} pok_sched_slot_t;

// as usual, defined in deployment.c
extern const pok_sched_slot_t pok_module_sched[POK_CONFIG_SCHEDULING_NBSLOTS];
#endif


extern pok_thread_id_t current_thread;
#define POK_SCHED_CURRENT_THREAD current_thread

// must match libpok/include/core/thread.h
typedef enum
{
  // comments describe to what states of ARINC653 these correspond to
  POK_STATE_STOPPED = 0, // DORMANT (must be started first)
  POK_STATE_RUNNABLE = 1, // READY 
  POK_STATE_WAITING = 2, // WAITING (sleeping for specified time OR waiting for a lock with timeout)
  POK_STATE_LOCK = 3, // WAITING (waiting for a lock without timeout)
  POK_STATE_WAIT_NEXT_ACTIVATION = 4, // WAITING (for next activation aka "release point")
  POK_STATE_DELAYED_START = 5 // WAITING (waitng for partition mode NORMAL)
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

pok_ret_t pok_sched_end_period(void);
pok_ret_t pok_sched_replenish(int64_t budget);

uint32_t pok_sched_get_current(pok_thread_id_t *thread_id);

#endif /* POK_NEEDS.... */

#endif /* !__POK_SCHED_H__ */

