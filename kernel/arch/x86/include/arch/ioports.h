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


#ifndef __JET_X86_IOPORTS_H__
#define __JET_X86_IOPORTS_H__

#include <stdint.h>

static inline void outb(uint16_t port, uint8_t value)
{
    asm volatile("outb %b0,%w1"::"a" (value),"d" (port));
}

static inline uint8_t inb(uint16_t port)
{
    uint8_t res;
    asm volatile("inb %w1,%0" :"=a" (res) :"d" (port));
    return res;
}

static inline void outw(uint16_t port, uint16_t value)
{
    asm volatile("outw %0,%w1"::"a" (value),"d" (port));
}

static inline uint16_t inw(uint16_t port)
{
    uint16_t res;
    asm volatile("inw %w1,%0" :"=a" (res) :"d" (port));
    return res;
}

static inline void outl(uint16_t port, uint8_t value)
{
    asm volatile("outl %0,%w1"::"a" (value),"d" (port));
}

static inline uint32_t inl(uint16_t port)
{
    uint32_t res;
    asm volatile("inl %w1,%0" :"=a" (res) :"d" (port));
    return res;
}

#endif /* __JET_X86_IOPORTS_H__ */
