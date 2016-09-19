/*
 * GENERATED! DO NOT MODIFY!
 *
 * Instead of modifying this file, modify the one it generated from (syspart/include/interfaces/network.yaml).
 */
#ifndef __INTERFACES_SIMPLE_SEND_DATA_H__
#define __INTERFACES_SIMPLE_SEND_DATA_H__

#include <lib/common.h>
    #include <ret_type.h>

typedef struct {
    ret_t (*send)(self_t *, char *, size_t);
} simple_send_data;


#endif

