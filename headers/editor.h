#ifndef _EDITOR_H_
#define _EDITOR_H_

#include "file.h"

#define ListItem struct main_editor_listItem
ListItem {
	ListItem * next;
	ListItem * prev;
};

#define Line struct main_editor_line
Line {
	Line * next;
	Line * prev;
	
	unsigned long id;
};

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