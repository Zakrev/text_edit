#include "../headers/file.h"

/*
        STATIC FUNCTIONS
*/
static void pLine(Line * line)
{
        if(line == NULL)
                return;
        PFUNC_START();
        PRINT("%lld/%lld: ", (long long)line->len, (long long)line->alloc);
        ssize_t i;
        for(i = 0; i < line->len; i++){
                PRINT("%c", line->data[i] == 0 ? '~' : line->data[i]);
        }
        PRINT("\n");
        PFUNC_END();
}

static Line * read_line_fd(int fd, const char * eol_chs, ssize_t eol_chs_len
#ifdef DBG_ALLOC_MEM
        , ssize_t * alloc
#endif
        )
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
        line->list_item_type = list_item_type_DEFAULT;
        line->alloc = sizeof(char) * MIN_LINE_ALLOC_LENGHT;
        line->type = main_editor_line_type_LINE;
        line->len = 0;
        max_len = MIN_LINE_ALLOC_LENGHT;
        while(1){
                if( (line->len + 1) == max_len ){
                        max_len += MIN_LINE_ALLOC_LENGHT;
                        line->data = realloc(line->data, sizeof(char) * max_len);
                        if(line->data == NULL){
                                PCERR("realloc %lld", (long long)max_len);
                                goto free_end;
                        }
                        line->alloc = sizeof(char) * max_len;
                }
                rc = read(fd, line->data + offset, eol_chs_len);
                if(eol_chs_len == rc){
                        line->len += eol_chs_len;
                        if(0 == memcmp(line->data + offset, eol_chs, eol_chs_len))
                                break;
                        offset += eol_chs_len;
                } else {
                        if(rc == -1){
                                PCERR("read data: %lld", (long long)eol_chs_len);
                                goto free_end;
                        }
                        line->len += rc;
                        break;
                }
        }
        if(line->len == 0){
#if DBG_LVL >= 3
                PRINT("new line is emrty\n");
#endif
                bzero(line->data, MIN_LINE_ALLOC_LENGHT);
                return line;
        }
        if(MAX_LINE_LEN_TO_ALLOC_ALIGNMENT > line->len && (line->len + 1) < line->alloc){
                /*Выравниваем выделенную память*/
                line->data = realloc(line->data, sizeof(char) * (line->len + 1));
                if(line->data == NULL){
                        PCERR("realloc %lld", (long long)(line->len + 1));
                        goto free_end;
                }
                line->alloc = sizeof(char) * (line->len + 1);
        }
#if DBG_LVL >= 3
        pLine(line);
        PFUNC_END();
#endif
#ifdef DBG_ALLOC_MEM
        *alloc += line->alloc;
#endif
        return line;
        free_end:
                if(line != NULL)
                        free(line->data);
                free(line);
                return NULL;
}

static Line * read_line_str(const char * str, ssize_t str_len, const char * eol_chs, ssize_t eol_chs_len
#ifdef DBG_ALLOC_MEM
        , ssize_t * alloc
#endif
        )
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
        line->list_item_type = list_item_type_DEFAULT;
        line->alloc = sizeof(char) * MIN_LINE_ALLOC_LENGHT;
        line->type = main_editor_line_type_LINE;
        line->len = 0;
        max_len = MIN_LINE_ALLOC_LENGHT;
        while(1){
                if(offset >= str_len)
                        break;
                if( (line->len + 1) == max_len ){
                        max_len += MIN_LINE_ALLOC_LENGHT;
                        line->data = realloc(line->data, sizeof(char) * max_len);
                        if(line->data == NULL){
                                PCERR("realloc %lld", (long long)max_len);
                                goto free_end;
                        }
                        line->alloc = sizeof(char) * max_len;
                }
                if(eol_chs_len <= (str_len - offset)){
                        memcpy(line->data + offset, str + offset, eol_chs_len);
                        line->len += eol_chs_len;
                        if(0 == memcmp(line->data + offset, eol_chs, eol_chs_len))
                                break;
                        offset += eol_chs_len;
                } else {
                        memcpy(line->data + offset, str + offset, eol_chs_len - (str_len - offset));
                        line->len += eol_chs_len;
                        break;
                }
        }
        if(line->len == 0){
#if DBG_LVL >= 3
                PRINT("new line is emrty\n");
#endif
#ifdef DBG_ALLOC_MEM
                *alloc += line->alloc;
#endif
                bzero(line->data, MIN_LINE_ALLOC_LENGHT);
                return line;
        }
        if(MAX_LINE_LEN_TO_ALLOC_ALIGNMENT > line->len && (line->len + 1) < line->alloc){
                /*Выравниваем выделенную память*/
                line->data = realloc(line->data, sizeof(char) * (line->len + 1));
                if(line->data == NULL){
                        PCERR("realloc %lld", (long long)(line->len + 1));
                        goto free_end;
                }
                line->alloc = sizeof(char) * (line->len + 1);
        }
