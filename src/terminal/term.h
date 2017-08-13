#ifndef __TE_TERMINAL_VIEW__
#define __TE_TERMINAL_VIEW__

#include <stdio.h>
#include <stdlib.h>

#include <termios.h> 
#include <unistd.h>

#include <string.h>

//#include "../core/editor.h"
//#include "../core/file.h"
#include "../encoding/utf_8.h"

#define TE_DBG_FILE_NAME "te_terminal_debug.txt"

typedef struct teView teView;
struct teView {
	struct termios tcurrent;
	struct termios tbackup;
	unsigned int x_max;
	unsigned int y_max;
	//teEditor editor;
};

/*
	Настраивает терминал для работы
	Создает и настраивает рабочие структуры
*/
int init_view_terminal(teView * view);

/*
	Завершение работы
*/
int close_view_terminal(teView * view);

/*
	Запуск редактора
*/
int run_editor_terminal(teView * view);

#endif