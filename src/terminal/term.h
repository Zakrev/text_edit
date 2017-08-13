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

typedef ssize_t bytes_t;
typedef ssize_t letter_t;

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

/*
	Параметры вывода
*/
#define TE_TERMINAL_SET_FORMAT(format)\
		write(1, format, strlen(format))
#define FORMAT(format) \
		TE_TERMINAL_SET_FORMAT(format)

/*Символы*/
#define TE_TERMINAL_BOARD_HORIZONTAL '-'
#define TE_TERMINAL_BOARD_VERTICAL '|'
#define TE_TERMINAL_VOID ' '
/*Цвета*/
#define TE_TERMINAL_DEFAULT_FORMAT "\x1b[0m"
#define TE_TERMINAL_VIEW_1_HEADER_LETTER_COLOR "\x1b[1m\x1b[44;37m"
#define TE_TERMINAL_ACCESS_RIGHTS_RDONLY "\x1b[1m\x1b[41;37m"
#define TE_TERMINAL_ACCESS_RIGHTS_EXE "\x1b[1m\x1b[42;37m"
/*Текст*/
#define TE_TERMINAL_MENU_OPEN "(Меню: ALT + M)"

#endif