/*
 *                               POK header
 *
 * The following file is a part of the POK project. Any modification should
 * made according to the POK licence. You CANNOT use this file or a part of
 * this file is this part of a file for your own project
 *
 * For more information on the POK licence, please see our LICENCE FILE
 *
 * Please follow the coding guidelines described in doc/CODING_GUIDELINES
 *
 *                                      Copyright (c) 2007-2009 POK team
 *
 * Created by julien on Mon May 18 18:44:51 2009
 */

/**
 * @file
 * @author Laurent
 * @brief  RTL8029 driver
 * @date   PFE GISTR 2010
 */

#include "rtl8029.h"
#ifdef POK_NEEDS_RTL8029

//#include <middleware/port.h>
#include <pci.h>
#include <ioports.h>
#include <net/network.h>

//TODO
# define outb_inverse(a,b) outb((b), (a))

// global since there is no way to get device data otherwise...
static s_ne2000_dev dev;

/*
 * We *always* assume page 0 to be selected.
 * Two exceptions: initialization and polling.
 * Therefore, each time we need to switch to page 1,
 * the card is switched to page 0 again when we're done...
 */
#define NE2000_SELECT_PAGE(dev, page)					\
  outb_inverse((inb((dev)->addr + NE2000_CR) &				\
	~(NE2000_CR_PS0 | NE2000_CR_PS1)) | ((page) << 6), (dev)->addr)
static
int ne2000_write(const s_ne2000_dev* dev,
		     const void*	 buf,
		     unsigned short	 count,
		     unsigned short	 offset)
{
  printf("ne2000_write\n");
  hexdump(buf, count);
  const char* p = NULL;
  int ret = count;

  // Sets RD2 (abort/complete remote DMA)
  outb_inverse((inb((dev)->addr + NE2000_CR) & ~(NE2000_CR_RD0 | NE2000_CR_RD1)) |
       NE2000_CR_RD2, (dev)->addr);

  /* These two registers set the start address of remote DMA. */
  outb_inverse(offset, dev->addr + NE2000_RSAR0);
  outb_inverse(offset >> 8, dev->addr + NE2000_RSAR1);

  /* These two registers set the data byte counts of remote DMA. */
  outb_inverse(count, dev->addr + NE2000_RBCR0);
  outb_inverse(count >> 8, dev->addr + NE2000_RBCR1);

  // Sets RD1 (remote write)
  outb_inverse((inb((dev)->addr + NE2000_CR) & ~(NE2000_CR_RD0 | NE2000_CR_RD2)) |
       NE2000_CR_RD1, (dev)->addr);

  for (p = buf; count > 0; count--, p++)
  {
    outb_inverse(*p, dev->addr + NE2000_DMA_PORT);
  }

  return (ret);
}

static
int ne2000_read(const s_ne2000_dev*	dev,
		    void*		buf,
		    unsigned short	count,
		    unsigned short	offset)
{
  char* p = NULL;
  int ret = count;

  // Sets RD2 (abort/complete remote DMA)
  outb_inverse((inb((dev)->addr + NE2000_CR) & ~(NE2000_CR_RD0 | NE2000_CR_RD1)) |
       NE2000_CR_RD2, (dev)->addr);

  /* These two registers set the start address of remote DMA. */
  outb_inverse(offset, dev->addr + NE2000_RSAR0);
  outb_inverse(offset >> 8, dev->addr + NE2000_RSAR1);

  /* These two registers set the data byte counts of remote DMA. */
  outb_inverse(count, dev->addr + NE2000_RBCR0);
  outb_inverse(count >> 8, dev->addr + NE2000_RBCR1);

  // Sets RD0 (remote read)
  outb_inverse((inb((dev)->addr + NE2000_CR) & ~(NE2000_CR_RD1 | NE2000_CR_RD2)) |
       NE2000_CR_RD0, (dev)->addr);

  for (p = buf; count > 0; count--, p++)
  {
    *p = inb(dev->addr + NE2000_DMA_PORT);
  }

  return (ret);
}


static void set_packet_received_callback(void (*f)(const char *, size_t))
{
    dev.packet_received_callback = f;
}

#if 0
/**
 *  @brief Enqueues a packet in the appropriate queue
 *
 */
