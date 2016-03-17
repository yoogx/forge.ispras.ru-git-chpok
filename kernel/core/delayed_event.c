#include <core/delayed_event.h>

void delayed_event_queue_init(struct delayed_event_queue* q)
{
	INIT_LIST_HEAD(&q->events);
}

/**
 * Emit all events which have time expired.
 * 
 * For each such event @process_event is called, and event is removed
 * from the queue.
 */
void delayed_event_queue_check(struct delayed_event_queue* q, pok_time_t time,
	process_event_t process_event)
{
	while(!list_empty(&q->events))
	{
		struct delayed_event* event = list_first_entry(&q->events, struct delayed_event, elem);
		if(event->timepoint > time) break;
		
		list_del_init(&event->elem);
		process_event(event);
	}
}

/**
 *  Initialize delayed event.
 * 
 * Initially event doesn't belong to any queue.
 */
void delayed_event_init(struct delayed_event* event)
{
	INIT_LIST_HEAD(&event->elem);
}

/** 
 * Add delayed event to the queue.
 * 
 * Initially event should either be not added to any timer,
 * or should be added to queue `q`.
 * 
 * In the last case event will (possibly) be reordered in the queue.
 */
void delayed_event_add(struct delayed_event* event, pok_time_t timepoint,
	struct delayed_event_queue* q)
{
	list_del_init(&event->elem); // Remove event from queue, if it was.
	
	event->timepoint = timepoint;
	struct delayed_event* event_other;
	
	list_for_each_entry(event_other, &q->events, elem)
	{
		if(event_other->timepoint >= event->timepoint)
		{
			list_add_tail(&event->elem, &event_other->elem);
			return;
		}
	}
	
	list_add_tail(&event->elem, &q->events);
}

/** Delete event from the queue. */
void delayed_event_remove(struct delayed_event* event)
{
	list_del_init(&event->elem);
}
