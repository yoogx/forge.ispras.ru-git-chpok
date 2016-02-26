#ifndef __POK_COMPILER_H__
#define __POK_COMPILER_H__
/*
 * Functions and macros, which are compiler-specific.
 * 
 * Normally, there are operations which disables some compiler
 * optimizations, which can invalidate semanic when interrupts occures.
 * 
 * Currently assume compiler to be gcc.
 */

#define barrier() __asm__ __volatile__("": : :"memory")

/*
 * Access (read or write) given variable only once. Do not cache its value.
 * 
 * Applicable to simple types, which can be accessed using single asm instruction.
 */
#define ACCESS_ONCE(x) (*(volatile typeof(x) *)&(x))


#endif /* !__POK_COMPILER_H__ */
