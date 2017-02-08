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
 
  #include <stdio.h>
  #include "INTEGRITY_CHECKER_gen.h"
  #include "afdx.h"
  
  #define C_NAME "INTEGRITY_CHECKER: "
  
void integrity_checker_init(INTEGRITY_CHECKER *self)
{
    self->state.last_in_seq_number = 0;
    self->state.first_message_received = FALSE;
}

ret_t integrity_checker_receive_packet(INTEGRITY_CHECKER *self,
                                            const char *payload, 
                                            size_t payload_size,
                                            SYSTEM_TIME_TYPE arrival_time)
{
    uint8_t seq_numb = payload[payload_size - 1];

    /* Transmission ES restarted */
    if (seq_numb == 0)
    {
        self->state.last_in_seq_number = seq_numb;
        self->state.first_message_received = TRUE;
        return EOK;
    }

    /*
     * The first message is not received
     * accept any message
     */
    if (self->state.first_message_received == FALSE) {
        self->state.last_in_seq_number = seq_numb;
        self->state.first_message_received = TRUE;
        return EOK;
    } else {
        /* SN period ended */
        if (self->state.last_in_seq_number > seq_numb) {
            if (self->state.last_in_seq_number == 254) {
                if (seq_numb == 1) {
                    self->state.last_in_seq_number = seq_numb;
                    ret_t res = INTEGRITY_CHECKER_call_portB_handle(self, payload, payload_size, arrival_time);
                    if (res != EOK)
                        printf(C_NAME"Error in sending");

                    return EOK;
                }
            }
            
            if (self->state.last_in_seq_number == 255) {
                if ((seq_numb == 1) || (seq_numb == 2)) {
                    self->state.last_in_seq_number = seq_numb;
                    ret_t res = INTEGRITY_CHECKER_call_portB_handle(self, payload, payload_size, arrival_time);
                    if (res != EOK)
                        printf(C_NAME"Error in sending");

                    return EOK;
                }
            }
        } else {
            uint8_t diff_betw_sn = seq_numb - self->state.last_in_seq_number;
            if ((diff_betw_sn == 1) || (diff_betw_sn == 2)) {
                self->state.last_in_seq_number = seq_numb;
                ret_t res = INTEGRITY_CHECKER_call_portB_handle(self, payload, payload_size, arrival_time);
                    if (res != EOK)
                        printf(C_NAME"Error in sending");

                return EOK;
            } else
                return EINVAL;
        }
    }

    return EINVAL;
}
                                                
