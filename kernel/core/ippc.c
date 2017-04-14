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

#include <core/ippc.h>
#include <core/sched.h>
#include <core/debug.h>
#include <core/memblocks.h>

/* Return true if connection is in active state, that is can be terminated. */
static pok_bool_t connection_is_active(struct jet_ippc_connection* connection)
{
    return (connection->state != JET_IPPC_CONNECTION_STATE_INACTIVE)
        && (connection->state != JET_IPPC_CONNECTION_STATE_TERMINATED);
}


struct jet_ippc_connection*
jet_ippc_connection_last(struct jet_ippc_connection* connection)
{
    struct jet_ippc_connection* last_connection = connection;

    while(last_connection->child_connection)
        last_connection = last_connection->child_connection;

    return last_connection;
}

struct jet_ippc_connection*
jet_ippc_connection_first(struct jet_ippc_connection* connection)
{
    struct jet_ippc_connection* first_connection = connection;

    assert(!connection->cannot_wait);

    while(first_connection->parent_connection_first)
        first_connection = first_connection->parent_connection_first;

    return first_connection;
}


/* Create connection object. Called when whole module is initialized. */
static void connection_create(struct jet_ippc_connection* connection)
{
    connection->state = JET_IPPC_CONNECTION_STATE_INACTIVE;
    connection->server_handler_is_shared = FALSE; // May be redefined later
    connection->cannot_wait = FALSE; // May be redefined later

    connection->child_connection = NULL;
    connection->parent_connection_first = NULL;
    connection->parent_connection_next = NULL;
}

/* Initialize connection object. */
static void connection_init(struct jet_ippc_connection* connection,
    uint16_t server_handler_id)
{
    connection->server_handler_id = server_handler_id;
}

/*
 * Terminate given connection. Connection should be in active/stopped state.
 *
 * Should be called with preemption disabled.
 */
static void connection_terminate(struct jet_ippc_connection* connection,
    enum jet_ippc_connection_terminate_status terminate_status)
{
    assert(connection_is_active(connection));

    if(connection->state == JET_IPPC_CONNECTION_STATE_PAUSED) {
        struct jet_ippc_connection* first_connection = jet_ippc_connection_first(connection);
        pok_partition_add_event(first_connection->client_part,
            JET_PARTITION_EVENT_TYPE_IPPC_UNPAUSED,
            first_connection->client_handler_id);
    }

    connection->state = JET_IPPC_CONNECTION_STATE_TERMINATED;
    connection->terminate_status = terminate_status;

    if(connection->server_part->ippc_handled == connection) {
        // Connection is currently handled. Clear this state.
        connection->server_part->ippc_handled = NULL;
        pok_sched_invalidate(); // Control should be transferred to the client via scheduler.
        if(connection->client_part == base_partition) {
            // Base partition is direct client. Whole connection's chain should be terminated.
            base_partition->ippc_executed = NULL;
        }
    }

    // Detach connection from the chain.
    for(struct jet_ippc_connection* parent_connection_first = connection->parent_connection_first;
        parent_connection_first != NULL;
        parent_connection_first = connection->parent_connection_first) {

        parent_connection_first->child_connection = NULL;
        connection->parent_connection_first = parent_connection_first->parent_connection_next;
        parent_connection_first->parent_connection_next = NULL;
    }
}

pok_bool_t jet_ippc_connection_pause(struct jet_ippc_connection* connection,
    pok_time_t timeout)
{
    pok_bool_t result = FALSE;

    assert(!connection->cannot_wait);

    pok_preemption_disable();

    if(!connection->is_cancelled) {
        struct jet_ippc_connection* first_connection = jet_ippc_connection_first(connection);
        if(first_connection->state == JET_IPPC_CONNECTION_STATE_ACTIVE) {
            first_connection->state = JET_IPPC_CONNECTION_STATE_PAUSED;
            first_connection->timeout = timeout;
            pok_partition_add_event(first_connection->client_part,
                JET_PARTITION_EVENT_TYPE_IPPC_PAUSED,
                first_connection->client_handler_id);
            result = TRUE;
        }
    }

    pok_preemption_enable();

    return result;
}


