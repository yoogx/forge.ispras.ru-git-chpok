#ifndef __POK_IOPORTS_H__
#define __POK_IOPORTS_H__

extern uint32_t devtree_address; 
void devtree_handle();

struct boot_param_header {
    uint32_t  magic;                  /* magic word OF_DT_HEADER */
    uint32_t  totalsize;              /* total size of DT block */
    uint32_t  off_dt_struct;          /* offset to structure */
    uint32_t  off_dt_strings;         /* offset to strings */
    uint32_t  off_mem_rsvmap;         /* offset to memory reserve map */
    uint32_t  version;                /* format version */
    uint32_t  last_comp_version;      /* last compatible version */
    /* version 2 fields below */
    uint32_t  boot_cpuid_phys;        /* Physical CPU id we're booting on */
    /* version 3 fields below */
    uint32_t  dt_strings_size;        /* size of the DT strings block */
    /* version 17 fields below */
    uint32_t  dt_struct_size;         /* size of the DT structure block */
};

#endif
