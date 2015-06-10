include $(POK_PATH)/misc/mk/config.mk
-include $(POK_PATH)/misc/mk/common-$(ARCH).mk


LO_TARGET=	libfdt.lo

LO_OBJS=	fdt.o \
		fdt_ro.o \
		fdt_wip.o \
		fdt_sw.o \
		fdt_rw.o \
		fdt_strerror.o \
		fdt_empty_tree.o \

LO_DEPS=

all: $(LO_TARGET)

.PHONY: clean distclean depend all

clean: common-clean

distclean: clean
	$(RM) .depend.mk

depend:
	$(if $(LO_OBJS), $(CC) $(CFLAGS) -MM $(wildcard *.c) $(wildcard *.S) > .depend.mk,)

include $(POK_PATH)/misc/mk/rules-common.mk
include $(POK_PATH)/misc/mk/rules-kernel.mk

# old makefile
#LIBFDT_soname = libfdt.$(SHAREDLIB_EXT).1
#LIBFDT_INCLUDES = fdt.h libfdt.h libfdt_env.h
#LIBFDT_VERSION = version.lds
#LIBFDT_SRCS = fdt.c fdt_ro.c fdt_wip.c fdt_sw.c fdt_rw.c fdt_strerror.c fdt_empty_tree.c
#LIBFDT_OBJS = $(LIBFDT_SRCS:%.c=%.o)
#
