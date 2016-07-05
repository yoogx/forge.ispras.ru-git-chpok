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

#ifndef __JET_LOGBOOK_H__
#define __JET_LOGBOOK_H__

#include <arinc653/types.h>


/*----------------------------------------------------------------------*/
/* */
/* LOGBOOK type definitions */
/* */
/*----------------------------------------------------------------------*/
/*------------------------------*/
/* logbook ident type */
/*------------------------------*/
typedef APEX_INTEGER LOGBOOK_ID_TYPE;
/*------------------------------*/
/* event name type */
/*------------------------------*/
typedef NAME_TYPE LOGBOOK_NAME_TYPE;
/*------------------------------*/
/* write status type */
/*------------------------------*/
enum WRITE_STATUS_VALUE_TYPE { ABORTED, IN_PROGRESS, COMPLETE };

typedef enum WRITE_STATUS_VALUE_TYPE WRITE_STATUS_TYPE;

/*------------------------------*/
/* logbook status type            */
/*------------------------------*/
typedef struct {
    APEX_INTEGER MAX_MESSAGE_SIZE;
    APEX_INTEGER MAX_NB_LOGGED_MESSAGES;
    APEX_INTEGER MAX_NB_IN_PROGRESS_MESSAGES;
    APEX_INTEGER NB_LOGGED_MESSAGES;
    APEX_INTEGER NB_IN_PROGRESS_MESSAGES;
    APEX_INTEGER NB_ABORTED_MESSAGES;
} LOGBOOK_STATUS_TYPE;

/*----------------------------------------------------------------------*/
/* */
/* logbook management services */
/* */
/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
void CREATE_LOGBOOK (
      /*IN */ LOGBOOK_NAME_TYPE LOGBOOK_NAME,
      /*IN */ MESSAGE_SIZE_TYPE MAX_MESSAGE_SIZE,
      /*IN */ MESSAGE_RANGE_TYPE MAX_NB_LOGGED_MESSAGES,
      /*IN */ MESSAGE_RANGE_TYPE MAX_NB_IN_PROGRESS_MESSAGES,
      /*OUT*/ LOGBOOK_ID_TYPE *LOGBOOK_ID,
      /*OUT*/ RETURN_CODE_TYPE *RETURN_CODE );
/*----------------------------------------------------------------------*/
void WRITE_LOGBOOK (
      /*IN */ LOGBOOK_ID_TYPE LOGBOOK_ID,
      /*IN */ MESSAGE_ADDR_TYPE MESSAGE_ADDR,
      /*IN */ MESSAGE_SIZE_TYPE LENGTH,
      /*OUT*/ RETURN_CODE_TYPE *RETURN_CODE );
/*----------------------------------------------------------------------*/
void OVERWRITE_LOGBOOK (
      /*IN */ LOGBOOK_ID_TYPE LOGBOOK_ID,
      /*IN */ MESSAGE_ADDR_TYPE MESSAGE_ADDR,
      /*IN */ MESSAGE_SIZE_TYPE LENGTH,
      /*OUT*/ RETURN_CODE_TYPE *RETURN_CODE );
/*----------------------------------------------------------------------*/
void READ_LOGBOOK (
      /*IN */ LOGBOOK_ID_TYPE LOGBOOK_ID,
      /*IN */ MESSAGE_RANGE_TYPE LOGBOOK_ENTRY,
      /*IN */ MESSAGE_ADDR_TYPE MESSAGE_ADDR,
      /*OUT*/ MESSAGE_SIZE_TYPE *LENGTH,
      /*OUT*/ WRITE_STATUS_TYPE *WRITE_STATUS,
      /*OUT*/ RETURN_CODE_TYPE *RETURN_CODE );
/*----------------------------------------------------------------------*/
void CLEAR_LOGBOOK (
      /*IN */ LOGBOOK_ID_TYPE LOGBOOK_ID,
      /*OUT*/ RETURN_CODE_TYPE *RETURN_CODE );
/*----------------------------------------------------------------------*/
void GET_LOGBOOK_ID (
      /*IN */ LOGBOOK_NAME_TYPE LOGBOOK_NAME,
      /*OUT*/ LOGBOOK_ID_TYPE *LOGBOOK_ID,
      /*OUT*/ RETURN_CODE_TYPE *RETURN_CODE );
/*----------------------------------------------------------------------*/
void GET_LOGBOOK_STATUS (
      /*IN */ LOGBOOK_ID_TYPE LOGBOOK_ID,
      /*OUT*/ LOGBOOK_STATUS_TYPE *LOGBOOK_STATUS,
      /*OUT*/ RETURN_CODE_TYPE *RETURN_CODE );
/*----------------------------------------------------------------------*/
#endif