pok_bool_t jet_ippc_connection_unpause(struct jet_ippc_connection* connection)
{
    pok_bool_t result = FALSE;

    assert(!connection->cannot_wait);

    pok_preemption_disable();

    if(!connection->is_cancelled) {
        struct jet_ippc_connection* first_connection = jet_ippc_connection_first(connection);
        if(first_connection->state == JET_IPPC_CONNECTION_STATE_PAUSED) {
            first_connection->state = JET_IPPC_CONNECTION_STATE_ACTIVE;
            pok_partition_add_event(first_connection->client_part,
                JET_PARTITION_EVENT_TYPE_IPPC_UNPAUSED,
                first_connection->client_handler_id);
            result = TRUE;
        }
    }

    pok_preemption_enable();

    return result;
}

pok_bool_t jet_ippc_connection_continue(struct jet_ippc_connection* connection)
{
    pok_bool_t result = FALSE;

    assert(!connection->cannot_wait);
    assert(connection->child_connection == NULL); // First in the chain
    assert(connection->timeout > 0); // Should be pause with timeout.

    pok_preemption_disable();

    if(connection->state == JET_IPPC_CONNECTION_STATE_PAUSED) {
        connection->state = JET_IPPC_CONNECTION_STATE_ACTIVE;
        result = TRUE;
    }

    pok_preemption_enable();

    return result;
}


/* Check that client allows access to given range. */
static pok_bool_t ippc_check_access(struct jet_ippc_connection* connection,
    const void* __user addr, size_t size, pok_bool_t is_write)
{
    const void* __user addr_end = (const char* __user)addr + size;

    if(addr_end <= addr) return FALSE;

    for(int i = 0; i < connection->access_ranges_n; i++) {
        const struct jet_ippc_access_range* access_range = &connection->access_ranges[i];
        if(addr < access_range->start) continue;
        if(addr_end > access_range->end) continue;

        return !is_write || access_range->is_writable;
    }

    return FALSE;
}

/*
 * Prepare for copyiing to the client.
 *
 * Return POK_ERRNO_OK on success.
 *
 * Return POK_ERRNO_EFAULT if access to the client range is forbidden.
 */
pok_ret_t jet_ippc_connection_copy_to_client_init(
    struct jet_ippc_connection* connection,
    struct jet_ippc_remote_access_state* ra_state,
    void* __user dst, // User address in the client
    const void* src, // Kernel(!) address in the server
    size_t n)
{
    if(!ippc_check_access(connection, dst, n, TRUE)) return POK_ERRNO_EFAULT;

    const struct memory_block* mb = jet_partition_get_memory_block_for_addr(
        connection->client_part, dst, n);

    if((mb->maccess & MEMORY_BLOCK_ACCESS_WRITE) == 0) return POK_ERRNO_EFAULT;

    void* __remote dst_remote = jet_memory_block_get_remote_addr(mb, dst);

    ra_state->dst_remote = dst_remote;
    ra_state->src = src;
    ra_state->n = n;
    ra_state->is_to_client = TRUE;

    ra_state->n_processed = 0;
    ra_state->is_completed = FALSE;
    ra_state->is_cancelled = FALSE;

    return POK_ERRNO_OK;
}

/*
 * Prepare for copyiing from the client.
 *
 * Return POK_ERRNO_OK on success.
 *
 * Return POK_ERRNO_EFAULT if access to the client range is forbidden.
 */
