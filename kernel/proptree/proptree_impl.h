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
#include <proptree.h>

/* Все числовые значения заданы в big-endian формате. */

#define JET_PT_MAGIC 0x50725472 /* 'P', 'r', 'T', 'r' */

/** Структура данных для представления заголовка дерева параметров.
 */
struct jet_pt_header_struct {
	uint32_t magic;
	uint32_t nodes_offset; /**< Смещение от начала дерева до блока со списком узлов */
	uint32_t nodes_count; /**< Число узлов в дереве параметров */
	uint32_t strings_offset; /**< Смещение от начала дерева до блока со строками */
	uint32_t strings_size; /**< Общее количество байт в блоке со строками */
	uint32_t binary_data_offset; /**< Смещение от начала дерева до блока с бинарными данными */
	uint32_t binary_data_size; /**< Общий размер блока бинарных данных */
	uint32_t unused; /**< Padding */
};

/** Структура значения узла типа JET_PT_STRING. */
struct jet_pt_node_value_string {
	uint32_t string_offset; /**< Смещение в блоке строк. */
	uint32_t unused; /**< Не используется */
};

/** Структура значения узла типа JET_PT_INTEGER32. */
struct jet_pt_node_value_int32 {
	int32_t value; /**< Числовое значение. */
	uint32_t unused; /**< Не используется */
};

/** Структура значения узла типа JET_PT_INTEGER64. */
struct jet_pt_node_value_int64 {
	int64_t value; /**< Числовое значение. */
};

/** Структура значения узла типа JET_PT_UNSIGNED32. */
struct jet_pt_node_value_uint32 {
	uint32_t value; /**< Числовое значение. */
	uint32_t unused; /**< Не используется */
};

/** Структура значения узла типа JET_PT_UNSIGNED64. */
struct jet_pt_node_value_uint64 {
	uint64_t value; /**< Числовое значение. */
};

/** Структура значения узла типа JET_PT_FLOAT. */
struct jet_pt_node_value_float {
	float value; /**< Числовое значение. */
	uint32_t unused; /**< Не используется */
};

/** Структура значения узла типа JET_PT_DOUBLE. */
struct jet_pt_node_value_double {
	double value; /**< Числовое значение. */
};

/** Структура значения узла типа JET_PT_BINARY. */
struct jet_pt_node_value_binary {
	uint32_t binary_offset; /**< Смещение в блоке бинарных данных. */
	uint32_t size; /**< Размер данных */
};

/** Структура значения узла типа JET_PT_TREE. */
struct jet_pt_node_value_tree {
	uint32_t n_children; /**< Количество дочерних узлов. */
	uint32_t first_child; /**< Индекс первого ребенка в блоке узлов или JET_PT_INVALID_NODE если детей нет. */
};

/** Флаг: У узла есть "младшие" братья*/
#define JET_PT_FLAG_HAS_FUTHER_SIBLING 1

/** Структура данных для представления узла дерева */
typedef struct jet_pt_node {
	uint16_t type; /**< Тип узла */
	uint16_t flags; /**< Флаги узла. Комбинация 'JET_PT_FLAG_*' */
	uint32_t name; /**< Имя узла. Задаётся как смещение в блоке строк */
	union { /**< Значение узла, в зависимости от типа. Размер 64 бита (8 байт). */
		struct jet_pt_node_value_string value_string;
		struct jet_pt_node_value_int32 value_int32;
		struct jet_pt_node_value_int64 value_int64;
		struct jet_pt_node_value_uint32 value_uint32;
		struct jet_pt_node_value_uint64 value_uint64;
		struct jet_pt_node_value_float value_float;
		struct jet_pt_node_value_double value_double;
		struct jet_pt_node_value_binary value_binary;
		struct jet_pt_node_value_tree value_tree;
	};
} jet_pt_node_impl;
