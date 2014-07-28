TOPDIR=		../../

include $(TOPDIR)/misc/mk/config.mk
-include $(TOPDIR)/misc/mk/common-$(ARCH).mk

LO_TARGET=	drivers.lo

LO_OBJS= 

LO_DEPS= virtio/virtio.lo

all: $(LO_TARGET)

.PHONY: clean distclean depend all

clean: common-clean
	$(CD) virtio && $(MAKE) clean

distclean:
	$(CD) virtio && $(MAKE) distclean
	$(RM) $(LO_TARGET)
	$(RM) $(LO_OBJS)
	$(RM) .depend

virtio/virtio.lo:
	$(CD) virtio && $(MAKE)	

depend:
	$(CD) virtio && $(MAKE) depend
	$(TOUCH) .depend

include $(TOPDIR)/misc/mk/rules-common.mk
include $(TOPDIR)/misc/mk/rules-kernel.mk
