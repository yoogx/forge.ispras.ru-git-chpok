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
#define Status_CU2_LG      30              /* Разрешается доступ к сопроцессору 2 */
#define Status_CU1_LG      29              /* Разрешается доступ к сопроцессору 1 */
#define Status_CU0_LG      28              /* Разрешается доступ к сопроцессору 0 */
#define Status_RP_LG       27              /* Зарезервирован. Будет использоваться в дальнейших реализациях.*/
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

#define Cause_IV_LG         23             /* Показывает, использует ли исключение по прерыванию общий вектор исключений или специальный вектор прерываний*/

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

#define CP0_STATUS_XX       __MASK(Status_XX_LG)              /* Разрешает выполнение вещественных инструкций с кодом COP1X*/
#define CP0_STATUS_CU2      __MASK(Status_CU2_LG)             /* Разрешается доступ к сопроцессору 2 */
#define CP0_STATUS_CU1      __MASK(Status_CU1_LG)             /* Разрешается доступ к сопроцессору 1 */
#define CP0_STATUS_CU0      __MASK(Status_CU0_LG)             /* Разрешается доступ к сопроцессору 0 */
#define CP0_STATUS_FP       __MASK(Status_FR_LG)              /* Контролирует режим работы вещественного регистрового файла (регистрового файла CP1)*/
#define CP0_STATUS_PX       __MASK(Status_PX_LG)              /* Обеспечивает доступ к 64-разрядным инструкциям в пользовательском режиме (User)*/
#define CP0_STATUS_BEV      __MASK(Status_BEV_LG)             /* Контролирует размещение векторов исключений*/
#define CP0_STATUS_SR       __MASK(Status_SR_LG)              /* Показывает, что возникло исключение Soft Reset*/
#define CP0_STATUS_NMI      __MASK(Status_NMI_LG)             /* Показывает, что возникло исключение NMI*/
#define CP0_STATUS_CH       __MASK(Status_CH_LG)              /* Указывает на попадание или непопадание в кэш второго уровня для последних
                                                                         инструкций CACHE Hit Invalidate, Hit Write Back Invalidate, или Hit Write Back*/
#define CP0_STATUS_HE       __MASK(Status_HE_LG)              /* Hamming checking Enable – разрешает контроль кода Хемминга в кэш-памяти второго уровня*/
#define CP0_STATUS_HC       __MASK(Status_HC_LG)              /* Hamming Correction enable – разрешает автоматическую коррекцию одиночной ошибки в данных кэш-памяти второго уровня*/
#define CP0_STATUS_IM7      __MASK(Status_IM7_LG)             /* (IM7 -прерывания таймера)*/
#define CP0_STATUS_IM6      __MASK(Status_IM6_LG)             /*         Маски                */
#define CP0_STATUS_IM5      __MASK(Status_IM5_LG)             /*                                  */
#define CP0_STATUS_IM4      __MASK(Status_IM4_LG)             /*             соответствующих          */
#define CP0_STATUS_IM3      __MASK(Status_IM3_LG)             /*                                          */
#define CP0_STATUS_IM2      __MASK(Status_IM2_LG)             /*                       прерываний             */
#define CP0_STATUS_IM1      __MASK(Status_IM1_LG)             /*                                                  */
#define CP0_STATUS_IM0      __MASK(Status_IM0_LG)             /*                              (по номерам)            */
#define CP0_STATUS_KX       __MASK(Status_KX_LG)              /* Определяет режим разрядности при адресации в режиме Kernel*/
#define CP0_STATUS_SX       __MASK(Status_SX_LG)              /* Определяет режим разрядности при адресации в режиме Supervisor*/
#define CP0_STATUS_UX       __MASK(Status_UX_LG)              /* Определяет режим разрядности при адресации в режиме User*/
#define CP0_STATUS_ERL      __MASK(Status_ERL_LG)             /* Error Level. Устанавливается аппаратно при возникновении исключений Reset, Soft Reset, NMI или Cache Error*/
#define CP0_STATUS_EXL      __MASK(Status_EXL_LG)             /* Exception Level. Устанавливается аппаратно, если происходит исключение, отличное от Reset, Soft Reset, NMI или Cache Error*/
#define CP0_STATUS_IE       __MASK(Status_IE_LG)              /* Разрешение прерываний*/

#define CP0_CAUSE_IV        __MASK(Cause_IV_LG)               /* Роказывает, использует ли исключение по прерыванию общий вектор исключений или специальный вектор прерываний*/

#define CP0_INDEX         $0    /* Index       Register*/ /* Номер строки TLB и бит ошибки */    
#define CP0_RANDOM        $1    /* Random      Register*/ /* Псевдослучайное число в диапазоне [63..wired] */
#define CP0_ENTRYLO0      $2    /* EntryLo0    Register*/ /*  */
#define CP0_ENTRYLO1      $3    /* EntryLo1    Register*/ /*  */
#define CP0_CONTEXT       $4    /* Context     Register*/ /* Используется программным обеспечением и есть поле для записи исключений TLB */
#define CP0_PAGEMASK      $5    /* PageMask    Register*/ /* Битовая маска TLB */
#define CP0_WIRED         $6    /* Wired       Register*/ /* Минимальное значение, вырабатываемое в регистре Random */
#define CP0_BadVAddr      $8    /* BadVAddr    Register*/ /* Регистр BadVAddr содержит последний из адресов, вызвавших одно из следующих исключений: 
                                                                                                                     //~ Address error (AdEL or AdES);
                                                                                                                     //~ TLB/XTLB Refill;
                                                                                                                     //~ TLB Invalid (TLBL, TLBS);
                                                                                                                     //~ TLB Modified*/
