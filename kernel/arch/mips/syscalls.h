#ifndef __POK_PPC_SYSCALLS_H__
#define __POK_PPC_SYSCALLS_H__

#include <types.h>

pok_ret_t pok_arch_sc_int(uint32_t num, uint32_t arg1, uint32_t arg2,
                          uint32_t arg3, uint32_t arg4, uint32_t arg5);
#endif
