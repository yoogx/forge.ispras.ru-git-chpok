/*
 * Institute for System Programming of the Russian Academy of Sciences
 * Copyright (C) 2016 ISPRAS
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, Version 3.
 *
 * This program is distributed in the hope # that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License version 3 for more details.
 */

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

        printf("\t name='%s'\tstr_data='%s'\tlen=%lu\n", prop_name, (char *)prop->data, (unsigned long) prop->len);
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
    printf("devtree_address = 0x%lx\n", (unsigned long) devtree_address);

    struct boot_param_header *fdt = 
        (struct boot_param_header *) devtree_address;
    printf("devtree magic 0x%lx\n",          (unsigned long) fdt->magic);
    printf("devtree version %lu\n",          (unsigned long) fdt->version);
    printf("devtree totalsize %lu (0x%lx)\n",  (unsigned long) fdt->totalsize, 
                                             (unsigned long) fdt->totalsize);
    const char *cp = fdt_getprop(fdt, 0, "compatible", NULL);
    printf("comatible propertry of root: %s \n", cp);
    dummy_nodes_dump(fdt);
    printf("---------------------------------------\n");
}



static int get_pci_node(const void *fdt)
{
    return fdt_subnode_offset_namelen(fdt, 0, "pci", 3);
}


struct pci_bridge_props devtree_get_pci_props()
{
    struct pci_bridge_props bridge;
    int prop_len;

    const void *fdt = (const void *) devtree_address;

    int pci_node = get_pci_node(fdt);
    if (pci_node < 0)
        pok_fatal("No pci node in devtree");

    const int * bus_range = fdt_getprop(fdt, pci_node, "bus-range", &prop_len);
    if (bus_range == NULL || prop_len < (int)( 2*sizeof(int)))
        pok_fatal("reading 'bus_range' error");
    bridge.bus_startno = bus_range[0];
    bridge.bus_endno   = bus_range[1];

    int parent_address_len = *((int *) fdt_getprop(fdt, 0, "#address-cells", NULL));

    const int * reg = fdt_getprop(fdt, pci_node, "reg", NULL);
    if (parent_address_len != 2 || reg == NULL || reg[0] != 0) 
        pok_fatal("reading 'reg' error");
    bridge.cfg_addr = reg[1];
    bridge.cfg_data = reg[1] + 4;

    return bridge;
}
