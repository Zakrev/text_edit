#include "../headers/file.h"

/*
        STATIC FUNCTIONS
*/
static int find_ch(const char * str, ssize_t len, char ch)
{
#if DBG_LVL >= 4
        PFUNC_START();
#endif
        if(str == NULL)
                return 0;
        size_t i;
        for(i = 0; i < len; i++)
                if(str[i] == ch)
                        return 1;
        #if DBG_LVL >= 4
                PFUNC_END();
        #endif
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
        PFUNC_START();
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
        line->alloc = sizeof(char) * MIN_LINE_ALLOC_LENGHT;
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
                        line->alloc = sizeof(char) * max_len;
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
#if DBG_LVL >= 3
                PRINT("new line is emrty\n");
#endif
                goto free_end;
        }
        
#if DBG_LVL >= 3
        int dbg_i;
        for(dbg_i = 0; dbg_i <= line->len; dbg_i++)
                printf("%c", line->data[dbg_i]);
        PRINT("\n");
        PFUNC_END();
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
        PFUNC_START();
#endif
        Line * line;
        size_t max_len;
        ssize_t offset = 0;
        
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
        line->alloc = sizeof(char) * MIN_LINE_ALLOC_LENGHT;
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
                        line->alloc = sizeof(char) * max_len;
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
        PFUNC_END();
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
                
                FIXME:
                        если передать неправильный индекс (idx), то некоторые строки могут сдвинуться, хотя этого не нужно
                        например если вставить строку 500, а индекс передать 1, тогда младшие группы сдвинут указатели
        */
        PFUNC_START();
        if(ftext->lines_count == 0){
                if(ftext->lines_group != NULL){
                        free(ftext->lines_group);
                }
                ftext->groups_count = 0;
                ftext->group_size = 0;
                PRINT("0 / 0\n");
                PFUNC_END();
                return 0;
        }
        if(lines_old_count == ftext->lines_count){
                PRINT("lines count equal\n");
                PFUNC_END();
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
#ifdef DBG_ALLOC_MEM
                        ftext->alloc_mem -= sizeof(Line *) * group_old_count;
#endif
                        ftext->lines_group = realloc(ftext->lines_group, sizeof(Line *) * ftext->groups_count);
                        if(ftext->lines_group == NULL){
                                PCERR("group = realloc");
                                return -1;
                        }
#ifdef DBG_ALLOC_MEM
                        ftext->alloc_mem += sizeof(Line *) * ftext->groups_count;
#endif
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
#ifdef DBG_ALLOC_MEM
                        ftext->alloc_mem += sizeof(Line *) * ftext->groups_count;
#endif
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
                        /*Обновляем ссылки на строки
                        Только если группы уже были созданы*/
                        if(ftext->lines_group[gr_idx] == 0)
                                break;
                        unsigned long diff = ftext->lines_count - lines_old_count;
                        foreach_in_list_reverse(line, ftext->lines_group[gr_idx]){
                                if(line->type != main_editor_line_type_LINE)
                                        continue;
                                if(diff == 0)
                                        break;
                                diff -= 1;
                        }
                        if(line == NULL){
                                PERR("ptr is NULL");
                                return -1;
                        }
                        ftext->lines_group[gr_idx] = line;
                }
        } else if(ftext->lines_count < lines_old_count){
                for(; gr_idx < ftext->groups_count; gr_idx++){
                        /*Обновляем ссылки на строки
                        Только если группы уже были созданы*/
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
        PFUNC_END();
        return 0;
}

static int read_lines_fd(FileText * ftext, const char * eol_chs, size_t eol_chs_len)
{
        /*
               Читает строки из файла в память, представляя их в структурах
               В случае успеха возвращает 0
               Изменяется размер файла
               TODO:
                        удалить созданные строки при ошибке
        */
        PFUNC_START();
        if(ftext == NULL){
                PERR("ptr is NULL");
                return -1;
        }
        if(eol_chs == NULL){
                PERR("ptr is NULL");
                return -1;
        }
        if(eol_chs_len <= 0){
                PERR("unexpected eol_chs_len: %lld", (long long)eol_chs_len);
                return -1;
        }
        if(lseek(ftext->fd, 0, SEEK_SET) == -1){
                PCERR("lseek to start of file: %s", ftext->path);
                return -1;
        }
                
        Line * line;
        ftext->esize = 0;
        ftext->lines_count = 0;
        while(1){
                /*Чтение строк*/
                line = read_line_fd(ftext->fd, eol_chs, eol_chs_len);
                if(line == NULL)
                        break;
#ifdef DBG_ALLOC_MEM
                ftext->alloc_mem += line->alloc;
#endif
                if(0 == insert_ListItem_offset_down((ListItem *)&ftext->lines_end, (ListItem *)line)){
                        ftext->lines_count += 1;
                        ftext->esize += line->len;
                } else
                        break;
        }
        PRINT("%lld / %lld\n", (long long)ftext->esize, (long long)ftext->size);
        if(ftext->lines_count == 0){
                PFUNC_END();
                return 0;
        }
        if(ftext->esize != ftext->size){
                PERR("Size of file not equal readed size: %s", ftext->path);
                return -1;
        }
        PFUNC_END();
        return create_lines_groups(ftext, 0, 0);
}

static Line * read_lines_str(const char * data, ssize_t data_len, const char * eol_chs, size_t eol_chs_len)
{
        /*
               Читает строки из строки, представляя их в структурах
               В случае успеха возвращает указатель на первую считанную строку
               Строки будут идти по порядку чтения
               TODO:
                        удалить созданные строки при ошибке
        */
        PFUNC_START();
        if(data == NULL){
                PERR("ptr is NULL");
                return NULL;
        }
        if(data_len <= 0){
                PERR("unexpected data_len: %lld", (long long)data_len);
                return NULL;
        }
        if(eol_chs == NULL){
                PERR("ptr is NULL");
                return NULL;
        }
        if(eol_chs_len <= 0){
                PERR("unexpected line_end_len: %lld", (long long)eol_chs_len);
                return NULL;
        }
        Line * first = NULL;
        Line * line = NULL;
        Line lines_end;
        lines_end.type = main_editor_line_type_LINE_END;
        lines_end.next = NULL;
        lines_end.prev = NULL;
        ssize_t offset = 0;
        ssize_t offset_len = data_len;
        
        /*Чтение первой строки*/
        first = read_line_str(data + offset, offset_len, eol_chs, eol_chs_len);
        if(first != NULL){
                if(0 == insert_ListItem_offset_down((ListItem *)&lines_end, (ListItem *)first)){
                        offset_len -= first->len;
                        offset += first->len;
                        
                        while(1){
                        /*Чтение строк*/
                        line = read_line_str(data + offset, offset_len, eol_chs, eol_chs_len);
                        if(line == NULL)
                                break;
                        if(0 == insert_ListItem_offset_down((ListItem *)&lines_end, (ListItem *)line)){
                                offset_len -= line->len;
                                offset += line->len;
                        } else
                                break;
                        }
                }
        }
        
        if(lines_end.prev != NULL){
                lines_end.prev->next = NULL;
        }
        
        if(offset_len != 0){
                PERR("offset_len not equal 0: %lld", (long long)offset_len);
        }
        
        PFUNC_END();
        return first;
}

static void free_groups(FileText * ftext)
{
        PFUNC_START();
#ifdef DBG_ALLOC_MEM
        ftext->alloc_mem -= sizeof(Line *) * ftext->groups_count;
#endif
        ftext->groups_count = 0;
        ftext->group_size = 0;
        free(ftext->lines_group);
        PFUNC_END();
}

static void free_lines(FileText * ftext)
{
        PFUNC_START();
        Line * line, * ltmp;
        
        ftext->lines_count = 0;
        ltmp = NULL;
        foreach_in_list(line, ftext->lines.next){
                if(ltmp != NULL){
                        erase_ListItem((ListItem *)ltmp);
                        ftext->esize -= ltmp->len;
#ifdef DBG_ALLOC_MEM
                        ftext->alloc_mem -= ltmp->alloc;
#endif
                        free(ltmp->data);
                        free(ltmp);
                        ltmp = NULL;
                }
                if(line->type != main_editor_line_type_LINE)
                        continue;
                ltmp = line;
        }
        PFUNC_END();
}

static int edit_Line(Line * line, ssize_t start, ssize_t len, const char * data, ssize_t data_len)
{
        /*
                Функция редактирует строку line
                start - номер символа, с которого начнется редактирование
                len - длина заменяемых данных, данные начиная с start (включая этот символ) и заканчивая len-1 будут заменены
                        при len == 0, данные data будут записаны перед символом start
                data - добавляемые данные
                        при data == NULL, данные начиная с start (включая этот символ) и заканчивая len-1 будут удалены
                data_len - размер добавляемых данных
                        при data_len == 0, данные начиная с start (включая этот символ) и заканчивая len-1 будут удалены
                Функция не записывает в конец строки символ конца строки, но может удалить
                
                В случае успеха возвращает 0
        */
        PFUNC_START();
        if(line == NULL){
                PERR("ptr is NULL");
                return -1;
        }
        if(start < 0 || line->len <= start){
                PERR("unexpexted 'start': %lld", (long long)start);
                return -1;
        }
        if(len < 0 || line->len < (start + len)){
                PERR("unexpexted 'len': %lld", (long long)len);
                return -1;
        }
        if(data_len < 0){
                PERR("unexpexted 'data_len': %lld", (long long)len);
                return -1;
        }
        ssize_t new_len = line->len;
        ssize_t pos, new_pos;
                
        if(len > 0){
                if(line->data != NULL && line->len > 0){
                        /*Вырезаем символы*/
                        bzero(line->data + start, len);
                        new_len = line->len - len;
                } else {
                        PINF("line data: %p %lld", line->data, (long long)line->len);
                }
        }
        if(data != NULL && data_len > 0){
                if(line->data != NULL){
                        /*Вставляем символы*/
                        new_len += data_len;
                        if(new_len > line->alloc){
                                line->data = realloc(line->data, sizeof(char) * new_len);
                                if(line->data == NULL){
                                        PCERR("realloc %lld", (long long)new_len);
                                        return -1;
                                }
                                line->alloc = sizeof(char) * new_len;
                        }
                        for(pos = line->len - 1, new_pos = new_len - 1; line->data[pos] != 0 && pos >= start; pos--, new_pos--){
                                /*перемещаем старые данные в конец*/
                                line->data[new_pos] = line->data[pos];
                                
                        }
                        for(pos = 0, new_pos = start; pos < data_len; pos++, new_pos++){
                                /*перемещаем новые данные*/
                                line->data[new_pos] = data[pos];
                        }
                } else {
                        PINF("line data: %p %lld", line->data, (long long)line->len);
                }
        }
        if(data_len < len){
                /*Убираем "пустоту" в строке*/
                for(pos = start + len, new_pos = start + data_len; pos < new_len; pos++, new_len++){
                        /*перемещаем старые данные*/
                        line->data[new_pos] = line->data[pos];
                }
        }
        line->len = new_len;
        if(line->alloc > line->len){
                /*Выравниваем выделенную память*/
                line->data = realloc(line->data, sizeof(char) * line->len);
                if(line->data == NULL){
                        PCERR("realloc %lld", (long long)line->len);
                        return -1;
                }
                line->alloc = sizeof(char) * line->len;
        }
        
        PFUNC_END();
        return 0;
}

/*
        GLOBAL FUNCTIONS
*/
int FileText_init(FileText * ftext, const char * eol_chs, unsigned char eol_chs_len)
{
        /*
                Инициализирует структуру FileText_init
                eol_chs  - символы конца строки
                eol_chs_len - кол-во символов
        */
        PFUNC_START();
        if(ftext == NULL){
                PERR("ptr is NULL");
                return -1;
        }
        bzero(ftext, sizeof(FileText));
        ftext->lines.type = main_editor_line_type_LINE_0;
        ftext->lines.next = &ftext->lines_end;
        ftext->lines_end.type = main_editor_line_type_LINE_END;
        ftext->lines_end.prev = &ftext->lines;
        bzero(ftext->eol_chs, MAX_EOL_CHS_LEN);
        if(eol_chs_len > MAX_EOL_CHS_LEN)
                eol_chs_len = MAX_EOL_CHS_LEN;
        memcpy(ftext->eol_chs, eol_chs, eol_chs_len);
        ftext->eol_chs_len = eol_chs_len;
#ifdef DBG_ALLOC_MEM
        ftext->alloc_mem = sizeof(FileText);
#endif
        PFUNC_END();
        return 0;
}

FileText * FileText_open_file(const char * path)
{
        /*
                Открывает файл и представляет его содержимое в структупах
        */
        PFUNC_START();
        if(path == NULL){
                PERR("ptr is NULL");
                return NULL;
        }
        FileText * ftext;
        
        ftext = malloc(sizeof(FileText));
        if(ftext == NULL){
               PCERR("FileText malloc");
               return NULL;
        }
        FileText_init(ftext, "\n", 1);
        
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
        if(0 != read_lines_fd(ftext, ftext->eol_chs, ftext->eol_chs_len)){
               PERR("Read file: %s", path);
               free_lines(ftext);
               free_groups(ftext);
               goto free_return;
        }
        /*TODO убрать*/
        unsigned long gridx;
        unsigned long lnum = 1;
        unsigned long grsize;
        unsigned long counte_print_lines = 52;
        Line * line;
        FilePos cut_pos;
        
        if(0 != fill_FilePos(ftext, &cut_pos, NULL, 6, 24, 0)){
                PERR("fault fill FilePos");
        } else
                if(0 != cut_Line(ftext, &cut_pos)){
                        PERR("fault cut line");
                }
        line = get_Line(ftext, 1);
        edit_Line(line, 0, 0, "helo ", 5);
        for(gridx = 0; gridx < ftext->groups_count && counte_print_lines >= lnum; gridx++){
                printf("\ngroup: %ld", gridx);
                grsize = ftext->group_size;
                foreach_in_list(line, ftext->lines_group[gridx]){
                        if(line->type != main_editor_line_type_LINE)
                                continue;
                        printf("\n%lu: ", lnum);
                        int i;
                        for(i = 0; i < line->len - 1; i++)
                                printf("%c", line->data[i]);
                        lnum += 1;
                        if(counte_print_lines < lnum)
                                break;
                        if(--grsize == 0 && (gridx + 1) != ftext->groups_count)
                                break;
                }
        }
        /*TODO убрать\ */
        /*Информация о файле*/
        PRINT("\n\tpath: %s\n", path);
        PRINT("\tsize: %lld\n", (long long)ftext->size);
        PRINT("\tlines: %lu\n", ftext->lines_count);
#ifdef DBG_ALLOC_MEM
        PRINT("\talloc mem: %lld\n", (long long)ftext->alloc_mem);
#endif
        PFUNC_END();
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
        PFUNC_START();
        Line * line = NULL;
        unsigned long gr_idx;
        unsigned long tmp;
        
        if(idx > ftext->lines_count){
                PERR("unexpected idx: %lu", idx);
                return NULL;
        }
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
                        if(tmp == idx){
                                PFUNC_END();
                                return line;
                        }
                        tmp += 1;
                }
        } else {
                /*От конца группы*/
                Line * start;
                if((gr_idx + 1) >= ftext->groups_count){
                        /*Если это последняя группа*/
                        start = ftext->lines_end.prev;
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
                        if(tmp == idx){
                                PFUNC_END();
                                return line;
                        }
                        tmp -= 1;
                }
        }
        if(line == NULL){
                PERR("fault: %ld / %ld", tmp, idx);
        }
        
        return NULL;
}

