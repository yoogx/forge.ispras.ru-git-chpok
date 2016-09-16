/*
 * Institute for System Programming of the Russian Academy of Sciences
 * Copyright (C) 2016 ISPRAS
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

#ifndef __JA_X86_BSP_BSP_H__
#define __JA_X86_BSP_BSP_H__

#include <arch/interrupt_frame.h>

/* Initialize all board-specific stuff. */
void ja_bsp_init(void);

/*
 * Board-specific header should define:
 * 
 *  - macro EXCEPTION_TIMER as integer constant
 *    It corresponds to interrupt index of the timer.
 */
#include <board/bsp.h>

/* Called when interrupt from the timer occures. */
void ja_bsp_process_timer(interrupt_frame* frame);


#endif /* __JET_X86_BSP_BSP_H__ */
