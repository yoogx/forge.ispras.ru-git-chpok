/*
 * COPIED! DO NOT MODIFY!
 *
 * Instead of modifying this file, modify original one (kernel/include/uapi/kernel_shared_data.h).
 */
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

#ifndef __JET_UAPI_KERNEL_SHARED_DATA_H__
#define __JET_UAPI_KERNEL_SHARED_DATA_H__

#include <types.h>
#include <uapi/partition_arinc_types.h>

typedef uint16_t jet_thread_id_t;
/*
 * Special value of type jet_thread_id_t, which cannot be id of actual thread.
 */
#define JET_THREAD_ID_NONE (jet_thread_id_t)(-1)

/* Data about the thread, shared between kernel and user spaces. */
struct jet_thread_shared_data
{
    // TODO: Which data should be there?
    int unused;
};

/* Instance of this struct will be shared between kernel and user spaces. */
struct jet_kernel_shared_data
{
    /* 
     * Set by the kernel, read by the user.
     * 
     * Race while reading is impossible, as changing mode invalidates
     * all running threads.
     */
    pok_partition_mode_t partition_mode;
    
    /* 
     * Set by the kernel, read by the user.
     * 
     * NOTE: Any thread has its id, even main and error.
     * For ARINC requests, id is filtered out.
     * 
     * Each thread sees its own id, race is impossible.
     * 
     * DEV: When porting to multicore, this should be stored in the register.
     */
    jet_thread_id_t current_thread_id;
    
    /*
     * Actually, accessed only by the user.
     * 
     * Set once before partition's entry is executed.
     * Read when need to check process_id from ARINC.
     */
    jet_thread_id_t main_thread_id;

    /*
     * Set by the kernel when error thread is created.
     * Until that, it is JET_THREAD_ID_NONE.
     * 
     * Read by the user when it needs to determine whether error thread
     * currently is executed or when need to check process_id from ARINC.
     * 
     * Race is impossible, as the field is changed in INIT mode.
     */
    jet_thread_id_t error_thread_id;
    
    /* 
     * Maximum number of threads.
     * 
     * TODO: Does partition really need that info?
     */
    jet_thread_id_t max_n_threads;
    
    /* Open-bounds array of thread shared data. */
    struct jet_thread_shared_data tshd[];
};

#endif /* __JET_UAPI_KERNEL_SHARED_DATA_H__ */
