#ifndef _EDITOR_H_
#define _EDITOR_H_

/*
        Модуль 'editor'
        
        Отвечает за редактирование текстового файла, поиск в файле и парсинг файла.
        На основе функций и структур из модуля 'file' предоставляет функции для 
        редактирования данных как символов.
*/

#include "file.h"

#define File struct main_editor_file
File {
	File * next;
	File * prev;
	
	FileText text;
};

#define Editor struct main_editor_data
Editor {
	ListItem * files;
};

#endif