static inline
void rtl8029_enqueue (pok_packet_t *packet)
{
  pok_queue_t*	queue = dev.recv_buf + packet->udp.dst;
  uint32_t	off = 0;
  uint32_t	i = 0;

  /* overflow? */
  if (queue->len + packet->udp.len > RECV_BUF_SZ)
  {
    printf("rtl8029_read: error: ring buffer %d overflow!\n", packet->udp.dst);
    return;
  }

  /* at which offset should we start writing? */
  off = (queue->off + queue->len) % RECV_BUF_SZ;

  /* copying data from the packet to the circular buffer in the queue */
  for (i = 0; i < packet->udp.len; i++)
  {
    queue->data[off] = packet->data[i];
    off = (off + 1) % RECV_BUF_SZ;
  }

  /* updating data length in this queue */
  queue->len += packet->udp.len;
}

/**
 *  @brief Reads data from the corresponding network stack
 *
 *  Reads enqueued data in the stack partition.
 */
void rtl8029_read (pok_port_id_t port_id, void* data, uint32_t len)
{
  pok_port_id_t global;
  pok_ret_t     ret;

  ret = pok_port_virtual_get_global (port_id, &global);

  if (ret == POK_ERRNO_OK)
  {
    char	*dest = data;
    pok_queue_t* queue = dev.recv_buf + global;
    uint32_t	size = len < queue->len ? len : queue->len;
    uint32_t	copied = 0;

    printf ("[RTL8029] READ DATA FROM LOCAL PORT %d "
	    "GLOBAL_PORT=%d), size=%d\n", port_id, global, len);

    /* is there something to read ? */
    if (queue->len == 0)
    {
      printf("rtl8029_read: error: empty read ring buffer %d!\n", port_id);
      return;
    }

    /* copy from the queue to the buffer */
    for (copied = 0; copied < size; copied++)
    {
      dest[copied % RECV_BUF_SZ] = queue->data[queue->off];
      queue->off = (queue->off + 1) % RECV_BUF_SZ;
    }

    /* updating data length in this queue */
    queue->len -= size;
  }
}

/**
 *  @brief Send data to the interface
 *
 *  Writes data to be sent to network.
 */
