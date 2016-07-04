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


#ifndef __POK_SYSPART_X86_IOPORTS_H__
#define __POK_SYSPART_X86_IOPORTS_H__

#define outb(port, data)                                        \
  asm volatile ("outb %b0,%w1"                                  \
                :						\
		:"a" (data),"d" (port))

#define inb(port)                                               \
({                                                              \
  unsigned char res;                                            \
  asm volatile ("inb %w1,%0"                                    \
                :"=a" (res)                                     \
                :"d" (port));                                   \
  res;                                                          \
})

#define outw(port, data)                                        \
  asm volatile ("outw %0,%w1"					\
                :						\
		:"a" (data),"d" (port))

#define inw(port)                                               \
({                                                              \
  unsigned short res;						\
  asm volatile ("inw %w1,%0"					\
                :"=a" (res)                                     \
                :"d" (port));					\
  res;                                                          \
})

#define outl(port, data)                                        \
  asm volatile ("outl %0,%w1"					\
                :						\
		:"a" (data),"d" (port))

#define inl(port)                                               \
({                                                              \
  unsigned int res;						\
  asm volatile ("inl %w1,%0"					\
                :"=a" (res)                                     \
                :"d" (port));					\
  res;                                                          \
})

#endif /* __POK_X86_IOPORTS_H__ */
