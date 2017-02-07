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

#ifndef __LIBJET_PROCESS_H__
#define __LIBJET_PROCESS_H__

/* 
 * Bind data to the specific process.
 * 
 * 'process_id' - ARINC identificator of the process (returned by CREATE_PROCESS).
 * 'data' - data for set.
 * 
 * Initially, binded data is NULL for every process created.
 */
void jet_set_process_data(long process_id, void* data);

/* 
 * Return data binded to the current process.
 */
void* jet_get_my_data(void);

#endif /* __LIBJET_PROCESS_H__ */