#if DBG_LVL >= 3
        int dbg_i;
        for(dbg_i = 0; dbg_i <= line->len; dbg_i++)
                printf("%c", line->data[dbg_i]);
        PRINT("\n");
        PFUNC_END();
#endif
#ifdef DBG_ALLOC_MEM
        *alloc += line->alloc;
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
                Логически разбивает строки по группам
                В случае успеха возвращает 0
                TODO: многопоточное разбиение на группы
        */
        PFUNC_START();
        if(ftext->lines_count == 0){
                if(ftext->lines_group != NULL){
                        free(ftext->lines_group);
                }
                ftext->groups_count = 0;
                ftext->group_size = 0;
                PRINT("0 * 0\n");
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
#ifdef USE_PTHREADS
        pthreads!
#else
        Line * start;
        unsigned long ln_idx;
        unsigned long ln_idx_cur;
        unsigned long gr_idx = 0;
        Line * line = NULL;
        
        ftext->lines_group[0] = ftext->lines.next;
        start = ftext->lines_group[0]->next;
        ln_idx = 2;
        gr_idx = 1;
        
        ln_idx_cur = gr_idx * ftext->group_size + 1;
        foreach_in_list(line, start){
                if(line->type != main_editor_line_type_LINE)
                        continue;
                if(gr_idx >= ftext->groups_count)
                        break;
                if(ln_idx == ln_idx_cur){
                        ftext->lines_group[gr_idx] = line;
                        gr_idx += 1;
                        ln_idx_cur = gr_idx * ftext->group_size + 1;
                }
                ln_idx += 1;
        }
#endif
        PRINT("%lu * %lu\n", ftext->groups_count, ftext->group_size);
        PFUNC_END();
        return 0;
}

static void free_lines_Line(FileText * ftext, Line * lines)
{
        PFUNC_START();
        Line * line, * ltmp;
        unsigned long count = 0;
#ifdef DBG_ALLOC_MEM
        ssize_t free_alloc = 0;
#endif
        ltmp = NULL;
        foreach_in_list(line, lines){
                if(ltmp != NULL){
                        erase_ListItem((ListItem *)ltmp);
#ifdef DBG_ALLOC_MEM
                        ftext->alloc_mem -= ltmp->alloc;
                        free_alloc += ltmp->alloc;
#endif
                        free(ltmp->data);
                        free(ltmp);
                        ltmp = NULL;
                        count += 1;
                }
                if(line->type != main_editor_line_type_LINE)
                        continue;
                ltmp = line;
        }
        if(ltmp != NULL){
                erase_ListItem((ListItem *)ltmp);
#ifdef DBG_ALLOC_MEM
                ftext->alloc_mem -= ltmp->alloc;
                free_alloc += ltmp->alloc;
#endif
                free(ltmp->data);
                free(ltmp);
                ltmp = NULL;
                count += 1;
        }
#ifdef DBG_ALLOC_MEM
        PINF("free lines: %lu: %lld", count, (long long)free_alloc);
#else
        PINF("free lines: %lu", count);
#endif
        PFUNC_END();
}

