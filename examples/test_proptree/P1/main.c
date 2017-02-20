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

const char strings[] = "\0a\0b\0c1\0c2\0c3\0Hello,world!\0";

int integers[] = {100,200};

jet_pt_node_impl nodes[] = {
	{
		/* ROOT */
		JET_PT_TREE,
		0, /* empty name */
		JET_PT_INVALID_OFFSET, /* no sibling */
		JET_PT_INVALID_OFFSET /* no value */
	},
	{
		JET_PT_TREE,
		1, /* a */
		JET_PT_INVALID_OFFSET, /* no sibling */
		JET_PT_INVALID_OFFSET /* no value */
	},
	{
		JET_PT_TREE,
		3, /* b */
		JET_PT_INVALID_OFFSET, /* no sibling */
		JET_PT_INVALID_OFFSET /* no value */
	},
	{
		JET_PT_INTEGER,
		5, /* c1 */
		1, /* c2 */
		0  /* 100 */
	},
	{
		JET_PT_INTEGER,
		8, /* c2 */
		1, /* c3 */
		1  /* 200 */
	},
	{
		JET_PT_STRING,
		11, /* c3 */
		JET_PT_INVALID_OFFSET, /* no sibling */
		13  /* Hello, world! */
	},
};

jet_pt_tree_t tree = NULL;

static void first_process(void)
{
	printf("Working process!\n");
	jet_pt_node_t node = jet_pt_find(tree, JET_PT_ROOT, "/a/b/c2");
	if (node == JET_PT_INVALID_NODE) {
		printf("Invalid node!\n");
		return;
	}
	printf("name: %s\n", jet_pt_get_node_name(tree, node)); 
	int value;
	int ret = jet_pt_get_int32(tree, node, NULL, &value);
	if (ret != 0) {
		printf("Not an integer value\n");
	} else {
		printf("Value: %d\n", value);
	}
	
	return;
} 

void main(void) {
    RETURN_CODE_TYPE ret;
    PROCESS_ID_TYPE pid;
    PROCESS_ATTRIBUTE_TYPE process_attrs = {
        .PERIOD = INFINITE_TIME_VALUE,
        .TIME_CAPACITY = INFINITE_TIME_VALUE,
        .STACK_SIZE = 8096, // the only accepted stack size!
        .BASE_PRIORITY = MIN_PRIORITY_VALUE,
        .DEADLINE = SOFT,
    };

	// Create tree
	tree = malloc(1000);
	tree->node_count = sizeof(nodes)/sizeof(nodes[0]);
	tree->integer_count = sizeof(integers)/sizeof(integers[0]);
	tree->float_count = 0;
	tree->char_count = sizeof(strings);
	
	char* dst = (char*)(tree)+sizeof(*tree);
	memcpy(dst, nodes, sizeof(nodes));
	
	dst += tree->node_count*sizeof(nodes[0]);
	memcpy(dst, integers, sizeof(integers));
	
	dst += sizeof(integers);
	memcpy(dst, strings, sizeof(strings));

    // create process 1
    process_attrs.ENTRY_POINT = first_process;
    strncpy(process_attrs.NAME, "process 1", sizeof(PROCESS_NAME_TYPE));

    CREATE_PROCESS(&process_attrs, &pid, &ret);
    if (ret != NO_ERROR) {
        printf("couldn't create process 1: %d\n", (int) ret);
        return ;
    } else {
        printf("process 1 created\n");
    }
    
    START(pid, &ret);
    if (ret != NO_ERROR) {
        printf("couldn't start process 1: %d\n", (int) ret);
        return ;
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
