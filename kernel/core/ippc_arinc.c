/*
 * Institute for System Programming of the Russian Academy of Sciences
 * Copyright (C) 2017 ISPRAS
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

#include <core/ippc_arinc.h>
#include <core/sched_arinc.h>
#include <core/uaccess.h>
#include <assert.h>
#include "thread_internal.h"

static struct jet_partition_arinc_ippc_portal* find_arinc_portal(const char* kernel_name)
{
    pok_partition_arinc_t* part = current_partition_arinc;

    for(int i = 0; i < part->portals_n; i++) {
        struct jet_partition_arinc_ippc_portal* portal = &part->portals[i];

        if(strcmp(kernel_name, portal->portal_name) == 0)
            return portal;
    }

    return NULL;
}

static struct jet_partition_arinc_ippc_portal* get_arinc_portal(int portal_id)
{
    pok_partition_arinc_t* part = current_partition_arinc;
    if(portal_id < 0 || portal_id >= part->portals_n) return NULL;

    struct jet_partition_arinc_ippc_portal* arinc_portal = &part->portals[portal_id];
    if(!arinc_portal->is_initialized) return NULL;

    return arinc_portal;
}

pok_ret_t jet_ippc_partition_arinc_init_portal(const char* __user portal_name,
    int* __user portal_id)
{
    pok_partition_arinc_t* part = current_partition_arinc;

    char portal_name_k[MAX_NAME_LENGTH];

    if (kstrncpy(portal_name_k, portal_name, MAX_NAME_LENGTH) == NULL)
        return POK_ERRNO_EFAULT;

    int* __kuser portal_id_k = jet_user_to_kernel_typed(portal_id);

    if(portal_id_k == NULL)
        return POK_ERRNO_EFAULT;

    if (part->mode == POK_PARTITION_MODE_NORMAL)
        return POK_ERRNO_PARTITION_MODE;

    struct jet_partition_arinc_ippc_portal* arinc_portal = find_arinc_portal(portal_name_k);
    if(arinc_portal == NULL) return POK_ERRNO_NOTFOUND;

    if(arinc_portal->is_initialized) return POK_ERRNO_READY;

    struct jet_ippc_connection* init_connection
        = jet_ippc_portal_open_init_connection(arinc_portal->portal, 0);


    while(jet_ippc_connection_get_state(init_connection) != JET_IPPC_CONNECTION_STATE_TERMINATED) {
        jet_ippc_connection_execute(init_connection, part->base_part.ippc_handled_actual);
    }

    pok_ret_t ret = POK_ERRNO_UNAVAILABLE;

    switch(init_connection->terminate_status) {
        case JET_IPPC_CONNECTION_TERMINATE_STATUS_OK:
            if(jet_ippc_portal_ready_state_from_init_connection(init_connection))
                ret = POK_ERRNO_OK;
            else
                ret = POK_ERRNO_UNAVAILABLE;
            break;
        case JET_IPPC_CONNECTION_TERMINATE_STATUS_SERVICE_RESETED:
            ret = POK_ERRNO_UNAVAILABLE;
            break;
        default:
            unreachable();
    }

    jet_ippc_connection_close(init_connection);

    if(ret == POK_ERRNO_OK) {
        arinc_portal->is_initialized = TRUE;
        *portal_id_k = arinc_portal - part->portals;
    }

    return ret;
}

pok_ret_t jet_ippc_partition_arinc_call(int portal_id)
{
    pok_partition_arinc_t* part = current_partition_arinc;

    struct jet_partition_arinc_ippc_portal* arinc_portal = get_arinc_portal(portal_id);
    if(arinc_portal == NULL) return POK_ERRNO_EINVAL;

    pok_thread_t* t = part->thread_current;

    struct jet_thread_shared_data* tshd_current = part->kshd->tshd
        + (t - part->threads);

    int input_params_n = tshd_current->ippc_input_params_n;

    if(input_params_n < 0 || input_params_n > IPPC_MAX_INPUT_PARAMS_N)
        return POK_ERRNO_EINVAL;

    pok_preemption_local_disable();

    t->ippc_connection = jet_ippc_portal_open_connection(
        arinc_portal->portal,
        t - part->threads);

    pok_preemption_local_enable();

    if(t->ippc_connection == NULL) return POK_ERRNO_UNAVAILABLE;

    if(input_params_n > 0) {
        t->ippc_connection->input_params_n = input_params_n;
        memcpy(t->ippc_connection->input_params,
            tshd_current->ippc_input_params,
            input_params_n * sizeof(*t->ippc_connection->input_params)
        );
    }

    if(!thread_is_waiting_allowed())
        t->ippc_connection->cannot_wait = TRUE;

    enum jet_ippc_connection_state state = jet_ippc_connection_get_state(t->ippc_connection);

    while(state != JET_IPPC_CONNECTION_STATE_TERMINATED)  {
        jet_ippc_connection_execute(t->ippc_connection, t->ippc_server_connection);
        state = jet_ippc_connection_get_state(t->ippc_connection);

        // if(state == JET_IPPC_CONNECTION_STATE_PAUSED) unreachable(); // TODO
    }

    pok_ret_t ret = POK_ERRNO_OK;

    switch(t->ippc_connection->terminate_status) {
        case JET_IPPC_CONNECTION_TERMINATE_STATUS_OK:
            ret = POK_ERRNO_OK;

            int output_params_n = t->ippc_connection->output_params_n;
            tshd_current->ippc_output_params_n = output_params_n;
            if(output_params_n > 0) {
                memcpy(tshd_current->ippc_output_params,
                    t->ippc_connection->output_params,
                    output_params_n * sizeof(*t->ippc_connection->output_params)
                );
            }
            break;
        case JET_IPPC_CONNECTION_TERMINATE_STATUS_SERVICE_RESETED:
        case JET_IPPC_CONNECTION_TERMINATE_STATUS_FAILED:
        case JET_IPPC_CONNECTION_TERMINATE_STATUS_CANCELLED:
            ret = POK_ERRNO_UNAVAILABLE;
            break;
        default:
            unreachable();
    }

    pok_preemption_local_disable();
    jet_ippc_connection_close(t->ippc_connection);
    t->ippc_connection = NULL;

    if(t->relations_stop.first_donator != NULL) {
        // It is safe to stop thread now.
        thread_stop(t);
    }

    pok_preemption_local_enable();

    return ret;
}

static struct jet_partition_arinc_ippc_portal_type* find_arinc_portal_type(const char* portal_name)
{
    pok_partition_arinc_t* part = current_partition_arinc;

    for(int i = 0; i < part->portal_types_n; i++) {
        struct jet_partition_arinc_ippc_portal_type* arinc_portal_type = &part->portal_types[i];

        if(strcmp(portal_name, arinc_portal_type->portal_type->portal_name) == 0)
            return arinc_portal_type;
    }

    return NULL;
}

/* static struct jet_partition_arinc_ippc_portal_type* get_arinc_portal_type(int portal_type_id)
{
    pok_partition_arinc_t* part = current_partition_arinc;
    if(portal_type_id < 0 || portal_type_id >= part->portal_types_n) return NULL;

    struct jet_partition_arinc_ippc_portal_type* arinc_portal_type = &part->portal_types[portal_type_id];

    return arinc_portal_type;
} */

