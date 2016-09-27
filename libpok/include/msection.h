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

#ifndef __LIBJET_MSECTION_H__

#include <uapi/msection.h>

/* Initialize mscection object. */
void msection_init(struct msection* section);

/* 
 * Enter into given msection.
 * 
 * On error (incorrect state, incorrect parameter) partition is terminated
 * (but can be restarted).
 */
void msection_enter(struct msection* section);

/*
 * Leave given msection.
 * 
 * On error (incorrect state, incorrect parameter) partition is terminated
 * (but can be restarted).
 */
void msection_leave(struct msection* section);

#define __LIBJET_MSECTION_H__
#endif /* __LIBJET_MSECTION_H__ */