int insert_Line_down(FileText * ftext, FilePos * pos, Line * line)
{
        /*
                Вставляет группу линий line в позицию pos
                Линии встанут перед pos
                Например:
                вставить        l1-l2-l3 в L2 из L1-L2-L3-L4
                получится       L1-l1-l2-l3-L2-L3-L4
                В случае успеха возвращает 0
                Изменяется размер файла
        */
        PFUNC_START();
        if(ftext == NULL){
                PERR("ptr is NULL");
                return -1;
        }
        if(pos == NULL){
                PERR("ptr is NULL");
                return -1;
        }
        if(pos->line == NULL){
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
                        if(0 != insert_ListItem_offset_down((ListItem *)pos->line, (ListItem *)tmp_line)){
                                PERR("fault insert item: %lu", ftext->lines_count);
                                return -1;
                        }
                        ftext->lines_count += 1;
                        ftext->esize += tmp_line->len;
                }
                tmp_line = fe_line;
        }
        if(tmp_line != NULL){
                /*Последний item*/
                if(0 != insert_ListItem_offset_down((ListItem *)pos->line, (ListItem *)tmp_line)){
                        PERR("fault insert item: %lu", ftext->lines_count);
                        return -1;
                }
                ftext->lines_count += 1;
                ftext->esize += tmp_line->len;
        }
        
        PFUNC_END();
        return create_lines_groups(ftext, pos->ln_idx, lines_old_count);
}

