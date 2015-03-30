#ifndef __POK_PPC_TIMER_H__
#define __POK_PPC_TIMER_H__

/* Bit definitions related to the TCR. */
#define TCR_WP(x)       (((x)&0x3)<<30) /* WDT Period */
#define TCR_WP_MASK     TCR_WP(3)
#define WP_2_17         0               /* 2^17 clocks */
#define WP_2_21         1               /* 2^21 clocks */
#define WP_2_25         2               /* 2^25 clocks */
#define WP_2_29         3               /* 2^29 clocks */
#define TCR_WRC(x)      (((x)&0x3)<<28) /* WDT Reset Control */
#define TCR_WRC_MASK    TCR_WRC(3)
#define WRC_NONE        0               /* No reset will occur */
#define WRC_CORE        1               /* Core reset will occur */
#define WRC_CHIP        2               /* Chip reset will occur */
#define WRC_SYSTEM      3               /* System reset will occur */
#define TCR_WIE         0x08000000      /* WDT Interrupt Enable */
#define TCR_PIE         0x04000000      /* PIT Interrupt Enable */
#define TCR_DIE         TCR_PIE         /* DEC Interrupt Enable */
#define TCR_FP(x)       (((x)&0x3)<<24) /* FIT Period */
#define TCR_FP_MASK     TCR_FP(3)
#define FP_2_9          0               /* 2^9 clocks */
#define FP_2_13         1               /* 2^13 clocks */
#define FP_2_17         2               /* 2^17 clocks */
#define FP_2_21         3               /* 2^21 clocks */
#define TCR_FIE         0x00800000      /* FIT Interrupt Enable */
#define TCR_ARE         0x00400000      /* Auto Reload Enable */

/* Bit definitions for the TSR. */
#define TSR_ENW         0x80000000      /* Enable Next Watchdog */
#define TSR_WIS         0x40000000      /* WDT Interrupt Status */
#define TSR_WRS(x)      (((x)&0x3)<<28) /* WDT Reset Status */
#define WRS_NONE        0               /* No WDT reset occurred */
#define WRS_CORE        1               /* WDT forced core reset */
#define WRS_CHIP        2               /* WDT forced chip reset */
#define WRS_SYSTEM      3               /* WDT forced system reset */
#define TSR_PIS         0x08000000      /* PIT Interrupt Status */
#define TSR_DIS         TSR_PIS         /* DEC Interrupt Status */
#define TSR_FIS         0x04000000      /* FIT Interrupt Status */

#endif
