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

/*
 * Unlike to other headers in asp, functions in which should be defined
 * in arch-dependent code and used by arch-independent one,
 * this header declares functions which should be defined in
 * arch-independent code and called from arch-dependent one at specific
 * execution points.
 */

#ifndef __JET_ASP_ENTRIES_H__
#define __JET_ASP_ENTRIES_H__

/**
 * Starts the kernel.
 *
 * This function should be called after everything is ready for
 * start OS.
 */
void jet_boot(void);

/* 
 * Initialize main and debug consoles with their default values (provided by arch).
 * 
 * Any stream which is not supported by arch is set to 'jet_stream_null'.
 * 
 * Should be called before 'jet_boot'.
 */
void jet_console_init_all(void);

/* Should be called on timer tick with interrupts disabled. */
void jet_on_tick(void);


#endif /* __JET_ASP_ENTRIES_H__ */
