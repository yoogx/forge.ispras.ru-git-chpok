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
#ifndef	APEX_MUTEX
#define	APEX_MUTEX

#include <arinc653/types.h>
#include <arinc653/process.h>

#define	MAX_NUMBER_OF_MUTEXES			SYSTEM_LIMIT_NUMBER_OF_MUTEXES

typedef	NAME_TYPE		MUTEX_NAME_TYPE;

typedef	APEX_INTEGER	MUTEX_ID_TYPE;

#define	NO_MUTEX_OWNED					-2
#define	PREEMPTION_LOCK_MUTEX			-3

typedef	APEX_INTEGER	LOCK_COUNT_TYPE;

typedef	enum {AVAILABLE = 0, OWNED = 1} MUTEX_STATE_TYPE;

typedef
	struct
	{
		MUTEX_STATE_TYPE	MUTEX_STATE;
		PROCESS_ID_TYPE		MUTEX_OWNER;
		PRIORITY_TYPE		MUTEX_PRIORITY;
		LOCK_COUNT_TYPE		LOCK_COUNT;
		WAITING_RANGE_TYPE	WAITING_PROCESSES;
	} MUTEX_STATUS_TYPE;
	
extern void CREATE_MUTEX(
	/*in */	MUTEX_NAME_TYPE			MUTEX_NAME,
	/*in */ PRIORITY_TYPE			MUTEX_PRIORITY,
	/*in */ QUEUING_DISCIPLINE_TYPE	QUEUING_DISCIPLINE,
	/*out*/	MUTEX_ID_TYPE			*MUTEX_ID,
	/*out*/	RETURN_CODE_TYPE		*RETURN_CODE);
	
extern void ACQUIRE_MUTEX(
	/*in */	MUTEX_ID_TYPE			MUTEX_ID,
	/*in */	SYSTEM_TIME_TYPE		TIME_OUT,
	/*out*/	RETURN_CODE_TYPE		*RETURN_CODE);
	
extern void RELEASE_MUTEX(
	/*in */	MUTEX_ID_TYPE			MUTEX_ID,
	/*out*/	RETURN_CODE_TYPE		*RETURN_CODE);
	
extern void RESET_MUTEX(
	/*in */	MUTEX_ID_TYPE			MUTEX_ID,
	/*in */	PROCESS_ID_TYPE			PROCESS_ID,
	/*out*/	RETURN_CODE_TYPE		*RETURN_CODE);
	
extern void	GET_MUTEX_ID(
	/*in */	MUTEX_NAME_TYPE			MUTEX_NAME,
	/*out*/	MUTEX_ID_TYPE			*MUTEX_ID,
	/*out*/	RETURN_CODE_TYPE		*RETURN_CODE);

extern void	GET_MUTEX_STATUS(
	/*in */	MUTEX_ID_TYPE			MUTEX_ID,
	/*out*/	MUTEX_STATUS_TYPE		*MUTEX_STATUS,
	/*out*/	RETURN_CODE_TYPE		*RETURN_CODE);
	
extern void GET_PROCESS_MUTEX_STATE(
	/*in */	PROCESS_ID_TYPE			PROCESS_ID,
	/*out*/	MUTEX_ID_TYPE			*MUTEX_ID,
	/*out*/	RETURN_CODE_TYPE		*RETURN_CODE);

#endif
#endif