pok_ret_t jet_ippc_connection_copy_from_client_init(
    struct jet_ippc_connection* connection,
    struct jet_ippc_remote_access_state* ra_state,
    void* dst, // Kernel(!) address in the server
    const void* __user src, // User address in the client
    size_t n)
{
    if(!ippc_check_access(connection, src, n, FALSE)) return POK_ERRNO_EFAULT;

    const struct memory_block* mb = jet_partition_get_memory_block_for_addr(
        connection->client_part, src, n);

    if((mb->maccess & MEMORY_BLOCK_ACCESS_READ) == 0) return POK_ERRNO_EFAULT;

    void* __remote src_remote = jet_memory_block_get_remote_addr(mb, src);

    ra_state->dst = dst;
    ra_state->src_remote = src_remote;
    ra_state->n = n;
    ra_state->is_to_client = FALSE;

    ra_state->n_processed = 0;
    ra_state->is_completed = FALSE;
    ra_state->is_cancelled = FALSE;

    return POK_ERRNO_OK;
}

/*
 * Execute given remote access.
 *
 * The function tends to complete access, but may return in incomplete state.
 *
 * Currently, the function returns only after the completing the request.
 */
void jet_ippc_connection_remote_access_execute(
    struct jet_ippc_connection* connection,
    struct jet_ippc_remote_access_state* ra_state)
{
    size_t chunk_size; // Size of the data we copy atomically, with preemption disabled.

    if(ra_state->is_completed) return;

    for(;ra_state->n_processed < ra_state->n; ra_state->n_processed += chunk_size) {
        chunk_size = ra_state->n - ra_state->n_processed;
        if(chunk_size > 4096) chunk_size = 4096;

        pok_preemption_disable();

        if(connection->is_cancelled) {
            ra_state->is_completed = TRUE;
            ra_state->is_cancelled = TRUE;
            pok_preemption_enable();
            return;
        }

        if(ra_state->is_to_client) {
            ja_copy_to_remote(connection->client_part->space_id,
                ra_state->dst_remote + ra_state->n_processed,
                ra_state->src + ra_state->n_processed,
                chunk_size);
        }
        else {
            ja_copy_from_remote(connection->client_part->space_id,
                ra_state->dst + ra_state->n_processed,
                ra_state->src_remote + ra_state->n_processed,
                chunk_size);
        }

        pok_preemption_enable();
    }

    ra_state->is_completed = TRUE;
    ra_state->is_cancelled = FALSE;
}


/* Open connection (generic steps). */
static void connection_open(struct jet_ippc_connection* connection,
    uint16_t client_handler_id)
{
    assert(connection->state == JET_IPPC_CONNECTION_STATE_INACTIVE);

    connection->state = JET_IPPC_CONNECTION_STATE_ACTIVE;
    connection->client_handler_id = client_handler_id;
    connection->input_params_n = 0;
    connection->output_params_n = 0;
    connection->access_ranges_n = 0;
    connection->is_cancelled = FALSE;
    connection->is_fixed = FALSE;
    connection->cannot_wait = connection->server_handler_is_shared;
}

/*
 * Set output parameters for connection according to portal state.
 *
 * Intended to be used before setting state of the connection into TERMINATED
 * with status OK.
 */
static void init_connection_set_output_params(struct jet_ippc_portal* portal)
{
    assert(portal->portal_state != JET_IPPC_PORTAL_STATE_INITIALIZING);

    struct jet_ippc_connection* init_connection = portal->init_connection;
    init_connection->output_params_n = 1;
    init_connection->output_params[0]
        = (portal->portal_state == JET_IPPC_PORTAL_STATE_READY) ? 1 : 0;
}

struct jet_ippc_connection*
jet_ippc_portal_open_init_connection(struct jet_ippc_portal* portal,
    uint16_t client_handler_init_id)
{
    struct jet_ippc_connection* init_connection = portal->init_connection;

    pok_preemption_disable();

    connection_open(init_connection, client_handler_init_id);

    if(portal->portal_state != JET_IPPC_PORTAL_STATE_INITIALIZING) {
        // Portal state is fixed, terminate connection immediately.
        init_connection_set_output_params(portal);
        connection_terminate(init_connection, JET_IPPC_CONNECTION_TERMINATE_STATUS_OK);
    }

    pok_preemption_enable();

    return init_connection;
}

