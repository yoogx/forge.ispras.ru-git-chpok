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

#include <libc.h>
#include <errno.h>
#include <core/thread.h>

#include "context.h"

struct jet_context* ja_context_init (jet_stack_t sp, void (*entry)(void))
{
  //~ uint32_t id = 0; // Was: thread_id
  //~ struct jet_stack_frame_null* stack_frame_null =
    //~ (struct jet_stack_frame_null*)(sp - sizeof(*stack_frame_null));

  struct jet_context* ctx = (struct jet_context*)
    (sp - sizeof(*ctx));
    //(sp - (sizeof(*stack_frame_null) + sizeof(*ctx)));

  // Fill start frame
  //~ memset(stack_frame_null, 0, sizeof(*stack_frame_null));

  // Fill frame for context switch
  memset (ctx, 0, sizeof (*ctx));
  ctx->a1      = (uint32_t)entry;
  ctx->a0      = (uint32_t)sp;
  ctx->r16     = (unsigned long)entry;
  //~ ctx->r15     = id;
  ctx->ra      = (uint32_t)entry;
  //~ ctx->sp      = (uint32_t)&stack_frame_null->stack_frame;
  ctx->sp      = (uint32_t)sp;
  // Linkage between frames
  //~ jet_stack_frame_link(&stack_frame_null->stack_frame,
    //~ &ctx->stack_frame, entry);

  return ctx;
}
