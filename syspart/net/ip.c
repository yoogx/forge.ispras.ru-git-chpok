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

#include <net/ip.h>

uint16_t ip_hdr_checksum(const struct ip_hdr *ip_hdr)
{
    uint32_t acc = 0;

    // TODO does it break aliasing?
    const uint16_t *words = (const uint16_t*) ip_hdr;

    int i;
    for (i = 0; i < (ip_hdr->version_len & 0xf) * 2; i++) {
        acc += words[i];
    }

    return ~((acc & 0xFFFF) + (acc >> 16));
}
