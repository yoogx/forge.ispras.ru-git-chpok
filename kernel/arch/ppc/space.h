#ifndef __POK_PPC_SPACE_H__
#define __POK_PPC_SPACE_H__

// various useful constants describing memory layout

#define KERNEL_STACK_SIZE 8192

#define POK_PARTITION_MEMORY_BASE 0x80000000ULL
#define POK_PARTITION_MEMORY_SIZE 0x1000000ULL 

#define CCSRBAR_BASE 0xE0000000ULL
#define CCSRBAR_SIZE 0x1000000ULL

#endif
