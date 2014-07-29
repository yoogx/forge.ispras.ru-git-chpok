TOPDIR=		../../../

include $(TOPDIR)/misc/mk/config.mk
-include $(TOPDIR)/misc/mk/common-$(ARCH).mk

LO_TARGET=	virtio.lo

LO_OBJS= \
	 virtio_virtqueue.o \
	 virtio_network.o \

LO_DEPS= 

all: $(LO_TARGET)

.PHONY: clean distclean depend all

clean: common-clean

distclean:
	$(RM) $(LO_TARGET)
	$(RM) $(LO_OBJS)
	$(RM) .depend.mk

depend:
	$(if $(LO_OBJS), $(CC) $(CFLAGS) -MM $(wildcard *.c) $(wildcard *.S) > .depend.mk,)

include $(TOPDIR)/misc/mk/rules-common.mk
include $(TOPDIR)/misc/mk/rules-kernel.mk
