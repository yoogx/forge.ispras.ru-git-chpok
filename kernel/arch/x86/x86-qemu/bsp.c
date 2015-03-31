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


#include <errno.h>
#include <arch.h>

#include "cons.h"
#include "pm.h"
#include "pit.h"
#include "pic.h"

#include <assert.h>

#include <bsp.h>

#define ALIGN_UP(boundary, val) \
	(val + (boundary - 1)) & (~(boundary - 1))

pok_ret_t pok_bsp_init (void)
{
   pok_cons_init ();
   pok_pm_init ();
   pok_pic_init ();

   return (POK_ERRNO_OK);
}

pok_ret_t pok_bsp_irq_acknowledge (uint8_t irq)
{
   pok_pic_eoi (irq);

   return (POK_ERRNO_OK);
}

pok_ret_t pok_bsp_irq_register (uint8_t   irq,
                                void      (*handler)(void))
{
   pok_pic_unmask (irq);

   pok_arch_event_register (32 + irq, handler);

   return (POK_ERRNO_OK);
}

/**
 * Allocate data. At this time, the pok_pm_sbrk function
 * only increment size each time we allocate memory
 * and was not designed to free previously allocated
 * memory.
 */
void *pok_bsp_mem_alloc (size_t size)
{
   return ((void *)pok_pm_sbrk(size));
}

void  *pok_bsp_mem_alloc_aligned(size_t size, size_t alignment)
{
   uintptr_t original_brk = (uintptr_t) pok_pm_sbrk(0);
   uintptr_t aligned_brk = ALIGN_UP(alignment, original_brk); 
   uintptr_t new_brk = pok_pm_sbrk(size + (aligned_brk - original_brk));

   assert(original_brk == new_brk);
   assert(aligned_brk - new_brk < alignment);

   return (void *) aligned_brk;
}

void *pok_bsp_alloc_partition(size_t size)
{
    return pok_bsp_mem_alloc(size); 
}   

/**
 * Init time. \a freq is the frequency
 * of the oscillator.
 */
pok_ret_t pok_bsp_time_init ()
{
   return (pok_x86_qemu_timer_init ());
}

