/*
 * COPIED! DO NOT MODIFY!
 *
 * Instead of modifying this file, modify original one (kernel/proptree/proptree.c).
 */
/*
 * Institute for System Programming of the Russian Academy of Sciences
 * Copyright (C) 2017 ISPRAS
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

#include "proptree_impl.h"

#include <assert.h>
#include <string.h>
#include <arpa/inet.h>

/* Возвращает указатель на начало блока в дереве. */
static const void* pt_get_block(jet_pt_tree_t tree, uint32_t block_offset)
{
    return (const char*)tree + block_offset;
}

/* Возвращает количество узлов в дереве. */
static uint32_t pt_n_nodes(jet_pt_tree_t tree)
{
    return ntohl(tree->nodes_count);
}

/* Возвращает внутреннюю реализацию указанного узла. */
static const jet_pt_node_impl * pt_get_node(jet_pt_tree_t tree, jet_pt_node_t node)
{
    // Внутренняя функция.
    // Предполагаем, что выполняются условия
    // tree != NULL
    // node < tree->nodes_count
    const jet_pt_node_impl* nodes = pt_get_block(tree, ntohl(tree->nodes_offset));
    return nodes + node;
}

/* Возвращает тип узла */
static jet_pt_node_type_t pt_node_type(const jet_pt_node_impl* node_impl)
{
    return ntohs(node_impl->type);
}

/** Возвращает строку по заданному смещению.
 *
 * Внутренняя функция, предполагает, что выполняются ряд условий на аргументы.
 *
 * @return NULL если смещение слишком велико и указатель на строку в противном случае.
 *
 * @pre tree != NULL
 */
static const char * pt_get_string(jet_pt_tree_t tree, uint32_t string_offset)
{
    assert(string_offset < ntohl(tree->strings_size));

    const char * strings_start = pt_get_block(tree, ntohl(tree->strings_offset));

    return strings_start + string_offset;
}

/** Возвращает имя узла */
static const char* pt_node_name(jet_pt_tree_t tree, const jet_pt_node_impl* node_impl)
{
    return pt_get_string(tree, ntohl(node_impl->name));
}


/** Возвращает число дочерних узлов данного узла.
 *
 * @param tree Указатель на дерево параметров.
 * @param node Узел, для которого возвращается число дочерних узлов.
 *
 * @pre node - корректный узел.
 *
 * @return Число дочерних узлов заданного узла.
 */
size_t jet_pt_children_count(jet_pt_tree_t tree, jet_pt_node_t node)
{
    assert(tree != NULL);
    assert(node < pt_n_nodes(tree));

    const jet_pt_node_impl* node_impl = pt_get_node(tree, node);

    if(pt_node_type(node_impl) == JET_PT_TREE) return 0;

    return ntohl(node_impl->value_tree.n_children);
}

/** Возвращает дочерний узел с заданным номером.
 *
 * @param tree Указатель на дерево параметров.
 * @param node Узел, для которого возвращается дочерний узел.
 * @param num Номер дочернего узла. Дочерние узлы нумеруются, начиная с 0.
 *
 * @pre node - корректный узел.
 *
 * @return Заданный дочерний узел. Если у заданного узла меньше чем
 * num дочерних, то возвращается JET_PT_INVALID_NODE.
 */
jet_pt_node_t jet_pt_get_child(jet_pt_tree_t tree, jet_pt_node_t node, size_t num)
{
    assert(tree != NULL);
    assert(node < pt_n_nodes(tree));

    const jet_pt_node_impl* node_impl = pt_get_node(tree, node);

    if(pt_node_type(node_impl) != JET_PT_TREE) return JET_PT_INVALID_NODE;
    if(num >= ntohl(node_impl->value_tree.n_children)) return JET_PT_INVALID_NODE;

    return ntohl(node_impl->value_tree.first_child) + num;
}

/** Возвращает следующего брата заданного узла.
 *
 * @param tree Указатель на дерево параметров.
 * @param node Узел, для которого возвращается брат.
 *
 * @pre node - корректный узел.
 *
 * @return Следующий брат заданного узла. Если у заданного узла нет
 * следующего брата, то возвращается JET_PT_INVALID_NODE.
 */
jet_pt_node_t jet_pt_next_sibling(jet_pt_tree_t tree, jet_pt_node_t node)
{
    assert(tree != NULL);
    assert(node < pt_n_nodes(tree));

    const jet_pt_node_impl* node_impl = pt_get_node(tree, node);

    if(ntohs(node_impl->flags) & JET_PT_FLAG_HAS_FUTHER_SIBLING)
        return node + 1;
    else
        return JET_PT_INVALID_NODE;
}


