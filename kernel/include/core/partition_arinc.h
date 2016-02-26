#ifndef __POK_PARTITION_ARINC_H__
#define __POK_PARTITION_ARINC_H__

/**
 * \enum pok_partition_mode_t
 * \brief The different modes of a partition
 */
typedef enum
{ 
   /*
    * In init mode, only main thread (process) is run.
    * This's the only mode where one can create various resources.
    *
    * There's really no difference between cold and warm init.
    *   
    * When partition is initially started, it's in cold init.
    *
    * HM table and set_partition_mode function may restart 
    * partition into either cold or warm init mode.
    *
    * The exact type of init can be introspected by an application,
    * but otherwise, it makes no difference.
    */
   POK_PARTITION_MODE_INIT_COLD = 1, 
   POK_PARTITION_MODE_INIT_WARM = 2,

   /*
    * In normal mode, all threads except main can be run.
    *
    * No resources can be allocated.
    */
   POK_PARTITION_MODE_NORMAL    = 3, 

   /*
    * Partition is stopped.
    */
   POK_PARTITION_MODE_IDLE      = 4,
}pok_partition_mode_t;

/*!
 * \struct pok_partition_t
 * \brief This structure contains all needed information for partition management
 */
typedef struct
{
    pok_partition_t        base_part;

    pok_partition_mode_t   mode;           /**< Current mode of the partition */

    uint32_t               size;           /**< Size of the allocated memory segment */
    uint32_t               base_addr;    /**< The base address inside the whole memory (where the segment is in the whole memory ?) */
    uint32_t               base_vaddr;   /**< The virtual address of the partition. The address the threads sees when they are executed */
    
    uint32_t               user_stack_state; /* State of the use-space allocation. */

    uint32_t               nthreads;       /**< Number of (allocated) threads inside the partition */
    pok_thread_t*          threads;        /** Array of threads. */
    uint32_t               nthreads_used;   /**< Number of threads which are currently in use (created). */


    pok_thread_t*          thread_current; // Normal thread or special thread. NULL if doing nothing.


    pok_port_queuing_t*    ports_queuing; /* List of queuing ports. Set in deployment.c. */
    size_t                 nports_queuing;

    pok_port_sampling_t*   ports_sampling; /* List of sampling ports. Set in deployment.c. */
    size_t                 nports_sampling;


#if defined(POK_NEEDS_LOCKOBJECTS) || defined(POK_NEEDS_ERROR_HANDLING)
    uint8_t                lockobj_index_low;   /**< The low bound in the lockobject array. */
    uint8_t                lockobj_index_high;  /**< The high bound in the lockobject array */
    uint8_t                nlockobjs;           /**< The amount of lockobjects reserved for the partition */
#endif

/* Error and main threads are special in sence, that they cannot be reffered by ID.*/

#ifdef POK_NEEDS_ERROR_HANDLING
    pok_thread_t*          thread_error;     /**< Error thread. One of the @threads. */
    pok_error_status_t     error_status;       /**< A pointer used to store information about errors */
#endif

    uintptr_t              main_entry;
    uint32_t               main_user_stack_size;


    uint32_t		         lock_level;
    pok_thread_t*          thread_locked; /* Thread which locks preemption. */
   
    pok_partition_id_t		partition_id;
    
    int                    preempt_local_counter;
    
    /**
     * Priority/FIFO ordered queue of eligible threads.
     * 
     * Used only in NORMAL mode.
     */
    struct list_head       eligible_threads; 
    
    /**
     * Queue of threads with deadline events.
     * 
     * Corresponded thread's field is .thread_deadline_event.
     */
    struct delayed_event_queue queue_deadline;
    /** 
     * Queue of delayed events, which should awoke thread
     * (or make it closer to RUNNING state).
     * 
     * Corresponded thread's field is .thread_delayed_event.
     */
    struct delayed_event_queue queue_delayed;
    
    /* 
     * After main partition's thread creates start thread,
     * it is used as idle thread.
     * 
     * This is context pointer for switch to it.
     */
    uint32_t               idle_sp; 
} pok_partition_arinc_t;

#define current_partition_arinc container_of(current_partition, pok_partition_arinc_t, base_part)
#define current_thread (current_partition_arinc->current_thread)