static int read_lines_fd(FileText * ftext, const char * eol_chs, size_t eol_chs_len)
{
        /*
               Читает строки из файла в память, представляя их в структурах
               В случае успеха возвращает 0
               Изменяется размер файла
               Если последняя считанная строка оканчивается символом eol,
               то создается дополнительная пустая строка за ней
               TODO:
                        открытие файла перенести в эту функцию
                        многопоточное чтение из файла:
                                каждый поток отдельно открывает файл
                                читает свой блок данных, создает строки
                                после, строки "склеиваются"
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
                if(ftext->esize == ftext->size)
                        break;
                line = read_line_fd(ftext->fd, eol_chs, eol_chs_len
#ifdef DBG_ALLOC_MEM
                                        , &ftext->alloc_mem
#endif
                );
                if(0 == insert_ListItem_offset_down((ListItem *)&ftext->lines_end, (ListItem *)line)){
                        ftext->lines_count += 1;
                        ftext->esize += line->len;
                } else
                        break;
        }
        if(ftext->lines_count == 0){
                line = read_line_str(NULL, 0, NULL, 0
#ifdef DBG_ALLOC_MEM
                                        , &ftext->alloc_mem
#endif
                                );
                if(0 == insert_ListItem_offset_down((ListItem *)&ftext->lines_end, (ListItem *)line)){
                        ftext->lines_count += 1;
                        ftext->esize += line->len;
                }
                PINF("%lu: %lld/%lld", ftext->lines_count, (long long)ftext->esize, (long long)ftext->size);
                PFUNC_END();
                return create_lines_groups(ftext);
        }
        if(ftext->esize != ftext->size){
                PERR("Size of file not equal readed size: %s", ftext->path);
                free_lines_Line(ftext, ftext->lines.next);
                return -1;
        }
        
        line  = ftext->lines_end.prev;
        if(ftext->eol_chs_len <= line->len)
                if(0 == memcmp(line->data + (line->len - ftext->eol_chs_len), ftext->eol_chs, ftext->eol_chs_len)){
                        /*Создаем пустую строку, если последняя строка содержит символ eol*/
                        line = read_line_str(NULL, 0, NULL, 0
#ifdef DBG_ALLOC_MEM
                                                , &ftext->alloc_mem
#endif
                                        );
                        if(0 == insert_ListItem_offset_down((ListItem *)&ftext->lines_end, (ListItem *)line)){
                                ftext->lines_count += 1;
                                ftext->esize += line->len;
                        }
                }
        PINF("%lu: %lld/%lld", ftext->lines_count, (long long)ftext->esize, (long long)ftext->size);
        PFUNC_END();
        return create_lines_groups(ftext);
}

static Line * read_lines_str(const char * data, ssize_t data_len, const char * eol_chs, size_t eol_chs_len
#ifdef DBG_ALLOC_MEM
                                , ssize_t * alloc
#endif
                                )
{
        /*
               Читает строки из строки, представляя их в структурах
               В случае успеха возвращает указатель на первую считанную строку
               Строки будут идти по порядку чтения
               Если data_len <= 0 возвращается NULL
               TODO:
                        многопоточное чтение? нужно-ли?
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
#if DBG_LVL >= 2
        unsigned long lines_count = 0;
#endif
        
        /*Чтение первой строки*/
        first = read_line_str(data + offset, offset_len, eol_chs, eol_chs_len
#ifdef DBG_ALLOC_MEM
                                , alloc
#endif
                                );
        if(first != NULL){
                if(0 == insert_ListItem_offset_down((ListItem *)&lines_end, (ListItem *)first)){
                        offset_len -= first->len;
                        offset += first->len;
#if DBG_LVL >= 2
                        lines_count = 1;
#endif
                        while(1){
                                /*Чтение строк*/
                                if(offset_len == 0)
                                        break;
                                line = read_line_str(data + offset, offset_len, eol_chs, eol_chs_len
#ifdef DBG_ALLOC_MEM
                                                        , alloc
#endif
                                                        );
                                if(0 == insert_ListItem_offset_down((ListItem *)&lines_end, (ListItem *)line)){
                                        offset_len -= line->len;
                                        offset += line->len;
#if DBG_LVL >= 2
                                        lines_count += 1;
#endif
                                } else {
                                        PERR("insert line");
                                        break;
                                }
                        }
                }
        }
        
        if(lines_end.prev != NULL){
                lines_end.prev->next = NULL;
        }
        
        if(offset_len != 0){
                PERR("offset_len not equal 0: %lld", (long long)offset_len);
                free_lines_Line(NULL, first);
                return NULL;
        }
        
        PINF("%lu: %lld/%lld", lines_count, (long long)data_len, (long long)data_len);
        PFUNC_END();
        return first;
}

