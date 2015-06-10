/*
 * devtree - device tree
 * fdt - flatten device tree 
 */

#include <core/debug.h>
#include "devtree.h"
#include "libfdt/libfdt.h"

uint32_t devtree_address; //global

//TODO:move to 
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

/**
 * kbasename - return the last part of a pathname.
 *
 * @path: path to extract the filename from.
 */
static inline const char *kbasename(const char *path)
{
        const char *tail = strrchr(path, '/');
        return tail ? tail + 1 : path;
}


void scan_node_for_property(const void *fdt, 
               int offset)
{
    for (offset = fdt_first_property_offset(fdt, offset);
            (offset >= 0);
            (offset = fdt_next_property_offset(fdt, offset))) {
        const struct fdt_property *prop;

        if (!(prop = fdt_get_property_by_offset(fdt, offset, NULL))) {
            offset = -FDT_ERR_INTERNAL;
            break;
        }
        const char *prop_name = fdt_string(fdt, fdt32_to_cpu(prop->nameoff));
        //it(prop, prop_name);
        printf("\t\t%s=%s\n", prop_name, prop->data);

    }

}

int of_scan_flat_dt(const void *fdt)
{
    const char *pathp;
    int offset, rc = 0, depth = -1;

    printf("device tree:\n");
    for (offset = fdt_next_node(fdt, -1, &depth);
            offset >= 0 && depth >= 0 && !rc;
            offset = fdt_next_node(fdt, offset, &depth)) {

        pathp = fdt_get_name(fdt, offset, NULL);
        if (*pathp == '/')
            pathp = kbasename(pathp);

        //rc = it(offset, pathp, depth, data);
        printf("\toffset %d, name %s\n", offset, pathp);
        if (!strncmp(pathp, "pci", 3)) {
            scan_node_for_property(fdt, 
                    offset);
        }

    }
    return rc;
}

void devtree_handle()
{

    printf("---------------------------------------\n");
    printf("devtree_address = 0x%x\n",      devtree_address);
    struct boot_param_header *fdt = 
        (struct boot_param_header *) devtree_address;
    printf("devtree magic %x\n",            fdt->magic);
    printf("devtree version %x\n",          fdt->version);
    printf("devtree totalsize %d (0x%x)\n", fdt->totalsize, 
                                            fdt->totalsize);

    const char *cp = fdt_getprop(fdt, 0, "compatible", NULL);
    printf("comatible propertry of root: %s\n", cp);
    of_scan_flat_dt(fdt);

    printf("---------------------------------------\n");


}
