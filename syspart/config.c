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

#include <depl.h>
#include <pci.h>
enum {
    PCI_DRIVER_TABLE_SIZE = 10,
    DYNAMIC_MEMORY_SIZE = 0x100000,
};

struct pci_driver pci_driver_table[PCI_DRIVER_TABLE_SIZE];
unsigned pci_driver_table_size = PCI_DRIVER_TABLE_SIZE;


char dynamic_memory[DYNAMIC_MEMORY_SIZE];
unsigned dynamic_memory_size = DYNAMIC_MEMORY_SIZE;