static void free_groups(FileText * ftext)
{
        PFUNC_START();
        if(ftext->groups_count <= 0){
                PINF("unexpected groups count: %lu", ftext->groups_count);
                return;
        }
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
        unsigned long count = 0;
#ifdef DBG_ALLOC_MEM
        ssize_t free_alloc = 0;
#endif
        ftext->lines_count = 0;
        ltmp = NULL;
        foreach_in_list(line, ftext->lines.next){
                if(ltmp != NULL){
                        erase_ListItem((ListItem *)ltmp);
                        ftext->esize -= ltmp->len;
#ifdef DBG_ALLOC_MEM
                        ftext->alloc_mem -= ltmp->alloc;
                        free_alloc += ltmp->alloc;
#endif
                        free(ltmp->data);
                        free(ltmp);
                        ltmp = NULL;
                        count += 1;
                }
                if(line->type != main_editor_line_type_LINE)
                        continue;
                ltmp = line;
        }
        if(ltmp != NULL){
                erase_ListItem((ListItem *)ltmp);
                ftext->esize -= ltmp->len;
#ifdef DBG_ALLOC_MEM
                ftext->alloc_mem -= ltmp->alloc;
                free_alloc += ltmp->alloc;
#endif
                free(ltmp->data);
                free(ltmp);
                ltmp = NULL;
                count += 1;
        }
#ifdef DBG_ALLOC_MEM
        PINF("free lines: %lu: %lld", count, (long long)free_alloc);
#else
        PINF("free lines: %lu", count);
#endif
        PFUNC_END();
}

static int edit_Line(Line * line, ssize_t start, ssize_t len, const char * data, ssize_t data_len
#ifdef DBG_ALLOC_MEM
                        , ssize_t * alloc
#endif
                        )
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
                TODO: неправильно удляет от начала строки до середины
        */
        PFUNC_START();
        if(line == NULL){
                PERR("ptr is NULL");
                return -1;
        }
        if(start < 0 || line->len < start){
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
                        if((new_len + 1) > line->alloc){
#ifdef DBG_ALLOC_MEM
                                *alloc -= line->alloc;
#endif
                                line->data = realloc(line->data, sizeof(char) * (new_len + 1));
                                if(line->data == NULL){
                                        PCERR("realloc %lld", (long long)(new_len + 1));
                                        return -1;
                                }
                                line->alloc = sizeof(char) * (new_len + 1);
#ifdef DBG_ALLOC_MEM
                                *alloc += line->alloc;
#endif
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
                for(pos = start + len, new_pos = start + data_len; new_pos < new_len; pos++, new_pos++){
                        /*перемещаем старые данные*/
                        line->data[new_pos] = line->data[pos];
                }
        }
        line->len = new_len;
        if(MAX_LINE_LEN_TO_ALLOC_ALIGNMENT > line->len && line->alloc > (line->len + 1)){
                /*Выравниваем выделенную память*/
#ifdef DBG_ALLOC_MEM
                                *alloc -= line->alloc;
#endif
                if(line->len == 0)
                        line->data = realloc(line->data, sizeof(char) * MIN_LINE_ALLOC_LENGHT);
                else
                        line->data = realloc(line->data, sizeof(char) * (line->len + 1));
                if(line->data == NULL){
                        PCERR("realloc %lld", (long long)(line->len + 1));
                        return -1;
                }
                if(line->len == 0)
                        line->alloc = sizeof(char) * MIN_LINE_ALLOC_LENGHT;
                else
                        line->alloc = sizeof(char) * (line->len + 1);
#ifdef DBG_ALLOC_MEM
                                *alloc += line->alloc;
#endif
        }
        PFUNC_END();
        return 0;
}

