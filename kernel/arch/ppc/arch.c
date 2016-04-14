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

#include <config.h>

#include <types.h>
#include <errno.h>
#include <core/partition.h>
#include "reg.h"
#include "msr.h"
#include "space.h"
#include "devtree.h"
#include <bsp_common.h>

pok_ret_t pok_arch_init ()
{
  mtmsr(MSR_IP | MSR_FP);

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

void pok_arch_inf_loop()
{
   pok_arch_preempt_disable();

   while (1)
   {}
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


uint32_t    pok_thread_stack_addr   (uint8_t    space_id,
                                     uint32_t stack_size,
                                     uint32_t* state)
{
   uint32_t result = POK_PARTITION_MEMORY_BASE + POK_PARTITION_MEMORY_SIZE - 16 - (*state);
   // TODO: Check boundaries.
   (void) space_id;
   *state += stack_size;
   return result;
}


#include <arch/ppc/linux_io.h>
#define DCFG_RSTCR 0xb0
#define RSTCR_RESET_REQ 0x2
void pok_arch_cpu_reset()
{
    uintptr_t addr = pok_bsp.ccsrbar_base + pok_bsp.dcfg_offset + DCFG_RSTCR;
    out_be32((void*)addr, RSTCR_RESET_REQ);
}
