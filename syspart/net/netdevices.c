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

#include <net/network.h>
#include <string.h>
#include <stdio.h>

//TODO move to config.c
enum {
    NETDEVICE_TBL_SIZE = 10,
};

struct netdevices_tbl_entry {
    char *name;
    pok_netdevice_t *dev;
};

struct netdevices_tbl_entry netdevices_tbl[NETDEVICE_TBL_SIZE];
unsigned netdevices_tbl_size = NETDEVICE_TBL_SIZE ;


unsigned netdevices_cnt = 0;
void register_netdevice(char *name, pok_netdevice_t *dev)
{
    printf("registering %s device \n", name);
    //TODO NAME
    netdevices_tbl[netdevices_cnt].name = name;
    netdevices_tbl[netdevices_cnt].dev  = dev;

    netdevices_cnt++;
}

pok_netdevice_t* get_netdevice(char *name)
{
    for (int i = 0; i < netdevices_cnt; i++) {
        if (strcmp(name, netdevices_tbl[i].name) == 0) {
            return netdevices_tbl[i].dev;
        }
    }

    //printf("%s: no netdevice\n", __func__);
    return NULL;
}
