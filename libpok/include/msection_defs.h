#include <uapi/msection.h>

void msection_init(struct msection* section);
void msection_enter(struct msection* section);
void msection_leave(struct msection* section);
pok_ret_t msection_wait(struct msection* section, pok_time_t timeout);
pok_ret_t msection_notify(struct msection* section, pok_thread_id_t thread_id);
void msection_wq_init(struct msection_wq* wq);
void msection_wq_add(struct msection_wq* wq, pok_thread_id_t next);
void msection_wq_add_after(struct msection_wq* wq, pok_thread_id_t prev);
void msection_wq_del(struct msection_wq* wq, pok_thread_id_t thread);
pok_ret_t msection_wq_notify(struct msection* section, struct msection_wq* wq, pok_bool_t is_all);
size_t msection_wq_size(struct msection* section, struct msection_wq* wq);
