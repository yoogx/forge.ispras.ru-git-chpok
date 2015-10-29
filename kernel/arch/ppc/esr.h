/*
 *                               POK header
 * 
 * The following file is a part of the POK project. Any modification should
 * made according to the POK licence. You CANNOT use this file or a part of
 * this file is this part of a file for your own project
 *
 * For more information on the POK licence, please see our LICENCE FILE
 *
 * Please follow the coding guidelines described in doc/CODING_GUIDELINES
 *
 *                                      Copyright (c) 2007-2009 POK team 
 *
 * Created by julien on Thu Jan 15 23:34:13 2009 
 */
 /* Actually, created by Alexey Ovcharov, Thu Oct 29 2015 */

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