/*
 * Extract result of terminated portal's init connection.
 *
 * Can only be used if connection is successfully terminated.
 */
pok_bool_t jet_ippc_portal_ready_state_from_init_connection(
    struct jet_ippc_connection* init_connection)
{
    return init_connection->output_params[0] == 1;
}

struct jet_ippc_connection*
jet_ippc_portal_open_connection(struct jet_ippc_portal* portal,
    uint16_t client_handler_id)
{
    struct jet_ippc_connection* connection = NULL;

    pok_preemption_disable();

    for(int i = 0; i < portal->n_connections; i++) {
        struct jet_ippc_connection* connection_tmp = &portal->connections[i];
        if(connection_tmp->state == JET_IPPC_CONNECTION_STATE_INACTIVE) {
            connection = connection_tmp;
            break;
        }
    }

    if(connection) {
        connection_open(connection, client_handler_id);
        if(portal->portal_state != JET_IPPC_PORTAL_STATE_READY)
            connection_terminate(connection, JET_IPPC_CONNECTION_TERMINATE_STATUS_SERVICE_RESETED);
    }

    pok_preemption_enable();

    return connection;
}


/* Return state of IPPC connection. */
enum jet_ippc_connection_state
jet_ippc_connection_get_state(struct jet_ippc_connection* connection)
{
    enum jet_ippc_connection_state ret;

    pok_preemption_disable();
    ret = connection->state;
    pok_preemption_enable();

    return ret;
}

pok_ret_t jet_ippc_connection_set_access_windows(
    struct jet_ippc_connection* connection,
    const struct jet_ippc_client_access_window* access_windows,
    int n)
{
    if((n < 0) || (n > IPPC_MAX_ACCESS_WINDOWS_N)) return POK_ERRNO_EINVAL;

    // TODO: Currently created array is unordered.
    // Checks for interleaving are missed too.

    for(int i = 0; i < n; i++)
    {
        const struct jet_ippc_client_access_window* access_window
            = &access_windows[i];
        struct jet_ippc_access_range* access_range = &connection->access_ranges[i];
        access_range->start = access_window->start;
        access_range->end = (const char* __user)access_window->start + access_window->size;

        if(access_range->end <= access_range->start) return POK_ERRNO_EINVAL;

        access_range->is_writable = access_window->is_writable;
    }

    connection->access_ranges_n = n;

    return POK_ERRNO_OK;
}


void jet_ippc_portal_create(struct jet_ippc_portal* portal,
    uint16_t init_server_handler_id)
{
    portal->n_connections_ready = 0;
    portal->portal_state = JET_IPPC_PORTAL_STATE_INITIALIZING;

    connection_create(portal->init_connection);

    for(int i = 0; i < portal->n_connections; i++) {
        connection_create(&portal->connections[i]);
    }

    connection_init(portal->init_connection, init_server_handler_id);
}

struct jet_ippc_connection* jet_ippc_portal_create_connection(
    struct jet_ippc_portal* portal, uint16_t server_handler_id)
{
    struct jet_ippc_connection* connection = NULL;

    pok_preemption_disable();

    if(portal->n_connections_ready < portal->n_connections) {
        connection = &portal->connections[portal->n_connections_ready];
        portal->n_connections_ready++;
        connection_init(connection, server_handler_id);
    }

    pok_preemption_enable();

    return connection;
}


/* Set state of the portal.
 *
 * Initialization of normal connections is not cancelled.
 *
 * Should be called with preemption disabled.
 *
 * Allowed transitions:
 *
 *  - INITIALIZING  -> INITIALIZING
 *  - INITIALIZING  -> READY
 *  - INITIALIZING  -> UNUSABLE
 *  - READY         -> INITIALIZING
 *  - READY         -> UNUSABLE
 *  - UNUSABLE      -> INITIALIZING
 *  - UNUSABLE      -> UNUSABLE
 */
