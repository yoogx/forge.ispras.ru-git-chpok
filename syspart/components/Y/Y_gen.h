/*
 * GENERATED! DO NOT MODIFY!
 *
 * Instead of modifying this file, modify the one it generated from (syspart/components/Y/config.yaml).
 */
#ifndef __Y_GEN_H__
#define __Y_GEN_H__


    #include <interfaces/tick_t_gen.h>

    #include <interfaces/port_example_gen.h>

typedef struct Y_state {
    int y;
}Y_state;

typedef struct {
    Y_state state;
    struct {
            struct {
                tick_t ops;
            } portA;
    } in;
    struct {
            struct {
                port_example *ops;
                self_t *owner;
            } portB;
    } out;
} Y;



      void y_tick(Y *);






#endif
