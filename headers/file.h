#ifndef _FILE_H_
#define _FILE_H_

#include <sys/stat.h>

#define MAX_FILE_PATCH 255

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

#define FileText struct main_editor_file_text
FileText {
	int fd;
	char patch[MAX_FILE_PATCH + 1];
	ListItem * lines;
	unsigned long lines_count;
	struct stat file_state;
};

#endif