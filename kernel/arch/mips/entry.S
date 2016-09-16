
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
 *
 * This file also incorporates work covered by POK License.
 * Copyright (c) 2007-2009 POK team
 */




        .section ".start", "ax"
    
    base = .
    
        .globl _pok_reset
    system_reset_0100:
    _pok_reset:
        
        sync;                   
    
        .section ".bss", "aw"
    pok_stack:
        .global pok_stack_end
        .space 8 * 1024
    pok_stack_end:
    
    pok_save_area:
       .space 4 * 8 

