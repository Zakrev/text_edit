#include "../headers/file.h"

static int find_ch(const char * str, size_t len, char ch){
        if(str == NULL)
                return 0;
        size_t i;
        for(i = 0; i < len; i++)
                if(str[i] == ch)
                        return 1;
        return 0;
}

static Line * read_line(int fd, const char * line_end, size_t line_end_len)
{
        /*
               Читает строку из файла
               Конец строки определяется набором символов в line_end
               Возвращает структуру, описывающую строку, либо NULL
        */
#if DBG_LVL >= 3
        PRINT("\tRead line: ");
#endif
        Line * line;
        size_t max_len;
        ssize_t offset = 0;
        int rc;
        
        line = malloc(sizeof(Line));
        if(line == NULL){
                PCERR("line = malloc");
                goto free_end;
        }
        line->data = malloc(sizeof(char) * MIN_LINE_ALLOC_LENGHT);
        if(line->data == NULL){
                PCERR("data = malloc");
                goto free_end;
        }
        
        line->len = 0;
        max_len = MIN_LINE_ALLOC_LENGHT;
        while(1){
                if( line->len == max_len ){
                        max_len += MIN_LINE_ALLOC_LENGHT;
                        line->data = realloc(line->data, sizeof(char) * max_len);
                        if(line->data == NULL){
                                PCERR("realloc %lld", (long long)max_len);
                                goto free_end;
                        }
                }
                rc = read(fd, line->data + offset, 1);
                if(1 == rc){
                        line->len += 1;
                        if(find_ch(line_end, line_end_len, line->data[offset])){
                                break;
                        }
                        offset += 1;
                } else {
                        if(rc == -1){
                                PCERR("read char");
                                goto free_end;
                        }
                        break;
                }
        }
        if(line->len == 0){
                goto free_end;
        }
#if DBG_LVL >= 3
        int dbg_i;
        for(dbg_i = 0; dbg_i <= line->len; dbg_i++)
                printf("%c", line->data[dbg_i]);
        PRINT("\n");
#endif
        return line;
        free_end:
                if(line != NULL)
                        free(line->data);
                free(line);
                return NULL;
}

static int create_lines_groups(FileText * ftext)
{
        /*
                Логически разбивает/переразбивает строки по группам
                В случае успеха возвращает 0
        */
        PRINT("\tCreate groups lines (size/count): ");
        ftext->group_size = 50;
        while((ftext->lines_count / ftext->group_size) > MAX_GROUP_COUNT)
                ftext->group_size *= 10;
        ftext->groups_count = ftext->lines_count / ftext->group_size;
        if(ftext->groups_count == 0)
                ftext->groups_count = 1;
        if(ftext->lines_group != NULL){
                ftext->lines_group = realloc(ftext->lines_group, sizeof(Line *) * ftext->groups_count);
                if(ftext->lines_group == NULL){
                        PCERR("group = realloc");
                        return -1;
                }
        } else {
                ftext->lines_group = malloc(sizeof(Line *) * ftext->groups_count);
                if(ftext->lines_group == NULL){
                        PCERR("group = malloc");
                        return -1;
                }
        }
        ftext->lines_group[0] = ftext->lines;
        Line * line = (Line *)ftext->lines->next;
        unsigned int i = 1, j = 1;
        while(line != NULL){
                i += 1;
                if(i == (ftext->group_size + 1)){
                        ftext->lines_group[j] = (ListItem *)line;
                        j += 1;
                        i = 1;
                }
                line = line->next;
        }
        PRINT("%lu / %lu\n", ftext->group_size, ftext->groups_count);
        return 0;
}

static int read_lines(FileText * ftext, const char * line_end, size_t line_end_len)
{
        /*
               Читает строки из файла в память, представляя их в структурах
               В случае успеха возвращает 0
        */
        PRINT("\tRead lines(rsize/fsize): ");
        if(ftext == NULL)
                return -1;
                
        if(lseek(ftext->fd, 0, SEEK_SET) == -1){
                PCERR("lseek to start of file: %s", ftext->path);
                return -1;
        }
                
        Line * line;
        ftext->esize = 0;
        ftext->lines_count = 0;
        while(1){
                /*Чтение строк*/
                line = read_line(ftext->fd, line_end, line_end_len);
                if(line == NULL)
                        break;
                if(0 == push_ListItem((ListItem **)&ftext->lines, (ListItem **)&ftext->lines_end, (ListItem *)line)){
                        ftext->lines_count += 1;
                        ftext->esize += line->len;
                } else
                        break;
        }
        PRINT("%lld / %lld\n", (long long)ftext->esize, (long long)ftext->size);
        if(ftext->lines_count == 0){
                return 0;
        }
        if(ftext->esize != ftext->size){
                PERR("Size of file not equal readed size: %s", ftext->path);
                return -1;
        }
        return create_lines_groups(ftext);
}

static void free_groups(FileText * ftext)
{
        ftext->groups_count = 0;
        ftext->group_size = 0;
        free(ftext->lines_group);
}

static void free_lines(FileText * ftext)
{
        Line * line, * ltmp;
        
        ftext->lines_count = 0;
        if(ftext->lines != NULL){
                line = (Line *)ftext->lines;
                while(line != NULL){
                        ltmp = line;
                        line = (Line *)line->next;
                        free(ltmp->data);
                        free(ltmp);
                }
                ftext->lines = NULL;
                ftext->lines_end = NULL;
                return;
        }
        if(ftext->lines_end != NULL){
                line = (Line *)ftext->lines_end;
                while(line != NULL){
                        ltmp = line;
                        line = (Line *)line->prev;
                        free(ltmp->data);
                        free(ltmp);
                }
                ftext->lines = NULL;
                ftext->lines_end = NULL;
        }
}

FileText * FileText_open_file(const char * path)
{
        PRINT("\tOpen file: ");
        if(path == NULL)
                return NULL;
        FileText * ftext;
        
        ftext = malloc(sizeof(FileText));
        if(ftext == NULL){
               PCERR("FileText malloc");
               return NULL;
        }
        PRINT("\n\tpath: %s\n", path);
        if(0 != stat(path, &ftext->fstat)){
               PCERR("Get stat of file: %s", path);
               goto free_return;
        }
        /*
               TODO: открывать файл в зависимости от количества ссылок на него и от прав доступа
        */
        memcpy(ftext->path, path, strlen(path));
        ftext->fd = open(path, O_RDWR);
        if(ftext->fd == -1){
               PCERR("Open file: %s", path);
               goto free_return;
        }
        ftext->size = lseek(ftext->fd, 0, SEEK_END);
        if(ftext->size == -1){
               PCERR("lseek file: %s", path);
               ftext->size = ftext->fstat.st_size;
        } else
               lseek(ftext->fd, 0, SEEK_SET);
        PRINT("\tsize: %lld\n", (long long)ftext->size);
        if(0 != read_lines(ftext, "\n", 1)){
               PERR("Read file: %s", path);
               /*
                       TODO: free_lines
               */
               free_lines(ftext);
               free_groups(ftext);
               goto free_return;
        }
        PRINT("\tlines: %lu\n", ftext->lines_count);
        
        Line * line;
        unsigned long lnum = 1;
        line = (Line *)ftext->lines;
        while(line != NULL){
                printf("%lu: ", lnum);
                int i;
                for(i = 0; i < line->len; i++)
                        printf("%c", line->data[i]);
                printf("\n");
                lnum += 1;
                line = (Line *)line->next;
        }
        
        return ftext;
        free_return:
               free(ftext);
               return NULL;
}