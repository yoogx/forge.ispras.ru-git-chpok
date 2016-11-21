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
 * This file also incorporates work covered by POK License.
 * Copyright (c) 2007-2009 POK team
 */
# ifndef __POK__PCI_INTERNAL_H__
# define __POK__PCI_INTERNAL_H__


/* 
 * These struct is based on http://www.jbox.dk/sanos/source/sys/krnl/pci.c.html 
 * Copyright (C) 2002 Michael Ringgaard. All rights reserved.
 */
const struct {
    int classcode;
    char *name;
} pci_classnames[] = {
    {0x0000, "Non-VGA unclassified device"},
    {0x0001, "VGA compatible unclassified device"},
    {0x0100, "SCSI storage controller"},
    {0x0101, "IDE interface"},
    {0x0102, "Floppy disk controller"},
    {0x0103, "IPI bus controller"},
    {0x0104, "RAID bus controller"},
    {0x0180, "Unknown mass storage controller"},
    {0x0200, "Ethernet controller"},
    {0x0201, "Token ring network controller"},
    {0x0202, "FDDI network controller"},
    {0x0203, "ATM network controller"},
    {0x0204, "ISDN controller"},
    {0x0280, "Network controller"},
    {0x0300, "VGA controller"},
    {0x0301, "XGA controller"},
    {0x0302, "3D controller"},
    {0x0380, "Display controller"},
    {0x0400, "Multimedia video controller"},
    {0x0401, "Multimedia audio controller"},
    {0x0402, "Computer telephony device"},
    {0x0480, "Multimedia controller"},
    {0x0500, "RAM memory"},
    {0x0501, "FLASH memory"},
    {0x0580, "Memory controller"},
    {0x0600, "Host bridge"},
    {0x0601, "ISA bridge"},
    {0x0602, "EISA bridge"},
    {0x0603, "MicroChannel bridge"},
    {0x0604, "PCI bridge"},
    {0x0605, "PCMCIA bridge"},
    {0x0606, "NuBus bridge"},
    {0x0607, "CardBus bridge"},
    {0x0608, "RACEway bridge"},
    {0x0609, "Semi-transparent PCI-to-PCI bridge"},
    {0x060A, "InfiniBand to PCI host bridge"},
    {0x0680, "Bridge"},
    {0x0700, "Serial controller"},
    {0x0701, "Parallel controller"},
    {0x0702, "Multiport serial controller"},
    {0x0703, "Modem"},
    {0x0780, "Communication controller"},
    {0x0800, "PIC"},
    {0x0801, "DMA controller"},
    {0x0802, "Timer"},
    {0x0803, "RTC"},
    {0x0804, "PCI Hot-plug controller"},
    {0x0880, "System peripheral"},
    {0x0900, "Keyboard controller"},
    {0x0901, "Digitizer Pen"},
    {0x0902, "Mouse controller"},
    {0x0903, "Scanner controller"},
    {0x0904, "Gameport controller"},
    {0x0980, "Input device controller"},
    {0x0A00, "Generic Docking Station"},
    {0x0A80, "Docking Station"},
    {0x0B00, "386"},
    {0x0B01, "486"},
    {0x0B02, "Pentium"},
    {0x0B10, "Alpha"},
    {0x0B20, "Power PC"},
    {0x0B30, "MIPS"},
    {0x0B40, "Co-processor"},
    {0x0C00, "FireWire (IEEE 1394)"},
    {0x0C01, "ACCESS Bus"},
    {0x0C02, "SSA"},
    {0x0C03, "USB Controller"},
    {0x0C04, "Fiber Channel"},
    {0x0C05, "SMBus"},
    {0x0C06, "InfiniBand"},
    {0x0D00, "IRDA controller"},
    {0x0D01, "Consumer IR controller"},
    {0x0D10, "RF controller"},
    {0x0D80, "Wireless controller"},
    {0x0E00, "I2O"},
    {0x0F00, "Satellite TV controller"},
    {0x0F01, "Satellite audio communication controller"},
    {0x0F03, "Satellite voice communication controller"},
    {0x0F04, "Satellite data communication controller"},
    {0x1000, "Network and computing encryption device"},
    {0x1010, "Entertainment encryption device"},
    {0x1080, "Encryption controller"},
    {0x1100, "DPIO module"},
    {0x1101, "Performance counters"},
    {0x1110, "Communication synchronizer"},
    {0x1180, "Signal processing controller"},
    {0x0000, NULL}
};


#endif
