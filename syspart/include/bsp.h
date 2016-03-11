
#ifndef __SYSPART_BSP_H__
#define __SYSPART_BSP_H__
#include <types.h>
#include <core/syscall.h>

struct pci_bridge {
    uint32_t cfg_addr;
    uint32_t cfg_data;
    uint32_t iorange;
};

typedef struct {
    uint32_t ccsrbar_size;
    uint64_t ccsrbar_base;
    uint64_t ccsrbar_base_phys;
    uint32_t dcfg_offset;
    uint32_t serial0_regs_offset;
    uint32_t timebase_freq;
    struct pci_bridge pci_bridge;
} pok_bsp_t;

void pok_bsp_get_info(pok_bsp_t *addr) {
   pok_syscall1(POK_SYSCALL_GET_BSP_INFO, (uint32_t) addr);
}
#endif