static void portal_set_state(struct jet_ippc_portal* portal,
    enum jet_ippc_portal_state portal_state)
{
    enum jet_ippc_portal_state portal_state_old = portal->portal_state;
    portal->portal_state = portal_state;

    if(portal_state_old == JET_IPPC_PORTAL_STATE_INITIALIZING) {
        // Terminate init connection if it is active.
        struct jet_ippc_connection* init_connection = portal->init_connection;
        if(connection_is_active(init_connection)) {
            if(portal_state == JET_IPPC_PORTAL_STATE_INITIALIZING) {
                connection_terminate(init_connection, JET_IPPC_CONNECTION_TERMINATE_STATUS_SERVICE_RESETED);
            }
            else {
                init_connection_set_output_params(portal);
                connection_terminate(init_connection, JET_IPPC_CONNECTION_TERMINATE_STATUS_OK);
            }
        }
    }

    else if(portal_state_old == JET_IPPC_PORTAL_STATE_READY) {
        enum jet_ippc_connection_terminate_status terminate_status
            = (portal_state == JET_IPPC_PORTAL_STATE_INITIALIZING)
                ? JET_IPPC_CONNECTION_TERMINATE_STATUS_SERVICE_RESETED
                : JET_IPPC_CONNECTION_TERMINATE_STATUS_FAILED;

        for(int i = 0; i < portal->n_connections; i++) {
            struct jet_ippc_connection* connection = &portal->connections[i];

            if(connection_is_active(connection)) {
                connection_terminate(connection, terminate_status);
            }
        }
    }
}

void jet_ippc_portal_terminate(struct jet_ippc_portal* portal,
    pok_bool_t is_reseted)
{
    enum jet_ippc_portal_state portal_state = is_reseted
        ? JET_IPPC_PORTAL_STATE_INITIALIZING
        : JET_IPPC_PORTAL_STATE_UNUSABLE;

    pok_preemption_disable();

    portal_set_state(portal, portal_state);
    portal->n_connections_ready = 0;

    pok_preemption_enable();
}

void jet_ippc_portal_finish_initialization(struct jet_ippc_portal* portal,
    pok_bool_t success)
{
    pok_preemption_disable();

    assert(portal->portal_state == JET_IPPC_PORTAL_STATE_INITIALIZING);

    if(success) {
        assert(portal->n_connections_ready == portal->n_connections);
        portal_set_state(portal, JET_IPPC_PORTAL_STATE_READY);
    }
    else {
        portal_set_state(portal, JET_IPPC_PORTAL_STATE_UNUSABLE);
    }

    pok_preemption_enable();
}

/*
 * Execute IPPC request through given connection.
 *
 * Connection should be previously opened.
 *
 * Called by the client.
 */
void
jet_ippc_connection_execute(struct jet_ippc_connection* connection,
    struct jet_ippc_connection* chained_connection)
{
    pok_partition_t* part = current_partition;

    assert(connection->client_part == part);
    if(chained_connection) {
        assert(chained_connection->server_part == part);
    }

    pok_preemption_disable();

    if(connection->state == JET_IPPC_CONNECTION_STATE_TERMINATED) goto out;

    if(chained_connection && chained_connection->state != JET_IPPC_CONNECTION_STATE_ACTIVE) {
        /*
         * We could miss the fact that chained connection has been terminated.
         * This only can be in case of shared connection.
         */
        assert(chained_connection->server_handler_is_shared);
        chained_connection = NULL;
    }

