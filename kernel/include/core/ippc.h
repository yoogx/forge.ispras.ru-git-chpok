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

#ifndef __POK_CORE_IPPC_H__
#define __POK_CORE_IPPC_H__

#include <core/partition.h>
#include <uapi/ippc_types.h>

enum jet_ippc_connection_state
{
    /* Connection is not opened. */
    JET_IPPC_CONNECTION_STATE_INACTIVE,
    /* Connection may be executed on the server. */
    JET_IPPC_CONNECTION_STATE_ACTIVE,
    /* Connection is paused (by the server). Only the first connection in the chain may have given status. */
    JET_IPPC_CONNECTION_STATE_PAUSED,
    /* IPPC request has been terminated. */
    JET_IPPC_CONNECTION_STATE_TERMINATED,
};

/* Status of terminated connection. */
enum jet_ippc_connection_terminate_status
{
    /*
     * Server has processed IPPC request successfully.
     *
     * Additional info can be extracted from output parameters.
     */
    JET_IPPC_CONNECTION_TERMINATE_STATUS_OK,
    /*
     * Server fails to process IPPC request for unknown reason.
     *
     * Connection may be reseted by the server or service may be not ready.
     * This status is used also when IPPC request has been processed by the server,
     * but cancelled by the client.
     *
     * Whether IPPC request may be performed after reopening connection is unspecified.
     */
    JET_IPPC_CONNECTION_TERMINATE_STATUS_FAILED,

    /*
     * Service has been reseted during IPPC request execution.
     *
     * If you want to complete IPPC request, you may reopen connection after service being initialized.
     *
     * Init connection may be restarted immediately.
     */
    JET_IPPC_CONNECTION_TERMINATE_STATUS_SERVICE_RESETED,

    /* Request has been cancelled. */
    JET_IPPC_CONNECTION_TERMINATE_STATUS_CANCELLED,
};


/* Internal representation of struct jet_ippc_client_access_window. */
struct jet_ippc_access_range{
    const void* __user start;
    const void* __user end;
    pok_bool_t is_writable;
};

/* Single IPPC connection between client and server partitions. */
struct jet_ippc_connection
{
    /*
     * State of the connection (from the view of the client).
     */
    enum jet_ippc_connection_state state;

    /*
     * If state is PAUSED, this field contains timeout for that pause.
     * (<0 means inifinite pause).
     *
     * If client finds connection in PAUSED state, it may read this field
     * without any protection. This is because resuming connection from
     * the server side doesn't clear the field.
     */
    pok_time_t timeout;

    /* If connection's state is TERMINATED, this is a reason of the termination. */
    enum jet_ippc_connection_terminate_status terminate_status;

    /* Set in deployment.c */
    pok_partition_t* client_part;
    /* Set when connection is opened by the client. */
    uint16_t client_handler_id;

    /*
     * Whether connection is prohibited to pause.
     *
     * By default, this flag is FALSE.
     *
     * It can be set by the client before first execute().
     * This flag is set automatically on first execute, if it chaines from
     * connection with this flag set.
     */
    pok_bool_t cannot_wait;

    uintptr_t input_params[IPPC_MAX_INPUT_PARAMS_N];
    int input_params_n;
    uintptr_t output_params[IPPC_MAX_OUTPUT_PARAMS_N];
    int output_params_n;

    struct jet_ippc_access_range access_ranges[IPPC_MAX_ACCESS_WINDOWS_N];
    int access_ranges_n;

    /* Set in deployment.c */
    pok_partition_t* server_part;
    /* Set when connection is initialized by the server. */
    uint16_t server_handler_id;

    /*
     * Whether server handler can be shared between several connections.
     * This affects on ability for such server handler to use (chained)
     * IPPC requests.
     *
     * Several connections with this flag set may share same '.child_connection'.
     * But do that in a way, that they may not to affect on shared connection:
     *
     * 1. If client cancels connection with this flag set, cancelling isn't
     * propagated to the chained connection. Instead, cancelled connection
     * terminates immediately.
     * 2. Connections with this flag set cannot be paused.
     * (Implies cannot_wait set to TRUE).
     *
     * This is useful for IPPC init calls from main() process of the
     * server partition.
     *
     * May be set by the server before connection can be used.
     */
    pok_bool_t server_handler_is_shared;

