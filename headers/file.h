#ifndef _FILE_H_
#define _FILE_H_

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "debug_print.h"
#include "listitem.h"

#define MAX_FILE_PATCH 255
#define MIN_LINE_ALLOC_LENGHT 10
#define MAX_GROUP_COUNT 200
#define MIN_GROUP_SIZE 50

//#define USE_PTHREADS

typedef struct main_editor_line Line;
struct main_editor_line {
	Line * next;
	Line * prev;
	
	size_t len;
	char * data;
};

typedef struct main_editor_file_position FilePos;
struct main_editor_file_position {
        ssize_t ch_idx;
        unsigned long ln_idx;
};

typedef struct main_editor_file_text FileText;
struct main_editor_file_text {
	int fd;
	char path[MAX_FILE_PATCH + 1];
	ListItem * lines;			//Строки
	ListItem * lines_end;			//Строки
	unsigned long lines_count;		//Кол-во строк
	ListItem ** lines_group;		//Группы строк
	unsigned long groups_count;		//Кол-во групп
	unsigned long group_size;		//Размер группы
	size_t size;				//Размер файла
	size_t esize;				//Размер файла, во время редактирования
	FilePos pos;                           //Позиция указателя в файле
	
	struct stat fstat;                     //Информация о файле
};

FileText * FileText_open_file(const char * path);
Line * get_Line(FileText * ftext, unsigned long idx);

#endif