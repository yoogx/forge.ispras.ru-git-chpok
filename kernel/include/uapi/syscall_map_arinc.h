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

pok_ret_t pok_thread_create(const char* __user name,
    void* __user entry,
    const pok_thread_attr_t* __user attr,
    pok_thread_id_t* __user thread_id);
static inline pok_ret_t pok_syscall_wrapper_POK_SYSCALL_THREAD_CREATE(const pok_syscall_args_t* args)
{
    return pok_thread_create(
        (const char* __user)args->arg1,
        (void* __user)args->arg2,
        (const pok_thread_attr_t* __user)args->arg3,
        (pok_thread_id_t* __user)args->arg4);
}

#ifdef POK_NEEDS_THREAD_SLEEP
pok_ret_t pok_thread_sleep(const pok_time_t* __user time);
static inline pok_ret_t pok_syscall_wrapper_POK_SYSCALL_THREAD_SLEEP(const pok_syscall_args_t* args)
{
    return pok_thread_sleep(
        (const pok_time_t* __user)args->arg1);
}
#endif

#ifdef POK_NEEDS_THREAD_SLEEP_UNTIL
pok_ret_t pok_thread_sleep_until(const pok_time_t* __user time);
static inline pok_ret_t pok_syscall_wrapper_POK_SYSCALL_THREAD_SLEEP_UNTIL(const pok_syscall_args_t* args)
{
    return pok_thread_sleep_until(
        (const pok_time_t* __user)args->arg1);
}
#endif
pok_ret_t pok_sched_end_period(void);
static inline pok_ret_t pok_syscall_wrapper_POK_SYSCALL_THREAD_PERIOD(const pok_syscall_args_t* args)
{
    return pok_sched_end_period();
}

#if defined (POK_NEEDS_THREAD_SUSPEND) || defined (POK_NEEDS_ERROR_HANDLING)
pok_ret_t pok_thread_suspend(const pok_time_t* __user time);
static inline pok_ret_t pok_syscall_wrapper_POK_SYSCALL_THREAD_SUSPEND(const pok_syscall_args_t* args)
{
    return pok_thread_suspend(
        (const pok_time_t* __user)args->arg1);
}
#endif

pok_ret_t pok_thread_get_status(pok_thread_id_t thread_id,
    char* __user name,
    void** __user entry,
    pok_thread_status_t* __user status);
static inline pok_ret_t pok_syscall_wrapper_POK_SYSCALL_THREAD_STATUS(const pok_syscall_args_t* args)
{
    return pok_thread_get_status(
        (pok_thread_id_t)args->arg1,
        (char* __user)args->arg2,
        (void** __user)args->arg3,
        (pok_thread_status_t* __user)args->arg4);
}

pok_ret_t pok_thread_delayed_start(pok_thread_id_t thread_id,
    const pok_time_t* __user time);
static inline pok_ret_t pok_syscall_wrapper_POK_SYSCALL_THREAD_DELAYED_START(const pok_syscall_args_t* args)
{
    return pok_thread_delayed_start(
        (pok_thread_id_t)args->arg1,
        (const pok_time_t* __user)args->arg2);
}

pok_ret_t pok_thread_set_priority(pok_thread_id_t thread_id,
    uint32_t priority);
static inline pok_ret_t pok_syscall_wrapper_POK_SYSCALL_THREAD_SET_PRIORITY(const pok_syscall_args_t* args)
{
    return pok_thread_set_priority(
        (pok_thread_id_t)args->arg1,
        (uint32_t)args->arg2);
}

pok_ret_t pok_thread_resume(pok_thread_id_t thread_id);
static inline pok_ret_t pok_syscall_wrapper_POK_SYSCALL_THREAD_RESUME(const pok_syscall_args_t* args)
{
    return pok_thread_resume(
        (pok_thread_id_t)args->arg1);
}

pok_ret_t pok_thread_suspend_target(pok_thread_id_t thread_id);
static inline pok_ret_t pok_syscall_wrapper_POK_SYSCALL_THREAD_SUSPEND_TARGET(const pok_syscall_args_t* args)
{
    return pok_thread_suspend_target(
        (pok_thread_id_t)args->arg1);
}

pok_ret_t pok_thread_yield(void);
static inline pok_ret_t pok_syscall_wrapper_POK_SYSCALL_THREAD_YIELD(const pok_syscall_args_t* args)
{
    return pok_thread_yield();
}

