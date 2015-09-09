#!/bin/sh
#exec powerpc-elf-gdb pok.elf -ex "target remote :1234" -ex "layout split"
exec powerpc-elf-gdb pok.elf -ex "target remote :1234"