static int erase_from_Line(FileText * ftext, Line * line, ssize_t start, ssize_t len, unsigned char keep_eof)
{
        /*
                Функция вырезает символы из строки
                Сохраняет символы конца строки если keep_eof == 1
                Изменяет размер файла
        */
        PFUNC_START();
        ssize_t old_len = line->len;

        if(0 != edit_Line(line, start, len, ftext->eol_chs, 
                         keep_eof && line->len == (len + start) ? ftext->eol_chs_len : 0
#ifdef DBG_ALLOC_MEM
                        , &ftext->alloc_mem
#endif
                                )){
                PERR("fault edit line");
                return -1;
        }
        ftext->esize -= (old_len - line->len);
        
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
        ftext->lines.list_item_type = list_item_type_NOT_ERASE;
        ftext->lines.next = &ftext->lines_end;
        ftext->lines_end.type = main_editor_line_type_LINE_END;
        ftext->lines_end.list_item_type = list_item_type_NOT_ERASE;
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
        void pfinf(FileText * ftext)
        {
                PRINT("\n\tpath: %s\n", path);
                PRINT("\tsize: %lld\n", (long long)ftext->size);
                PRINT("\tesize: %lld\n", (long long)ftext->esize);
                PRINT("\tlines: %lu\n", ftext->lines_count);
                PRINT("\tgroups: %lu * %lu\n", ftext->groups_count, ftext->group_size);
#ifdef DBG_ALLOC_MEM
                PRINT("\talloc mem: %ld + %lu + %lld\n", (long)sizeof(FileText), 
                        (unsigned long)sizeof(Line *) * ftext->groups_count,
                        (long long)ftext->alloc_mem - sizeof(FileText) - sizeof(Line *) * ftext->groups_count);
#endif
        }
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
        /*Открывать в режиме только чтение
          для записи изменений открывать повторно*/
        ftext->fd = open(path, O_RDONLY);
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
        /*Закрываем файл*/
        /*close(ftext->fd);
        ftext->fd = 0;*/
        pfinf(ftext);
        /*TODO убрать*/
        unsigned long gridx;
        unsigned long lnum = 1;
        unsigned long grsize;
        unsigned long counte_print_lines = 50;
        Line * line;
        FilePos pos;
        
        /*if(0 != fill_FilePos(ftext, &pos, NULL, 1, 0, 17)){
                PERR("fault fill FilePos");
        } else
                edit_text(ftext, &pos, "Hello world\nHello world!\n", strlen("Hello world\nHello world!\n"));*/
        if(0 != fill_FilePos(ftext, &pos, NULL, 1, 0, 63188)){
                PERR("fault fill FilePos");
        } else
                edit_text(ftext, &pos, NULL, 0);
        PRINT("\nFILE");
        for(gridx = 0; gridx < ftext->groups_count && counte_print_lines >= lnum; gridx++){
                printf("\ngroup: %ld", gridx);
                grsize = ftext->group_size;
                foreach_in_list(line, ftext->lines_group[gridx]){
                        if(line->type == main_editor_line_type_LINE_END)
                                break;
                        printf("\n%lu: %lld/%lld: ", lnum, (long long)line->len, (long long)line->alloc);
                        int i;
                        for(i = 0; i < line->len; i++)
                                printf("%c", line->data[i] == '\0' ? '~' : line->data[i]);
                        lnum += 1;
                        if(counte_print_lines < lnum)
                                break;
                        if(--grsize == 0 && (gridx + 1) != ftext->groups_count)
                                break;
                }
        }
        PRINT("\nFILE\n");
        //write_to_file(ftext);
        //close(ftext->fd);
        pfinf(ftext);
        free_lines(ftext);
        free_groups(ftext);
        pfinf(ftext);
        free(ftext);
        /*TODO убрать\ */
        PFUNC_END();
        return ftext;
        free_return:
               free(ftext);
               return NULL;
}

int write_to_file(FileText * ftext)
{
        /*
                Функция записывает изменения в файл
                В случае успеха возвращает 0
        */
        PFUNC_START();
        if(ftext == NULL){
                PERR("ptr is NULL");
                return -1;
        }
        Line * line = ftext->lines.next;
        /*
        TODO: записывать не весь файл, а только изменения
        Line * line = get_Line(ftext, ftext->min_edit_idx);
        if(line == NULL){
                PERR("fault get first line: %lu", ftext->min_edit_idx);
                return -1;
        }
        */
        ftruncate(ftext->fd, ftext->esize);
        lseek(ftext->fd, 0, SEEK_SET);
        foreach_in_list(line, line){
                if(line->type != main_editor_line_type_LINE)
                        continue;
                write(ftext->fd, line->data, line->len);
        }
        PFUNC_END();
        return 0;
}

Line * get_Line(FileText * ftext, unsigned long idx)
{
        /*
                Возвращает указатель на линию idx
                Либо NULL
        */
        PFUNC_START();
        if(ftext == NULL){
                PERR("ptr is NULL");
                return NULL;
        }
        
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
        if(gr_idx <= 0)
                gr_idx = 1;
        
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
        return create_lines_groups(ftext);
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
        return create_lines_groups(ftext);
}

