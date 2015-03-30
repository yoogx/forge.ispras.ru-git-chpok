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
 * \file    arch/ppc/arch.c
 * \author  Tristan Gingold
 * \date    2009
 * \brief   Provide generic architecture access for PPC architecture
 */

#include <types.h>
#include <errno.h>
#include <core/partition.h>
#include "reg.h"
#include "msr.h"
#include "space.h"

extern void pok_arch_space_init (void);


pok_ret_t pok_arch_init ()
{
  mtmsr(MSR_IP);
#if POK_NEEDS_PARTITIONS
  pok_arch_space_init();
#endif

  return (POK_ERRNO_OK);
}

pok_ret_t pok_arch_preempt_disable()
{
  mtmsr(mfmsr() & ~MSR_EE);

  return (POK_ERRNO_OK);
}

pok_ret_t pok_arch_preempt_enable()
{
  mtmsr(mfmsr() | MSR_EE);

  return (POK_ERRNO_OK);
}

pok_bool_t pok_arch_preempt_enabled(void)
{
  return !!(mfmsr() & MSR_EE);
}

pok_ret_t pok_arch_idle()
{
   pok_arch_preempt_enable();

   while (1)
   {
   }

   return (POK_ERRNO_OK);	
}

pok_ret_t pok_arch_event_register (uint8_t vector, void (*handler)(void))
{
  (void) vector;
  (void) handler;

  return (POK_ERRNO_OK);
}


uint32_t    pok_thread_stack_addr   (const uint8_t    partition_id,
                                     const uint32_t   local_thread_id)
{
   (void) partition_id; 
   return POK_PARTITION_MEMORY_BASE + POK_PARTITION_MEMORY_SIZE - 16 - (local_thread_id * POK_USER_STACK_SIZE);
}


