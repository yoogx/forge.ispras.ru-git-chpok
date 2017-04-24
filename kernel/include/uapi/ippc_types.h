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

#ifndef __JET_UAPI_IPPC_TYPES_H__
#define __JET_UAPI_IPPC_TYPES_H__

#define IPPC_MAX_INPUT_PARAMS_N 6 // Request identificator as function's selector plus (at most) 5 function parameters.
#define IPPC_MAX_OUTPUT_PARAMS_N 2 // Return status plus (at most) 1 OUT function's parameter.

/* Address range in the client which can be accessed from the server. */
struct jet_ippc_client_access_window
{
    const void* start;
    size_t size;

    pok_bool_t is_writable;
};

#define IPPC_MAX_ACCESS_WINDOWS_N 5 // Maximum number of access windows for single IPPC request.

#endif /* __JET_UAPI_IPPC_TYPES_H__ */
