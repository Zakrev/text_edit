#define DBG_LVL 2

#if DBG_LVL >= 1
#include <stdio.h>
static FILE * te_stdout = NULL;
#define DBG_STDOUT te_stdout
#define DBG_STDERR te_stdout
#endif

#include "../debug.h"
#include "term.h"

static char convert_RGB_to_terminal_static(unsigned char R, unsigned char G, unsigned char B, unsigned char is_back)
{
	/*
		Конвертирует цвет RGB в терминальный
	*/
	static char black[3] = TCT_BLACK;
	static char red[3] = TCT_RED;
	static char green[3] = TCT_GREEN;
	static char brown[3] = TCT_BROWN;
	static char blue[3] = TCT_BLUE;
	static char lilac[3] = TCT_LILAC;
	static char light_blue[3] = TCT_LIGHT_BLUE;
	static char white[3] = TCT_WHITE;

	static char bblack[3] = TCTB_BLACK;
	static char bred[3] = TCTB_RED;
	static char bgreen[3] = TCTB_GREEN;
	static char bbrown[3] = TCTB_BROWN;
	static char bblue[3] = TCTB_BLUE;
	static char blilac[3] = TCTB_LILAC;
	static char blight_blue[3] = TCTB_LIGHT_BLUE;
	static char bwhite[3] = TCTB_WHITE;

	if(!is_back){
		if(R == G == B == 0)
			return black;
		if(R == G == B)
			return white;
		if(G == B == 0)
			return red;
		if(R == B == 0)
			return green;
		if(R == G == 0)
			return blue;

		if(R >= 128 && G > 0 && B <= 128)
			return brown;
		if(R == G == 255 && B <= 128)
			return brown;

		if(R >= 128 && G <= 128 && B >= 128)
			return lilac;
		if(R == B == 255)
			return lilac;

		return light_blue;
	}
	if(R == G == B == 0)
		return bblack;
	if(R == G == B)
		return bwhite;
	if(G == B == 0)
		return bred;
	if(R == B == 0)
		return bgreen;
	if(R == G == 0)
		return bblue;

	if(R >= 128 && G > 0 && B <= 128)
		return bbrown;
	if(R == G == 255 && B <= 128)
		return bbrown;

	if(R >= 128 && G <= 128 && B >= 128)
		return blilac;
	if(R == B == 255)
		return blilac;

	return blight_blue;
}

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

static void set_win_caption_terminal(char * str, bytes_t len)
{
	/*
		Устанавливает заголовок окна терминала (эмулятора)
	*/
	PFUNC_START();
	char buff[512];
	bytes_t offset = 0;

	offset = sprintf(buff, "\x1b]2;");
	if(len > (512 - (offset + strlen("\x07")))){
		len = len - (len - (512 - (offset + strlen("\x07") + strlen("..."))));
		str[len] = '\0';
		offset += sprintf(buff + offset, "%s...", str);
	} else {
		offset += sprintf(buff + offset, "%s", str);
	}
	offset += sprintf(buff + offset, "\x07");

	write(1, buff, offset);
	PFUNC_END();
}

static int parse_input_terminal(teView * view, char byte)
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
			view->cmd = te_terminal_cmd_type_EXIT;
			return 1;
			break;
		case 0x7f: //BACKSPACE
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

static letter_t print_line_pos_terminal(teView * view, const unsigned x, const unsigned y, char * str, letter_t len, char * format)
{
	/*
		Печатает СИМВОЛЬНУЮ строку начиная с позиции (x, y)
		format		формат вывода
		Возвращает количество напечатаных символов
	*/
	PFUNC_START();
	if(view == NULL){
		PERR("ptr is NULL");
		return 0;
	}
	if(str == NULL){
		PERR("ptr is NULL");
		return 0;
	}

	bytes_t ret;
	if(view->x_max < len)
		len = view->x_max;
	set_pos_terminal(x, y);
	if(format != NULL){
		FORMAT(format);
		ret = write(1, str, convert_utf8_to_ascii_strlen(str, len));
		FORMAT(TE_TERMINAL_DEFAULT_FORMAT);
	} else {
		ret = write(1, str, convert_utf8_to_ascii_strlen(str, len));
	}
	PFUNC_END();

	return convert_ascii_to_utf8_strlen(str, ret);
}

