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

#include <string.h>
#include <stdio.h>
#include <pci.h>
#include <ioports.h>

#include <net/byteorder.h>
#include <net/ether.h>
#include <net/ip.h>
#include <net/udp.h>
#include <memory.h>

#include "virtio_config.h"
#include "virtio_ids.h"
#include "virtio_pci.h"
#include "virtio_ring.h"
#include "virtio_virtqueue.h"
#include "virtio_net.h"

#define VIRTIO_PCI_VENDORID 0x1AF4
#define VIRTIO_PCI_DEVICEID_MIN 0x1000
#define VIRTIO_PCI_DEVICEID_MAX 0x103F

#define VIRTIO_NETWORK_RX_VIRTQUEUE 0
#define VIRTIO_NETWORK_TX_VIRTQUEUE 1

// FIXME
#define POK_MAX_RECEIVE_BUFFERS 100

#define PRINTF(fmt, ...) printf("virtio_network: " fmt, ##__VA_ARGS__)

struct virtio_network_device {
    s_pci_device pci_device;

    struct virtio_virtqueue rx_vq, tx_vq;

    uint8_t mac[ETH_ALEN];

    void (*packet_received_callback)(const char *, size_t);
};

struct receive_buffer {
    struct virtio_net_hdr virtio_net_hdr;
    struct ether_hdr ether_hdr;
    char payload[ETH_DATA_LENGTH];
} __attribute__((packed));

static struct receive_buffer receive_buffers[POK_MAX_RECEIVE_BUFFERS];


// the device (statically allocated)
static struct virtio_network_device virtio_network_device;

/*
 * When we're in interrupt context (e.g. system call or timer),
 * preemption is already disabled, and we certainly don't want to
 * enable it there.
 *
 * On the other hand, in the network thread context, preemption is
 * enabled, and we need some critical sections, you know.
 */
static void maybe_lock_preemption(pok_bool_t *saved)
{
    return;
    /*
    *saved = pok_arch_preempt_enabled();
    if (*saved) {
        pok_arch_preempt_disable();
    }
    */
}


static void maybe_unlock_preemption(const pok_bool_t *saved)
{
    return;
    /*
    if (*saved) {
        pok_arch_preempt_enable();
    }
    */
}
//TODO move to pci.c
static unsigned pci_read(unsigned bus,
                         unsigned dev,
                         unsigned fun,
                         unsigned reg)
{
    unsigned addr = (1 << 31) | (bus << 16) | (dev << 11) | (fun << 8) | (reg & 0xfc);
    unsigned val = -1;
    unsigned cfg_addr = 0xe0008000;
    unsigned cfg_data = 0xe0008004;

    out_be32((uint32_t *) cfg_addr, addr);
    val = in_le32((uint32_t *) cfg_data);

    return (val >> ((reg & 3) << 3));
}
//TODO move to pci.c
unsigned pci_read_reg(s_pci_device* d,
        unsigned reg)
{
    return (pci_read(d->bus, d->dev, d->fun, reg));
}

