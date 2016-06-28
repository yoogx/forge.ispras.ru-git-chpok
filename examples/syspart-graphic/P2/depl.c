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
#include <depl.h>
#include <channel_driver.h>
#include <net/network.h>
#include <pci_config.h>

const uint32_t pok_network_ip_address;
unsigned sys_queuing_channels_nb;
unsigned sys_sampling_channels_nb;
unsigned channel_drivers_nb;
char *ipnet_netdev_name;
unsigned mac_addr_mapping_nb;


sys_channel_t sys_queuing_channels[] = {};
sys_channel_t sys_sampling_channels[] = {};
struct mac_ip mac_addr_mapping[] = {};
channel_driver_t * channel_drivers[] = {};

struct pci_dev_config pci_configs[] = {
    {
        .bus = 0,
        .dev = 1,
        .fn  = 0,
        .resources = {
            [PCI_RESOURCE_BAR0] = {
                .addr = 0xee000000,
                .type = PCI_RESOURCE_TYPE_BAR_MEM
            },
            [PCI_RESOURCE_ROM] = {
                .addr = 0xedf00000,
                .type = PCI_RESOURCE_TYPE_ROM
            }
        }
    }
};
unsigned pci_configs_nb = ARRAY_SIZE(pci_configs);
