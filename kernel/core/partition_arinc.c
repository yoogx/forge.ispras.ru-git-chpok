/*
 * Institute for System Programming of the Russian Academy of Sciences
 * Copyright (C) 2016 ISPRAS
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, Version 3.
 *
 * This program is distributed in the hope # that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License version 3 for more details.
 */

#include <config.h>

#include <core/partition_arinc.h>
#include <core/sched_arinc.h>
#include "thread_internal.h"
#include <common.h>
#include <asp/arch.h>
#include <core/uaccess.h>
#include <system_limits.h>

#include <cswitch.h>
#include <core/loader.h>
#include <alloc.h>

#include <core/memblocks.h>

/*
 * Function which is executed in kernel-only partition's context when
 * partition's mode is IDLE.
 *
 * Note, that we do not enable local preemption here.
 * This has nice effect in case when partition has moved into this mode
 * because of errors: even if some partition's data are corrupted,
 * idle has high chance to work.
 */
static void idle_func(void)
{
    ja_inf_loop();
}

/*
 * Terminates all (server) portals for given partition.
 *
 * Should be executed with local preemption disabled.
 */
static void partition_arinc_portals_terminate(enum jet_ippc_portal_state portal_state)
{
    pok_partition_arinc_t* part = current_partition_arinc;

    for(int i = 0; i < part->portal_types_n; i++) {
        struct jet_partition_arinc_ippc_portal_type* arinc_portal_type = &part->portal_types[i];
        for(int j = 0; j < arinc_portal_type->portal_type->n_portals; j++) {
            struct jet_ippc_portal* portal = &arinc_portal_type->portal_type->portals[j];
            jet_ippc_portal_terminate(portal, portal_state);
        }
    }
}

/*
 * Cancels all active (client) connections for given partition.
 *
 * Should be executed with local preemption disabled.
 */
static void partition_arinc_connections_cancel(void)
{
    pok_partition_arinc_t* part = current_partition_arinc;

    for(int i = 0; i < part->nthreads; i++) {
        pok_thread_t* t = &part->threads[i];

        if(t->ippc_connection) {
            jet_ippc_connection_cancel(t->ippc_connection);
        }
    }
}

void pok_partition_arinc_idle(void)
{
    pok_partition_arinc_t* part = current_partition_arinc;

    // Unconditionally off preemption.
    part->base_part.preempt_local_disabled = 1;

    partition_arinc_portals_terminate(JET_IPPC_PORTAL_STATE_UNUSABLE);
    partition_arinc_connections_cancel();

    jet_context_restart(part->base_part.initial_sp, &idle_func);
}

/* Helpers */

/*
 * Reset thread object as it is not used.
 */
static void thread_reset(pok_thread_t* t)
{
    t->name[0] = '\0';

    if(t->ippc_connection) {
        pok_partition_arinc_t* part = current_partition_arinc;

        // Connection is already cancelled. Need to wait it until terminate.
        while(jet_ippc_connection_get_state(t->ippc_connection) != JET_IPPC_CONNECTION_STATE_TERMINATED)
        {
            /*
             * Because local preemption is disabled, we cannot react
             * on outer events with 'on_event()' handler. But we should
             * react on these events, otherwise executing IPPC connection
             * returns immediately without a progress.
             *
             * The only "event" possible in initialization state is
             * changing 'part->ippc_handled' field. So it is sufficient
             * to set 'ippc_handled_actual' field for mark this event as handled.
             */
             part->base_part.ippc_handled_actual = ACCESS_ONCE(part->base_part.ippc_handled);

             jet_ippc_connection_execute(t->ippc_connection,
                part->base_part.ippc_handled_actual);
        }

        jet_ippc_connection_close(t->ippc_connection);
        t->ippc_connection = NULL;
    }

    t->ippc_server_connection = NULL;
}


/*
 * Initialize thread object.
 *
 * Executed once during partition initialization.
 */
static void thread_init(pok_thread_t* t)
{
    t->initial_sp = pok_stack_alloc(KERNEL_STACK_SIZE_DEFAULT);
    t->fp_store = ja_alloc_fp_store();
}

// This name is not accessible for user space
static char main_thread_name[MAX_NAME_LENGTH] = "main";

