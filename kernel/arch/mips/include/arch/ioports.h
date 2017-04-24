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

#ifndef __JET_MIPS_IOPORTS_H__
#define __JET_MIPS_IOPORTS_H__

#include <stdint.h>

#define JET_ARCH_DECLARE_IO_PORT 1

static inline void outb(unsigned int port, uint8_t value)
{
    *(uint8_t *) port = value;
}

static inline uint8_t inb(unsigned int port)
{
    return *(uint8_t *) port;
}

static inline void outw(unsigned int port, uint16_t value)
{
    *(uint16_t *) port = value;
}

static inline uint16_t inw(unsigned int port)
{
    return *(uint16_t *) port;
}

static inline void outl(unsigned int port, uint32_t value)
{
    *(uint32_t *) port = value;
}

static inline uint32_t inl(unsigned int port)
{
    return *(uint32_t *) port;
}

#endif // __JET_MIPS_IOPORTS_H__
