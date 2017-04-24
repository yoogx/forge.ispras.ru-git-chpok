/*
 * Institute for System Programming of the Russian Academy of Sciences
 * Copyright (C) 2017 ISPRAS
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

#include <config.h>

#include <types.h>
#include <errno.h>
#include <arinc653/types.h>
#include <arinc653/error.h>
#include <core/error.h>
#include <core/syscall.h>
#include <string.h>
#include <stdio.h>

#include "map_error.h"

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
 * If I understand this correctly, it's essentially
 * a logging function, and shouldn't raise any errors.
 */
void REPORT_APPLICATION_MESSAGE (MESSAGE_ADDR_TYPE    MESSAGE,
                                 MESSAGE_SIZE_TYPE    LENGTH,
                                 RETURN_CODE_TYPE     *RETURN_CODE )
{
   if (LENGTH < 0 || LENGTH > MAX_ERROR_MESSAGE_SIZE) {
       *RETURN_CODE = INVALID_PARAM;
       return;
   }

   // TODO LENGTH is ignored (shouldn't be)
   printf("%s\n", MESSAGE);

   *RETURN_CODE = NO_ERROR;
}

void CREATE_ERROR_HANDLER (SYSTEM_ADDRESS_TYPE  ENTRY_POINT,
                           STACK_SIZE_TYPE      STACK_SIZE,
                           RETURN_CODE_TYPE     *RETURN_CODE)
{
    jet_ret_t core_ret = pok_error_thread_create(STACK_SIZE, (void*)ENTRY_POINT);

    MAP_ERROR_BEGIN(core_ret)
        MAP_ERROR(EOK, NO_ERROR);
        MAP_ERROR(EEXIST, NO_ACTION);
        MAP_ERROR(JET_INVALID_MODE, INVALID_MODE);
        MAP_ERROR(JET_INVALID_CONFIG, INVALID_CONFIG);
        MAP_ERROR_DEFAULT();
    MAP_ERROR_END()
}

void GET_ERROR_STATUS (ERROR_STATUS_TYPE  *ERROR_STATUS,
                       RETURN_CODE_TYPE   *RETURN_CODE )
{
    pok_error_status_t   core_status;
    jet_ret_t            core_ret;

    core_ret = pok_error_get (&core_status, ERROR_STATUS->MESSAGE);

    MAP_ERROR_BEGIN(core_ret)
        MAP_ERROR(EOK, NO_ERROR);
        MAP_ERROR(JET_INVALID_MODE, INVALID_CONFIG); // Yes, ARINC treats non-error thread as INVALID_CONFIG.
        MAP_ERROR(EAGAIN, NO_ACTION);
        MAP_ERROR_EFAULT();
        MAP_ERROR_DEFAULT();
    MAP_ERROR_END()

    if (core_ret == EOK) {
        ERROR_STATUS->ERROR_CODE = error_pok_to_arinc(core_status.error_kind);
        ERROR_STATUS->LENGTH = core_status.msg_size;
        ERROR_STATUS->FAILED_PROCESS_ID = core_status.failed_thread + 1; // ARINC process IDs are one higher
        ERROR_STATUS->FAILED_ADDRESS = (SYSTEM_ADDRESS_TYPE)core_status.failed_addr;
    }
}

void RAISE_APPLICATION_ERROR (ERROR_CODE_TYPE            ERROR_CODE,
                              MESSAGE_ADDR_TYPE          MESSAGE,
                              ERROR_MESSAGE_SIZE_TYPE    LENGTH,
                              RETURN_CODE_TYPE           *RETURN_CODE)
{
    jet_ret_t core_ret;

    if ((ERROR_CODE != APPLICATION_ERROR)) {
        *RETURN_CODE = INVALID_PARAM;
        return;
    }

    core_ret = pok_error_raise_application_error ((char*) MESSAGE, LENGTH);

    MAP_ERROR_BEGIN(core_ret)
        MAP_ERROR(EOK, NO_ERROR);
        MAP_ERROR(EINVAL, INVALID_PARAM);
        MAP_ERROR_EFAULT();
        MAP_ERROR_DEFAULT();
    MAP_ERROR_END()

    *RETURN_CODE = NO_ERROR;
}
