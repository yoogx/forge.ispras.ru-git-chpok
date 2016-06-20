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

#define OK 0
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

//Depricated
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


int pci_read_config_byte(struct pci_device *dev, int where, uint8_t *val)
{
    uint32_t addr = (uint32_t) (1 << 31) | (dev->bus << 16) | (dev->dev << 11) |
        (dev->fun << 8) | (where & 0xfc);

#ifdef __PPC__
    out_be32((uint32_t *) bridge.cfg_addr, addr);
    *val = in_8((void *) (bridge.cfg_data) + (where & 3));
#else
    outl(PCI_CONFIG_ADDRESS, addr);
    *val = inb(PCI_CONFIG_DATA + (where & 3));
#endif
    return OK;
}

int pci_read_config_word(struct pci_device *dev, int where, uint16_t *val)
{
    uint32_t addr = (uint32_t) (1 << 31) | (dev->bus << 16) | (dev->dev << 11) |
        (dev->fun << 8) | (where & 0xfc);

#ifdef __PPC__
    out_be32((uint32_t *) bridge.cfg_addr, addr);
    *val = in_le16((void *) (bridge.cfg_data) + (where & 3));
#else
    outl(PCI_CONFIG_ADDRESS, addr);
    *val = inw(PCI_CONFIG_DATA + (where & 2));
#endif
    return OK;
}

int pci_read_config_dword(struct pci_device *dev, int where, uint32_t *val)
{
    uint32_t addr = (uint32_t) (1 << 31) | (dev->bus << 16) | (dev->dev << 11) |
        (dev->fun << 8) | (where & 0xfc);

#ifdef __PPC__
    out_be32((uint32_t *) bridge.cfg_addr, addr);
    *val = in_le32((void *) bridge.cfg_data + (where & 3));
#else
    outl(PCI_CONFIG_ADDRESS, addr);
    *val = inl(PCI_CONFIG_DATA);
#endif
    return OK;
}

int pci_write_config_byte(struct pci_device *dev, int where, uint8_t val)
{
    uint32_t addr = (uint32_t) (1 << 31) | (dev->bus << 16) | (dev->dev << 11) |
        (dev->fun << 8) | (where & 0xfc);

#ifdef __PPC__
    out_be32((uint32_t *) bridge.cfg_addr, addr);
    out_8((void *) (bridge.cfg_data) + (where & 3), val);
#else
    outl(PCI_CONFIG_ADDRESS, addr);
    outb(val, PCI_CONFIG_DATA + (where & 3));
#endif
    return OK;
}

int pci_write_config_word(struct pci_device *dev, int where, uint16_t val)
{
    uint32_t addr = (uint32_t) (1 << 31) | (dev->bus << 16) | (dev->dev << 11) |
        (dev->fun << 8) | (where & 0xfc);

#ifdef __PPC__
    out_be32((uint32_t *) bridge.cfg_addr, addr);
    out_le16((void *) (bridge.cfg_data) + (where & 3), val);
#else
    outl(PCI_CONFIG_ADDRESS, addr);
    outw(val, PCI_CONFIG_DATA + (where & 2));
#endif
    return OK;
}

int pci_write_config_dword(struct pci_device *dev, int where, uint32_t val)
{
    uint32_t addr = (uint32_t) (1 << 31) | (dev->bus << 16) | (dev->dev << 11) |
        (dev->fun << 8) | (where & 0xfc);

#ifdef __PPC__
    out_be32((uint32_t *) bridge.cfg_addr, addr);
    out_le32((void *) (bridge.cfg_data) + (where & 3), val);
#else
    outl(PCI_CONFIG_ADDRESS, addr);
    *val = inl(PCI_CONFIG_DATA);
#endif
    return OK;
}

