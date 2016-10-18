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


/* Macros for setting and retrieving special purpose registers */
#ifndef __ASSEMBLY__

#ifndef __stringify
#define __stringify_1(x)        #x
#define __stringify(x)          __stringify_1(x)
#endif



#define mfc0(rn)       ({unsigned long rval; \
                                asm volatile("mfc0 %0, " __stringify(rn) \
                                                                    : "=r" (rval)); rval;})
                                                                    

#define mtc0(v, rn)    asm volatile("mtc0 %0, " __stringify(rn)";"       \
                                             " nop; nop; nop;":          \
                                             : "r" ((unsigned long)(v))  \
                                             : "memory")

#define mfc1(rn)       ({unsigned long rval; \
                                asm volatile("mfc1 %0, " __stringify(rn) \
                                                                    : "=r" (rval)); rval;})
                                                                    
#define mtc1(v, rn)    asm volatile("mtc1 %0, " __stringify(rn)";"       \
                                             " nop;":                    \
                                             : "r" ((unsigned long)(v))  \
                                             : "memory")

#define mfsr()         mfc0(CP0_STATUS)
#define mtsr(v)        mtc0(v, CP0_STATUS)

#endif // __ASSEMBLY__

#endif
