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
 */

#ifndef __VBE_H__
#define __VBE_H__

#define VBE_DISPI_IOPORT_INDEX 0x01CE
#ifdef __PPC__
#define VBE_DISPI_IOPORT_DATA 0x01D0
#else
#define VBE_DISPI_IOPORT_DATA 0x01CF
#endif

enum {
    VBE_DISPI_INDEX_ID,
    VBE_DISPI_INDEX_XRES,
    VBE_DISPI_INDEX_YRES,
    VBE_DISPI_INDEX_BPP,
    VBE_DISPI_INDEX_ENABLE,
    VBE_DISPI_INDEX_BANK,
    VBE_DISPI_INDEX_VIRT_WIDTH,
    VBE_DISPI_INDEX_VIRT_HEIGHT,
    VBE_DISPI_INDEX_X_OFFSET,
    VBE_DISPI_INDEX_Y_OFFSET,
};

enum {
    VBE_DISPI_BPP_4 = 0x04,
    VBE_DISPI_BPP_8 = 0x08,
    VBE_DISPI_BPP_15 = 0x0F,
    VBE_DISPI_BPP_16 = 0x10,
    VBE_DISPI_BPP_24 = 0x18,
    VBE_DISPI_BPP_32 = 0x20,
};
#endif //__VBE_H__
