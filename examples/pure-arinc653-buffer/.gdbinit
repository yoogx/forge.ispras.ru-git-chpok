#add-symbol-file P1/build/e500mc/part.elf 0x80000000
set architecture mips:isa64
set remoteaddresssize 64
add-symbol-file P1/build/mips-qemu/part.elf 0x60000000
target remote:1234
