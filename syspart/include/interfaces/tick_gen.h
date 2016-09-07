#ifndef __INTERFACES_TICK_H__
#define __INTERFACES_TICK_H__

#include <lib/common.h>

    typedef struct {
        void (*tick)(self_t *);
    } tick_t;

    typedef struct {
        void (*tick)(self_t *);
    } some_interface_t;


#endif

