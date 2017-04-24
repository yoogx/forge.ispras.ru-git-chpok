#include <stdio.h>
#include <string.h>
#include <arinc653/partition.h>
#include <arinc653/time.h>

#include <ippc.h>
#include <kernel_shared_data.h>

static void server_handler(void)
{
    static int counter = 0;

    struct jet_thread_shared_data* tshd = &kshd->tshd[kshd->current_thread_id];

    // Setup output parameters
    tshd->ippc_output_params_server_n = 1;
    tshd->ippc_output_params_server[0] = ++counter;

    jet_ippc_partition_arinc_return();
}

static int real_main(void)
{
    RETURN_CODE_TYPE ret;
    jet_ret_t ret_internal;

    // Extract identificator for portal type and number of client partitions.
    int portal_type_id;
    int n_clients;
    ret_internal = jet_ippc_partition_arinc_get_portal_type_info("Test", &portal_type_id, &n_clients);

    if(ret_internal != EOK) {
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
