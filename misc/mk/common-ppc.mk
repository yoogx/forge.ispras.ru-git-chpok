CFLAGS = $(CONFIG_CFLAGS) -std=gnu99 -iwithprefix include -DPOK_ARCH_PPC $(KIND_CFLAGS) $(GENERIC_FLAGS) -Wall -g -O -Wuninitialized -ffreestanding -nostdlib -nostdinc -mregnames -mcpu=e500mc
LDFLAGS	= 
