TOPDIR=		../../../

include $(TOPDIR)/misc/mk/config.mk
-include $(TOPDIR)/misc/mk/common-$(ARCH).mk

LO_TARGET=	ppc.lo

LO_OBJS=	entry.o \
		timer.o \
		cswitch.o \
		thread.o \
		space.o \
		syscalls.o \
		arch.o \
		interrupt.o \
		loader.o \
		pci.o \
		devtree.o

LO_DEPS=	$(BSP)/$(BSP).lo \
		libfdt/libfdt.lo


all: $(LO_TARGET)

$(BSP)/$(BSP).lo::
	$(CD) $(BSP) && $(MAKE)

libfdt/libfdt.lo::
	$(CD) libfdt && $(MAKE)

.PHONY: clean distclean depend all

clean: common-clean
	$(MAKE) -C $(BSP) $@
ifdef BSP
	$(CD) $(BSP) && $(MAKE) clean
endif
	$(CD) libfdt && $(MAKE) clean

distclean: clean
	$(MAKE) -C $(BSP) $@
	$(RM) .depend.mk
ifdef BSP
	$(CD) $(BSP) && $(MAKE) distclean
endif
	$(CD) libfdt && $(MAKE) distclean 

depend:
	$(CC) $(CFLAGS) -MM $(wildcard *.c) $(wildcard *.S) > .depend.mk
ifdef BSP
	$(CD) $(BSP) && $(MAKE) depend
endif
	$(CD) libfdt && $(MAKE) depend


include $(TOPDIR)/misc/mk/rules-common.mk
include $(TOPDIR)/misc/mk/rules-kernel.mk

