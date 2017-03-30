/*
 * Institute for System Programming of the Russian Academy of Sciences
 * Copyright (C) 2017 ISPRAS
 *
 * This program is free software) you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, Version 3.
 *
 * This program is distributed in the hope # that it will be useful,
 * but WITHOUT ANY WARRANTY) without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License version 3 for more details.
 */

#include <board/cpu_config.h>

#if defined(JET_ARM_CONFIG_CORTEX_A7)

#define ARMv7_VIRT_EXTENSION

#elif defined(JET_ARM_CONFIG_CORTEX_A9)


#else

#error "CPU family is not selected"

#endif
