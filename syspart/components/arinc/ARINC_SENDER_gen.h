/*
 * GENERATED! DO NOT MODIFY!
 *
 * Instead of modifying this file, modify the one it generated from (syspart/components/arinc/config.yaml).
 */
#ifndef __ARINC_SENDER_GEN_H__
#define __ARINC_SENDER_GEN_H__

    #include <arinc653/queueing.h>
    #include <arinc653/sampling.h>

    #include <interfaces/send_flush_t_gen.h>


struct ARINC_SENDER_state {
        NAME_TYPE port_name;
        PORT_DIRECTION_TYPE port_direction;
        unsigned overhead;
        MESSAGE_SIZE_TYPE max_message_size;
        int is_queuing_port;
        MESSAGE_RANGE_TYPE q_max_nb_message;
        APEX_INTEGER id;
};

typedef struct {
    struct ARINC_SENDER_state state;
    struct {
            struct {
                send_flush_t ops;
            } portC;
    } in;
    struct {
    } out;
} ARINC_SENDER;



      void x_send(ARINC_SENDER *, void *, size_t);
      void x_flush(ARINC_SENDER *);




    void arinc_sender_init(ARINC_SENDER *);

#endif
