/*
 * devtree - device tree
 * fdt - flatten device tree 
 */

#include <core/debug.h>
#include "devtree.h"
#include "libfdt/libfdt.h"
#include <ioports.h>

uint32_t devtree_address; //global


static void dummy_props_dump(const void *fdt, int offset)
{       
    for (offset = fdt_first_property_offset(fdt, offset);
            (offset >= 0);
            (offset = fdt_next_property_offset(fdt, offset))) {
        const struct fdt_property *prop;

        if (!(prop = fdt_get_property_by_offset(fdt, offset, NULL))) {
            break;
        }
        const char *prop_name = fdt_string(fdt, fdt32_to_cpu(prop->nameoff));

        printf("\t name='%s'\tstr_data='%s'\tlen=%d\n", prop_name, (char *)prop->data, prop->len);
    }
}

static void dummy_nodes_dump(const void *fdt)
{
    const char *pathp;
    int offset, depth = -1;

    for (offset = fdt_next_node(fdt, -1, &depth);
            offset >= 0 && depth >= 0;
            offset = fdt_next_node(fdt, offset, &depth)) {

        pathp = fdt_get_name(fdt, offset, NULL);

        printf("offset %d, name %s\n", offset, pathp);

        if (!strncmp(pathp, "pci", 3)) {
            dummy_props_dump(fdt, offset);
        }
    }
}

/* 
 * Instead of this dummy dump you could use "smart" dump by qemu or u-boot.
 * Qemu: add "-machine dumpdtb='dump.dtb'" to qemu launch command. 
 * U-boot: TBD
 * Then run "fdtdump dump.dtb". You can get fdtdump from dtc 
 */
void devtree_dummy_dump()
{
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

    printf("---------------------------------------\n");
    printf("devtree_address = 0x%x\n",      devtree_address);

    struct boot_param_header *fdt = 
        (struct boot_param_header *) devtree_address;
    printf("devtree magic 0x%x\n",            fdt->magic);
    printf("devtree version %d\n",          fdt->version);
    printf("devtree totalsize %d (0x%x)\n", fdt->totalsize, 
                                            fdt->totalsize);
    const char *cp = fdt_getprop(fdt, 0, "compatible", NULL);
    printf("comatible propertry of root: %s \n", cp);
    dummy_nodes_dump(fdt);
    printf("---------------------------------------\n");
}



static int get_pci_node(const void *fdt)
{
    return fdt_subnode_offset_namelen(fdt, 0, "pci", 3);
}


//struct pci_props 
//
void devtree_get_pci_props()
{
    const void *fdt = (const void *) devtree_address;
    int pci_node = get_pci_node(fdt);
    if (pci_node < 0) {
        printf("no pci node in devtree");
    }
    int len;
    const void * tmp = fdt_getprop(fdt, pci_node, "compatible", &len);
    printf("------===========> %s", (const char *)tmp);

//        if (!strcmp(prop_name, "bus-range")) {
            /*
            int *bus_range = (int *)prop->data;
            if (bus_range == NULL || prop->len < 2 * sizeof(int)) {
                printf("reading bus_range error\n");
                continue;
            }
            printf("\t\t\t0x%x, 0x%x\n", bus_range[0], bus_range[1]);
            */

#if 0

            static uint32_t bridge_cfg_addr, bridge_cfg_data;
            if (!strcmp(prop_name, "reg")) {

            uint32_t *regs = (uint32_t *)prop->data;

            //TODO:assume curently assume that parents' #address-cells = 2. Should be generalized
            if (regs[0] != 0) {
                printf("reg is not 32 bit's");
                continue;
            }
            bridge_cfg_addr = regs[1];
            bridge_cfg_data = regs[1] + 4;

            printf("PCI host bridge at 0x%x ", bridge_cfg_addr);

            outl(bridge_cfg_addr, 0x80000000);
            uint32_t device_and_vendor_ids = inl(bridge_cfg_data);
            uint8_t *dev_ven_ids = (uint8_t *) & device_and_vendor_ids;

            printf("\t%x%x:%x%x\n", dev_ven_ids[1], dev_ven_ids[0], dev_ven_ids[3], dev_ven_ids[2]);

            {
                outl(bridge_cfg_addr, 0x80000000|1 << 11);
                uint32_t device_and_vendor_ids = inl(bridge_cfg_data);
                uint8_t *dev_ven_ids = (uint8_t *) & device_and_vendor_ids;

                printf("\t%x%x:%x%x\n", dev_ven_ids[1], dev_ven_ids[0], dev_ven_ids[3], dev_ven_ids[2]);
            }
        }
#endif
}
