#include "memlayout.h"
#include "regs.h"

.section .multiboot
.align 4
multiboot_header:
    .long   0x1BADB002                      /* magic */
    .long   0x00000000                      /* flags */
    .long   - 0x00000000 - 0x1BADB002       /* check = -(flags + magic) */


.section .text
.globl _core_entry
_core_entry = PHYS_ADDR(__entry)

__entry:
    cli #disable iterrupts

    # load page table
    movl $(PHYS_ADDR(entry_pgdir)), %eax
    movl %eax, %cr3

    # Turn on Page Size extension.
    movl %cr4, %eax
    orl $(CR4_PSE), %eax
    movl %eax, %cr4

    # Turn on Paging.
    movl %cr0, %eax
    orl $(CR0_PE | CR0_PG | CR0_WP), %eax
    movl %eax, %cr0

    # Now paging is enabled, but we're still running at a low EIP
    # Jump up.
    mov $relocated, %eax
    jmp *%eax

relocated:
    movl $(jet_entry_stack - 4), %ebp
    movl %ebp, %esp

    /* Set EFLAGS to 0 */
    pushl $0
    popf

    call pok_arch_init
    call jet_boot

fail:
    hlt
    jmp fail

/* For GDB breakpoints */
.global pok_trap_addr
pok_trap_addr:
.global pok_trap
pok_trap:
    int3
    ret
