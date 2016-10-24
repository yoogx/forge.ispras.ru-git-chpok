/*
 * GENERATED! DO NOT MODIFY!
 *
 * Instead of modifying this file, modify the one it generated from (syspart/include/interfaces/port_example.yaml).
 */
#ifndef __INTERFACES_PORT_EXAMPLE_H__
#define __INTERFACES_PORT_EXAMPLE_H__

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

typedef struct {
    void (*send)(self_t *, void *, size_t);
    void (*flush)(self_t *);
} port_example;


#endif

