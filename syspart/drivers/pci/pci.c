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

#include "pci_internal.h"

#include <memblocks.h>
#include <stdlib.h>
#include <assert.h>

#include <string.h>


#define OK 0

#define PCI_DEBUG

#ifdef __PPC__

struct pci_controller {
    uint32_t cfg_addr;
    uint32_t cfg_data;

    //legacy_io for IO BAR
    uint32_t legacy_io_vaddr;
    uint64_t legacy_io_paddr;
} pci_controller;

#endif

#ifdef PCI_DEBUG
static char *get_pci_class_name(int classcode) {
    int i = 0;
    while (pci_classnames[i].name != NULL) {
        if (pci_classnames[i].classcode == (classcode & 0xFFFF00))
            return pci_classnames[i].name;
        i++;
    }
    return "Unknown";
}
#endif

int pci_read_config_byte(struct pci_dev *dev, int where, uint8_t *val)
{
    uint32_t addr = (uint32_t) (1 << 31) | (dev->bus << 16) | (dev->dev << 11) |
        (dev->fn << 8) | (where & 0xfc);

#ifdef __PPC__
    out_be32((uint32_t *) pci_controller.cfg_addr, addr);
    *val = in_8((void *) (pci_controller.cfg_data) + (where & 3));
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
    out_be32((uint32_t *) pci_controller.cfg_addr, addr);
    *val = in_le16((void *) (pci_controller.cfg_data) + (where & 3));
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
    out_be32((uint32_t *) pci_controller.cfg_addr, addr);
    *val = in_le32((void *) pci_controller.cfg_data + (where & 3));
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
    out_be32((uint32_t *) pci_controller.cfg_addr, addr);
    out_8((void *) (pci_controller.cfg_data) + (where & 3), val);
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
    out_be32((uint32_t *) pci_controller.cfg_addr, addr);
    out_le16((void *) (pci_controller.cfg_data) + (where & 3), val);
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
    out_be32((uint32_t *) pci_controller.cfg_addr, addr);
    out_le32((void *) (pci_controller.cfg_data) + (where & 3), val);
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
            return d->c_resources[idx].vaddr;
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
        reg = PCI_BASE_ADDRESS_0 + res_idx*4; //TODO use pci_resource_address
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

void pci_list()
{
#ifdef PCI_DEBUG
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
                    printf("(virt addr = 0x%x) ", res->addr);
#endif
                } else {
                    printf("_ ");

                }

                printf("[size=0x%zx]\n", res->size);

            }
        }
#endif
}

//dev will be usefull when pci_controller is not hardcoded
uintptr_t pci_convert_legacy_port(struct pci_dev *dev, uint16_t port)
{
    (void) dev;
#ifdef __PPC__
    return pci_controller.legacy_io_vaddr + port;
#else
    return port;
#endif
}

#ifdef __PPC__

#define PEX_ATMU_SHIFT 0xC00

#define WAR_EN         0x80000000
#define WAR_RTT_IO     0x00080000
#define WAR_RTT_MEM    0x00040000
#define WAR_WTT_IO     0x00008000
#define WAR_WTT_MEM    0x00004000
#define WAR_OWS_8K     0xc
#define WAR_OWS_16K    0xf
#define WAR_OWS_256M   0x1b
#define WAR_OWS_512M   0x1c
#define WAR_OWS_4G     0x1f

//TODO move to fsl_pci.h
/* PCI/PCI Express outbound window reg */
struct pci_outbound_window_regs {
    uint32_t potar;  /* 0x.0 - Outbound translation address register */
    uint32_t potear; /* 0x.4 - Outbound translation extended address register */
    uint32_t powbar; /* 0x.8 - Outbound window base address register */ //Not used to window 0
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
    struct pci_atmu_windows *atmu = (struct pci_atmu_windows *)(pci_controller.cfg_addr + PEX_ATMU_SHIFT);

    for(int i = 1; i < 5; i++) {
        out_be32(&atmu->pow[i].powar, 0);
    }
    for(int i = 1; i < 4; i++) {
        out_be32(&atmu->piw[i].piwar, 0);
    }
}

