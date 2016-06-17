#ifndef __POK_PPC_SPACE_H__
#define __POK_PPC_SPACE_H__

#include <types.h>
#include "thread.h"

typedef enum {
    PF_DATA_TLB_MISS,
    PF_INST_TLB_MISS,
    PF_DATA_STORAGE,
    PF_INST_STORAGE
} pf_type_t;


void pok_arch_handle_page_fault(
        volatile_context_t *vctx,
        uintptr_t faulting_address,
        uint32_t syndrome,
        pf_type_t type);

/* 
 * Offset of "large" part of the kernel from the "natural" one.
 * 
 * Set based on module's config.
 * (Currently is just 512M * cpu).
 */
extern unsigned long kernel_offset;

/* 
 * This function is called on CPU 0.
 * 
 * It should prepare memory for all modules and should start them.
 */
void ja_single_run(void);

void pok_arch_space_init (void);


// various useful constants describing memory layout

#define KERNEL_STACK_SIZE 8192

#define POK_PARTITION_MEMORY_BASE 0x80000000ULL
#define POK_PARTITION_MEMORY_SIZE 0x1000000ULL 

#endif