int cut_Line(FileText * ftext, FilePos * pos)
{
        /*
                Разрезает группу линий в позициях pos
                В структуре pos должны быть заполнены поля:
                        pos->line
                        pos->ch_idx
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
        Line * first_line = NULL;
        Line * new_line = NULL;
        FilePos * fe_pos;
        Line new_lines_end;
        
        new_lines_end.prev = NULL;
        new_lines_end.next = NULL;
        new_lines_end.list_item_type = list_item_type_NOT_ERASE;
        new_lines_end.type = main_editor_line_type_LINE_END;
        
        Line * _cut_line(FileText * ftext, FilePos * pos)
        {
                if(pos->line == NULL){
                        PERR("ptr is NULL");
                        return NULL;
                }
                Line * new_line = NULL;
                if(pos->line->len > 0){
                        if(pos->ch_idx < 0 || pos->ch_idx > pos->line->len){
                                PERR("unexpected ch_idx: %lld", (long long)pos->ch_idx);
                                return NULL;
                        }
                        new_line = read_line_str(pos->line->data + pos->ch_idx, pos->line->len - pos->ch_idx, ftext->eol_chs, ftext->eol_chs_len
#ifdef DBG_ALLOC_MEM
                                                        , &ftext->alloc_mem
#endif
                                                );
                        if(0 != erase_from_Line(ftext, fe_pos->line, fe_pos->ch_idx, new_line->len, 1)){
                                free_lines_Line(ftext, new_line);
                                return NULL;
                        }
                } else {
                        new_line = read_line_str(NULL, 0, NULL, 0
#ifdef DBG_ALLOC_MEM
                                                        , &ftext->alloc_mem
#endif
                                                );
                }
                
                return new_line;
        }
        
        /*Разрезаем первую строку*/
        first_line = _cut_line(ftext, pos);
        if(0 != insert_ListItem_offset_down((ListItem *)&new_lines_end, (ListItem *)first_line)){
                PERR("fault insert item");
                free_lines_Line(ftext, first_line);
                return -1;
        }
        
        foreach_in_list(fe_pos, pos->next){
                /*Разрезаем остальные строки*/
                new_line = _cut_line(ftext, fe_pos);
                if(0 != insert_ListItem_offset_down((ListItem *)&new_lines_end, (ListItem *)new_line)){
                        PERR("fault insert item");
                        free_lines_Line(ftext, first_line);
                        return -1;
                }
        }
        if(new_lines_end.prev != NULL)
                new_lines_end.prev->next = NULL;
        
        new_line = first_line;
        foreach_in_list(fe_pos, pos){
                /*Вставляем строки*/
                if(new_line != NULL){
                        if(0 != erase_ListItem((ListItem *)new_line))
                                break;
                        if(0 != insert_ListItem_offset_down((ListItem *)&fe_pos->line, (ListItem *)new_line))
                                break;
                        ftext->lines_count += 1;
                        ftext->esize += new_line->len;
                } else
                        break;
                new_line = new_line->next;
        }
        
        free_lines_Line(ftext, first_line);
        PFUNC_END();
        return create_lines_groups(ftext);
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
        
        pos->list_item_type = list_item_type_DEFAULT;
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
        if(ch_idx < 0 || ch_idx > pos->line->len){
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
                В структуре pos должны быть заполнены поля:
                        pos->line
                        pos->ch_idx
                pos->len - длина заменяемых данных, данные начиная с pos (включая этот символ) и заканчивая pos->len-1 будут заменены
                        при pos->len == 0, данные data будут записаны перед символом pos
                data - добавляемые данные
                        при data == NULL, данные начиная с pos (включая этот символ) и заканчивая pos->len-1 будут удалены
                data_len - размер добавляемых данных
                        при data_len == 0, данные начиная с pos (включая этот символ) и заканчивая pos->len-1 будут удалены
                
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
        if(pos->line == NULL){
                PERR("ptr is NULL");
                return -1;
        }
        if(data_len < 0){
                PERR("unexpexted 'data_len': %lld", (long long)data_len);
                return -1;
        }
        
        Line * new_lines = NULL;
        Line * erase_line;
        Line * tmp_line;
        Line * last_line = NULL;
        Line erases_lines_end;
        ssize_t last_len;
        
        erases_lines_end.type = main_editor_line_type_LINE_END;
        erases_lines_end.next = NULL;
        erases_lines_end.prev = NULL;
        if(pos->len > 0){
                /*Вырезаем старые данные и стороки*/
                if((pos->len + pos->ch_idx) > pos->line->len){
                        /*Удаляемые данные выходят за строку в pos*/
                        last_len = pos->line->len - pos->ch_idx;
                        foreach_in_list(last_line, pos->line->next){
                                last_len += last_line->len;
                                if(last_len >= pos->len)
                                        break;
                        }
                        if(last_len < pos->len){
                                PERR("fault counting erase len: %lld / %lld", (long long)last_len, (long long)pos->len);
                                return -1;
                        }
                        if(last_line == NULL){
                                PERR("fault counting erase len: %lld / %lld: ptr is NULL", (long long)last_len, (long long)pos->len);
                                return -1;
                        }
                        last_len = last_line->len - (last_len - pos->len);
                        if(last_len <= 0){
                                PERR("fault counting erase len: %lld / %lld: unexpected last_len", (long long)last_len, (long long)pos->len);
                                return -1;
                        }
                        tmp_line = NULL;
                        foreach_in_list(erase_line, pos->line->next){
                                /*Составляем список строк на удаление, исключая первую и последнюю*/
                                if(tmp_line != NULL){
                                        erase_ListItem((ListItem *) tmp_line);
                                        insert_ListItem_offset_down((ListItem *)&erases_lines_end, (ListItem *)tmp_line);
                                        ftext->lines_count -= 1;
                                        ftext->esize -= tmp_line->len;
                                }
                                if(erase_line == last_line){
                                        erase_line = NULL;
                                        break;
                                }
                                tmp_line = erase_line;
                        }
                        if(erases_lines_end.prev != NULL){
                                erases_lines_end.prev->next = NULL;
                                foreach_in_list_reverse(erase_line, erases_lines_end.prev){
                                        /*Получаем указатель на начало списка*/
                                        if(erase_line->prev == NULL)
                                                break;
                                }
                                free_lines_Line(ftext, erase_line);
                        }
                        
                        if(0 != erase_from_Line(ftext, pos->line, pos->ch_idx, pos->line->len - pos->ch_idx, 0))
                                return -1;
                        if(0 != erase_from_Line(ftext, last_line, 0, last_len, 0))
                                return -1;
                } else {
                        /*Удаляемые данные НЕ выходят за строку в pos*/
                        if(0 != erase_from_Line(ftext, pos->line, pos->ch_idx, pos->len, 0))
                                return -1;
                        /*Делим первую строку на две*/
                        if(0 != cut_Line(ftext, pos)){
                                PERR("fault cut line");
                                return -1;
                        }
                        if(ftext->eol_chs_len <= pos->line->len)
                                if(0 == memcmp(pos->line->data + (pos->line->len - ftext->eol_chs_len), ftext->eol_chs, ftext->eol_chs_len)){
                                        /*Вырезаем символ eol*/
                                        if(0 != erase_from_Line(ftext, pos->line, pos->line->len - ftext->eol_chs_len, ftext->eol_chs_len, 0))
                                                return -1;
                                }
                        last_line = pos->line->next;
                }
        }
        if(last_line == NULL){
                PERR("ptr is NULL");
                return -1;
        }
        if(data_len > 0 && data != NULL){
                /*Вставляем новые данные и строки*/
                /*Формируем строки из текста*/
                new_lines = read_lines_str(data, data_len, ftext->eol_chs, ftext->eol_chs_len
#ifdef DBG_ALLOC_MEM
                                                , &ftext->alloc_mem
#endif
                                                );
                if(new_lines == NULL){
                        PERR("fault read lines");
                        return -1;
                }
                if(new_lines != NULL){
                        /*Вставляем новые данные и строки
                                исключая первую и последнюю*/
                        tmp_line = NULL;
                        if(new_lines->next != NULL){
                                foreach_in_list(new_lines, new_lines->next){
                                        if(tmp_line != NULL){
                                                erase_ListItem((ListItem *)tmp_line);
                                                insert_ListItem_offset_down((ListItem *)last_line, (ListItem *)tmp_line);
                                                ftext->esize += tmp_line->len;
                                                ftext->lines_count += 1;
                                        }
                                        tmp_line = new_lines;
                                        if(new_lines->next == NULL)
                                                break;
                                }
                                if(tmp_line != NULL){
                                        new_lines = tmp_line->prev;
                                }
                                if(new_lines == NULL){
                                        PERR("ptr is NULL");
                                        return -1;
                                }
                        }
                }
        }
        if(new_lines != NULL){
                /*Склеиваем строки после вставки новых данных*/
                if(0 != edit_Line(pos->line, pos->ch_idx, 0, new_lines->data, new_lines->len
#ifdef DBG_ALLOC_MEM
                                        , &ftext->alloc_mem
#endif
                                        )){
                        PERR("fault edit line");
                        goto err_free_end;
                }
                ftext->esize += new_lines->len;
                tmp_line = new_lines->next;
                if(tmp_line == NULL){
                        /*Была вставлена только одна строка*/
                        if(last_line->len == 0 && last_line->next->type  == main_editor_line_type_LINE){
                                erase_ListItem((ListItem *)last_line);
                                insert_ListItem_offset_up((ListItem *)new_lines, (ListItem *)last_line);
                                ftext->lines_count -= 1;
                                last_line = pos->line->next;
                        }
                        
                        if(ftext->eol_chs_len <= pos->line->len)
                                if(0 == memcmp(pos->line->data + (pos->line->len - ftext->eol_chs_len), ftext->eol_chs, ftext->eol_chs_len)){
                                        /*Строка pos->line имеет символ eol*/
                                        free_lines_Line(ftext, new_lines);
                                        
                                        PFUNC_END();
                                        return create_lines_groups(ftext);
                                }
                        
                        if(0 != edit_Line(pos->line, pos->line->len, 0, last_line->data, last_line->len
#ifdef DBG_ALLOC_MEM
                                                , &ftext->alloc_mem
#endif
                                                )){
                                PERR("fault edit line");
                                goto err_free_end;
                        }
                        erase_ListItem((ListItem *)last_line);
                        insert_ListItem_offset_up((ListItem *)new_lines, (ListItem *)last_line);
                        
                        ftext->lines_count -= 1;
                        free_lines_Line(ftext, new_lines);
                } else {
                        /*Было вставлено больше одной строки*/
                        erase_ListItem((ListItem *)tmp_line);
                        insert_ListItem_offset_down((ListItem *)last_line, (ListItem *)tmp_line);
                        ftext->esize += tmp_line->len;
                        ftext->lines_count += 1;
                        
                        if(last_line->len == 0 && last_line->next->type == main_editor_line_type_LINE){
                                erase_ListItem((ListItem *)last_line);
                                insert_ListItem_offset_up((ListItem *)new_lines, (ListItem *)last_line);
                                ftext->lines_count -= 1;
                                last_line = tmp_line->next;
                        }
                        
                        if(ftext->eol_chs_len <= tmp_line->len)
                                if(0 == memcmp(tmp_line->data + (tmp_line->len - ftext->eol_chs_len), ftext->eol_chs, ftext->eol_chs_len)){
                                        /*Строка tmp_line имеет символ eol*/
                                        free_lines_Line(ftext, new_lines);
                                        
                                        PFUNC_END();
                                        return create_lines_groups(ftext);
                                }

                        if(0 != edit_Line(tmp_line, tmp_line->len, 0, last_line->data, last_line->len
#ifdef DBG_ALLOC_MEM
                                                , &ftext->alloc_mem
#endif
                                                )){
                                PERR("fault edit line");
                                goto err_free_end;
                        }
                        
                        erase_ListItem((ListItem *)last_line);
                        insert_ListItem_offset_up((ListItem *)new_lines, (ListItem *)last_line);
                        
                        ftext->lines_count -= 1;
                        free_lines_Line(ftext, new_lines);
                }
        } else {
                /*Склеиваем строки после удаления данных*/
                if(last_line->len == 0 && last_line->next->type == main_editor_line_type_LINE){
                        erase_ListItem((ListItem *)last_line);
                        new_lines = last_line;
                        ftext->lines_count -= 1;
                        last_line = pos->line->next;
                }
                
                if(ftext->eol_chs_len <= pos->line->len)
                        if(0 == memcmp(pos->line->data + (pos->line->len - ftext->eol_chs_len), ftext->eol_chs, ftext->eol_chs_len)){
                                /*Строка pos->line имеет символ eol*/
                                free_lines_Line(ftext, new_lines);
                                
                                PFUNC_END();
                                return create_lines_groups(ftext);
                        }
                
                if(0 != edit_Line(pos->line, pos->line->len, 0, last_line->data, last_line->len
#ifdef DBG_ALLOC_MEM
                                        , &ftext->alloc_mem
#endif
                                        )){
                        PERR("fault edit line");
                        goto err_free_end;
                }
                erase_ListItem((ListItem *)last_line);
                if(new_lines != NULL)
                        insert_ListItem_offset_up((ListItem *)new_lines, (ListItem *)last_line);
                else
                        new_lines = last_line;
                
                ftext->lines_count -= 1;
                free_lines_Line(ftext, new_lines);
        }
        
        PFUNC_END();
        return create_lines_groups(ftext);
        
        err_free_end:
                free_lines_Line(ftext, new_lines);
                return -1;
}

/*
        TODO
        Функция копирует len символов с позиции pos (включая символ в pos)
        в буфер
*/