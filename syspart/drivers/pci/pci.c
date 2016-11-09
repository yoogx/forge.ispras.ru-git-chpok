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
#include <pci_config.h>
#include <ioports.h>
#include <stdio.h>

#include <bsp.h>
#include "pci_internal.h"


extern struct pci_driver pci_driver_table[];
extern unsigned pci_driver_table_size;

#include <libc/string.h>

#define OK 0
unsigned pci_driver_table_used_cnt = 0;

#ifdef __PPC__
struct pci_bridge bridge;
uint32_t ccsrbar_base;

struct legacy_io {
    uint32_t virt_addr;
    uint64_t phys_addr; //system physical addr
    // uint32_t pci_addr; always zero
} legacy_io;

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



int pci_match_device(const struct pci_dev_id *id, const struct pci_dev *dev)
{
    if ((id->vendor == PCI_ANY_ID || id->vendor == dev->vendor_id) &&
            (id->device == PCI_ANY_ID || id->device == dev->device_id))
        return 1;
    return 0;
}

void register_pci_driver(struct pci_driver *driver)
{
    pci_driver_table[pci_driver_table_used_cnt++] = *driver;
}


int pci_read_config_byte(struct pci_dev *dev, int where, uint8_t *val)
{
    uint32_t addr = (uint32_t) (1 << 31) | (dev->bus << 16) | (dev->dev << 11) |
        (dev->fn << 8) | (where & 0xfc);

#ifdef __PPC__
    out_be32((uint32_t *) bridge.cfg_addr, addr);
    *val = in_8((void *) (bridge.cfg_data) + (where & 3));
#else
    outl(PCI_CONFIG_ADDRESS, addr);
    *val = inb(PCI_CONFIG_DATA + (where & 3));
#endif
    return OK;
}

int pci_read_config_word(struct pci_dev *dev, int where, uint16_t *val)
{
    uint32_t addr = (uint32_t) (1 << 31) | (dev->bus << 16) | (dev->dev << 11) |
        (dev->fn << 8) | (where & 0xfc);

#ifdef __PPC__
    out_be32((uint32_t *) bridge.cfg_addr, addr);
    *val = in_le16((void *) (bridge.cfg_data) + (where & 3));
#else
    outl(PCI_CONFIG_ADDRESS, addr);
    *val = inw(PCI_CONFIG_DATA + (where & 2));
#endif
    return OK;
}

int pci_read_config_dword(struct pci_dev *dev, int where, uint32_t *val)
{
    uint32_t addr = (uint32_t) (1 << 31) | (dev->bus << 16) | (dev->dev << 11) |
        (dev->fn << 8) | (where & 0xfc);

#ifdef __PPC__
    out_be32((uint32_t *) bridge.cfg_addr, addr);
    *val = in_le32((void *) bridge.cfg_data + (where & 3));
#else
    outl(PCI_CONFIG_ADDRESS, addr);
    *val = inl(PCI_CONFIG_DATA);
#endif
    return OK;
}

int pci_write_config_byte(struct pci_dev *dev, int where, uint8_t val)
{
    uint32_t addr = (uint32_t) (1 << 31) | (dev->bus << 16) | (dev->dev << 11) |
        (dev->fn << 8) | (where & 0xfc);

#ifdef __PPC__
    out_be32((uint32_t *) bridge.cfg_addr, addr);
    out_8((void *) (bridge.cfg_data) + (where & 3), val);
#else
    outl(PCI_CONFIG_ADDRESS, addr);
    outb(val, PCI_CONFIG_DATA + (where & 3));
#endif
    return OK;
}

int pci_write_config_word(struct pci_dev *dev, int where, uint16_t val)
{
    uint32_t addr = (uint32_t) (1 << 31) | (dev->bus << 16) | (dev->dev << 11) |
        (dev->fn << 8) | (where & 0xfc);

#ifdef __PPC__
    out_be32((uint32_t *) bridge.cfg_addr, addr);
    out_le16((void *) (bridge.cfg_data) + (where & 3), val);
#else
    outl(PCI_CONFIG_ADDRESS, addr);
    outw(val, PCI_CONFIG_DATA + (where & 2));
#endif
    return OK;
}

int pci_write_config_dword(struct pci_dev *dev, int where, uint32_t val)
{
    uint32_t addr = (uint32_t) (1 << 31) | (dev->bus << 16) | (dev->dev << 11) |
        (dev->fn << 8) | (where & 0xfc);

#ifdef __PPC__
    out_be32((uint32_t *) bridge.cfg_addr, addr);
    out_le32((void *) (bridge.cfg_data) + (where & 3), val);
#else
    outl(PCI_CONFIG_ADDRESS, addr);
    outl(PCI_CONFIG_DATA, val);
#endif
    return OK;
}


