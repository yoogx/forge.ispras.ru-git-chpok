/*
 * GENERATED! DO NOT MODIFY!
 *
 * Instead of modifying this file, modify the one it generated from (syspart/include/interfaces/port_example.yaml).
 */
#ifndef __INTERFACES_PORT_EXAMPLE_H__
#define __INTERFACES_PORT_EXAMPLE_H__

#include <lib/common.h>

typedef struct {
    void (*send)(self_t *, void *, size_t);
    void (*flush)(self_t *);
} port_example;


#endif

