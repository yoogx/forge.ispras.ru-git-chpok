#ifndef __INTERFACES_SEND_FLUSH_T_H__
#define __INTERFACES_SEND_FLUSH_T_H__

#include <lib/common.h>

typedef struct {
    void (*send)(self_t *, void *, size_t);
    void (*flush)(self_t *);
} send_flush_t;


#endif

