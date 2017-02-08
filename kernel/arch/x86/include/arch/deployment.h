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

/* Definitions which are used in deployment.c */

#ifndef __JET_X86_DEPLOYMENT_H__
#define __JET_X86_DEPLOYMENT_H__

#include <stdint.h>
#include <asp/space.h>

/* 
 * Description of one segment.
 */
struct x86_segment
{
    uintptr_t   paddr; /* Set in deployment.c. */
    size_t      size; /* Set in deployment.c. */
};

/*
 * Array of memory segments.
 *
 * Segment (i) corresponds to space_id (i+1).
 *
 * Should be defined in deployment.c.
 */
extern const struct x86_segment ja_segments[];
extern const int ja_segments_n;

#endif /* __JET_PPC_DEPLOYMENT_H__ */
