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
 * Created by julien on Mon Jan 19 10:51:40 2009 
 */

#ifndef __POK_ERROR_H
#define __POK_ERROR_H

#include <config.h>

#include <core/dependencies.h>

#ifdef POK_NEEDS_ERROR_HANDLING

#include <types.h>
#include <errno.h>

#define POK_ERROR_MAX_MSG_SIZE                  250

#define POK_ERROR_KIND_INVALID                   0 // this is "NULL" error - that is, no error
#define POK_ERROR_KIND_DEADLINE_MISSED          10
#define POK_ERROR_KIND_APPLICATION_ERROR        11
#define POK_ERROR_KIND_NUMERIC_ERROR            12
#define POK_ERROR_KIND_ILLEGAL_REQUEST          13
#define POK_ERROR_KIND_STACK_OVERFLOW           14
#define POK_ERROR_KIND_MEMORY_VIOLATION         15
#define POK_ERROR_KIND_HARDWARE_FAULT           16
#define POK_ERROR_KIND_POWER_FAIL               17
#define POK_ERROR_KIND_PARTITION_CONFIGURATION  30 
#define POK_ERROR_KIND_PARTITION_INIT           31
#define POK_ERROR_KIND_PARTITION_SCHEDULING     32
#define POK_ERROR_KIND_PARTITION_HANDLER        33
#define POK_ERROR_KIND_PARTITION_PROCESS        34
#define POK_ERROR_KIND_KERNEL_INIT              50
#define POK_ERROR_KIND_KERNEL_SCHEDULING        51
#define POK_ERROR_KIND_KERNEL_CONFIG            52
typedef uint8_t pok_error_kind_t;

typedef struct
{
   pok_error_kind_t     error_kind;
   pok_thread_id_t      failed_thread;
   uintptr_t            failed_addr;
   size_t               msg_size;
   char                 msg[POK_ERROR_MAX_MSG_SIZE];
} pok_error_status_t;

#include <core/syscall.h>

pok_ret_t pok_error_handler_create ();
void pok_error_ignore  (const uint32_t error_id, const uint32_t thread_id);
void pok_error_confirm (const uint32_t error_id, const uint32_t thread_id);
pok_ret_t pok_error_handler_set_ready (const pok_error_status_t*);

//void pok_error_raise_application_error (const char* msg, size_t msg_size);

/**
 * pok_error_get returns POK_ERRNO_OK if the error pointer
 * was registered and an error was registered.
 * It also returns POK_ERRNO_UNAVAILABLE if the pointer
 * was not registered or if nothing was detected
 */
//pok_ret_t pok_error_get (pok_error_status_t* status);

//pok_ret_t pok_error_is_handler(void);

#endif

#endif /* __POK_ERROR_H */