/* Start function for partition. */
static void partition_arinc_start(void)
{
    pok_partition_arinc_t* part = current_partition_arinc;

    if(part->base_part.restarted_externally)
    {
        part->base_part.restarted_externally = FALSE;
        part->mode = POK_PARTITION_MODE_INIT_COLD;
    }

    // Initialize memory blocks. ELF is loaded as a part of this process.
    for( int i = 0; i < part->memory_block_init_entries_n; i++)
    {
        const struct jet_partition_memory_block_init_entry* part_init_entry =
            &part->memory_block_init_entries[i];
        jet_memory_block_init(part_init_entry->init_type,
            part,
            part_init_entry->mblocks,
            part_init_entry->source_id
        );
    }

    // Setup kernel shared data
    const struct memory_block* mblock_kshd = jet_partition_arinc_find_memory_block(
        part, ".KSHD");

    assert(mblock_kshd);

    part->kshd = (void* __kuser)mblock_kshd->kaddr;

    /* Setup memory for stacks. */
    const struct memory_block* mblock_stacks = jet_partition_arinc_find_memory_block(
        part, ".STACKS");

    assert(mblock_stacks);

    // Note: We store virtual (user) address.
    part->stacks_start = mblock_stacks->vaddr;
    part->stacks_end = part->stacks_start + mblock_stacks->size;

    part->stacks_current = part->stacks_start;

    jet_uspace_revoke_access_local(&part->base_part);

    for(unsigned i = 0; i < part->nports_queuing; i++)
    {
        pok_port_queuing_init(&part->ports_queuing[i]);
    }

    for(unsigned i = 0; i < part->nports_sampling; i++)
    {
        pok_port_sampling_init(&part->ports_sampling[i]);
    }

    INIT_LIST_HEAD(&part->eligible_threads);
    delayed_event_queue_init(&part->partition_delayed_events);

    for(int i = 0; i < part->nthreads; i++)
    {
        thread_reset(&part->threads[i]);
    }

    part->nthreads_normal_used = 0;

    part->thread_current = NULL;

    part->thread_error = NULL;
    INIT_LIST_HEAD(&part->error_list);

    pok_thread_t* thread_main = &part->threads[POK_PARTITION_ARINC_MAIN_THREAD_ID];

    thread_main->entry = (void* __user)part->main_entry;
    thread_main->base_priority = 0;
    thread_main->period = POK_TIME_INFINITY;
    thread_main->time_capacity = POK_TIME_INFINITY;
    thread_main->deadline = DEADLINE_SOFT;
    strncpy(thread_main->name, main_thread_name, MAX_NAME_LENGTH);
    thread_main->user_stack_size = part->main_user_stack_size;

    if(!thread_create(thread_main)) unreachable(); // Configurator should check stack size for main thread.

    part->nthreads_used = POK_PARTITION_ARINC_MAIN_THREAD_ID + 1;

    // Fill initial kernel shared data.
    part->kshd->current_thread_id = JET_THREAD_ID_NONE;
    part->kshd->max_n_threads = part->nthreads;
    part->kshd->partition_mode = part->mode;
    // Transfer data about intra communication to user space
    part->kshd->arinc_config_nbuffers = part->arinc_config_nbuffers;
    part->kshd->arinc_config_nblackboards = part->arinc_config_nblackboards;
    part->kshd->arinc_config_nsemaphores = part->arinc_config_nsemaphores;
    part->kshd->arinc_config_nevents = part->arinc_config_nevents;
    part->kshd->arinc_config_messages_memory_size = part->arinc_config_messages_memory_size;

    sched_arinc_start();

    /* Current context is lost and may be reused for "do_nothing" thread
     * or for IDLE partition's mode.
     */
}

void pok_partition_arinc_reset(pok_partition_mode_t mode)
{
    pok_partition_arinc_t* part = current_partition_arinc;

    part->base_part.preempt_local_disabled = 1;
    jet_uspace_reset_access_local(&part->base_part);

    assert(mode == POK_PARTITION_MODE_INIT_WARM
        || mode == POK_PARTITION_MODE_INIT_COLD);

    assert(mode == POK_PARTITION_MODE_INIT_COLD ||
        part->mode != POK_PARTITION_MODE_INIT_COLD);

    part->mode = mode;

    partition_arinc_portals_terminate(JET_IPPC_PORTAL_STATE_INITIALIZING);
    partition_arinc_connections_cancel();

    pok_partition_restart();
}

