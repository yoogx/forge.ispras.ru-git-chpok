
/*
 * Institute for System Programming of the Russian Academy of Sciences
 * Copyright (C) 2016 ISPRAS
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, Version 3.
 *
 * This program is distributed in the hope # that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License version 3 for more details.
 *
 * This file also incorporates work covered by POK License.
 * Copyright (c) 2007-2009 POK team
 */

#ifndef __POK_BSP_COMMON_H__
#define __POK_BSP_COMMON_H__


#include <config.h>

#include <types.h>
#include <errno.h>

#include <arch/bsp.h>

int pok_bsp_init(void);

pok_ret_t pok_bsp_irq_acknowledge (uint8_t irq);

pok_ret_t pok_bsp_irq_register (uint8_t irq,
				                    void    (*handler)(void));

void  *pok_bsp_mem_alloc (size_t size);

void  *pok_bsp_alloc_partition(size_t size);

void  *pok_bsp_mem_alloc_aligned(size_t size, size_t alignment);

pok_ret_t pok_bsp_time_init ();

void pok_bsp_get_info(void *addr);


pok_bool_t pok_cons_write (const char* s,
                       size_t length);
pok_bool_t pok_cons_write_1 (const char* s,
                       size_t length);


int data_to_read_0();
int read_serial_0();
int data_to_read_1();
int read_serial_1();


#ifdef POK_NEEDS_DEBUG
void pok_bsp_debug();
#endif


#endif /* !BSP_H_ */