static struct jet_ippc_portal* get_server_portal(
    int server_portal_id)
{
    pok_partition_arinc_t* part = current_partition_arinc;

    int portal_type_id = server_portal_id >> 16;
    int client_index = server_portal_id & 0xffff;

    if(portal_type_id < 0 || portal_type_id >= part->portal_types_n) return NULL;

    struct jet_ippc_portal_type* portal_type = part->portal_types[portal_type_id].portal_type;

    if(client_index < 0 || client_index > portal_type->n_portals) return NULL;

    return &portal_type->portals[client_index];
}


pok_ret_t jet_ippc_partition_arinc_get_portal_type_info(
    const char* __user portal_name,
    int* __user portal_type_id,
    int* __user n_clients)
{
    pok_partition_arinc_t* part = current_partition_arinc;

    char portal_name_k[MAX_NAME_LENGTH];

    if(kstrncpy(portal_name_k, portal_name, MAX_NAME_LENGTH) == NULL)
        return POK_ERRNO_EFAULT;

    int* __kuser portal_type_id_k = jet_user_to_kernel_typed(portal_type_id);
    if(portal_type_id_k == NULL) return POK_ERRNO_EFAULT;

    int* __kuser n_clients_k = jet_user_to_kernel_typed(n_clients);
    if(n_clients_k == NULL) return POK_ERRNO_EFAULT;

    struct jet_partition_arinc_ippc_portal_type* arinc_portal_type
        = find_arinc_portal_type(portal_name_k);

    if(arinc_portal_type == NULL) return POK_ERRNO_NOTFOUND;

    *portal_type_id_k = arinc_portal_type - part->portal_types;
    *n_clients_k = arinc_portal_type->portal_type->n_portals;

    return POK_ERRNO_OK;
}

