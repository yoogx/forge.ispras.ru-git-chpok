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

#ifndef __POK_PPC_IOPORTS_H__
#define __POK_PPC_IOPORTS_H__

#include <stdint.h>
#include <arch/linux_io.h>

/// Template starts

static inline void ja_outb(unsigned int port, uint8_t value)
{
    out_8((uint8_t *) port, value);
}

static inline uint8_t ja_inb(unsigned int port)
{
    return in_8((uint8_t *) port);
}

static inline void ja_outw(unsigned int port, uint16_t value)
{
    out_le16((uint16_t *) port, value);
}

static inline uint16_t ja_inw(unsigned int port)
{
    return in_le16((uint16_t *) port);
}

static inline void ja_outl(unsigned int port, uint32_t value)
{
    out_le32((uint32_t *) port, value);
}

static inline uint32_t ja_inl(unsigned int port)
{
    return in_le32((uint32_t *) port);
}

/// Template ends

#endif // __POK_PPC_IOPORTS_H__
