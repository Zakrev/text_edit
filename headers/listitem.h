#ifndef _LISTITEM_H_
#define _LISTITEM_H_

#include "debug_print.h"

typedef struct main_editor_listItem ListItem;
struct main_editor_listItem {
	ListItem * next;
	ListItem * prev;
};

#define foreach_in_list(__item__, __start__) \
        for(__item__ = __start__; __item__ != NULL; __item__ = __item__->next)
        
#define foreach_in_list_reverse(__item__, __start__) \
        for(__item__ = __start__; __item__ != NULL; __item__ = __item__->prev)

/*
        Вставляет item в позицию pos списка
        Элемент pos сдвигается "под" item
        Возвращает 0 в случае успеха
*/
int insert_ListItem_offset_down(ListItem * pos, ListItem * item);

/*
        Вставляет item в позицию pos списка
        Элемент item сдвигается "под" pos
        Возвращает 0 в случае успеха
*/
int insert_ListItem_offset_up(ListItem * pos, ListItem * item);

/*
        Удаляет item из списка
        в котором он находится
*/
int erase_ListItem(ListItem * item);

#endif