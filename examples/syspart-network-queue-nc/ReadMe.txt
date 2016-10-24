0. wireshark should be installed in the system
1. run `./misc/setup-tap_nobr.sh 2` in POK_PATH dir
2. run wireshark on tap0 (or `tshark -i tap0` for command-line interface)
3. run this example by `scons run-tap`
4. In wireshark (or tshark) you will see 30 outgoing udp packets

5. You can also send something in JetOS.
Run

sudo ifconfig tap0 192.168.56.1
echo 'Hello' | nc -u 192.168.56.101 10001

and you you will see that 'Hello' is received in JetOS output.
