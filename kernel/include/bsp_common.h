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
 * \file    include/bsp.h
 * \brief   Interfaces that BSP must provide
 * \author  Julian Pidancet
 * \date    2008-2009
 */

#ifndef __POK_BSP_COMMON_H__
#define __POK_BSP_COMMON_H__

#include <config.h>

#include <types.h>
#include <errno.h>


int pok_bsp_init();

pok_ret_t pok_bsp_irq_acknowledge (uint8_t irq);

pok_ret_t pok_bsp_irq_register (uint8_t irq,
				                    void    (*handler)(void));

void  *pok_bsp_mem_alloc (size_t size);

void  *pok_bsp_alloc_partition(size_t size);

void  *pok_bsp_mem_alloc_aligned(size_t size, size_t alignment);

pok_ret_t pok_bsp_time_init ();

bool_t pok_cons_write (const char* s,
                       size_t length);


int data_to_read();
int read_serial();


#ifdef POK_NEEDS_DEBUG
void pok_bsp_debug();
#endif


#ifdef __PPC__
#include <arch/ppc/bsp.h>
#endif

#ifdef __i386__
//#include <arch/x86/bsp.h>
#endif

#endif /* !BSP_H_ */
