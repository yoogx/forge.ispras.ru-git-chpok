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

ret_t ic_send_packet(INTEGRITY_CHECKER *self,
                        const uint8_t *afdx_packet,
                        size_t afdx_packet_size,
                        SYSTEM_TIME_TYPE arrival_time)
{
    ret_t res = INTEGRITY_CHECKER_call_portB_handle(self, afdx_packet, afdx_packet_size, arrival_time);
    if (res != EOK)
        printf(C_NAME"%s Error in sending", self->instance_name);

    return res;
}

void set_expected_sn(INTEGRITY_CHECKER *self, uint8_t seq_numb)
{
    /* check that expected_seq_number != 0 (we expect 0 only if we restarted) */
    self->state.expected_seq_number = (seq_numb % 255) + 1;
}

void integrity_checker_init(INTEGRITY_CHECKER *self)
{
    self->state.expected_seq_number = 0;
}

ret_t integrity_checker_receive_packet(INTEGRITY_CHECKER *self,
                                            const uint8_t *afdx_packet,
                                            size_t afdx_packet_size,
                                            SYSTEM_TIME_TYPE arrival_time)
{
    ret_t ret = check_afdx_frame_size(afdx_packet_size);
    if (ret != EOK) {
        return ret;
    }

    uint8_t seq_numb = afdx_packet[afdx_packet_size - 1];
    uint16_t diff_betw_sn = (seq_numb < self-> state.expected_seq_number) ?
                    (seq_numb + 255 - self->state.expected_seq_number) :
                    (seq_numb - self->state.expected_seq_number);

    if (self->state.expected_seq_number == 0 ||
        seq_numb == 0 ||
        diff_betw_sn < 2) {
        /*
         * Correct situations:
         * First incoming message during the work of this instance or
         * the sender of this vl sent the first message (or restarted) or
         * the message with the correct number
         */
            set_expected_sn(self, seq_numb);
            return ic_send_packet(self, afdx_packet, afdx_packet_size, arrival_time);

    } else {
        /*
         * Wrong situation:
         * we only need to set sn
         */
        set_expected_sn(self, seq_numb);
        return EOK; // or another return code?
    }
}
