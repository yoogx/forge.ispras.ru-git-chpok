#include <config.h>
#include <types.h>
#include <middleware/blackboard.h>
#include <arinc653/arincutils.h>

typedef struct pok_buffer_wait_list_t
{
    struct pok_buffer_wait_list_t *next;

    pok_thread_id_t thread;
    int priority;
    int64_t timeout;
    pok_ret_t result;
    
    union {
        struct {
            const char *data_ptr;
            pok_port_size_t data_size;
        } sending;

        struct {
            char *data_ptr;
            pok_port_size_t *data_size_ptr;
        } receiving;
    };
} pok_buffer_wait_list_t;

#define POK_BUFFER_MAX_NAME_LENGTH 30
typedef struct
{
   pok_bool_t                   ready;
   void                         *buffer;
   pok_port_size_t              msg_size;
   pok_port_size_t              head_index;
   pok_port_size_t              number_of_messages;          
   pok_port_size_t              max_number_of_messages;       
   pok_queueing_discipline_t    discipline;
   pok_event_id_t               lock;
   pok_buffer_wait_list_t       *wait_list;
   char                         name[POK_BUFFER_MAX_NAME_LENGTH];
} pok_buffer_t;

#define POK_EVENT_MAX_NAME_LENGTH 30
typedef struct
{
   pok_event_id_t    core_id;
   pok_bool_t        ready;
   pok_bool_t        is_up;
   char              name[POK_EVENT_MAX_NAME_LENGTH];
}pok_arinc653_event_layer_t;

#define POK_SEM_MAX_NAME_LENGTH 30
typedef struct
{
   pok_bool_t        ready;
   pok_sem_id_t      core_id;
   char              name[POK_SEM_MAX_NAME_LENGTH];
} pok_arinc653_semaphore_layer_t;

pok_blackboard_t *pok_blackboards;
char *pok_blackboards_data;
pok_buffer_t *pok_buffers;
char *pok_buffers_data;
ARINC_ATTRIBUTE *arinc_process_attribute;
pok_arinc653_event_layer_t *pok_arinc653_events_layers;
pok_arinc653_semaphore_layer_t *pok_arinc653_semaphores_layers;

void pok_libpok_config_init() {
    pok_blackboard_t tmp_pok_blackboards[POK_CONFIG_NB_BLACKBOARDS];
    pok_blackboards = tmp_pok_blackboards;
    
    char tmp_pok_blackboards_data[POK_CONFIG_BLACKBOARD_DATA_SIZE]; 
    pok_blackboards_data = tmp_pok_blackboards_data;
    
    pok_buffer_t tmp_pok_buffers[POK_CONFIG_NB_BUFFERS];
    pok_buffers = tmp_pok_buffers;
    
    char tmp_pok_buffers_data[POK_CONFIG_BUFFER_DATA_SIZE]; 
    pok_buffers_data = tmp_pok_buffers_data;
    
    ARINC_ATTRIBUTE tmp_arinc_process_attribute[POK_CONFIG_NB_THREADS];
    arinc_process_attribute = tmp_arinc_process_attribute;
    
    pok_arinc653_event_layer_t tmp_pok_arinc653_events_layers[POK_CONFIG_NB_EVENTS];
    pok_arinc653_events_layers = tmp_pok_arinc653_events_layers;
    
    pok_arinc653_semaphore_layer_t tmp_pok_arinc653_semaphores_layers[POK_CONFIG_ARINC653_NB_SEMAPHORES];
    pok_arinc653_semaphores_layers = tmp_pok_arinc653_semaphores_layers;
}
