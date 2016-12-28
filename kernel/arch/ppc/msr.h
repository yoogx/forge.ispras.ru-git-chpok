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

#ifndef __POK_PPC_MSR_H__
#define __POK_PPC_MSR_H__

//#define MSR_SF_LG       63              /* Enable 64 bit mode */
//#define MSR_ISF_LG      61              /* Interrupt 64b mode valid on 630 */
//#define MSR_HV_LG       60              /* Hypervisor state */
#define MSR_TS_T_LG     34              /* Trans Mem state: Transactional */
#define MSR_TS_S_LG     33              /* Trans Mem state: Suspended */
#define MSR_TS_LG       33              /* Trans Mem state (2 bits) */
#define MSR_TM_LG       32              /* Trans Mem Available */
//#define MSR_VEC_LG      25              /* Enable AltiVec */
#define MSR_SPE_LG      25              /* Enable SPE */
#define MSR_VSX_LG      23              /* Enable VSX */
#define MSR_POW_LG      18              /* Enable Power Management */
#define MSR_WE_LG       18              /* Wait State Enable */
#define MSR_TGPR_LG     17              /* TLB Update registers in use */
#define MSR_CE_LG       17              /* Critical Interrupt Enable */
#define MSR_ILE_LG      16              /* Interrupt Little Endian */
#define MSR_EE_LG       15              /* External Interrupt Enable */
#define MSR_PR_LG       14              /* Problem State / Privilege Level */
#define MSR_FP_LG       13              /* Floating Point enable */
#define MSR_ME_LG       12              /* Machine Check Enable */
#define MSR_FE0_LG      11              /* Floating Exception mode 0 */
#define MSR_SE_LG       10              /* Single Step */
#define MSR_BE_LG       9               /* Branch Trace */
#define MSR_DE_LG       9               /* Debug Exception Enable */
#define MSR_FE1_LG      8               /* Floating Exception mode 1 */
#define MSR_IP_LG       6               /* Exception prefix 0x000/0xFFF */
#define MSR_IR_LG       5               /* Instruction Relocate */
#define MSR_DR_LG       4               /* Data Relocate */
#define MSR_PE_LG       3               /* Protection Enable */
#define MSR_PX_LG       2               /* Protection Exclusive Mode */
#define MSR_PMM_LG      2               /* Performance monitor */
#define MSR_RI_LG       1               /* Recoverable Exception */
#define MSR_LE_LG       0               /* Little Endian */

#define __MASK(X)       (1<<(X))

#define MSR_VEC         __MASK(MSR_VEC_LG)      /* Enable AltiVec */
#define MSR_SPE         __MASK(MSR_SPE_LG)      /* Enable SPE */
#define MSR_VSX         __MASK(MSR_VSX_LG)      /* Enable VSX */
#define MSR_POW         __MASK(MSR_POW_LG)      /* Enable Power Management */
#define MSR_WE          __MASK(MSR_WE_LG)       /* Wait State Enable */
#define MSR_TGPR        __MASK(MSR_TGPR_LG)     /* TLB Update registers in use */
#define MSR_CE          __MASK(MSR_CE_LG)       /* Critical Interrupt Enable */
#define MSR_ILE         __MASK(MSR_ILE_LG)      /* Interrupt Little Endian */
#define MSR_EE          __MASK(MSR_EE_LG)       /* External Interrupt Enable */
#define MSR_PR          __MASK(MSR_PR_LG)       /* Problem State / Privilege Level */
#define MSR_FP          __MASK(MSR_FP_LG)       /* Floating Point enable */
#define MSR_ME          __MASK(MSR_ME_LG)       /* Machine Check Enable */
#define MSR_FE0         __MASK(MSR_FE0_LG)      /* Floating Exception mode 0 */
#define MSR_SE          __MASK(MSR_SE_LG)       /* Single Step */
#define MSR_BE          __MASK(MSR_BE_LG)       /* Branch Trace */
#define MSR_DE          __MASK(MSR_DE_LG)       /* Debug Exception Enable */
#define MSR_FE1         __MASK(MSR_FE1_LG)      /* Floating Exception mode 1 */
#define MSR_IP          __MASK(MSR_IP_LG)       /* Exception prefix 0x000/0xFFF */
#define MSR_IR          __MASK(MSR_IR_LG)       /* Instruction Relocate */
#define MSR_DR          __MASK(MSR_DR_LG)       /* Data Relocate */
#define MSR_PE          __MASK(MSR_PE_LG)       /* Protection Enable */
#define MSR_PX          __MASK(MSR_PX_LG)       /* Protection Exclusive Mode */
#define MSR_PMM         __MASK(MSR_PMM_LG)      /* Performance monitor */
#define MSR_RI          __MASK(MSR_RI_LG)       /* Recoverable Exception */
#define MSR_LE          __MASK(MSR_LE_LG)       /* Little Endian */

#define MSR_TM          __MASK(MSR_TM_LG)       /* Transactional Mem Available */
#define MSR_TS_N        0                       /*  Non-transactional */
#define MSR_TS_S        __MASK(MSR_TS_S_LG)     /*  Transaction Suspended */
#define MSR_TS_T        __MASK(MSR_TS_T_LG)     /*  Transaction Transactional */
#define MSR_TS_MASK     (MSR_TS_T | MSR_TS_S)   /* Transaction State bits */
#define MSR_TM_ACTIVE(x) (((x) & MSR_TS_MASK) != 0) /* Transaction active? */
#define MSR_TM_TRANSACTIONAL(x) (((x) & MSR_TS_MASK) == MSR_TS_T)
#define MSR_TM_SUSPENDED(x)     (((x) & MSR_TS_MASK) == MSR_TS_S)


#define MSR_KERNEL (0) // MSR value on kernel boot.
#define MSR_USER (MSR_EE |  MSR_PR | MSR_FP | MSR_SPE)

#ifndef __ASSEMBLY__ //if this header was not included in asm file

#define mfmsr() ({unsigned long rval; \
        asm volatile("mfmsr %0" : "=r" (rval) : \
                : "memory"); rval;})
#define mtmsr(v) asm volatile("mtmsr %0" : \
        : "r" ((unsigned long)(v)) \
        : "memory")
#endif // __ASSEMBLY__

#endif
