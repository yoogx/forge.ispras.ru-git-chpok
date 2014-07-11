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


#ifdef POK_NEEDS_ARINC653_ERROR
#include <types.h>
#include <errno.h>
#include <arinc653/types.h>
#include <arinc653/error.h>
#include <core/error.h>
#include <core/syscall.h>
#include <libc/string.h>

#define MAP_ERROR(from, to) case (from): *RETURN_CODE = (to); break
#define MAP_ERROR_DEFAULT(to) default: *RETURN_CODE = (to); break

static int error_arinc_to_pok(ERROR_CODE_TYPE error)
{
    #define MAP(to, from) case (from): return (to);
    switch (error) {
        MAP(POK_ERROR_KIND_DEADLINE_MISSED, DEADLINE_MISSED);
        MAP(POK_ERROR_KIND_APPLICATION_ERROR, APPLICATION_ERROR);
        MAP(POK_ERROR_KIND_NUMERIC_ERROR, NUMERIC_ERROR);
        MAP(POK_ERROR_KIND_ILLEGAL_REQUEST, ILLEGAL_REQUEST);
        MAP(POK_ERROR_KIND_STACK_OVERFLOW, STACK_OVERFLOW);
        MAP(POK_ERROR_KIND_MEMORY_VIOLATION, MEMORY_VIOLATION);
        MAP(POK_ERROR_KIND_HARDWARE_FAULT, HARDWARE_FAULT);
        MAP(POK_ERROR_KIND_POWER_FAIL, POWER_FAIL);
        default: return 0;
    }
    #undef MAP
}

static ERROR_CODE_TYPE error_pok_to_arinc(int pok_error)
{
    #define MAP(from, to) case (from): return (to);
    switch (pok_error) {
        MAP(POK_ERROR_KIND_DEADLINE_MISSED, DEADLINE_MISSED);
        MAP(POK_ERROR_KIND_APPLICATION_ERROR, APPLICATION_ERROR);
        MAP(POK_ERROR_KIND_NUMERIC_ERROR, NUMERIC_ERROR);
        MAP(POK_ERROR_KIND_ILLEGAL_REQUEST, ILLEGAL_REQUEST);
        MAP(POK_ERROR_KIND_STACK_OVERFLOW, STACK_OVERFLOW);
        MAP(POK_ERROR_KIND_MEMORY_VIOLATION, MEMORY_VIOLATION);
        MAP(POK_ERROR_KIND_HARDWARE_FAULT, HARDWARE_FAULT);
        MAP(POK_ERROR_KIND_POWER_FAIL, POWER_FAIL);
        default: return 0;
    }
    #undef MAP
}

/**
 * At this time, it is implemented to have the same behavior as 
 * RAISE_APPLICATION_ERROR. Should change that in the future
 *
 * XXX If I understand this correctly, it's essentially 
 * a logging function, and shouldn't raise any errors.
 */
void REPORT_APPLICATION_MESSAGE (MESSAGE_ADDR_TYPE    MESSAGE,
                                 MESSAGE_SIZE_TYPE    LENGTH,
                                 RETURN_CODE_TYPE     *RETURN_CODE )
{
   (void) LENGTH;

   // TODO LENGTH is ignored (shouldn't be)
   printf("%s\n", MESSAGE);

   *RETURN_CODE = NO_ERROR;
}

void CREATE_ERROR_HANDLER (SYSTEM_ADDRESS_TYPE  ENTRY_POINT,
                           STACK_SIZE_TYPE      STACK_SIZE,
                           RETURN_CODE_TYPE     *RETURN_CODE)
{
    pok_ret_t core_ret;
    core_ret = pok_syscall2 (POK_SYSCALL_ERROR_HANDLER_CREATE, (uint32_t)STACK_SIZE, (uint32_t)ENTRY_POINT);

    switch (core_ret) {
        MAP_ERROR(POK_ERRNO_OK, NO_ERROR);
        MAP_ERROR(POK_ERRNO_EXISTS, NO_ACTION);
        MAP_ERROR(POK_ERRNO_MODE, INVALID_MODE);
        MAP_ERROR_DEFAULT(NOT_AVAILABLE);

    }
}

void GET_ERROR_STATUS (ERROR_STATUS_TYPE  *ERROR_STATUS,
                       RETURN_CODE_TYPE   *RETURN_CODE )
{
    pok_error_status_t   core_status;
    pok_ret_t            core_ret;

    core_ret = pok_error_get (&core_status);

    if (core_ret == POK_ERRNO_OK) {
        ERROR_STATUS->ERROR_CODE = error_pok_to_arinc(core_status.error_kind);
        memcpy (ERROR_STATUS->MESSAGE, core_status.msg, MAX_ERROR_MESSAGE_SIZE);
        ERROR_STATUS->LENGTH = core_status.msg_size;
        ERROR_STATUS->FAILED_PROCESS_ID = core_status.failed_thread + 1; // ARINC process IDs are one higher
        ERROR_STATUS->FAILED_ADDRESS = (SYSTEM_ADDRESS_TYPE)core_status.failed_addr;
    }

    switch (core_ret) {
        MAP_ERROR(POK_ERRNO_OK, NO_ERROR);
        MAP_ERROR(POK_ERRNO_THREAD, INVALID_CONFIG);
        MAP_ERROR(POK_ERRNO_UNAVAILABLE, NO_ACTION);
        MAP_ERROR_DEFAULT(NOT_AVAILABLE);
    }

}

void RAISE_APPLICATION_ERROR (ERROR_CODE_TYPE            ERROR_CODE,
                              MESSAGE_ADDR_TYPE          MESSAGE,
                              ERROR_MESSAGE_SIZE_TYPE    LENGTH,
                              RETURN_CODE_TYPE           *RETURN_CODE)
{
  if (LENGTH > 64)
   {
      *RETURN_CODE = INVALID_PARAM;
      return;
   }

   if ( (ERROR_CODE != APPLICATION_ERROR))
   {
      *RETURN_CODE = INVALID_PARAM;
      return;
   }

   pok_error_raise_application_error ((char*) MESSAGE, LENGTH);

   *RETURN_CODE = NO_ERROR;
}

#endif
