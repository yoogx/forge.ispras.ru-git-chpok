/*
 * GENERATED! DO NOT MODIFY!
 *
 * Instead of modifying this file, modify the one it generated from (syspart/include/interfaces/network.yaml).
 */
#ifndef __INTERFACES_PREALLOCATED_SENDER_H__
#define __INTERFACES_PREALLOCATED_SENDER_H__

#include <lib/common.h>
    #include <ret_type.h>

typedef struct {
    ret_t (*send)(self_t *, char *, size_t, size_t);
    ret_t (*flush)(self_t *);
} preallocated_sender;


#endif