static uint32_t rom(struct pci_device *dev)
{
    uint32_t rom, size;
    int where = PCI_ROM_ADDRESS;
    printf("\t ROM ");
    pci_read_config_dword(dev, where, &rom);
    printf("0x%lx ", rom);
    if ((rom & PCI_ROM_ADDRESS_ENABLE) == 0) {
        printf("\n");
        return 0;
    }
    pci_write_dword(dev, where, 0xffffffff);
    pci_read_config_dword(dev, where, &size);
    pci_write_dword(dev, where, rom);

    if (size == 0 || size == 0xffffffff) {
        printf("\n");
        return 0;
    }

    size &= PCI_ROM_ADDRESS_MASK;
    printf("%s bit memory %s",
            (rom & PCI_BASE_ADDRESS_MEM_TYPE_MASK) == PCI_BASE_ADDRESS_MEM_TYPE_32?
                "32": "<UNSUPPORTED YET> 64",
            rom & PCI_BASE_ADDRESS_MEM_PREFETCH? "prefetchable":"");

    size = (size & ~(size-1));
    printf(" [size=0x%lx]\n", size);

    return size;
}
static uint32_t pci_bar(struct pci_device *dev, int where)
{
    uint32_t bar, size;
    pci_read_config_dword(dev, where, &bar);
    pci_write_dword(dev, where, 0xffffffff);
    pci_read_config_dword(dev, where, &size);
    pci_write_dword(dev, where, bar);

    if (size == 0 || size == 0xffffffff) {
        printf("\n");
        return 0;
    }

    if ((bar&PCI_BASE_ADDRESS_SPACE) == PCI_BASE_ADDRESS_SPACE_IO) {
        printf("I/O");
        size &= PCI_BASE_ADDRESS_IO_MASK;
    } else {
        printf("%s bit memory %s",
                (bar & PCI_BASE_ADDRESS_MEM_TYPE_MASK) == PCI_BASE_ADDRESS_MEM_TYPE_32?
                    "32": "<UNSUPPORTED YET> 64",
                bar & PCI_BASE_ADDRESS_MEM_PREFETCH? "prefetchable":"");
        size &= PCI_BASE_ADDRESS_MEM_MASK;
    }

    size = (size & ~(size-1));
    printf(" [size=0x%lx]\n", size);

    return size;
}

void pci_enumerate()
{
    uint16_t vendor_id, device_id, classcode;
    uint8_t header_type;

    struct pci_device pci_dev;
    for (unsigned int bus = 0; bus < PCI_BUS_MAX; bus++)
      for (unsigned int dev = 0; dev < PCI_DEV_MAX; dev++)
        for (unsigned int fun = 0; fun < PCI_FUN_MAX; fun++)
        {
            pci_dev.bus = bus;
            pci_dev.dev = dev;
            pci_dev.fun = fun;

            pci_read_config_word(&pci_dev, PCI_VENDOR_ID, &vendor_id);
            if (vendor_id == 0xFFFF) {
                continue;
            }

            pci_read_config_word(&pci_dev, PCI_DEVICE_ID, &device_id);
            pci_read_config_word(&pci_dev, PCI_CLASS_DEVICE, &classcode);
            pci_read_config_byte(&pci_dev, PCI_HEADER_TYPE, &header_type);

            printf("%02x:%02x:%02x %s\n", bus, dev, fun, get_pci_class_name(classcode));
            printf("\t PCI device %04x:%04x (header 0x%02x)\n",
                    vendor_id,
                    device_id,
                    header_type);

            if (header_type != 0) {
                continue;
            }

            for (int i = 0; i < 6; i++) {
            //for (int i = 0; i < 1; i++) {
                int reg = PCI_BASE_ADDRESS_0 + i*4;
                printf("\t BAR%d: ", i);
                pci_bar(&pci_dev, reg);
            }
            rom(&pci_dev);
        }
}