// zero means that device config is not found
static uintptr_t get_resource_addr_from_config(uint8_t bus, uint8_t dev, uint8_t fn, enum PCI_RESOURCE_INDEX idx)
{
    //TODO optimize this linear search
    struct pci_dev_config *d;
    for (int i = 0; i < pci_configs_nb; i++) {
        d = &pci_configs[i];
        if (d->bus == bus && d->dev == dev && d->fn == fn)
            return d->resources[idx].addr;
    }

    return 0;
}



static void pci_fill_resource(struct pci_dev *dev, enum PCI_RESOURCE_INDEX res_idx)
{
    //TODO :
    // 1. Decode (I/O or memory) of a register is disabled via the command
    //       register before sizing a Base Address register.
    // 2. The original value in the Base Address register is restored before re-enabling
    //       decode in the command register of the device.
    // 3. "Note that the upper 16 bits of the result is ignored if the Base
    //       Address register is for I/O and bits 16-31 returned zero upon read."
    // 4. 64 bits bars
    //
    int reg;
    uint32_t val, size, mask;

    struct pci_resource *resource = &dev->resources[res_idx];
    memset(resource, 0, sizeof(*resource));


    if (res_idx != PCI_RESOURCE_ROM) {
        reg = PCI_BASE_ADDRESS_0 + res_idx*4;
        mask =  0xffffffff;
    } else {
        reg = PCI_ROM_ADDRESS;
        mask = PCI_ROM_ADDRESS_MASK;
    }

    pci_read_config_dword(dev, reg, &val);
    pci_write_config_dword(dev, reg, mask);
    pci_read_config_dword(dev, reg, &size);
    pci_write_config_dword(dev, reg, val);

    if (res_idx == PCI_RESOURCE_ROM)  {
        resource->type = PCI_RESOURCE_TYPE_ROM;
        resource->pci_addr = val & PCI_ROM_ADDRESS_MASK;
        size &= PCI_ROM_ADDRESS_MASK;

    } else if ((val&PCI_BASE_ADDRESS_SPACE) == PCI_BASE_ADDRESS_SPACE_IO) {
        resource->type = PCI_RESOURCE_TYPE_BAR_IO;
        resource->pci_addr = val & PCI_BASE_ADDRESS_IO_MASK;
        size &= PCI_BASE_ADDRESS_IO_MASK;

    } else {
        if ((val & PCI_BASE_ADDRESS_MEM_TYPE_MASK) == PCI_BASE_ADDRESS_MEM_TYPE_32) {
            resource->mem_flags |= PCI_RESOURCE_MEM_MASK_32;
        }
        if (val & PCI_BASE_ADDRESS_MEM_PREFETCH) {
            resource->mem_flags |= PCI_RESOURCE_MEM_MASK_PREFETCH;
        }
        resource->type = PCI_RESOURCE_TYPE_BAR_MEM;
        resource->pci_addr = val & PCI_BASE_ADDRESS_MEM_MASK;
        size &= PCI_BASE_ADDRESS_MEM_MASK;
    }

    resource->size = ~size + 1;
    resource->addr = get_resource_addr_from_config(dev->bus, dev->dev, dev->fn, res_idx);
}

/*
 * see comment in pci.h
 */
void pci_get_dev_by_bdf(uint8_t bus, uint8_t dev, uint8_t fn, struct pci_dev *pci_dev)
{
    memset(pci_dev, 0, sizeof(*pci_dev));

    pci_dev->bus = bus;
    pci_dev->dev = dev;
    pci_dev->fn = fn;

    pci_read_config_word(pci_dev, PCI_VENDOR_ID, &pci_dev->vendor_id);
    //if (vendor_id == 0xFFFF) {
    //}
    pci_read_config_word(pci_dev, PCI_DEVICE_ID,    &pci_dev->device_id);
    pci_read_config_word(pci_dev, PCI_CLASS_DEVICE, &pci_dev->class_code);
    pci_read_config_byte(pci_dev, PCI_HEADER_TYPE,  &pci_dev->hdr_type);

    if (pci_dev->hdr_type != 0)
        return;

    for (int i = 0; i < PCI_NUM_RESOURCES; i++) {
        pci_fill_resource(pci_dev, i);
    }

}

