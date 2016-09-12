/*
 * GENERATED! DO NOT MODIFY!
 *
 * Instead of modifying this file, modify the one it generated from (syspart/components/arinc/config.yaml).
 */
#ifndef __ARINC_SENDER_GEN_H__
#define __ARINC_SENDER_GEN_H__


    #include <interfaces/send_flush_t_gen.h>


struct ARINC_SENDER_state {
        char x;
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


      void x_send(ARINC_SENDER *
, void *, size_t);
      void x_flush(ARINC_SENDER *
);




    void arinc_sender_init(ARINC_SENDER *);

#endif
