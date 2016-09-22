all: tap
tap:
	sudo ip tuntap add tap0 mode tap user ekkk
	sudo ifconfig tap0 192.168.56.1 mtu 70
delete:
	sudo ip link delete tap0
mtu:
	sudo ifconfig tap0 mtu 1500