void pci_enumerate()
{
    struct pci_dev pci_dev;
    for (unsigned int bus = 0; bus < PCI_BUS_MAX; bus++)
      for (unsigned int dev = 0; dev < PCI_DEV_MAX; dev++)
        for (unsigned int fn = 0; fn < PCI_FN_MAX; fn++)
        {
            pci_get_dev_by_bdf(bus, dev, fn, &pci_dev);

            if (pci_dev.vendor_id == 0xFFFF) {
                continue;
            }

            printf("%02x:%02x:%02x %s\n",
                    bus, dev, fn, get_pci_class_name(pci_dev.class_code));
            printf("\t PCI device %04x:%04x (header 0x%02x)\n",
                    pci_dev.vendor_id,
                    pci_dev.device_id,
                    pci_dev.hdr_type);

            if (pci_dev.hdr_type != 0) {
                continue;
            }

            for (int i = 0; i < PCI_NUM_RESOURCES; i++) {
                struct pci_resource *res = &pci_dev.resources[i];
                if (res->size == 0)
                    continue;

                if (res->type != PCI_RESOURCE_TYPE_ROM){
                    printf("\t BAR%d: ", i);
                    if (res->type == PCI_RESOURCE_TYPE_BAR_IO) {
                        printf("I/O ");
                    } else {
                        printf("%s bit %smemory ",
                            res->mem_flags & PCI_RESOURCE_MEM_MASK_32 ? "32": "<UNSUPPORTED> 64",
                            res->mem_flags & PCI_RESOURCE_MEM_MASK_PREFETCH ? "prefetchable ":"");
                    }
                }
                else {
                    printf("\t ROM: ");
                }
                printf("[size=0x%zx]\n", res->size);

            }
        }
}


void pci_list()
{
    struct pci_dev pci_dev;
    for (unsigned int bus = 0; bus < PCI_BUS_MAX; bus++)
      for (unsigned int dev = 0; dev < PCI_DEV_MAX; dev++)
        for (unsigned int fn = 0; fn < PCI_FN_MAX; fn++)
        {
            pci_get_dev_by_bdf(bus, dev, fn, &pci_dev);

            if (pci_dev.vendor_id == 0xFFFF) {
                continue;
            }

            printf("%02x:%02x:%02x %s\n",
                    bus, dev, fn, get_pci_class_name(pci_dev.class_code));
            printf("\t PCI device %04x:%04x (header 0x%02x)\n",
                    pci_dev.vendor_id,
                    pci_dev.device_id,
                    pci_dev.hdr_type);

            if (pci_dev.hdr_type != 0) {
                uint16_t tmp;
                uint32_t tmp32;

                pci_read_config_word(&pci_dev, 0x10, &tmp);
                printf("\t bar0: 0x%x\n", tmp);

                pci_read_config_word(&pci_dev, 0x20, &tmp);
                printf("\t mem base: 0x%x\n", tmp);

                pci_read_config_word(&pci_dev, 0x22, &tmp);
                printf("\t mem limit: 0x%x\n", tmp);


                pci_read_config_word(&pci_dev, 0x24, &tmp);
                printf("\t pr mem base: 0x%x\n", tmp);


                pci_read_config_word(&pci_dev, 0x26, &tmp);
                printf("\t pr mem limit: 0x%x\n", tmp);

                pci_read_config_dword(&pci_dev, 0x28, &tmp32);
                printf("\t pr mem base upp: 0x%x\n", tmp);

                pci_read_config_dword(&pci_dev, 0x2C, &tmp32);
                printf("\t pr limit upp: 0x%x\n", tmp);

                pci_read_config_dword(&pci_dev, 0x38, &tmp32);
                printf("\t exp rom : 0x%lx\n", tmp32);

                continue;
            }

            for (int i = 0; i < PCI_NUM_RESOURCES; i++) {
                struct pci_resource *res = &pci_dev.resources[i];
                if (res->size == 0)
                    continue;

                if (res->type != PCI_RESOURCE_TYPE_ROM){
                    printf("\t BAR%d: ", i);
                    if (res->type == PCI_RESOURCE_TYPE_BAR_IO) {
                        printf("I/O ");
                    } else {
                        printf("%s bit %smem ",
                            res->mem_flags & PCI_RESOURCE_MEM_MASK_32 ? "32": "<UNSUPPORTED> 64",
                            res->mem_flags & PCI_RESOURCE_MEM_MASK_PREFETCH ? "prefetch ":"");
                    }
                }
                else {
                    printf("\t ROM: ");
                }


                if (res->pci_addr) {
                    printf("0x%llx ", res->pci_addr);
#ifdef __PPC__
                    printf("(addr = 0x%x) ", res->addr);
#endif
                } else {
                    printf("_ ");

                }

                printf("[size=0x%zx]\n", res->size);

            }
        }
}

