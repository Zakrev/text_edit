#define DBG_LVL 1
#include "../headers/debug.h"
#include "../headers/listitem.h"

int insert_ListItem_offset_down(ListItem * pos, ListItem * item)
{
	/*
		Вставляет item в позицию pos списка
		Элемент pos сдвигается "под" item
		Возвращает 0 в случае успеха
	*/
	if(pos == NULL){
		PERR("ptr is NULL");
		return -1;
	}
	if(item == NULL){
		PERR("ptr is NULL");
		return -1;
	}
	if(pos->prev != NULL){
		pos->prev->next = item;
	}
	item->prev = pos->prev;
	pos->prev = item;
	item->next = pos;
	
	return 0;
}

int insert_ListItem_offset_up(ListItem * pos, ListItem * item)
{
	/*
		Вставляет item в позицию pos списка
		Элемент item сдвигается "под" pos
		Возвращает 0 в случае успеха
	*/
	if(pos == NULL){
		PERR("ptr is NULL");
		return -1;
	}
	if(item == NULL){
		PERR("ptr is NULL");
		return -1;
	}
	if(pos->next != NULL){
		pos->next->prev = item;
	}
	item->next = pos->next;
	pos->next = item;
	item->prev = pos;
	
	return 0;
}

int erase_ListItem(ListItem * item)
{
	/*
		Удаляет item из списка
		в котором он находится
	*/
	if(item == NULL){
		PERR("ptr is NULL");
		return -1;
	}
	if(item->list_item_type == list_item_type_NOT_ERASE){
		PERR("item type: list_item_type_NOT_ERASE");
		return -1;
	}
	if(item->next != NULL){
		item->next->prev = item->prev;
	}
	if(item->prev != NULL){
		item->prev->next = item->next;
	}
	item->prev = NULL;
	item->next = NULL;
	
	return 0;
}