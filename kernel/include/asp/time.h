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

#ifndef __JET_ASP_TIME_H__
#define __JET_ASP_TIME_H__

#include <types.h>
#include <uapi/time.h>

/* Return current system time. */
pok_time_t ja_system_time(void);

/* Return current calendar time (seconds since Epoch). */
time_t ja_calendar_time(void);


#endif /* __JET_ASP_TIME_H__ */