void rtl8029_write (pok_port_id_t port_id, const void* data, uint32_t len)
{
  uint32_t        nbdest;
  uint32_t        tmp;
  uint32_t        dest;
  pok_ret_t       ret;
  char            node2[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
  pok_packet_t    packet;
  const char*     d;
  size_t          cpylen = 0;
  size_t          sndlen = 0;
  unsigned char	state; // ISR state

  ret = pok_port_virtual_nb_destinations (port_id, &nbdest);
  if (ret != POK_ERRNO_OK)
  {
    return;
  }

  for (tmp = 0 ; tmp < nbdest ; tmp++)
  {
    ret = pok_port_virtual_destination (port_id, tmp, &dest);
    if (ret == POK_ERRNO_OK)
    {
      printf ("[RTL8029] SEND DATA THROUGH NETWORK FROM LOCAL PORT %d "
	      "TO GLOBAL PORT %d, size=%d\n", port_id, dest, len);

      memcpy(packet.eth.src, dev.mac, ETH_MAC_LEN);
      memcpy(packet.eth.dst, node2, ETH_MAC_LEN);
      packet.eth.ethertype = 0x4242;
      packet.udp.src = port_id;
      packet.udp.dst = dest;

      for (d = data; len != 0; len -= cpylen, data += cpylen)
      {
	// too short; let's cut
	if (len <= NET_DATA_MINLEN)
	{
	  cpylen = len;
	  sndlen = ETH_DATA_MINLEN + sizeof(eth_hdr_t);
	}
	else
	{
	  // too big; let's pad
	  if (len >= NET_DATA_MAXLEN)
	  {
	    cpylen = NET_DATA_MAXLEN;
	    sndlen = ETH_DATA_MAXLEN + sizeof(eth_hdr_t);
	  }
	  // normal
	  else
	  {
	    cpylen = len;
	    sndlen = sizeof(eth_hdr_t) + sizeof(udp_hdr_t) + cpylen;
	  }
	}

	packet.udp.len = cpylen;
	memcpy(&(packet.data), data, cpylen);

	ne2000_write(&dev, &packet, sndlen, NE2000_TXBUF * 256);

	do
	{
	  state = inb(dev.addr + NE2000_ISR);
	}
	while ((state & NE2000_ISR_RDC) != NE2000_ISR_RDC);

	/* This register sets the start page address of
	   the packet to the transmitted. */
	outb_inverse(NE2000_TXBUF, dev.addr + NE2000_TPSR); //?

	/* These two registers set the byte counts of
	   the packet to be transmitted. */
	outb_inverse(sndlen, dev.addr + NE2000_TBCR0);
	outb_inverse(sndlen >> 8, dev.addr + NE2000_TBCR1);

	/* This bit must be set to transmit a packet. */
	outb_inverse(inb(dev.addr + NE2000_CR) | NE2000_CR_TXP,
	     dev.addr + NE2000_CR);

	outb_inverse(NE2000_ISR_RDC, dev.addr + NE2000_ISR); // Clear RDC bit
      }
    }
  }
}

/**
 *  @brief Polls rtl8029 device.
 *
 *  Watches for events, typically for receiving queued packets.
 */
void rtl8029_polling ()
{
  unsigned char	state; // ISR state

  NE2000_SELECT_PAGE(&dev, 0);

  while (1)
  {
    // do we have an interrupt flag set?
    if ((state = inb(dev.addr + NE2000_ISR)) == 0)
      continue;

    if (state & NE2000_ISR_PRX)
    {
      if ((inb(dev.addr + NE2000_RSR) & NE2000_RSR_PRX) == 0)
      {
	// error
      }

      printf("[*]\n");

      /* no errors */
      s_ne2000_header	ne2000_hdr;	// ne2000 packet header
      unsigned short	offset;		// dma offset
      unsigned char	start, end;	// pointers for the ring buffer
      pok_packet_t	recv_packet;

      while (1)
      {

	/* This register is used to prevent overwrite of the receive buffer ring.
	   It is typically used as a pointer indicating the last receive buffer
	   page the host has read.*/
	start = inb(dev.addr + NE2000_BNRY) + 1;

	/* This register points to the page address of the first receive
	   buffer page to be used for a packet reception. */
	NE2000_SELECT_PAGE(&dev, 1);
	end = inb(dev.addr + NE2000_CURR);
	NE2000_SELECT_PAGE(&dev, 0);

	if ((end % NE2000_MEMSZ) == (start % NE2000_MEMSZ) + 1)
	{
	  break;
	}

	/* et on decapsule! */
	offset = start << 8;
	// ne2000 header
	offset += ne2000_read(&dev, &ne2000_hdr, sizeof(s_ne2000_header),
			      offset);

	ne2000_read(&dev, &recv_packet,
		    ne2000_hdr.size - sizeof(s_ne2000_header), offset);
	rtl8029_enqueue(&recv_packet);

	// update the BNRY register... almost forgot that
	outb_inverse(ne2000_hdr.next > NE2000_MEMSZ ?
	     NE2000_RXBUF - 1 : ne2000_hdr.next - 1, dev.addr + NE2000_BNRY);

      }

      outb_inverse(NE2000_ISR_PRX, dev.addr + NE2000_ISR); // Clear PRX flag
    }

    if (state & NE2000_ISR_PTX)
    {
      outb_inverse(NE2000_ISR_PTX, dev.addr + NE2000_ISR); // Clear PTX flag
    }

    if (state & NE2000_ISR_RXE)
    {
      outb_inverse(NE2000_ISR_RXE, dev.addr + NE2000_ISR); // Clear RXE flag
    }

    if (state & NE2000_ISR_TXE)
    {
      outb_inverse(NE2000_ISR_TXE, dev.addr + NE2000_ISR); // Clear TXE flag
    }

    if (state & NE2000_ISR_OVW)
    {
      outb_inverse(NE2000_ISR_OVW, dev.addr + NE2000_ISR); // Clear OVW flag
    }

    if (state & NE2000_ISR_CNT)
    {
      outb_inverse(NE2000_ISR_CNT, dev.addr + NE2000_ISR); // Clear CNT flag
    }

    if (state & NE2000_ISR_RST)
    {
      outb_inverse(NE2000_ISR_RST, dev.addr + NE2000_ISR); // Clear RST bit
    }
  }
}

#endif

//TODO Temp and dirty. Move to pci.c
static unsigned pci_read(unsigned bus,
                         unsigned dev,
                         unsigned fun,
                         unsigned reg)
{
    unsigned addr = (1 << 31) | (bus << 16) | (dev << 11) | (fun << 8) | (reg & 0xfc);
    unsigned val = -1;
    unsigned cfg_addr = 0xe0008000;
    unsigned cfg_data = 0xe0008004;

#ifdef __PPC__
    out_be32((uint32_t *) cfg_addr, addr);
    val = in_le32((uint32_t *) cfg_data);
#endif

    return (val >> ((reg & 3) << 3));
}
//TODO move to pci.c
static unsigned pci_read_reg(s_pci_device* d,
        unsigned reg)
{
    return (pci_read(d->bus, d->dev, d->fun, reg));
}

static inline
int pci_open(s_pci_device* d)
{
  unsigned int bus = 0;
  unsigned int dev = 0;
  unsigned int fun = 0;

  for (bus = 0; bus < PCI_BUS_MAX; bus++)
    for (dev = 0; dev < PCI_DEV_MAX; dev++)
      for (fun = 0; fun < PCI_FUN_MAX; fun++)
	if (((unsigned short) pci_read(bus, dev, fun, PCI_REG_VENDORID)) == d->vendorid &&
	    ((unsigned short) pci_read(bus, dev, fun, PCI_REG_DEVICEID)) == d->deviceid)
	{
	  // we do not handle type 1 or 2 PCI configuration spaces
	  if (pci_read(bus, dev, fun, PCI_REG_HEADERTYPE) != 0)
	    continue;

	  d->bus = bus;
	  d->dev = dev;
	  d->fun = fun;

#ifdef __PPC__
          d->bar[0] = 0xe1001000;
          printf ("bar0 %lx\n", d->bar[0]);
          {
                uint32_t * cfg_addr = (uint32_t*) 0xe0008000;
                void * cfg_data = (void *) 0xe0008004;

                //TODO call pci write word
                out_be32(cfg_addr, 0x80000810); //write something to BAR0
                out_le16(cfg_data, 0x1001);

                out_be32(cfg_addr, 0x80000804);//write something to COMMAND register
                out_le16(cfg_data, 0x7);
          }
#else
	  d->bar[0] = pci_read(bus, dev, fun, PCI_REG_BAR0);
#endif
          //XXX curently interrupts are unsupported
          //d->irq_line = (unsigned char) pci_read_reg(d, PCI_REG_IRQLINE);

	  return (0);
	}

  return (-1);
}

pok_ret_t pci_register(s_pci_device*	dev)
{
  if (pci_open(dev) != 0)
    return (-1);

  /*
  pok_idt_set_gate(32 + dev->irq_line,
		   GDT_CORE_CODE_SEGMENT,
		   (uint32_t) pci_handler,
		   IDTE_INTERRUPT,
		   0);
  */

  return (0);
}

/**
 *  @brief Initializes rtl8029 device.
 *
 *  Seeks and registers PCI interface, set configuration and fills the
 *  dev structure.
 */
static pok_bool_t rtl8029_init (void)
{
    printf("rtl8029 init called\n");
  dev.pci.vendorid = 0x10ec;
  dev.pci.deviceid = 0x8029;
  dev.pci.io_range = 0x10;

  if (pci_register(&(dev.pci)) != 0)
  {
    printf("rtl8029: PCI init failed!\n");
    return FALSE;
  }

  dev.addr = dev.pci.bar[0] & (~0x1F);
  printf("bar0 addr = 0x%x\n", dev.addr);

  unsigned char	i = 0;
  unsigned char	buf[6 * 2]; // used for MAC address

  NE2000_SELECT_PAGE(&dev, 0);

  /* This bit is the STOP command. When it is set, no packets will be
     received or transmitted. POWER UP=1. */
  outb_inverse(NE2000_CR_STP, dev.addr + NE2000_CR);

  // Sets several options... Read the datasheet!
  outb_inverse(0x00, dev.addr + NE2000_TCR);
  outb_inverse(NE2000_RCR_AB, dev.addr + NE2000_RCR);
  outb_inverse(NE2000_DCR_LS | NE2000_DCR_FT1, dev.addr + NE2000_DCR);

  /* The Page Start register sets the start page address
     of the receive buffer ring. */
  outb_inverse(NE2000_RXBUF, dev.addr + NE2000_PSTART);
  /* The Page Stop register sets the stop page address
     of the receive buffer ring. */
  outb_inverse(NE2000_MEMSZ, dev.addr + NE2000_PSTOP);
  /* This register is used to prevent overwrite of the receive buffer ring.
     It is typically used as a pointer indicating the last receive buffer
     page the host has read. */
  outb_inverse(NE2000_RXBUF, dev.addr + NE2000_BNRY);

  /* These two registers set the data byte counts of remote DMA. */
  outb_inverse(0, dev.addr + NE2000_RBCR0);
  outb_inverse(0, dev.addr + NE2000_RBCR1);

  NE2000_SELECT_PAGE(&dev, 1);

  /* This register points to the page address of the first receive buffer
     page to be used for a packet reception. */
  outb_inverse(NE2000_RXBUF + 1, dev.addr + NE2000_CURR);

  // Init mac address
  /* Here's something I do not understand... Section 6.2.2 of the datasheet
     says bytes 00H-05H of the PROM corresponds to the Ethernet ID. But it
     looks like each byte of the MAC address is written twice...
     Therefore I read 2 * sizeof(mac) and select one of the two bytes
     corresponding to the MAC... Weird... Really... */
  ne2000_read(&dev, buf, 6 * 2, 0);
  for (i = 0; i < 6; i++)
    dev.mac[i] = buf[i * 2];

  /* These registers contain my Ethernet node address and are used to compare
    the destination address of incoming packets for acceptation or rejection.*/
  outb_inverse(dev.mac[0], dev.addr + NE2000_PAR0);
  outb_inverse(dev.mac[1], dev.addr + NE2000_PAR1);
  outb_inverse(dev.mac[2], dev.addr + NE2000_PAR2);
  outb_inverse(dev.mac[3], dev.addr + NE2000_PAR3);
  outb_inverse(dev.mac[4], dev.addr + NE2000_PAR4);
  outb_inverse(dev.mac[5], dev.addr + NE2000_PAR5);

  NE2000_SELECT_PAGE(&dev, 0);

  // Start command
  outb_inverse(NE2000_CR_STA, dev.addr + NE2000_CR);

  // Reactivating interrupts
  /* ISR register must be cleared after power up. */
  outb_inverse(0xFF, dev.addr + NE2000_ISR);
  /* All bits correspond to the bits in the ISR register. POWER UP=all 0s.
     Setting individual bits will enable the corresponding interrupts. */
  /* Since POK use polling, ALL interrupts are disabled */
  outb_inverse(0x00, dev.addr + NE2000_IMR);

  for (i = 0; i < 20; i++) /* TODO: random constant */
  {
    dev.recv_buf[i].len = 0;
    dev.recv_buf[i].off = 0;
  }

  return TRUE;
}

void dummy() {
}

static pok_bool_t send_frame_gather(const pok_network_sg_list_t *sg_list,
                                    size_t sg_list_len,
                                    pok_network_buffer_callback_t callback,
                                    void *callback_arg)
{
    printf("send_frame_gather ... \n");
#if 0
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
#endif


    //for (i = 0; i < sg_list_len; i++) {
    //    sg_list[i].buffer
    //    sg_list[i].size
    //}
    
     
    unsigned char	state; // ISR state
    int i = 0;
    unsigned sndlen = sg_list[i].size;
    {


	ne2000_write(&dev, sg_list[i].buffer, sg_list[i].size, NE2000_TXBUF * 256);
	do
	{
	  state = inb(dev.addr + NE2000_ISR);
	}
	while ((state & NE2000_ISR_RDC) != NE2000_ISR_RDC);

	/* This register sets the start page address of
	   the packet to the transmitted. */
	outb_inverse(NE2000_TXBUF, dev.addr + NE2000_TPSR); //?

	/* These two registers set the byte counts of
	   the packet to be transmitted. */
	outb_inverse(sndlen, dev.addr + NE2000_TBCR0);
	outb_inverse(sndlen >> 8, dev.addr + NE2000_TBCR1);

	/* This bit must be set to transmit a packet. */
	outb_inverse(inb(dev.addr + NE2000_CR) | NE2000_CR_TXP,
	     dev.addr + NE2000_CR);

	outb_inverse(NE2000_ISR_RDC, dev.addr + NE2000_ISR); // Clear RDC bit
    }

    return TRUE;
}

static pok_bool_t send_frame(char *buffer,
                             size_t size,
                             pok_network_buffer_callback_t callback,
                             void *callback_arg)
{
    printf("send_frame\n");
    pok_network_sg_list_t sg_list[1] = {{.buffer=buffer, .size=size}};
    return send_frame_gather(sg_list, 1, callback, callback_arg);
}

static void reclaim_send_buffers(void)
{
    printf("reclaim_send_buffers\n");
#if 0
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
#endif
}

static void reclaim_receive_buffers(void)
{
    printf("reclaim_receive_buffer\n");
#if 0
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
#endif
}

static void flush_send(void)
{
    printf("flush_send\n");
#if 0
    struct virtio_network_device *dev = &virtio_network_device;

    outw(dev->pci_device.bar[0] + VIRTIO_PCI_QUEUE_NOTIFY, (uint16_t) VIRTIO_NETWORK_TX_VIRTQUEUE);
#endif
}

static const pok_network_driver_ops_t driver_ops = {
    .init                         = rtl8029_init,
    .send_frame = send_frame,
    .send_frame_gather = send_frame_gather,
    .set_packet_received_callback = set_packet_received_callback,
    .reclaim_send_buffers = reclaim_send_buffers,
    .reclaim_receive_buffers = reclaim_receive_buffers,
    .flush_send = flush_send,
};

pok_network_driver_device_t pok_network_ne2000_device = {
    .ops = &driver_ops,
    .mac = dev.mac
};

#endif
