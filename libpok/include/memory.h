#ifndef __POK_MEMORY_H__
#define __POK_MEMORY_H__

#include <core/syscall.h>

static inline uintptr_t pok_virt_to_phys(void * virt) {
   return pok_syscall1(POK_SYSCALL_MEM_VIRT_TO_PHYS, (uintptr_t) virt);
}



#endif

