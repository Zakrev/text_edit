#include "file.h"

static int find_ch(char * str, size_t len, char ch){
	if(str == NULL)
		return 0;
	size_t i;
	for(i = 0; i < len; i++)
		if(str[i] == ch)
			return 1;
	return 0;
}

static Line * read_line(int fd, char * line_end, size_t line_end_len)
{
	/*Чтение и создание строки*/
	Line * line;
	size_t len_size;
	ssize_t offset = 0;
	
	line = malloc(sizeof(Line));
	if(line == NULL){
		PERR("line = malloc");
		perror("line = malloc");
		goto free_end;
	}
	len_size = MIN_LINE_ALLOC_LENGHT;
	line->data = malloc(sizeof(char) * MIN_LINE_ALLOC_LENGHT);
	if(line->data == NULL){
		PERR("data = malloc");
		perror("data = malloc");
		goto free_end;
	}
	
	if(lseek(fd, 0, SEEK_SET) == -1){
		PERR("lseek to start");
		perror("lseek to start");
		goto free_end;
	}
	
	line->len = 0;
	while(1){
		if(1 == read(fd, line->data + offset, 1)){
			line->len += 1;
			if(1 == find_ch(line_end, line_end_len, line->data + offset)){
				break;
			}
			offset += 1;
			if(line->len == len_size){
				line->data = realloc(line->data, sizeof(char) * (len_size + MIN_LINE_ALLOC_LENGHT));
				if(line->data == NULL){
					PERR("data = realloc");
					perror("data = realloc");
					goto free_end;
				}
			}
		}
	}
	if(line->len == 0){
		PINF("line->len == 0");
		goto free_end;
	}
	
	return line;
	free_end:
		if(line != NULL)
			free(line->data);
		free(line);
		return NULL;
}

static int create_lines_groups(FileText * ftext)
{
	ftext->group_size = 50;
	while((ftext->lines_count / ftext->group_size) > MAX_GROUP_COUNT)
		ftext->group_size *= 10;
	ftext->groups_count = ftext->lines_count / ftext->group_size;
	if(ftext->groups_count == 0)
		ftext->groups_count = 1;
	ftext->lines_group = malloc(sizeof(Line *) * ftext->groups_count);
	if(ftext->lines_group == NULL){
		PERR("group = malloc");
		perror("group = malloc");
		return ERROR;
	}
	ftext->lines_group[0] = ftext->lines;
	Line * line = ftext->lines->next;
	unsigned int i = 1, j = 1;
	while(line != NULL){
		i += 1;
		if(i == (ftext->group_size + 1)){
			ftext->lines_group[j] = line;
			j += 1;
			i = 1;
		}
		line = line->next;
	}
	return SUCESS;
}

static int resize_lines_groups(FileText * ftext)
{
	ftext->group_size = 50;
	while((ftext->lines_count / ftext->group_size) > MAX_GROUP_COUNT)
		ftext->group_size *= 10;
	ftext->groups_count = ftext->lines_count / ftext->group_size;
	if(ftext->groups_count == 0)
		ftext->groups_count = 1;
	ftext->lines_group = realloc(ftext->lines_group, sizeof(Line *) * ftext->groups_count);
	if(ftext->lines_group == NULL){
		PERR("group = realloc");
		perror("group = realloc");
		return ERROR;
	}
	ftext->lines_group[0] = ftext->lines;
	Line * line = ftext->lines->next;
	unsigned int i = 1, j = 1;
	while(line != NULL){
		i += 1;
		if(i == (ftext->group_size + 1)){
			ftext->lines_group[j] = line;
			j += 1;
			i = 1;
		}
		line = line->next;
	}
	return SUCESS;
}

static int read_lines(FileText * ftext, char * line_end, size_t line_end_len)
{
	/*Чтение и создание строк*/
	if(ftext == NULL)
		return 0;
	Line * line;
	ftext->esize = 0;
	ftext->lines_count = 0;
	while(1){
		/*Чтение строк*/
		line = read_line(ftext->fd, line_end, line_end_len);
		if(line == NULL)
			break;
		if(0 == push_ListItem((ListItem *)&ftext->lines, (ListItem *)&ftext->lines_end, (ListItem *)line)){
			ftext->lines_count += 1;
			ftext->esize += line->len;
		} else
			break;
	}
	if(ftext->lines_count == 0)
		return SUCESS;
	if(ftext->esize != ftext->size){
		PERR("Size of file not equal readed size");
		return ERROR;
	}
	return create_lines_groups(ftext);
}

static void close_file(FileText * ftext)
{
	
}

FileText * FileText_open_file(char * patch)
{
	if(patch == NULL)
		return NULL;
	
}