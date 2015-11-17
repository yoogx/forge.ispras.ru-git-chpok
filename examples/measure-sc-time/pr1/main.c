#include <stdio.h>
#include <string.h>
#include <arinc653/buffer.h>
#include <arinc653/partition.h>
#include <arinc653/time.h>
#include <core/syscall.h>


static inline uint64_t
__ppc_get_timebase (void)
{
  uint32_t __tbu, __tbl, __tmp; 
  asm volatile ("0:\n\t"
            "mftbu %0\n\t"
            "mftbl %1\n\t"
            "mftbu %2\n\t"
            "cmpw %0, %2\n\t"
            "bne- 0b"
            : "=r" (__tbu), "=r" (__tbl), "=r" (__tmp));
  return (((uint64_t) __tbu << 32) | __tbl);
}

enum {
	MEASURE_TB,
	MEASURE_FUNCALL,
	MEASURE_SC,
	MEASURE_SYSCALL,
};

static unsigned int deadbeef(unsigned int x, unsigned int y) {
	return x+y;
}

static void measure_sc(int mode)
{
	unsigned int i, cnt = 0;
	unsigned int lim = 10000;
	unsigned int ticks = 0;
	unsigned int ticks_sq = 0;
	
	
	for (i = 0; i < lim; i++) {
		uint64_t base, base2;
		if (mode == MEASURE_SC) {
			base = __ppc_get_timebase();
			asm volatile ("lis %r3, 2048; lis %r4, 0; sc");
			base2 = __ppc_get_timebase();
		} 
		if (mode == MEASURE_FUNCALL) {
			base = __ppc_get_timebase();
			deadbeef(i, lim);
			base2 = __ppc_get_timebase();
		} 
		if (mode == MEASURE_SYSCALL) {
			base = __ppc_get_timebase();
			pok_syscall2(2049, i, lim);
			base2 = __ppc_get_timebase();
		} 
		if (mode == MEASURE_TB) {
			base = __ppc_get_timebase();
			base2 = __ppc_get_timebase();
		}

		uint64_t delta = base2 - base;
		unsigned int tick = (unsigned int)delta;
		if (tick > 3000) continue;
		
		ticks += tick;
		ticks_sq += tick*tick;
		cnt ++;
	}
	
	unsigned int avg = ticks / cnt;
	unsigned int disp = avg*avg - ticks_sq/cnt;
	if (disp > avg*avg) disp = 0;
	
    printf("Average number of ticks per call: %u,\n dispersion square: %u\n", 
		avg, disp);
}

void main(void) {
	printf("Measure time base access\n");
	measure_sc(MEASURE_TB);
	printf("Measure function call\n");
	measure_sc(MEASURE_FUNCALL);
	printf("Measure sc/rfi instruction\n");
	measure_sc(MEASURE_SC);
	printf("Measure system call\n");
	measure_sc(MEASURE_SYSCALL);
	STOP_SELF();
}  
