/*
 * GENERATED! DO NOT MODIFY!
 *
 * Instead of modifying this file, modify the one it generated from (syspart/components/arinc/config.yaml).
 */
#ifndef __ARINC_SENDER_GEN_H__
#define __ARINC_SENDER_GEN_H__

    #include <arinc653/queueing.h>
    #include <arinc653/sampling.h>
    #include <port_info.h>


    #include <interfaces/send_net_data_gen.h>

typedef struct ARINC_SENDER_state {
    PORT_DIRECTION_TYPE port_direction;
    MESSAGE_RANGE_TYPE q_port_max_nb_messages;
    sys_port_data_t * port_buffer;
    MESSAGE_SIZE_TYPE port_max_message_size;
    unsigned overhead;
    NAME_TYPE port_name;
    int is_queuing_port;
    APEX_INTEGER port_id;
}ARINC_SENDER_state;

typedef struct {
    ARINC_SENDER_state state;
    struct {
    } in;
    struct {
            struct {
                send_net_data *ops;
                self_t *owner;
            } portA;
    } out;
} ARINC_SENDER;




      ret_t ARINC_SENDER_call_portA_send(ARINC_SENDER *, char *, size_t, size_t);
      ret_t ARINC_SENDER_call_portA_flush(ARINC_SENDER *);



    void arinc_sender_init(ARINC_SENDER *);

    void arinc_sender_activity(ARINC_SENDER *);


#endif
