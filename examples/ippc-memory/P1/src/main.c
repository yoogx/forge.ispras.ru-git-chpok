#include <stdio.h>
#include <string.h>
#include <arinc653/partition.h>
#include <arinc653/time.h>

#include <ippc.h>
#include <kernel_shared_data.h>

/*
 * Remote call 1.
 *
 * Take integer pointed by 'value', add 'addition' to it and return result.
 */
static uint32_t add_value_by_ref(const uint32_t* value, uint32_t addition)
{
    uint32_t value_my = 0;
    
    pok_ret_t ret_internal;
    
    ret_internal = jet_ippc_partition_arinc_copy_from_client(&value_my, value, sizeof(*value));
    
    if(ret_internal) {
        printf("copy_from_client() returns %d\n", (int)ret_internal);
    }
    
    return value_my + addition;
}

/*
 * Remote call 2.
 *
 * Increment variable pointed by 'value', by 'addition'.
 */
static void inc_value(uint32_t* value, uint32_t addition)
{
    uint32_t value_my = 0;
    
    pok_ret_t ret_internal;
    
    ret_internal = jet_ippc_partition_arinc_copy_from_client(&value_my, value, sizeof(*value));
    
    if(ret_internal) {
        printf("copy_from_client() returns %d\n", (int)ret_internal);
        return;
    }
    
    value_my += addition;

    ret_internal = jet_ippc_partition_arinc_copy_to_client(value, &value_my, sizeof(*value));
    
    if(ret_internal) {
        printf("copy_to_client() returns %d\n", (int)ret_internal);
        return;
    }
}


static void server_handler(void)
{
    struct jet_thread_shared_data* tshd = &kshd->tshd[kshd->current_thread_id];

    int id = tshd->ippc_input_params_server[0];
    switch(id) {
        case 1:
        {
            uint32_t ret = add_value_by_ref(
                (const uint32_t*)tshd->ippc_input_params_server[1],
                (uint32_t)tshd->ippc_input_params_server[2]);
            
            // Setup output parameters
            tshd->ippc_output_params_server[0] = ret;
            tshd->ippc_output_params_server_n = 1;
        }
        break;
        case 2:
        {
            inc_value(
                (uint32_t*)tshd->ippc_input_params_server[1],
                (uint32_t)tshd->ippc_input_params_server[2]);
        }
        break;
        default:
            printf("Invalid id: %d\n", id);
    }

    jet_ippc_partition_arinc_return();
}

static int real_main(void)
{
    RETURN_CODE_TYPE ret;
    pok_ret_t ret_internal;

    // Extract identificator for portal type and number of client partitions.
    int portal_type_id;
    int n_clients;
    ret_internal = jet_ippc_partition_arinc_get_portal_type_info("Test", &portal_type_id, &n_clients);

    if(ret_internal != POK_ERRNO_OK) {
        printf("Failed to get information about portal type: %d", ret_internal);
        STOP_SELF();
    }

    // Create connections for each client.
    for(int i = 0; i < n_clients; i++) {
        // This is how identificator of the portal is obtained from identificator of the portal type.
        int portal_id = (portal_type_id << 16) + i;
        int n_connections;

        ret_internal = jet_ippc_partition_arinc_get_portal_info(portal_id, &n_connections);
        if(ret_internal) {
            printf("Failed to get information about portal %d: %d", portal_id, ret_internal);
            STOP_SELF();
        }

        pok_thread_id_t server_thread_id;

        ret_internal = jet_ippc_partition_arinc_create_connections(portal_id,
            &server_handler, 4096, n_connections, &server_thread_id);

        if(ret_internal) {
            printf("Failed to get create connections for portal %d: %d", portal_id, ret_internal);
            STOP_SELF();
        }
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
