/*
 *                               POK header
 * 
 * The following file is a part of the POK project. Any modification should
 * made according to the POK licence. You CANNOT use this file or a part of
 * this file is this part of a file for your own project
 *
 * For more information on the POK licence, please see our LICENCE FILE
 *
 * Please follow the coding guidelines described in doc/CODING_GUIDELINES
 *
 *                                      Copyright (c) 2007-2009 POK team 
 *
 * Created by julien on Thu Jan 15 23:34:13 2009 
 */

/**
 **\file    thread.c
 **\brief   x86-dependent code for thread management
 **\author  Julian Pidancet
 */

#include <config.h>
#include <asp/arch.h>

#include <libc.h>
#include <errno.h>
#include <asp/cswitch.h>

#include "gdt.h"

#include "thread.h"

struct jet_context* ja_context_init (jet_stack_t sp, void (*entry)(void))
{
    struct jet_context* ctx = (struct jet_context*)(sp - sizeof(*ctx));

    memset (ctx, 0, sizeof (*ctx));
    /*
    * TODO: Current implementation of context switching requires
    * Interrupt Enabled flag to be specified when context for the switch
    * is created.
    * 
    * Currently we use hardcoded guess, that flag for the switched context
    * should be same as current flag.
    * 
    * In the future Interrupt Enabled flag should be same as one for
    * context we switched from.
    */
    ctx->eflags  = ja_preempt_enabled()? 1 << 9 : 0;
    ctx->ebp = sp;
    ctx->ret = (uint32_t)entry;

    return ctx;
}
