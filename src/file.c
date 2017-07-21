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
        line->type = main_editor_line_type_LINE;
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

static int create_lines_groups(FileText * ftext, unsigned long idx, unsigned long lines_old_count)
{
        /*
                Логически разбивает строки по группам
                Разбиение начинается с строки idx
                lines_old_count - старое количество строк
                В случае успеха возвращает 0
        */
        PRINT("\tCreate groups lines (size/count): ");
        
        if(ftext->lines_count == 0){
                if(ftext->lines_group != NULL){
                        free(ftext->lines_group);
                }
                ftext->groups_count = 0;
                ftext->group_size = 0;
                PRINT("0 / 0\n");
                return 0;
        }
        if(lines_old_count == ftext->lines_count){
                PRINT("lines count equal\n");
                return 0;
        }
        unsigned long group_old_size = ftext->group_size;
        unsigned long group_old_count = ftext->groups_count;
        
        ftext->group_size = MIN_GROUP_SIZE;
        while((ftext->lines_count / ftext->group_size) > MAX_GROUP_COUNT)
                ftext->group_size *= 10;
        ftext->groups_count = (ftext->lines_count - 1) / ftext->group_size;
        ftext->groups_count += 1;
        if(ftext->group_size != group_old_size || ftext->groups_count != group_old_count){
                if(ftext->lines_group != NULL){
                        ftext->lines_group = realloc(ftext->lines_group, sizeof(Line *) * ftext->groups_count);
                        if(ftext->lines_group == NULL){
                                PCERR("group = realloc");
                                return -1;
                        }
                        if(ftext->group_size != group_old_size){
                                bzero(ftext->lines_group, ftext->groups_count);
                                group_old_count = ftext->groups_count;
                        } else if(ftext->groups_count > group_old_count){
                                bzero(ftext->lines_group + group_old_count, ftext->groups_count - group_old_count);
                        }
                } else {
                        ftext->lines_group = malloc(sizeof(Line *) * ftext->groups_count);
                        if(ftext->lines_group == NULL){
                                PCERR("group = malloc");
                                return -1;
                        }
                        bzero(ftext->lines_group, ftext->groups_count);
                }
        }
        unsigned long gr_idx;
        Line * line = NULL;
        if(idx > 1){
                gr_idx = idx / ftext->group_size;
                if( (gr_idx * ftext->group_size) < idx)
                                gr_idx += 1;
        } else {
                gr_idx = 0;
        }
        if(ftext->lines_count > lines_old_count){
                for(; gr_idx < ftext->groups_count; gr_idx++){
                        /*Обновляем ссылки на строки*/
                        if(ftext->lines_group[gr_idx] == 0)
                                break;
                        unsigned long diff = ftext->lines_count - lines_old_count;
                        foreach_in_list(line, (Line *)ftext->lines_group[gr_idx]){
                                if(line->type != main_editor_line_type_LINE)
                                        continue;
                                if(diff == 0)
                                        break;
                                diff -= 1;
                        }
                        if(line == NULL){
                                PERR("line is NULL");
                                return -1;
                        }
                        ftext->lines_group[gr_idx] = (ListItem *)line;
                }
        } else if(ftext->lines_count < lines_old_count){
                for(; gr_idx < ftext->groups_count; gr_idx++){
                        /*Обновляем ссылки на строки*/
                        if(ftext->lines_group[gr_idx] == 0)
                                break;
                        unsigned long diff = lines_old_count - ftext->lines_count;
                        foreach_in_list_reverse(line, (Line *)ftext->lines_group[gr_idx]){
                                if(line->type != main_editor_line_type_LINE)
                                        continue;
                                if(diff == 0)
                                        break;
                                diff -= 1;
                        }
                        if(line == NULL){
                                PERR("line is NULL");
                                return -1;
                        }
                        ftext->lines_group[gr_idx] = (ListItem *)line;
                }
        }
#ifdef USE_PTHREADS
        pthreads!
#else
        Line * start;
        unsigned long ln_idx;
        unsigned long ln_idx_cur;
        if(line != NULL){
                start = line;
        } else {
                start = (Line *)ftext->lines.next;
        }
        if(start->type != main_editor_line_type_LINE){
                PERR("not LINE type");
                return -1;
        }
        ln_idx = gr_idx * ftext->group_size;
        if(ln_idx == 0){
                ftext->lines_group[gr_idx] = (ListItem *)start;
                ln_idx = 1;
                gr_idx = 1;
                start = start->next;
        }
        ln_idx_cur = gr_idx * ftext->group_size;
        foreach_in_list(line, start){
                if(line->type != main_editor_line_type_LINE)
                        continue;
                if(gr_idx >= ftext->groups_count)
                        break;
                if(ln_idx == ln_idx_cur){
                        ftext->lines_group[gr_idx] = (ListItem *)line;
                        gr_idx += 1;
                        ln_idx_cur = gr_idx * ftext->group_size;
                }
                ln_idx += 1;
        }
#endif
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
                if(0 == insert_ListItem_offset_down((ListItem *)&ftext->lines_end, (ListItem *)line)){
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
        return create_lines_groups(ftext, 0, 0);
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
        ltmp = NULL;
        foreach_in_list(line, (Line *)ftext->lines.next){
                if(ltmp != NULL){
                        erase_ListItem((ListItem *)ltmp);
                        free(ltmp->data);
                        free(ltmp);
                        ltmp = NULL;
                }
                if(line->type != main_editor_line_type_LINE)
                        continue;
                ltmp = line;
        }
}

int FileText_init(FileText * ftext)
{
        if(ftext == NULL){
                PERR("ptr is NULL");
                return -1;
        }
        bzero(ftext, sizeof(FileText));
        ((Line *)&ftext->lines)->type = main_editor_line_type_LINE_0;
        ftext->lines.next = &ftext->lines_end;
        ((Line *)&ftext->lines_end)->type = main_editor_line_type_LINE_END;
        ftext->lines.prev = &ftext->lines;
        
        return 0;
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
        FileText_init(ftext);
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
        
        unsigned long gridx;
        unsigned long lnum = 1;
        unsigned long grsize;
        Line * line;
        for(gridx = 0; gridx < ftext->groups_count; gridx++){
                printf("group: %ld\n", gridx);
                grsize = ftext->group_size;
                foreach_in_list(line, (Line *)ftext->lines_group[gridx]){
                        printf("%lu: ", lnum);
                        int i;
                        for(i = 0; i < line->len; i++)
                                printf("%c", line->data[i]);
                        lnum += 1;
                        if(--grsize == 0 && (gridx + 1) != ftext->groups_count)
                                break;
                }
        }
        
        return ftext;
        free_return:
               free(ftext);
               return NULL;
}

Line * get_Line(FileText * ftext, unsigned long idx)
{
        /*
                Возвращает указатель на линию idx
                Либо NULL
        */
        PRINT("get Line: %ld\n", idx);
        
        Line * line = NULL;
        unsigned long gr_idx;
        unsigned long tmp;
        
        if(idx == 0)
                idx = 1;
        gr_idx = idx / ftext->group_size;
        if( (gr_idx * ftext->group_size) < idx){
                if(ftext->groups_count > (gr_idx + 1))
                        gr_idx += 1;
        }
        tmp = (gr_idx * ftext->group_size) - (ftext->group_size / 2);
        if( tmp >= idx ){
                /*От начала группы*/
                tmp = ftext->group_size * (gr_idx - 1) + 1;
                foreach_in_list(line, (Line *)ftext->lines_group[gr_idx - 1]){
                        if(tmp == idx)
                                return line;
                        tmp += 1;
                }
        } else {
                /*От конца группы*/
                Line * start;
                if((gr_idx + 1) >= ftext->groups_count){
                        /*Если это последняя группа*/
                        start = (Line *)&ftext->lines_end;
                        tmp = ftext->lines_count;
                } else {
                        if(ftext->lines_group[gr_idx] == NULL){
                                PERR("gr_idx is NULL: %ld", gr_idx);
                                return NULL;
                        }
                        start = (Line *)ftext->lines_group[gr_idx]->prev;
                        tmp = ftext->group_size * gr_idx;
                }
                foreach_in_list_reverse(line, start){
                        if(tmp == idx)
                                return line;
                        tmp -= 1;
                }
        }
        if(line == NULL){
                PERR("fault: %ld / %ld", tmp, idx);
        }
        
        return NULL;
}

int insert_Line_idx(FileText * ftext, unsigned long idx, Line * line)
{
        /*
                Вставляет линию line в позицию idx
                Сдвигает линию idx ниже (т.е. в idx+1)
                В случае успеха возвращает 0
        */
        if(ftext == NULL){
                PERR("ptr is NULL");
                return -1;
        }
        if(line == NULL){
                PERR("ptr is NULL");
                return -1;
        }
        if(idx == ftext->lines_count){
                /*Вставляем как предпоследнюю строку*/
                //int push_ListItem_middle(ListItem * pos, ListItem * item);
                return 0;
        }
        if(idx == ftext->lines_count + 1){
                /*Вставляем как последнюю строку*/
                return 0;
        }
        if(idx == 1){
                /*Вставляем как первую строку*/
                return 0;
        }
        if(idx > 1 && idx < ftext->lines_count){
                /*Вставляем в 'середину'*/
                Line * l_fnd;
                l_fnd = get_Line(ftext, idx);
                if(l_fnd == NULL){
                        
                }
                return 0;
        }
        
        return -1;
}

int cut_Line(FileText * ftext, FilePos * pos)
{
        /*
                Разрезает линию в позиции pos
                После разрезания текущей линии остается левая часть данных,
                создается новая линия послей текущей, с "правыми" данными текущей
                В случае успеха возвращает 0
        */
        
        return 0;
}