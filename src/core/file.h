#ifndef _FILE_H_
#define _FILE_H_

/*
	Модуль 'file'
	
	Отвечает за представление файла в памяти и редактирование данных как байт.
	Открывает файл -> представляет его в памяти в виде отдельных строк ->
		позволяет редактировать данные как байты -> записывает изменения в (другой) файл -> 
			освобождает память.
*/

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include "listitem.h"

#define MAX_FILE_PATCH 255			//длина полного пути файла
#define MIN_LINE_ALLOC_LENGHT 10		//минимальный размер выделяемой памяти для строки
#define MAX_LINE_LEN_TO_ALLOC_ALIGNMENT 100	//максимальный размер данных, до которого будет выполняться выравнивание памяти для строк
#define MAX_GROUP_COUNT 200			//максимальное кол-во групп строк
#define MIN_GROUP_SIZE 50			//минимальный размер группы строк
#define MAX_EOL_CHS_LEN 5			//максимальная длина "символа" конца строки

//#define FILEOPT_USE_PTHREADS

/*
	Тип, в котором будут храниться данные связанные с длинной байт
	Например позиция байта, длинна строки и т.д.
*/
typedef ssize_t bytes_t;

enum main_editor_line_type {
	/*Строка с данными*/
	main_editor_line_type_LINE,
	
	/*Неудаляемые строки*/
	main_editor_line_type_LINE_0,
	main_editor_line_type_LINE_END
};

typedef struct main_editor_line Line;
struct main_editor_line {
	/*ListItem*/
	Line * next;
	Line * prev;
	unsigned char list_item_type;
	
	/*Строка*/
	unsigned char type;
	bytes_t len;
	unsigned char * data;
	bytes_t alloc;
};

typedef struct main_editor_file_position FilePos;
struct main_editor_file_position {
	/*ListItem*/
	FilePos * next;
	FilePos * prev;
	unsigned char list_item_type;
	
	/*Позиция*/
	Line * line;
	bytes_t ch_idx;
	unsigned int ln_idx;
	bytes_t len;
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
	/*Строки*/
	Line lines;
	Line lines_end;
	unsigned int lines_count;
	unsigned char eol_chs[MAX_EOL_CHS_LEN];
	unsigned char eol_chs_len;
	
	/*Группы строк*/
	Line ** lines_group;
	unsigned char groups_count;
	unsigned short group_size;
	
	/*Файл*/
	bytes_t esize;
#ifdef DBG_ALLOC_MEM
	bytes_t alloc_mem;
#endif
};

/*
	Инициализирует структуру FileText_init
	eol_chs	- байты конца строки
	eol_chs_len - кол-во байт
	Возвращает:
		OK	0
		ERR	-1
*/
char init_FileText(FileText * ftext, const unsigned char * eol_chs, unsigned char eol_chs_len);

/*
	Открывает файл path и заполняет структуру ftext
	Возвращает:
		OK	0
		ERR	-1
*/
char read_from_file_FileText(FileText * ftext, const char * path);

/*
	Функция записывает строки в файл
	Создает новый файл, либо
	полностью перезаписывает существующий
	Возвращает:
		OK	0
		ERR	-1
*/
char write_to_file_FileText(FileText * ftext, const char * path);

/*
	Освобождает память от структур
	Возвращает:
		OK	0
		ERR	-1
*/
char close_file_FileText(FileText * ftext);

/*
	Находит строку по номеру (1 .. кол-во строк)
	Возвращает:
		OK	Указатель на строку
		ERR	NULL
*/
Line * get_line_by_idx_FileText(FileText * ftext, unsigned int idx);

/*
	Вставляет группу линий line в позицию pos
	В FilePos должны быть заполнены:
		line
	Линии встанут перед pos
	Например:
	вставить	l1-l2-l3 в L2 из L1-L2-L3-L4
	получится	L1-l1-l2-l3-L2-L3-L4
	Изменяется размер файла
	Возвращает:
		OK	0
		ERR	-1
*/
char insert_lines_by_pos_down_FileText(FileText * ftext, FilePos * pos, Line * line);

/*
	Вставляет группу линий line в позицию pos
	В FilePos должны быть заполнены:
		line
	Линии встанут за pos
	Например:
	вставить	l1-l2-l3 в L2 из L1-L2-L3-L4
	получится	L1-L2-l1-l2-l3-L3-L4
	Изменяется размер файла
	Возвращает:
		OK	0
		ERR	-1
*/
char insert_lines_by_pos_up_FileText(FileText * ftext, FilePos * pos, Line * line);

/*
	Разрезает группу линий в позициях pos
	В структуре pos должны быть заполнены поля:
		pos->line
		pos->ch_idx
	После разрезания текущей линии остается левая часть данных,
	создается новая линия послей текущей, с "правыми" данными текущей.
	Символ в позиции ch_idx является началом "правых" данных.
	Например:
	разрезать	l2-l3-l6 из l1-l2-l3-l4-l5-l6
	получится	l1-l2/1-l2/2-l3/1-l3/2-l4-l5-l6/1-l6/2
	Изменяется размер файла
	Возвращает:
		OK	0
		ERR	-1
*/
char cut_lines_by_pos_FileText(FileText * ftext, FilePos * pos);

/*
	Функция заполняет структуру FilePos
	Если line == NULL, то ищет строку по ln_idx
	Возвращает:
		OK	0
		ERR	-1
*/
char fill_pos_FileText(FileText * ftext, FilePos * pos, Line * line, unsigned int ln_idx, bytes_t ch_idx, bytes_t len);

/*
	Функция редактирует строки
	pos - позиция символа, с которого начнется редактирование
	В структуре pos должны быть заполнены поля:
		pos->line
		pos->ch_idx
		pos->len
	pos->len - длина заменяемых данных, данные начиная с pos (включая этот символ) и заканчивая pos->len-1 будут заменены
		при pos->len == 0, данные data будут записаны перед символом pos
	data - добавляемые данные
		при data == NULL, данные начиная с pos (включая этот символ) и заканчивая pos->len-1 будут удалены
	data_len - размер добавляемых данных
		при data_len == 0, данные начиная с pos (включая этот символ) и заканчивая pos->len-1 будут удалены
	
	Изменяется размер файла
	Изменяется mem alloc
	Возвращает:
		OK	0
		ERR	-1
	TODO:
		удалить созданные строки в случае ошибки
*/
char edit_lines_by_pos_FileText(FileText * ftext, FilePos * pos, const unsigned char * data, bytes_t data_len);

/*
	Функция копирует байты в буфер
	pos - позиция символа, с которого начнется копирование (включая символ pos->ch_idx)
	В структуре pos должны быть заполнены поля:
		pos->line
		pos->ch_idx
		pos->len
	pos->len - размер копируемых данных
	data - буфер назначения
	data_len - размер буфера
	
	Возвращает:
		OK	кол-во записанных символов
		ERR	-1
*/
bytes_t get_data_by_pos_FileText(FileText * ftext, FilePos * pos, unsigned char * data, bytes_t data_len);
#endif