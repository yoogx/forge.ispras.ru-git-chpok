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

// Wrapper around IPPC call 1.
uint32_t add_value_by_ref(const uint32_t *value, uint32_t addition)
{
    pok_ret_t ret_internal;

    struct jet_thread_shared_data* tshd = &kshd->tshd[kshd->current_thread_id];

    tshd->ippc_input_params[0] = 1; // Id
    tshd->ippc_input_params[1] = (uintptr_t)value;
    tshd->ippc_input_params[2] = (uintptr_t)addition;
    
    tshd->ippc_input_params_n = 3;
    
    struct jet_ippc_client_access_window client_access_window = {
        .start = value,
        .size = sizeof(uint32_t),
        .is_writable = FALSE
    };

    ret_internal = jet_ippc_partition_arinc_call(portal_id, &client_access_window, 1);
    if(ret_internal) {
        printf("IPPC call 1 failed: %d\n", ret_internal);
        STOP_SELF();
    }

    if(tshd->ippc_output_params_n != 1) {
        printf("Unexpected number of output parameters of IPPC call 1: %d\n", tshd->ippc_output_params_n);
        STOP_SELF();
    }

    return (uint32_t)tshd->ippc_output_params[0];
}

// Wrapper around IPPC call 1.
void inc_value(uint32_t *value, uint32_t addition)
{
    pok_ret_t ret_internal;

    struct jet_thread_shared_data* tshd = &kshd->tshd[kshd->current_thread_id];

    tshd->ippc_input_params[0] = 2; // Id
    tshd->ippc_input_params[1] = (uintptr_t)value;
    tshd->ippc_input_params[2] = (uintptr_t)addition;
    
    tshd->ippc_input_params_n = 3;
    
    struct jet_ippc_client_access_window client_access_window = {
        .start = value,
        .size = sizeof(uint32_t),
        .is_writable = TRUE
    };

    ret_internal = jet_ippc_partition_arinc_call(portal_id, &client_access_window, 1);
    if(ret_internal) {
        printf("IPPC call 2 failed: %d\n", ret_internal);
        STOP_SELF();
    }

    if(tshd->ippc_output_params_n != 0) {
        printf("Unexpected number of output parameters of IPPC call 2: %d\n", tshd->ippc_output_params_n);
        STOP_SELF();
    }
}


static void first_process(void)
{
    RETURN_CODE_TYPE ret;

    uint32_t value1 = 1;
    uint32_t value2 = 1;
    
    while (1) {
        printf("PR2: Executing IPPC 1 ...\n");

        uint32_t res = add_value_by_ref(&value1, 30);
        
        printf("IPPC 1 return value: %d\n", (int)res);
        
        value1 = res;


        printf("PR2: Executing IPPC 2.\n");

        inc_value(&value2, 30);
        
        printf("Value after IPPC 2: %d\n", (int)value2);
        
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
