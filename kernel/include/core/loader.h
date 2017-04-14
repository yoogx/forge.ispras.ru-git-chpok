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
#include <core/partition_arinc.h>

/**
 * Load elf for given partition.
 * 
 * Entry point is returned via 'entry' parameter.
 */
void jet_loader_elf_load   (const void* elf_image,
                                 pok_partition_arinc_t* part,
                                 const struct memory_block* const* mblocks,
                                 void (** entry)(void));
#endif /* __JET_LOADER_H__ */

