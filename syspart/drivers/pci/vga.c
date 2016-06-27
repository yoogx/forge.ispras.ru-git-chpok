#include <pci.h>
#include <ioports.h>
#include <stdio.h>

//TODO DELETE
#define VGA_ADDR 0xee000008

#include "vbe.h"
void vga_draw(void);

struct gimp_image {
  unsigned int 	 width;
  unsigned int 	 height;
  unsigned int 	 bytes_per_pixel; /* 2:RGB16, 3:RGB, 4:RGBA */ 
  unsigned char	 pixel_data[];
};
extern const struct gimp_image gimp_image;

void vbe_write(struct pci_dev *dev, uint16_t reg, uint16_t val)
{
    (void) dev;
    iowrite16(reg, (void *)pci_convert_legacy_port(dev, VBE_DISPI_IOPORT_INDEX));
    iowrite16(val, (void *)pci_convert_legacy_port(dev, VBE_DISPI_IOPORT_DATA));
}


#define SCREEN_WIDTH 800
#define SCREEN_HIGHT 600
struct pci_dev vga_dev;
void vga_init()
{
    printf("initializing vga\n");

    pci_get_dev_by_bdf(0, 1, 0, &vga_dev);

    printf("vga bios[1] 0x%x\n", ioread8((uint8_t *)vga_dev.resources[6].addr));
    printf("vga bios[1] 0x%x\n", ioread8((uint8_t *)vga_dev.resources[6].addr + 1));

    vbe_write(&vga_dev, VBE_DISPI_INDEX_ENABLE, 0);
    vbe_write(&vga_dev, VBE_DISPI_INDEX_XRES, SCREEN_WIDTH);
    vbe_write(&vga_dev, VBE_DISPI_INDEX_YRES, SCREEN_HIGHT);
    vbe_write(&vga_dev, VBE_DISPI_INDEX_BPP, VBE_DISPI_BPP_16);
    vbe_write(&vga_dev, VBE_DISPI_INDEX_ENABLE, 1);

    //TODO this is mmio bar
    iowrite8(0x20, (uint8_t *) pci_convert_legacy_port(&vga_dev, 0x3c0)); // PAS

    vbe_write(&vga_dev, VBE_DISPI_INDEX_VIRT_WIDTH, SCREEN_WIDTH);

    vga_draw();
}

void vga_draw(void)
{

    /*
       for (int i = 0; i < 800*600; i++) {
       out_be32((uint32_t *) VGA_ADDR + i, 0x00FF0000);
    //    out_be16((uint16_t *) VGA_ADDR + i, 0xe000);
    }*/
    int start = 400;
    for (int y = 0; y < gimp_image.height; y++) {
        for (int x = 0; x < gimp_image.width; x++) {
            //out_be16((uint16_t *) VGA_ADDR + y * 800 + x, 0xe000);
            //for (int i = 0; i<2; i++) {
            //    out_8((uint8_t *) VGA_ADDR + (y*800+x)*2 + i,
            //            gimp_image.pixel_data[(y*gimp_image.width + x)*2 + 1-i]);
            //}
            //This is not IO! TODO: use out_xx16 instead
            iowrite16(
                    ((uint16_t*)gimp_image.pixel_data)[y*gimp_image.width + x],
                    (uint16_t *) VGA_ADDR + (y+start)*SCREEN_WIDTH + x);
        }
    }
}
void vga_set_y_offset(int offset){
    vbe_write(&vga_dev, VBE_DISPI_INDEX_Y_OFFSET, offset);
}
