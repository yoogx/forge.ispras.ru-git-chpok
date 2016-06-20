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
 *
 * Some defines was taken from linux source "include/uapi/linux/pci_regs.h"
 * PCI standard defines
 * Copyright 1994, Drew Eckhardt
 * Copyright 1997--1999 Martin Mares <mj@ucw.cz>
 *
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
#define PCI_VENDOR_ID           0x00    /* 16 bits */
#define PCI_DEVICE_ID           0x02    /* 16 bits */
#define PCI_COMMAND             0x04    /* 16 bits */
#define PCI_CLASS_REVISION      0x08    /* High 24 bits are class, low 8 revision */
#define PCI_REVISION_ID         0x08    /* Revision ID */
#define PCI_CLASS_PROG          0x09    /* Reg. Level Programming Interface */
#define PCI_CLASS_DEVICE        0x0a    /* Device class */
#define PCI_HEADER_TYPE         0x0E
#define  PCI_HEADER_TYPE_NORMAL         0
#define  PCI_HEADER_TYPE_BRIDGE         1
#define  PCI_HEADER_TYPE_CARDBUS        2

/*
 * Base addresses specify locations in memory or I/O space.
 * Decoded size can be determined by writing a value of
 * 0xffffffff to the register, and reading it back.  Only
 * 1 bits are decoded.
 */
#define PCI_BASE_ADDRESS_0      0x10    /* 32 bits */
#define PCI_BASE_ADDRESS_1      0x14    /* 32 bits [htype 0,1 only] */
#define PCI_BASE_ADDRESS_2      0x18    /* 32 bits [htype 0 only] */
#define PCI_BASE_ADDRESS_3      0x1c    /* 32 bits */
#define PCI_BASE_ADDRESS_4      0x20    /* 32 bits */
#define PCI_BASE_ADDRESS_5      0x24    /* 32 bits */

#define  PCI_BASE_ADDRESS_SPACE         0x01    /* 0 = memory, 1 = I/O */
#define  PCI_BASE_ADDRESS_SPACE_IO      0x01
#define  PCI_BASE_ADDRESS_SPACE_MEMORY  0x00
#define  PCI_BASE_ADDRESS_MEM_TYPE_MASK 0x06
#define  PCI_BASE_ADDRESS_MEM_TYPE_32   0x00    /* 32 bit address */
#define  PCI_BASE_ADDRESS_MEM_TYPE_1M   0x02    /* Below 1M [obsolete] */
#define  PCI_BASE_ADDRESS_MEM_TYPE_64   0x04    /* 64 bit address */
#define  PCI_BASE_ADDRESS_MEM_PREFETCH  0x08    /* prefetchable? */
#define  PCI_BASE_ADDRESS_MEM_MASK      (~0x0fUL)
#define  PCI_BASE_ADDRESS_IO_MASK       (~0x03UL)

/*
 * Useful defines...
 */
#  define PCI_BUS_MAX		8
#  define PCI_DEV_MAX		32
#  define PCI_FUN_MAX		8

/* Command regs*/
#define  PCI_COMMAND_IO           0x1     /* Enable response in I/O space */
#define  PCI_COMMAND_MEMORY       0x2     /* Enable response in Memory space */
#define  PCI_COMMAND_MASTER       0x4     /* Enable bus mastering */
#define  PCI_COMMAND_SPECIAL      0x8     /* Enable response to special cycles */
#define  PCI_COMMAND_INVALIDATE   0x10    /* Use memory write and invalidate */
#define  PCI_COMMAND_VGA_PALETTE  0x20   /* Enable palette snooping */
#define  PCI_COMMAND_PARITY       0x40    /* Enable parity checking */
#define  PCI_COMMAND_WAIT         0x80    /* Enable address/data stepping */
#define  PCI_COMMAND_SERR         0x100   /* Enable SERR */
#define  PCI_COMMAND_FAST_BACK    0x200   /* Enable back-to-back writes */
#define  PCI_COMMAND_INTX_DISABLE 0x400 /* INTx Emulation Disable */

//This from wiki.osdev.org/Pci
#define BAR_IOADDR_MASK 0xFFFFFFFC

/*
 * Structure to holds some device information
 */
typedef struct pci_device
{
    uint16_t    bus;
    uint16_t    dev;
    uint16_t    fun;
    uint16_t    vendorid;
    uint16_t    deviceid;
    uint16_t    irq_line;
    uint16_t    io_range;
    uint32_t    bar[6];
    uint32_t    ioaddr;
    void       *irq_handler;
} s_pci_device;

void pci_init(void);
void pci_list(void);

uint32_t pci_read(
        uint32_t bus,
        uint32_t dev,
        uint32_t fun,
        uint32_t reg);

//stupid workaround. pci_write should be added in x86
#ifdef __PPC__

void pci_write_word(s_pci_device *d, uint32_t reg, uint16_t val);

#endif

#if 0
unsigned int pci_read_reg(s_pci_device* d,
			  unsigned int reg);

pok_ret_t pci_register(s_pci_device* dev);

#endif

/* These was got from Linux kernel */
#define PCI_ANY_ID (uint16_t)(~0)

struct pci_device_id {
    uint16_t vendor;
    uint16_t device;
};

struct pci_driver {
    const char *name;
    pok_bool_t (*probe) (struct pci_device *dev);
    const struct pci_device_id *id_table;
};


void register_pci_driver(struct pci_driver *driver);

# endif /* __POK_PCI_H__ */
