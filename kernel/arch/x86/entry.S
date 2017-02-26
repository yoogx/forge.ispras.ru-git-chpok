#include "memlayout.h"
#include "regs.h"

#define STACK_SIZE 0x4000


.section .multiboot
.align 4
multiboot_header:
    .long   0x1BADB002                      /* magic */
    .long   0x00000000                      /* flags */
    .long   - 0x00000000 - 0x1BADB002       /* check */


.section .text
.globl _core_entry
_core_entry = PHYS_ADDR(entry)

.globl entry
entry:
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

    movl $(pok_stack + STACK_SIZE - 4), %ebp
    movl %ebp, %esp

    /* Set EFLAGS to 0 */
    pushl $0
    popf


    call pok_arch_init
    call jet_boot
loop:
    hlt
    jmp loop

.comm   pok_stack, STACK_SIZE #TODO move to ldscript

/* For GDB breakpoints */
.global pok_trap_addr
pok_trap_addr:
.global pok_trap
pok_trap:
    int3
    ret
