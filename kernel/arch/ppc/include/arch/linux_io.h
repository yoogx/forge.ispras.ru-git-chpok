/*
 * From linux sources:
 * arch/powerpc/include/asm/io.h
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 *
 */

#ifndef __POK_LINUX_IOPORTS_H__
#define __POK_LINUX_IOPORTS_H__

#include <stdint.h>


typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

/*
 *
 * Low level MMIO accessors
 *
 * This provides the non-bus specific accessors to MMIO. Those are PowerPC
 * specific and thus shouldn't be used in generic code. The accessors
 * provided here are:
 *
 *	in_8, in_le16, in_be16, in_le32, in_be32, in_le64, in_be64
 *	out_8, out_le16, out_be16, out_le32, out_be32, out_le64, out_be64
 *	_insb, _insw_ns, _insl_ns, _outsb, _outsw_ns, _outsl_ns
 *
 * Those operate directly on a kernel virtual address. Note that the prototype
 * for the out_* accessors has the arguments in opposite order from the usual
 * linux PCI accessors. Unlike those, they take the address first and the value
 * next.
 *
 * Note: I might drop the _ns suffix on the stream operations soon as it is
 * simply normal for stream operations to not swap in the first place.
 *
 */

#define DEF_MMIO_IN_X(name, size, insn)				\
static inline u##size name(const volatile u##size  *addr)	\
{									\
	u##size ret;							\
	__asm__ __volatile__("sync;"#insn" %0,%y1;twi 0,%0,0;isync"	\
		: "=r" (ret) : "Z" (*addr) : "memory");			\
	return ret;							\
}

#define DEF_MMIO_OUT_X(name, size, insn)				\
static inline void name(volatile u##size  *addr, u##size val)	\
{									\
	__asm__ __volatile__("sync;"#insn" %1,%y0"			\
		: "=Z" (*addr) : "r" (val) : "memory");			\
}

#define DEF_MMIO_IN_D(name, size, insn)				\
static inline u##size name(const volatile u##size  *addr)	\
{									\
	u##size ret;							\
	__asm__ __volatile__("sync;"#insn"%U1%X1 %0,%1;twi 0,%0,0;isync"\
		: "=r" (ret) : "m" (*addr) : "memory");			\
	return ret;							\
}

#define DEF_MMIO_OUT_D(name, size, insn)				\
static inline void name(volatile u##size  *addr, u##size val)	\
{									\
	__asm__ __volatile__("sync;"#insn"%U0%X0 %1,%0"			\
		: "=m" (*addr) : "r" (val) : "memory");			\
}

DEF_MMIO_IN_D(in_8,     8, lbz);
DEF_MMIO_OUT_D(out_8,   8, stb);

#ifdef __BIG_ENDIAN__
DEF_MMIO_IN_D(in_be16, 16, lhz);
DEF_MMIO_IN_D(in_be32, 32, lwz);
DEF_MMIO_IN_X(in_le16, 16, lhbrx);
DEF_MMIO_IN_X(in_le32, 32, lwbrx);

DEF_MMIO_OUT_D(out_be16, 16, sth);
DEF_MMIO_OUT_D(out_be32, 32, stw);
DEF_MMIO_OUT_X(out_le16, 16, sthbrx);
DEF_MMIO_OUT_X(out_le32, 32, stwbrx);
#else
DEF_MMIO_IN_X(in_be16, 16, lhbrx);
DEF_MMIO_IN_X(in_be32, 32, lwbrx);
DEF_MMIO_IN_D(in_le16, 16, lhz);
DEF_MMIO_IN_D(in_le32, 32, lwz);

DEF_MMIO_OUT_X(out_be16, 16, sthbrx);
DEF_MMIO_OUT_X(out_be32, 32, stwbrx);
DEF_MMIO_OUT_D(out_le16, 16, sth);
DEF_MMIO_OUT_D(out_le32, 32, stw);

#endif /* __BIG_ENDIAN */

#endif
