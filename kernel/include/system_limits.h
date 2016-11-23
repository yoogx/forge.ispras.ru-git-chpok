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
 *
 * This file also incorporates work covered by POK License.
 * Copyright (c) 2007-2009 POK team
 */

/*
 * System limits, used by the kernel.
 * 
 * Taken from Appendix E of ARINC653 P1-3.
 */

#ifndef __POK_SYSTEM_LIMITS_H__
#define __POK_SYSTEM_LIMITS_H__

// Number of partition should be contstrainted by integrator.
#define SYSTEM_LIMIT_NUMBER_OF_MESSAGES       512  /* module scope */
#define SYSTEM_LIMIT_MESSAGE_SIZE             8192 /* module scope */

// SYSTEM_LIMIT_NUMBER_OF_.. should be constrainted by integrator.

#define MAX_LOCK_LEVEL 16

#define MIN_PRIORITY_VALUE 1
#define MAX_PRIORITY_VALUE 239

#endif /* __POK_SYSTEM_LIMITS_H__ */
