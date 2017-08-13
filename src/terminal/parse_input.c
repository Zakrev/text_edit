#include <stdio.h>
#include <stdlib.h>

#include <termios.h> 
#include <unistd.h>

#include <string.h>

#include "../encoding/utf_8.h"
/*
	Отображает шестнадцатиричные коды вводимых символов
*/

//#define NEED_BITS

#ifdef NEED_BITS
static char * get_bits_static(unsigned char ch)
{
	static char bits[10] = "0000 0000";
	
	char i;
	char offset = 7;

	for(i = 0; i != 4; i++, offset--){
		sprintf(bits + i, "%d", (ch >> offset) & 0x1);
	}
	bits[4] = ' ';
	for(i = 5; i != 9; i++, offset--){
		sprintf(bits + i, "%d", (ch >> offset) & 0x1);
	}

	return bits;
}
#endif

static int parse_input(char byte)
{
	void pparse(char byte)
	{
		char pbuff[1024];
		sprintf(pbuff, 
		"0x%x "
#ifdef NEED_BITS
		"(%s) "
#endif
		, (unsigned char)byte
#ifdef NEED_BITS
		, get_bits_static(byte)
#endif
		);
		write(1, pbuff, strlen(pbuff));
	}
	void peol()
	{
		write(1, "\n", strlen("\n"));
	}
	
	peol();
	pparse(byte);
	switch(byte){
		case 0x1b: //esc
			if(0 >= read(0, &byte, 1))
				return -1;
			pparse(byte);
			switch(byte){
				case 0x5b: //[
					if(0 >= read(0, &byte, 1))
						return -1;
					pparse(byte);
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
		case 0xa: //ENTER
			break;
		case 0x3: //CTRL + C
			return 1;
		case 0x1a: //CTRL + Z
			break;
		default:
			switch(get_utf8_letter_size(byte)){
				case 2:
					if(0 >= read(0, &byte, 1))
						return -1;
					pparse(byte);
					break;
				case 3:
					if(0 >= read(0, &byte, 1))
						return -1;
					pparse(byte);
					if(0 >= read(0, &byte, 1))
						return -1;
					pparse(byte);
					break;
				case 4:
					if(0 >= read(0, &byte, 1))
						return -1;
					pparse(byte);
					if(0 >= read(0, &byte, 1))
						return -1;
					pparse(byte);
					if(0 >= read(0, &byte, 1))
						return -1;
					pparse(byte);
					break;
			}
	}
	return 0;
}

int main()
{
	struct termios tcurr, tdef;
	if(0 != tcgetattr(0, &tdef)){
		return 1;
	}
	tcurr = tdef;
	tcurr.c_lflag &= ~ICANON;	//отключить канонический режим, при котором ожидается ввод символа NL и т.п.
	tcurr.c_lflag &= ~ECHO;		//отключить вывод вводимых символов
	tcurr.c_lflag &= ~ISIG;		//отключить сигналы, такие как Ctrl + C и т.д.
	if(0 != tcsetattr(0, TCSANOW, &tcurr)){
		return 1;
	}

	while(1){
		int rd;
		char byte;

		rd = read(0, &byte, 1);
		if(rd > 0){
			if(0 != parse_input(byte))
				break;
		}
	}
	if(0 != tcsetattr(0, TCSANOW, &tdef)){
		return 1;
	}

	return 0;
}