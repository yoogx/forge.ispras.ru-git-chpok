
CFLAGS	=	$(CONFIG_CFLAGS) -gnu99 -nostdinc -iwithprefix include -fno-builtin -DPOK_ARCH_PPC $(KIND_CFLAGS) $(GENERIC_FLAGS) -Wall -g -O -Wuninitialized

#LDFLAGS	=	-m $(ELF_MODE)
