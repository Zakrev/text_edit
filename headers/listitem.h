#ifndef _LISTITEM_H_
#define _LISTITEM_H_

#include "debug_print.h"

typedef struct main_editor_listItem ListItem;
struct main_editor_listItem {
	ListItem * next;
	ListItem * prev;
};

#define foreach_in_list(item, start) \
        for(item = start; item != NULL; item = item->next)
        
#define foreach_in_list_reverse(item, start) \
        for(item = start; item != NULL; item = item->prev)

int push_ListItem(ListItem ** items_start, ListItem ** items_end, ListItem * item);
ListItem * erase_ListItem(ListItem ** items_start, ListItem ** items_end, ListItem * item);
int push_ListItem_middle(ListItem * pos, ListItem * item);

#endif