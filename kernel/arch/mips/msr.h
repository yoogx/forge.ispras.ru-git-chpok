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
#define MSR_VEC_LG      25              /* Enable AltiVec */
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


#define Status_XX_LG       31              /* Разрешает выполнение вещественных инструкций с кодом COP1X*/
#define Status_FR_LG       26              /* Контролирует режим работы вещественного регистрового файла (регистрового файла CP1)*/
#define Status_PX_LG       23              /* Обеспечивает доступ к 64-разрядным инструкциям в пользовательском режиме (User)*/
#define Status_BEV_LG      22              /* Контролирует размещение векторов исключений*/
#define Status_SR_LG       20              /* Показывает, что возникло исключение Soft Reset*/
#define Status_NMI_LG      19              /* Показывает, что возникло исключение NMI*/
#define Status_CH_LG       18              /* Указывает на попадание или непопадание в кэш второго уровня для последних
                                                                    инструкций CACHE Hit Invalidate, Hit Write Back Invalidate, или Hit Write Back*/
#define Status_HE_LG       17              /* Hamming checking Enable – разрешает контроль кода Хемминга в кэш-памяти второго уровня*/
#define Status_HC_LG       16              /* Hamming Correction enable – разрешает автоматическую коррекцию одиночной ошибки в данных кэш-памяти второго уровня*/
#define Status_IM7_LG      15              /* (IM7 -прерывания таймера)*/
#define Status_IM6_LG      14              /*                              */
#define Status_IM5_LG      13              /*             Номера               */
#define Status_IM4_LG      12              /*                                      */
#define Status_IM3_LG      11              /*                  соответствующих         */
#define Status_IM2_LG      10              /*                                              */
#define Status_IM1_LG       9              /*                              прерываний          */
#define Status_IM0_LG       8              /*                                                      */
#define Status_KX_LG        7              /* Определяет режим разрядности при адресации в режиме Kernel*/
#define Status_SX_LG        6              /* Определяет режим разрядности при адресации в режиме Supervisor*/
#define Status_UX_LG        5              /* Определяет режим разрядности при адресации в режиме User*/
#define Status_ERL_LG       2              /* Error Level. Устанавливается аппаратно при возникновении исключений Reset, Soft Reset, NMI или Cache Error*/
#define Status_EXL_LG       1              /* Exception Level. Устанавливается аппаратно, если 
                                                                            происходит исключение, отличное от Reset, Soft Reset, NMI или Cache Error*/
#define Status_IE_LG        0              /* Разрешение прерываний*/


#define __MASK(X)       (1<<(X))

#define MSR_VEC         __MASK(MSR_VEC_LG)      /* Enable AltiVec */
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

#define MSR_KERNEL (MSR_ME|MSR_RI|MSR_IR|MSR_DR)
#define MSR_USER (MSR_KERNEL|MSR_PR|MSR_EE)

#define Status_XX       __MASK(Status_XX_LG)              /* Разрешает выполнение вещественных инструкций с кодом COP1X*/
#define Status_FP       __MASK(Status_FR_LG)              /* Контролирует режим работы вещественного регистрового файла (регистрового файла CP1)*/
#define Status_PX       __MASK(Status_PX_LG)              /* Обеспечивает доступ к 64-разрядным инструкциям в пользовательском режиме (User)*/
#define Status_BEV      __MASK(Status_BEV_LG)             /* Контролирует размещение векторов исключений*/
#define Status_SR       __MASK(Status_SR_LG)              /* Показывает, что возникло исключение Soft Reset*/
#define Status_NMI      __MASK(Status_NMI_LG)             /* Показывает, что возникло исключение NMI*/
#define Status_CH       __MASK(Status_CH_LG)              /* Указывает на попадание или непопадание в кэш второго уровня для последних
                                                                    инструкций CACHE Hit Invalidate, Hit Write Back Invalidate, или Hit Write Back*/
#define Status_HE       __MASK(Status_HE_LG)              /* Hamming checking Enable – разрешает контроль кода Хемминга в кэш-памяти второго уровня*/
#define Status_HC       __MASK(Status_HC_LG)              /* Hamming Correction enable – разрешает автоматическую коррекцию одиночной ошибки в данных кэш-памяти второго уровня*/
#define Status_IM7      __MASK(Status_IM7_LG)             /* (IM7 -прерывания таймера)*/
#define Status_IM6      __MASK(Status_IM6_LG)             /*         Маски                */
#define Status_IM5      __MASK(Status_IM5_LG)             /*                                  */
#define Status_IM4      __MASK(Status_IM4_LG)             /*             соответствующих          */
#define Status_IM3      __MASK(Status_IM3_LG)             /*                                          */
#define Status_IM2      __MASK(Status_IM2_LG)             /*                       прерываний             */
#define Status_IM1      __MASK(Status_IM1_LG)             /*                                                  */
#define Status_IM0      __MASK(Status_IM0_LG)             /*                              (по номерам)            */
#define Status_KX       __MASK(Status_KX_LG)              /* Определяет режим разрядности при адресации в режиме Kernel*/
#define Status_SX       __MASK(Status_SX_LG)              /* Определяет режим разрядности при адресации в режиме Supervisor*/
#define Status_UX       __MASK(Status_UX_LG)              /* Определяет режим разрядности при адресации в режиме User*/
#define Status_ERL      __MASK(Status_ERL_LG)             /* Error Level. Устанавливается аппаратно при возникновении исключений Reset, Soft Reset, NMI или Cache Error*/
#define Status_EXL      __MASK(Status_EXL_LG)             /* Exception Level. Устанавливается аппаратно, если 
                                                                            происходит исключение, отличное от Reset, Soft Reset, NMI или Cache Error*/
#define Status_IE       __MASK(Status_IE_LG)              /* Разрешение прерываний*/





#endif