pok_ret_t jet_ippc_partition_arinc_get_portal_info(
    int server_portal_id,
    int* __user n_connections)
{
    int* __kuser n_connections_k = jet_user_to_kernel_typed(n_connections);
    if(n_connections_k == NULL) return POK_ERRNO_EFAULT;

    struct jet_ippc_portal* portal = get_server_portal(server_portal_id);

    if(portal == NULL) return POK_ERRNO_EINVAL;

    *n_connections_k = portal->n_connections;

    return POK_ERRNO_OK;
}

pok_ret_t jet_ippc_partition_arinc_create_connections(
    int server_portal_id,
    void* __user entry,
    size_t user_stack_size,
    int n_connections,
    pok_thread_id_t* __user thread_id)
{
    pok_partition_arinc_t* part = current_partition_arinc;

    pok_thread_id_t* __kuser thread_id_k = jet_user_to_kernel_typed(thread_id);
    if(thread_id_k == NULL) return POK_ERRNO_EFAULT;

    /*
     * We can create threads only if the partition is in INIT mode.
     */
    if (part->mode == POK_PARTITION_MODE_NORMAL) {
        return POK_ERRNO_PARTITION_MODE;
    }

    struct jet_ippc_portal* portal = get_server_portal(server_portal_id);
    if(portal == NULL) return POK_ERRNO_EINVAL;

    if(n_connections <= 0) return POK_ERRNO_EINVAL;

    if(portal->n_connections_ready + n_connections > portal->n_connections)
        return POK_ERRNO_TOOMANY;

    pok_thread_id_t thread_id_first = part->nthreads_used;

    assert(thread_id_first + n_connections <= part->nthreads);

    for(int i = 0; i < n_connections; i++) {
        // TODO: Make server threads to have different names
        static const char server_thread_name[] = "SERVER THREAD";

        pok_thread_t* t = &part->threads[thread_id_first + i];

        memcpy(t->name, server_thread_name, sizeof(server_thread_name));
        t->entry = entry;
        t->base_priority = 0; // Priority is ignored for server threads.
        t->period = POK_TIME_INFINITY;
        t->time_capacity = POK_TIME_INFINITY;
        t->deadline = DEADLINE_SOFT;
        t->user_stack_size = user_stack_size;
    }

    if(!thread_create_several(&part->threads[thread_id_first], n_connections))
        return POK_ERRNO_UNAVAILABLE;

    for(int i = 0; i < n_connections; i++) {
        struct jet_ippc_connection* connection
            = jet_ippc_portal_create_connection(portal, thread_id_first + i);

        assert(connection != NULL);

        part->threads[thread_id_first + i].ippc_server_connection = connection;
    }

    *thread_id_k = thread_id_first;

    return POK_ERRNO_OK;
}

pok_ret_t jet_ippc_partition_arinc_return(void)
{
    pok_partition_arinc_t* part = current_partition_arinc;
    pok_thread_t* t = current_thread;

    struct jet_ippc_connection* connection = current_thread->ippc_server_connection;

    if(connection == NULL)
        return POK_ERRNO_MODE; // Not a server thread.

    struct jet_thread_shared_data* tshd_current = part->kshd->tshd
        + (t - part->threads);

    int output_params_n = tshd_current->ippc_output_params_server_n;

    if(output_params_n < 0 || output_params_n > IPPC_MAX_OUTPUT_PARAMS_N)
        return POK_ERRNO_EINVAL;

    connection->output_params_n = output_params_n;

    if(output_params_n > 0) {
        memcpy(connection->output_params,
            tshd_current->ippc_output_params_server,
            output_params_n * sizeof(*t->ippc_connection->output_params)
        );
    }

    pok_preemption_local_disable();

    // Thread cannot execute anything in donation state.
    assert(t->relations_stop.donate_target == NULL);
    // While already stopped, thread shouldn't stop itself.
    assert_os(t->relations_stop.first_donator == NULL);

    jet_ippc_connection_terminate(connection, JET_IPPC_CONNECTION_TERMINATE_STATUS_OK);

    /* If IPPC request will be started again, local scheduler shouldn't miss that point. */
    pok_sched_local_invalidate();

    t->state = POK_STATE_STOPPED;

    pok_preemption_local_enable();

    return POK_ERRNO_OK;
}