#if POK_NEEDS_GDB
static int pok_sched_arinc_get_number_of_threads(pok_partition_t* part)
{
    pok_partition_arinc_t* part_arinc = container_of(part,
        typeof(*part_arinc), base_part);

    return part_arinc->nthreads + 1; //IDLE thread
}

static int pok_sched_arinc_get_current_thread_index(pok_partition_t* part)
{
    pok_partition_arinc_t* part_arinc = container_of(part,
        typeof(*part_arinc), base_part);

    return part_arinc->thread_current
        ? part_arinc->thread_current - part_arinc->threads
        : part_arinc->nthreads; //IDLE thread
}

static int pok_sched_arinc_get_thread_at_index(pok_partition_t* part,
    int index, void** private)
{
    pok_partition_arinc_t* part_arinc = container_of(part,
        typeof(*part_arinc), base_part);
    if(index >part_arinc->nthreads) return 1;

    if(index == part_arinc->nthreads)
    {
        *private = NULL;
    }
    else
    {
        *private = part_arinc->threads + index;
    }

    return 0;
}

static void pok_sched_arinc_get_thread_info(pok_partition_t* part, int index, void* private,
    print_cb_t print_cb, void* cb_data)
{
// Write given string (null-terminated)
#define WRITE_STR(s) print_cb(s, strlen(s), cb_data)
    pok_thread_t* t = private;
    pok_partition_arinc_t* part_arinc = container_of(part, typeof(*part_arinc), base_part);
    if(!t)
    {
        WRITE_STR("IDLE");
    }
    else if(index == POK_PARTITION_ARINC_MAIN_THREAD_ID)
    {
        // Main thread. Currently do not bother with its state
        WRITE_STR(t->name);
    }
    else if(index < part_arinc->nthreads_used)
    {
        WRITE_STR(t->name);
        // Write state of the thread
        WRITE_STR(" ");
        switch(t->state)
        {
        case POK_STATE_STOPPED:
            WRITE_STR("Stopped");
        break;
        case POK_STATE_WAITING:
            WRITE_STR("Waiting"); // Waiting for anything except resume.
        break;
        case POK_STATE_RUNNABLE:
            if(t->suspended)
                WRITE_STR("Suspended");
            else if(part_arinc->thread_current == t)
                WRITE_STR("Running");
            else
                WRITE_STR("Ready");
        break;
        default:
            unreachable();
        }
    }
    else
    {
        WRITE_STR("[Not created]");
    }
#undef WRITE_STR
}

static struct jet_interrupt_context* pok_sched_arinc_get_thread_registers(pok_partition_t* part,
    int index, void* private)
{
    pok_thread_t* t = private;
    pok_partition_arinc_t* part_arinc = container_of(part, typeof(*part_arinc), base_part);

    if(!t)
    {
        // Idle thread
        return NULL;
    }
    else if(index < part_arinc->nthreads_used && t->entry_sp_user)
    {
        return t->entry_sp_user;
    }
    else
    {
        // Not created or user space hasn't been called yet.
        return NULL;
    }
}

static void* __kuser pok_sched_arinc_uaddr_to_gdb(
    struct _pok_partition* part, const void* __user addr, size_t size)
{
    pok_partition_arinc_t* part_arinc = container_of(part, typeof(*part_arinc), base_part);

    const struct memory_block* mblock = jet_partition_arinc_get_memory_block_for_addr(
        part_arinc, addr, size);

    if(mblock) return jet_memory_block_get_kaddr(mblock, addr);

    return NULL;
}

#endif /* POK_NEEDS_GDB */

