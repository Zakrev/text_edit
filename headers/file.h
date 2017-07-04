#ifndef _FILE_H_
#define _FILE_H_

#include <sys/stat.h>
#include "print.h"
#include "listitem.h"

#define MAX_FILE_PATCH 255
#define MIN_LINE_ALLOC_LENGHT 10
#define MAX_GROUP_COUNT 200

#define Line struct main_editor_line
Line {
	Line * next;
	Line * prev;
	
	size_t len;
	char * data;
};

#define FileText struct main_editor_file_text
FileText {
	int fd;
	char patch[MAX_FILE_PATCH + 1];
	ListItem * lines;			//Строки
	ListItem * lines_end;			//Строки
	unsigned long lines_count;		//Кол-во строк
	ListItem ** lines_group;		//Группы строк
	unsigned long groups_count;		//Кол-во групп
	unsigned long group_size;		//Размер группы
	size_t size;				//Размер файла
	size_t esize;				//Размер файла, во время редактирования
};

#endif