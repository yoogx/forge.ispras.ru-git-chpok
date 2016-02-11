#include <libc/gcov.h>
#include <core/syscall.h>

#include <stdio.h>

#ifdef POK_NEEDS_GCOV

void pok_gcov_init(void) {
    extern uint32_t __PARTITION_CTOR_START__, __PARTITION_CTOR_END__; // linker defined symbols
    uint32_t start = (uint32_t)(&__PARTITION_CTOR_START__ + 1);
    uint32_t end = (uint32_t)(&__PARTITION_CTOR_END__ - 1);
    printf("start 0x%lx, end 0x%lx\n", start, end);
    void (**p)(void);
    
    while(start < end) {
        p = (void(**)(void))start; // get function pointer
        (*p)(); // call constructor
        start += sizeof(p);
    }
}

void __gcov_init(struct gcov_info *info) {
    pok_syscall2(POK_SYSCALL_GCOV_INIT, (uint32_t) info, 0);
}

#endif /* POK_NEEDS_GCOV */
