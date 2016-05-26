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

#include <bsp_common.h>
#include <libc.h>
#include <errno.h>
#include <core/thread.h>

#include "gdt.h"

#include "thread.h"

#ifdef POK_NEEDS_THREADS

uint32_t		ja_context_init (uint32_t sp, void (*entry)(void))
{
  start_context_t* start_ctx;

  start_ctx = (start_context_t *) (sp - 4 - sizeof (start_context_t));

  memset (start_ctx, 0, sizeof (start_context_t));

  start_ctx->ctx.__esp   = (uint32_t) (&start_ctx->ctx.eip); /* for pusha */
  start_ctx->ctx.eip     = (uint32_t) entry;
  start_ctx->ctx.cs      = GDT_CORE_CODE_SEGMENT << 3;
  start_ctx->ctx.eflags  = 1 << 9;

  start_ctx->entry       = 0; /* Was: entry */
  start_ctx->id          = 0; /* Was: thread_id */

  return ((uint32_t)start_ctx);
}


void			ja_context_switch (uint32_t* old_sp,
                                uint32_t new_sp);
asm (".global ja_context_switch	\n"
     "ja_context_switch:		\n"
     "pushf				\n"
     "pushl %cs				\n"
     "pushl $1f				\n"
     "pusha				\n"
     "movl 48(%esp), %ebx		\n" /* 48(%esp) : &old_sp, 52(%esp) : new_sp */
     "movl %esp, (%ebx)			\n"
     "movl 52(%esp), %esp		\n"
     "popa				\n"
     "iret				\n"
     "1:				\n"
     "ret"
     );

#endif
