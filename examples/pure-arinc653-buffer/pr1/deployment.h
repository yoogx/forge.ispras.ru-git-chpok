/*
 *                               POK header
 * 
 * The following file is a part of the POK project. Any modification should
 * made according to the POK licence. You CANNOT use this file or a part of
 * this file is this part of a file for your own project
 *
 * For more information on the POK licence, please see our LICENCE FILE
 *
 * Please follow the coding guidelines described in doc/CODING_GUIDELINES
 *
 *                                      Copyright (c) 2007-2009 POK team 
 *
 * Created by julien on Thu Jan 15 23:34:13 2009 
 */


#ifndef __POK_USER_GENERATED_DEPLOYMENT_H_
#define __POK_USER_GENERATED_DEPLOYMENT_H_ 

#define POK_NEEDS_TIMER 1
#define POK_NEEDS_THREADS 1

#define POK_NEEDS_PARTITIONS 1

#define POK_NEEDS_DEBUG 1
#define POK_NEEDS_CONSOLE 1

#define POK_NEEDS_LIBC_STDLIB 1
#define POK_CONFIG_NEEDS_FUNC_MEMCPY         1
#define POK_CONFIG_NEEDS_FUNC_MEMSET         1
#define POK_CONFIG_NEEDS_FUNC_MEMCMP         1
#define POK_CONFIG_NEEDS_FUNC_STRCMP         1
#define POK_CONFIG_NEEDS_FUNC_STRNCMP        1
#define POK_CONFIG_NEEDS_FUNC_STRCPY         1
#define POK_CONFIG_NEEDS_FUNC_STRNCPY        1
#define POK_CONFIG_NEEDS_FUNC_STRLEN         1
#define POK_CONFIG_NEEDS_FUNC_STREQ          1
#define POK_CONFIG_NEEDS_FUNC_ITOA           1
#define POK_CONFIG_NEEDS_FUNC_UDIVDI3        1

#define POK_NEEDS_MIDDLEWARE 1

// XXX check all _NB_ defines, they might be off by one

#define POK_NEEDS_BUFFERS 1
#define POK_NEEDS_ARINC653_BUFFER 1
#define POK_CONFIG_NB_BUFFERS 4

#define POK_NEEDS_BLACKBOARDS 1
#define POK_NEEDS_ARINC653_BLACKBOARD 1
#define POK_CONFIG_NB_BLACKBOARDS 3

#define POK_NEEDS_ARINC653_SEMAPHORE 1
#define POK_CONFIG_ARINC653_NB_SEMAPHORES 4

#define POK_NEEDS_ARINC653_EVENT 1
#define POK_CONFIG_ARINC653_NB_EVENTS 3
#define POK_CONFIG_NB_EVENTS 3

#define POK_NEEDS_ARINC653_PARTITION 1
#define POK_NEEDS_ARINC653_PROCESS 1
#define POK_NEEDS_ARINC653_ERROR 1
#define POK_NEEDS_ARINC653_SAMPLING 1
#define POK_NEEDS_ARINC653_QUEUEING 1
#define POK_NEEDS_ARINC653_TIME 1

// also in kernel
#define POK_CONFIG_NB_THREADS       3 

#endif
