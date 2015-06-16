/*
 * devtree - device tree
 * fdt - flatten device tree 
 */

#include <core/debug.h>
#include "devtree.h"
#include "libfdt/libfdt.h"
#include <ioports.h>

uint32_t devtree_address; //global

void hexdump (const void *addr, int len)
{
    int i;
    unsigned char buff[17];
    const unsigned char *pc = (const unsigned char*)addr;

    if (len == 0) {
        printf("Len is zero\n");
        return;
    }

    // Process every byte in the data.
    for (i = 0; i < len; i++) {
        // Multiple of 16 means new line (with line offset).

        if ((i % 16) == 0) {
            // Just don't print ASCII for the zeroth line.
            if (i != 0)
                printf("  %s\n", buff);

            // Output the offset.
            printf("  %x ", i);
        }

        // Now the hex code for the specific character.
        printf(" %x", pc[i]);

        // And store a printable ASCII character for later.
        if ((pc[i] < 0x20) || (pc[i] > 0x7e))
            buff[i % 16] = '.';
        else
            buff[i % 16] = pc[i];
        buff[(i % 16) + 1] = '\0';
    }

    // Pad out last line if not exactly 16 characters.
    while ((i % 16) != 0) {
        printf("   ");
        i++;
    }

    // And print the final ASCII bit.
    printf("  %s\n", buff);
}

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
    uint32_t cfg_addr, cfg_data;

    for (offset = fdt_first_property_offset(fdt, offset);
            (offset >= 0);
            (offset = fdt_next_property_offset(fdt, offset))) {
        const struct fdt_property *prop;

        if (!(prop = fdt_get_property_by_offset(fdt, offset, NULL))) {
            break;
        }
        const char *prop_name = fdt_string(fdt, fdt32_to_cpu(prop->nameoff));

        //printf("\t\t%s: data len %d\n", prop_name, prop->len);

        if (!strcmp(prop_name, "bus-range")) {
            int *bus_range = (int *)prop->data;
            if (bus_range == NULL || prop->len < 2 * sizeof(int)) {
                printf("reading bus_range error\n");
                continue;
            }
            printf("\t\t\t0x%x, 0x%x\n", bus_range[0], bus_range[1]);

        } else if (!strcmp(prop_name, "reg")) {

            uint32_t *regs = (uint32_t *)prop->data;

            //TODO:assume curently assume that parents' #address-cells = 2. Should be generalized
            if (regs[0] != 0) {
                printf("reg is not 32 bit's");
                continue;
            }
            cfg_addr = regs[1];
            cfg_data = regs[1] + 4;

            printf("PCI host bridge at 0x%x ", cfg_addr);

            outl(cfg_addr, 0x80000000);
            uint32_t device_and_vendor_ids = inl(cfg_data);
            uint8_t *dev_ven_ids = (uint8_t *) & device_and_vendor_ids;

            printf("\t%x%x:%x%x\n", dev_ven_ids[1], dev_ven_ids[0], dev_ven_ids[3], dev_ven_ids[2]);

            {
                outl(cfg_addr, 0x80000000|1 << 11);
                uint32_t device_and_vendor_ids = inl(cfg_data);
                uint8_t *dev_ven_ids = (uint8_t *) & device_and_vendor_ids;

                printf("\t%x%x:%x%x\n", dev_ven_ids[1], dev_ven_ids[0], dev_ven_ids[3], dev_ven_ids[2]);
            }
        }


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
        //printf("\toffset %d, name %s\n", offset, pathp);
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
    printf("devtree magic 0x%x\n",            fdt->magic);
    printf("devtree version %d\n",          fdt->version);
    printf("devtree totalsize %d (0x%x)\n", fdt->totalsize, 
                                            fdt->totalsize);

    const char *cp = fdt_getprop(fdt, 0, "compatible", NULL);
    printf("comatible propertry of root: %s\n", cp);
    of_scan_flat_dt(fdt);

    printf("---------------------------------------\n");
}
