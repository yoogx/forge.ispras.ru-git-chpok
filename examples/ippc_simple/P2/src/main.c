#include <stdio.h>
#include <string.h>
//#include <arinc653/buffer.h>
#include <arinc653/partition.h>
#include <arinc653/time.h>
#include <arinc653/queueing.h>
#include <arinc653/sampling.h>

#include <ippc.h>
#include <kernel_shared_data.h>

#define SECOND 1000000000LL

/* 
 * Identificator of the IPPC portal.
 * 
 * Obtained in main, used in the process.
 */
int portal_id;

static void first_process(void)
{
    RETURN_CODE_TYPE ret;
    pok_ret_t ret_internal;
    
    struct jet_thread_shared_data* tshd = &kshd->tshd[kshd->current_thread_id];
    
    while (1) {
        printf("PR2: Executing IPPC ...\n");

        ret_internal = jet_ippc_partition_arinc_call(portal_id);
        if(ret_internal) {
            printf("IPPC call failed: %d\n", ret);
            STOP_SELF();
        }

        if(tshd->ippc_output_params_n != 1) {
            printf("Unexpected number of output parameters of IPPC call: %d\n", tshd->ippc_output_params_n);
            STOP_SELF();
        }
        
        printf("IPPC return value: %d\n", (int)tshd->ippc_output_params[0]);
        
        TIMED_WAIT(SECOND, &ret);
    }
}

static int real_main(void)
{
    RETURN_CODE_TYPE ret;
    PROCESS_ID_TYPE pid;
    PROCESS_ATTRIBUTE_TYPE process_attrs = {
        .PERIOD = INFINITE_TIME_VALUE,
        .TIME_CAPACITY = INFINITE_TIME_VALUE,
        .STACK_SIZE = 8096, // the only accepted stack size!
        .BASE_PRIORITY = MIN_PRIORITY_VALUE,
        .DEADLINE = SOFT,
    };

    ret = jet_ippc_partition_arinc_init_portal("Test", &portal_id);
    if(ret) {
        printf("Failed to init portal:%d\n", ret);
        STOP_SELF();
    }
    
    // create process 1
    process_attrs.ENTRY_POINT = first_process;
    strncpy(process_attrs.NAME, "process 1", sizeof(PROCESS_NAME_TYPE));

    CREATE_PROCESS(&process_attrs, &pid, &ret);
    if (ret != NO_ERROR) {
        printf("couldn't create process 1: %d\n", (int) ret);
        return 1;
    }
    
    START(pid, &ret);
    if (ret != NO_ERROR) {
        printf("couldn't start process 1: %d\n", (int) ret);
        return 1;
    } else {
        printf("process 1 \"started\" (it won't actually run until operating mode becomes NORMAL)\n");
    }


    // transition to NORMAL operating mode
    // N.B. if everything is OK, this never returns
    printf("going to NORMAL mode...\n");
    SET_PARTITION_MODE(NORMAL, &ret);

    if (ret != NO_ERROR) {
        printf("couldn't transit to normal operating mode: %d\n", (int) ret);
    }

    STOP_SELF();
    return 0;
}

void main(void) {
    real_main();
    STOP_SELF();
}
