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

#include <interrupt.h>
#include <core/error.h>
#include <event.h>

{%if global_c%}

{{global_c}}
{%endif%}
// Declare exception functions. They are defined in exception_entries.S
{%for exception in exceptions%}
void exception_{{exception.id}}(void);
{%endfor%}


const struct exception_descriptor exception_list[] =
{
{%for exception in exceptions%}
    {EXCEPTION_{{exception.id}}, exception_{{exception.id}}},
{%endfor%}
    {0, NULL}
};

void ja_exception_init(void)
{
  int i;

  for (i = 0; exception_list[i].handler != NULL; ++i)
  {
    pok_idt_set_gate (exception_list[i].vector,
                      exception_list[i].handler,
                      IDTE_INTERRUPT);
  }
}


__attribute__((unused))
static void dump_registers (interrupt_frame *frame)
{
  printf ("ES: %lx, DS: %lx\n",  frame->es, frame->ds);
  printf ("CS: %lx, SS: %lx\n",  frame->cs, frame->ss);
  printf ("EDI: %lx, ESI: %lx\n", frame->edi, frame->esi);
  printf ("EBP: %lx, ESP: %lx\n", frame->ebp, frame->esp);
  printf ("EAX: %lx, ECX: %lx\n", frame->eax, frame->ecx);
  printf ("EDX: %lx, EBX: %lx\n", frame->edx, frame->ebx);
  printf ("EIP: %lx, ErrorCode: %lx\n", frame->eip, frame->error);
  printf ("EFLAGS: %lx\n\n", frame->eflags);
}

/* 
 * Raise error from interrupt handler.
 * 
 * Call pok_raise_error() with parameters error_id, user_space,
 * failed_addr passed to it and output formatted message, given as rest
 * parameters, with printf_debug().
 * 
 * Interrupt frame is ignored in any case.
 */
#define raise_error_from_interrupt(error_id, user_space, failed_addr, ... /* message format and its args */) \
   do { (void)frame; printf_debug(__VA_ARGS__); \
   pok_raise_error(error_id, user_space, failed_addr); } while(0)

/* Helper. dump_registers() in debug mode, otherwise argument is ignored. */
#ifdef POK_NEEDS_DEBUG
#define dump_registers_debug(frame) dump_registers(frame)
#else
#define dump_registers_debug(frame) (void)frame
#endif

/* 
 * Raise error from interrupt handler and dump registers.
 * 
 * Call pok_raise_error() with parameters error_id, user_space,
 * failed_addr passed to it and output formatted message, given as rest
 * parameters, with printf_debug().
 * 
 * If debugging is enabled then dump_registers() is called.
 */
#define raise_error_from_interrupt_dump(error_id, user_space, failed_addr, ... /* message format and its args */) \
   do { dump_registers_debug(frame); printf_debug(__VA_ARGS__); \
   pok_raise_error(error_id, user_space, failed_addr); } while(0)

// Define exception handlers. They are called in exception_entries.S.
{%for exception in exceptions%}
void exception_{{exception.id}}_handler(interrupt_frame* frame)
{
{%if exception.raise_error %}
    raise_error_from_interrupt{%if exception.raise_error.dump_registers%}_dump{%endif%}(POK_ERROR_ID_{{exception.raise_error.error_id}},
      /*TODO: User or kernel*/TRUE,
      /*TODO: Failed address*/NULL,
      "[KERNEL] {{exception.raise_error.debug_message}}\n"{%if exception.raise_error.debug_message_args%}
{%for arg in exception.raise_error.debug_message_args%}, {{arg}}
{%endfor%}
{%endif%}
    );
{%else%}
    {{exception.code}}
{%endif%}
}
{%endfor%}
