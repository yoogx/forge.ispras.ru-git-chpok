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

#include <process.h>
#include <kernel_shared_data.h>

void jet_set_process_data(long process_id, void* data)
{
    int process_index = process_id - 1;
    
    kshd->tshd[process_index].private_data = data;
}

/* 
 * Return data binded to the current process.
 */
void* jet_get_my_data(void)
{
    int process_index = kshd->current_thread_id;
    
    return kshd->tshd[process_index].private_data;
}