static int virtio_pci_search(s_pci_device* d) {
    // slightly adapted code from pci.c

    unsigned bus = 0;
    unsigned dev = 0;
    unsigned fun = 0;

    //TODO: change to bus_startno, bus_endno
    for (bus = 0; bus < PCI_BUS_MAX; bus++)
        for (dev = 0; dev < PCI_DEV_MAX; dev++)
        {
            uint16_t vendor = (uint16_t) pci_read(bus, dev, 0, PCI_REG_VENDORID);
            if (vendor != VIRTIO_PCI_VENDORID)
                continue;

            uint16_t deviceid = (uint16_t) pci_read(bus, dev, 0, PCI_REG_DEVICEID);
            if (!(deviceid >= VIRTIO_PCI_DEVICEID_MIN && deviceid <= VIRTIO_PCI_DEVICEID_MAX))
                continue;
            
            // we do not handle type 1 or 2 PCI configuration spaces
	    if (pci_read(bus, dev, fun, PCI_REG_HEADERTYPE) != 0)
	        continue;

            uint16_t subsystem = pci_read(bus, dev, 0, PCI_REG_SUBSYSTEM) >> 16;
            if (subsystem != VIRTIO_ID_NET)
                continue;
            
            d->bus = bus;
            d->dev = dev;
            d->fun = fun;
            //d->irq_line = (unsigned char) pci_read_reg(d, PCI_REG_IRQLINE);

#ifdef __PPC__
            d->bar[0] = 0xe1001000;

            //init pci-device
            {
                uint32_t * cfg_addr = (uint32_t*) 0xe0008000;
                void * cfg_data = (void *) 0xe0008004;

                //TODO call pci write word
                out_be32(cfg_addr, 0x80000810);
                out_le16(cfg_data, 0x1001);

                out_be32(cfg_addr, 0x80000804);
                out_le16(cfg_data, 0x7);
            }
#else
    //i386
            d->bar[0] = pci_read(bus, dev, fun, PCI_REG_BAR0) & ~0xFU; // mask lower bits, which mean something else
#endif

            return 0;
        }

    return -1;
}

static void setup_virtqueue(
        struct virtio_network_device *dev, 
        int16_t index, 
        struct virtio_virtqueue *vq)
{
    // queue selector
    outw(dev->pci_device.bar[0] + VIRTIO_PCI_QUEUE_SEL, index);

    // get queue size
    uint16_t queue_size = inw(dev->pci_device.bar[0] + VIRTIO_PCI_QUEUE_NUM);

    // allocate memory and fill in vq fields
    void *mem = virtio_virtqueue_setup(vq, queue_size, VIRTIO_PCI_VRING_ALIGN);

    // give device queue's physical address
    outl(dev->pci_device.bar[0] + VIRTIO_PCI_QUEUE_PFN, pok_virt_to_phys(mem) / VIRTIO_PCI_VRING_ALIGN);
}

static void set_status_bit(s_pci_device *pcidev, uint8_t bit)
{
    bit |= inb(pcidev->bar[0] + VIRTIO_PCI_STATUS);
    outb(pcidev->bar[0] + VIRTIO_PCI_STATUS, bit);
}

static void read_mac_address(struct virtio_network_device *dev)
{
    uint8_t *mac = dev->mac;

    int i;
    for (i = 0; i < ETH_ALEN; i++) {
        mac[i] = inb(dev->pci_device.bar[0] + VIRTIO_PCI_CONFIG_OFF(FALSE) + i);
    }
}

static void use_receive_buffer(struct virtio_network_device *dev, struct receive_buffer *buf)
{
    struct virtio_virtqueue *vq = &dev->rx_vq;

    if (vq->num_free < 1) {
        PRINTF("no free RX descriptors\n");
        return; // FIXME return error code
    }

    vq->num_free--;

    struct vring_desc *desc;
    uint16_t head = vq->free_index;

    desc = &vq->vring.desc[head];
    vq->free_index = desc->next;

    desc->addr = pok_virt_to_phys(buf);
    desc->len = sizeof(*buf);
    desc->flags = VRING_DESC_F_WRITE;

    int avail = vq->vring.avail->idx & (vq->vring.num-1); // wrap around
    vq->vring.avail->ring[avail] = head;
    
    __sync_synchronize();
    
    vq->vring.avail->idx++;
}

// must be called after one or more receive buffers has been added to rx avail. ring
static void notify_receive_buffers(struct virtio_network_device *dev)
{
    outw(dev->pci_device.bar[0] + VIRTIO_PCI_QUEUE_NOTIFY, (uint16_t) VIRTIO_NETWORK_RX_VIRTQUEUE);
}

static void setup_receive_buffers(struct virtio_network_device *dev)
{
    int i;
    for (i = 0; i < POK_MAX_RECEIVE_BUFFERS; i++) {
        // this pushes buffer to avail ring
        use_receive_buffer(dev, &receive_buffers[i]);
    }
    notify_receive_buffers(dev);
}

