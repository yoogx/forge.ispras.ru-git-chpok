
#include <pci.h>
#include <ioports.h>
#include <stdio.h>

#include "pci_internal.h"

extern struct pci_driver ne2k_driver;
extern struct pci_driver virtio_pci_driver;

struct pci_driver *pci_driver_table[] = {
    //&ne2k_driver,
    &virtio_pci_driver,
};

struct pci_bridge bridge;

static char *get_pci_class_name(int classcode) {
    int i = 0;
    while (pci_classnames[i].name != NULL) {
        if (pci_classnames[i].classcode == (classcode & 0xFFFF00))
            return pci_classnames[i].name;
        i++;
    }
    return "Unknown";
}

int pci_match_device(const struct pci_device_id *id, const struct pci_device *dev)
{
    if ((id->vendor == PCI_ANY_ID || id->vendor == dev->vendorid) &&
            (id->device == PCI_ANY_ID || id->device == dev->deviceid))
        return 1;
    return 0;
}

unsigned int pci_read(
        unsigned int bus,
        unsigned int dev,
        unsigned int fun,
        unsigned int reg)
{
    unsigned int addr = (1 << 31) | (bus << 16) | (dev << 11) | 
        (fun << 8) | (reg & 0xfc);
    unsigned int val = -1;

#ifdef __PPC__
    out_be32((uint32_t *) bridge.cfg_addr, addr);
    val = in_le32((uint32_t *) bridge.cfg_data);
#else
    outl(PCI_CONFIG_ADDRESS, addr);
    val = inl(PCI_CONFIG_DATA);
#endif

    return (val >> ((reg & 3) << 3));
}


void pci_write_word(s_pci_device *d, uint32_t reg, uint16_t val)
{
    uint32_t addr = (1 << 31) | (d->bus << 16) | (d->dev << 11) | 
        (d->fun << 8) | (reg & 0xfc);

#ifdef __PPC__
    out_be32((uint32_t *) bridge.cfg_addr, addr);
    out_le16((uint16_t *) bridge.cfg_data, val);
#else
    outl(PCI_CONFIG_ADDRESS, addr);
    outw(PCI_CONFIG_DATA, val);
#endif

}

void pci_list()
{
//#ifdef __PPC__
//    {
//        //TODO call pci write word
//        out_be32(bridge.cfg_addr, 0x80000810); //write something to BAR0
//        out_le16(bridge.cfg_data, 0x1001);
//
//        out_be32(bridge.cfg_addr, 0x80000804);//write something to COMMAND register
//        out_le16(bridge.cfg_data, 0x7);
//    }
//#endif

    for (unsigned int bus = 0; bus < PCI_BUS_MAX; bus++)
      for (unsigned int dev = 0; dev < PCI_DEV_MAX; dev++)
        for (unsigned int fun = 0; fun < PCI_FUN_MAX; fun++)
        {
            uint16_t vendor = (uint16_t) pci_read(bus, dev, fun, PCI_REG_VENDORID);
            uint16_t device = (uint16_t) pci_read(bus, dev, fun, PCI_REG_DEVICEID);

            if (vendor != 0xFFFF) {
                uint32_t classcode = 0xFFFFFF & pci_read(bus, dev, fun, PCI_REG_PROGIFID);
                printf("%02x:%02x:%02x ", bus, dev, fun);
                printf("%s:\n", get_pci_class_name(classcode));
                printf("\t PCI device %04x:%04x (header 0x%02x)\n",
                        vendor,
                        device,
                        0xFF & pci_read(bus, dev, fun, PCI_REG_HEADERTYPE));

                uint32_t bar0 = pci_read(bus, dev, fun, PCI_REG_BAR0);
                if (bar0) {
                    printf("\t BAR0: 0x%lx\n", bar0);
                }

                /*
                if (classcode == 0x20000) {
                    usigned addr = bar0;
                    printf("MAC addr = ");
                }
                */
            }
        }
}


void pci_init()
{

    //TODO make syscall
    //bridge = devtree_get_pci_props();
//#ifdef __PPC__
    bridge.cfg_addr = (uint32_t*) 0xe0008000;
    bridge.cfg_data = (void *) 0xe0008004;
//#endif

    printf("\n***********************************************\n");
    printf("PCI initializing\n");
    printf("bridge cfg_addr: %p cfg_data: %p\n",
            bridge.cfg_addr, bridge.cfg_data);
    pci_list();
    printf("\n***********************************************\n\n");


    for (unsigned int bus = 0; bus < PCI_BUS_MAX; bus++)
      for (unsigned int dev = 0; dev < PCI_DEV_MAX; dev++)
      {
            int fun = 0;
            // Currently we do not handle type 1 or 2 PCI configuration spaces
            if ((pci_read(bus, dev, fun, PCI_REG_HEADERTYPE) & 0xFF) != 0)
                continue;
            struct pci_device pci_dev;
            pci_dev.bus = bus;
            pci_dev.dev = dev;
            pci_dev.fun = fun;
            pci_dev.vendorid  = (uint16_t) pci_read(bus, dev, fun, PCI_REG_VENDORID);
            pci_dev.deviceid  = (uint16_t) pci_read(bus, dev, fun, PCI_REG_DEVICEID);
            //pci_dev.classcode = (uint16_t) pci_read(bus, dev, fun, PCI_REG_PROGIFID);

            //for (pci_driver in pci_driver_table) {
            //    if pci_match(&pci_driver, &pci_dev) {
             {{
                  struct pci_driver *pci_driver = pci_driver_table[0];
#ifdef __PPC__
                    {
                        //TODO call pci write word
                        out_be32(bridge.cfg_addr, 0x80000810); //write something to BAR0
                        out_le16(bridge.cfg_data, 0x1001);

                        out_be32(bridge.cfg_addr, 0x80000804);//write something to COMMAND register
                        out_le16(bridge.cfg_data, 0x7);
                    }
                    pci_dev.bar[0] = 0xe1001000;
#else
                    pci_dev.bar[0] = pci_read(bus, dev, fun, PCI_REG_BAR0);
#endif
                    //uint32_t bar0 = pci_read(bus, dev, fun, PCI_REG_BAR0);
                    //dev init;
                    pci_driver->probe(&pci_dev);
                }
            }
        }
}
