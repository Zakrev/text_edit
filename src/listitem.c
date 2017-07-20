#include "../headers/listitem.h"

int push_ListItem_middle(ListItem * pos, ListItem * item)
{
        /*
                Вставляет item в позицию pos списка
                Вставка происходит только в середину, т.е node-
                Элемент pos сдвигается "под" item
                Возвращает 0 в случае успеха
        */
        if(item == NULL || pos == NULL || pos->prev == NULL || pos->next == NULL){
                PERR("NULL pointer: %p %p %p %p", item, pos, pos->prev, pos->next);
                return -1;
        }
        pos->prev->next = item;
        item->prev = pos->prev;
        item->next = pos;
        pos->prev = item;
        
        return 0;
}

int push_ListItem(ListItem ** items_start, ListItem ** items_end, ListItem * item)
{
        if(item == NULL || items_start == NULL || items_end == NULL){
                PERR("NULL pointer: %p %p %p", item, items_start, items_end);
                return -1;
        }
                
        item->prev = NULL;
        item->next = NULL;
        if(*items_start == NULL){
                *items_start = item;
                *items_end = item;
                return 0;
        }
        if(*items_end == NULL){
                PERR("NULL pointer");
                return 0;
        }
        (*items_end)->next = item;
        item->prev = *items_end;
        *items_end = item;
        
        return 0;
}

ListItem * erase_ListItem(ListItem ** items_start, ListItem ** items_end, ListItem * item)
{
        if(items_start == NULL || items_end == NULL){
                PERR("NULL pointer: %p %p", items_start, items_end);
                return item;
        }
        
        if(*items_end == NULL && *items_end == NULL)
                return item;
        if(*items_end == NULL){
                PERR("NULL pointer");
                return item;
        }
        if(item == NULL){
                item = *items_end;
        }
        if(*items_start == *items_end){
                *items_start = NULL;
                *items_end = NULL;
        } else {
                if(item == *items_end){
                        *items_end = item->prev;
                        item->prev->next = NULL;
                } else if(item == *items_start){
                                *items_start = item->next;
                                item->next->prev = NULL;
                        } else {
                                item->next->prev = item->prev;
                                item->prev->next = item->next;
                        }
        }
        
        item->prev = NULL;
        item->next = NULL;
        return item;
}