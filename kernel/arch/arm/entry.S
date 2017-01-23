// To keep this in the first portion of the binary.
.section ".text.boot"


// Entry point for the kernel.
// r15 -> should begin execution at 0x8000.
// r0 -> 0x00000000
// r1 -> 0x00000C42
// r2 -> 0x00000100 - start of ATAGS
// preserve these registers as argument for kernel_main
.globl jet_start
jet_start:
        // Setup the stack.
        ldr sp, =jet_entry_stack

        // Call arch_init
        ldr r3, =kernel_main
        blx r3

        // Call jet_boot. Should not return
        ldr r3, =jet_boot
        blx r3

        // halt
halt:
        wfe
        b halt
