/*
 * devtree - device tree
 * fdt - flatten device tree 
 */

#include <core/debug.h>
#include "devtree.h"
#include "libfdt/libfdt.h"

uint32_t devtree_address; //global

#define FDT_BUF_SIZE 0x10000
//static char fdt_buffer[FDT_BUF_SIZE]; 
//struct boot_param_header * devtree = (struct boot_param_header *) fdt_buffer;

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
               int offset,
               int (*it) (const struct fdt_property *prop,
                          const char *prop_name))
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
        it(prop, prop_name);

    }

}

/* I use dummy for simplify analyze by gdb. Now I can just set breakpoit on dummy*/
int dummy_prop_it (
        __attribute__((unused)) const struct fdt_property *prop,
        __attribute__((unused)) const char *prop_name)
{
    return 0;
}


int of_scan_flat_dt(const void *fdt,
                    int (*it) (unsigned long node,
                               const char *uname, int depth,
                               void *data),
                    void *data)
{
    const char *pathp;
    int offset, rc = 0, depth = -1;

    for (offset = fdt_next_node(fdt, -1, &depth);
            offset >= 0 && depth >= 0 && !rc;
            offset = fdt_next_node(fdt, offset, &depth)) {

        pathp = fdt_get_name(fdt, offset, NULL);
        if (*pathp == '/')
            pathp = kbasename(pathp);

        rc = it(offset, pathp, depth, data);
        if (!strncmp(pathp, "pci", 3)) {
            scan_node_for_property(fdt, 
                    offset, 
                    dummy_prop_it);
        }

    }
    return rc;
}

int dummy_it (
        __attribute__((unused)) unsigned long node,
        __attribute__((unused)) const char *uname, 
        __attribute__((unused)) int depth,
        __attribute__((unused)) void *data)
{
    return 0;
}

int of_fdt_is_compatible(const void *blob,
                      unsigned long node, const char *compat)
{
        const char *cp;
        int cplen;
        unsigned long l, score = 0;

        cp = fdt_getprop(blob, node, "compatible", &cplen);
        if (cp == NULL)
                return 0;

        if (cp[1] == 'o')
            return 5;
        //----------
        if (compat == NULL){
                l = 0;
                l++;
                score = 0;
                score++;
                return 0;
        }
        //-----------
        /*
        while (cplen > 0) {
                score++;
                if (of_compat_cmp(cp, compat, strlen(compat)) == 0)
                        return score;
                l = strlen(cp) + 1;
                cp += l;
                cplen -= l;
        }
        */

        return 0;
}


/*XXX: remember console has not initialized yet */
void devtree_load()
{
    if (devtree_address == 0)
        return;
    struct boot_param_header *devtree = (struct boot_param_header *) devtree_address;
    
    of_fdt_is_compatible(devtree, 0, "test_input_str");

    of_scan_flat_dt(devtree, dummy_it, NULL);




/*
    uint32_t tsize = ((struct boot_param_header *) devtree_address)->totalsize;

    if (tsize <= FDT_BUF_SIZE)
        memcpy((void *)fdt_buffer, (void *) devtree_address, tsize);
    else
        pok_fatal("FDT_BUF_SIZE is too small");
        */

//    devtree  = *( (struct boot_param_header *) devtree_address );
}


void devtree_handle()
{
    return;
    /*
    printf("---------------------------------------\n");
    printf("devtree_address = 0x%x\n",      devtree_address);
    printf("devtree magic %x\n",            devtree->magic);
    printf("devtree version %x\n",          devtree->version);
    printf("devtree totalsize %d (0x%x)\n", devtree->totalsize, 
                                            devtree->totalsize);
    printf("---------------------------------------\n");
    */

}
