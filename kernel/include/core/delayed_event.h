#ifndef __POK_DELAYED_EVENT_H__
#define __POK_DELAYED_EVENT_H__

/*
 * Delayed events - events which should fire at specific time point
 */

#include <list.h>
#include <types.h>

/** Event which should occure at a specific time point. */
struct delayed_event {
	struct list_head elem;
	pok_time_t timepoint;
};

typedef void (*process_event_t)(struct delayed_event* event);

/** 
 * Queue of delayed events of same type.
 */
struct delayed_event_queue {
	struct list_head events;
};

/** Initialize delayed events queue. */
void delayed_event_queue_init(struct delayed_event_queue* q);

/**
 * Emit all events which have time expired.
 * 
 * For each such event @process_event is called, and event is removed
 * from the queue.
 */
void delayed_event_queue_check(struct delayed_event_queue* q, pok_time_t time,
	process_event_t process_event);

/**
 *  Initialize delayed event.
 * 
 * Initially event doesn't belong to any queue.
 */
void delayed_event_init(struct delayed_event* event);

/** 
 * Add delayed event to the queue.
 * 
 * Initially event should either be not added to any timer,
 * or should be added to queue `q`.
 * 
 * In the last case event will (possibly) be reordered in the queue.
 */
void delayed_event_add(struct delayed_event* event, pok_time_t timepoint,
	struct delayed_event_queue* q);

/** Delete event from the queue. */
void delayed_event_remove(struct delayed_event* event);

#endif /* ! __POK_DELAYED_EVENT_H__*/
