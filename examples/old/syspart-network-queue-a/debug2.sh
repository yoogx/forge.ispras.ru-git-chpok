#!/bin/sh
#exec gdb pok.elf -ex "target remote :1234" -ex "layout split"
exec powerpc-elf-gdb pok.elf -ex "add-symbol-file pr2/pr2.elf 0x80000000" -ex "target remote :1234"
