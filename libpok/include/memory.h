#ifndef __POK_MEMORY_H__
#define __POK_MEMORY_H__

#include <core/syscall.h>

static inline uintptr_t pok_virt_to_phys(void * virt) {
   return pok_syscall1(POK_SYSCALL_MEM_VIRT_TO_PHYS, (uintptr_t) virt);
}

static inline void* pok_phys_to_virt(uintptr_t phys) {
   return (void *) pok_syscall1(POK_SYSCALL_MEM_PHYS_TO_VIRT, phys);
}



#endif

