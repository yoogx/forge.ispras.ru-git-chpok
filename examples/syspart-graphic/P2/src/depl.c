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
#include <pci_config.h>

#include <utils.h>

struct pci_dev_config pci_configs[] = {
    {
        .bus = 0,
        .dev = 5,
        .fn  = 0,
        .c_resources = {
            [PCI_RESOURCE_BAR0] = {
                .memblock_name = "PCI_DEV_VGA_BAR0",
                .type = PCI_RESOURCE_TYPE_BAR_MEM
            },
            [PCI_RESOURCE_ROM] = {
                .memblock_name = "PCI_DEV_VGA_ROM",
                .type = PCI_RESOURCE_TYPE_ROM
            }
        }
    }
    /*
    ,{
        .bus = 1,
        .dev = 0,
        .fn  = 0,
        .c_resources = {
            [PCI_RESOURCE_BAR0] = {
                .addr = 0x20000000,
                .pci_addr = 0, //leave unmodified (as set by u-boot)
                .type = PCI_RESOURCE_TYPE_BAR_MEM
            },
            [PCI_RESOURCE_BAR2] = {
                .addr = 0x30000000,
                .pci_addr = 0, //leave unmodified (as set by u-boot)
                .type = PCI_RESOURCE_TYPE_BAR_MEM
            },
            [PCI_RESOURCE_ROM] = {
                .addr =     0x30020000,
                .pci_addr = 0x90020000,
                .type = PCI_RESOURCE_TYPE_ROM
            }
        }
    },
    */
};

unsigned pci_configs_nb = ARRAY_SIZE(pci_configs);