//dev will be usefull when bridge is not hardcoded
uintptr_t pci_convert_legacy_port(struct pci_dev *dev, uint16_t port)
{
    (void) dev;
#ifdef __PPC__
    return legacy_io.virt_addr + port;
#else
    return port;
#endif
}

#define PEX1_PEXOTAR0  0xc00
#define PEX1_PEXOTEAR0 0xc04
#define PEX1_PEXOWAR0  0xc10

#define PEX1_PEXOTAR1  0xc20
#define PEX1_PEXOTEAR1 0xc24
#define PEX1_PEXOWBAR1 0xc28
#define PEX1_PEXOWAR1  0xc30

#define PEX_SHIFT      0x20

#define PEX1_PEXOTAR2  0xc40
#define PEX1_PEXOWBAR2 0xc48
#define PEX1_PEXOWAR2  0xc50

#define PEX1_PEXITAR3  0xd80
#define PEX1_PEXIWBAR3 0xd8c

#define WAR_EN         0x80000000
#define WAR_RTT_IO     0x00080000
#define WAR_RTT_MEM    0x00040000
#define WAR_WTT_IO     0x00008000
#define WAR_WTT_MEM    0x00004000
#define WAR_OWS_8K     0xC
#define WAR_OWS_256M   0x1b
#define WAR_OWS_4G     0x1f

#ifdef __PPC__
//TODO move to fsl_pci.h
/* PCI/PCI Express outbound window reg */
struct pci_outbound_window_regs {
    uint32_t potar;  /* 0x.0 - Outbound translation address register */
    uint32_t potear; /* 0x.4 - Outbound translation extended address register */
    uint32_t powbar; /* 0x.8 - Outbound window base address register */
    uint8_t  pad1[4];
    uint32_t powar;  /* 0x.10 - Outbound window attributes register */
    uint8_t  pad2[12];
};

/* PCI/PCI Express inbound window reg */
struct pci_inbound_window_regs {
    uint32_t pitar;  /* 0x.0 - Inbound translation address register */
    uint8_t  pad1[4];
    uint32_t piwbar; /* 0x.8 - Inbound window base address register */
    uint32_t piwbear; /* 0x.c - Inbound window base extended address register */
    uint32_t piwar;  /* 0x.10 - Inbound window attributes register */
    uint8_t  pad2[12];
};

struct pci_atmu_windows {
    /* PCI/PCI Express outbound window 0-4
     * Window 0 is the default window and is the only window enabled upon reset.
     * The default outbound register set is used when a transaction misses
     * in all of the other outbound windows.
     */
    struct pci_outbound_window_regs pow[5];
    uint8_t pad1[96];
    struct pci_inbound_window_regs pmit;   /* 0xd00 - 0xd9c Inbound MSI */
    uint8_t pad2[96];
    /* PCI/PCI Express inbound window 3-0
     * inbound window 1 supports only a 32-bit base address and does not
     * define an inbound window base extended address register.
     */
    struct pci_inbound_window_regs piw[4];
};

void pci_ATMU_windows_clear()
{
    struct pci_atmu_windows *atmu = (struct pci_atmu_windows *)(bridge.cfg_addr + PEX1_PEXOTAR0);
//FIXME
    struct pci_atmu_windows *atmu2 = (struct pci_atmu_windows *)(ccsrbar_base + 0x201000 + PEX1_PEXOTAR0);

    for(int i = 1; i < 5; i++) {
        out_be32(&atmu->pow[i].powar, 0);
        out_be32(&atmu2->pow[i].powar, 0);
    }
    for(int i = 1; i < 4; i++) {
        out_be32(&atmu->piw[i].piwar, 0);
        out_be32(&atmu2->piw[i].piwar, 0);
    }
}

void pci_ATMU_windows_list()
{
    struct pci_atmu_windows *atmu = (struct pci_atmu_windows *)(bridge.cfg_addr + PEX1_PEXOTAR0);

    printf("ATMU:\n");
    printf("   outbound windows:\n");
    for (int i = 0; i < 5; i++) {
        printf("\t window %d   %lx -> %lx:%lx [%lx]\n", i,
                atmu->pow[i].powbar,
                atmu->pow[i].potear, atmu->pow[i].potar,
                atmu->pow[i].powar);
    }
    printf("   MSI window\n");
    printf("\t window %lx -> %lx:%lx [%lx]\n",
            atmu->pmit.pitar,
            atmu->pmit.piwbar, atmu->pmit.piwbear,
            atmu->pmit.piwar);
    printf("   inbound windows:\n");
    for (int i = 0; i < 4; i++) {
        printf("\t window %d   %lx -> %lx:%lx [%lx]\n", i,
                atmu->piw[i].pitar,
                atmu->piw[i].piwbar, atmu->piw[i].piwbear,
                atmu->piw[i].piwar);
    }

}

