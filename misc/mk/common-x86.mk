CFLAGS = $(CONFIG_CFLAGS) -std=gnu99 -iwithprefix include -DPOK_ARCH_X86 $(KIND_CFLAGS) $(GENERIC_FLAGS) -Wall -g -Og -Wuninitialized -ffreestanding -nostdlib
LDFLAGS	= 
# FIXME: architecture should not be hardcoded...
ADAFLAGS = -gnaty -gnata -I $(POK_PATH)/libpok/ada/arinc653
