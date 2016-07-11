#include <stdio.h>
#include <string.h>
#include <pci.h>


#define RADEON_BUS 1
#define RADEON_DEV 0
#define RADEON_FUN 0

struct pci_dev radeon_dev;
uint8_t *radeon_bios;

#define RBIOS8(i) (radeon_bios[i])
#define RBIOS16(i) (RBIOS8(i) | (RBIOS8((i)+1) << 8))

int radeon_tst_atom_magic_simple(int b, int d, int f)
{
    int val1 = 0, val2 = 0, res = -1;
    size_t size = 0;
    uint16_t tmp;
    uint16_t bios_header_start;


    printf("radeon ATOM test. pci(%d:%d:%d)\n", b, d, f);

    memset(&radeon_dev, 0x0, sizeof(radeon_dev));

    pci_get_dev_by_bdf(b, d, f, &radeon_dev);
    if (radeon_dev.vendor_id == 0xffff) {
        printf("radeon test: Have not found device\n");
        return res;
    }

    printf("radeon test: vendor: 0x%04x\n", radeon_dev.vendor_id);
    printf("radeon test: device: 0x%04x\n", radeon_dev.device_id);

    radeon_bios = (uint8_t*)radeon_dev.resources[PCI_RESOURCE_ROM].addr;
    size = radeon_dev.resources[PCI_RESOURCE_ROM].size;
    if (radeon_bios == NULL) {
        printf("radeon test: Wrong ROM address\n");
        return res;
    }

    if (size >= 2) {
        val1 = ioread8(radeon_bios);
        val2 = ioread8(radeon_bios + 1);

        printf("radeon test: rom(%p, %d): 0x%x 0x%x\n",radeon_bios, size, val1, val2);
    } else {
        printf("radeon test: error: invalid rom size\n");

        return res;
    }

    tmp = RBIOS16(0x18);

    if (RBIOS8(tmp + 0x14) != 0x0) {
        printf("Not an x86 BIOS ROM\n");
    }

    bios_header_start = RBIOS16(0x48);
    tmp = bios_header_start + 4;

    if (!memcmp(radeon_bios + tmp, "ATOM", 4) ||
            !memcmp(radeon_bios + tmp, "MOTA", 4)) {
        printf("ATOM BIOS detected\n");

        res = 0;
    } else {
        printf("Fail. BIOS not detected\n");
    }

    return res;
}

int p3041_test()
{
    return radeon_tst_atom_magic_simple(RADEON_BUS, RADEON_DEV, RADEON_FUN);
}
