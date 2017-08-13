#ifndef _TE_EDITOR_H_
#define _TE_EDITOR_H_

/*
	Модуль 'editor'

	Отвечает за редактирование текстового файла, поиск в файле и парсинг файла.
	На основе функций и структур из модуля 'file' предоставляет функции для 
	редактирования данных как символов.
*/

#include "file.h"
#include "listitem.h"

#define TEXT_EDIT_EDITOR_INIT_TABS_COUNT 100

/*
	Тип, в котором будут храниться данные связанные с длинной символов
	Например позиция символа, длинна строки и т.д.
*/
typedef ssize_t letter_t;

enum tePrintOptColor_type {
	tePrintOptColor_type_HEX,
	tePrintOptColor_type_RGB
};

typedef struct tePrintOptColor tePrintOptColor;
typedef struct tePrintOptColor {
	int type;
	int color;
};

typedef struct tePrintOpt tePrintOpt;
typedef struct tePrintOpt {
	tePrintOptColor c_background;
	tePrintOptColor c_letter;
};

typedef struct tePrintMapPos tePrintMapPos;
typedef struct tePrintMapPos {
	/*ListItem*/
	tePrintMapPos * next;
	tePrintMapPos * prev;
	unsigned char list_item_type;
	
	/*Позиция*/
	teLine * line;
	bytes_t ch_idx;

	/*Параметры*/
	tePrintOpt opt;
};

typedef struct tePrintMap tePrintMap;
struct tePrintMap {
	/*
		Параметры текста для вывода.
		Цвет, стиль, размер и т.д.
	*/

	/*Скомпилированные парсером*/
	tePrintMapPos * opt;

	/*Дефолтные*/
	tePrintMapPos default_opt;
};

typedef struct teTab teTab;
struct teTab {
	char file_path[256];
	teText file;
	tePos pos;
	tePrintMap map;
};

typedef struct teEditor teEditor;
struct teEditor {
	teTab * tabs[TEXT_EDIT_EDITOR_INIT_TABS_COUNT];
};

#endif