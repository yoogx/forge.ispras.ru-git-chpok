#include <uapi/types.h>
#include <uapi/thread_types.h>
#include <uapi/partition_types.h>
#include <uapi/partition_arinc_types.h>
#include <uapi/port_types.h>
#include <uapi/buffer_types.h>
#include <uapi/blackboard_types.h>
#include <uapi/semaphore_types.h>
#include <uapi/event_types.h>
#include <uapi/error_arinc_types.h>
#include <uapi/errno.h>

pok_ret_t pok_thread_create(const char* name, void* entry, const pok_thread_attr_t* attr, pok_thread_id_t* thread_id);
pok_ret_t pok_thread_sleep(const pok_time_t* time);
pok_ret_t pok_thread_sleep_until(const pok_time_t* time);
pok_ret_t pok_sched_end_period(void);
pok_ret_t pok_thread_suspend(const pok_time_t* time);
pok_ret_t pok_sched_get_current(pok_thread_id_t* thread_id);
pok_ret_t pok_thread_get_status(pok_thread_id_t thread_id, char* name, void** entry, pok_thread_status_t* status);
pok_ret_t pok_thread_delayed_start(pok_thread_id_t thread_id, const pok_time_t* time);
pok_ret_t pok_thread_set_priority(pok_thread_id_t thread_id, uint32_t priority);
pok_ret_t pok_thread_resume(pok_thread_id_t thread_id);
pok_ret_t pok_thread_suspend_target(pok_thread_id_t thread_id);
pok_ret_t pok_thread_yield(void);
pok_ret_t pok_sched_replenish(const pok_time_t* budget);
pok_ret_t pok_thread_stop_target(pok_thread_id_t thread_id);
pok_ret_t pok_thread_stop(void);
pok_ret_t pok_thread_find(const char* name, pok_thread_id_t* id);
pok_ret_t pok_partition_set_mode_current(pok_partition_mode_t mode);
pok_ret_t pok_current_partition_get_status(pok_partition_status_t* status);
pok_ret_t pok_current_partition_inc_lock_level(int32_t* lock_level);
pok_ret_t pok_current_partition_dec_lock_level(int32_t* lock_level);
pok_ret_t pok_buffer_create(char* name, pok_message_size_t max_message_size, pok_message_range_t max_nb_message, pok_queuing_discipline_t discipline, pok_buffer_id_t* id);
pok_ret_t pok_buffer_send(pok_buffer_id_t id, const void* data, pok_message_size_t length, const pok_time_t* timeout);
pok_ret_t pok_buffer_receive(pok_buffer_id_t id, const pok_time_t* timeout, void* data, pok_message_size_t* length);
pok_ret_t pok_buffer_get_id(char* name, pok_buffer_id_t* id);
pok_ret_t pok_buffer_status(pok_buffer_id_t id, pok_buffer_status_t* status);
pok_ret_t pok_blackboard_create(const char* name, pok_message_size_t max_message_size, pok_blackboard_id_t* id);
pok_ret_t pok_blackboard_read(pok_blackboard_id_t id, const pok_time_t* timeout, void* data, pok_message_size_t* len);
pok_ret_t pok_blackboard_display(pok_blackboard_id_t id, const void* message, pok_message_size_t len);
pok_ret_t pok_blackboard_clear(pok_blackboard_id_t id);
pok_ret_t pok_blackboard_id(const char* name, pok_blackboard_id_t* id);
pok_ret_t pok_blackboard_status(pok_blackboard_id_t id, pok_blackboard_status_t* status);
pok_ret_t pok_semaphore_create(const char* name, pok_sem_value_t value, pok_sem_value_t max_value, pok_queuing_discipline_t discipline, pok_sem_id_t* id);
pok_ret_t pok_semaphore_wait(pok_sem_id_t id, const pok_time_t* timeout);
pok_ret_t pok_semaphore_signal(pok_sem_id_t id);
pok_ret_t pok_semaphore_id(const char* name, pok_sem_id_t* id);
pok_ret_t pok_semaphore_status(pok_sem_id_t id, pok_semaphore_status_t* status);
pok_ret_t pok_event_create(const char* name, pok_event_id_t* id);
pok_ret_t pok_event_set(pok_event_id_t id);
pok_ret_t pok_event_reset(pok_event_id_t id);
pok_ret_t pok_event_wait(pok_event_id_t id, const pok_time_t* timeout);
pok_ret_t pok_event_id(const char* name, pok_event_id_t* id);
pok_ret_t pok_event_status(pok_event_id_t id, pok_event_status_t* status);
pok_ret_t pok_error_thread_create(uint32_t stack_size, void* entry);
pok_ret_t pok_error_raise_application_error(const char* msg, size_t msg_size);
pok_ret_t pok_error_get(pok_error_status_t* status, void* msg);
pok_ret_t pok_port_sampling_create(const char* name, pok_port_size_t size, pok_port_direction_t direction, const pok_time_t* refresh, pok_port_id_t* id);
pok_ret_t pok_port_sampling_write(pok_port_id_t id, const void* data, pok_port_size_t len);
pok_ret_t pok_port_sampling_read(pok_port_id_t id, void* data, pok_port_size_t* len, pok_bool_t* valid);
pok_ret_t pok_port_sampling_id(const char* name, pok_port_id_t* id);
pok_ret_t pok_port_sampling_status(pok_port_id_t id, pok_port_sampling_status_t* status);
pok_ret_t pok_port_sampling_check(pok_port_id_t id);
pok_ret_t pok_port_queuing_create_packed(const char* name, const pok_port_queuing_create_arg_t* arg, pok_port_id_t* id);
pok_ret_t pok_port_queuing_send(pok_port_id_t id, const void* data, pok_port_size_t len, const pok_time_t* timeout);
pok_ret_t pok_port_queuing_receive(pok_port_id_t id, const pok_time_t* timeout, void* data, pok_port_size_t* len);
pok_ret_t pok_port_queuing_id(const char* name, pok_port_id_t* id);
pok_ret_t pok_port_queuing_status(pok_port_id_t id, pok_port_queuing_status_t* status);
