#define DBG_LVL 1

#if DBG_LVL >= 1
#include <stdio.h>
static FILE * te_stdout = NULL;
#define DBG_STDOUT te_stdout
#define DBG_STDERR te_stdout
#endif

#include "../debug.h"
#include "term.h"

static void set_pos_terminal(unsigned int x, unsigned int y)
{
	/*
		Устанавливает позицию курсора терминала
	*/
	char cmd[20];
	sprintf(cmd, "\x1b[%d;%dH", y, x);
	write(1, cmd, strlen(cmd));
}

static int get_pos_terminal(unsigned int * x, unsigned int * y)
{
	/*
		Определяет позицию курсора терминала в координатах
	*/
	write(1, "\x1b[6n", strlen("\x1b[6n"));
	if(2 != scanf("\x1b[%d;%dR", y, x)){
		PERR("not accept the answer");
		return -1;
	}

	return 0;
}

static void clear_terminal()
{
	/*
		Очищает терминал от курсора до конца окна
	*/
	write(1, "\x1b[J", strlen("\x1b[J"));
}

static int get_win_size_terminal(unsigned int * x, unsigned int * y)
{
	/*
		Определяет размер терминала в координатах
	*/
	write(1, "\x1b[1000;1000H\x1b[6n", strlen("\x1b[1000;1000H\x1b[6n"));
	if(2 != scanf("\x1b[%d;%dR", y, x)){
		PERR("not accept the answer");
		return -1;
	}

	return 0;
}

static void set_win_caption_terminal(char * str, ssize_t len)
{
	/*
		Устанавливает заголовок окна терминала (эмулятора)
	*/
	PFUNC_START();
	char buff[256];
	ssize_t offset = 0;

	offset = sprintf(buff, "\x1b]2;");
	if(len > (256 - (offset + strlen("\x07")))){
		len = len - (len - (256 - (offset + strlen("\x07") + strlen("..."))));
		str[len] = '\0';
		offset += sprintf(buff + offset, "%s...", str);
	} else {
		offset += sprintf(buff + offset, "%s", str);
	}
	offset += sprintf(buff + offset, "\x07");

	write(1, buff, offset);
	PFUNC_END();
}

static int parse_input_terminal(char byte)
{
	/*
		Парсит ввод терминала
	*/
	PFUNC_START();
	switch(byte){
		case 0x1b: //esc
			if(0 >= read(0, &byte, 1)){
				PERR("unexpected end of escape-sequence");
				return -1;
			}
			switch(byte){
				case 0x5b: //[
					if(0 >= read(0, &byte, 1)){
						PERR("unexpected end of escape-sequence");
						return -1;
					}
					switch(byte){
						case 0x44: //LEFT
							break;
						case 0x43: //RIGHT
							break;
						case 0x42: //DOWN
							break;
						case 0x41: //UP
							break;
					}
					break;
			}
			break;
		case 0x3: //CTRL + C
		case 0x1a: //CTRL + Z
			return 1;
			break;
		case 0xa: //ENTER
		default:
			switch(get_utf8_letter_size(byte)){
				/*FIXME: работает только для UTF-8*/
				case 1:
					write(1, &byte, 1);
					break;
				case 2:
					write(1, &byte, 1);
					if(0 >= read(0, &byte, 1))
						return -1;
					write(1, &byte, 1);
					break;
				case 3:
					write(1, &byte, 1);
					if(0 >= read(0, &byte, 1))
						return -1;
					write(1, &byte, 1);
					if(0 >= read(0, &byte, 1))
						return -1;
					write(1, &byte, 1);
					break;
				case 4:
					write(1, &byte, 1);
					if(0 >= read(0, &byte, 1))
						return -1;
					write(1, &byte, 1);
					if(0 >= read(0, &byte, 1))
						return -1;
					write(1, &byte, 1);
					if(0 >= read(0, &byte, 1))
						return -1;
					write(1, &byte, 1);
					break;
				default:
					PERR("undefined symbol byte length");
					return -1;
			}
			break;
	}
	PFUNC_END();
	return 0;
}

