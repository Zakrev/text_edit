#ifndef _EDITOR_H_
#define _EDITOR_H_

/*
	Модуль 'editor'
	
	Отвечает за редактирование текстового файла, поиск в файле и парсинг файла.
	На основе функций и структур из модуля 'file' предоставляет функции для 
	редактирования данных как символов.
*/

#include "file.h"

#define TEXT_EDIT_EDITOR_INIT_TABS_COUNT 100

typedef struct teTab teTab;
struct teTab {
	char file_path[256];
	teText file;
	tePos pos;
};

typedef struct teEditor teEditor;
struct teEditor {
	teTab tabs[TEXT_EDIT_EDITOR_INIT_TABS_COUNT];
};

#endif