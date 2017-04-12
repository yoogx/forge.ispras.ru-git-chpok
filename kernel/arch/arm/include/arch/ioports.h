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

#include <stdint.h>

static inline void iowrite32(uintptr_t reg, uint32_t data)
{
    *(volatile uint32_t *)reg = data;
}

static inline uint32_t ioread32(uintptr_t reg)
{
    return *(volatile uint32_t *)reg;
}

static inline void iowrite16(uintptr_t reg, uint16_t data)
{
    *(volatile uint16_t *)reg = data;
}

static inline uint16_t ioread16(uintptr_t reg)
{
    return *(volatile uint16_t *)reg;
}

static inline void iowrite8(uintptr_t reg, uint8_t data)
{
    *(volatile uint8_t *)reg = data;
}

static inline uint8_t ioread8(uintptr_t reg)
{
    return *(volatile uint8_t *)reg;
}