#if DBG_LVL >= 1
static int open_DBG_file()
{
	/*
		Создает файл для дебага
		FIXME: 
			1) Создавать файл в определенной дирректории
			2) Уникальное имя файла
	*/
	te_stdout = fopen(TE_DBG_FILE_NAME, "w");
	if(te_stdout == NULL){
		perror("open file for DBG");
		return -1;
	}

	return 0;
}

static void close_DBG_file()
{
	if(0 >= ftell(te_stdout)){
		fclose(te_stdout);
		remove(TE_DBG_FILE_NAME);
	} else
		fclose(te_stdout);
}
#endif

static int print_view_1_header_terminal(teView * view)
{
	/*
		Печатает часть окна вида:

		(Имя файла)		(Menu: ALT + M)
		---------------------------------------
	*/
	PFUNC_START();
	if(view == NULL){
		PERR("ptr is NULL");
		return -1;
	}

	ssize_t part_len = strlen(TE_TERMINAL_MENU_OPEN);
	ssize_t fname_len;
	char * file_name = "Hello world!Hello world!Hello world!Hello world!Hello world!Hello world!Hello world!Hello world!Hello world!";

	fname_len = strlen(file_name);
	set_win_caption_terminal(file_name, fname_len);
	if(fname_len > (view->x_max - part_len - 3 - 2)){
		fname_len = fname_len - (fname_len - (view->x_max - part_len - 3 - 2));
		set_pos_terminal((unsigned int)(view->x_max - part_len - 3 + 1), 1);
		write(1, "...)", 3);
	} else {
		set_pos_terminal((unsigned int)(view->x_max - part_len ), 1);
		write(1, ")", 1);
	}
	set_pos_terminal((unsigned int)(view->x_max - part_len + 1), 1);
	write(1, TE_TERMINAL_MENU_OPEN, part_len);
	set_pos_terminal(1, 1);
	write(1, "(", 1);
	write(1, file_name, fname_len);

	PFUNC_END();
	return 0;
}

int init_view_terminal(teView * view)
{
	/*
		Настраивает терминал для работы
		Создает и настраивает рабочие структуры
	*/
#if DBG_LVL >= 1
	if(0 != open_DBG_file())
		return -1;
#endif
	PFUNC_START();
	if(view == NULL){
		PERR("ptr is NULL");
		return -1;
	}

	if(0 != tcgetattr(0, &view->tbackup)){
		return 1;
	}
	view->tcurrent = view->tbackup;

	view->tcurrent.c_lflag &= ~ICANON;	//отключить канонический режим, при котором ожидается ввод символа NL и т.п.
	view->tcurrent.c_lflag &= ~ECHO;	//отключить вывод вводимых символов
	view->tcurrent.c_lflag &= ~ISIG;	//отключить сигналы, такие как Ctrl + C и т.д.
	if(0 != tcsetattr(0, TCSANOW, &view->tcurrent)){
		return 1;
	}
	/*Очистка перед первой отрисовкой интерфейса*/
	unsigned int px, py;
	if(0 != get_pos_terminal(&px, &py))
		return -1;
	if(0 != get_win_size_terminal(&view->x_max, &view->y_max))
		return -1;
	set_pos_terminal(px, py);
	py = view->y_max - 1;
	while(py > 0){
		py -= 1;
		write(1, "\x1b""D", strlen("\x1b""D"));
	}
	set_pos_terminal(0, 0);
	/*Первая отрисовка интерфейса*/
	print_view_1_header_terminal(view);

	PFUNC_END();
	return 0;
}

int close_view_terminal(teView * view)
{
	/*
		Завершение работы
	*/
	PFUNC_START();
	if(view == NULL){
		PERR("ptr is NULL");
		return -1;
	}
	
	set_pos_terminal(0, 0);
	clear_terminal();

	if(0 != tcsetattr(0, TCSANOW, &view->tbackup)){
		return 1;
	}
	PFUNC_END();
#if DBG_LVL >= 1
	close_DBG_file();
#endif
	return 0;
}

int run_editor_terminal(teView * view)
{
	/*
		Запуск редактора
	*/
	PFUNC_START();
	if(view == NULL){
		PERR("ptr is NULL");
		return -1;
	}

	while(1){
		int rd;
		char byte;
		rd = read(0, &byte, 1);
		if(rd > 0){
			if(0 != parse_input_terminal(byte))
				break;
		}
	}
	PFUNC_END();
	return 0;
}