int insert_Line_up(FileText * ftext, FilePos * pos, Line * line)
{
        /*
                Вставляет группу линий line в позицию pos
                Линии встанут за pos
                Например:
                вставить        l1-l2-l3 в L2 из L1-L2-L3-L4
                получится       L1-L2-l1-l2-l3-L3-L4
                В случае успеха возвращает 0
                Изменяется размер файла
        */
        PFUNC_START();
        if(ftext == NULL){
                PERR("ptr is NULL");
                return -1;
        }
        if(pos == NULL){
                PERR("ptr is NULL");
                return -1;
        }
        if(pos->line == NULL){
                PERR("ptr is NULL");
                return -1;
        }
        if(line == NULL){
                PERR("ptr is NULL");
                return -1;
        }
        Line * fe_line, * tmp_line;
        Line * insert_pos = pos->line;
        unsigned long lines_old_count = ftext->lines_count;
        
        tmp_line = NULL;
        foreach_in_list(fe_line, line){
                if(tmp_line != NULL){
                        if(0 != insert_ListItem_offset_up((ListItem *)insert_pos, (ListItem *)tmp_line)){
                                PERR("fault insert item: %lu", ftext->lines_count);
                                return -1;
                        }
                        insert_pos = tmp_line;
                        ftext->lines_count += 1;
                        ftext->esize += tmp_line->len;
                }
                tmp_line = fe_line;
        }
        if(tmp_line != NULL){
                /*Последний item*/
                if(0 != insert_ListItem_offset_up((ListItem *)insert_pos, (ListItem *)tmp_line)){
                        PERR("fault insert item: %lu", ftext->lines_count);
                        return -1;
                }
                ftext->lines_count += 1;
                ftext->esize += tmp_line->len;
        }
        
        PFUNC_END();
        return create_lines_groups(ftext, pos->ln_idx, lines_old_count);
}

