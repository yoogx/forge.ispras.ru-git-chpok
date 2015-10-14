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
 /* Actually, created by Alexey Ovcharov, Tue Oct 13 2015 */

#ifndef __POK_PPC_FPSCR_H__
#define __POK_PPC_FPSCR_H__

#define __MASK_FLIP(X)       (1<<(31-X))

#define FPSCR_FX_LG			0	/* Exception Summary */
#define FPSCR_FEX_LG		1	/* Enabled Exception Summary */
#define FPSCR_VX_LG			2	/* Invalid Operation Exception Summary */
#define FPSCR_OX_LG			3	/* Overflow Exception */
#define FPSCR_UX_LG			4	/* Underflow Exception */
#define FPSCR_ZX_LG			5	/* Zero Divide Exception */
#define FPSCR_XX_LG			6	/* Inexact Exception */
#define FPSCR_VXSNAN_LG		7	/* Invalid Operation Exception (SNaN) */
#define FPSCR_VXISI_LG		8	/* Invalid Operation Exception (Infinity - Infinity) */
#define FPSCR_VXIDI_LG		9	/* Invalid Operation Exception (Infinity / Infinity) */
#define FPSCR_VXZDZ_LG		10	/* Invalid Operation Exception (0 / 0) */
#define FPSCR_VXIMZ_LG		11	/* Invalid Operation Exception (Infinity * 0) */
#define FPSCR_VXVC_LG		12	/* Invalid Operation Exception (Invalid Compare) */
#define FPSCR_FR_LG			13	/* Fraction Rounded */
#define FPSCR_FI_LG			14	/* Fraction Inexact */
#define FPSCR_C_LG			15	/* Result Class Descriptor */
/* Bits 16:19 - Condition Code */
#define FPSCR_FL_LG			16	/* Less Than Or Negative */
#define FPSCR_FG_LG			17	/* Greater Than Or Positive */
#define FPSCR_FE_LG			18	/* Equal Or Zero */
#define FPSCR_FU_LG			19	/* Unordered Or NaN */
/* Bit 20 is reserved */
#define FPSCR_VXSOFT_LG		21	/* Invalid Operation Exception (Software-Defined Condition) */
#define FPSCR_VXSQRT_LG		22	/* Invalid Operation Exception (Invalid Square Root) */
#define FPSCR_VXCVI_LG		23	/* Invalid Operation Exception (Invalid Integer Convert) */
#define FPSCR_VE_LG			24	/* Invalid Operation Exception Enable */
#define FPSCR_OE_LG			25	/* Overflow Exception Enable */
#define FPSCR_UE_LG			26	/* Underflow Exception Enable */
#define FPSCR_ZE_LG			27	/* Zero Divide Exception Enable */
#define FPSCR_XE_LG			28	/* Inexact Exception Enable */
#define FPSCR_NI_LG			29	/* Non-IEEE Mode */
/* 
 * Bits 30:31 - Rounding Control 
 * 00 Round to Nearest
 * 01 Round toward Zero
 * 10 Round toward +Infinity
 * 11 Round toward -Infinity
 */
#define FPSCR_RN_LG			30	/* Rounding Control */


#define FPSCR_VE			__MASK_FLIP(FPSCR_VE_LG)	/* Invalid Operation Exception Enable */
#define FPSCR_OE			__MASK_FLIP(FPSCR_OE_LG)	/* Overflow Exception Enable */
#define FPSCR_UE			__MASK_FLIP(FPSCR_UE_LG)	/* Underflow Exception Enable */
#define FPSCR_ZE			__MASK_FLIP(FPSCR_ZE_LG)	/* Zero Divide Exception Enable */
#define FPSCR_XE			__MASK_FLIP(FPSCR_XE_LG)	/* Inexact Exception Enable */
#define FPSCR_NI			__MASK_FLIP(FPSCR_NI_LG)	/* Non-IEEE Mode */

#define FPSCR_AEE			(FPSCR_VE|FPSCR_OE|FPSCR_UE|FPSCR_ZE|FPSCR_XE)

#endif
