You can run 2 modes of receiving messages:
Queuing ports or Sampling ports

Queuing ports it is standart condition
to run this example by `scons run-tap-rec`

Sampling ports:

1. change ports in config.xml in this catalog
QP1 -> SP1
UOUT -> SUOUT
QP2 -> SP2
UIN -> SUIN

2. change state of arinc_port_writer_1 in P2/glue_config.yaml
port_name: '"UIN"' to '"SUIN"'
is_queuing_port: 1 to 0

3. in file P1/src/main.c
uncomment #define SAMPLING_MODE
and connent #define QUEUING_MODE

3. build this example by 'scons cdeveloper=1'
4. run this example by `scons run-tap-rec`
