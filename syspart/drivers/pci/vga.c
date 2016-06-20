#include <pci.h>
#include <ioports.h>
#include <stdio.h>

//TODO DELETE
#define VGA_ADDR 0xee000008

#include "vbe.h"

struct gimp_image {
  unsigned int 	 width;
  unsigned int 	 height;
  unsigned int 	 bytes_per_pixel; /* 2:RGB16, 3:RGB, 4:RGBA */ 
  unsigned char	 pixel_data[];
};
extern const struct gimp_image gimp_image;

void vga_init()
{
#ifdef __PPC__
    printf("initializing vga\n");
    struct pci_device pci_dev;
    pci_dev.bus = 0;
    pci_dev.dev = 1;
    pci_dev.fun = 0;


    out_le16((void *)(0xe1000000 + VBE_DISPI_IOPORT_INDEX), VBE_DISPI_INDEX_ENABLE);
    out_le16((void *)(0xe1000000 + VBE_DISPI_IOPORT_DATA), 0);

    out_le16((void *)(0xe1000000 + VBE_DISPI_IOPORT_INDEX), VBE_DISPI_INDEX_XRES);
    out_le16((void *)(0xe1000000 + VBE_DISPI_IOPORT_DATA), 800);

    out_le16((void *)(0xe1000000 + VBE_DISPI_IOPORT_INDEX), VBE_DISPI_INDEX_YRES);
    out_le16((void *)(0xe1000000 + VBE_DISPI_IOPORT_DATA), 600);

    out_le16((void *)(0xe1000000 + VBE_DISPI_IOPORT_INDEX), VBE_DISPI_INDEX_BPP);
    out_le16((void *)(0xe1000000 + VBE_DISPI_IOPORT_DATA), VBE_DISPI_BPP_16);

    out_le16((void *)(0xe1000000 + VBE_DISPI_IOPORT_INDEX), VBE_DISPI_INDEX_ENABLE);
    out_le16((void *)(0xe1000000 + VBE_DISPI_IOPORT_DATA), 1);

    out_8((void *) 0xe10003c0, 0x20); // PAS

    /*
       for (int i = 0; i < 800*600; i++) {
       out_be32((uint32_t *) VGA_ADDR + i, 0x00FF0000);
    //    out_be16((uint16_t *) VGA_ADDR + i, 0xe000);
    }*/
    for (int y = 0; y < gimp_image.height; y++) {
        for (int x = 0; x < gimp_image.width; x++) {
            //out_be16((uint16_t *) VGA_ADDR + y * 800 + x, 0xe000);
            //for (int i = 0; i<2; i++) {
            //    out_8((uint8_t *) VGA_ADDR + (y*800+x)*2 + i,
            //            gimp_image.pixel_data[(y*gimp_image.width + x)*2 + 1-i]);
            //}
            out_le16((uint16_t *) VGA_ADDR + y*800+x,
                    ((uint16_t*)gimp_image.pixel_data)[y*gimp_image.width + x]);
        }
    }
#endif
}
