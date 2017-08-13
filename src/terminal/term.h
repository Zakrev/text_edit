#ifndef __TE_TERMINAL_VIEW__
#define __TE_TERMINAL_VIEW__

#include <stdio.h>
#include <stdlib.h>

#include <termios.h> 
#include <unistd.h>

#include <string.h>

#include "../core/editor.h"
#include "../core/file.h"
#include "../encoding/utf_8.h"

typedef ssize_t bytes_t;
typedef ssize_t letter_t;

#define TE_DBG_FILE_NAME "te_terminal_debug.txt"

/*Типы подсказок*/
#define TEINF_MENU 0x1
#define TEINF_SAVE 0x2
#define TEINF_EDIT 0x4
#define TEINF_UPDATE 0x8

enum te_terminal_cmd_type {
	te_terminal_cmd_type_UNDEF,
	
	/*Окна программы*/
	te_terminal_cmd_type_EDIT_FILE,		/*ALT + E	Редактор файла*/
	te_terminal_cmd_type_MENU,		/*ALT + M	Меню программы, пункты меню и ALT + H*/
	te_terminal_cmd_type_SAVE_FILE_AS,	/*Из меню	Назначить имя файлу и расположение, после сохранить*/
	te_terminal_cmd_type_OPEN_FILE_AS,	/*Из меню	Список файлов, открыть файл*/
	te_terminal_cmd_type_SEARCH,		/*ALT + F	Поиск по файлу(-ам)*/
	te_terminal_cmd_type_HELP,		/*ALT + H	Помощь по комбинациям*/
	te_terminal_cmd_type_OPTION,		/*Из меню	Настройки программы*/
	te_terminal_cmd_type_TABS, 		/*ALT + T	Открывается список вкладок, выбор по номеру в списке*/

	/*Действия*/
	te_terminal_cmd_type_NOTING,		/*Ничего не делать*/
	te_terminal_cmd_type_EDIT_ADD,		/*		Байт данных*/
	te_terminal_cmd_type_EDIT_ERASE_BACK,	/*Backspace	Удалить данные*/
	te_terminal_cmd_type_EDIT_ERASE_FRONT,	/*Del		Удалить данные*/
	te_terminal_cmd_type_EXIT,		/*CTRL + C	Закрыть программу*/
	te_terminal_cmd_type_SELECT,		/*ALT + S	Начать/завершить выделение текста*/
	te_terminal_cmd_type_OPEN_FILE,		/*ALT + O	Открыть файл*/
	te_terminal_cmd_type_CLOSE_FILE,	/*ALT + C	Закрыть файл*/
	te_terminal_cmd_type_SAVE_FILE,		/*ALT + W	Записать изменения в файл*/
	te_terminal_cmd_type_ARROW_UP,		/*		Вверх*/
	te_terminal_cmd_type_ARROW_DOWN,	/*		Вниз*/
	te_terminal_cmd_type_ARROW_RIGHT,	/*		Вправо*/
	te_terminal_cmd_type_ARROW_LEFT,	/*		Влево*/
	te_terminal_cmd_type_GOTO,		/*ALT + G	Перейти к строке и символу*/
	te_terminal_cmd_type_RESIZE,		/*ALT + R	Перерисовать окно*/
};

typedef struct teView teView;
struct teView {
	struct termios tcurrent;
	struct termios tbackup;
	unsigned int x_max;
	unsigned int y_max;
	unsigned int cmd;
	teEditor editor;
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
int run_editor_terminal(teView * view, int names_count, char ** names);

/*
	Параметры вывода
*/
#define TE_TERMINAL_SET_FORMAT(format)\
		write(1, format, strlen(format))
#define FORMAT(format)\
		TE_TERMINAL_SET_FORMAT(format)

/*Символы*/
#define TE_TERMINAL_BOARD_HORIZONTAL '-'
#define TE_TERMINAL_BOARD_VERTICAL '|'
#define TE_TERMINAL_VOID ' '

/*Цвета*/
#define TE_TERMINAL_DEFAULT_FORMAT "\x1b[0m"
#define TE_TERMINAL_VIEW_EDIT_FILE_HEADER_LETTER_COLOR "\x1b[1m\x1b[44;37m"
#define TE_TERMINAL_VIEW_EDIT_FILE_HEADER_INF_SAVE "\x1b[1m\x1b[42;37m"
#define TE_TERMINAL_VIEW_EDIT_FILE_HEADER_INF_UPDATE "\x1b[1m\x1b[43;37m"
#define TE_TERMINAL_ACCESS_RIGHTS_RDONLY "\x1b[1m\x1b[41;37m"
#define TE_TERMINAL_ACCESS_RIGHTS_EXE "\x1b[1m\x1b[42;37m"

/*Текст*/
#define TE_TERMINAL_INF_MENU "(Меню: ALT + M)"
#define TE_TERMINAL_INF_SAVE "(Записать: ALT + W)"
#define TE_TERMINAL_INF_EDIT "(Изменить: ALT + E)"
#define TE_TERMINAL_INF_UPDATE "(Обновить: ALT + U)"

#endif