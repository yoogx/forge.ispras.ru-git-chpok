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

#ifndef __POK_NET_IP_H__
#define __POK_NET_IP_H__

#include <types.h>

#define IPPROTO_ICMP 1
#define IPPROTO_UDP 17

struct ip_hdr {
    uint8_t version_len;
    uint8_t dscp; 
    uint16_t length;
    uint16_t id; 
    uint16_t offset; 

    uint8_t ttl;
    uint8_t proto;
    uint16_t checksum;

    uint32_t src, dst;
    
    char options[];
} __attribute__((packed)); 

uint16_t ip_hdr_checksum(const struct ip_hdr *ip_hdr);

static inline void* ip_hdr_payload(struct ip_hdr *ip_hdr)
{
    char *p = (char *) ip_hdr;
    return (void*) p + (ip_hdr->version_len & 0xf) * 4;
}

#endif
