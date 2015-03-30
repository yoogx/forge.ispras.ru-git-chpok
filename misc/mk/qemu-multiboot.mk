launch-run:
	$(ECHO) $(ECHO_FLAGS) "[QEMU] Start"
	$(QEMU) $(CONFIG_QEMU) $(QEMU_MISC) $(NETWORK_ARGS) -kernel pok.elf $(QEMU_ENDCOMMAND)

run: launch-run