pok_ret_t pok_sched_replenish(const pok_time_t* __user budget);
static inline pok_ret_t pok_syscall_wrapper_POK_SYSCALL_THREAD_REPLENISH(const pok_syscall_args_t* args)
{
    return pok_sched_replenish(
        (const pok_time_t* __user)args->arg1);
}

pok_ret_t pok_thread_stop_target(pok_thread_id_t thread_id);
static inline pok_ret_t pok_syscall_wrapper_POK_SYSCALL_THREAD_STOP(const pok_syscall_args_t* args)
{
    return pok_thread_stop_target(
        (pok_thread_id_t)args->arg1);
}

pok_ret_t pok_thread_stop(void);
static inline pok_ret_t pok_syscall_wrapper_POK_SYSCALL_THREAD_STOPSELF(const pok_syscall_args_t* args)
{
    return pok_thread_stop();
}

pok_ret_t pok_thread_find(const char* __user name,
    pok_thread_id_t* __user id);
static inline pok_ret_t pok_syscall_wrapper_POK_SYSCALL_THREAD_FIND(const pok_syscall_args_t* args)
{
    return pok_thread_find(
        (const char* __user)args->arg1,
        (pok_thread_id_t* __user)args->arg2);
}


pok_ret_t jet_resched(void);
static inline pok_ret_t pok_syscall_wrapper_POK_SYSCALL_RESCHED(const pok_syscall_args_t* args)
{
    return jet_resched();
}

pok_ret_t jet_msection_enter_helper(struct msection* __user section);
static inline pok_ret_t pok_syscall_wrapper_POK_SYSCALL_MSECTION_ENTER_HELPER(const pok_syscall_args_t* args)
{
    return jet_msection_enter_helper(
        (struct msection* __user)args->arg1);
}

pok_ret_t jet_msection_wait(struct msection* __user section,
    const pok_time_t* __user timeout);
static inline pok_ret_t pok_syscall_wrapper_POK_SYSCALL_MSECTION_WAIT(const pok_syscall_args_t* args)
{
    return jet_msection_wait(
        (struct msection* __user)args->arg1,
        (const pok_time_t* __user)args->arg2);
}

pok_ret_t jet_msection_notify(struct msection* __user section,
    pok_thread_id_t thread_id);
static inline pok_ret_t pok_syscall_wrapper_POK_SYSCALL_MSECTION_NOTIFY(const pok_syscall_args_t* args)
{
    return jet_msection_notify(
        (struct msection* __user)args->arg1,
        (pok_thread_id_t)args->arg2);
}

pok_ret_t jet_msection_wq_notify(struct msection* __user section,
    struct msection_wq* __user wq,
    pok_bool_t is_all);
static inline pok_ret_t pok_syscall_wrapper_POK_SYSCALL_MSECTION_WQ_NOTIFY(const pok_syscall_args_t* args)
{
    return jet_msection_wq_notify(
        (struct msection* __user)args->arg1,
        (struct msection_wq* __user)args->arg2,
        (pok_bool_t)args->arg3);
}

pok_ret_t jet_msection_wq_size(struct msection* __user section,
    struct msection_wq* __user wq,
    size_t* __user size);
static inline pok_ret_t pok_syscall_wrapper_POK_SYSCALL_MSECTION_WQ_SIZE(const pok_syscall_args_t* args)
{
    return jet_msection_wq_size(
        (struct msection* __user)args->arg1,
        (struct msection_wq* __user)args->arg2,
        (size_t* __user)args->arg3);
}


#ifdef POK_NEEDS_PARTITIONS
pok_ret_t pok_partition_set_mode_current(pok_partition_mode_t mode);
static inline pok_ret_t pok_syscall_wrapper_POK_SYSCALL_PARTITION_SET_MODE(const pok_syscall_args_t* args)
{
    return pok_partition_set_mode_current(
        (pok_partition_mode_t)args->arg1);
}

pok_ret_t pok_current_partition_get_status(pok_partition_status_t* __user status);
static inline pok_ret_t pok_syscall_wrapper_POK_SYSCALL_PARTITION_GET_STATUS(const pok_syscall_args_t* args)
{
    return pok_current_partition_get_status(
        (pok_partition_status_t* __user)args->arg1);
}

