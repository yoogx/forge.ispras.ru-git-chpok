/*
 *                               POK header
 * 
 * The following file is a part of the POK project. Any modification should
 * made according to the POK licence. You CANNOT use this file or a part of
 * this file is this part of a file for your own project
 *
 * For more information on the POK licence, please see our LICENCE FILE
 *
 * Please follow the coding guidelines described in doc/CODING_GUIDELINES
 *
 *                                      Copyright (c) 2007-2009 POK team 
 *
 * Created by julien on Thu Jan 15 23:34:13 2009 
 */

#ifndef __POK_IOPORTS_H__
#define __POK_IOPORTS_H__

#include <inttypes.h>

static inline void outb(uintptr_t port, unsigned char data)
{
    // TODO rewrite in assembly
    volatile unsigned char *p = (volatile unsigned char *) port;
    *p = data;
}

static inline unsigned char inb(uintptr_t port)
{
    // TODO rewrite in assembly
    volatile unsigned char *p = (volatile unsigned char *) port;
    return *p;
}


#endif /* __POK_IOPORTS_H__ */
