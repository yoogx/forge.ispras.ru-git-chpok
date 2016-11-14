#ifndef __RECEIVING_H_
#define __RECEIVING_H_


#include <afdx/AFDX_ES.h>
#include <stdlib.h>


/* This structure (integrity_check_data_t) describes the information needed
 * for receiving ES.
 *
 * last_in_seq_number           -the number of the last incoming message
 * first_message_received       -the boolean flag,
 *                        0 - the first message is not received,
 *                        1 - the first nessage is received.
 */
typedef struct
{
    uint8_t                last_in_seq_number;
    pok_bool_t             first_message_received;

} integrity_check_data_t;


/* This structure (redundancy_management_data_t) describes the information needed
 * for receiving ES.
 *
 * arrival_time            -in this buffer we safe information about accepted message per SN
 *                      -time of arriving
 *
 * last_accepted_seq_numb    -sequence umber of the last accepted message from both subnetworks
 * last_accepted_msg_time    -time of the last accepted message from both subnetworks
 *
 */
 typedef struct
 {
    uint8_t                   last_accepted_seq_numb;
    pok_time_t                last_accepted_msg_time;
    pok_time_t                arrival_time[SUBNETWORKS_COUNT][MAX_SEQUENCE_NUMBER + 1];
 } redundancy_management_data_t;

#endif