/** Возвращает дочерний узел с заданным именем.
 *
 * Имя дочернего узла должно совпадать с len первых символов параметра name.
 *
 * @return NULL если узел листовой или нет дочернего узла с заданным именем,
 *     или дочерний узел, если такой существует.
 *
 * @pre strlen(segment) <= len
 * @pre tree != NULL
 * @pre parent_impl != NULL
 */
static const jet_pt_node_impl* pt_find_child(jet_pt_tree_t tree,
    const jet_pt_node_impl* parent_impl, const char * name, int len)
{
    if(pt_node_type(parent_impl) != JET_PT_TREE) return NULL;
    const struct jet_pt_node_value_tree* value_tree = &parent_impl->value_tree;

    uint32_t n_children = ntohl(value_tree->n_children);

    if(n_children == 0) return NULL;

    const jet_pt_node_impl* children_impl = pt_get_node(tree, ntohl(value_tree->first_child));

    for(uint32_t i = 0; i < n_children; i++) {
        const jet_pt_node_impl* child_impl = children_impl + i;

        const char* child_name = pt_node_name(tree, child_impl);

        if((strncmp(child_name, name, len) == 0)
            && (child_name[len] == '\0'))
            return child_impl;
    }

    return NULL;
}

const jet_pt_node_impl* pt_find(jet_pt_tree_t tree, jet_pt_node_t parent, const char * path) {
    // Считаем что все проверки сделаны.
    const jet_pt_node_impl* parent_impl;
    // Путь, начинающийся со '/' отсчитывается от корня
    if (*path == '/') {
        parent_impl = pt_get_node(tree, JET_PT_ROOT);
    } else {
        parent_impl = pt_get_node(tree, parent);
    }

    while(1) {
        // Skip slashes at the beginning of the path component.
        while(*path == '/') path++;
        // Empty path means parent itself
        if(*path == '\0') break;
        // Calculate length of the component. It is one until slash or null.
        size_t component_len = 1;

        while(path[component_len]) {
            if(path[component_len] == '/') break;
            component_len++;
        }
        // Lookup the child
        parent_impl = pt_find_child(tree, parent_impl, path, component_len);

        if(!parent_impl) break;
        // Advance path after the component.
        path = path + component_len;
    }

    return parent_impl;
}


jet_pt_node_t jet_pt_find(jet_pt_tree_t tree, jet_pt_node_t parent, const char * path) {
    // Проверки параметров
    // Указатель на область дерева не может быть нулевым
    assert(tree != NULL);
    // Указатель на путь не может быть нулевым
    assert(path != NULL);
    // parent - корректный узел
    assert(parent < pt_n_nodes(tree));

    const jet_pt_node_impl* node_impl = pt_find(tree, parent, path);

    return node_impl ? (jet_pt_node_t)(node_impl - pt_get_node(tree, JET_PT_ROOT)) : JET_PT_INVALID_NODE;
}

/** Возвращает имя узла.
 *
 * @param tree Указатель на дерево параметров.
 * @param node Идентификатор узла в дереве.
 *
 * @pre node - корректный узел.
 *
 * @return указатель на строку с именем узла.
 */
const char* jet_pt_get_node_name(jet_pt_tree_t tree, jet_pt_node_t node) {
    // Проверки параметров
    // Указатель на область дерева не может быть нулевым
    assert(tree != NULL);
    assert(node < pt_n_nodes(tree));

    const jet_pt_node_impl* node_impl = pt_get_node(tree, node);
    return pt_node_name(tree, node_impl);
}

/** Возвращает тип узла.
 * Если path != NULL, то узел задается с помощью узла node и пути относительно него по правилам,
 * данным в описании jet_pt_find().
 * Если path == NULL, то возвращается информация об узле node.
 *
 * @param tree Указатель на дерево параметров.
 * @param node Идентификатор родительского или целевого узла.
 * @param path Путь относительно узла node или NULL.
 *
 * @return тип узла, если целевой узел существует, и JET_PT_INVALID_NODE_TYPE в противном случае.
 */
jet_pt_node_type_t jet_pt_get_node_type(jet_pt_tree_t tree, jet_pt_node_t node, const char* path)
{
    // Проверки параметров
    assert(tree != NULL);
    assert(node < pt_n_nodes(tree));

    const jet_pt_node_impl* node_impl;

    if(path) {
        node_impl = pt_find(tree, node, path);
        if(!node_impl) return JET_PT_INVALID_NODE_TYPE;
    } else {
        node_impl = pt_get_node(tree, node);
    }

    return pt_node_type(node_impl);
}


