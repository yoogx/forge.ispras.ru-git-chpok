/*  
 *  Copyright (C) 2014 Maxim Malkov, ISPRAS <malkov@ispras.ru> 
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "virtio_network.h"

#include <arch/x86/pci.h>

#include <libc.h>

#ifdef POK_NEEDS_NETWORKING

#define VIRTIO_PCI_VENDORID 0x1AF4
#define VIRTIO_PCI_DEVICEID_MIN 0x1000
#define VIRTIO_PCI_DEVICEID_MAX 0x103F
#define VIRTIO_PCI_NETWORK_SUBSYSTEM_ID 1 // other subsystems are used by other virtio devices

static s_pci_device virtio_network_device;

static int virtio_pci_search(s_pci_device* d) {
    // slightly adapted code from pci.c

    unsigned int bus = 0;
    unsigned int dev = 0;
    unsigned int fun = 0;

    for (bus = 0; bus < PCI_BUS_MAX; bus++)
        for (dev = 0; dev < PCI_DEV_MAX; dev++)
        {
            uint16_t vendor = (uint16_t) pci_read(bus, dev, 0, PCI_REG_VENDORID);
            if (vendor != VIRTIO_PCI_VENDORID) continue;

            uint16_t deviceid = (uint16_t) pci_read(bus, dev, 0, PCI_REG_DEVICEID);
            if (!(deviceid >= VIRTIO_PCI_DEVICEID_MIN && deviceid <= VIRTIO_PCI_DEVICEID_MAX)) continue;
            
            // we do not handle type 1 or 2 PCI configuration spaces
	    if (pci_read(bus, dev, fun, PCI_REG_HEADERTYPE) != 0)
	        continue;

            uint16_t subsystem = pci_read(bus, dev, 0, PCI_REG_SUBSYSTEM) >> 16;

            if (subsystem != VIRTIO_PCI_NETWORK_SUBSYSTEM_ID) continue;
            
            d->bus = bus;
            d->dev = dev;
            d->fun = fun;
            d->bar[0] = pci_read(bus, dev, fun, PCI_REG_BAR0);
            d->irq_line = (unsigned char) pci_read_reg(d, PCI_REG_IRQLINE);

            return 0;
        }

    return -1;
}

void pok_network_init(void)
{
    if (virtio_pci_search(&virtio_network_device) < 0) {
        printf("failed to find virtio network device\n");
        return;
    }

    printf("Found virtio network card on %d:%d\n", (int) virtio_network_device.bus, (int) virtio_network_device.dev);
}

#endif
