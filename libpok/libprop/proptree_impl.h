#pragma once
#include <proptree.h>

struct jet_pt_header_struct {
	size_t node_count;
	size_t integer_count;
	size_t float_count;
	size_t char_count;
};

#define JET_PT_INVALID_OFFSET (size_t)(-1)

typedef size_t jet_pt_string_offset_t;

typedef struct jet_pt_node {
	jet_pt_node_type_t type;
	jet_pt_string_offset_t name;
	size_t next_sibling_offset;
	size_t value_offset;
} jet_pt_node_impl;