int cut_Line(FileText * ftext, FilePos * pos)
{
        /*
                Разрезает группу линий в позициях pos
                В структуре pos должны быть заполнены поля:
                        pos->line
                        pos->ch_idx
                        pos->ln_idx (можно 0)
                После разрезания текущей линии остается левая часть данных,
                создается новая линия послей текущей, с "правыми" данными текущей.
                Символ в позиции ch_idx является началом "правых" данных.
                Например:
                разрезать       l2-l3-l6 из l1-l2-l3-l4-l5-l6
                получится       l1-l2/1-l2/2-l3/1-l3/2-l4-l5-l6/1-l6/2
                В случае успеха возвращает 0
                Изменяется размер файла
        */
        PFUNC_START();
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
        new_lines_end.type = main_editor_line_type_LINE_END;
        
        minimal_idx = pos->ln_idx;
        foreach_in_list(fe_pos, pos){
                /*Создание новых строк
                TODO: удаление созданных строк в случае ошибки*/
                if(fe_pos->line == NULL){
                        PERR("ptr is NULL");
                        return -1;
                }
                if(fe_pos->line->len > 0){
                        if(fe_pos->ch_idx < 0 || fe_pos->ch_idx >= fe_pos->line->len){
                                PERR("unexpected ch_idx: %lld", (long long)fe_pos->ch_idx);
                                return -1;
                        }
                        ssize_t esize_tmp = ftext->esize;
                        ftext->esize -= pos->line->len;
                        new_line = read_line_str(pos->line->data + pos->ch_idx, pos->line->len - pos->ch_idx, ftext->eol_chs, ftext->eol_chs_len);
                        if(new_line == NULL){
                                PERR("fault create item");
                                ftext->esize = esize_tmp;
                                return -1;
                        }
                        if(0 != insert_ListItem_offset_down((ListItem *)&new_lines_end, (ListItem *)new_line)){
                                PERR("fault insert item");
                                ftext->esize = esize_tmp;
                                return -1;
                        }
#ifdef DBG_ALLOC_MEM
                        ssize_t __dbg_alloc_mem_tmp__ = ftext->alloc_mem;
                        ftext->alloc_mem -= fe_pos->line->alloc;
#endif
                        if(0 != edit_Line(fe_pos->line, fe_pos->ch_idx, new_line->len, ftext->eol_chs, ftext->eol_chs_len)){
                                PERR("fault erase chars");
#ifdef DBG_ALLOC_MEM
                                ftext->alloc_mem = __dbg_alloc_mem_tmp__;
#endif
                                ftext->esize = esize_tmp;
                                return -1;
                        }
#ifdef DBG_ALLOC_MEM
                        ftext->alloc_mem += fe_pos->line->alloc;
#endif
                        ftext->esize += fe_pos->line->len;
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
                ftext->esize += tmp_line->len;
        }
        
        PFUNC_END();
        return create_lines_groups(ftext, minimal_idx, lines_old_count);
}

int fill_FilePos(FileText * ftext, FilePos * pos, Line * line, unsigned long ln_idx, ssize_t ch_idx, ssize_t len)
{
        /*
                Функция заполняет структуру FilePos
                Если line == NULL, то ищет строку по ln_idx
                В случае успеха возвращает 0
                (FilePos * pos, unsigned long ln_idx, ssize_t ch_idx)
        */
        PFUNC_START();
        if(ftext == NULL){
                PERR("ptr is NULL");
                return -1;
        }
        if(pos == NULL){
                PERR("ptr is NULL");
                return -1;
        }
        
        pos->next = NULL;
        pos->prev = NULL;
        
        if(line == NULL){
                pos->line = get_Line(ftext, ln_idx);
                if(pos->line == NULL){
                        PERR("line not found: %lu", ln_idx);
                        return -1;
                }
        } else {
                pos->line = line;
        }
        
        pos->ln_idx = ln_idx;
        if(ch_idx < 0 || ch_idx >= pos->line->len){
                PERR("unexpected ch_idx: %lld", (long long)ch_idx);
                return -1;
        }
        pos->ch_idx = ch_idx;
        if(len < 0 || len > ftext->esize){
                PERR("unexpected len: %lld", (long long)len);
                return -1;
        }
        pos->len = len;
        
        PFUNC_END();
        return 0;
}

int edit_text(FileText * ftext, FilePos * pos, const char * data, ssize_t data_len)
{
        /*
                Функция редактирует текст (затрагивает несколько строк)
                pos - позиция символа, с которого начнется редактирование
                pos->len - длина заменяемых данных, данные начиная с pos (включая этот символ) и заканчивая pos->len-1 будут заменены
                        при pos->len == 0, данные data будут записаны перед символом pos
                data - добавляемые данные
                        при data == NULL, данные начиная с pos (включая этот символ) и заканчивая pos->len-1 будут удалены
                data_len - размер добавляемых данных
                        при data_len == 0, данные начиная с pos (включая этот символ) и заканчивая pos->len-1 будут удалены
                Функция не записывает в конец строки символ конца строки, но может удалить
                
                В случае успеха возвращает 0
                Изменяется размер файла
                TODO:
                        удалить созданные строки в случае ошибки
        */
        PFUNC_START();
        if(pos == NULL){
                PERR("ptr is NULL");
                return -1;
        }
        if(data_len < 0){
                PERR("unexpexted 'data_len': %lld", (long long)data_len);
                return -1;
        }
        Line * new_lines = NULL;
        if(data_len > 0){
                /*Формируем строки из текста*/
                new_lines = read_lines_str(data, data_len, ftext->eol_chs, ftext->eol_chs_len);
                if(new_lines == NULL){
                        PERR("fault read lines");
                        return -1;
                }
#ifdef DBG_ALLOC_MEM
                Line * __debug_tmp__;
                foreach_in_list(__debug_tmp__, new_lines){
                        ftext->alloc_mem += __debug_tmp__->alloc;
                }
#endif
        }
        if(pos->len > 0){
                /*Вырезаем старые данные и стороки*/
                
        }
        if(new_lines != NULL){
                /*Вставляем новые данные и строки*/
                /*TODO:
                        Если удаляли данные из "середины" строки, то первую новую строку
                        склеиваем с старой
                        тоже самое и для последней новой строки
                        Даже если данные не удаляли, строки нужно склеивать, потому что
                        pos по любому указывает на "середину" строки,кроме случая, когда
                        указывает на конец строки
                */
        }
        PFUNC_END();
        return 0;
}

/*
        TODO
        Функция копирует len символов с позиции pos (включая символ в pos)
        в буфер
*/

/*
        TODO
        Функция ищет idx строки по структуре Line
        Обязательно параллельная
        Скорей всего не нужная функция
*/

/*
        TODO
        Функция вырезает группу строк
        Изменяется размер файла
        Скорей всего не нужная функция
*/