struct LAW_regs {
    uint32_t barh;
    uint32_t barl;
    uint32_t ar;
    uint32_t pad;
};

void LAW_list()
{

    //FIXME
    struct LAW_regs *law = (struct LAW_regs *)(ccsrbar_base + 0xC00);

    //law[6].barl = 0x40000000;
    asm("isync");
    printf("LAW (local access windows):\n");

    for (int i = 0; i < 32; i++) {
        printf("\t law %d  %lx:%lx [%lx]\n", i,
                law[i].barh,
                law[i].barl,
                law[i].ar);
    }

}

#endif

void pci_init()
{
#ifdef __PPC__
    pok_bsp_t pok_bsp;
    pok_bsp_get_info(&pok_bsp);
    bridge = pok_bsp.pci_bridge;
    ccsrbar_base = (uint32_t) pok_bsp.ccsrbar_base;

    //TODO add pci_bridge too?
    //static uint32_t bar0_addr = 0x1001;
    //const uint32_t BAR0_SIZE = 0x100;

    printf("bridge cfg_addr: %p cfg_data: %p\n",
            (void *)bridge.cfg_addr, (void *)bridge.cfg_data);

    legacy_io.virt_addr = 0xe1000000;
    legacy_io.phys_addr = 0xe1000000;
    //legacy_io.phys_addr = 0;

    struct pci_atmu_windows *atmu = (struct pci_atmu_windows *)(bridge.cfg_addr + PEX1_PEXOTAR0);

    atmu->pow[1].powbar = legacy_io.phys_addr>>12;
    atmu->pow[1].potar = 0;
    atmu->pow[1].potear = 0;
    atmu->pow[1].powar = WAR_EN|WAR_RTT_IO|WAR_WTT_IO|WAR_OWS_8K;

    atmu->pow[2].powbar = 0x80000000 >> 12;
    atmu->pow[2].potar =  0x80000000 >> 12;
    atmu->pow[2].potear = 0;
    atmu->pow[2].powar = WAR_EN|WAR_RTT_IO|WAR_WTT_IO|WAR_OWS_256M;

    pci_ATMU_windows_list();

    LAW_list();
#endif

    //printf("PCI enumeration:\n");
    //pci_enumerate();

    printf("PCI initialization using configuration\n");

    for (int i = 0; i < pci_configs_nb; i++) {
        struct pci_dev_config *dev_config = &pci_configs[i];
        struct pci_dev pci_dev;

        printf("%02x:%02x:%02x  ",
                dev_config->bus,
                dev_config->dev,
                dev_config->fn);

        pci_dev.bus = dev_config->bus;
        pci_dev.dev = dev_config->dev;
        pci_dev.fn =  dev_config->fn;

        {
            uint16_t vendor_id;
            pci_read_config_word(&pci_dev, PCI_VENDOR_ID, &vendor_id);

            if (vendor_id == 0xFFFF) {
                printf("Not found\n");
                continue;
            }
            printf("OK\n");
        }

        int command = 0;
        for (int i = 0; i < PCI_RESOURCE_ROM; i++) {
            if (dev_config->resources[i].pci_addr == 0)
                continue;

            pci_write_config_dword(&pci_dev, PCI_BASE_ADDRESS_0,
                    dev_config->resources[i].pci_addr);

            if (dev_config->resources[i].type == PCI_RESOURCE_TYPE_BAR_MEM)
                command |= PCI_COMMAND_MEMORY;
            else if (dev_config->resources[i].type == PCI_RESOURCE_TYPE_BAR_IO)
                command |= PCI_COMMAND_IO;
        }

        if (dev_config->resources[PCI_RESOURCE_ROM].pci_addr != 0) {
            pci_write_config_dword(&pci_dev, PCI_ROM_ADDRESS,
                    dev_config->resources[PCI_RESOURCE_ROM].pci_addr|PCI_ROM_ADDRESS_ENABLE);
            command |= PCI_COMMAND_MEMORY;
        }

        pci_write_config_word(&pci_dev, PCI_COMMAND, command);
    }
    printf("PCI init result:\n");
    pci_list();
    printf("\n");
}
