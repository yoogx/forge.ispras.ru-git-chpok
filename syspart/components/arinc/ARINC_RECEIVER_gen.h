/*
 * GENERATED! DO NOT MODIFY!
 *
 * Instead of modifying this file, modify the one it generated from (syspart/components/arinc/config.yaml).
 */
#ifndef __ARINC_RECEIVER_GEN_H__
#define __ARINC_RECEIVER_GEN_H__

    #include <arinc653/queueing.h>
    #include <arinc653/sampling.h>
    #include <port_info.h>

    #include <interfaces/message_handler_gen.h>


typedef struct ARINC_RECEIVER_state {
    PORT_DIRECTION_TYPE port_direction;
    MESSAGE_RANGE_TYPE q_port_max_nb_messages;
    MESSAGE_SIZE_TYPE port_max_message_size;
    NAME_TYPE port_name;
    int is_queuing_port;
    APEX_INTEGER port_id;
}ARINC_RECEIVER_state;

typedef struct {
    ARINC_RECEIVER_state state;
    struct {
            struct {
                message_handler ops;
            } portA;
    } in;
    struct {
    } out;
} ARINC_RECEIVER;



      ret_t arinc_receive_message(ARINC_RECEIVER *, const char *, size_t);




    void arinc_receiver_init(ARINC_RECEIVER *);



#endif
