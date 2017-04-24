/*
 * Institute for System Programming of the Russian Academy of Sciences
 * Copyright (C) 2016 ISPRAS
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, Version 3.
 *
 * This program is distributed in the hope # that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License version 3 for more details.
 */

#ifndef __LIBJET_MSECTION_H__
#define __LIBJET_MSECTION_H__

#include <uapi/msection.h>

/* Initialize mscection object. */
void msection_init(struct msection* section);

/* 
 * Enter into given msection.
 * 
 * On error (incorrect state, incorrect parameter) partition is terminated
 * (but can be restarted).
 */
void msection_enter(struct msection* section);

/*
 * Leave given msection.
 * 
 * On error (incorrect state, incorrect parameter) partition is terminated
 * (but can be restarted).
 */
void msection_leave(struct msection* section);

/*
 * Atomically leave msection and wait.
 * 
 * If timeout is non-negative, waiting will be automatically stopped since given interval.
 * 
 * The section should the only one which the process enters.
 * After being awoken (whatever reason), the process enters to given section again.
 * 
 * Returns:
 * 
 *     EOK: Process has been awoken by msection_notify().
 *     ETIMEDOUT: Process has been awoken by timeout.
 *     JET_CANCELLED: Someone calls STOP() for us or server IPPC handler has been cancelled.
 * 
 *     JET_INVALID_MODE: Waiting is not allowed in given mode.
 */
jet_ret_t msection_wait(struct msection* section, pok_time_t timeout);


/*
 * Awoke thread which waits on given section.
 * 
 * Current thread should have given section entered.
 * So, awoken thread may continue only after given thread leaves the section.
 * 
 * Returns:
 * 
 *     EOK: Thread had wait on the section and we have awoken it.
 *     JET_NOACTION: Thread is not waited.
 */
jet_ret_t msection_notify(struct msection* section, pok_thread_id_t thread_id);


/* Initialize waitqueue. */
void msection_wq_init(struct msection_wq* wq);

/* 
 * Add current thread into the msection wait queue.
 * 
 * If 'next' is not JET_THREAD_ID_NONE, add before given thread.
 * Otherwise add to the end of the queue.
 */
void msection_wq_add(struct msection_wq* wq, pok_thread_id_t next);

/* 
 * Add current thread into the msection wait queue.
 * 
 * If 'prev' is not JET_THREAD_ID_NONE, add after given thread.
 * Otherwise add to the beginning of the queue.
 */
void msection_wq_add_after(struct msection_wq* wq, pok_thread_id_t prev);


/* 
 * Remove thread from the msection wait queue, if it has been added before.
 */
void msection_wq_del(struct msection_wq* wq, pok_thread_id_t thread);


/* 
 * Awoke waiting threads in the waitqueue.
 * 
 * Every thread in the queue which hasn't waited at the function's call
 * is removed from the queue.
 * 
 * If 'is_all' is TRUE, notify all waiting threads. List of the
 * awoken threads may be iterated directly from user space.
 * If 'is_all' is FALSE, the first waiting thread only. This thread
 * will be pointed by wq->first after the call.
 * 
 * May be called only by the owner of the section.
 * 
 * Returns:
 * 
 *     EOK - at least on thread has been notified.
 *     JET_NOACTION - there is no waiting threads in the waitqueue.
 */
jet_ret_t msection_wq_notify(struct msection* section,
    struct msection_wq* wq,
    pok_bool_t is_all);

/*
 * Return number of waiting threads in the waitqueue.
 * 
 * Every thread in the queue which hasn't waited at the function's call
 * is removed from the queue.
 */
size_t msection_wq_size(struct msection* section,
   struct msection_wq* wq);

#endif /* __LIBJET_MSECTION_H__ */
