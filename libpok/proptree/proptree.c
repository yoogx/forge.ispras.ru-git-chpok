#include <types.h>
#include "proptree_impl.h"
#include <assert.h>
#include <string.h>

static const void* pt_get_block(jet_pt_tree_t tree, uint32_t block_offset)
{
	// Внутренняя функция. 
	// Возвращает указатель на начало блока в дереве.
	return (const char*)tree + block_offset;
}

static const jet_pt_node_impl * pt_get_node(jet_pt_tree_t tree, jet_pt_node_t node)
{
	// Внутренняя функция. 
	// Предполагаем, что выполняются условия
	// tree != NULL
	// node < tree->nodes_count
	const jet_pt_node_impl* nodes = pt_get_block(tree, tree->nodes_offset);
	return nodes + node;
}

const char* jet_pt_get_node_name(jet_pt_tree_t tree, jet_pt_node_t node)
{
	assert(node < tree->nodes_count);
	const jet_pt_node_impl * node_impl = pt_get_node(tree, node);
	
	const char* strings = pt_get_block(tree, tree->strings_offset);
	
	return strings + node_impl->name;
}

jet_pt_node_t jet_pt_find(jet_pt_tree_t tree, jet_pt_node_t parent, const char * path)
{
	// TODO
	return JET_PT_INVALID_NODE;
}

int jet_pt_get_int32(jet_pt_tree_t tree, jet_pt_node_t node, const char* path, int32_t* result)
{
	//TODO
	return -1;
}
