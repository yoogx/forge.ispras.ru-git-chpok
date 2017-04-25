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

#ifndef __JET_MIPS_FP_REGISTERS_H__
#define __JET_MIPS_FP_REGISTERS_H__

#include <asp/space.h>

struct jet_fp_store
{  
  double fp_regs[32];
  uint32_t fir;
  uint32_t fconfig;
  uint32_t fcsr;
};

#endif /* __JET_MIPS_FP_REGISTERS_H__ */
