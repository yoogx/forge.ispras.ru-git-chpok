/*
 * GENERATED! DO NOT MODIFY!
 *
 * Instead of modifying this file, modify the one it generated from (syspart/include/interfaces/afdx_net.yaml).
 */
#ifndef __INTERFACES_AFDX_QUEUE_ENQUEUER_H__
#define __INTERFACES_AFDX_QUEUE_ENQUEUER_H__

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


#include <lib/common.h>
    #include <ret_type.h>

typedef struct {
    ret_t (*afdx_add_to_queue)(self_t *, char *, size_t, size_t, size_t);
} afdx_queue_enqueuer;


#endif

