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
 * Created by julien on Wed Jan 21 23:07:15 2009
 */


#ifndef __POK_MEMORY_BLOCKS_H__
#define __POK_MEMORY_BLOCKS_H__

#include <uapi/types.h>
#define TMP__MAX_PARTITON 32
struct memory_block {
    char name[MAX_NAME_LENGTH];
    uint32_t virt_addr;
    unsigned size;
    unsigned avail_for_pids[TMP__MAX_PARTITON];
};
#endif
