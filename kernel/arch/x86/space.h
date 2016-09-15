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

/**
 * \file    arch/x86/space.h
 * \author  Julian Pidancet
 * \date    2008-2009
 */

#ifndef __POK_X86_SPACE_H__
#define __POK_X86_SPACE_H__

#include <types.h>
#include "thread.h"
#include <arch/deployment.h>

/* 
 * Virtual address where partition's memory starts.
 * 
 * DEV: Segment addressing cannot affect virtual addresses: they always starts from 0.
 */
#define POK_PARTITION_MEMORY_BASE 0x0ULL
/*
 * Beginning of the phys memory used for partitions.
 */
#define POK_PARTITION_MEMORY_PHYS_START 0x1000000ULL

void ja_space_init(void);

#endif /* !__POK_X86_SPACE_H__ */

