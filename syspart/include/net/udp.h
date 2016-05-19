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

#ifndef __POK_NET_UDP_H__
#define __POK_NET_UDP_H__

#include <types.h>

struct udp_hdr {
    uint16_t src_port, dst_port;
    uint16_t length;
    uint16_t checksum;

    char payload[];
} __attribute__((packed));

#endif