void pci_list()
{
    for (unsigned int bus = 0; bus < PCI_BUS_MAX; bus++)
      for (unsigned int dev = 0; dev < PCI_DEV_MAX; dev++)
        for (unsigned int fun = 0; fun < PCI_FUN_MAX; fun++)
        {
            uint16_t vendor = (uint16_t) pci_read(bus, dev, fun, PCI_VENDOR_ID);
            uint16_t device = (uint16_t) pci_read(bus, dev, fun, PCI_DEVICE_ID);

            if (vendor != 0xFFFF) {
                uint32_t classcode = 0xFFFFFF & pci_read(bus, dev, fun, PCI_CLASS_PROG);
                printf("%02x:%02x:%02x ", bus, dev, fun);
                printf("%s:\n", get_pci_class_name(classcode));
                printf("\t PCI device %04x:%04x (header 0x%02lx)\n",
                        vendor,
                        device,
                        0xFF & pci_read(bus, dev, fun, PCI_HEADER_TYPE));

                uint32_t bar0 = pci_read(bus, dev, fun, PCI_BASE_ADDRESS_0);
                if (bar0) {
                    printf("\t BAR0: 0x%lx", bar0);

                    {
                        struct pci_device pci_dev;
                        pci_dev.bus = bus;
                        pci_dev.dev = dev;
                        pci_dev.fun = fun;

                        pci_write_dword(&pci_dev, PCI_BASE_ADDRESS_0, 0xffffffff);
                        bar0 = pci_read(bus, dev, fun, PCI_BASE_ADDRESS_0);
                        printf("\t BAR0 size: 0x%lx", bar0);
                    }

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

#include "vbe.h"


struct gimp_image {
  unsigned int 	 width;
  unsigned int 	 height;
  unsigned int 	 bytes_per_pixel; /* 2:RGB16, 3:RGB, 4:RGBA */ 
  unsigned char	 pixel_data[];
};
extern const struct gimp_image gimp_image;

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

    printf("\nPCI enumeration:\n");
    pci_enumerate();

    for (unsigned int bus = 0; bus < PCI_BUS_MAX; bus++)
      for (unsigned int dev = 0; dev < PCI_DEV_MAX; dev++)
      {
            int fun = 0;
            // Currently we do not handle type 1 or 2 PCI configuration spaces
            if ((pci_read(bus, dev, fun, PCI_HEADER_TYPE) & 0xFF) != 0)
                continue;
            printf(">>>%02x:%02x:%02x \n", bus, dev, fun);
            if (dev == 1) {

#ifdef __PPC__
                printf("initializing BAR0 for 1 dev\n");
                    struct pci_device pci_dev;
                    pci_dev.bus = bus;
                    pci_dev.dev = dev;
                    pci_dev.fun = fun;

                    pci_write_dword(&pci_dev, PCI_BASE_ADDRESS_0, VGA_ADDR);
                    pci_write_word(&pci_dev, PCI_COMMAND, PCI_COMMAND_MEMORY);

                    out_le16((void *)(0xe1000000 + VBE_DISPI_IOPORT_INDEX), VBE_DISPI_INDEX_ENABLE);
                    out_le16((void *)(0xe1000000 + VBE_DISPI_IOPORT_DATA), 0);

                    out_le16((void *)(0xe1000000 + VBE_DISPI_IOPORT_INDEX), VBE_DISPI_INDEX_XRES);
                    out_le16((void *)(0xe1000000 + VBE_DISPI_IOPORT_DATA), 800);

                    out_le16((void *)(0xe1000000 + VBE_DISPI_IOPORT_INDEX), VBE_DISPI_INDEX_YRES);
                    out_le16((void *)(0xe1000000 + VBE_DISPI_IOPORT_DATA), 600);

                    out_le16((void *)(0xe1000000 + VBE_DISPI_IOPORT_INDEX), VBE_DISPI_INDEX_BPP);
                    out_le16((void *)(0xe1000000 + VBE_DISPI_IOPORT_DATA), VBE_DISPI_BPP_16);

                    out_le16((void *)(0xe1000000 + VBE_DISPI_IOPORT_INDEX), VBE_DISPI_INDEX_ENABLE);
                    out_le16((void *)(0xe1000000 + VBE_DISPI_IOPORT_DATA), 1);

                    out_8((void *) 0xe10003c0, 0x20); // PAS

                    /*
                    for (int i = 0; i < 800*600; i++) {
                        out_be32((uint32_t *) VGA_ADDR + i, 0x00FF0000);
                    //    out_be16((uint16_t *) VGA_ADDR + i, 0xe000);
                    }*/
                    for (int y = 0; y < gimp_image.height; y++) {
                        for (int x = 0; x < gimp_image.width; x++) {
                            //out_be16((uint16_t *) VGA_ADDR + y * 800 + x, 0xe000);
                            //for (int i = 0; i<2; i++) {
                            //    out_8((uint8_t *) VGA_ADDR + (y*800+x)*2 + i,
                            //            gimp_image.pixel_data[(y*gimp_image.width + x)*2 + 1-i]);
                            //}
                            out_le16((uint16_t *) VGA_ADDR + y*800+x,
                                ((uint16_t*)gimp_image.pixel_data)[y*gimp_image.width + x]);
                        }
                    }
#endif

                    // bridge.iorange
                continue;
            }
            struct pci_device pci_dev;
            pci_dev.bus = bus;
            pci_dev.dev = dev;
            pci_dev.fun = fun;
            pci_dev.vendorid  = (uint16_t) pci_read(bus, dev, fun, PCI_VENDOR_ID);
            pci_dev.deviceid  = (uint16_t) pci_read(bus, dev, fun, PCI_DEVICE_ID);
            //pci_dev.classcode = (uint16_t) pci_read(bus, dev, fun, PCI_REG_PROGIFID);

            for (int i = 0; i < pci_driver_table_used_cnt; i++) {
                struct pci_driver *pci_driver = &pci_driver_table[i];
                if (pci_match_device(pci_driver->id_table, &pci_dev)) {
                    printf("MATCH %s\n", pci_driver->name);
#ifdef __PPC__
                    pci_write_dword(&pci_dev, PCI_BASE_ADDRESS_0, bar0_addr);
                    pci_write_word(&pci_dev, PCI_COMMAND, PCI_COMMAND_IO);
                    pci_dev.bar[0] = bridge.iorange + bar0_addr;
                    pci_dev.ioaddr = pci_dev.bar[0] & BAR_IOADDR_MASK;
                    bar0_addr += BAR0_SIZE;
#else
                    pci_dev.bar[0] = pci_read(bus, dev, fun, PCI_BASE_ADDRESS_0) & BAR_IOADDR_MASK; //TODO!!!!
                    pci_dev.ioaddr = pci_dev.bar[0] & BAR_IOADDR_MASK;
#endif
                    pci_driver->probe(&pci_dev);
                }
            }
        }
    pci_list();
    printf("\n");
}
