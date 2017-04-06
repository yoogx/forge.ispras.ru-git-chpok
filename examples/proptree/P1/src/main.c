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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arinc653/partition.h>
#include <arinc653/time.h>
#include <proptree.h>
#include <proptree_impl.h>
#include <smalloc.h>

const char strings[] = "\0a\0b\0c1\0c2\0c3\0Hello,world!\0";

jet_pt_node_impl nodes[] = {
	{
		/* ROOT */
		.type = JET_PT_TREE,
		.flags = 0, /* No siblings */
		.name = 0, /* empty name */
		.value_tree = {
		    .n_children = 1,
		    .first_child = 1, /* The only child is 'a'*/
		},
	},
	{
		.type = JET_PT_TREE,
		.flags = 0, /* No siblings */
		.name = 1, /* a */
		.value_tree = {
		    .n_children = 1,
		    .first_child = 2, /* The only child is 'b'*/
		},
	},
	{
		.type = JET_PT_TREE,
		.flags = 0, /* No siblings */
		.name = 3, /* b */
		.value_tree = {
		    .n_children = 3,
		    .first_child = 3, /* First children is 'c1'*/
		},
	},
	{
		.type = JET_PT_INTEGER32,
		.flags = JET_PT_FLAG_HAS_FUTHER_SIBLING, /* Has sibling ('c2') */
		.name = 5, /* c1 */
		.value_int32 = {
		    .value = 100
		},
	},
	{
		.type = JET_PT_INTEGER32,
		.flags = JET_PT_FLAG_HAS_FUTHER_SIBLING, /* Has sibling ('c3') */
		.name = 8, /* c1 */
		.value_int32 = {
		    .value = 200
		},
	},
	{
		.type = JET_PT_STRING,
		.flags = 0, /* No siblings */
		.name = 11, /* c3 */
		.value_string = {
		    .string_offset = 13,  /* Hello, world! */
		},
	},
};

jet_pt_tree_t tree = NULL;

static void first_process(void)
{
	int res;

	printf("Working process!\n");
	jet_pt_node_t node = jet_pt_find(tree, JET_PT_ROOT, "/a/b/c2");
	if (node == JET_PT_INVALID_NODE) {
		printf("Invalid node!\n");
		STOP_SELF();
	}
	printf("name: %s\n", jet_pt_get_node_name(tree, node)); 

	int32_t value;

	res = jet_pt_get_int32(tree, node, NULL, &value);
	if (res) {
		printf("Not an integer value\n");
	} else {
		printf("Value: %d\n", (int)value);
	}

	STOP_SELF();
} 

void main(void) {
    RETURN_CODE_TYPE ret;
    PROCESS_ID_TYPE pid;
    PROCESS_ATTRIBUTE_TYPE process_attrs = {
        .PERIOD = INFINITE_TIME_VALUE,
        .TIME_CAPACITY = INFINITE_TIME_VALUE,
        .STACK_SIZE = 8096,
        .BASE_PRIORITY = MIN_PRIORITY_VALUE,
        .DEADLINE = SOFT,
    };

    // Create tree. Use different name for *writable* tree.
    struct jet_pt_header_struct* tree_raw = smalloc(1000);;
    
    tree_raw->nodes_offset = 6 /*fields in the header*/ * 4 /*size of the field */;
    tree_raw->nodes_count = sizeof(nodes)/sizeof(nodes[0]);
    memcpy((char*)tree_raw + tree_raw->nodes_offset, nodes, sizeof(nodes));
    
    tree_raw->strings_offset = tree_raw->nodes_offset + sizeof(nodes);
    tree_raw->strings_size = sizeof(strings);
    memcpy((char*)tree_raw + tree_raw->strings_offset, strings, sizeof(strings));
    
    // There is no binary data.
    tree_raw->binary_data_offset = 0;
    tree_raw->binary_data_size = 0;
    
    tree = tree_raw;

    // create process 1
    process_attrs.ENTRY_POINT = first_process;
    strncpy(process_attrs.NAME, "process 1", sizeof(PROCESS_NAME_TYPE));

    CREATE_PROCESS(&process_attrs, &pid, &ret);
    if (ret != NO_ERROR) {
        printf("couldn't create process 1: %d\n", (int) ret);
        STOP_SELF() ;
    } else {
        printf("process 1 created\n");
    }
    
    START(pid, &ret);
    if (ret != NO_ERROR) {
        printf("couldn't start process 1: %d\n", (int) ret);
        STOP_SELF() ;
    } else {
        printf("process 1 \"started\" (it won't actually run until operating mode becomes NORMAL)\n");
    }

    // transition to NORMAL operating mode
    // N.B. if everything is OK, this never returns
    SET_PARTITION_MODE(NORMAL, &ret);

    if (ret != NO_ERROR) {
        printf("couldn't transit to normal operating mode: %d\n", (int) ret);
    } 

    STOP_SELF();
}  
