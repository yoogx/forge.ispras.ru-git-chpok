CFLAGS = $(CONFIG_CFLAGS) -std=gnu99 -iwithprefix include -DPOK_ARCH_SPARC $(KIND_CFLAGS) $(GENERIC_FLAGS) -Wall -g -O -Wuninitialized -ffreestanding -nostdlib -mno-app-regs -mcpu=v8 
LDFLAGS	= 

