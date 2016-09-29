#pragma once

#include <proptree.h>

/** Возвращает конфигурационное дерево свойств для раздела. */
jet_pt_tree_t jet_conf_partition_tree(void);

/** Синоним jet_pt_children_count(jet_conf_partition_tree(), ...). */
size_t jet_conf_children_count(jet_pt_node_t node);

/** Синоним jet_pt_get_child(jet_conf_partition_tree(), ...). */
jet_pt_node_t jet_conf_get_child(jet_pt_node_t node, size_t num);

/** Синоним jet_pt_find(jet_conf_partition_tree(), ...). */
jet_pt_node_t jet_conf_find(jet_pt_node_t parent, const char * path);

/** Синоним jet_pt_get_node_name(jet_conf_partition_tree(), ...). */
const char* jet_conf_get_node_name(jet_pt_node_t node);

/** Синоним jet_pt_get_node_type(jet_conf_partition_tree(), ...). */
jet_pt_node_type_t jet_conf_get_node_type(jet_pt_node_t parent, const char* path);

/** Синоним jet_pt_get_string(jet_conf_partition_tree(), ...). */
const char* jet_conf_get_string(jet_pt_tree_t tree, jet_pt_node_t parent, const char* path);

/** Синоним jet_pt_get_int32_t(jet_conf_partition_tree(), ...). */
int jet_conf_get_int32(jet_pt_node_t parent, const char* path, int32_t* result);

/** Синоним jet_pt_get_int64_t(jet_conf_partition_tree(), ...). */
int jet_conf_get_int64(jet_pt_node_t parent, const char* path, int64_t* result);

/** Синоним jet_pt_get_uint32_t(jet_conf_partition_tree(), ...). */
int jet_conf_get_uint32(jet_pt_node_t parent, const char* path, uint32_t* result);

/** Синоним jet_pt_get_uint64_t(jet_conf_partition_tree(), ...). */
int jet_conf_get_uint64(jet_pt_node_t parent, const char* path, uint64_t* result);

/** Синоним jet_pt_get_double(jet_conf_partition_tree(), ...). */
int jet_conf_get_double(jet_pt_node_t node, const char* path, double* result);

/** Синоним jet_pt_get_float(jet_conf_partition_tree(), ...). */
int jet_conf_get_float(jet_pt_node_t node, const char* path, float* result);
