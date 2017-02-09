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

#ifndef __POK_NET_ETHER_H__
#define __POK_NET_ETHER_H__

#include <types.h>

#define ETH_ALEN 6
#define ETH_DATA_LENGTH 1500

enum ethertype {
    ETH_P_IP = 0x0800,
    ETH_P_ARP = 0x0806
};

struct ether_hdr {
    uint8_t dst[ETH_ALEN];
    uint8_t src[ETH_ALEN];
    uint16_t ethertype;
    char payload[];
} __attribute__((packed));

static inline int ether_is_multicast(const uint8_t addr[ETH_ALEN])
{
    return (addr[0] & 0x01) != 0;
}

#endif
