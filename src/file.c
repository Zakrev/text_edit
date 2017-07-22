#include "../headers/file.h"

/*
        STATIC FUNCTIONS
*/
static int find_ch(const char * str, size_t len, char ch){
#if DBG_LVL >= 4
        PFUNC();
#endif
        if(str == NULL)
                return 0;
        size_t i;
        for(i = 0; i < len; i++)
                if(str[i] == ch)
                        return 1;
        return 0;
}

static Line * read_line_fd(int fd, const char * eol_chs, ssize_t eol_chs_len)
{
        /*
               Читает строку из файла
               Конец строки определяется набором символов в eol_chs
               Возвращает структуру, описывающую строку, либо NULL
        */
#if DBG_LVL >= 3
        PFUNC();
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
                        if(find_ch(eol_chs, eol_chs_len, line->data[offset])){
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

static Line * read_line_str(const char * str, ssize_t str_len, const char * eol_chs, ssize_t eol_chs_len)
{
        /*
               Читает строку из str
               Конец строки определяется набором символов в eol_chs
               Возвращает структуру, описывающую строку, либо NULL
        */
#if DBG_LVL >= 3
        PFUNC();
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
                if(offset >= str_len)
                        break;
                if( line->len == max_len ){
                        max_len += MIN_LINE_ALLOC_LENGHT;
                        line->data = realloc(line->data, sizeof(char) * max_len);
                        if(line->data == NULL){
                                PCERR("realloc %lld", (long long)max_len);
                                goto free_end;
                        }
                }
                line->data[offset] = str[offset];
                line->len += 1;
                if(find_ch(eol_chs, eol_chs_len, line->data[offset])){
                        break;
                }
                offset += 1;
        }
        if(line->len == 0){
#if DBG_LVL >= 3
                PRINT("new line is emrty\n");
#endif
                bzero(line->data, MIN_LINE_ALLOC_LENGHT);
                return line;
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
        PFUNC();
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
                        foreach_in_list(line, ftext->lines_group[gr_idx]){
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
                        ftext->lines_group[gr_idx] = line;
                }
        } else if(ftext->lines_count < lines_old_count){
                for(; gr_idx < ftext->groups_count; gr_idx++){
                        /*Обновляем ссылки на строки*/
                        if(ftext->lines_group[gr_idx] == 0)
                                break;
                        unsigned long diff = lines_old_count - ftext->lines_count;
                        foreach_in_list_reverse(line, ftext->lines_group[gr_idx]){
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
                        ftext->lines_group[gr_idx] = line;
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
                start = ftext->lines.next;
        }
        if(start->type != main_editor_line_type_LINE){
                PERR("not LINE type: %u", start->type);
                return -1;
        }
        ln_idx = gr_idx * ftext->group_size;
        if(ln_idx == 0){
                ftext->lines_group[gr_idx] = start;
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
                        ftext->lines_group[gr_idx] = line;
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
        PFUNC();
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
                line = read_line_fd(ftext->fd, line_end, line_end_len);
                if(line == NULL)
                        break;
                if(0 == insert_ListItem_offset_down((ListItem *)&ftext->lines_end, (ListItem *)line)){
                        PRINT("%p %p %p\n", line->prev, line, line->next);
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
        PFUNC();
        ftext->groups_count = 0;
        ftext->group_size = 0;
        free(ftext->lines_group);
}

static void free_lines(FileText * ftext)
{
        PFUNC();
        Line * line, * ltmp;
        
        ftext->lines_count = 0;
        ltmp = NULL;
        foreach_in_list(line, ftext->lines.next){
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

static int edit_Line(Line * line, ssize_t start, ssize_t len, const char * data, ssize_t data_len)
{
        /*
                Функция редактирует строку line
                start - номер символа, с которого начнется редактирование
                len - длина заменяемых данных, данные начиная с start (включая этот символ) и заканчивая len-1 будут заменены
                        при len == 0, данные data будут записаны после символа start
                data - добавляемые данные
                        при data == NULL, данные начиная с start (включая этот символ) и заканчивая len-1 будут удалены
                data_len - размер добавляемых данных
                        при data_len == 0, данные начиная с start (включая этот символ) и заканчивая len-1 будут удалены
                Функция не записывает в конец строки символ конца строки, но может удалить
                
                В случае успеха возвращает 0
        */
        PFUNC();
        /*TODO*/
        
        return 0;
}

/*
        GLOBAL FUNCTIONS
*/
int FileText_init(FileText * ftext)
{
        PFUNC();
        if(ftext == NULL){
                PERR("ptr is NULL");
                return -1;
        }
        bzero(ftext, sizeof(FileText));
        ftext->lines.type = main_editor_line_type_LINE_0;
        ftext->lines.next = &ftext->lines_end;
        ftext->lines_end.type = main_editor_line_type_LINE_END;
        ftext->lines_end.prev = &ftext->lines;
        ftext->eol_chs[0] = "\n";
        ftext->eol_chs_len = 1;
        
        return 0;
}

FileText * FileText_open_file(const char * path)
{
        PFUNC();
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
        if(0 != read_lines(ftext, ftext->eol_chs, ftext->eol_chs_len)){
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
                foreach_in_list(line, ftext->lines_group[gridx]){
                        if(line->type != main_editor_line_type_LINE)
                                continue;
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
        PFUNC();
        Line * line = NULL;
        unsigned long gr_idx;
        unsigned long tmp;
        
        if(idx > ftext->lines_count)
                return NULL;
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
                foreach_in_list(line, ftext->lines_group[gr_idx - 1]){
                        if(line->type != main_editor_line_type_LINE)
                                continue;
                        if(tmp == idx)
                                return line;
                        tmp += 1;
                }
        } else {
                /*От конца группы*/
                Line * start;
                if((gr_idx + 1) >= ftext->groups_count){
                        /*Если это последняя группа*/
                        start = &ftext->lines_end.prev;
                        tmp = ftext->lines_count;
                } else {
                        if(ftext->lines_group[gr_idx] == NULL){
                                PERR("gr_idx is NULL: %ld", gr_idx);
                                return NULL;
                        }
                        start = ftext->lines_group[gr_idx]->prev;
                        tmp = ftext->group_size * gr_idx;
                }
                foreach_in_list_reverse(line, start){
                        if(line->type != main_editor_line_type_LINE)
                                continue;
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

int insert_Line_obj_down(FileText * ftext, Line * pos, Line * line)
{
        /*
                Вставляет группу линий line в позицию pos
                Линии сдвинут pos вниз
                Например:
                вставить        l1-l2-l3 в L2 из L1-L2-L3-L4
                получится       L1-l1-l2-l3-L2-L3-L4
                В случае успеха возвращает 0
        */
        PFUNC();
        if(ftext == NULL){
                PERR("ptr is NULL");
                return -1;
        }
        if(pos == NULL){
                PERR("ptr is NULL");
                return -1;
        }
        if(line == NULL){
                PERR("ptr is NULL");
                return -1;
        }
        Line * fe_line, * tmp_line;
        unsigned long lines_old_count = ftext->lines_count;

        tmp_line = NULL;
        foreach_in_list(fe_line, line){
                if(tmp_line != NULL){
                        if(0 != insert_ListItem_offset_down((ListItem *)pos, (ListItem *)tmp_line)){
                                PERR("fault insert item: %lu", ftext->lines_count);
                                return -1;
                        }
                        ftext->lines_count += 1;
                }
                tmp_line = fe_line;
        }
        if(tmp_line != NULL){
                /*Последний item*/
                if(0 != insert_ListItem_offset_down((ListItem *)pos, (ListItem *)tmp_line)){
                        PERR("fault insert item: %lu", ftext->lines_count);
                        return -1;
                }
                ftext->lines_count += 1;
        }
        
        return create_lines_groups(ftext, idx, lines_old_count);
}

int insert_Line_obj_up(FileText * ftext, Line * pos, Line * line)
{
        /*
                Вставляет группу линий line в позицию pos
                Линии встанут за pos
                Например:
                вставить        l1-l2-l3 в L2 из L1-L2-L3-L4
                получится       L1-L2-l1-l2-l3-L3-L4
                В случае успеха возвращает 0
        */
        PFUNC();
        if(ftext == NULL){
                PERR("ptr is NULL");
                return -1;
        }
        if(pos == NULL){
                PERR("ptr is NULL");
                return -1;
        }
        if(line == NULL){
                PERR("ptr is NULL");
                return -1;
        }
        Line * fe_line, * tmp_line;
        unsigned long lines_old_count = ftext->lines_count;
        
        tmp_line = NULL;
        foreach_in_list(fe_line, line){
                if(tmp_line != NULL){
                        if(0 != insert_ListItem_offset_up((ListItem *)pos, (ListItem *)tmp_line)){
                                PERR("fault insert item: %lu", ftext->lines_count);
                                return -1;
                        }
                        pos = tmp_line;
                        ftext->lines_count += 1;
                }
                tmp_line = fe_line;
        }
        if(tmp_line != NULL){
                /*Последний item*/
                if(0 != insert_ListItem_offset_up((ListItem *)pos, (ListItem *)tmp_line)){
                        PERR("fault insert item: %lu", ftext->lines_count);
                        return -1;
                }
                ftext->lines_count += 1;
        }
        
        return create_lines_groups(ftext, idx, lines_old_count);
}

int insert_Line_idx_down(FileText * ftext, unsigned long idx, Line * line)
{
        /*
                Вставляет группу линий line в позицию idx
                Линии сдвинут idx вниз
                Например:
                вставить        l1-l2-l3 в L2 из L1-L2-L3-L4
                получится       L1-l1-l2-l3-L2-L3-L4
                В случае успеха возвращает 0
        */
        PFUNC();
        if(ftext == NULL){
                PERR("ptr is NULL");
                return -1;
        }
        if(line == NULL){
                PERR("ptr is NULL");
                return -1;
        }
        
        if(idx == (ftext->lines_count + 1)){
                /*Вставляем с конца файла*/
                return insert_Line_obj_down(ftext, &ftext->lines_end, line);
        } else {
                return insert_Line_obj_down(ftext, get_Line(ftext, idx), line);
        }
        
        return -1;
}

int insert_Line_idx_up(FileText * ftext, unsigned long idx, Line * line)
{
        /*
                Вставляет группу линий line в позицию idx
                Линии встанут за idx
                Например:
                вставить        l1-l2-l3 в L2 из L1-L2-L3-L4
                получится       L1-L2-l1-l2-l3-L3-L4
                В случае успеха возвращает 0
        */
        PFUNC();
        if(ftext == NULL){
                PERR("ptr is NULL");
                return -1;
        }
        if(line == NULL){
                PERR("ptr is NULL");
                return -1;
        }
        
        if(idx == (ftext->lines_count + 1)){
                /*Вставляем с конца файла*/
                return insert_Line_obj_up(ftext, ftext->lines_end.prev, line);
        } else {
                return insert_Line_obj_up(ftext, get_Line(ftext, idx), line);
        }
        
        return -1;
}

int cut_Line(FileText * ftext, const FilePos * pos, char ch_new_line)
{
        /*
                Разрезает группу линий в позициях pos
                В структуре pos должны быть заполнены поля:
                        pos->line
                        pos->ch_idx
                        pos->ln_idx (можно 0)
                После разрезания текущей линии остается левая часть данных,
                создается новая линия послей текущей, с "правыми" данными текущей.
                ch_new_line     - определяет символ окончания строки, который будет вставлен в конец "левых данных".
                Символ в позиции ch_idx является началом "правых" данных.
                Например:
                разрезать       l2-l3-l6 из l1-l2-l3-l4-l5-l6
                получится       l1-l2/1-l2/2-l3/1-l3/2-l4-l5-l6/1-l6/2
                В случае успеха возвращает 0
        */
        PFUNC();
        if(ftext == NULL){
                PERR("ptr is NULL");
                return -1;
        }
        if(pos == NULL){
                PERR("ptr is NULL");
                return -1;
        }
        FilePos * fe_pos;
        Line new_lines_end;
        Line * new_line = NULL;
        unsigned long minimal_idx;
        
        bzero(&new_lines_end, sizeof(Line));
        new_lines_end->type = main_editor_line_type_LINE_END;
        
        minimal_idx = pos->ln_idx;
        foreach_in_list(fe_pos, pos){
                /*Создание новых строк*/
                if(fe_pos->line == NULL){
                        PERR("ptr is NULL");
                        return -1;
                }
                if(fe_pos->line->len > 0){
                        if(fe_pos->ch_idx < 0 || fe_pos->ch_idx >= fe_pos->line->len){
                                PERR("unexpected ch_idx: %lld", fe_pos->ch_idx);
                                return -1;
                        }
                        new_line = read_line_str(pos->line->data + pos->ch_idx, pos->line->len - pos->ch_idx, ftext->eol_chs, ftext->eol_chs_len);
                        if(new_line == NULL){
                                PERR("fault create item");
                                return -1;
                        }
                        if(0 != insert_ListItem_offset_down((ListItem *)&new_lines_end, (ListItem *)new_line)){
                                PERR("fault insert item");
                                return -1;
                        }
                        /*TODO: вырезать текст из line*/
                } else {
                        /*Полностью пустая строка*/
                        new_line = read_line_str(NULL, 0, NULL, 0);
                        if(new_line == NULL){
                                PERR("fault create item");
                                return -1;
                        }
                        if(0 != insert_ListItem_offset_down((ListItem *)&new_lines_end, (ListItem *)new_line)){
                                PERR("fault insert item");
                                return -1;
                        }
                }
                if(minimal_idx > fe_pos->ln_idx)
                        minimal_idx = pos->ln_idx;
                if(fe_pos->next == NULL)
                        break;
        }
        new_line = new_lines_end.prev;
        if(new_line == NULL){
                PERR("ptr is NULL");
                return -1;
        }
        Line * tmp_line = NULL;
        unsigned long lines_old_count = ftext->lines_count;
        foreach_in_list_reverse(fe_pos, fe_pos){
                /*Добавление строк*/
                tmp_line = new_line;
                new_line = new_line->prev;
                erase_ListItem((ListItem *)tmp_line);
                if(0 != insert_ListItem_offset_up((ListItem *)fe_pos->line, (ListItem *)tmp_line)){
                        PERR("fault insert item");
                        break;
                }
                ftext->lines_count += 1;
        }
        
        return create_lines_groups(ftext, minimal_idx, lines_old_count);
}

/*
        TODO
        Функция удаляет len символов начиная с позиции pos
        Затрагиваются все строки, находящиеся в диапазоне len
        Или лучше РЕДАКТИРУЕТ?
*/