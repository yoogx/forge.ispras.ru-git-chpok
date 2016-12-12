#include <libc.h>
#include <types.h>
#include "mmu.h"
#include "reg.h"

#include <assert.h>

#define START_ADDR 0x10000000
#define COUNT 50

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

#define write_tlb_registers_macros(entry, shift) {\
    mtspr(SPRN_MAS0, MAS0_TLBSEL(1) | MAS0_ESEL(entry));\
    mtspr(SPRN_MAS1, MAS1_VALID| MAS1_TID(0) | MAS1_TSIZE(E500MC_PGSIZE_4K));\
    mtspr(SPRN_MAS2, ((START_ADDR + entry*0x1000 + shift*0x100000) & MAS2_EPN) | 0);\
    mtspr(SPRN_MAS3, ((START_ADDR + entry*0x1000 + shift*0x100000) & MAS3_RPN) | MAS3_SW | MAS3_SR | MAS3_UW | MAS3_UR | MAS3_UX);\
    mtspr(SPRN_MAS7, 0);}

#define tlb_with_sync(entry, shift) { write_tlb_registers_macros(entry, shift); asm volatile("isync; tlbwe; isync":::"memory"); }
#define tlb_without_sync(entry, shift) { write_tlb_registers_macros(entry, shift); asm volatile("tlbwe":::"memory"); }

#define call_60_times_in_range_2_61(f, a) {\
    f(2 ,a); f(3 ,a); f(4 ,a); f(5 ,a); f(6 ,a); f(7 ,a); f(8 ,a); f(9 ,a);\
    f(10,a); f(11,a); f(12,a); f(13,a); f(14,a); f(15,a); f(16,a); f(17,a);\
    f(18,a); f(19,a); f(20,a); f(21,a); f(22,a); f(23,a); f(24,a); f(25,a);\
    f(26,a); f(27,a); f(28,a); f(29,a); f(30,a); f(31,a); f(32,a); f(33,a);\
    f(34,a); f(35,a); f(36,a); f(37,a); f(38,a); f(39,a); f(40,a); f(41,a);\
    f(42,a); f(43,a); f(44,a); f(45,a); f(46,a); f(47,a); f(48,a); f(49,a);\
    f(50,a); f(51,a); f(52,a); f(53,a); f(54,a); f(55,a); f(56,a); f(57,a);\
    f(58,a); f(59,a); f(60,a); f(61,a); }

void run_test_on_macros()
{
    printf("run_test_on_macros  sync on every tlbwe\n");
    for (int j = 0; j < COUNT; j++) {
        uint64_t start = __ppc_get_timebase();

        call_60_times_in_range_2_61(tlb_with_sync, j%2);

        uint64_t stop = __ppc_get_timebase();
        printf("%lld\n", stop - start);
    }

    printf("run_test_on_macros  sync once\n");
    for (int j = 0; j < COUNT; j++) {
        uint64_t start = __ppc_get_timebase();

        call_60_times_in_range_2_61(tlb_without_sync, j%2);
        asm volatile("isync":::"memory");

        uint64_t stop = __ppc_get_timebase();
        printf("%lld\n", stop - start);
    }
}



void tlb_measure()
{
    uint64_t a = __ppc_get_timebase();
    uint64_t b = __ppc_get_timebase();
    printf("%lld %lld\n", a, b);

    run_test_on_macros();

    while (1);
}
