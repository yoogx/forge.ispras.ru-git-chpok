#ifndef __POK_PPC_SPACE_H__
#define __POK_PPC_SPACE_H__

#include <types.h>
#include "interrupt_context.h"

typedef enum {
    PF_DATA_TLB_MISS,
    PF_INST_TLB_MISS,
    PF_DATA_STORAGE,
    PF_INST_STORAGE
} pf_type_t;


void pok_arch_handle_page_fault(
        struct jet_interrupt_context *vctx,
        uintptr_t faulting_address,
        uint32_t syndrome,
        pf_type_t type);

void pok_arch_space_init (void);


// various useful constants describing memory layout

#define KERNEL_STACK_SIZE 8192

#define POK_PARTITION_MEMORY_BASE 0x80000000ULL
#define POK_PARTITION_MEMORY_SIZE 0x1000000ULL 

#endif
