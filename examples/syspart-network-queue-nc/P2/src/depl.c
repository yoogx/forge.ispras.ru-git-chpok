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

struct pci_dev_config pci_configs[] = {
    {
        .bus = 0,
        .dev = 2,
        .fn  = 0,
        .resources = {
            [PCI_RESOURCE_BAR0] = {
                .addr = 0xe1001000,
                .pci_addr = 0x1000,
                .type = PCI_RESOURCE_TYPE_BAR_IO
            }
        }
    },
};
unsigned pci_configs_nb = 1;
