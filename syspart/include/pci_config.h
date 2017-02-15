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
# ifndef __POK__PCI_CONFIG_H__
# define __POK__PCI_CONFIG_H__

#include <pci.h>

struct pci_resource_config {
    uintptr_t vaddr; //virtual addr
    uint64_t  pci_addr; //"physical" addr in pci address space
    char memblock_name[32];
    enum PCI_RESOURCE_TYPE type;
};

struct pci_dev_config
{
    uint16_t    bus;
    uint16_t    dev;
    uint16_t    fn;
    struct pci_resource_config c_resources[PCI_NUM_RESOURCES];
};

extern struct pci_dev_config pci_configs[];
extern unsigned pci_configs_nb;
# endif
