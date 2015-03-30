#!/bin/sh

# 1. uncomment line
#       QEMU_MISC += -S -s
#    in the makefile
# 2. run 'make run'
# 3. run './debug.sh'

exec powerpc-elf-gdb pok.elf -ex "target remote :1234" -ex "layout split"
