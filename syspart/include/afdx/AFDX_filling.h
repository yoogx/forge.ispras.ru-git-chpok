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
*=======================================================================
*
*                   Function that fill the frame 
*
* The following file is a part of the AFDX project. Any modification should
* made according to the AFDX standard.
*
*
* Created by ....
*/

#ifndef __AFDX_FILLING_H_
#define __AFDX_FILLING_H_

#include <afdx/AFDX_ES.h>
#include <afdx/AFDX_frame.h>

#define CARRY_ADD(Result, Value1, Value2) {             \
    uint32_t Sum;                                       \
                                                        \
    Sum = (uint32_t)(Value1) + (uint32_t)(Value2);      \
    (Result) = (Sum & 0xFFFF) + (Sum >> 16);            \
}

#endif
