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


#ifndef __POK_PPC_THREAD_H__
#define __POK_PPC_THREAD_H__

#include <types.h>

typedef struct
{
  uint32_t sp;
  uint32_t unused_lr;

  uint32_t cr; /* 8 */
  uint32_t r2;

  uint32_t r13; /* 16 */
  uint32_t r14;
  uint32_t r15;
  uint32_t r16;

  uint32_t r17; /* 32 */
  uint32_t r18;
  uint32_t r19;
  uint32_t r20;

  uint32_t r21; /* 48 */
  uint32_t r22;
  uint32_t r23;
  uint32_t r24;

  uint32_t r25; /* 64 */
  uint32_t r26;
  uint32_t r27;
  uint32_t r28;

  uint32_t r29; /* 80 */
  uint32_t r30;
  uint32_t r31;
  /* Just for cleanliness. If we don't explicitly 8-byte align f0, then it will be done
   * implicilty by gcc */
  uint32_t pad_for_float;

  uint64_t f0; /* 96 */
  uint64_t f1;
  uint64_t f2;
  uint64_t f3;
  uint64_t f4;
  uint64_t f5;
  uint64_t f6;
  uint64_t f7;
  uint64_t f8;
  uint64_t f9;
  uint64_t f10;
  uint64_t f11;
  uint64_t f12;
  uint64_t f13;
  uint64_t f14;
  uint64_t f15;
  uint64_t f16;
  uint64_t f17;
  uint64_t f18;
  uint64_t f19;
  uint64_t f20;
  uint64_t f21;
  uint64_t f22;
  uint64_t f23;
  uint64_t f24;
  uint64_t f25;
  uint64_t f26;
  uint64_t f27;
  uint64_t f28;
  uint64_t f29;
  uint64_t f30;
  uint64_t f31; /* 344 */

  /* Previous frame.  */
  uint32_t back_chain; /* 352 */
  uint32_t lr;
} context_t;

_Static_assert(offsetof(context_t, back_chain) % 16 == 0, "stack size should be quad-word aligned");

/*
 * NOTE: back_chain offset in this struct must be equal to entry.S FRAME_SIZE
 * and must be
 */
typedef struct
{
  uint32_t sp;
  uint32_t unused_lr;

  uint32_t cr; /* 8 */
  uint32_t r0;

  uint32_t r2; /* 16 */
  uint32_t r3;
  uint32_t r4;
  uint32_t r5;

  uint32_t r6; /* 32 */
  uint32_t r7;
  uint32_t r8;
  uint32_t r9;

  uint32_t r10; /* 48 */
  uint32_t r11;
  uint32_t r12;
  uint32_t r13;

  uint32_t ctr; /* 64 */
  uint32_t xer;
  uint32_t srr0;
  uint32_t srr1;

  uint64_t fp0; /* 80 */
  /* for back_chain alignment */
  uint32_t b_pad0; /* 88 */
  uint32_t b_pad1;

  /* Previous frame.  */
  uint32_t back_chain; /* 96 */
  uint32_t lr;

  /* For initial frame alignment.  */
  uint32_t pad0;
  uint32_t pad1;
} volatile_context_t;

_Static_assert(offsetof(volatile_context_t, back_chain) % 16 == 0, "stack size should be quad-word aligned");

uint32_t		pok_context_create(uint32_t id,
					   uint32_t stack_size,
					   uint32_t entry);

void			pok_context_switch(uint32_t* old_sp,
					   uint32_t new_sp);


#endif /* !__POK_PPC_THREAD_H__ */

