/*
 * Institute for System Programming of the Russian Academy of Sciences
 * Copyright (C) 2017 ISPRAS
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

#include "cpu.h"
#ifdef JET_ARM_CONFIG_FPU_NEON
.fpu neon
#endif

    .globl ja_fp_init
ja_fp_init:
    mov pc, lr

    .globl ja_fp_restore
ja_fp_restore:
    vldm r0!, {d0-d15}
#ifdef JET_ARM_CONFIG_FPU_NEON
    vldm r0!, {d16-d31}
#endif
    //restore FPSCR
    ldm r0!, {r3}
    VMSR FPSCR, r3

    mov pc, lr

    .globl ja_fp_save
ja_fp_save:
    vstm r0!, {d0-d15}
#ifdef JET_ARM_CONFIG_FPU_NEON
    vstm r0!, {d16-d31}
#endif
    //save FPSCR
    VMRS r3, FPSCR
    stm r0!, {r3}

    mov pc, lr


#define FPEXC_EN (1<<30)
    .globl floating_point_enable
floating_point_enable:
    ldr r3, =FPEXC_EN
    VMSR FPEXC, r3
    mov pc, lr
