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

//#ifdef POK_ARCH_PPC
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
