/*
 * GENERATED! DO NOT MODIFY!
 *
 * Instead of modifying this file, modify the one it generated from (syspart/components/X/config.yaml).
 */
#ifndef __X1_GEN_H__
#define __X1_GEN_H__


    #include <interfaces/send_flush_t_gen.h>


struct X1_state {
        char x;
};

typedef struct {
    struct X1_state state;
    struct {
            struct {
                send_flush_t ops;
            } portC;
    } in;
    struct {
    } out;
} X1;


      void x_send(X1 *
, void *, size_t);
      void x_flush(X1 *
);




    void x_init(X1 *);

#endif
