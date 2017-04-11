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

/*
 * PHYS(va) - translates va (virtual address) to physical address
 * VIRT(pa) - translates pa (physical address) to virtual address
 */
#ifdef __ASSEMBLER__
#define PHYS(va) ((va) - KERNBASE)
#define VIRT(pa) ((pa) + KERNBASE)
#else
#define PHYS(x) ((uintptr_t)(x) - KERNBASE)
#define VIRT(x) ((uintptr_t)(x) + KERNBASE)
#endif