    /* Cancelled by the client. Have a sence only for active connection. */
    pok_bool_t is_cancelled;

    /*
     * Whether parameters of the connection are fixed.
     *
     * This flag is set when execute() has been called first time.
     *
     * If this flag is not set, then cancelling connection automatically
     * terminates it.
     *
     * Doesn't have a sence for inactive connection.
     */
    pok_bool_t is_fixed;

    /* Chaining of the connections.
     *
     * Inactive/terminated connection shouldn't be part of any chaining.
     *
     * Normally, chain of connections is linear: it has single start
     * and single leaf connection.
     *
     * Exception is for connections with '.server_handler_is_shared'
     * flag set: several such connections may have single child connection.
     *
     * Only the last(leaf) connection in the chain may be terminated,
     * paused or unpaused by the server.
     *
     * Only the first(start) connection in the chain accepts PAUSE/UNPAUSE
     * events.
     *
     * (In case of connections with '.sever_handler_is_shared' flag set,
     * it could be several start connections.
     * But in that case PAUSE/UNPAUSE events are not generated.)
     */

    /*
     * For make a progress in executing our connection, this connection should be executed.
     *
     * NULL if given connection is the last(leaf) in the chain.
     */
    struct jet_ippc_connection* child_connection;

    /*
     * First connection in the list of (direct) waiters for us.
     *
     * NULL if given connection is the first in the chain.
     */
    struct jet_ippc_connection* parent_connection_first;

    /*
     * Next connection in the list of waiters for child_connection.
     *
     * May be not-NULL only if field .server_handler_is_shared is TRUE.
     */
    struct jet_ippc_connection* parent_connection_next;
};

/*
 * Return the last connection in the chain of connections.
 *
 * Should be executed with global preemption disabled.
 */
struct jet_ippc_connection*
jet_ippc_connection_last(struct jet_ippc_connection* connection);

/*
 * Return the first connection in the chain of connections.
 *
 * Should be executed with global preemption disabled.
 *
 * This function can be called only for connection with '.cannot_pause' flag is FALSE.
 * In that case flag '.server_handler_is_shared' is FALSE too, so
 * chain of connections is linear and has single top-most parent.
 */
struct jet_ippc_connection*
jet_ippc_connection_first(struct jet_ippc_connection* connection);


/* Return state of IPPC connection. */
enum jet_ippc_connection_state
jet_ippc_connection_get_state(struct jet_ippc_connection* connection);

/*
 * Set access windows for given connection.
 *
 * Called by the client after connection opening but before
 * executing it.
 *
 * Return EOK on success.
 *
 * Return EINVAL if *format* of parameters is incorrect:
 * some window has zero size, some window's are overlapped, or there are
 * too many windows.
 *
 * Note, that accessibility of given windows by the client isn't
 * checked here: it will be checked on access attempt.
 */
jet_ret_t jet_ippc_connection_set_access_windows(
    struct jet_ippc_connection* connection,
    const struct jet_ippc_client_access_window* access_windows,
    int n);

/*
 * Execute IPPC request through given connection.
 *
 * Connection should be previously opened (that is, not in INACTIVE state).
 *
 * Executing in TERMINATED state returns immediately.
 *
 * Called by the client.
 *
 * After the first time this function is called, any changes in connection
 * are prohibited.
 *
 * First call to the function forms the chain of connections:
 *
 *  - If 'chained_connection' is NULL, 'connection' is assumed to be the
 *      first in the chain.
 *
 *  - If 'chained_connection' is not NULL, it should be opened connection
 *      which terminates the chain (jet_ippc_connection_last() returns itself)
 *      and 'executed()'.
 *
 *      After the call, 'chained_connection' will be chained to the 'connection'.
 *
 * Futher calls to the function should pass the same 'chained_connection'
 * as for the first call.
 *
 * Exception is for 'chained_connection' with 'server_handler_is_shared'
 * flag set: such connection may be attached at any time, but only if
 * all connections attached previously have this flag set too.
 */
