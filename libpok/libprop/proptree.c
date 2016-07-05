#include <types.h>
#include "proptree_impl.h"
#include <assert.h>
#include <string.h>


jet_pt_node_t jet_pt_root(jet_pt_tree_t tree) {
	return 0;
}

jet_pt_node_impl * jet_pt_get_node(jet_pt_tree_t tree, jet_pt_node_t node) {
	// Внутренняя функция. 
	// Предполагаем, что выполняются условия
	// tree != NULL
	// node < tree->node_count
	jet_pt_node_impl* nodes = (jet_pt_node_impl*)(tree+1);
	return nodes + node;
}

/** Проверяет, есть ли следующий сиблинг у заданного узла.
 * 
 * Внутренняя функция, предполагает, что выполняются ряд условий на аргументы.
 * 
 * @return TRUE если есть следующий сиблинг и FALSE в противном случае.
 * 
 * @pre tree != NULL
 * @pre child < tree->node_count
 */
jet_bool_t jet_pt_has_sibling(jet_pt_tree_t tree, jet_pt_node_t child) {
	jet_pt_node_impl * node = jet_pt_get_node(tree, child);
	return node->next_sibling_offset != JET_PT_INVALID_OFFSET;
}

/** Возвращает строку по заданному смещению.
 * 
 * Внутренняя функция, предполагает, что выполняются ряд условий на аргументы.
 * 
 * @return NULL если смещение слишком велико и указатель на строку в противном случае.
 * 
 * @pre tree != NULL
 */
const char * jet_pt_get_string(jet_pt_tree_t tree, jet_pt_string_offset_t name_offset) {
	if (name_offset >= tree->char_count) {
		return NULL;
	}
	const char * data_start = (const char*)(tree+1);
	/* Данные лежат в памяти следующим образом:
	 * -- узлы дерева, общее число node_count
	 * -- целые значения, общее число integer_count
	 * -- вещественные значения, общее число float_count
	 */
	 // TODO надо сделать выравнивание для вещественных значений.
	const char * strings_start = data_start 
		+ tree->node_count*sizeof(jet_pt_node_impl) 
		+ tree->integer_count*sizeof(int)
		+ tree->float_count* sizeof(double)
		;
	return strings_start + name_offset;
}

/** Возвращает указатель на ближайший к началу символ, отличающийся от  первого.
 * 
 * Например, <code>jet_util_skip_char("rright")</code> вернёт указатель на 'i'.
 * 
 * @pre str != NULL && str[0] != '\0'
 */
const char * jet_util_skip_leading_char(const char* str) {
	// Выполняется: str != NULL
	char c = str[0];
	/* Обоснование завершения цикла:
	 * -- предполагаем, что str - корректная нуль-терминированная строка,
	 * -- кроме того, по предусловию c != '\0'
	 * -- тогда через strlen(str) итераций str[0] == '\0', то есть str[0] != c
	 * То есть число итераций не более strlen(str).
	 */
	while (str[0] == c) {
		++str;
	}
	return str;
}

/** Проверяет совпадение имени узла заданной строке.
 * 
 * Строка для проверки задаётся как len первых символов строки str.
 */
jet_bool_t jet_pt_name_match(jet_pt_tree_t tree, jet_pt_node_t node, const char* str, int len) {
	jet_pt_node_impl* node_impl = jet_pt_get_node(tree, node);
	
	const char* name = jet_pt_get_string(tree, node_impl->name);
	assert(name != NULL);
	
	return strlen(name) == len
		&& strncmp(name, str, len) == 0;
}

/** Возвращает идентификатор следующего сиблинга заданного узла.
 * 
 * @return JET_PT_INVALID_NODE если нет сиблингов (листовой узел или крайний сиблинг)
 *   и идентификатор следующего сиблинга в противном случае.
 * 
 * @pre tree != NULL
 * @pre node < tree->node_count
 */
jet_pt_node_t jet_pt_next_sibling(jet_pt_tree_t tree, jet_pt_node_t node) {
	jet_pt_node_impl* node_impl = jet_pt_get_node(tree, node);
	
	if (node_impl->next_sibling_offset == JET_PT_INVALID_OFFSET) {
		return JET_PT_INVALID_NODE;
	}
	return node + node_impl->next_sibling_offset;
}

