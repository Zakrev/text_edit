#ifndef _FILE_H_
#define _FILE_H_

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "debug.h"
#include "listitem.h"

#define MAX_FILE_PATCH 255
#define MIN_LINE_ALLOC_LENGHT 10
#define MAX_GROUP_COUNT 200
#define MIN_GROUP_SIZE 50
#define MAX_EOL_CHS_LEN 1

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
	ssize_t len;            //длина строки
	char * data;            //данные строки
	ssize_t alloc;          //выделено памяти под строку (может быть больше длины)
};

typedef struct main_editor_file_position FilePos;
struct main_editor_file_position {
        FilePos * next;
        FilePos * prev;

        Line * line;            //Указатель на строку ln_idx
        ssize_t ch_idx;         //Позиция первого выделенного символа в строке
        unsigned long ln_idx;   //Номер строки
        ssize_t len;            //Количество выделенных символов
};

/*
        В файле хранятся две пустые строки (объекты Line: lines и lines_end).
        Они не удаляются и не создаются.
        Нужны только для хранения других строк.
        
        В циклах лучше проверять:
        if(line->type != main_editor_line_type_LINE)
                        continue;
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
	char eol_chs[MAX_EOL_CHS_LEN];          //Символы конца строки
	unsigned char eol_chs_len;              //Количество символов конца строки
	
	struct stat fstat;                      //Информация о файле
	
#ifdef DBG_ALLOC_MEM
        ssize_t alloc_mem;                      //Выделено памяти под структуры
#endif
};

/*
        Инициализирует структуру FileText_init
        eol_chs  - символы конца строки
        eol_chs_len - кол-во символов
*/
int FileText_init(FileText * ftext, const char * eol_chs, unsigned char eol_chs_len);

/*
        Открывает файл и представляет его содержимое в структупах
*/
FileText * FileText_open_file(const char * path);

/*
        Возвращает указатель на линию idx
        Либо NULL
*/
Line * get_Line(FileText * ftext, unsigned long idx);

/*
        Вставляет группу линий line в позицию pos
        Линии встанут перед pos
        Например:
        вставить        l1-l2-l3 в L2 из L1-L2-L3-L4
        получится       L1-l1-l2-l3-L2-L3-L4
        В случае успеха возвращает 0
        Изменяется размер файла
*/
int insert_Line_down(FileText * ftext, FilePos * pos, Line * line);

/*
        Вставляет группу линий line в позицию pos
        Линии встанут за pos
        Например:
        вставить        l1-l2-l3 в L2 из L1-L2-L3-L4
        получится       L1-L2-l1-l2-l3-L3-L4
        В случае успеха возвращает 0
        Изменяется размер файла
*/
int insert_Line_up(FileText * ftext, FilePos * pos, Line * line);

/*
        Разрезает группу линий в позициях pos
        В структуре pos должны быть заполнены поля:
                pos->line
                pos->ch_idx
                pos->ln_idx (можно 0)
        После разрезания текущей линии остается левая часть данных,
        создается новая линия послей текущей, с "правыми" данными текущей.
        Символ в позиции ch_idx является началом "правых" данных.
        Например:
        разрезать       l2-l3-l6 из l1-l2-l3-l4-l5-l6
        получится       l1-l2/1-l2/2-l3/1-l3/2-l4-l5-l6/1-l6/2
        В случае успеха возвращает 0
        Изменяется размер файла
*/
int cut_Line(FileText * ftext, FilePos * pos);

/*
        Функция заполняет структуру FilePos
        Если line == NULL, то ищет строку по ln_idx
        В случае успеха возвращает 0
        (FilePos * pos, unsigned long ln_idx, ssize_t ch_idx)
*/
int fill_FilePos(FileText * ftext, FilePos * pos, Line * line, unsigned long ln_idx, ssize_t ch_idx, ssize_t len);
#endif