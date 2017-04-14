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

#ifndef __POK_CORE_IPPC_ARINC_H__
#define __POK_CORE_IPPC_ARINC_H__

#include <core/partition_arinc.h>

/*
 * Wait until IPPC portal is initialized.
 *
 * Also requests identificator of that portal.
 *
 * Used by the client.
 */
pok_ret_t jet_ippc_partition_arinc_init_portal(const char* __user portal_name,
    int* __user portal_id);

/*
 * IPPC call.
 *
 * Parameters are extracted from thread shared data: 'ippc_input_params' and 'ippc_input_params_n'.
 *
 * Used by the client.
 */
pok_ret_t jet_ippc_partition_arinc_call(int portal_id,
    const struct jet_ippc_client_access_window* __user access_windows,
    int access_windows_n);

/*
 * Return information about given portal type.
 *
 * portal_type_id - identificator of the portal type.
 * n_clients - number of partitions used given portal type as clients.
 *
 * Used by the server.
 */
pok_ret_t jet_ippc_partition_arinc_get_portal_type_info(
    const char* __user portal_name,
    int* __user portal_type_id,
    int* __user n_clients);

/*
 * Return information about given portal.
 *
 * server_portal_id is combined from portal_type_id (high 16 bits)
 * and client index (from 0, low 16 bits).
 *
 * n_connections - number of connections for given portal.
 *
 * Used by the server.
 */
pok_ret_t jet_ippc_partition_arinc_get_portal_info(
    int server_portal_id,
    int* __user n_connections);

/*
 * Create connections for given portal.
 *
 * server_portal_id is combined from portal_type_id (high 16 bits)
 * and client index (from 0, low 16 bits).
 *
 * n_connections - number of connections created.
 *
 * thread_id - identificator of the thread corresponded for the first
 * connection; other connections will have successive identificators.
 *
 * Used by the server.
 */
pok_ret_t jet_ippc_partition_arinc_create_connections(
    int server_portal_id,
    void* __user entry,
    size_t user_stack_size,
    int n_connections,
    pok_thread_id_t* __user thread_id);

/*
 * Return from the IPPC call.
 *
 * Output parameters should be set in thread shared data before:
 * 'ippc_input_params_server' and 'ippc_input_params_server_n'.
 */
pok_ret_t jet_ippc_partition_arinc_return(void);

pok_ret_t jet_ippc_partition_arinc_copy_to_client(
    void* __user dst, const void* __user src, size_t n);

pok_ret_t jet_ippc_partition_arinc_copy_from_client(
    void* __user dst, const void* __user src, size_t n);

#endif /* __POK_CORE_IPPC_ARINC_H__ */
