/*------------------------------------------------------------------*/
/*                                                                  */
/* Logbook system constant and type definitions and management      */
/* services                                                         */
/*                                                                  */
/*------------------------------------------------------------------*/

#ifndef APEX_LOGBOOK_SYSTEM
#define APEX_LOGBOOK_SYSTEM

#include <config.h>


/* Types */

typedef APEX_INTEGER    LOGBOOK_ID_TYPE;
typedef NAME_TYPE       LOGBOOK_NAME_TYPE;
typedef
    enum {
        ABORTED     = 0,
        IN_PROGRESS = 1,
        COMPLETE    = 2
} WRITE_STATUS_TYPE;

typedef
    struct {
        APEX_INTEGER    MAX_MESSAGE_SIZE;
        APEX_INTEGER    MAX_NB_LOGGED_MESSAGES;
        APEX_INTEGER    MAX_NB_IN_PROGRESS_MESSAGES;
        APEX_INTEGER    NB_LOGGED_MESSAGES;
        APEX_INTEGER    NB_IN_PROGRESS_MESSAGES;
        APEX_INTEGER    NB_ABORTED_MESSAGES;
} LOGBOOK_STATUS_TYPE;

/* Services */

extern void CREATE_LOGBOOK(
    /*in */ LOGBOOK_NAME_TYPE     LOGBOOK_NAME,
    /*in */ MESSAGE_SIZE_TYPE     MAX_MESSAGE_SIZE,
    /*in */ MESSAGE_RANGE_TYPE    MAX_NB_LOGGED_MESSAGES,
    /*in */ MESSAGE_RANGE_TYPE    MAX_NB_IN_PROGRESS_MESSAGES,
    /*out*/ LOGBOOK_ID_TYPE       *LOGBOOK_ID,
    /*out*/ RETURN_CODE_TYPE      *RETURN_CODE);

extern void WRITE_LOGBOOK(
    /*in */ LOGBOOK_ID_TYPE      LOGBOOK_ID,
    /*in */ MESSAGE_ADDR_TYPE    MESSAGE_ADDR,
    /*in */ MESSAGE_SIZE_TYPE    LENGTH,
    /*out*/ RETURN_CODE_TYPE     *RETURN_CODE);

extern void OVERWRITE_LOGBOOK(
    /*in */ LOGBOOK_ID_TYPE      LOGBOOK_ID,
    /*in */ MESSAGE_ADDR_TYPE    MESSAGE_ADDR,    
    /*in */ MESSAGE_SIZE_TYPE    LENGTH,
    /*out*/ RETURN_CODE_TYPE    *RETURN_CODE);

extern void READ_LOGBOOK(
    /*in */ LOGBOOK_ID_TYPE       LOGBOOK_ID,
    /*in */ MESSAGE_RANGE_TYPE    LOGBOOK_ENTRY,
    /*in */ MESSAGE_ADDR_TYPE     MESSAGE_ADDR,
    /*out*/ MESSAGE_SIZE_TYPE     *LENGTH,
    /*out*/ WRITE_STATUS_TYPE     *WRITE_STATUS,
    /*out*/ RETURN_CODE_TYPE      *RETURN_CODE);

extern void CLEAR_LOGBOOK(
    /*in */ LOGBOOK_ID_TYPE       LOGBOOK_ID,
    /*out*/ RETURN_CODE_TYPE      *RETURN_CODE);

extern void GET_LOGBOOK_ID(
    /*in */ LOGBOOK_NAME_TYPE     LOGBOOK_NAME,
    /*out*/ LOGBOOK_ID_TYPE       *LOGBOOK_ID,
    /*out*/ RETURN_CODE_TYPE      *RETURN_CODE);

extern void GET_LOGBOOK_STATUS(
    /*in */ LOGBOOK_ID_TYPE        LOGBOOK_ID,
    /*out*/ LOGBOOK_STATUS_TYPE    *LOGBOOK_STATUS,
    /*out*/ RETURN_CODE_TYPE       *RETURN_CODE);
#endif
