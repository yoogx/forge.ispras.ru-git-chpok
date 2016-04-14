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


int pok_bsp_init(void);

pok_ret_t pok_bsp_irq_acknowledge (uint8_t irq);

pok_ret_t pok_bsp_irq_register (uint8_t irq,
				                    void    (*handler)(void));

void  *pok_bsp_mem_alloc (size_t size);

/**
 * Alloc (kernel) stack of specified size.
 * 
 * Return pointer to the head of the stack, that is when stack is assumed
 * to be empty.
 * 
 * DEV: Because stacks are allocated from global pool, shared between
 * partitions, all stack's allocation should be done at initialization
 * stage. Such way partitions couldn't affect on each other during work.
 */
static inline uint32_t pok_stack_alloc(uint32_t stack_size)
{
    void* addr = pok_bsp_mem_alloc(stack_size);
    return (uint32_t)addr + stack_size;
}


void  *pok_bsp_alloc_partition(size_t size);

void  *pok_bsp_mem_alloc_aligned(size_t size, size_t alignment);

pok_ret_t pok_bsp_time_init ();

void pok_bsp_get_info(void *addr);


bool_t pok_cons_write (const char* s,
                       size_t length);
bool_t pok_cons_write_1 (const char* s,
                       size_t length);


int data_to_read_0();
int read_serial_0();
int data_to_read_1();
int read_serial_1();


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
