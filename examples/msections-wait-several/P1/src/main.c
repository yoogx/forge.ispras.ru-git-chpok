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

#include <stdio.h>
#include <string.h>
#include <arinc653/buffer.h>
#include <arinc653/partition.h>
#include <arinc653/time.h>
#include <kernel_shared_data.h>

#include <msection.h>

static int completion = 0;

static struct msection section;
static struct msection_wq waitqueue;

volatile int some_work;

static void notify_process(void)
{
    printf("Notify process starts.\n");
    msection_enter(&section);
    
    // Do some work
    for(some_work = 0; some_work < 10000000; some_work++);
    
    printf("Work is done.\n");
    
    completion = 1;
    
    if(msection_wq_notify(&section, &waitqueue, TRUE) == POK_ERRNO_OK)
    {
        pok_thread_id_t t = waitqueue.first;
        
        printf("Notify processes:\n");
        
        do {
            pok_thread_id_t t_next = kshd->tshd[t].wq_next;
            
            printf("- %d\n", t + 1); // Thread ID -> PROCESS_ID
            
            msection_wq_del(&waitqueue, t);
            
            t = t_next;
            
        } while(t != JET_THREAD_ID_NONE);
    }
    else
    {
        printf ("Nothing to notify\n");
    }
    
    msection_leave(&section);
    printf("Notify process is finishing.\n");

    STOP_SELF();
}

static void wait_process_common(SYSTEM_TIME_TYPE timeout)
{
    pok_thread_id_t t = kshd->current_thread_id;
    
    printf("Wait process %d starts.\n", t + 1);

    msection_enter(&section);
    if(completion == 0)
    {
        msection_wq_add(&waitqueue, JET_THREAD_ID_NONE);
        
        printf ("Wait process %d waits on waitqueue ...\n", t + 1);
        
        switch(msection_wait(&section, timeout >= 0 ? timeout / 1000000 : INFINITE_TIME_VALUE))
        {
        case POK_ERRNO_OK:
            printf ("Wait process %d resumes because has been notified.\n", t + 1);
            break;
        case POK_ERRNO_CANCELLED:
            printf ("Wait process %d resumes because has been killed.\n", t + 1);
            msection_wq_del(&waitqueue, t);
            break;
        case POK_ERRNO_TIMEOUT:
            printf ("Wait process %d resumes because timeout.\n", t + 1);
            msection_wq_del(&waitqueue, t);
            break;
        default:
            // unreachable
            break;
        }
    }
    msection_leave(&section);
    
    printf("Wait process %d is finishing.\n", t + 1);

    STOP_SELF();
}

static void wait_process(void)
{
    wait_process_common(INFINITE_TIME_VALUE);
}

static void wait_process_timeout(void)
{
    wait_process_common(5000000 /* 5ms*/);
}

PROCESS_ID_TYPE killing_process_id;

static void killer_process(void)
{
    RETURN_CODE_TYPE ret;
    
    printf("Killer process starts.\n");
    
    // Killer process should preempt notifier process while it "does some work".
    // Adjust time if needed.
    TIMED_WAIT(2000000 /*2 ms */, &ret);
    
    if(ret != NO_ERROR)
    {
        printf("TIMED_WAIT has been failed with error %d\n", (int)ret);
    }
    
    printf("Killing process %d\n", (int) killing_process_id);
    
    STOP(killing_process_id, &ret);
    
    printf("Killer process finishing.\n");
    
    STOP_SELF();
}


static PROCESS_ID_TYPE run_process_common(const char* process_name,
    void (*entry)(void),
    PRIORITY_TYPE prio)
{
    RETURN_CODE_TYPE ret;
    PROCESS_ID_TYPE pid;
    PROCESS_ATTRIBUTE_TYPE process_attrs = {
        .PERIOD = INFINITE_TIME_VALUE,
        .ENTRY_POINT = entry,
        .TIME_CAPACITY = INFINITE_TIME_VALUE,
        .STACK_SIZE = 8096, // the only accepted stack size!
        .BASE_PRIORITY = prio,
        .DEADLINE = SOFT,
    };

    strncpy(process_attrs.NAME, process_name, sizeof(PROCESS_NAME_TYPE));

    CREATE_PROCESS(&process_attrs, &pid, &ret);
    if (ret != NO_ERROR) {
        printf("couldn't create process '%s': %d\n", process_name, (int) ret);
        STOP_SELF();
    } else {
        printf("process '%s' created (id %d)\n", process_name, (int) pid);
    }
    
    START(pid, &ret);
    if (ret != NO_ERROR) {
        printf("couldn't start process '%s': %d\n", process_name, (int) ret);
        STOP_SELF();
    } else {
        printf("process '%s' \"started\" (it won't actually run until operating mode becomes NORMAL)\n", process_name);
    }
    
    return pid;
}

void main(void)
{
    msection_init(&section);
    msection_wq_init(&waitqueue);

    run_process_common("Notifier", notify_process, MIN_PRIORITY_VALUE);
    run_process_common("First waiter", wait_process, MIN_PRIORITY_VALUE + 4);
    run_process_common("Second waiter", wait_process, MIN_PRIORITY_VALUE + 3);
    run_process_common("Timeouted waiter", wait_process_timeout, MIN_PRIORITY_VALUE + 2);

    killing_process_id
        = run_process_common("Killed waiter", wait_process, MIN_PRIORITY_VALUE + 1);

    run_process_common("Killer process", killer_process, MIN_PRIORITY_VALUE + 1);

    // transition to NORMAL operating mode
    
    RETURN_CODE_TYPE ret;
    SET_PARTITION_MODE(NORMAL, &ret);

    printf("couldn't transit to normal operating mode: %d\n", (int) ret);

    STOP_SELF();
}
