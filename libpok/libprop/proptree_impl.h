#pragma once
#include <proptree.h>

/** Структура данных для представления заголовка дерева параметров.
 */
struct jet_pt_header_struct {
	size_t node_count;    /**< Число узлов в дереве параметров */
	size_t integer_count; /**< Число целочисленных значений */
	size_t float_count;	  /**< Число вещественных параметров */
	size_t char_count;    /**< Число строковых параметров */
};

/** Недопустимое значение отступа */
#define JET_PT_INVALID_OFFSET (size_t)(-1)

/** Тип данных для задания смещения в блоке строк */
typedef size_t jet_pt_string_offset_t;

/** Структура данных для представления узла дерева */
typedef struct jet_pt_node {
	jet_pt_node_type_t type;	 /**< Тип узла */
	jet_pt_string_offset_t name; /**< Имя узла. Задаётся как смещение в блоке строк */
	size_t next_sibling_offset;  /**< Смещение до следующего сиблинга в массиве узлов дерева */
	size_t value_offset;		 /**< Значение узла. Задаётся как смещение до значения в соответствующем блоке (целых, вещественных или строковых значений) */
} jet_pt_node_impl;