pok_ret_t pok_current_partition_inc_lock_level(int32_t* __user lock_level);
static inline pok_ret_t pok_syscall_wrapper_POK_SYSCALL_PARTITION_INC_LOCK_LEVEL(const pok_syscall_args_t* args)
{
    return pok_current_partition_inc_lock_level(
        (int32_t* __user)args->arg1);
}

pok_ret_t pok_current_partition_dec_lock_level(int32_t* __user lock_level);
static inline pok_ret_t pok_syscall_wrapper_POK_SYSCALL_PARTITION_DEC_LOCK_LEVEL(const pok_syscall_args_t* args)
{
    return pok_current_partition_dec_lock_level(
        (int32_t* __user)args->arg1);
}
#endif


#ifdef POK_NEEDS_ERROR_HANDLING
pok_ret_t pok_error_thread_create(uint32_t stack_size,
    void* __user entry);
static inline pok_ret_t pok_syscall_wrapper_POK_SYSCALL_ERROR_HANDLER_CREATE(const pok_syscall_args_t* args)
{
    return pok_error_thread_create(
        (uint32_t)args->arg1,
        (void* __user)args->arg2);
}

pok_ret_t pok_error_raise_application_error(const char* __user msg,
    size_t msg_size);
static inline pok_ret_t pok_syscall_wrapper_POK_SYSCALL_ERROR_RAISE_APPLICATION_ERROR(const pok_syscall_args_t* args)
{
    return pok_error_raise_application_error(
        (const char* __user)args->arg1,
        (size_t)args->arg2);
}

pok_ret_t pok_error_get(pok_error_status_t* __user status,
    void* __user msg);
static inline pok_ret_t pok_syscall_wrapper_POK_SYSCALL_ERROR_GET(const pok_syscall_args_t* args)
{
    return pok_error_get(
        (pok_error_status_t* __user)args->arg1,
        (void* __user)args->arg2);
}
#endif

pok_ret_t pok_error_raise_os_error(const char* __user msg,
    size_t msg_size);
static inline pok_ret_t pok_syscall_wrapper_POK_SYSCALL_ERROR_RAISE_OS_ERROR(const pok_syscall_args_t* args)
{
    return pok_error_raise_os_error(
        (const char* __user)args->arg1,
        (size_t)args->arg2);
}


   /* Middleware syscalls */
#ifdef POK_NEEDS_PORTS_SAMPLING
pok_ret_t pok_port_sampling_create(const char* __user name,
    pok_port_size_t size,
    pok_port_direction_t direction,
    const pok_time_t* __user refresh,
    pok_port_id_t* __user id);
static inline pok_ret_t pok_syscall_wrapper_POK_SYSCALL_MIDDLEWARE_SAMPLING_CREATE(const pok_syscall_args_t* args)
{
    return pok_port_sampling_create(
        (const char* __user)args->arg1,
        (pok_port_size_t)args->arg2,
        (pok_port_direction_t)args->arg3,
        (const pok_time_t* __user)args->arg4,
        (pok_port_id_t* __user)args->arg5);
}

pok_ret_t pok_port_sampling_write(pok_port_id_t id,
    const void* __user data,
    pok_port_size_t len);
static inline pok_ret_t pok_syscall_wrapper_POK_SYSCALL_MIDDLEWARE_SAMPLING_WRITE(const pok_syscall_args_t* args)
{
    return pok_port_sampling_write(
        (pok_port_id_t)args->arg1,
        (const void* __user)args->arg2,
        (pok_port_size_t)args->arg3);
}

pok_ret_t pok_port_sampling_read(pok_port_id_t id,
    void* __user data,
    pok_port_size_t* __user len,
    pok_bool_t* __user valid);
static inline pok_ret_t pok_syscall_wrapper_POK_SYSCALL_MIDDLEWARE_SAMPLING_READ(const pok_syscall_args_t* args)
{
    return pok_port_sampling_read(
        (pok_port_id_t)args->arg1,
        (void* __user)args->arg2,
        (pok_port_size_t* __user)args->arg3,
        (pok_bool_t* __user)args->arg4);
}

pok_ret_t pok_port_sampling_id(const char* __user name,
    pok_port_id_t* __user id);
static inline pok_ret_t pok_syscall_wrapper_POK_SYSCALL_MIDDLEWARE_SAMPLING_ID(const pok_syscall_args_t* args)
{
    return pok_port_sampling_id(
        (const char* __user)args->arg1,
        (pok_port_id_t* __user)args->arg2);
}

