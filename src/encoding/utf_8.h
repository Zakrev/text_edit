#ifndef __TE_UTF_8_H__
#define __TE_UTF_8_H__

/*
	Определяет размер символа, по первому байту
	Возвращает размер символа, либо 0
*/
char get_utf8_letter_size(unsigned char ch);

#endif