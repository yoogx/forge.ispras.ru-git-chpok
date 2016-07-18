#ifndef __POK_PPC_REG_H__
#define __POK_PPC_REG_H__

#define SPRN_DEC        0x016   /* Decrement Register */
#define SPRN_TBRL       0x10C   /* Time Base Read Lower Register (user, R/O) */
#define SPRN_TBRU       0x10D   /* Time Base Read Upper Register (user, R/O) */
#define SPRN_TBWL       0x11C   /* Time Base Lower Register (super, R/W) */
#define SPRN_TBWU       0x11D   /* Time Base Upper Register (super, R/W) */
#define SPRN_TSR        0x150   /* Timer Status Register */
#define SPRN_TCR        0x154   /* Timer Control Register */

#define SPRN_IVPR       0x03F   /* Interrupt Vector Prefix Register */

#define SPRN_IVOR0      0x190   /* Interrupt Vector Offset Register 0 */
#define SPRN_IVOR1      0x191   /* Interrupt Vector Offset Register 1 */
#define SPRN_IVOR2      0x192   /* Interrupt Vector Offset Register 2 */
#define SPRN_IVOR3      0x193   /* Interrupt Vector Offset Register 3 */
#define SPRN_IVOR4      0x194   /* Interrupt Vector Offset Register 4 */
#define SPRN_IVOR5      0x195   /* Interrupt Vector Offset Register 5 */
#define SPRN_IVOR6      0x196   /* Interrupt Vector Offset Register 6 */
#define SPRN_IVOR7      0x197   /* Interrupt Vector Offset Register 7 */
#define SPRN_IVOR8      0x198   /* Interrupt Vector Offset Register 8 */
#define SPRN_IVOR9      0x199   /* Interrupt Vector Offset Register 9 */
#define SPRN_IVOR10     0x19a   /* Interrupt Vector Offset Register 10 */
#define SPRN_IVOR11     0x19b   /* Interrupt Vector Offset Register 11 */
#define SPRN_IVOR12     0x19c   /* Interrupt Vector Offset Register 12 */
#define SPRN_IVOR13     0x19d   /* Interrupt Vector Offset Register 13 */
#define SPRN_IVOR14     0x19e   /* Interrupt Vector Offset Register 14 */
#define SPRN_IVOR15     0x19f   /* Interrupt Vector Offset Register 15 */
#define SPRN_IVOR38     0x1b0   /* Interrupt Vector Offset Register 38 */
#define SPRN_IVOR39     0x1b1   /* Interrupt Vector Offset Register 39 */
#define SPRN_IVOR40     0x1b2   /* Interrupt Vector Offset Register 40 */
#define SPRN_IVOR41     0x1b3   /* Interrupt Vector Offset Register 41 */

#define SPRN_CSRR0      0x03A   /* Critical Save and Restore Register 0 */
#define SPRN_CSRR1      0x03B   /* Critical Save and Restore Register 1 */
#define SPRN_DEAR       0x03D   /* Data Error Address Register */
#define SPRN_ESR        0x03E   /* Exception Syndrome Register */
#define SPRN_PIR        0x11E   /* Processor Identification Register */
#define SPRN_DBSR       0x130   /* Debug Status Register */
#define SPRN_DBCR0      0x134   /* Debug Control Register 0 */
#define SPRN_DBCR1      0x135   /* Debug Control Register 1 */
#define SPRN_IAC1       0x138   /* Instruction Address Compare 1 */
#define SPRN_IAC2       0x139   /* Instruction Address Compare 2 */
#define SPRN_DAC1       0x13C   /* Data Address Compare 1 */
#define SPRN_DAC2       0x13D   /* Data Address Compare 2 */
#define SPRN_TSR        0x150   /* Timer Status Register */
#define SPRN_TCR        0x154   /* Timer Control Register */

#define SPRN_TLB0CFG    0x2B0   /* TLB 0 Config Register */
#define SPRN_TLB1CFG    0x2B1   /* TLB 1 Config Register */
#define SPRN_TLB2CFG    0x2B2   /* TLB 2 Config Register */
#define SPRN_TLB3CFG    0x2B3   /* TLB 3 Config Register */

#define SPRN_PID        0x030   /* Process ID */

#define SPRN_MAS0       0x270   /* MMU Assist Register 0 */
#define SPRN_MAS1       0x271   /* MMU Assist Register 1 */
#define SPRN_MAS2       0x272   /* MMU Assist Register 2 */
#define SPRN_MAS3       0x273   /* MMU Assist Register 3 */
#define SPRN_MAS4       0x274   /* MMU Assist Register 4 */
#define SPRN_MAS5       0x153   /* MMU Assist Register 5 */
#define SPRN_MAS6       0x276   /* MMU Assist Register 6 */
#define SPRN_MAS7       0x3B0   /* MMU Assist Register 7 */


#define CP0_Index            0x00     /* Index       Register*/ /* Номер строки TLB и бит ошибки */
#define CP0_Random           0x01     /* Random      Register*/ /* Псевдослучайное число в диапазоне [63..wired] */
#define CP0_EntryLo0         0x02     /* EntryLo0    Register*/ /*  */
#define CP0_EntryLo1         0x03     /* EntryLo1    Register*/ /*  */
#define CP0_Context          0x04     /* Context     Register*/ /* Используется программным обеспечением и есть поле для записи исключений TLB */
#define CP0_PageMask         0x05     /* PageMask    Register*/ /* Битовая маска TLB */
#define CP0_Wired            0x06     /* Wired       Register*/ /* Минимальное значение, вырабатываемое в регистре Random */
#define CP0_BadVAddr         0x08     /* BadVAddr    Register*/ /* Регистр BadVAddr содержит последний из адресов, вызвавших одно из следующих исключений: 
                                                                                                                            Address error (AdEL or AdES);
                                                                                                                            TLB/XTLB Refill;
                                                                                                                            TLB Invalid (TLBL, TLBS);
                                                                                                                            TLB Modified*/
