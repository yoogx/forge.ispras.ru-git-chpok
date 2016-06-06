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


#ifndef  __POK_USER_BLACKBOARD_H__
#define __POK_USER_BLACKBOARD_H__

#include <config.h>

#ifdef POK_NEEDS_BLACKBOARDS

// must be at least MAX_NAME_LENGTH of ARINC653
#define POK_BLACKBOARD_MAX_NAME_LENGTH 30
#define POK_BLACKBOARD_NAME_EQ(x, y) (strncmp((x), (y), POK_BLACKBOARD_MAX_NAME_LENGTH) == 0)

#include <uapi/blackboard_types.h>

// All blackboard-related functions are already defined as syscalls.
#include <core/syscall.h>

#endif

#endif

