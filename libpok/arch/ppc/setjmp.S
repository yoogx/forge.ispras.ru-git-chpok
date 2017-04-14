/* longjmp for PowerPC.
   Copyright (C) 1995-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.
   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#define LOAD_GP(N)    lwz r##N,((JB_GPRS+(N)-14)*4)(r3)
#define SAVE_GP(N)    stw r##N,((JB_GPRS+(N)-14)*4)(3)
#define JB_GPR1   0  /* Also known as the stack pointer */
#define JB_GPRS   3  /* GPRs 14 through 31 are saved, 18*2 words total */
#define JB_LR     2  /* The address we will return to */
#define JB_CR     21 /* Condition code registers */

.globl lja_longjmp
.globl lja_setjmp

lja_longjmp:
    lwz r1,(JB_GPR1*4)(r3)
    lwz r0,(JB_LR*4)(r3)
    LOAD_GP (14)
    LOAD_GP (15)
    LOAD_GP (16)
    LOAD_GP (17)
    LOAD_GP (18)
    LOAD_GP (19)
    LOAD_GP (20)

    /* longjmp/longjmp_target probe expects longjmp first argument (4@3),
    second argument (-4@4), and target address (4@0), respectively. */

    mtlr r0
    LOAD_GP (21)
    LOAD_GP (22)
    lwz r5,(JB_CR*4)(r3)
    LOAD_GP (23)
    LOAD_GP (24)
    LOAD_GP (25)
    mtcrf 0xFF,r5
    LOAD_GP (26)
    LOAD_GP (27)
    LOAD_GP (28)
    LOAD_GP (29)
    LOAD_GP (30)
    LOAD_GP (31)
    mr r3,r4
    blr

lja_setjmp:
    stw  r1,(JB_GPR1*4)(3)
    mflr r0

    /* setjmp probe expects longjmp first argument (4@3), second argument
    (-4@4), and target address (4@0), respectively. */

    SAVE_GP (14)
    stw  r0,(JB_LR*4)(3)
    SAVE_GP (15)
    mfcr r0
    SAVE_GP (16)
    stw  r0,(JB_CR*4)(3)
    SAVE_GP (17)
    SAVE_GP (18)
    SAVE_GP (19)
    SAVE_GP (20)
    SAVE_GP (21)
    SAVE_GP (22)
    SAVE_GP (23)
    SAVE_GP (24)
    SAVE_GP (25)
    SAVE_GP (26)
    SAVE_GP (27)
    SAVE_GP (28)
    SAVE_GP (29)
    SAVE_GP (30)
    SAVE_GP (31)
    li   r3,0
    blr