/** Возвращает дочерний узел с заданным именем.
 * 
 * Имя дочернего узла должно совпадать с len первых символов параметра segment.
 * 
 * @return JET_PT_INVALID_NODE если узел листовой или нет дочернего узла с заданным именем,
 * 	или дочерний узел, если такой существует.
 * 
 * @pre strlen(segment) <= len
 * @pre tree != NULL
 * @pre parent < tree->node_count
 */
jet_pt_node_t jet_pt_find_child(jet_pt_tree_t tree, jet_pt_node_t parent, const char * segment, int len) {
	jet_pt_node_impl* node = jet_pt_get_node(tree, parent);
	if (node->type != JET_PT_TREE) {
		return JET_PT_INVALID_NODE;
	}
	// Перебор детей
	// Первый ребёнок находится в следующем элементе массива
	jet_pt_node_t child = parent + 1;
	// В корректно сформированном дереве выполняется следующее требование:
	assert(child < tree->node_count);

	if (jet_pt_name_match(tree, child, segment, len)) {
		return child;
	}
	/* Обоснование завершения цикла: в корректно сфорированном дереве 
	 *   число сиблингов не превосходит tree->node_count-1 (-1 так как 
	 *   у корня нет сиблингов). Поэтому число итераций не превосходит
	 *   tree->node_count-1 
	 * Более точно: число итераций не превосходит tree->node_count - parent - 1,
	 *   так как в дереве все дети находятся справа от родителя. -1 так как 
	 *   первый дочерний узел уже проверен.
	 */ 
	while(jet_pt_has_sibling(tree, child)) {
		child = jet_pt_next_sibling(tree, child);
		if (jet_pt_name_match(tree, child, segment, len)) {
			return child;
		}
	}
	return JET_PT_INVALID_NODE;
}

jet_pt_node_t jet_pt_find(jet_pt_tree_t tree, jet_pt_node_t parent, const char * path) {
	// Проверки параметров
	// Указатель на область дерева не может быть нулевым
	assert(tree != NULL);
	// Указатель на путь не может быть нулевым 
	// TODO: подумать -- возможно, нулевой указатель пути должен указывать на сам узел
	assert(path != NULL);
	
	if (parent > tree->node_count) {
		return JET_PT_INVALID_NODE;
	}
	// Путь, начинающийся со '/' отсчитывается от корня
	if (path[0] == '/') {
		parent = jet_pt_root(tree);
		// Смещаем указатель за слэш
		path = jet_util_skip_leading_char(path);
	}
	// Выполняется: path[0] != '/'
	// Обоснование: либо изначально path[0] != '/', либо в результате 
	//   работы цикла skip_slashes указатель path перемещен на символ, отличный от '/'
	
	// Пустой путь указывает на сам узел
	if (path[0] == '\0') {
		return parent;
	}
	
	jet_pt_node_t node = parent;
	// Вместо рекурсии используется цикл.
	/* Вариант цикла: strlen(path) 
	 * 	эта величина всё время монотонно убывает.
	 * 
	 * Число итераций цикла не превосходит число сегментов в пути к узлу.
	 * Максимальное число сегментов не превосходит strlen(path)/2 + 1
	 * Такая длина реализуется на путях вида "a/b/c/"
	 */
	while(path[0] != '\0') {
		// Выполняется: путь не пустой
		const char * slash = strchr(path, '/');
		if (slash == NULL) {
			// Найден последний сегмент
			return jet_pt_find_child(tree, node, path, strlen(path));
		}
		// Выполняется: путь содержит слэш не только в первой позиции
		// Выполняется: path[0] != '/' (см. выше)
		// Следствие: slash - path > 0
		node = jet_pt_find_child(tree, node, path, slash - path);
		
		if (node == JET_PT_INVALID_NODE) {
			return node;
		}
		// Выполняется: node - корректный узел
		// Перемещаем указатель пути за слэш.
		// Слэшей может быть несколько, поэтому проматываем указатель за них
		path = jet_util_skip_leading_char(slash);
		// Выполняется: path[0] != '/'
		// Возможно: path[0] == '\0'
	}
	return node;
}

