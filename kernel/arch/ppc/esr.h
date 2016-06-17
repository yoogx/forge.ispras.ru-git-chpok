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

#ifndef __POK_PPC_ESR_H__
#define __POK_PPC_ESR_H__

#define __MASK_FLIP(X)       (1<<(31-(X)))

#define ESR_PIL_LG			4	/* Illegal Instruction exception */
#define ESR_PPR_LG			5	/* Privileged Instruction exception */
#define ESR_PTR_LG			6	/* Trap exception */
#define ESR_FP_LG			7	/* Floating-Point operation */
#define ESR_ST_LG			8	/* Store operation */
/* Bit 9 is reserved */
#define ESR_DLK0_LG			10	/* Cache Locking */
#define ESR_DLK1_LG			11	/* Cache Locking */
#define ESR_AP_LG			12	/* Auxiliary Processor operation */
#define ESR_PUO_LG			13	/* Unimplemented Operation exception */
#define ESR_BO_LG			14	/* Byte Ordering exception */
#define ESR_PIE_LG			15	/* Imprecise exception */


#define ESR_PIL				__MASK_FLIP(ESR_PIL_LG)		/* Illegal Instruction exception */
#define ESR_PPR				__MASK_FLIP(ESR_PPR_LG)		/* Privileged Instruction exception */
#define ESR_PTR				__MASK_FLIP(ESR_PTR_LG)		/* Trap exception */
#define ESR_FP				__MASK_FLIP(ESR_FP_LG)		/* Floating-Point operation */
#define ESR_ST				__MASK_FLIP(ESR_ST_LG)		/* Store operation */
#define ESR_PUO				__MASK_FLIP(ESR_PUO_LG)		/* Unimplemented Operation exception */
#define ESR_BO				__MASK_FLIP(ESR_BO_LG)		/* Byte Ordering exception */
#define ESR_PIE				__MASK_FLIP(ESR_PIE_LG)		/* Imprecise exception */

#endif
