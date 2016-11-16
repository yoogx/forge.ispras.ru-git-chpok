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
 
#include <config.h>
 
#ifdef POK_NEEDS_ARINC653_MUTEX

#include <arinc653/types.h>
#include <arinc653/mutex.h>

static size_t nmutexes_used = 0;

void CREATE_MUTEX(
	MUTEX_NAME_TYPE			MUTEX_NAME,
	PRIORITY_TYPE			MUTEX_PRIORITY,
	QUEUING_DISCIPLINE_TYPE	QUEUING_DISCIPLINE,
	MUTEX_ID_TYPE			*MUTEX_ID,
	RETURN_CODE_TYPE		*RETURN_CODE)
{
	*MUTEX_ID = ++nmutexes_used;
	*RETURN_CODE = NO_ERROR;
}


void ACQUIRE_MUTEX(
	MUTEX_ID_TYPE			MUTEX_ID,
	SYSTEM_TIME_TYPE		TIME_OUT,
	RETURN_CODE_TYPE		*RETURN_CODE)
{
	*RETURN_CODE = NO_ERROR;
}

void RELEASE_MUTEX(
	MUTEX_ID_TYPE			MUTEX_ID,
	RETURN_CODE_TYPE		*RETURN_CODE)
{
	*RETURN_CODE = NO_ERROR;
}
	
void RESET_MUTEX(
	MUTEX_ID_TYPE			MUTEX_ID,
	PROCESS_ID_TYPE			PROCESS_ID,
	RETURN_CODE_TYPE		*RETURN_CODE)
{
	*RETURN_CODE = NO_ERROR;
}
	
void GET_MUTEX_ID(
	MUTEX_NAME_TYPE			MUTEX_NAME,
	MUTEX_ID_TYPE			*MUTEX_ID,
	RETURN_CODE_TYPE		*RETURN_CODE)
{
	*MUTEX_ID = nmutexes_used;
	*RETURN_CODE = NO_ERROR;
}

void GET_MUTEX_STATUS(
	MUTEX_ID_TYPE			MUTEX_ID,
	MUTEX_STATUS_TYPE		*MUTEX_STATUS,
	RETURN_CODE_TYPE		*RETURN_CODE)
{
	MUTEX_STATUS->MUTEX_STATE = AVAILABLE;
	MUTEX_STATUS->MUTEX_OWNER = NULL_PROCESS_ID;
	MUTEX_STATUS->MUTEX_PRIORITY = MIN_PRIORITY_VALUE;
	MUTEX_STATUS->LOCK_COUNT = 0;
	MUTEX_STATUS->WAITING_PROCESSES = 0;
	
	*RETURN_CODE = NO_ERROR;
}
	
void GET_PROCESS_MUTEX_STATE(
	PROCESS_ID_TYPE			PROCESS_ID,
	MUTEX_ID_TYPE			*MUTEX_ID,
	RETURN_CODE_TYPE		*RETURN_CODE)
{
	*MUTEX_ID = NO_MUTEX_OWNED;
	*RETURN_CODE = NO_ERROR;
}
#endif

