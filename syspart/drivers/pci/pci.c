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
 *
 * This file also incorporates work covered by POK License.
 * Copyright (c) 2007-2009 POK team
 */

#include <pci.h>
#include <ioports.h>
#include <stdio.h>

#include <bsp.h>
#include "pci_internal.h"
#include <depl.h>

unsigned pci_driver_table_used_cnt = 0;

#ifdef __PPC__
struct pci_bridge bridge;
#endif

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

void register_pci_driver(struct pci_driver *driver)
{
    pci_driver_table[pci_driver_table_used_cnt++] = *driver;
}

uint32_t pci_read(
        uint32_t bus,
        uint32_t dev,
        uint32_t fun,
        uint32_t reg)
{
    uint32_t addr = (1 << 31) | (bus << 16) | (dev << 11) | 
        (fun << 8) | (reg & 0xfc);
    uint32_t val = -1;

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
void pci_write_dword(s_pci_device *d, uint32_t reg, uint32_t val)
{
    uint32_t addr = (1 << 31) | (d->bus << 16) | (d->dev << 11) | 
        (d->fun << 8) | (reg & 0xfc);

#ifdef __PPC__
    out_be32((uint32_t *) bridge.cfg_addr, addr);
    out_le32((uint32_t *) bridge.cfg_data, val);
#else
    outl(PCI_CONFIG_ADDRESS, addr);
    outw(PCI_CONFIG_DATA, val);
#endif

}


void pci_list()
{
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
                printf("\t PCI device %04x:%04x (header 0x%02lx)\n",
                        vendor,
                        device,
                        0xFF & pci_read(bus, dev, fun, PCI_REG_HEADERTYPE));

                uint32_t bar0 = pci_read(bus, dev, fun, PCI_REG_BAR0);
                if (bar0) {
                    printf("\t BAR0: 0x%lx", bar0);
#ifdef __PPC__
                    printf("(addr = %lx)\n", (bar0 & BAR_IOADDR_MASK) + bridge.iorange);
#else
                    printf("(addr = %lx)\n", bar0 & BAR_IOADDR_MASK);
#endif
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

#define PEX1_PEXOWBAR1 0xc28
#define PEX1_PEXOWAR1  0xc30
#define WAR_EN         0x80000000
#define WAR_RTT_IO     0x00080000
#define WAR_WTT_IO     0x00008000
#define WAR_OWS_8K     0xC

//addr should be 16MB aligned (this is the size of qemu vga mem arrea)
//and 3rd bit equal to 1 means prefetchable memory
#define VGA_ADDR 0xee000008

#define VBE_DISPI_IOPORT_INDEX 0x01CE
#ifdef __PPC__
#define VBE_DISPI_IOPORT_DATA 0x01D0
#else
#define VBE_DISPI_IOPORT_DATA 0x01CF
#endif
enum {
    VBE_DISPI_INDEX_ID, // (0)
    VBE_DISPI_INDEX_XRES, // (1)
    VBE_DISPI_INDEX_YRES, // (2)
    VBE_DISPI_INDEX_BPP, // (3)
    VBE_DISPI_INDEX_ENABLE, // (4)
    VBE_DISPI_INDEX_BANK, // (5)
    VBE_DISPI_INDEX_VIRT_WIDTH, // (6)
    VBE_DISPI_INDEX_VIRT_HEIGHT, // (7)
    VBE_DISPI_INDEX_X_OFFSET, // (8)
    VBE_DISPI_INDEX_Y_OFFSET, // (9)
};

void pci_init()
{
    printf("\nPCI initializing\n");

#ifdef __PPC__
    pok_bsp_t pok_bsp;
    pok_bsp_get_info(&pok_bsp);
    bridge = pok_bsp.pci_bridge;

    //TODO add pci_bridge too?
    static uint32_t bar0_addr = 0x1001;
    const uint32_t BAR0_SIZE = 0x100;

    printf("bridge cfg_addr: %p cfg_data: %p\n",
            (void *)bridge.cfg_addr, (void *)bridge.cfg_data);

    out_be32((uint32_t *) (bridge.cfg_addr + PEX1_PEXOWBAR1), bridge.iorange>>12);
    out_be32((uint32_t *) (bridge.cfg_addr + PEX1_PEXOWAR1), WAR_EN|WAR_RTT_IO|WAR_WTT_IO|WAR_OWS_8K);

#endif

    for (unsigned int bus = 0; bus < PCI_BUS_MAX; bus++)
      for (unsigned int dev = 0; dev < PCI_DEV_MAX; dev++)
      {
            int fun = 0;
            // Currently we do not handle type 1 or 2 PCI configuration spaces
            if ((pci_read(bus, dev, fun, PCI_REG_HEADERTYPE) & 0xFF) != 0)
                continue;
            printf(">>>%02x:%02x:%02x \n", bus, dev, fun);
            if (dev == 1) {

                printf("initializing BAR0 for 1 dev\n");
                    struct pci_device pci_dev;
                    pci_dev.bus = bus;
                    pci_dev.dev = dev;
                    pci_dev.fun = fun;

                    pci_write_dword(&pci_dev, PCI_REG_BAR0, VGA_ADDR);
                    pci_write_word(&pci_dev, PCI_REG_COMMAND, PCI_COMMAND_MEMORY);

                    //out_8((void *) 0xe10003c0, 0);

                    out_le16((void *)(0xe1000000 + VBE_DISPI_IOPORT_INDEX), 4);
                    out_le16((void *)(0xe1000000 + VBE_DISPI_IOPORT_DATA), 0);

                    out_le16((void *)(0xe1000000 + VBE_DISPI_IOPORT_INDEX), 1);
                    out_le16((void *)(0xe1000000 + VBE_DISPI_IOPORT_DATA), 800);

                    out_le16((void *)(0xe1000000 + VBE_DISPI_IOPORT_INDEX), 2);
                    out_le16((void *)(0xe1000000 + VBE_DISPI_IOPORT_DATA), 600);

                    out_le16((void *)(0xe1000000 + VBE_DISPI_IOPORT_INDEX), 3);
                    out_le16((void *)(0xe1000000 + VBE_DISPI_IOPORT_DATA), 0x20);
                    //out_le16((void *)(0xe1000000 + VBE_DISPI_IOPORT_DATA), 0x08);

                    out_le16((void *)(0xe1000000 + VBE_DISPI_IOPORT_INDEX), 4);
                    out_le16((void *)(0xe1000000 + VBE_DISPI_IOPORT_DATA), 1);

                    //out_8((void *) 0xe10003c0, 0);
                    out_8((void *) 0xe10003c0, 0x20);

                    for (int i = 0; i < 800*600; i++) {
                        out_be32((uint32_t *)VGA_ADDR + i, 0x00FF0000);
                    }

                    //pci_write_dword(&pci_dev, PCI_REG_BAR0, 0x2000);
                    //pci_write_word(&pci_dev, PCI_REG_COMMAND, PCI_COMMAND_IO);

                    //pci_dev.bar[0] = bridge.iorange + bar0_addr;
                    //pci_dev.ioaddr = pci_dev.bar[0] & BAR_IOADDR_MASK;
                continue;
            }
            struct pci_device pci_dev;
            pci_dev.bus = bus;
            pci_dev.dev = dev;
            pci_dev.fun = fun;
            pci_dev.vendorid  = (uint16_t) pci_read(bus, dev, fun, PCI_REG_VENDORID);
            pci_dev.deviceid  = (uint16_t) pci_read(bus, dev, fun, PCI_REG_DEVICEID);
            //pci_dev.classcode = (uint16_t) pci_read(bus, dev, fun, PCI_REG_PROGIFID);

            for (int i = 0; i < pci_driver_table_used_cnt; i++) {
                struct pci_driver *pci_driver = &pci_driver_table[i];
                if (pci_match_device(pci_driver->id_table, &pci_dev)) {
                    printf("MATCH %s\n", pci_driver->name);
#ifdef __PPC__
                    pci_write_dword(&pci_dev, PCI_REG_BAR0, bar0_addr);
                    pci_write_word(&pci_dev, PCI_REG_COMMAND, PCI_COMMAND_IO);
                    pci_dev.bar[0] = bridge.iorange + bar0_addr;
                    pci_dev.ioaddr = pci_dev.bar[0] & BAR_IOADDR_MASK;
                    bar0_addr += BAR0_SIZE;
#else
                    pci_dev.bar[0] = pci_read(bus, dev, fun, PCI_REG_BAR0) & BAR_IOADDR_MASK; //TODO!!!!
                    pci_dev.ioaddr = pci_dev.bar[0] & BAR_IOADDR_MASK;
#endif
                    pci_driver->probe(&pci_dev);
                }
            }
        }
    pci_list();
    printf("\n");
}