/** Возвращает строковое значение, хранящееся в узле.
 * Если path != NULL, то узел задается с помощью узла node и пути относительно него по правилам,
 * данным в описании jet_pt_find().
 * Если path == NULL, то возвращается информация об узле parent.
 *
 * @param tree Указатель на дерево параметров.
 * @param node Идентификатор родительского или целевого узла.
 * @param path Путь относительно узла node или NULL.
 *
 * @pre node - корректный узел.
 * @pre jet_pt_get_node_type(tree, node, path) == JET_PT_STRING \
 *   || jet_pt_get_node_type(tree, node, path) == JET_PT_INVALID_NODE_TYPE
 *
 * @return строку, хранящуюся в узле.
 * Если целевой узел не существует, то возвращается NULL.
 */
const char* jet_pt_get_string(jet_pt_tree_t tree, jet_pt_node_t node, const char* path)
{
    // Проверки параметров
    assert(tree != NULL);
    assert(node < pt_n_nodes(tree));

    const jet_pt_node_impl* node_impl;

    if(path) {
        node_impl = pt_find(tree, node, path);
        if(!node_impl) return NULL;
    } else {
        node_impl = pt_get_node(tree, node);
    }

    assert(pt_node_type(node_impl) == JET_PT_STRING);

    return pt_get_string(tree, ntohl(node_impl->value_string.string_offset));
}

/** Извлекает целое знаковое 32-битное число, хранящееся в узле.
 * Если path != NULL, то узел задается с помощью узла node и пути относительно него по правилам,
 * данным в описании jet_pt_find().
 * Если path == NULL, то извлекается содержимое узла node.
 *
 * @param tree Указатель на дерево параметров.
 * @param node Идентификатор родительского или целевого узла.
 * @param path Путь относительно узла node или NULL.
 * @param result указатель на переменную, в которую следует записать результат.
 *
 * @pre result != NULL
 * @pre jet_pt_get_node_type(tree, node, path) == JET_PT_INTEGER32 \
 *   || jet_pt_get_node_type(tree, node, path) == JET_PT_INVALID_NODE_TYPE
 *
 * @return 0 в случае успеха.
 * Если целевой узел не существует, то возвращается -1.
 */
int jet_pt_get_int32(jet_pt_tree_t tree, jet_pt_node_t node, const char* path, int32_t* result)
{
    // Проверки параметров
    assert(tree != NULL);
    assert(node < pt_n_nodes(tree));

    const jet_pt_node_impl* node_impl;

    if(path) {
        node_impl = pt_find(tree, node, path);
        if(!node_impl) return -1;
    } else {
        node_impl = pt_get_node(tree, node);
    }

    assert(pt_node_type(node_impl) == JET_PT_INTEGER32);

    *result = ntohl(node_impl->value_int32.value);

    return 0;
}

/** Извлекает целое знаковое 64-битное число, хранящееся в узле.
 * Если path != NULL, то узел задается с помощью узла node и пути относительно него по правилам,
 * данным в описании jet_pt_find().
 * Если path == NULL, то извлекается содержимое узла node.
 *
 * @param tree Указатель на дерево параметров.
 * @param node Идентификатор родительского или целевого узла.
 * @param path Путь относительно узла node или NULL.
 * @param result указатель на целочисленную переменную, в которую следует записать результат.
 *
 * @pre result != NULL
 * @pre jet_pt_get_node_type(tree, node, path) == JET_PT_INTEGER64 \
 *   || jet_pt_get_node_type(tree, node, path) == JET_PT_INVALID_NODE_TYPE
 *
 * @return 0 в случае успеха.
 * Если целевой узел не существует, то возвращается -1.
 */
int jet_pt_get_int64(jet_pt_tree_t tree, jet_pt_node_t node, const char* path, int64_t* result)
{
    // Проверки параметров
    assert(tree != NULL);
    assert(node < pt_n_nodes(tree));

    const jet_pt_node_impl* node_impl;

    if(path) {
        node_impl = pt_find(tree, node, path);
        if(!node_impl) return -1;
    } else {
        node_impl = pt_get_node(tree, node);
    }

    assert(pt_node_type(node_impl) == JET_PT_INTEGER64);

    *result = ntoh64(node_impl->value_int64.value);

    return 0;
}

