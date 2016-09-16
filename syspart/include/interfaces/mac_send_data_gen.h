/*
 * GENERATED! DO NOT MODIFY!
 *
 * Instead of modifying this file, modify the one it generated from (syspart/include/interfaces/send_net.yaml).
 */
#ifndef __INTERFACES_MAC_SEND_DATA_H__
#define __INTERFACES_MAC_SEND_DATA_H__

#include <lib/common.h>
    #include <ret_type.h>
    #include <net/ether.h>

typedef struct {
    ret_t (*mac_send)(self_t *, char *, size_t, size_t, uint8_t *, enum ethertype);
    ret_t (*flush)(self_t *);
} mac_send_data;


#endif

