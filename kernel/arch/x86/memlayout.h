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

#define KERNBASE 0xc0000000

/* return phys address for virtual addresses in kernel memory */
#ifdef __ASSEMBLER__
#define PHYS_ADDR(x) ((x) - KERNBASE)
#else
#define PHYS_ADDR(x) ((uintptr_t)(x) - KERNBASE)
#endif
