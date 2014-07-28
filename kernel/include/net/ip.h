/*  
 *  Copyright (C) 2014 Maxim Malkov, ISPRAS <malkov@ispras.ru> 
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __POK_NET_IP_H__
#define __POK_NET_IP_H__

#include <types.h>

#define IPPROTO_ICMP 1
#define IPPROTO_UDP 17

struct ip_hdr {
    // FIXME support big endian targets, too
    unsigned int header_length:4;
    unsigned int version:4;

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
    return (void*) p + ip_hdr->header_length * 4;
}

#endif
