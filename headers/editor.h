#ifndef _EDITOR_H_
#define _EDITOR_H_

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