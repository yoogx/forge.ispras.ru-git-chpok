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

#include <pci.h>
#include <ioports.h>
#include <stdio.h>
#include <byteorder.h>

//TODO DELETE
#include "vbe.h"

#include <fb.h>
struct pci_dev vga_dev;
char vga_initialized = 0;



void vbe_write(uint16_t reg, uint16_t val)
{
    iowrite16(reg, (void *)pci_convert_legacy_port(&vga_dev, VBE_DISPI_IOPORT_INDEX));
    iowrite16(val, (void *)pci_convert_legacy_port(&vga_dev, VBE_DISPI_IOPORT_DATA));
}

#define VGA_PAS 0x20 //palette address source. val 1 enables display.

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define SCREEN_BPP VBE_DISPI_BPP_32

void vga_init()
{
    printf("qemu-vga: initializing\n");

    pci_get_dev_by_bdf(0, 5, 0, &vga_dev);
    if (vga_dev.vendor_id == 0xFFFF) {
        printf("qemu-vga: have not found vga device\n");
        return;
    }

    if (vga_dev.resources[PCI_RESOURCE_BAR0].addr == 0) {
        printf("qemu-vga: BAR0 not mapped\n");
        return;
    }

    if (vga_dev.resources[PCI_RESOURCE_ROM].addr == 0) {
        printf("qemu-vga: ROM not mapped\n");
        return;
    }

    printf("qemu-vga: bios[0] 0x%x\n", ioread8((uint8_t *)vga_dev.resources[PCI_RESOURCE_ROM].addr));
    printf("qemu-vga: bios[1] 0x%x\n", ioread8((uint8_t *)vga_dev.resources[PCI_RESOURCE_ROM].addr + 1));

    vbe_write(VBE_DISPI_INDEX_ENABLE, 0);
    vbe_write(VBE_DISPI_INDEX_XRES, SCREEN_WIDTH);
    vbe_write(VBE_DISPI_INDEX_YRES, SCREEN_HEIGHT);
    vbe_write(VBE_DISPI_INDEX_BPP, SCREEN_BPP);
    vbe_write(VBE_DISPI_INDEX_ENABLE, 1);

    //TODO this is mmio bar
    iowrite8(VGA_PAS, (uint8_t *) pci_convert_legacy_port(&vga_dev, 0x3c0));

    vbe_write(VBE_DISPI_INDEX_VIRT_WIDTH, SCREEN_WIDTH);
    vga_initialized = 1;
}

uint32_t rgba_to_argb(uint32_t rgba_color)
{
    return rgba_color>>8 | (rgba_color&0xff)<<24;
}


#define DEFAULT_FB_DESCRIPTOR 1

int first = 1;
int fb_initialized = 0;

int uwrm_scm_get_direct_fb(struct uwrm_scm_direct_fb * fb)
{
    if (fb_initialized || !vga_initialized)
        return UWRM_ERROR;
    fb_initialized = 1;
    fb->hfb = DEFAULT_FB_DESCRIPTOR;
    fb->back_surface = (uint32_t *)vga_dev.resources[PCI_RESOURCE_BAR0].addr + SCREEN_WIDTH*SCREEN_HEIGHT;
    fb->front_surface = (void *) vga_dev.resources[PCI_RESOURCE_BAR0].addr;
    fb->width = SCREEN_WIDTH;
    fb->height = SCREEN_HEIGHT;
    fb->format = UWRM_FORMAT_ARGB8888;

    return UWRM_OK;
}

int uwrm_scm_fb_swap(struct uwrm_scm_direct_fb *fb)
{
    if (fb->hfb != DEFAULT_FB_DESCRIPTOR)
        return UWRM_ERROR;

    void *tmp = fb->front_surface;
    fb->front_surface = fb->back_surface;
    fb->back_surface = tmp;

    vbe_write(VBE_DISPI_INDEX_Y_OFFSET, SCREEN_HEIGHT*first);

    first = 1-first;

    return UWRM_OK;
}

