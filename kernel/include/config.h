/*
 * Institute for System Programming of the Russian Academy of Sciences
 * Copyright (C) 2016 ISPRAS
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, Version 3.
 *
 * This program is distributed in the hope # that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License version 3 for more details.
 */

#define POK_NEEDS_LOCKOBJECTS  1
#define POK_NEEDS_THREADS      1
#define POK_NEEDS_PARTITIONS   1
#define POK_NEEDS_SCHED        1
#define POK_NEEDS_TIME         1
#define POK_NEEDS_GETTICK      1
#define POK_NEEDS_DEBUG        1
#define POK_NEEDS_SERIAL       1
#define POK_NEEDS_CONSOLE      1
#define POK_NEEDS_ERROR_HANDLING 1
#define POK_NEEDS_THREAD_SUSPEND 1
#define POK_NEEDS_THREAD_SLEEP 1
#define POK_NEEDS_THREAD_ID 1
#define POK_NEEDS_MONITOR 1
#define POK_NEEDS_GDB 1

// Quick and dirty hack: currently debugger support is broken on x86
// This options provides the way to build JET OS on x86
// It is set in /misc/SConsript in CFLAGS
#ifndef POK_DISABLE_GDB
#define   POK_NEEDS_GDB 1
#endif

#define POK_NEEDS_PORTS_SAMPLING 1
#define POK_NEEDS_PORTS_QUEUEING 1
#define POK_NEEDS_BUFFERS 1
#define POK_NEEDS_BLACKBOARDS 1
#define POK_NEEDS_SEMAPHORES 1
#define POK_NEEDS_EVENTS 1

//#define POK_NEEDS_NETWORKING 1
//#define POK_NEEDS_NETWORKING_VIRTIO 1
//#define POK_NEEDS_PCI 1

//#define POK_NEEDS_INSTRUMENTATION 1
//#define POK_NEEDS_IO 1
//#define POK_NEEDS_COVERAGE_INFOS 1
//#define POK_NEEDS_DMA 1
//#define POK_NEEDS_DEPRECIATED 1
#define POK_TEST_SUPPORT_PRINT_WHEN_ALL_THREADS_STOPPED 1

//#define POK_NEEDS_SIMULATION 1

// #define POK_CONFIG_NB_THREADS pok_config_nb_threads
// #define POK_CONFIG_PARTITIONS_NTHREADS pok_config_partitions_nthreads
// #define POK_CONFIG_NB_PARTITIONS pok_config_nb_partitions
// #define POK_CONFIG_NB_LOCKOBJECTS pok_config_nb_lockobjects
// #define POK_CONFIG_PARTITIONS_NLOCKOBJECTS pok_config_partitions_nlockobjects
// #define POK_CONFIG_PARTITIONS_SIZE pok_config_partitions_size
// #define POK_CONFIG_SCHEDULING_NBSLOTS pok_config_scheduling_nbslots
// #define POK_CONFIG_SCHEDULING_MAJOR_FRAME pok_config_scheduling_major_frame
// #define POK_CONFIG_NB_SAMPLING_PORTS pok_config_nb_sampling_ports
// #define POK_CONFIG_NB_QUEUEING_PORTS pok_config_nb_queueing_ports
#define PARTITION_DEBUG_MODE 1

//extern uint8_t pok_config_nb_threads;
//extern uint32_t pok_config_partitions_nthreads[];
//extern unsigned pok_config_nb_partitions;
//extern unsigned pok_config_nb_lockobjects;
//extern uint8_t pok_config_partitions_nlockobjects[];
//extern uint32_t pok_config_partitions_size[];
//extern unsigned pok_config_scheduling_nbslots;
//extern unsigned pok_config_scheduling_major_frame;
//extern unsigned pok_config_nb_sampling_ports;
//extern unsigned pok_config_nb_queueing_ports;
