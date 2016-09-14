#ifndef __MACROS_H__
#define __MACROS_H__


//XXX: wrap everything do-while???

/*
#define CALL_PORT_FUNCTION(SELF, OUT_PORT, FUNC, ARGS...)\
    do {if (SELF->out.OUT_PORT.ops->FUNC)\
            SELF->out.OUT_PORT.ops->FUNC(SELF->out.OUT_PORT.owner, ##ARGS);} while(0)
*/

#define CALL_PORT_FUNCTION(SELF, OUT_PORT, FUNC, ARGS...) SELF->out.OUT_PORT.ops->FUNC(SELF->out.OUT_PORT.owner, ##ARGS)

//TODO ADD CALL_PORT_FUNCTION WITH ASSERT!!

typedef void self_t;

#endif