pok_ret_t pok_port_sampling_status(pok_port_id_t id,
    pok_port_sampling_status_t* __user status);
static inline pok_ret_t pok_syscall_wrapper_POK_SYSCALL_MIDDLEWARE_SAMPLING_STATUS(const pok_syscall_args_t* args)
{
    return pok_port_sampling_status(
        (pok_port_id_t)args->arg1,
        (pok_port_sampling_status_t* __user)args->arg2);
}

pok_ret_t pok_port_sampling_check(pok_port_id_t id);
static inline pok_ret_t pok_syscall_wrapper_POK_SYSCALL_MIDDLEWARE_SAMPLING_CHECK(const pok_syscall_args_t* args)
{
    return pok_port_sampling_check(
        (pok_port_id_t)args->arg1);
}
#endif /* POK_NEEDS_PORTS_SAMPLING */

#ifdef POK_NEEDS_PORTS_QUEUEING
pok_ret_t pok_port_queuing_create_packed(const char* __user name,
    const pok_port_queuing_create_arg_t* __user arg,
    pok_port_id_t* __user id);
static inline pok_ret_t pok_syscall_wrapper_POK_SYSCALL_MIDDLEWARE_QUEUEING_CREATE(const pok_syscall_args_t* args)
{
    return pok_port_queuing_create_packed(
        (const char* __user)args->arg1,
        (const pok_port_queuing_create_arg_t* __user)args->arg2,
        (pok_port_id_t* __user)args->arg3);
}

pok_ret_t pok_port_queuing_send(pok_port_id_t id,
    const void* __user data,
    pok_port_size_t len,
    const pok_time_t* __user timeout);
static inline pok_ret_t pok_syscall_wrapper_POK_SYSCALL_MIDDLEWARE_QUEUEING_SEND(const pok_syscall_args_t* args)
{
    return pok_port_queuing_send(
        (pok_port_id_t)args->arg1,
        (const void* __user)args->arg2,
        (pok_port_size_t)args->arg3,
        (const pok_time_t* __user)args->arg4);
}

pok_ret_t pok_port_queuing_receive(pok_port_id_t id,
    const pok_time_t* __user timeout,
    void* __user data,
    pok_port_size_t* __user len);
static inline pok_ret_t pok_syscall_wrapper_POK_SYSCALL_MIDDLEWARE_QUEUEING_RECEIVE(const pok_syscall_args_t* args)
{
    return pok_port_queuing_receive(
        (pok_port_id_t)args->arg1,
        (const pok_time_t* __user)args->arg2,
        (void* __user)args->arg3,
        (pok_port_size_t* __user)args->arg4);
}

pok_ret_t pok_port_queuing_id(const char* __user name,
    pok_port_id_t* __user id);
static inline pok_ret_t pok_syscall_wrapper_POK_SYSCALL_MIDDLEWARE_QUEUEING_ID(const pok_syscall_args_t* args)
{
    return pok_port_queuing_id(
        (const char* __user)args->arg1,
        (pok_port_id_t* __user)args->arg2);
}

pok_ret_t pok_port_queuing_status(pok_port_id_t id,
    pok_port_queuing_status_t* __user status);
static inline pok_ret_t pok_syscall_wrapper_POK_SYSCALL_MIDDLEWARE_QUEUEING_STATUS(const pok_syscall_args_t* args)
{
    return pok_port_queuing_status(
        (pok_port_id_t)args->arg1,
        (pok_port_queuing_status_t* __user)args->arg2);
}

pok_ret_t pok_port_queuing_clear(pok_port_id_t id);
static inline pok_ret_t pok_syscall_wrapper_POK_SYSCALL_MIDDLEWARE_QUEUEING_CLEAR(const pok_syscall_args_t* args)
{
    return pok_port_queuing_clear(
        (pok_port_id_t)args->arg1);
}

#endif /* POK_NEEDS_PORTS_QUEUEING */


pok_ret_t jet_memory_block_get_status(const char* __user name,
    jet_memory_block_status_t* __user status);
static inline pok_ret_t pok_syscall_wrapper_POK_SYSCALL_MEMORY_BLOCK_GET_STATUS(const pok_syscall_args_t* args)
{
    return jet_memory_block_get_status(
        (const char* __user)args->arg1,
        (jet_memory_block_status_t* __user)args->arg2);
}
