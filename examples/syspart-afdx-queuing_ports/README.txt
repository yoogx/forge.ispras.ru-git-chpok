0. wireshark should be installed in the system
1. run `./misc/setup-tap_nobr.sh 4` in POK_PATH dir
2. run wireshark on tap0 (tap1) (or `tshark -i tap0 (tap1)` for command-line interface)
3. run this example by `scons run-tap2`
4. In wireshark (or tshark) you will see outgoing udp packets
