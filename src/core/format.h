#ifndef _TE_FORMAT_H_
#define _TE_FORMAT_H_

/*
	Модуль 'format'

	Отвечает за форматированый вывод
*/

#include "listitem.h"

/*
	Тип, в котором будут храниться данные связанные с длинной символов
	Например позиция символа, длинна строки и т.д.
*/
typedef ssize_t letter_t;

enum teFormat_Outline_type {
	/*Начертание текста*/
	teFormat_Outline_type_DIRECT,		//прямое
	teFormat_Outline_type_ITALIC,		//курсивное
	teFormat_Outline_type_BOLD,		//полужирное
	teFormat_Outline_type_ITALIC_BOLD	//полужирный курсив
};

enum teFormat_Onderscore_type {
	/*Подчеркивание текста*/
	teFormat_Onderscore_type_DOWN,		//нижнее
	teFormat_Onderscore_type_UP,		//верхнее
	teFormat_Onderscore_type_MIDDLE		//зачеркнуть
};

typedef struct teFormat_Opt teFormat_Opt;
typedef struct teFormat_Opt {
	/*
		Описывает формат текста. Включая символ в этой позиции, ко всему следующему тексту должен применятся этот формат
	*/
	unsigned char[3] c_background;		//RGB цвет
	unsigned char[3] c_letter;		//RGB цвет
};

typedef struct teFormat_ teFormat;
typedef struct teFormat {
	/*
		Описывает
	*/
	/*ListItem*/
	tePrintMapPos * next;
	tePrintMapPos * prev;
	unsigned char list_item_type;
	
	/*Описывает позицию СИМВОЛА, к которому привязан формат*/
	teLine * line;
	unsigned int ln_idx;
	letter_t ch_idx;
	/*Тип структуры*/
	unsigned char type;

	/*Формат*/
	teFormat_Opt opt;
};

typedef struct teFormat teFormat;
typedef struct teFormat {
	/*
		Описывает формат текста
	*/
	/*ListItem*/
	tePrintMapPos * next;
	tePrintMapPos * prev;
	unsigned char list_item_type;
	
	/*Описывает позицию СИМВОЛА, к которому привязан формат*/
	teLine * line;
	unsigned int ln_idx;
	letter_t ch_idx;
	/*Тип структуры*/
	unsigned char type;

	/*Формат*/
	teFormat_Opt opt;
};

#endif