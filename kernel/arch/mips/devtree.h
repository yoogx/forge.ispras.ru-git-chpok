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

#ifndef __POK_DEVTREE_H__
#define __POK_DEVTREE_H__

extern uint32_t devtree_address; 
void devtree_dummy_dump();

struct pci_bridge_props {
    uint32_t bus_startno;
    uint32_t bus_endno;
    uint32_t cfg_addr;
    uint32_t cfg_data;
};

struct pci_bridge_props devtree_get_pci_props();


#endif
