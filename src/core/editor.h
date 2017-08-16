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

#define TEXT_EDIT_EDITOR_INIT_TABS_COUNT 20

/*
	Тип, в котором будут храниться данные связанные с длинной символов
	Например позиция символа, длинна строки и т.д.
*/
typedef ssize_t letter_t;

typedef struct teTab teTab;
struct teTab {
	char file_path[256];
	teText file;
	tePos pos;
};

typedef struct teEditor teEditor;
struct teEditor {
	teTab * tabs[TEXT_EDIT_EDITOR_INIT_TABS_COUNT];
};

#endif