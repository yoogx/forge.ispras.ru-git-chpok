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

#include <config.h>

#include <bsp_common.h>
#include <libc.h>
#include <errno.h>
#include <core/thread.h>

#include "thread.h"

#ifdef POK_NEEDS_THREADS

uint32_t		ja_context_init (uint32_t sp, void (*entry)(void))
{
  uint32_t id = 0; // Was: thread_id
  context_t* ctx = (context_t*)(sp - sizeof(context_t));

  memset (ctx, 0, sizeof (context_t));

  ctx->r14     = (unsigned long)entry;
  ctx->r15     = id;
  ctx->lr      = (uint32_t) entry;
  ctx->sp      = (uint32_t) &ctx->back_chain;

#ifdef POK_NEEDS_DEBUG
  printf ("ctxt_create %lu: sp=%p entry=%lx\n", (unsigned long) id, ctx, (unsigned long) entry);
#endif

  return (uint32_t)ctx;
}

#endif
