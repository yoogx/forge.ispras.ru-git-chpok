#include "../../../arch/ppc/mmu.h"
struct tlb_entry {
    uint32_t virt_addr;
    uint64_t phys_addr;
    unsigned size;
    unsigned permissions;
    unsigned cache_policy;
    unsigned pid;
};

extern struct tlb_entry jet_tlb_entries[];
extern size_t jet_tlb_entries_n;