void pci_ATMU_windows_list()
{
#ifdef PCI_DEBUG
    struct pci_atmu_windows *atmu = (struct pci_atmu_windows *)(pci_controller.cfg_addr + PEX_ATMU_SHIFT);

    printf("ATMU:\n");
    printf("   outbound windows (From CPU address space to PCI address space):\n");
    for (int i = 0; i < 5; i++) {
        if (atmu->pow[i].powar & WAR_EN) { //window enabled
            printf("\t window %d   %llx -> %llx, AR = %lx\n", i,
                    ((uint64_t) atmu->pow[i].powbar)<<12,
                    (((uint64_t) atmu->pow[i].potear)<<44) + (((uint64_t) atmu->pow[i].potar)<<12),
                    atmu->pow[i].powar);
        }
    }
    if (atmu->pmit.piwar & WAR_EN) { //window enabled
        printf("   MSI window\n");
        printf("\t window %lx -> %lx:%lx [%lx]\n",
                atmu->pmit.pitar,
                atmu->pmit.piwbar, atmu->pmit.piwbear,
                atmu->pmit.piwar);
    }
    printf("   inbound windows (From PCI address space to CPU address space):\n");
    for (int i = 0; i < 4; i++) {
        if (atmu->piw[i].piwar & WAR_EN) { //window enabled
            printf("\t window %d   %llx -> %llx, AR = %lx\n", i,
                    (((uint64_t) atmu->piw[i].piwbear)<<44) + (((uint64_t) atmu->piw[i].piwbar)<<12),
                    ((uint64_t) atmu->piw[i].pitar)<<12,
                    atmu->piw[i].piwar);
        }
    }
#endif
}

#endif

void pci_init()
{
#ifdef __PPC__
    jet_memory_block_status_t pci_controller_mb, pci_io_mb;

    if(jet_memory_block_get_status("PCI_Express_1", &pci_controller_mb) != POK_ERRNO_OK) {
        abort();
    }

    pci_controller.cfg_addr = pci_controller_mb.addr;
    pci_controller.cfg_data = pci_controller_mb.addr + 4;

    if(jet_memory_block_get_status("PCI_IO", &pci_io_mb) != POK_ERRNO_OK) {
        abort();
    }

    pci_controller.legacy_io_vaddr = pci_io_mb.addr;
    pci_controller.legacy_io_paddr = pci_io_mb.paddr;

    struct pci_atmu_windows *atmu = (struct pci_atmu_windows *)(pci_controller.cfg_addr + PEX_ATMU_SHIFT);
    //pci_ATMU_windows_list();

    atmu->pow[1].powbar = 0x80000000 >> 12;
    atmu->pow[1].potar =  0x80000000 >> 12;
    atmu->pow[1].potear = 0;
    atmu->pow[1].powar = WAR_EN|WAR_RTT_MEM|WAR_WTT_MEM|WAR_OWS_512M;

    atmu->pow[2].powbar = pci_controller.legacy_io_paddr>>12;
    atmu->pow[2].potar = 0;
    atmu->pow[2].potear = 0;
    atmu->pow[2].powar = WAR_EN|WAR_RTT_IO|WAR_WTT_IO|WAR_OWS_16K;


    pci_ATMU_windows_list();

#endif


    printf("PCI initialization using configuration:\n");

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
        for (int i = 0; i < PCI_NUM_RESOURCES; i++) {
            struct pci_resource_config *res = &dev_config->c_resources[i];
            if (res->type == PCI_RESOURCE_TYPE_NONE)
                continue; //skip empty resource


            if (res->type == PCI_RESOURCE_TYPE_BAR_MEM || res->type == PCI_RESOURCE_TYPE_ROM) {
                jet_memory_block_status_t mb;
                pok_ret_t ret_val = jet_memory_block_get_status(res->memblock_name, &mb);
                if (ret_val != POK_ERRNO_OK) {
                    printf("ERROR: Have not found mem block %s\n", res->memblock_name);
                    abort();
                }

                res->pci_addr = mb.paddr;
                printf("%d pci_addr = %llx\n", i, res->pci_addr);
                if (i == PCI_RESOURCE_ROM)
                    res->pci_addr |= PCI_ROM_ADDRESS_ENABLE;
                printf("%d pci_addr = %llx\n", i, res->pci_addr);

                res->vaddr = mb.addr;

                command |= PCI_COMMAND_MEMORY;

            } else if (res->type == PCI_RESOURCE_TYPE_BAR_IO) {
                assert(res->pci_addr != 0); //IO BAR pci_addr should be non zero
#ifdef __PPC__
                res->vaddr = pci_controller.legacy_io_vaddr + res->pci_addr;
#else
                res->vaddr = res->pci_addr;
#endif

                command |= PCI_COMMAND_IO;

            }

            printf("i = %d %x %llx\n", i, pci_resource_address(i), res->pci_addr);
            pci_write_config_dword(&pci_dev, pci_resource_address(i), res->pci_addr);
        }

        if (command != 0)
            pci_write_config_word(&pci_dev, PCI_COMMAND, command);
    }
    printf("PCI init result:\n");
    pci_list();
    printf("\n");
}