#define CP0_COUNT         $9    /* Count       Register*/ /* Счетчик тактов процессора*/
#define CP0_ENTRYHI       $10   /* EntryHi     Register*/ /*  */
#define CP0_COMPARE       $11   /* Compare     Register*/ /* Величина для сравнения с регистром Count, когда Count становится равным Compare происходит прерывание IM7 (прерывание по таймеру)*/
#define CP0_STATUS        $12   /* Status      Register*/ /* Описывает состояние системы */
#define CP0_CAUSE         $13   /* Cause       Register*/ /* Информация о прерывании */
#define CP0_EPC           $14   /* Exception Program Counter Register*/ /* Значение счетчика инструкций (виртуальный адрес команды) в момент возникновения исключительной ситуации. */
#define CP0_PRID          $15   /* Processor Identification  Register*/ /* Информация об производителе процессора */
#define CP0_EBASE         $15   /* Exception Base Register*/
#define CP0_CONFIG        $16   /* Configuration Register (select 0)*/ /*  */
#define CP0_CONFIG_1      $16   /* Configuration Register (select 1)*/ /*  */
#define CP0_CONFIG_2      $16   /* Configuration Register (select 2)*/ /*  */
#define CP0_CONFIG_3      $16   /* Configuration Register (select 3)*/ /*  */
#define CP0_LLADDR        $17   /*Load Linked Address  Register*/ /* Это поле содержит разряды 35..4 физического адреса, считанного командой Load Linked. */
#define CP0_WATCHLO       $18   /* WatchLo     Register*/ /* Виртуальный адрес watchpoint'а */
#define CP0_WATCHHI       $19   /* WatchHi     Register*/ /* Настройка watchpoint'а */
#define CP0_XCONTEXT      $20   /* Xcontext    Register*/ /*  */
#define CP0_ChipMemCtrl   $22   /* ChipMemCtrl Register*/ /*  */
#define CP0_DEBUG         $23   /* Debug       Register*/ /* Настройки для debug прерываний */
#define CP0_DEPC          $24   /* DEPC        Register*/ /* Адрес, по которому будет продолжено выполнение программы при выходу из режима DebugMode */
#define CP0_PC_0          $25   /* Performance Counter Register (select 0,2)*/ /*  */
#define CP0_PC_1          $25   /* Performance Counter Register (select 1,3)*/ /*  */
#define CP0_ECC           $26   /* Error Checking and Correction Register*/ /*  */
#define CP0_CACHEERR      $27   /* Cache Error Register*/ /* Информация об ошибках в кэше */
#define CP0_TAGLO         $28   /* TagLo       Register*/ /* Настройки кэша */
#define CP0_TAGHI         $29   /* TagHi       Register (select 0)*/ /* PTag – разряды [35:12] физического адреса. */
#define CP0_DATAHI        $29   /* DataHi      Register (select 1)*/ /* Двойное слово, считанное из кэш-памяти. */
#define CP0_ERROREPC      $30   /* Error Exception Program Counter Register*/ /* Адрес команды, вызвавшей исключение NMI, Reset или Soft Reset */
#define CP0_DESAVE        $31   /* DESAVE      Register*/ /* Регистр для хранения промежуточных данных */




#define CP1_FIR           $0    /* FIR       Register*/ /* Регистр идентификации блока вещественной арифметики или FPU(FPU – Floating Point Unit). */
#define CP1_FCONFIG       $24   /* FCONFIG   Register*/ /* Регистр конфигурации */
#define CP1_FCCR          $25   /* FCCR      Register*/ /* Регистр кодов условий */
#define CP1_FEXR          $26   /* FEXR      Register*/ /* Регистр исключений */
#define CP1_FENR          $28   /* FENR      Register*/ /* Регистр разрешений */
#define CP1_FCSR          $31   /* FCSR      Register*/ /* Регистр состояния */


#define JUMP_TO_REG(reg)   \
        jr  reg;           \
        nop                


#define JUMP_TO_ADDR(addr) \
        j   addr;          \
        nop                

#define JUMP_AND_LINK(addr) \
        jal   addr;         \
        nop                 
        
#define ERET_AND_NOP      \
        eret;             \
        nop;              \
        nop               

#define DERET_AND_NOP     \
        deret;            \
        nop               

#define MTC1(arg1, arg2)  \
        mtc1 arg1, arg2;  \
        nop               

#define MTHi(arg1)        \
        mthi arg1;        \
        nop               
        
#define MTlo(arg1)        \
        mtlo arg1;        \
        nop                 
        
#define MTC0(arg1, arg2)  \
        mtc0 arg1, arg2;  \
        nop;              \
        nop;              \
        nop               



#define MTC0_sel1(arg1, arg2)  \
        mtc0 arg1, arg2, 1;    \
        nop;                   \
        nop;                   \
        nop                    



#endif
