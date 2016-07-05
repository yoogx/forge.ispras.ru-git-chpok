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
void vga_draw(void);

struct pci_dev vga_dev;
char initialized = 0;

struct gimp_image {
  unsigned int  width;
  unsigned int  height;
  unsigned int  bytes_per_pixel; /* 2:RGB16, 3:RGB, 4:RGBA */
  unsigned char pixel_data[];
};

extern const struct gimp_image gimp_image;

void vbe_write(uint16_t reg, uint16_t val)
{
    iowrite16(reg, (void *)pci_convert_legacy_port(&vga_dev, VBE_DISPI_IOPORT_INDEX));
    iowrite16(val, (void *)pci_convert_legacy_port(&vga_dev, VBE_DISPI_IOPORT_DATA));
}

#define VGA_PAS 0x20 //palette address source. val 1 enables display.

#define SCREEN_WIDTH 800
#define SCREEN_HIGHT 600

void vga_init()
{
    printf("initializing vga\n");

    pci_get_dev_by_bdf(0, 1, 0, &vga_dev);
    if (vga_dev.vendor_id == 0xFFFF) {
        printf("have not found vga device\n");
        return;
    }


    printf("vga bios[0] 0x%x\n", ioread8((uint8_t *)vga_dev.resources[6].addr));
    printf("vga bios[1] 0x%x\n", ioread8((uint8_t *)vga_dev.resources[6].addr + 1));

    vbe_write(VBE_DISPI_INDEX_ENABLE, 0);
    vbe_write(VBE_DISPI_INDEX_XRES, SCREEN_WIDTH);
    vbe_write(VBE_DISPI_INDEX_YRES, SCREEN_HIGHT);
    vbe_write(VBE_DISPI_INDEX_BPP, VBE_DISPI_BPP_16);
    vbe_write(VBE_DISPI_INDEX_ENABLE, 1);

    //TODO this is mmio bar
    iowrite8(VGA_PAS, (uint8_t *) pci_convert_legacy_port(&vga_dev, 0x3c0));

    vbe_write(VBE_DISPI_INDEX_VIRT_WIDTH, SCREEN_WIDTH);
    initialized = 1;

    vga_draw();
}

void vga_draw(void)
{
    if (!initialized)
        return;
    uint16_t *addr;
    int start = 400;
    for (int y = 0; y < gimp_image.height; y++) {
        for (int x = 0; x < gimp_image.width; x++) {
            addr = (uint16_t *) vga_dev.resources[PCI_RESOURCE_BAR0].addr + (y+start)*SCREEN_WIDTH + x;
            *addr = le16_to_cpu(((uint16_t*)gimp_image.pixel_data)[y*gimp_image.width + x]);
        }
    }
}

void vga_set_y_offset(int offset)
{
    if (!initialized)
        return;
    vbe_write(VBE_DISPI_INDEX_Y_OFFSET, offset);
}
