/*
 * GENERATED! DO NOT MODIFY!
 *
 * Instead of modifying this file, modify the one it generated from (syspart/include/interfaces/network.yaml).
 */
#ifndef __INTERFACES_UDP_MESSAGE_HANDLER_H__
#define __INTERFACES_UDP_MESSAGE_HANDLER_H__

#include <lib/common.h>
    #include <ret_type.h>

typedef struct {
    ret_t (*udp_message_handle)(self_t *, const char *, size_t, uint32_t, uint16_t);
} udp_message_handler;


#endif

