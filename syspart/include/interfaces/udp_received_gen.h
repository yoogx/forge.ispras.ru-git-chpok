/*
 * GENERATED! DO NOT MODIFY!
 *
 * Instead of modifying this file, modify the one it generated from (syspart/include/interfaces/network.yaml).
 */
#ifndef __INTERFACES_UDP_RECEIVED_H__
#define __INTERFACES_UDP_RECEIVED_H__

#include <lib/common.h>
    #include <ret_type.h>

typedef struct {
    ret_t (*udp_received)(self_t *, char *, size_t, uint32_t, uint16_t);
} udp_received;


#endif

