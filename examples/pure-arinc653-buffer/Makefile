export ARCH=x86
export BSP=x86-qemu
export POK_CONFIG_OPTIMIZE_FOR_GENERATED_CODE=1

include $(POK_PATH)/misc/mk/config.mk
include $(POK_PATH)/misc/mk/common-$(ARCH).mk

TARGET=$(shell pwd)/pok.elf
ARCHIVE=$(shell pwd)/partitions.cpio
PARTITIONS+= pr1/pr1.elf
#PARTITIONS+= pr2/pr2.elf
KERNEL=kernel/kernel.lo

all: build-regular

build-regular: build-kernel partitions $(TARGET)

compiletest: all

build-kernel:
	$(CD) kernel && $(MAKE)

partitions:
	$(CD) pr1 && $(MAKE) 
	#$(CD) pr2 && $(MAKE)

clean: common-clean
	$(RM) node_impl
	$(CD) kernel && $(MAKE) clean
	$(CD) pr1 && $(MAKE) clean
	#$(CD) pr2 && $(MAKE) clean

distclean: clean
	$(CD) kernel && $(MAKE) distclean
	$(CD) pr1 && $(MAKE) distclean
	#$(CD) pr2 && $(MAKE) distclean

include $(POK_PATH)/misc/mk/rules-common.mk
include $(POK_PATH)/misc/mk/rules-main.mk
include $(POK_PATH)/misc/mk/install-rules.mk

# uncomment if you wish it to print to the console
QEMU_MISC += -serial file:/dev/stdout

# uncomment if you don't want to see QEMU window
# useful with the previous option
QEMU_MISC += -display none

# uncomment if you want to use KVM
#QEMU_MISC += -enable-kvm

# uncomment this if you want to use gdb
#QEMU_MISC += -S -s

# override it so QEMU doesn't run in background
launch-run:
	$(ECHO) $(ECHO_FLAGS) "[QEMU] Start"
	$(QEMU) $(CONFIG_QEMU) $(QEMU_MISC) $(NETWORK_ARGS) -hda fat:. -boot a $(QEMU_ENDCOMMAND) 
