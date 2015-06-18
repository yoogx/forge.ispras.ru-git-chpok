/*
 *                               POK header
 * 
 * The following file is a part of the POK project. Any modification should
 * made according to the POK licence. You CANNOT use this file or a part of
 * this file is this part of a file for your own project
 *
 * For more information on the POK licence, please see our LICENCE FILE
 *
 * Please follow the coding guidelines described in doc/CODING_GUIDELINES
 *
 *                                      Copyright (c) 2007-2009 POK team 
 *
 * Created by laurent on Mon Jun 08 11:01:12 2009 
 */

#ifdef POK_NEEDS_PCI

# include <errno.h>
# include <libc.h>
# include <pci.h>
# include <ioports.h>
# include "devtree.h"
# include <core/debug.h>

struct pci_bridge_props bridge_props;

void pok_pci_init()
{
    bridge_props = devtree_get_pci_props();

    printf("bridge\n\tcfg_addr: 0x%x\n\tcfg_data: 0x%x\n",
            bridge_props.cfg_addr, bridge_props.cfg_data);
}

unsigned int pci_read(unsigned int bus,
        unsigned int dev,
        unsigned int fun,
        unsigned int reg)
{
    unsigned int addr = (1 << 31) | (bus << 16) | (dev << 11) | (fun << 8) | (reg & 0xfc);
    unsigned int val = -1;

    out_be32((uint32_t *) bridge_props.cfg_addr, addr);
    val = in_le32((uint32_t *) bridge_props.cfg_data);

    return (val >> ((reg & 3) << 3));
}

unsigned int pci_read_reg(s_pci_device* d,
        unsigned int reg)
{
    return (pci_read(d->bus, d->dev, d->fun, reg));
}

void pci_write_word(s_pci_device *d, uint32_t reg, uint16_t val)
{
    uint32_t addr = (1 << 31) | (d->bus << 16) | (d->dev << 11) | (d->fun << 8) | (reg & 0xfc);

    out_be32((uint32_t *) bridge_props.cfg_addr, addr);
    out_le16((uint16_t *) bridge_props.cfg_data, val);
}

//TODO: this func is unused in virtio. Should be deleted?
pok_ret_t pci_register(__attribute__ ((unused)) s_pci_device* dev)
{
    pok_fatal("pci_register called");
    return 0;
}

#endif /* POK_NEEDS_PCI */
