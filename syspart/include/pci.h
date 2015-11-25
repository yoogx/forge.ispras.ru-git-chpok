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
 * Created by laurent on Mon Jun 08 11:03:02 2009 
 */


# ifndef __POK__PCI_H__
# define __POK__PCI_H__

#include <types.h>

/*
 * PCI configuration registers
 */
#  define PCI_CONFIG_ADDRESS	0xCF8
#  define PCI_CONFIG_DATA	0xCFC

/*
 * Configuration space registers
 */
#  define PCI_REG_VENDORID	0x00
#  define PCI_REG_DEVICEID	0x02
#  define PCI_REG_COMMAND       0x04
#  define PCI_REG_HEADERTYPE	0x0E
#  define PCI_REG_BAR0		0x10
#  define PCI_REG_SUBSYSTEM     0x2C
#  define PCI_REG_IRQLINE	0x3C

/*
 * Useful defines...
 */
#  define PCI_BUS_MAX		8
#  define PCI_DEV_MAX		32
#  define PCI_FUN_MAX		8
#  define PCI_COMMAND_IO	0x1	/* Enable response in I/O space */
#  define PCI_COMMAND_MEMORY	0x2	/* Enable response in Memory space */

/*
 * Structure to holds some device information
 */
typedef struct
{
  uint16_t	bus;
  uint16_t	dev;
  uint16_t	fun;
  uint16_t	vendorid;
  uint16_t	deviceid;
  uint16_t	irq_line;
  uint16_t	io_range;
  uint32_t	bar[6];
  uint32_t	addr;
  void*		irq_handler;
} s_pci_device;

#if 0
/*unsigned int pci_read(unsigned int bus,
		      unsigned int dev,
		      unsigned int fun,
		      unsigned int reg);
                      */

//stupid workaround. pci_write should be added in x86
#ifdef POK_ARCH_PPC

void pci_write_word(s_pci_device *d, uint32_t reg, uint16_t val);

#endif

unsigned int pci_read_reg(s_pci_device* d,
			  unsigned int reg);

pok_ret_t pci_register(s_pci_device* dev);

void pok_pci_init();

#endif


# endif /* __POK_PCI_H__ */