static letter_t print_long_line_pos_terminal(teView * view, const unsigned x, const unsigned y, char * str, const letter_t len, const letter_t max_len, char * format)
{
	/*
		Печатает СИМВОЛЬНУЮ строку начиная с позиции (x, y)
		Если длина строки больше max_len, то строка обрезается
		Если длина строки меньше max_len, то дописываются заполнители TE_TERMINAL_VOID
		format		формат вывода
		Возвращает количество напечатаных символов
	*/
	PFUNC_START();
	if(view == NULL){
		PERR("ptr is NULL");
		return 0;
	}
	if(str == NULL){
		PERR("ptr is NULL");
		return 0;
	}

	letter_t ret = 0;
	int is_do;
	if(max_len < len){
		bytes_t tmp_len = convert_utf8_to_ascii_strlen(str, max_len / 2);
		is_do = 1;
		while(is_do){
			if(tmp_len < 0){
				PERR("unexpected EOL");
				return 0;
			}
			switch(get_utf8_letter_size(str[tmp_len])){
				case 1:
				case 2:
				case 3:
				case 4:
					tmp_len -= 1;
					is_do = 0;
					break;
				default:
					tmp_len -= 1;
			}
		}
		letter_t left_len = convert_ascii_to_utf8_strlen(str, tmp_len);
		bytes_t ascii_len = convert_utf8_to_ascii_strlen(str, len);
		tmp_len = convert_utf8_to_ascii_strlen(str, len - (left_len + (len - max_len) + 3));
		if(tmp_len < 0){
			PERR("unexpected right_len");
			return 0;
		}
		is_do = 1;
		while(is_do){
			if(tmp_len >= ascii_len){
				PERR("unexpected EOL");
				return 0;
			}
			switch(get_utf8_letter_size(str[ascii_len - tmp_len])){
				case 1:
				case 2:
				case 3:
				case 4:
					is_do = 0;
					break;
				default:
					tmp_len -= 1;
			}
		}
		letter_t right_len = convert_ascii_to_utf8_strlen(str, tmp_len);
		ret = print_line_pos_terminal(view, x, y, str, left_len, format);
		if(left_len != ret){
			PFUNC_END();
			return ret;
		}
		ret += print_line_pos_terminal(view, x + ret, y, "...", 3, format);
		if((left_len + 3) != ret){
			PFUNC_END();
			return ret;
		}
		ret += print_line_pos_terminal(view, x + ret, y, str + convert_utf8_to_ascii_strlen(str, (left_len + len - (left_len + right_len))), right_len, format);
	} else {
		ret = print_line_pos_terminal(view, x, y, str, len, format);
		if(ret < max_len){
			char buff[max_len];
			bytes_t fill_len = convert_utf8_to_ascii_strlen(str, max_len - ret);

			memset(buff, (int)TE_TERMINAL_VOID, fill_len);
			print_line_pos_terminal(view, x + ret, y, buff, max_len - ret, format);
		}
	}

	PFUNC_END();
	return len;
}

static int print_view_EDIT_FILE_header_line_2_terminal(teView * view)
{
	/*
		Печатает часть окна вида:

		(Расположение файла)	  (rwx/rwx/rwx)
	*/
	PFUNC_START();
	letter_t part_len = get_utf8_strlen(TE_TERMINAL_MENU_OPEN, NULL);
	char arights[14] = "(---/---/---)";

	int ret;
	letter_t fname_len;
	char * file_name = "Печатает часть окна видаПечатает часть окна видаПечатает часть окна видаПечатает часть окна видаПечатает часть окна вида";
	letter_t path_len;
	char * path_name = "Печатает часть окна видаПечатает часть окна видаПечатает часть окна видаПечатает часть окна видаПечатает часть окна вида";
	
	fname_len = get_utf8_strlen(file_name, NULL);
	path_len = get_utf8_strlen(path_name, NULL);

	/*Заголовок*/
	set_win_caption_terminal(file_name, fname_len);
	ret = print_long_line_pos_terminal(view, 1, 1, file_name, fname_len, view->x_max - part_len, NULL);
	if(ret <= 0){
		PERR("fault write");
		return -1;
	}
	ret = print_line_pos_terminal(view, (unsigned int)(view->x_max - part_len + 1), 1, TE_TERMINAL_MENU_OPEN, part_len, TE_TERMINAL_VIEW_1_HEADER_LETTER_COLOR);
	if(ret <= 0){
		PERR("fault write");
		return -1;
	}

	/*Расположение файла*/
	ret = print_long_line_pos_terminal(view, 1, 2, path_name, path_len, view->x_max - strlen(arights), TE_TERMINAL_VIEW_1_HEADER_LETTER_COLOR);
	if(ret <= 0){
		PERR("fault write");
		return -1;
	}

	/*Права доступа
		TODO: цвет в зависимоти от прав доступа TE_TERMINAL_ACCESS_RIGHTS либо TE_TERMINAL_VIEW_1_HEADER_LETTER_COLOR*/
	ret = print_line_pos_terminal(view, (unsigned int)(view->x_max - strlen(arights) + 1), 2, arights, strlen(arights), TE_TERMINAL_ACCESS_RIGHTS_RDONLY);
	if(ret <= 0){
		PERR("fault write");
		return -1;
	}

	PFUNC_END();
	return 0;
}

