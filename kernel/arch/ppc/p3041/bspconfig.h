#ifndef BSPCONFIG_H
#define BSPCONFIG_H

#define POK_BSP_P3041

// FIXME on P3041 after uboot CCSRBAR points to 64bit address
//                   0xFFE000000ULL
#define CCSRBAR_BASE 0x0FE000000ULL

#endif
