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

/* Configuration parameters for ARINC. */

#ifndef __LIBJET_ARINC_CONFIG_H__
#define __LIBJET_ARINC_CONFIG_H__

#include <config.h>
#include <types.h>

// Maximum number of buffers. Set in deployment.c
extern size_t arinc_config_nbuffers;

// Maximum number of blackboards. Set in deployment.c
extern size_t arinc_config_nblackboards;

// Maximum number of semaphores. Set in deployment.c
extern size_t arinc_config_nsemaphores;

// Maximum number of events. Set in deployment.c
extern size_t arinc_config_nevents;

// Memory for messages, used by buffers and blackboards. Set in deployment.c.
extern size_t arinc_config_messages_memory_size;



#endif /* __LIBJET_ARINC_CONFIG_H__ */