/** Возвращает имя узла.
 * 
 * @param tree Указатель на дерево параметров.
 * @param node Идентификатор узла в дереве.
 * 
 * @pre tree != NULL
 * @return указатель на строку с именем узла, если идентификатор узла корректен, и NULL в противном случае.
 */
const char* jet_pt_get_node_name(jet_pt_tree_t tree, jet_pt_node_t node) {
	// Проверки параметров
	// Указатель на область дерева не может быть нулевым
	assert(tree != NULL);
	if (node >= tree->node_count) {
		return NULL;
	}
	
	jet_pt_node_impl* node_impl = jet_pt_get_node(tree, node);
	return jet_pt_get_string(tree, node_impl->name);
}

/** Возвращает строковое значение, хранящееся в узле.
 * 
 * @param result указатель на строковый буфер, в который следует записать результат.
 * @param len	указатель на переменную, содержащую размер буфера *result. 
 * 		Если буфер меньше строкового значения, содержащегося в узле дерева, в эту
 * 		переменную будет записан требуемый размер.
 * @pre tree != NULL
 * @pre result != NULL
 * @pre len != NULL
 * 
 * @return 
 *   -- POK_ERRNO_OK если узел является корректным и содержит строковое значение,
 *   -- POK_ERRNO_EINVAL, если узел некорректен или не содержит строковое значение 
 *   (например, является промежуточным узлом или листовым узлом с целочисленным или 
 *   вещественным значением).
 *   -- POK_ERRNO_PARAM, если узел содержит строковое значение, но размер выходного
 *   буфера слишком мал.  В этом случае в переменную *len записывается требуемый 
 *   размер.
 */
pok_ret_t jet_get_string_value(jet_pt_tree_t tree, jet_pt_node_t node, 
	const char ** result, size_t * len) {
	return POK_ERRNO_EINVAL;
}

/** Возвращает размер строкового значения, хранящегося в узле.
 * 
 * @param result указатель на переменную, в которую будет записан размер строкового значения.
 * @pre tree != NULL
 * @pre result != NULL
 * 
 * @return 
 *   -- POK_ERRNO_OK если узел является корректным и содержит строковое значение,
 *   -- POK_ERRNO_EINVAL, если узел некорректен или не содержит строковое значение 
 *   (например, является промежуточным узлом или листовым узлом с целочисленным или 
 *   вещественным значением).
 */
pok_ret_t jet_get_string_value_size(jet_pt_tree_t tree, jet_pt_node_t node, 
	size_t * result) {
	return POK_ERRNO_EINVAL;
}

/** Возвращает целое число, хранящееся в узле.
 * 
 * @param result указатель на целочисленную переменную, в которую следует записать результат.
 * @pre tree != NULL
 * @pre result != NULL
 * 
 * @return POK_ERRNO_OK если узел является корректным и содержит целочисленное значение,
 *   и POK_ERRNO_EINVAL, если узел некорректен или не содержит целочисленное значение 
 *   (например, является промежуточным узлом или листовым узлом со строковым или 
 *   вещественным значением). 
 */
pok_ret_t jet_get_integer_value(jet_pt_tree_t tree, jet_pt_node_t node, int* result) {
	return POK_ERRNO_EINVAL;
}

/** Возвращает вещественное число, хранящееся в узле.
 * 
 * @param result указатель на вещественную переменную, в которую следует записать результат.
 * @pre tree != NULL
 * @pre result != NULL
 * 
 * @return POK_ERRNO_OK если узел является корректным и содержит вещественное значение,
 *   и POK_ERRNO_EINVAL, если узел некорректен или не содержит вещественное значение 
 *   (например, является промежуточным узлом или листовым узлом со строковым или 
 *   целочисленным значением). 
 */
pok_ret_t jet_get_double_value(jet_pt_tree_t tree, jet_pt_node_t node, double* result) {
	return POK_ERRNO_EINVAL;
}