static inline uint8_t pok_partition_get_space(pok_partition_arinc_t* part)
{
   return part->partition_id;
}

// Main thread is always first allocated (Do not change this!).
#define POK_PARTITION_ARINC_MAIN_THREAD_ID 0


/** Initialize arinc partition. */
void pok_partition_arinc_init(pok_partition_arinc_t* part)

/** Reset arinc partition. */
void pok_partition_arinc_reset(pok_partition_arinc_t* part,
	pok_partition_mode_t mode,
	pok_start_condition_t start_condition)

pok_ret_t pok_partition_set_mode_current (const pok_partition_mode_t mode);

pok_ret_t pok_current_partition_get_id (pok_partition_id_t *id);

pok_ret_t pok_current_partition_get_period (uint64_t *period);

pok_ret_t pok_current_partition_get_duration (uint64_t *duration);

pok_ret_t pok_current_partition_get_operating_mode (pok_partition_mode_t *op_mode);

pok_ret_t pok_current_partition_get_lock_level (uint32_t *lock_level);

pok_ret_t pok_current_partition_get_start_condition (pok_start_condition_t *start_condition);

pok_ret_t pok_current_partition_inc_lock_level(uint32_t *lock_level);

pok_ret_t pok_current_partition_dec_lock_level(uint32_t *lock_level);

pok_ret_t pok_thread_create (pok_thread_id_t* thread_id, const pok_thread_attr_t* attr);
pok_ret_t pok_thread_start(pok_thread_id_t thread_id);
pok_ret_t pok_thread_suspend_self(int64_t ms);
pok_ret_t pok_thread_suspend(pok_thread_id_t id);
pok_ret_t pok_thread_stop_self(void);
pok_ret_t pok_thread_stop(pok_thread_id_t thread_id);
pok_ret_t pok_thread_delayed_start (pok_thread_id_t id, int64_t ms);
pok_ret_t pok_thread_get_id_self(pok_thread_id_t* thread_id);
pok_ret_t pok_thread_get_id(const char[MAX_NAME_LENGTH] name, pok_thread_id_t* thread_id);
pok_ret_t pok_thread_get_status(pok_thread_id_t id, pok_thread_status_t *attr);
pok_ret_t pok_thread_set_priority(pok_thread_id_t id, const uint32_t priority);
pok_ret_t pok_thread_resume(pok_thread_id_t id);

/* Find thread by its name. GET_PROCESS_ID in ARINC. */
pok_ret_t pok_thread_find(const char name[MAX_NAME_LENGTH], pok_thread_id_t* id);

// utility macro-like functions


/**
 * Chech that pointer \a ptr is located in the address space of partition
 * \a pid
 */

/* TODO dirty as hell */

#ifdef __i386__
#define POK_CHECK_PTR_IN_PARTITION(pid,ptr) (\
                                             ((uintptr_t)(ptr)) >= pok_partitions[pid].base_addr && \
                                             ((uintptr_t)(ptr)) <  pok_partitions[pid].base_addr + pok_partitions[pid].size\
                                             )

#define POK_CHECK_VPTR_IN_PARTITION(pid,ptr) (\
                                             ((uintptr_t)(ptr)) >= pok_partitions[pid].base_vaddr && \
                                             ((uintptr_t)(ptr)) <  pok_partitions[pid].base_vaddr + pok_partitions[pid].size\
                                             )
#elif defined(__PPC__)
#define POK_CHECK_PTR_IN_PARTITION(pid,ptr) (\
                                             ((uintptr_t)(ptr)) >= 0x80000000 && \
                                             ((uintptr_t)(ptr)) <  0x80000000 + 0x1000000ULL\
                                             )

#define POK_CHECK_VPTR_IN_PARTITION(pid,ptr) (\
                                             ((uintptr_t)(ptr)) >= 0x80000000 && \
                                             ((uintptr_t)(ptr)) <  0x80000000 + 0x1000000ULL\
                                             )
#else
#error "POK_CHECK_PTR macros are not implemented for this arch, do it now!"
#endif

#endif /* !__POK_PARTITION_ARINC_H__ */
