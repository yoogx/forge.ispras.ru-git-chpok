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


#ifndef __JET_LOADER_H__
#define __JET_LOADER_H__

#include <types.h>
#include <errno.h>
#include <core/space.h>

/**
 * Load elf into given space.
 * 
 * Entry point is returned via 'entry' parameter.
 * 
 * If 'heap_size' is not 0, allocates heap of given size and setup heap
 * in kernel_shared_data.
 */
void jet_loader_elf_load   (uint8_t elf_id,
                                 jet_space_id space_id,
                                 void (** entry)(void));
#endif /* __JET_LOADER_H__ */