static int print_view_EDIT_FILE_header_line_1_terminal(teView * view, unsigned char info)
{
	/*
		Печатает часть окна вида:

		(Имя файла)		(UPDATE: ALT + U)(EDIT_ON: ALI + E)(Save: ALT + S)(Menu: ALT + M)
	*/
	PFUNC_START();
	letter_t part_len = 0;
	letter_t ret = 0;
	char arights[14] = "(---/---/---)";

	/*Находится в view*/
	letter_t fname_len;
	char * file_name = "Печатает часть окна видаПечатает часть окна видаПечатает часть окна видаПечатает часть окна видаПечатает часть окна вида";
	letter_t path_len;
	char * path_name = "Печатает часть окна видаПечатает часть окна видаПечатает часть окна видаПечатает часть окна видаПечатает часть окна вида";
	fname_len = get_utf8_strlen(file_name, NULL);
	path_len = get_utf8_strlen(path_name, NULL);
	/*Находится в view*/

	/*Подсказки*/
	if(info & TEINF_MENU){
		part_len = get_utf8_strlen(TE_TERMINAL_INF_MENU, NULL);
		if(view->x_max < part_len)
			return 0;
		ret += print_line_pos_terminal(view, (unsigned int)(view->x_max - part_len + 1), 1,
				TE_TERMINAL_INF_MENU, 
				part_len,
				TE_TERMINAL_VIEW_EDIT_FILE_HEADER_LETTER_COLOR);
		if(ret <= 0){
			PERR("fault write");
			return -1;
		}
	}
	if(info & TEINF_SAVE){
		part_len = get_utf8_strlen(TE_TERMINAL_INF_SAVE, NULL);
		if(view->x_max < part_len + ret)
			return 0;
		ret += print_line_pos_terminal(view, (unsigned int)(view->x_max - part_len + 1), 1,
				TE_TERMINAL_INF_SAVE, 
				part_len,
				TE_TERMINAL_VIEW_EDIT_FILE_HEADER_INF_SAVE);
		if(ret <= 0){
			PERR("fault write");
			return -1;
		}
	}

	/*Заголовок*/
	set_win_caption_terminal(file_name, fname_len);
	ret = print_long_line_pos_terminal(view, 1, 1, file_name, fname_len, view->x_max - part_len, NULL);
	if(ret <= 0){
		PERR("fault write");
		return -1;
	}
	

	/*Расположение файла*/
	ret = print_long_line_pos_terminal(view, 1, 2, path_name, path_len, view->x_max - strlen(arights), TE_TERMINAL_VIEW_EDIT_FILE_HEADER_LETTER_COLOR);
	if(ret <= 0){
		PERR("fault write");
		return -1;
	}

	/*Права доступа
		TODO: цвет в зависимоти от прав доступа TE_TERMINAL_ACCESS_RIGHTS либо TE_TERMINAL_VIEW_1_HEADER_LETTER_COLOR*/
	ret = print_line_pos_terminal(view, (unsigned int)(view->x_max - strlen(arights) + 1), 2, arights, strlen(arights), TE_TERMINAL_ACCESS_RIGHTS_RDONLY);
	if(ret <= 0){
		PERR("fault write");
		return -1;
	}

	PFUNC_END();
	return 0;
}

static int print_view_EDIT_FILE_terminal(teView * view)
{
	/*
		Печатает окно редактора
	*/
	if(view == NULL){
		PERR("ptr is NULL");
		return -1;
	}
	
	set_pos_terminal(0, 0);
	if(0 != print_view_EDIT_FILE_header_terminal(view))
		return -1;
	
	while(1){
		/*Обработка ввода*/
		int rd;
		char byte;
		rd = read(0, &byte, 1);
		if(rd > 0){
			if(0 != parse_input_terminal(view, byte))
				break;
		}
	}

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
	bzero(view, sizeof(teView));

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
	set_win_caption_terminal(" ", 1);
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

int run_editor_terminal(teView * view, int names_count, char ** names)
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
		switch(view->cmd){
			case te_terminal_cmd_type_EDIT_FILE:
				if(0 != print_view_EDIT_FILE_terminal(view))
					return -1;
				break;
			case te_terminal_cmd_type_EXIT:
				PFUNC_END();
				return 0;
			default:
				PERR("unexpected view type");
		}
	}
	return -1;
}