void
jet_ippc_connection_execute(struct jet_ippc_connection* connection,
    struct jet_ippc_connection* chained_connection);

/*
 * Close IPPC connection.
 *
 * Connection should be in TERMINATED state.
 *
 * Called by the client.
 */
void jet_ippc_connection_close(struct jet_ippc_connection* connection);

/*
 * Cancel IPPC connection.
 *
 * When such connection is executed, server tends to terminate it faster.
 * Cancellled connections cannot be PAUSED.
 *
 * Called by the client.
 *
 * Client gets no events from cancelled connections.
 */
void jet_ippc_connection_cancel(struct jet_ippc_connection* connection);

/*
 * Terminate the IPPC request with given status.
 *
 * Called by the server.
 *
 * If terminate_status is 'OK', output parameters should be set before
 * that call (otherwise they are assumed to be absent).
 *
 * Shouldn't be used for init connection.
 */
void jet_ippc_connection_terminate(struct jet_ippc_connection* connection,
    enum jet_ippc_connection_terminate_status terminate_status);

/*
 * Pause given connection.
 *
 * Called by the server.
 *
 * 'timeout' denotes the time, when connection can be continue()'d by the
 * client. If 'timeout' is negative, continue() is disallowed.
 *
 * Return TRUE on success, FALSE if connection cannot be paused:
 * - has '.cannot_wait' flag set
 * - has '.is_cancelled' flag set
 *
 * It is prohibited to pause a connection with '.server_handler_is_shared'
 * flag set.
 */
pok_bool_t jet_ippc_connection_pause(struct jet_ippc_connection* connection,
    pok_time_t timeout);

/*
 * Unpause given connection.
 *
 * Called by the server.
 *
 * Return TRUE if connection was paused previously, return FALSE if connection
 * was in ACTIVE state (e.g., continue()'d by the client).
 */
pok_bool_t jet_ippc_connection_unpause(struct jet_ippc_connection* connection);

/*
 * Continue(unpause) given connection.
 *
 * Called by the client.
 *
 * May be called only if connection has positive 'timeout' field, and
 * current time is after that moment.
 *
 * Return TRUE if connection was paused previously, return FALSE if connection
 * was in ACTIVE state (e.g., unpause()'d by the server).
 */
pok_bool_t jet_ippc_connection_continue(struct jet_ippc_connection* connection);

/* State of copiing to/from client. */
struct jet_ippc_remote_access_state
{
    union {
        char* dst;
        char* __remote dst_remote;
    };
    union {
        const char* src;
        const char* __remote src_remote;
    };

    size_t n;

    pok_bool_t is_to_client;

    size_t n_processed;

    // Whether access is completed.
    pok_bool_t is_completed;
    /*
     * Connection has been cancelled.
     *
     * This field has a sence only in "completed" state.
     */
    pok_bool_t is_cancelled;
};

/*
 * Prepare for copyiing to the client.
 *
 * Return EOK on success.
 *
 * Return EFAULT if access to the client range is forbidden.
 */
jet_ret_t jet_ippc_connection_copy_to_client_init(
    struct jet_ippc_connection* connection,
    struct jet_ippc_remote_access_state* ra_state,
    void* __user dst, // User address in the client
    const void* src, // Kernel(!) address in the server
    size_t n);

/*
 * Prepare for copyiing from the client.
 *
 * Return EOK on success.
 *
 * Return EFAULT if access to the client range is forbidden.
 */
jet_ret_t jet_ippc_connection_copy_from_client_init(
    struct jet_ippc_connection* connection,
    struct jet_ippc_remote_access_state* ra_state,
    void* dst, // Kernel(!) address in the server
    const void* __user src, // User address in the client
    size_t n);


/*
 * Execute given remote access.
 *
 * The function tends to complete access, but may return in incomplete state.
 *
 * Currently, the function returns only after the completing the request.
 */
void jet_ippc_connection_remote_access_execute(
    struct jet_ippc_connection* connection,
    struct jet_ippc_remote_access_state* ra_state);

