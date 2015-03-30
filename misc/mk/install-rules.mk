#rules to install and run program
ifeq ($(ARCH), x86)
  ifeq ($(BSP), x86-qemu) 
   include $(POK_PATH)/misc/mk/qemu-multiboot.mk
  else
   $(error Unknown BSP $(BSP))
  endif
endif
ifeq ($(ARCH), ppc)
  ifeq ($(BSP), e500mc)
    include $(POK_PATH)/misc/mk/ppc-e500mc.mk
  else
    $(error Unknown BSP $(BSP)) 
  endif
endif
ifeq ($(ARCH), sparc)
  include $(POK_PATH)/misc/mk/sparc-install.mk
endif
