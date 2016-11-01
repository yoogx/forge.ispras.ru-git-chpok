/*
 * GENERATED! DO NOT MODIFY!
 *
 * Instead of modifying this file, modify the one it generated from (syspart/include/interfaces/network.yaml).
 */
#ifndef __INTERFACES_ETHERNET_PACKET_SENDER_H__
#define __INTERFACES_ETHERNET_PACKET_SENDER_H__

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
    #include <net/ether.h>

typedef struct {
    ret_t (*mac_send)(self_t *, char *, size_t, size_t, uint8_t *, enum ethertype);
    ret_t (*flush)(self_t *);
} ethernet_packet_sender;


#endif

