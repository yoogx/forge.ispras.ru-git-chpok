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

#ifndef __POK_IOPORTS_H__
#define __POK_IOPORTS_H__

#include <arch/ioports.h>

#define outb ja_outb
#define inb ja_inb
#define outw ja_outw
#define inw ja_inw
#define outl ja_outl
#define inl ja_inl

#endif /* __POK_IOPORTS_H__ */
