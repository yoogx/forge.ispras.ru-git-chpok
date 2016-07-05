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

#include <arinc653/logbook.h>

void CREATE_LOGBOOK ( LOGBOOK_NAME_TYPE LOGBOOK_NAME,
                      MESSAGE_SIZE_TYPE MAX_MESSAGE_SIZE,
                      MESSAGE_RANGE_TYPE MAX_NB_LOGGED_MESSAGES,
                      MESSAGE_RANGE_TYPE MAX_NB_IN_PROGRESS_MESSAGES,
                      LOGBOOK_ID_TYPE *LOGBOOK_ID,
                      RETURN_CODE_TYPE *RETURN_CODE )
{
    return;
}

void WRITE_LOGBOOK ( LOGBOOK_ID_TYPE LOGBOOK_ID,
                     MESSAGE_ADDR_TYPE MESSAGE_ADDR,
                     MESSAGE_SIZE_TYPE LENGTH,
                     RETURN_CODE_TYPE *RETURN_CODE )
{
    return;
}

void OVERWRITE_LOGBOOK ( LOGBOOK_ID_TYPE LOGBOOK_ID,
                         MESSAGE_ADDR_TYPE MESSAGE_ADDR,
                         MESSAGE_SIZE_TYPE LENGTH,
                         RETURN_CODE_TYPE *RETURN_CODE )
{
    return;
}

void READ_LOGBOOK ( LOGBOOK_ID_TYPE LOGBOOK_ID,
                    MESSAGE_RANGE_TYPE LOGBOOK_ENTRY,
                    MESSAGE_ADDR_TYPE MESSAGE_ADDR,
                    MESSAGE_SIZE_TYPE *LENGTH,
                    WRITE_STATUS_TYPE *WRITE_STATUS,
                    RETURN_CODE_TYPE *RETURN_CODE )
{
    return;
}

void CLEAR_LOGBOOK ( LOGBOOK_ID_TYPE LOGBOOK_ID,
                     RETURN_CODE_TYPE *RETURN_CODE )
{
    return;
}

void GET_LOGBOOK_ID ( LOGBOOK_NAME_TYPE LOGBOOK_NAME,
                      LOGBOOK_ID_TYPE *LOGBOOK_ID,
                      RETURN_CODE_TYPE *RETURN_CODE )
{
    return;
}

void GET_LOGBOOK_STATUS ( LOGBOOK_ID_TYPE LOGBOOK_ID,
                          LOGBOOK_STATUS_TYPE *LOGBOOK_STATUS,
                          RETURN_CODE_TYPE *RETURN_CODE )
{
    return;
}