/** Извлекает целое беззнаковое 32-битное число, хранящееся в узле.
 * Если path != NULL, то узел задается с помощью узла node и пути относительно него по правилам,
 * данным в описании jet_pt_find().
 * Если path == NULL, то извлекается содержимое узла node.
 *
 * @param tree Указатель на дерево параметров.
 * @param node Идентификатор родительского или целевого узла.
 * @param path Путь относительно узла node или NULL.
 * @param result указатель на целочисленную переменную, в которую следует записать результат.
 *
 * @pre result != NULL
 * @pre jet_pt_get_node_type(tree, node, path) == JET_PT_UNSIGNED32 \
 *   || jet_pt_get_node_type(tree, node, path) == JET_PT_INVALID_NODE_TYPE
 *
 * @return 0 в случае успеха.
 * Если целевой узел не существует, то возвращается -1.
 */
int jet_pt_get_uint32(jet_pt_tree_t tree, jet_pt_node_t node, const char* path, uint32_t* result)
{
    // Проверки параметров
    assert(tree != NULL);
    assert(node < pt_n_nodes(tree));

    const jet_pt_node_impl* node_impl;

    if(path) {
        node_impl = pt_find(tree, node, path);
        if(!node_impl) return -1;
    } else {
        node_impl = pt_get_node(tree, node);
    }

    assert(pt_node_type(node_impl) == JET_PT_UNSIGNED32);

    *result = ntohl(node_impl->value_uint32.value);

    return 0;
}

/** Извлекает целое беззнаковое 64-битное число, хранящееся в узле.
 * Если path != NULL, то узел задается с помощью узла node и пути относительно него по правилам,
 * данным в описании jet_pt_find().
 * Если path == NULL, то извлекается содержимое узла node.
 *
 * @param tree Указатель на дерево параметров.
 * @param node Идентификатор родительского или целевого узла.
 * @param path Путь относительно узла node или NULL.
 * @param result указатель на целочисленную переменную, в которую следует записать результат.
 *
 * @pre result != NULL
 * @pre jet_pt_get_node_type(tree, node, path) == JET_PT_UNSIGNED64 \
 *   || jet_pt_get_node_type(tree, node, path) == JET_PT_INVALID_NODE_TYPE
 *
 * @return 0 в случае успеха.
 * Если целевой узел не существует, то возвращается -1.
 */
int jet_pt_get_uint64(jet_pt_tree_t tree, jet_pt_node_t node, const char* path, uint64_t* result)
{
    // Проверки параметров
    assert(tree != NULL);
    assert(node < pt_n_nodes(tree));

    const jet_pt_node_impl* node_impl;

    if(path) {
        node_impl = pt_find(tree, node, path);
        if(!node_impl) return -1;
    } else {
        node_impl = pt_get_node(tree, node);
    }

    assert(pt_node_type(node_impl) == JET_PT_UNSIGNED64);

    *result = ntoh64(node_impl->value_uint64.value);

    return 0;
}

/** Возвращает указатель на бинарные данные, хранящиеся в узле.
 * Если path != NULL, то узел задается с помощью узла node и пути относительно него по правилам,
 * данным в описании jet_pt_find().
 * Если path == NULL, то возвращается информация об узле node.
 *
 * @param tree Указатель на дерево параметров.
 * @param node Идентификатор родительского или целевого узла.
 * @param path Путь относительно узла node или NULL.
 * @param size Указатель на переменную, куда в случае успеха записывается размер бинарных данных.
 *             Если указатель NULL, то переменная не используется.
 *
 * @pre jet_pt_get_node_type(tree, node, path) == JET_PT_BINARY \
 *   || jet_pt_get_node_type(tree, node, path) == JET_PT_INVALID_NODE_TYPE
 *
 * @return указатель на бинарные данные, хранящиеся в узле.
 * Если целевой узел не существует, то возвращается NULL.
 */
const void* jet_pt_get_binary(jet_pt_tree_t tree, jet_pt_node_t node, const char* path, size_t* size)
{
    // Проверки параметров
    assert(tree != NULL);
    assert(node < pt_n_nodes(tree));

    const jet_pt_node_impl* node_impl;

    if(path) {
        node_impl = pt_find(tree, node, path);
        if(!node_impl) return NULL;
    } else {
        node_impl = pt_get_node(tree, node);
    }

    assert(pt_node_type(node_impl) == JET_PT_BINARY);

    const char* binary_data = pt_get_block(tree, ntohl(tree->binary_data_offset));
    uint32_t binary_offset = ntohl(node_impl->value_binary.binary_offset);

    // Check for the tree itself.
    assert(binary_offset < ntohl(tree->binary_data_size));

    if(size)
        *size = ntohl(node_impl->value_binary.size);

    return binary_data + binary_offset;
}

int jet_pt_check_magic(jet_pt_tree_t tree)
{
    return (ntohl(tree->magic) == JET_PT_MAGIC) ? 0 : -1;
}
