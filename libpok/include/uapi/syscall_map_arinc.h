/*
 * GENERATED! DO NOT MODIFY!
 *
 * Instead of modifying this file, modify the one it generated from (kernel/include/uapi/syscall_map_arinc.h.in).
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


#include <uapi/types.h>
#include <uapi/thread_types.h>
#include <uapi/partition_types.h>
#include <uapi/partition_arinc_types.h>
#include <uapi/port_types.h>
#include <uapi/error_arinc_types.h>
#include <uapi/memblock_types.h>
#include <uapi/msection.h>
#include <uapi/ippc_types.h>

pok_ret_t pok_thread_create(const char* name,
    void* entry,
    const pok_thread_attr_t* attr,
    pok_thread_id_t* thread_id);

pok_ret_t pok_thread_sleep(const pok_time_t* time);

pok_ret_t pok_sched_end_period(void);

pok_ret_t pok_thread_suspend(const pok_time_t* time);

pok_ret_t pok_thread_get_status(pok_thread_id_t thread_id,
    char* name,
    void** entry,
    pok_thread_status_t* status);

pok_ret_t pok_thread_delayed_start(pok_thread_id_t thread_id,
    const pok_time_t* time);

pok_ret_t pok_thread_set_priority(pok_thread_id_t thread_id,
    uint32_t priority);

pok_ret_t pok_thread_resume(pok_thread_id_t thread_id);

pok_ret_t pok_thread_suspend_target(pok_thread_id_t thread_id);

pok_ret_t pok_thread_yield(void);

pok_ret_t pok_sched_replenish(const pok_time_t* budget);

pok_ret_t pok_thread_stop_target(pok_thread_id_t thread_id);

pok_ret_t pok_thread_stop(void);

pok_ret_t pok_thread_find(const char* name,
    pok_thread_id_t* id);


pok_ret_t jet_resched(void);

pok_ret_t jet_msection_enter_helper(struct msection* section);

pok_ret_t jet_msection_wait(struct msection* section,
    const pok_time_t* timeout);

pok_ret_t jet_msection_notify(struct msection* section,
    pok_thread_id_t thread_id);

pok_ret_t jet_msection_wq_notify(struct msection* section,
    struct msection_wq* wq,
    pok_bool_t is_all);

pok_ret_t jet_msection_wq_size(struct msection* section,
    struct msection_wq* wq,
    size_t* size);


pok_ret_t pok_partition_set_mode_current(pok_partition_mode_t mode);

pok_ret_t pok_current_partition_get_status(pok_partition_status_t* status);

pok_ret_t pok_current_partition_inc_lock_level(int32_t* lock_level);

pok_ret_t pok_current_partition_dec_lock_level(int32_t* lock_level);


pok_ret_t pok_error_thread_create(uint32_t stack_size,
    void* entry);

pok_ret_t pok_error_raise_application_error(const char* msg,
    size_t msg_size);

pok_ret_t pok_error_get(pok_error_status_t* status,
    void* msg);

pok_ret_t pok_error_raise_os_error(const char* msg,
    size_t msg_size);


   /* Middleware syscalls */
pok_ret_t pok_port_sampling_create(const char* name,
    pok_port_size_t size,
    pok_port_direction_t direction,
    const pok_time_t* refresh,
    pok_port_id_t* id);

pok_ret_t pok_port_sampling_write(pok_port_id_t id,
    const void* data,
    pok_port_size_t len);

pok_ret_t pok_port_sampling_read(pok_port_id_t id,
    void* data,
    pok_port_size_t* len,
    pok_bool_t* valid);

pok_ret_t pok_port_sampling_id(const char* name,
    pok_port_id_t* id);

pok_ret_t pok_port_sampling_status(pok_port_id_t id,
    pok_port_sampling_status_t* status);

pok_ret_t pok_port_sampling_check(pok_port_id_t id);

pok_ret_t pok_port_queuing_create_packed(const char* name,
    const pok_port_queuing_create_arg_t* arg,
    pok_port_id_t* id);

pok_ret_t pok_port_queuing_send(pok_port_id_t id,
    const void* data,
    pok_port_size_t len,
    const pok_time_t* timeout);

pok_ret_t pok_port_queuing_receive(pok_port_id_t id,
    const pok_time_t* timeout,
    void* data,
    pok_port_size_t* len);

pok_ret_t pok_port_queuing_id(const char* name,
    pok_port_id_t* id);

pok_ret_t pok_port_queuing_status(pok_port_id_t id,
    pok_port_queuing_status_t* status);

pok_ret_t pok_port_queuing_clear(pok_port_id_t id);



pok_ret_t jet_memory_block_get_status(const char* name,
    jet_memory_block_status_t* status);

pok_ret_t jet_ippc_partition_arinc_init_portal(const char* portal_name,
    int* portal_id);

pok_ret_t jet_ippc_partition_arinc_call(int portal_id,
    const struct jet_ippc_client_access_window* access_windows,
    int access_windows_n);

pok_ret_t jet_ippc_partition_arinc_get_portal_type_info(const char* portal_name,
    int* portal_type_id,
    int* n_clients);

pok_ret_t jet_ippc_partition_arinc_get_portal_info(int server_portal_id,
    int* n_connections);

pok_ret_t jet_ippc_partition_arinc_create_connections(int server_portal_id,
    void* entry,
    size_t stack_size,
    int n_connections,
    pok_thread_id_t* thread_id);

pok_ret_t jet_ippc_partition_arinc_return(void);

pok_ret_t jet_ippc_partition_arinc_copy_to_client(void* dst,
    const void* src,
    size_t n);

pok_ret_t jet_ippc_partition_arinc_copy_from_client(void* dst,
    const void* src,
    size_t n);