static const struct pok_partition_sched_operations arinc_sched_ops = {
    .on_event = &pok_sched_arinc_on_event,
#if POK_NEEDS_GDB
    .get_number_of_threads = &pok_sched_arinc_get_number_of_threads,
    .get_current_thread_index = &pok_sched_arinc_get_current_thread_index,
    .get_thread_at_index = &pok_sched_arinc_get_thread_at_index,
    .get_thread_info = &pok_sched_arinc_get_thread_info,
    .get_thread_registers = &pok_sched_arinc_get_thread_registers,
    .uaddr_to_gdb = &pok_sched_arinc_uaddr_to_gdb,
#endif /* POK_NEEDS_GDB */
};

static const struct pok_partition_operations arinc_ops = {
    .start = &partition_arinc_start,
    .process_partition_error = &pok_partition_arinc_process_error,
};

void pok_partition_arinc_init(pok_partition_arinc_t* part)
{
    pok_partition_init(&part->base_part);

    part->base_part.initial_sp = pok_stack_alloc(DEFAULT_STACK_SIZE);

    for(int i = 0; i < part->nthreads; i++)
    {
        thread_init(&part->threads[i]);
    }

    part->base_part.part_ops = &arinc_ops;
    part->base_part.part_sched_ops = &arinc_sched_ops;

    for(int i = 0; i < part->portal_types_n; i++) {
        struct jet_ippc_portal_type* portal_type = part->portal_types[i].portal_type;

        for(int j = 0; j < portal_type->n_portals; j++) {
            struct jet_ippc_portal* portal = &portal_type->portals[j];

            jet_ippc_portal_create(portal, 0);
            portal->init_connection->server_handler_is_shared = TRUE;
        }
    }
}

/*
 * Transition from INIT_* mode to NORMAL.
 *
 * Executed with local preemption disabled.
 */
static void partition_set_mode_normal(void)
{
    pok_partition_arinc_t* part = current_partition_arinc;

    part->mode = POK_PARTITION_MODE_NORMAL;
    part->lock_level = 0;
    pok_sched_local_invalidate();

    for(int i = POK_PARTITION_ARINC_MAIN_THREAD_ID + 1; i < part->nthreads_used; i++)
    {
        pok_thread_t* t = &part->threads[i];

        if(t->state == POK_STATE_STOPPED) continue;

        /*
         * The only thing thread can wait in `INIT_*` mode is
         * NORMAL mode switch.
         */

        thread_start_normal(t, t->delayed_time);
    }

    // Mark portals with fully initialized connections as READY.
    for(int i = 0; i < part->portal_types_n; i++) {
        struct jet_partition_arinc_ippc_portal_type* arinc_portal_type = &part->portal_types[i];

        for(int j = 0; j < arinc_portal_type->portal_type->n_portals; j++) {
            struct jet_ippc_portal* portal = &arinc_portal_type->portal_type->portals[j];

            jet_ippc_portal_finish_initialization(portal,
                portal->n_connections_ready == portal->n_connections);
        }
    }

}

// Executed with local preemption disabled.
static pok_ret_t partition_set_mode_internal (const pok_partition_mode_t mode)
{
    pok_partition_arinc_t* part = current_partition_arinc;

    switch(mode)
    {
    case POK_PARTITION_MODE_INIT_WARM:
        if(part->mode == POK_PARTITION_MODE_INIT_COLD)
            return POK_ERRNO_PARTITION_MODE;
    // Walkthrough
    case POK_PARTITION_MODE_INIT_COLD:
        part->base_part.start_condition = POK_START_CONDITION_PARTITION_RESTART;
        pok_partition_arinc_reset(mode); // Never return.
    break;
    case POK_PARTITION_MODE_IDLE:
        pok_partition_arinc_idle(); // Never return.
    break;
    case POK_PARTITION_MODE_NORMAL:
        if(part->mode == POK_PARTITION_MODE_NORMAL)
            return POK_ERRNO_UNAVAILABLE; //TODO: revise error code
        partition_set_mode_normal();
    break;
    default:
        return POK_ERRNO_EINVAL;
    }

    // Update kernel shared data
    part->kshd->partition_mode = part->mode;

    return POK_ERRNO_OK;
}

pok_ret_t pok_partition_set_mode_current (const pok_partition_mode_t mode)
{
    pok_ret_t res;

    pok_preemption_local_disable();
    res = partition_set_mode_internal(mode);
    pok_preemption_local_enable();

    return res;
}

