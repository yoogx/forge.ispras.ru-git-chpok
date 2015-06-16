#ifndef __POK_DEVTREE_H__
#define __POK_DEVTREE_H__

extern uint32_t devtree_address; 
void devtree_dummy_dump();

struct pci_bridge_props {
    uint32_t bus_startno;
    uint32_t bus_endno;
    uint32_t cfg_addr;
    uint32_t cfg_data;
};

struct pci_bridge_props devtree_get_pci_props();


#endif
