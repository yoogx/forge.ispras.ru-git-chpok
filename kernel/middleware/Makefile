TOPDIR=		../../

include $(TOPDIR)/misc/mk/config.mk
-include $(TOPDIR)/misc/mk/common-$(ARCH).mk

LO_TARGET=middleware.lo

LO_OBJS=portsampling_ops.o \
	portqueueing_utils.o \
	portqueueing_ops.o \
	portflushall.o \
	portinit.o \
	portreset.o

LO_DEPS=

all: $(LO_TARGET)

.PHONY: clean distclean depend all

distclean:

clean: common-clean

depend:
	$(if $(LO_OBJS), $(CC) $(CFLAGS) -MM $(wildcard *.c) $(wildcard *.S) > .depend.mk,)

include $(TOPDIR)/misc/mk/rules-common.mk
include $(TOPDIR)/misc/mk/rules-kernel.mk

