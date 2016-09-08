#ifndef __INTERFACES_TICK_T_H__
#define __INTERFACES_TICK_T_H__

#include <lib/common.h>

typedef struct {
    void (*tick)(self_t *);
} tick_t;


#endif