/**
 * Get partition information. Used for ARINC GET_PARTITION_STATUS function.
 */
pok_ret_t pok_current_partition_get_status(pok_partition_status_t * __user status)
{
    pok_partition_status_t* __kuser k_status =
        jet_user_to_kernel_typed(status);
    if(!k_status) return POK_ERRNO_EFAULT;

    k_status->id = current_partition->partition_id;
    k_status->period = current_partition->period;
    k_status->duration = current_partition->duration;
    k_status->mode = current_partition_arinc->mode;
    k_status->lock_level = current_partition_arinc->lock_level;
    k_status->start_condition = current_partition->start_condition;

    return POK_ERRNO_OK;
}

/*
 * Whether lock level cannot be changed now.
 *
 * NOTE: Doesn't require disabled local preemption.
 */
static pok_bool_t is_lock_level_blocked(void)
{
    pok_partition_arinc_t* part = current_partition_arinc;
    return part->mode != POK_PARTITION_MODE_NORMAL ||
      part->thread_current == part->thread_error;
}

pok_ret_t pok_current_partition_inc_lock_level(int32_t * __user lock_level)
{
    pok_partition_arinc_t* part = current_partition_arinc;

    if(is_lock_level_blocked())
        return POK_ERRNO_PARTITION_MODE;
    if(part->lock_level == MAX_LOCK_LEVEL)
        return POK_ERRNO_EINVAL;

    uint32_t* __kuser k_lock_level = jet_user_to_kernel_typed(lock_level);
    if(!k_lock_level) return POK_ERRNO_EFAULT;

    pok_preemption_local_disable();
    part->lock_level++;
    part->thread_locked = part->thread_current;
    // Note: this doesn't invalidate any scheduling event.
    pok_preemption_local_enable();

    *k_lock_level = current_partition_arinc->lock_level;

    return POK_ERRNO_OK;
}

pok_ret_t pok_current_partition_dec_lock_level(int32_t * __user lock_level)
{
    if(is_lock_level_blocked())
        return POK_ERRNO_PARTITION_MODE;
    if(current_partition_arinc->lock_level == 0)
        return POK_ERRNO_EINVAL;

    uint32_t* __kuser k_lock_level = jet_user_to_kernel_typed(lock_level);
    if(!k_lock_level) return POK_ERRNO_EFAULT;

    pok_preemption_local_disable();
    if(--current_partition_arinc->lock_level == 0)
    {
        if(current_thread->eligible_elem.prev != &current_partition_arinc->eligible_threads)
        {
            // We are not the first thread in eligible queue
            pok_sched_local_invalidate();
        }
    }
    pok_preemption_local_enable();

    *k_lock_level = current_partition_arinc->lock_level;

    return POK_ERRNO_OK;
}


void pok_partition_arinc_init_all(void)
{
    for(int i = 0; i < pok_partitions_arinc_n; i++) {
        pok_partition_arinc_init(&pok_partitions_arinc[i]);
    }
}

const struct memory_block* jet_partition_arinc_find_memory_block(
    pok_partition_arinc_t* part, const char* name)
{
    for(int i = 0; i < part->nmemory_blocks; i++)
    {
        const struct memory_block* mblock = &part->memory_blocks[i];

        if(strcmp(mblock->name, name) == 0) return mblock;
    }

    return NULL;
}

const struct memory_block* jet_partition_arinc_get_memory_block_for_addr(
    pok_partition_arinc_t* part,
    const void* __user addr,
    size_t size)
{
    uintptr_t vaddr = (uintptr_t)addr;

    for(int i = 0; i < part->nmemory_blocks; i++)
    {
        const struct jet_partition_arinc_mb_addr_entry* mb_addr_entry = &part->mb_addr_table[i];

        if(mb_addr_entry->vaddr > vaddr) break;
        if(mb_addr_entry->vaddr + mb_addr_entry->size > vaddr) {
            // Maximum size available to the end of the block.
            uint64_t rest_size = mb_addr_entry->vaddr + mb_addr_entry->size - vaddr;

            // Check that end of range is not after the end of the block.
            if(size > rest_size) return NULL;

            return mb_addr_entry->mblock;
        }

    }

    return NULL;
}
