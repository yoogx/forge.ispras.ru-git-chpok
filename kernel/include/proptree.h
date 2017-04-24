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

#pragma once


#include <stddef.h> /* size_t defition. */
#include <stdint.h> /* int32_t and other defitions. */

/** Прозрачный тип дерева параметров. Не используется явно. */
struct jet_pt_header_struct;

/** Тип указателя на дерево параметров */
typedef const struct jet_pt_header_struct * jet_pt_tree_t;

/** Тип для представления идентификатора узла дерева */
typedef size_t jet_pt_node_t;

/** Корневой узел дерева параметров */
#define JET_PT_ROOT (jet_pt_node_t)(0)

/** Недопустимый идентификатор узла дерева */
#define JET_PT_INVALID_NODE (jet_pt_node_t)(-1)

/** Тип узла дерева */
typedef enum {
	JET_PT_STRING,	/**< Листовой узел дерева, содержит строковый параметр */
	JET_PT_INTEGER32, /**< Листовой узел дерева, содержит 32-битный целочисленный знаковый параметр */
	JET_PT_INTEGER64, /**< Листовой узел дерева, содержит 64-битный целочисленный знаковый параметр */
	JET_PT_UNSIGNED32, /**< Листовой узел дерева, содержит 32-битный целочисленный беззнаковый параметр */
	JET_PT_UNSIGNED64, /**< Листовой узел дерева, содержит 64-битный целочисленный беззнаковый параметр */
	JET_PT_FLOAT,   /**< Листовой узел дерева, содержит вещественный параметр одинарной точности (4 байта) */
	JET_PT_DOUBLE,   /**< Листовой узел дерева, содержит вещественный параметр двойной точности (8 байт) */
	JET_PT_BINARY,   /**< Листовой узел дерева, содержит бинарные данные */
	JET_PT_TREE,    /**< Узел дерева не содержит значения. Возможно, он является промежуточным (имеет детей). */
	JET_PT_INVALID_NODE_TYPE /**< Недопустимый тип узла дерева */
} jet_pt_node_type_t;

/** Возвращает число дочерних узлов данного узла.
 * 
 * @param tree Указатель на дерево параметров.
 * @param node Узел, для которого возвращается число дочерних узлов.
 * 
 * @pre node - корректный узел.
 * 
 * @return Число дочерних узлов заданного узла.
 */
size_t jet_pt_children_count(jet_pt_tree_t tree, jet_pt_node_t node);

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
jet_pt_node_t jet_pt_get_child(jet_pt_tree_t tree, jet_pt_node_t node, size_t num);

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
jet_pt_node_t jet_pt_next_sibling(jet_pt_tree_t tree, jet_pt_node_t node);

/** Возвращает идентификатор узла дерева по текстовому пути в дереве.
 * 
 * Путь в дереве имеет вид "a/b/c" и разрешается относительно узла parent.
 * Знаки слэш (/) разделяют имена промежуточных узлов в пути.
 * Несколько идущих подряд знаков слэш (/) интерпретируются как один
 * знак слэш.
 * Концевой знак слэш (/) игнорируется.
 * Начальный знак слэш (/) игнорируется, но при этом путь разрешается
 * относительно корневого узла дерева (другими словами, как если бы в
 * качестве узла parent указан JET_PT_ROOT).
 * 
 * @param tree Указатель на дерево параметров.
 * @param parent Узел, относительно которого ищется путь.
 * @param path Путь в дереве.
 * 
 * @pre path != NULL
 * @pre parent - корректный узел.
 * 
 * @return Идентификатор узла дерева, соответствующего пути, или
 *   JET_PT_INVALID_NODE если такой узел не существует.
 * Если path - пустая строка (или эквивалентен пустой строке), то
 * возвращается parent.
 */
jet_pt_node_t jet_pt_find(jet_pt_tree_t tree, jet_pt_node_t parent, const char * path);

/** Возвращает имя узла.
 * 
 * @param tree Указатель на дерево параметров.
 * @param node Идентификатор узла в дереве.
 * 
 * @pre node - корректный узел.
 * 
 * @return указатель на строку с именем узла.
 */
const char* jet_pt_get_node_name(jet_pt_tree_t tree, jet_pt_node_t node);

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
jet_pt_node_type_t jet_pt_get_node_type(jet_pt_tree_t tree, jet_pt_node_t node, const char* path);

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
const char* jet_get_string(jet_pt_tree_t tree, jet_pt_node_t node, const char* path);

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
int jet_pt_get_int32(jet_pt_tree_t tree, jet_pt_node_t node, const char* path, int32_t* result);

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
int jet_pt_get_int64(jet_pt_tree_t tree, jet_pt_node_t node, const char* path, int64_t* result);

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
int jet_pt_get_uint32(jet_pt_tree_t tree, jet_pt_node_t node, const char* path, uint32_t* result);

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
int jet_pt_get_uint64(jet_pt_tree_t tree, jet_pt_node_t node, const char* path, uint64_t* result);

/** Извлекает вещественное число двойной точности, хранящееся в узле.
 * Если path != NULL, то узел задается с помощью узла node и пути относительно него по правилам,
 * данным в описании jet_pt_find().
 * Если path == NULL, то извлекается содержимое узла node.
 * 
 * @param tree Указатель на дерево параметров.
 * @param node Идентификатор родительского или целевого узла.
 * @param path Путь относительно узла node или NULL.
 * @param result указатель на вещественную переменную, в которую следует записать результат.
 *
 * @pre result != NULL
 * @pre jet_pt_get_node_type(tree, node, path) == JET_PT_DOUBLE \
 *   || jet_pt_get_node_type(tree, node, path) == JET_PT_INVALID_NODE_TYPE
 * 
 * @return 0 в случае успеха.
 * Если целевой узел не существует, то возвращается -1.
 */
int jet_pt_get_double(jet_pt_tree_t tree, jet_pt_node_t node, const char* path, double* result);

/** Извлекает вещественное число одинарной точности, хранящееся в узле.
 * Если path != NULL, то узел задается с помощью узла node и пути относительно него по правилам,
 * данным в описании jet_pt_find().
 * Если path == NULL, то извлекается содержимое узла node.
 *
 * 
 * @param tree Указатель на дерево параметров.
 * @param node Идентификатор родительского или целевого узла.
 * @param path Путь относительно узла node или NULL.
 * @param result указатель на вещественную переменную, в которую следует записать результат.
 *
 * @pre result != NULL
 * @pre jet_pt_get_node_type(tree, node, path) == JET_PT_FLOAT \
 *   || jet_pt_get_node_type(tree, node, path) == JET_PT_INVALID_NODE_TYPE
 * 
 * @return 0 в случае успеха.
 * Если целевой узел не существует, то возвращается -1.
 */
int jet_pt_get_float(jet_pt_tree_t tree, jet_pt_node_t node, const char* path, float* result);

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
const void* jet_pt_get_binary(jet_pt_tree_t tree, jet_pt_node_t node, const char* path, size_t* size);

/*
 * Проверяет поле magic в заголовке дерева.
 * 
 * Возвращает 0 если поле корректно, -1 - если значение поля некорректно.
 */
int jet_pt_check_magic(jet_pt_tree_t tree);