static void process_received_buffer(
        struct receive_buffer *buf, 
        size_t length)
{
    struct virtio_network_device *dev = &virtio_network_device;

    // FIXME process Ethernet header or not?

    if (dev->packet_received_callback != NULL) {
        dev->packet_received_callback(
            (const char *)&buf->ether_hdr, 
            length - sizeof(struct virtio_net_hdr)
        );
    }
}

/*
 * BEGIN "public" interface
 */

static pok_bool_t init_driver(void)
{
    struct virtio_network_device *dev = &virtio_network_device;

    if (virtio_pci_search(&dev->pci_device) < 0) {
        PRINTF("failed to find the PCI device\n");
        return FALSE;
    }

    // 1. Reset the device
    outb(dev->pci_device.bar[0] + VIRTIO_PCI_STATUS, 0x0);

    // 2. ACK status bit
    set_status_bit(&dev->pci_device, VIRTIO_CONFIG_S_ACKNOWLEDGE);

    // 3. DRIVER status bit
    set_status_bit(&dev->pci_device, VIRTIO_CONFIG_S_DRIVER);

    // 4. Device-specific setup
    setup_virtqueue(dev, VIRTIO_NETWORK_RX_VIRTQUEUE, &dev->rx_vq); 
    setup_virtqueue(dev, VIRTIO_NETWORK_TX_VIRTQUEUE, &dev->tx_vq); 

    virtio_virtqueue_allocate_callbacks(&dev->tx_vq);

    setup_receive_buffers(dev);
    
    //pok_bsp_irq_register(virtio_network_device.pci_device.irq_line, virtio_interrupt_handler);

    // 5. Device feature bits

    uint32_t features = inl(dev->pci_device.bar[0] + VIRTIO_PCI_HOST_FEATURES);
    uint32_t recognized_features = 0;

    if (features & (1 << VIRTIO_NET_F_MAC)) {
        read_mac_address(dev);
        recognized_features |= (1 << VIRTIO_NET_F_MAC);
    } else {
        PRINTF("MAC address is not configured\n");
        set_status_bit(&dev->pci_device, VIRTIO_CONFIG_S_FAILED);
        return FALSE;
    }

    outl(dev->pci_device.bar[0] + VIRTIO_PCI_GUEST_FEATURES, recognized_features);

    // 6. DRIVER_OK status bit
    set_status_bit(&dev->pci_device, VIRTIO_CONFIG_S_DRIVER_OK);

    PRINTF("the device has been successfully initialized\n");

    return TRUE;
}



static pok_bool_t send_frame_gather(const pok_network_sg_list_t *sg_list,
                                    size_t sg_list_len,
                                    pok_network_buffer_callback_t callback,
                                    void *callback_arg)
{
    struct virtio_network_device *dev = &virtio_network_device;
    
    // now, send it to the virtqueue
    struct virtio_virtqueue *vq = &dev->tx_vq;
    if (vq->num_free < sg_list_len) {
        PRINTF("no free TX descriptors\n");
        return FALSE; 
    }

    vq->num_free -= sg_list_len;

    struct vring_desc *desc;
    uint16_t head = vq->free_index;
    
    desc = &vq->vring.desc[head];
    size_t i;
    for (i = 0; i < sg_list_len; i++) {
        if (i > 0) {
            desc = &vq->vring.desc[desc->next];
        }
        desc->addr = pok_virt_to_phys(sg_list[i].buffer);
        desc->len = sg_list[i].size;
        desc->flags = VRING_DESC_F_NEXT;
    }

    desc->flags = 0;
    vq->free_index = desc->next;

    vq->callbacks[head].callback = callback;
    vq->callbacks[head].callback_arg = callback_arg;

    int avail = vq->vring.avail->idx & (vq->vring.num-1); // wrap around
    vq->vring.avail->ring[avail] = head; 
    
    __sync_synchronize();
    
    vq->vring.avail->idx++;

    return TRUE;
}