#define CP0_Count            0x09     /* Count       Register*/ /* Счетчик тактов процессора*/
#define CP0_EntryHi          0x0A     /* EntryHi     Register*/ /*  */
#define CP0_Compare          0x0B     /* Compare     Register*/ /* Величина для сравнения с регистром Count */
#define CP0_Status           0x0C     /* Status      Register*/ /* Описывает состояние системы */
#define CP0_Cause            0x0D     /* Cause       Register*/ /* Информация о прерывании */
#define CP0_EPC              0x0E     /* Exception Program Counter Register*/ /* Значение счетчика инструкций (виртуальный адрес команды) в момент возникновения исключительной ситуации. */
#define CP0_PI               0x0F     /* Processor Identification  Register*/ /* Информация об производителе процессора */
#define CP0_Config_0         0x10     /* Configuration Register (select 0)*/ /*  */
#define CP0_Config_1         0x10     /* Configuration Register (select 1)*/ /*  */
#define CP0_Config_2         0x10     /* Configuration Register (select 2)*/ /*  */
#define CP0_Config_3         0x10     /* Configuration Register (select 3)*/ /*  */
#define CP0_LLA              0x11     /*Load Linked Address  Register*/ /* Это поле содержит разряды 35..4 физического адреса, считанного командой Load Linked. */
#define CP0_WatchLo          0x12     /* WatchLo     Register*/ /* Виртуальный адрес watchpoint'а */
#define CP0_WatchHi          0x13     /* WatchHi     Register*/ /* Настройка watchpoint'а */
#define CP0_Xcontext         0x14     /* Xcontext    Register*/ /*  */
#define CP0_ChipMemCtrl      0x16     /* ChipMemCtrl Register*/ /*  */
#define CP0_Debug            0x17     /* Debug       Register*/ /* Настройки для debug прерываний */
#define CP0_DEPC             0x18     /* DEPC        Register*/ /* Адрес, по которому будет продолжено выполнение программы при выходу из режима DebugMode */
#define CP0_PC_0             0x19     /* Performance Counter Register (select 0,2)*/ /*  */
#define CP0_PC_1             0x19     /* Performance Counter Register (select 1,3)*/ /*  */
#define CP0_ECC              0x1A     /* Error Checking and Correction Register*/ /*  */
#define CP0_CacheErr         0x1B     /* Cache Error Register*/ /* Информация об ошибках в кэше */
#define CP0_TagLo            0x1C     /* TagLo       Register*/ /* Настройки кэша */
#define CP0_TagHi            0x1D     /* TagHi       Register (select 0)*/ /* PTag – разряды [35:12] физического адреса. */
#define CP0_TagHi            0x1D     /* DataHi      Register (select 1)*/ /* Двойное слово, считанное из кэш-памяти. */
#define CP0_EEPC             0x1E     /* Error Exception Program Counter Register*/ /* Адрес команды, вызвавшей исключение NMI, Reset или Soft Reset */
#define CP0_DESAVE           0x1F     /* DESAVE      Register*/ /* Регистр для хранения промежуточных данных */



#define CP1_FIR              0x00     /* FIR       Register*/ /* Регистр идентификации блока вещественной арифметики или FPU(FPU – Floating Point Unit). */
#define CP1_FCONFIG          0x18     /* FCONFIG   Register*/ /* Регистр конфигурации */
#define CP1_FCCR             0x19     /* FCCR      Register*/ /* Регистр кодов условий */
#define CP1_FEXR             0x1A     /* FEXR      Register*/ /* Регистр исключений */
#define CP1_FENR             0x1C     /* FENR      Register*/ /* Регистр разрешений */
#define CP1_FCSR             0x1F     /* FCSR      Register*/ /* Регистр состояния */



/* Macros for setting and retrieving special purpose registers */
#ifndef __ASSEMBLY__

#ifndef __stringify
#define __stringify_1(x)        #x
#define __stringify(x)          __stringify_1(x)
#endif

#define mfmsr()         ({unsigned long rval; \
                                asm volatile("mfmsr %0" : "=r" (rval) : \
                                                                                    : "memory"); rval;})
#define mtmsr(v)        asm volatile("mtmsr %0" : \
                                             : "r" ((unsigned long)(v)) \
                                             : "memory")

#define mfspr(rn)       ({unsigned long rval; \
                                asm volatile("mfspr %0," __stringify(rn) \
                                                                    : "=r" (rval)); rval;})
#define mtspr(rn, v)    asm volatile("mtspr " __stringify(rn) ",%0" : \
                                             : "r" ((unsigned long)(v)) \
                                             : "memory")
                                             


#define mfc0(rn)       ({unsigned long rval; \
                                asm volatile("mfc0 %0," __stringify(rn) \
                                                                    : "=r" (rval)); rval;})
                                                                    
#define mtc0(v, rn)    asm volatile("mtс0 %0," __stringify(rn) : \
                                             : "r" ((unsigned long)(v)) \
                                             : "memory")

#define mfc1(rn)       ({unsigned long rval; \
                                asm volatile("mfc1 %0," __stringify(rn) \
                                                                    : "=r" (rval)); rval;})
                                                                    
#define mtc1(v, rn)    asm volatile("mtс1 %0," __stringify(rn) : \
                                             : "r" ((unsigned long)(v)) \
                                             : "memory")

#define mfsr()         mfc0(CP0_Status)
#define mtsr(v)        mtc0(v, CP0_Status)

#endif // __ASSEMBLY__

#endif
