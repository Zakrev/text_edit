#ifndef _LISTITEM_H_
#define _LISTITEM_H_

#define ListItem struct main_editor_listItem
ListItem {
	ListItem * next;
	ListItem * prev;
};

int push_ListItem(ListItem ** items_start, ListItem ** items_end, ListItem * item);
ListItem * erase_ListItem(ListItem ** items_start, ListItem ** items_end, ListItem * item);

#endif