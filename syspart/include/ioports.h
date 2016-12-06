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

#ifndef __JET__IOPORTS_H__
#define __JET__IOPORTS_H__

#include <libc/string.h>
#ifdef __PPC__
#include <arch/ppc/ioports.h>
#endif

//#ifdef POK_ARCH_X86
#ifdef __i386__
#include <arch/x86/ioports.h>
#endif

//#ifdef POK_ARCH_SPARC
#ifdef __sparc__
#include <arch/sparc/ioports.h>
#endif

static inline uint8_t  ioread8 (uint8_t  *addr)
{
#ifdef __PPC__
    return in_8(addr);
#endif
}

static inline uint16_t ioread16(uint16_t *addr)
{
#ifdef __PPC__
    return in_le16(addr);
#endif
}

static inline uint32_t ioread32(uint32_t *addr)
{
#ifdef __PPC__
    return in_le32(addr);
#endif
}

static inline void iowrite8(uint8_t value, uint8_t *addr)
{
#ifdef __PPC__
    out_8(addr, value);
#endif
}

static inline void iowrite16(uint16_t value, uint16_t *addr)
{
#ifdef __PPC__
    out_le16(addr, value);
#endif
}

static inline void iowrite32(uint32_t value, uint32_t *addr)
{
#ifdef __PPC__
    out_le32(addr, value);
#endif
}

static inline void memset_io(volatile void *addr, unsigned char val, size_t count)
{
    memset((void *)addr, val, count);
}

static inline void memcpy_fromio(void *dst, const volatile void *src, size_t count)
{
    memcpy(dst, (const void *)src, count);
}

static inline void memcpy_toio(volatile void *dst, const void *src, size_t count)
{
    memcpy((void *)dst, src, count);
}

#endif
