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

#ifndef __POK_NET_ETHER_H__
#define __POK_NET_ETHER_H__

#include <types.h>

#define ETH_ALEN 6
#define ETH_DATA_LENGTH 1500

#define ETH_P_IP 0x0800

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
