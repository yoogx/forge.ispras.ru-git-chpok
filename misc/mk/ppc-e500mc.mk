run: pok.uboot
	$(QEMU) $(CONFIG_QEMU) $(QEMU_MISC) $(NETWORK_ARGS) -M ppce500 -cpu e500mc -m 256 -kernel pok.uboot $(QEMU_ENDCOMMAND) 

pok.bin: $(TARGET)
	$(OBJCOPY) -O binary $< $@

pok.uboot: pok.bin
	mkimage -A ppc -O linux -T kernel -C none -a 0x10000 -e 0x10100 -n "CHPOK" -d $< $@
