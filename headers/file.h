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

enum main_editor_line_type {
        main_editor_line_type_LINE,
        main_editor_line_type_LINE_0,
        main_editor_line_type_LINE_END,
        main_editor_line_type_unexpected
};

typedef struct main_editor_line Line;
struct main_editor_line {
	Line * next;
	Line * prev;
	
	unsigned int type;
	ssize_t len;
	char * data;
};

typedef struct main_editor_file_position FilePos;
struct main_editor_file_position {
        ssize_t ch_idx;
        unsigned long ln_idx;
};

/*
        В файле хранятся две пустые строки (объекты Line: lines и lines_end).
        Они не удаляются и не создаются.
        Нужны только для хранения других строк.
*/

typedef struct main_editor_file_text FileText;
struct main_editor_file_text {
	int fd;
	char path[MAX_FILE_PATCH + 1];
	Line lines;			        //Строки
	Line lines_end;	                        //Строки
	unsigned long lines_count;		//Кол-во строк
	Line ** lines_group;                    //Группы строк
	unsigned long groups_count;		//Кол-во групп
	unsigned long group_size;		//Размер группы
	ssize_t size;				//Размер файла
	ssize_t esize;				//Размер файла, во время редактирования
	FilePos pos;                            //Позиция указателя в файле
	
	struct stat fstat;                      //Информация о файле
};

int FileText_init(FileText * ftext);
FileText * FileText_open_file(const char * path);
Line * get_Line(FileText * ftext, unsigned long idx);

#endif