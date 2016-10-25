#include <types.h>

struct TLB_entry{
        uint64_t virtual;
        uint64_t physical; 
        unsigned pgsize_enum; 
        unsigned permissions;
        unsigned wimge;
        unsigned pid;
        pok_bool_t is_resident;
    
    };
    
extern struct TLB_entry TLB_entries[];
extern int number_TLB_entry;

extern struct TLB_entry TLB_entries1[];
extern int number_TLB_entry1;
