/*
 * GENERATED! DO NOT MODIFY!
 *
 * Instead of modifying this file, modify the one it generated from (syspart/components/X/config.yaml).
 */
#ifndef __X_GEN_H__
#define __X_GEN_H__


    #include <interfaces/send_flush_t_gen.h>


struct X_state {
    char x;
};

typedef struct {
    struct X_state state;
    struct {
            struct {
                send_flush_t ops;
            } portC;
    } in;
    struct {
    } out;
} X;



      void x_send(X *, void *, size_t);
      void x_flush(X *);




    void x_init(X *);

#endif
