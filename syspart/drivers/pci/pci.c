

struct pci_bridge_props bridge_props;

void pok_pci_init()
{
    bridge_props = devtree_get_pci_props();

    printf("bridge\n\tcfg_addr: 0x%lx\n\tcfg_data: 0x%lx\n",
            bridge_props.cfg_addr, bridge_props.cfg_data);
}

void pci_list()
{
    //move to pci_init
    {
        uint32_t * cfg_addr = (uint32_t*) 0xe0008000;
        void * cfg_data = (void *) 0xe0008004;

        //TODO call pci write word
        out_be32(cfg_addr, 0x80000810); //write something to BAR0
        out_le16(cfg_data, 0x1001);

        out_be32(cfg_addr, 0x80000804);//write something to COMMAND register
        out_le16(cfg_data, 0x7);
    }

    for (unsigned int bus = 0; bus < PCI_BUS_MAX; bus++)
        for (unsigned int dev = 0; dev < PCI_DEV_MAX; dev++)
            for (unsigned int fun = 0; fun < PCI_FUN_MAX; fun++)
            {
                unsigned vendor = pci_read(bus, dev, fun, PCI_REG_VENDORID);

                if (vendor != 0xffffffff) {
                    printf("****0x%08x:0x%08x(0x%x) == 0x%x ***\n",
                            vendor,
                            pci_read(bus, dev, fun, PCI_REG_DEVICEID),
                            pci_read(bus, dev, fun, PCI_REG_HEADERTYPE),
                            pci_read(bus, dev, fun, PCI_REG_BAR0));
                }
            }

}

unsigned int pci_read(
        unsigned int bus,
        unsigned int dev,
        unsigned int fun,
        unsigned int reg)
{
    unsigned int addr = (1 << 31) | (bus << 16) | (dev << 11) | 
        (fun << 8) | (reg & 0xfc);
    unsigned int val = -1;

#ifdef __PPC__
    out_be32((uint32_t *) bridge_props.cfg_addr, addr);
    val = in_le32((uint32_t *) bridge_props.cfg_data);
#else
    outl(PCI_CONFIG_ADDRESS, addr);
    val = inl(PCI_CONFIG_DATA);
#endif

    return (val >> ((reg & 3) << 3));
}


unsigned int pci_read_reg(s_pci_device* d, unsigned int reg)
{
    return (pci_read(d->bus, d->dev, d->fun, reg));
}


void pci_write_word(s_pci_device *d, uint32_t reg, uint16_t val)
{
    uint32_t addr = (1 << 31) | (d->bus << 16) | (d->dev << 11) | 
        (d->fun << 8) | (reg & 0xfc);

    out_be32((uint32_t *) bridge_props.cfg_addr, addr);
    out_le16((uint16_t *) bridge_props.cfg_data, val);
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
          //TODO

	  //d->bar[0] = pci_read(bus, dev, fun, PCI_REG_BAR0);
	  //d->irq_line = (unsigned char) pci_read_reg(d, PCI_REG_IRQLINE);

	  return (0);
	}

  return (-1);
}


//Maybe some day we will implement interrupts. Then this can be usefull
#if 0 
void dummy_pci_handler(void)
{
  __asm__ volatile
    (
     ".globl pci_handler\n"
     "pci_handler:\n"
     "push %eax\n"		// save restricted context
     "push %edx\n"
     "mov $0x20, %al\n"
     "mov $0xA0, %dx\n"		// ack slave pic
     "outb %al, %dx\n"
     "mov $0x20, %dx\n"		// ack master pic
     "outb %al, %dx\n"
     "pop %edx\n"		// restore retricted context
     "pop %eax\n"
     "iret\n"			// return
    );
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
#endif