/* State of the IPPC portal. */
enum jet_ippc_portal_state {
    /* Portal's initialization is in progress. */
    JET_IPPC_PORTAL_STATE_INITIALIZING,
    /* Portal is ready to use. */
    JET_IPPC_PORTAL_STATE_READY,
    /* Portal cannot be used. */
    JET_IPPC_PORTAL_STATE_UNUSABLE,
};

/*
 * Portal as an array of IPPC connections which can be used by the client
 * for request a particular operation from the server.
 */
struct jet_ippc_portal
{
    /*
     * Connection for initialize the portal.
     *
     * This connection is always ready.
     *
     * Set in deployment.c.
     */
    struct jet_ippc_connection* init_connection;

    /*
     * Array of connections.
     *
     * Some of them may be uninitialized.
     *
     * Set in deployment.c.
     */
    int n_connections;
    struct jet_ippc_connection* connections;

    enum jet_ippc_portal_state portal_state;

    /*
     * Number of connections ready to work (or already used).
     *
     * This field may be non-zero even if .portal_state is UNAVAILABLE.
     */
    int n_connections_ready;
};

/* Create portal object. Called when whole module is initialized. */
void jet_ippc_portal_create(struct jet_ippc_portal* portal,
    uint16_t init_server_handler_id);


/*
 * Open init connection for given portal.
 *
 * Connection should be in INACTIVE state.
 *
 * Return init connection for the portal.
 *
 * NOTE: If portal state is not INITIALIZING, connection immediately
 * transists into TERMINATED state.
 *
 * Called by the the client.
 *
 * Input parameters are filled automatically.
 */
struct jet_ippc_connection*
jet_ippc_portal_open_init_connection(struct jet_ippc_portal* portal,
    uint16_t client_handler_init_id);

/*
 * Extract result of terminated portal's init connection.
 *
 * Can only be used if connection is successfully terminated.
 */
pok_bool_t jet_ippc_portal_ready_state_from_init_connection(
    struct jet_ippc_connection* init_connection);

/*
 * Open first unused (INACTIVE) connection for the portal.
 *
 * May be called even if the portal is not ready. In that case,
 * connection will be automatically in terminated state.
 *
 * Return connection opened.
 *
 * Return NULL if all connections are currently in use.
 *
 * Called by the the client.
 *
 * Input parameters may be set before first jet_ippc_connection_execute() call.
 * By default, parameters are assumed to be absent (input_params_n = 0).
 */
struct jet_ippc_connection*
jet_ippc_portal_open_connection(struct jet_ippc_portal* portal,
    uint16_t client_handler_id);

/*
 * Create (initialize) IPPC connection object for the portal.
 *
 * May be called if .portal_state is INITIALIZING.
 *
 * Called by the server.
 *
 * Return connection created or NULL if all connections has been
 * created before.
 */
struct jet_ippc_connection* jet_ippc_portal_create_connection(
    struct jet_ippc_portal* portal, uint16_t server_handler_id);

/*
 * Finish initialization of the portal.
 *
 * This automatically marks init connection as terminated (if it was opened).
 *
 * Value TRUE for 'success' parameter is allowed only when all connections
 * has been initialized before.
 */
void jet_ippc_portal_finish_initialization(struct jet_ippc_portal* portal,
    pok_bool_t success);


/*
 * Terminate portal, making it uninitialized again.
 *
 * This terminates all opened connections for this portal.
 *
 * If is_reseted is 'true', portal's ready state is not fixed.
 * Terminate status for connections is JET_IPPC_CONNECTION_TERMINATE_STATUS_SERVICE_RESETED.
 *
 * If is_reseted is 'false', portal's ready state is fixed.
 * Terminate status for connections is JET_IPPC_CONNECTION_TERMINATE_STATUS_FAILED.
 *
 * Called by the server when it is restarted or shutdown.
 */
void jet_ippc_portal_terminate(struct jet_ippc_portal* portal,
    pok_bool_t is_reseted);


/* List of portals with the same meaning but provided for different partitions.*/
struct jet_ippc_portal_type
{
    const char* portal_name;

    /* Array of portals. */
    int n_portals;
    struct jet_ippc_portal* portals;
};

#endif /* __POK_CORE_IPPC_H__ */