static pok_bool_t send_frame(char *buffer,
                             size_t size,
                             pok_network_buffer_callback_t callback,
                             void *callback_arg)
{
    pok_network_sg_list_t sg_list[1] = {{.buffer=buffer, .size=size}};
    return send_frame_gather(sg_list, 1, callback, callback_arg);
}

static void set_packet_received_callback(void (*f)(const char *, size_t))
{
    virtio_network_device.packet_received_callback = f;
}

static void reclaim_send_buffers(void)
{
    struct virtio_virtqueue *vq = &virtio_network_device.tx_vq;
    
    // this function can be called by any thread
    // callbacks don't do much work, so we can run them all
    // in single critical section without worrying too much
    
    pok_bool_t saved_preemption;
    maybe_lock_preemption(&saved_preemption);

    while (vq->last_seen_used != vq->vring.used->idx) {
        uint16_t index = vq->last_seen_used & (vq->vring.num-1);
        struct vring_used_elem *e = &vq->vring.used->ring[index];
        struct vring_desc *head = &vq->vring.desc[e->id];
        struct vring_desc *tail = head;

        pok_network_buffer_callback_t cb = vq->callbacks[e->id].callback;
        void *cb_arg = vq->callbacks[e->id].callback_arg;
        cb(cb_arg);

        // reclaim descriptor
        uint16_t total_descriptors = 1;
        while (tail->flags & VRING_DESC_F_NEXT) {
            total_descriptors++;;
            tail = &vq->vring.desc[tail->next];
        }

        vq->num_free += total_descriptors;
        
        // insert chain in the beginning of the free desc. list
        tail->next = vq->free_index; 
        vq->free_index = e->id; // id of head

        vq->last_seen_used++;
    }
    
    maybe_unlock_preemption(&saved_preemption);
}

static void reclaim_receive_buffers(void)
{
    struct virtio_network_device *dev = &virtio_network_device;
    struct virtio_virtqueue *vq = &dev->rx_vq;

    // network thread only
    // TODO also ensure we're not in timer interrupt
    //assert(POK_SCHED_CURRENT_THREAD == NETWORK_THREAD);
        
    pok_bool_t saved_preemption;
    maybe_lock_preemption(&saved_preemption);

    uint16_t old_last_seen_used = vq->last_seen_used;

    while (vq->last_seen_used != vq->vring.used->idx) {
        uint16_t index = vq->last_seen_used & (vq->vring.num-1);
        struct vring_used_elem *e = &vq->vring.used->ring[index];
        struct vring_desc *desc = &vq->vring.desc[e->id];

        struct receive_buffer *buf = pok_phys_to_virt(desc->addr);
        
        process_received_buffer(buf, e->len);

        // reclaim descriptor
        // FIXME support chained descriptors as well
        vq->num_free++;
        desc->next = vq->free_index;
        vq->free_index = e->id;
        
        vq->last_seen_used++;

        // reclaim buffer
        // i.e. push it back to avail. ring
        use_receive_buffer(dev, buf);

        // preemption point
        maybe_unlock_preemption(&saved_preemption);
        maybe_lock_preemption(&saved_preemption);
    }

    if (old_last_seen_used != vq->last_seen_used) {
        // this means we moved at least one buffer from
        // used to avail ring
        notify_receive_buffers(dev);
    }
        
    maybe_unlock_preemption(&saved_preemption);
}

static void flush_send(void)
{
    struct virtio_network_device *dev = &virtio_network_device;

    outw(dev->pci_device.bar[0] + VIRTIO_PCI_QUEUE_NOTIFY, (uint16_t) VIRTIO_NETWORK_TX_VIRTQUEUE);
}

static const pok_network_driver_ops_t driver_ops = {
    .init = init_driver,
    .send_frame = send_frame,
    .send_frame_gather = send_frame_gather,
    .set_packet_received_callback = set_packet_received_callback,
    .reclaim_send_buffers = reclaim_send_buffers,
    .reclaim_receive_buffers = reclaim_receive_buffers,
    .flush_send = flush_send,
};

pok_network_driver_device_t pok_network_virtio_device = {
    .ops = &driver_ops,
    .mac = virtio_network_device.mac
};