    if(!connection->is_fixed) {
        // Connection is executed first time.
        if(chained_connection) {
            // Create a chain.
            assert(chained_connection->child_connection == NULL);
            assert(!chained_connection->cannot_wait || connection->cannot_wait);

            chained_connection->child_connection = connection;
            connection->parent_connection_first = chained_connection;
        }

        connection->is_fixed = TRUE;
    }
    else if(chained_connection) {
        // Connection is executed non-first time.
        if(chained_connection->child_connection != connection) {
            /*
             * Adding new connection to the chain. Possible only with
             * .server_handler_is_shared flag set.
             */
             assert(chained_connection->server_handler_is_shared);
             assert(connection->cannot_wait);

             chained_connection->child_connection = connection;
             chained_connection->parent_connection_next = connection->parent_connection_first;
             connection->parent_connection_first = chained_connection;
        }
    }
    else {
        // No connection is chained for connection which is executed non-first time.
        assert(!connection->parent_connection_first
            || connection->parent_connection_first->server_handler_is_shared);
    }

    if(base_partition->ippc_executed != NULL) {
        if(jet_ippc_connection_last(base_partition->ippc_executed) == connection) {
            pok_sched_invalidate();
        }
    }
    else {
        // We are base partition actually.
        assert(base_partition == part);
        // TODO: What if chain starts and ends on base_partition?
        part->ippc_executed = connection;
        pok_sched_invalidate();
    }


out:
    pok_preemption_enable();
}

void jet_ippc_connection_cancel(struct jet_ippc_connection* connection)
{
    pok_preemption_disable();

    if(!connection_is_active(connection) || connection->is_cancelled) goto out;

    if(!connection->cannot_wait) {
        /* If first connection in the chain is paused, resume it. */
        struct jet_ippc_connection* first_connection = jet_ippc_connection_first(connection);
        if(first_connection->state == JET_IPPC_CONNECTION_STATE_PAUSED) {
            first_connection->state = JET_IPPC_CONNECTION_STATE_ACTIVE;
            /*
             * Need to notify partition about unpausing.
             *
             * Notification isn't needed if we are the same partition.
             */
            if(first_connection->client_part != connection->client_part) {
                pok_partition_add_event(first_connection->client_part,
                    JET_PARTITION_EVENT_TYPE_IPPC_UNPAUSED,
                    first_connection->client_handler_id);
            }
        }
    }

    // Cancel ourself and futher connections in the chain.
    for(struct jet_ippc_connection* c = connection;
        c != NULL;
        c = c->child_connection) {

        if(c->server_handler_is_shared) {
            // Connection can be safetly terminated.
            if(c->child_connection) {
                // Break chain.
                struct jet_ippc_connection* shared_c = c->child_connection->parent_connection_first;
                if(shared_c == c) {
                    // We are the first parent connection for the child.
                    c->child_connection->parent_connection_first = c->parent_connection_next;
                }
                else {
                    // Navigate to previous connection in the list.
                    while(shared_c->parent_connection_next != c)
                        shared_c = shared_c->parent_connection_next;
                    // Remove ourself from the list.
                    shared_c->parent_connection_next = c->parent_connection_next;
                }

                c->child_connection = NULL;
                c->parent_connection_next = NULL;
            }
            connection_terminate(connection, JET_IPPC_CONNECTION_TERMINATE_STATUS_CANCELLED);
            break;
        }
    }

out:
    pok_preemption_enable();
}


/*
 * Close IPPC connection.
 *
 * Connection should be in TERMINATED state.
 *
 * Called by the client.
 */
void jet_ippc_connection_close(struct jet_ippc_connection* connection)
{
    pok_preemption_disable();

    assert(connection->state == JET_IPPC_CONNECTION_STATE_TERMINATED);

    connection->state = JET_IPPC_CONNECTION_STATE_INACTIVE;

    pok_preemption_enable();
}

void jet_ippc_connection_terminate(struct jet_ippc_connection* connection,
    enum jet_ippc_connection_terminate_status terminate_status)
{
    pok_preemption_disable();

    connection_terminate(connection, terminate_status);

    pok_preemption_enable();
}

