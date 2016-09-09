#ifndef __INTERFACES_SOME_INTERFACE_T_H__
#define __INTERFACES_SOME_INTERFACE_T_H__

#include <lib/common.h>

typedef struct {
    void (*tick)(self_t *);
} some_interface_t;


#endif

