#include "../headers/file.h"

/*
        STATIC FUNCTIONS
*/
#if DBG_LVL >= 3
static void pline(Line * line)
{
        bytes_t i;
        for(i = 0; i < line->len; i++){
                PRINT("%c", line->data[i] == 0 ? '~' : line->data[i]);
        }
}
#endif


static void pfinf(FileText * ftext, const char * path)
{
#if DBG_LVL >= 2
        if(ftext == NULL)
                return;
        PRINT("FILE_INFO\n");
        PRINT("path: %s\n", path);
        PRINT("esize: %lld\n", (long long)ftext->esize);
        PRINT("lines: %u\n", ftext->lines_count);
        PRINT("groups: %hu * %hu\n", ftext->groups_count, ftext->group_size);
#ifdef DBG_ALLOC_MEM
        PRINT("alloc mem: %lu + %lu + %lld\n", sizeof(FileText), (unsigned long)sizeof(Line *) * ftext->groups_count,
                (long long)ftext->alloc_mem - sizeof(Line *) * ftext->groups_count);
#endif
        PRINT("FILE_INFO\n");
#endif
}

static Line * read_line_fd(int fd, const unsigned char * eol_chs, unsigned char eol_chs_len
#ifdef DBG_ALLOC_MEM
                                , bytes_t * alloc
#endif
                                )
{
        /*
               Читает строку из файла
               Конец строки определяется набором байт в eol_chs
               Возвращает:
                        OK      указатель на строку
                        ERR     NULL
        */
#if DBG_LVL >= 3
        PFUNC_START();
#endif
        Line * line;
        bytes_t offset = 0;
        int rc;
        
        line = malloc(sizeof(Line));
        if(line == NULL){
                PCERR("ptr is NULL");
                goto err_free_end;
        }
        line->data = malloc(sizeof(unsigned char) * MIN_LINE_ALLOC_LENGHT);
        if(line->data == NULL){
                PCERR("ptr is NULL");
                goto err_free_end;
        }
        
        line->list_item_type = list_item_type_DEFAULT;
        line->alloc = sizeof(unsigned char) * MIN_LINE_ALLOC_LENGHT;
        line->type = main_editor_line_type_LINE;
        line->len = 0;

        while(1){
                if(line->alloc < (line->len + eol_chs_len + 1)){
                        line->data = realloc(line->data, sizeof(unsigned char) * (line->len + MIN_LINE_ALLOC_LENGHT));
                        if(line->data == NULL){
                                PCERR("realloc %lld -> %lld", (long long)line->alloc, (long long)sizeof(unsigned char) * (line->len + MIN_LINE_ALLOC_LENGHT));
                                goto err_free_end;
                        }
                        line->alloc = sizeof(unsigned char) * (line->len + MIN_LINE_ALLOC_LENGHT);
                }
                rc = read(fd, line->data + offset, eol_chs_len);
                if(eol_chs_len == rc){
                        line->len += eol_chs_len;
                        if(0 == memcmp(line->data + offset, eol_chs, eol_chs_len))
                                break;
                        offset += eol_chs_len;
                } else {
                        if(rc == -1){
                                PCERR("read from fd: %d", fd);
                                goto err_free_end;
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
        if(MAX_LINE_LEN_TO_ALLOC_ALIGNMENT >= line->len && (line->len + 1) < line->alloc){
                /*Выравниваем выделенную память*/
                line->data = realloc(line->data, sizeof(unsigned char) * (line->len + 1));
                if(line->data == NULL){
                        PCERR("realloc %lld -> %lld", (long long)line->alloc, (long long)sizeof(unsigned char) * (line->len + 1));
                        goto err_free_end;
                }
                line->alloc = sizeof(char) * (line->len + 1);
        }
#ifdef DBG_ALLOC_MEM
        *alloc += line->alloc;
        *alloc += sizeof(Line);
#endif
#if DBG_LVL >= 3
        PRINT("readed line: %lld/%lld: ", (long long)line->len, (long long)line->alloc);
        pline(line);
        PRINT("\n");
        PFUNC_END();
#endif
        return line;
        
        err_free_end:
                if(line != NULL){
                        free(line->data);
                        free(line);
                }
                return NULL;
}

static Line * read_line_str(const unsigned char * data, bytes_t data_len, const unsigned char * eol_chs, unsigned char eol_chs_len
#ifdef DBG_ALLOC_MEM
                                , bytes_t * alloc
#endif
                                )
{
        /*
               Читает строку из буфера
               Конец строки определяется набором байт в eol_chs
               Возвращает:
                        OK      указатель на строку
                        ERR     NULL
        */
#if DBG_LVL >= 3
        PFUNC_START();
#endif
        if(data == NULL){
                PCERR("ptr is NULL");
                return NULL;
        }
        if(data_len <= 0){
                PCERR("unexpected data_len: %lld", (long long)data_len);
                return NULL;
        }
        Line * line;
        bytes_t offset = 0;
        
        line = malloc(sizeof(Line));
        if(line == NULL){
                PCERR("ptr is NULL");
                goto err_free_end;
        }
        line->data = malloc(sizeof(unsigned char) * MIN_LINE_ALLOC_LENGHT);
        if(line->data == NULL){
                PCERR("ptr is NULL");
                goto err_free_end;
        }
        
        line->list_item_type = list_item_type_DEFAULT;
        line->alloc = sizeof(unsigned char) * MIN_LINE_ALLOC_LENGHT;
        line->type = main_editor_line_type_LINE;
        line->len = 0;

        while(1){
                if(line->alloc < (line->len + eol_chs_len + 1)){
                        line->data = realloc(line->data, sizeof(unsigned char) * (line->len + MIN_LINE_ALLOC_LENGHT));
                        if(line->data == NULL){
                                PCERR("realloc %lld -> %lld", (long long)line->alloc, (long long)sizeof(unsigned char) * (line->len + MIN_LINE_ALLOC_LENGHT));
                                goto err_free_end;
                        }
                        line->alloc = sizeof(unsigned char) * (line->len + MIN_LINE_ALLOC_LENGHT);
                }
                if(eol_chs_len > data_len)
                        eol_chs_len = eol_chs_len - data_len;
                line->len += eol_chs_len;
                memcpy(line->data + offset, data + offset, eol_chs_len);
                if(0 == memcmp(line->data + offset, eol_chs, eol_chs_len))
                        break;
                offset += eol_chs_len;
                if(offset >= data_len)
                        break;
        }
        if(line->len == 0){
#if DBG_LVL >= 3
                PRINT("new line is emrty\n");
#endif
                bzero(line->data, MIN_LINE_ALLOC_LENGHT);
                return line;
        }
        if(MAX_LINE_LEN_TO_ALLOC_ALIGNMENT >= line->len && (line->len + 1) < line->alloc){
                /*Выравниваем выделенную память*/
                line->data = realloc(line->data, sizeof(unsigned char) * (line->len + 1));
                if(line->data == NULL){
                        PCERR("realloc %lld -> %lld", (long long)line->alloc, (long long)sizeof(unsigned char) * (line->len + 1));
                        goto err_free_end;
                }
                line->alloc = sizeof(char) * (line->len + 1);
        }
#ifdef DBG_ALLOC_MEM
        *alloc += line->alloc;
        *alloc += sizeof(Line);
#endif
#if DBG_LVL >= 3
        PRINT("readed line: %lld/%lld: ", (long long)line->len, (long long)line->alloc);
        pline(line);
        PRINT("\n");
        PFUNC_END();
#endif
        return line;
        
        err_free_end:
                if(line != NULL){
                        free(line->data);
                        free(line);
                }
                return NULL;
}

static char create_lines_groups(FileText * ftext)
{
        /*
                Логически разбивает строки по группам
                Каждая группа, указывает на первую строку своего набора
                Возвращает:
                        OK      0
                        ERR     -1
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

        unsigned short group_old_size = ftext->group_size;
        unsigned char group_old_count = ftext->groups_count;

        /*Вычисление размера группы*/
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
                                PCERR("ptr is NULL");
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
                                PCERR("ptr is NULL");
                                return -1;
                        }
                        bzero(ftext->lines_group, ftext->groups_count);
#ifdef DBG_ALLOC_MEM
                        ftext->alloc_mem += sizeof(Line *) * ftext->groups_count;
#endif
                }
        }
        /*Разбиение по группам*/
#ifdef FILEOPT_USE_PTHREADS
        pthreads!
#else
        Line * start;
        unsigned int ln_idx;
        unsigned int ln_idx_cur;
        unsigned char gr_idx = 0;
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
        PRINT("%hu * %hu\n", ftext->groups_count, ftext->group_size);
        PFUNC_END();
        return 0;
}

static void free_lines_Line(FileText * ftext, Line * lines)
{
        /*
                Освобождает память из под строк
                Изменяет mem alloc
        */
        PFUNC_START();
        Line * line, * ltmp;
        unsigned int count = 0;
#ifdef DBG_ALLOC_MEM
        bytes_t free_alloc = 0;
#endif
        ltmp = NULL;
        foreach_in_list(line, lines){
                if(ltmp != NULL){
                        erase_ListItem((ListItem *)ltmp);
#ifdef DBG_ALLOC_MEM
                        ftext->alloc_mem -= ltmp->alloc;
                        ftext->alloc_mem -= sizeof(Line);
                        free_alloc += ltmp->alloc;
                        free_alloc += sizeof(Line);
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
                ftext->alloc_mem -= sizeof(Line);
                free_alloc += ltmp->alloc;
                free_alloc += sizeof(Line);
#endif
                free(ltmp->data);
                free(ltmp);
                ltmp = NULL;
                count += 1;
        }
#ifdef DBG_ALLOC_MEM
        PINF("free lines: %u: %lld", count, (long long)free_alloc);
#else
        PINF("free lines: %u", count);
#endif
        PFUNC_END();
}

static Line * read_lines_path(const char * path, const unsigned char * eol_chs, unsigned char eol_chs_len
#ifdef FILEOPT_USE_PTHREADS
                                , bytes_t write_offset
                                , bytes_t write_len
#endif
#ifdef DBG_ALLOC_MEM
                                , bytes_t * alloc
#endif
                                )
{
        /*
               Читает строки из файла в память, представляя их в структурах
               Возвращает:
                        OK      указатель на первую строку
                        ERR     NULL
               TODO:
                        многопоточное чтение из файла:
                                каждый поток отдельно открывает файл
                                читает свой блок данных, создает строки
                                после, строки "склеиваются"
        */
        PFUNC_START();
        if(eol_chs == NULL){
                PERR("ptr is NULL");
                return NULL;
        }
        if(eol_chs_len <= 0){
                PERR("unexpected eol_chs_len: %hu", eol_chs_len);
                return NULL;
        }
        int fd;
        bytes_t fsize = 0;
        Line * first_line = NULL;
        Line * line = NULL;
        Line end_lines;
#if DBG_LVL >= 2
        unsigned int lines_count = 0;
#endif
        
        end_lines.type = main_editor_line_type_LINE_END;
        end_lines.list_item_type = list_item_type_NOT_ERASE;
        end_lines.prev = NULL;
        end_lines.next = NULL;
        
        fd = open(path, O_RDONLY);
        if(fd == -1){
               PCERR("open file: %s", path);
               return NULL;
        }
#ifdef FILEOPT_USE_PTHREADS
        fsize = write_len;
        if(write_offset != lseek(fd, write_offset, SEEK_SET)){
                PCERR("lseek in file: %lld: %s", (long long)write_offset, path);
                goto err_free_end;
        }
#else
        fsize = lseek(fd, 0, SEEK_END);
        if(fsize < 0){
                PCERR("lseek in file: %s", path);
                goto err_free_end;
        }
        if(0 != lseek(fd, 0, SEEK_SET)){
                PCERR("lseek in file: %s", path);
                goto err_free_end;
        }
#endif
        
        /*Чтение первой строки*/
        first_line = read_line_fd(fd, eol_chs, eol_chs_len
#ifdef DBG_ALLOC_MEM
                                        , alloc
#endif
                                        );
        if(0 != insert_ListItem_offset_down((ListItem *)&end_lines, (ListItem *)first_line)){
                PERR("fault insert item");
                goto err_free_end;
        }
        fsize -= first_line->len;
#if DBG_LVL >= 2
        lines_count = 1;
#endif
        while(fsize > 0){
                /*Чтение остальных строк*/
                line = read_line_fd(fd, eol_chs, eol_chs_len
#ifdef DBG_ALLOC_MEM
                                        , alloc
#endif
                                        );
                if(0 != insert_ListItem_offset_down((ListItem *)&end_lines, (ListItem *)line)){
                        PERR("fault insert item");
                        goto err_free_end;
                }
                fsize -= line->len;
#if DBG_LVL >= 2
                lines_count += 1;
#endif
        }
        if(fsize < 0){
                PERR("unexpected fsize");
                goto err_free_end;
        }
        
        if(end_lines.prev != NULL)
                        end_lines.prev->next = NULL;
        PINF("created lines: %u", lines_count);
        PFUNC_END();
        return first_line;
        
        err_free_end:
                if(end_lines.prev != NULL)
                        end_lines.prev->next = NULL;
                free_lines_Line(NULL, first_line);
                close(fd);
                return NULL;
}

static Line * read_lines_str(const unsigned char * data, bytes_t data_len, const unsigned char * eol_chs, unsigned char eol_chs_len
#ifdef DBG_ALLOC_MEM
                                , bytes_t * alloc
#endif
                                )
{
        /*
               Читает строки из буфера в память, представляя их в структурах
               Возвращает:
                        OK      указатель на первую строку
                        ERR     NULL
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
                PERR("unexpected eol_chs_len: %hu", eol_chs_len);
                return NULL;
        }
        bytes_t offset = 0;
        Line * first_line = NULL;
        Line * line = NULL;
        Line end_lines;
#if DBG_LVL >= 2
        unsigned int lines_count = 0;
#endif
        
        end_lines.type = main_editor_line_type_LINE_END;
        end_lines.list_item_type = list_item_type_NOT_ERASE;
        end_lines.prev = NULL;
        end_lines.next = NULL;
        
        /*Чтение первой строки*/
        first_line = read_line_str(data + offset, data_len, eol_chs, eol_chs_len
#ifdef DBG_ALLOC_MEM
                                        , alloc
#endif
                                        );
        if(0 != insert_ListItem_offset_down((ListItem *)&end_lines, (ListItem *)first_line)){
                PERR("fault insert item");
                goto err_free_end;
        }
        data_len -= first_line->len;
        offset += first_line->len;
#if DBG_LVL >= 2
        lines_count = 1;
#endif
         while(data_len > 0){
                /*Чтение остальных строк*/
                line = read_line_str(data + offset, data_len, eol_chs, eol_chs_len
#ifdef DBG_ALLOC_MEM
                                        , alloc
#endif
                                        );
                if(0 != insert_ListItem_offset_down((ListItem *)&end_lines, (ListItem *)line)){
                        PERR("fault insert item");
                        goto err_free_end;
                }
                data_len -= line->len;
                offset += line->len;
#if DBG_LVL >= 2
                lines_count += 1;
#endif
        }
        if(data_len < 0){
                PERR("unexpected fsize");
                goto err_free_end;
        }
        
        if(end_lines.prev != NULL)
                        end_lines.prev->next = NULL;
        PINF("created lines: %u", lines_count);
        PFUNC_END();
        return first_line;
        
        err_free_end:
                if(end_lines.prev != NULL)
                        end_lines.prev->next = NULL;
                free_lines_Line(NULL, first_line);
                return NULL;
}

static void free_groups(FileText * ftext)
{
        /*
                Освобождает память из под групп (строки не удаляет)
                Изменяет mem alloc
        */
        PFUNC_START();
        if(ftext->groups_count <= 0){
                PINF("unexpected groups count: %hu", ftext->groups_count);
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
        /*
                Освобождает память из под строк в структуре
                Изменяет размер файла
                Изменяет mem alloc
        */
        PFUNC_START();
        Line * line, * ltmp;
        unsigned int count = 0;
#ifdef DBG_ALLOC_MEM
        bytes_t free_alloc = 0;
#endif
        ftext->lines_count = 0;
        ltmp = NULL;
        foreach_in_list(line, ftext->lines.next){
                if(ltmp != NULL){
                        erase_ListItem((ListItem *)ltmp);
                        ftext->esize -= ltmp->len;
#ifdef DBG_ALLOC_MEM
                        ftext->alloc_mem -= ltmp->alloc;
                        ftext->alloc_mem -= sizeof(Line);
                        free_alloc += ltmp->alloc;
                        free_alloc += sizeof(Line);
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
                ftext->alloc_mem -= sizeof(Line);
                free_alloc += ltmp->alloc;
                free_alloc += sizeof(Line);
#endif
                free(ltmp->data);
                free(ltmp);
                ltmp = NULL;
                count += 1;
        }
#ifdef DBG_ALLOC_MEM
        PINF("free lines: %u: %lld", count, (long long)free_alloc);
#else
        PINF("free lines: %u", count);
#endif
        PFUNC_END();
}

static char edit_Line(Line * line, bytes_t start, bytes_t len, const unsigned char * data, bytes_t data_len
#ifdef DBG_ALLOC_MEM
                        , bytes_t * alloc
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
                
                Возвращает:
                        OK      0
                        ERR     -1
                TODO: неправильно удляет от начала строки до середины
        */
        PFUNC_START();
        if(line == NULL){
                PERR("ptr is NULL");
                return -1;
        }
        if(start < 0 || line->len < start){
                PERR("unexpexted 'start': %lld/%lld", (long long)start, (long long)line->len);
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
        bytes_t new_len = line->len;
        bytes_t pos, new_pos;
        
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
                                        PCERR("realloc %lld -> %lld", (long long)line->alloc, (long long)(new_len + 1));
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
                        PCERR("realloc %lld -> %lld", (long long)line->alloc, (long long)(new_len + 1));
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

static char erase_from_Line(FileText * ftext, Line * line, bytes_t start, bytes_t len, unsigned char keep_eof)
{
        /*
                Функция вырезает символы из строки
                Сохраняет символы конца строки если keep_eof == 1
                Изменяет размер файла
                Изменяет mem alloc
                Возвращает:
                        OK      0
                        ERR     -1
        */
        PFUNC_START();
        if(ftext == NULL){
                PERR("ptr is NULL");
                return -1;
        }
        if(line == NULL){
                PERR("ptr is NULL");
                return -1;
        }
        bytes_t old_len = line->len;

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

static char add_free_line(FileText * ftext)
{
        /*
                Добавляет пустую строку в конец файла, если это необходимо
                Изменяет размер файла
                Изменяет mem alloc
                Возвращает:
                        OK      0
                        ERR     -1
        */
        PFUNC_START();
        if(ftext == NULL){
                PERR("ptr is NULL");
                return -1;
        }
        Line * line;
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
                PFUNC_END();
                return create_lines_groups(ftext);
        }
        line = ftext->lines_end.prev;
        if(line->type != main_editor_line_type_LINE)
                return 0;
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
                        PFUNC_END();
                        return create_lines_groups(ftext);
                }
        PFUNC_END();
        return 0;
}

static unsigned int write_to_file_path(const char * path, Line * line, bytes_t write_offset, unsigned int write_count)
{
        /*
                Функция записывает строки в файл
                Возвращает:
                        OK      0
                        ERR     количество НЕ записанных строк
        */
        PFUNC_START();
        if(line == NULL){
                PERR("ptr is NULL");
                return write_count;
        }
        int fd;
        bytes_t wr;
        
        fd = open(path, O_WRONLY);
        if(fd == -1){
               PCERR("open file: %s", path);
               return write_count;
        }
        if(write_offset != lseek(fd, write_offset, SEEK_SET)){
                PCERR("fault lseek in file: %lld: %s", (long long)write_offset, path);
                close(fd);
                return write_count;
        }
        foreach_in_list(line, line){
                if(line->type != main_editor_line_type_LINE)
                        continue;
                if(write_count <= 0){
                        close(fd);
                        PFUNC_END();
                        return write_count;
                }
                wr = write(fd, line->data, line->len);
                if(wr < 0){
                        PCERR("fault write to file");
                        close(fd);
                        return write_count;
                }
                write_count -= 1;
        }
        
        close(fd);
        PFUNC_END();
        return write_count;
}







/*
        GLOBAL FUNCTIONS
*/
char init_FileText(FileText * ftext, const unsigned char * eol_chs, unsigned char eol_chs_len)
{
        /*
                Инициализирует структуру FileText_init
                eol_chs  - байты конца строки
                eol_chs_len - кол-во байт
                Возвращает:
                        OK      0
                        ERR     -1
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
        ftext->alloc_mem = 0;
#endif
        PFUNC_END();
        return 0;
}

char read_from_file_FileText(FileText * ftext, const char * path)
{
        /*
                Открывает файл path и заполняет структуру ftext
                Возвращает:
                        OK      0
                        ERR     -1
        */
        PFUNC_START();
        if(ftext == NULL){
                PERR("ptr is NULL");
                return -1;
        }
        if(path == NULL){
                PERR("ptr is NULL");
                return -1;
        }
        Line * file;
        FilePos pos;
        
        pfinf(ftext, path);
#ifdef FILEOPT_USE_PTHREADS
        pthreads!
#else
        file = read_lines_path(path, ftext->eol_chs, ftext->eol_chs_len
#ifdef DBG_ALLOC_MEM
                                , &ftext->alloc_mem
#endif
                                );
#endif
        pos.prev = NULL;
        pos.next = NULL;
        pos.line = &ftext->lines_end;
        if(0 != insert_lines_by_pos_down_FileText(ftext, &pos, file)){
                PERR("fault insert new lines");
                free_lines_Line(ftext, file);
                return -1;
        }
        if(0 != add_free_line(ftext)){
                free_lines(ftext);
                return -1;
        }
        pfinf(ftext, path);
        
        PFUNC_END();
        return 0;
}

char write_to_file_FileText(FileText * ftext, const char * path)
{
        /*
                Функция записывает строки в файл
                Создает новый файл, либо
                полностью перезаписывает существующий
                Возвращает:
                        OK      0
                        ERR     -1
        */
        PFUNC_START();
        if(ftext == NULL){
                PERR("ptr is NULL");
                return -1;
        }
#ifdef FILEOPT_USE_PTHREADS
        pthreads!
#else
        int fd = open(path, O_CREAT | O_RDWR, 0x0666);
        if(fd < 0){
                PCERR("fault open file: %s", path);
                return -1;
        }
        if(0 != ftruncate(fd, ftext->esize)){
                PCERR("fault set size of file: %s", path);
                return -1;
        }
        if(0 != write_to_file_path(path, ftext->lines.next, 0, ftext->lines_count)){
                PERR("failt write data to file");
                return -1;
        }
#endif
        
        PFUNC_END();
        return 0;
}

char close_file_FileText(FileText * ftext)
{
        /*
                Освобождает память от структур
                Возвращает:
                        OK      0
                        ERR     -1
        */
        PFUNC_START();
        pfinf(ftext, "");
        if(ftext == NULL){
                PERR("ptr is NULL");
                return -1;
        }
        
        free_groups(ftext);
        free_lines(ftext);
        
        pfinf(ftext, "");
        PFUNC_END();
        return 0;
}

Line * get_line_by_idx_FileText(FileText * ftext, unsigned int idx)
{
        /*
                Находит строку по номеру (1 .. кол-во строк)
                Возвращает:
                        OK      Указатель на строку
                        ERR     NULL
        */
        PFUNC_START();
        if(ftext == NULL){
                PERR("ptr is NULL");
                return NULL;
        }
        
        Line * line = NULL;
        unsigned char gr_idx;
        unsigned int ln_idx;

        if(idx > ftext->lines_count){
                PERR("unexpected idx: %u", idx);
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
        
        ln_idx = (gr_idx * ftext->group_size) - (ftext->group_size / 2);
        if( ln_idx >= idx ){
                /*От начала группы*/
                ln_idx = ftext->group_size * (gr_idx - 1) + 1;
                foreach_in_list(line, ftext->lines_group[gr_idx - 1]){
                        if(line->type != main_editor_line_type_LINE)
                                continue;
                        if(ln_idx == idx){
                                PFUNC_END();
                                return line;
                        }
                        ln_idx += 1;
                }
        } else {
                /*От конца группы*/
                Line * start;
                if((gr_idx + 1) >= ftext->groups_count){
                        /*Если это последняя группа*/
                        start = ftext->lines_end.prev;
                        ln_idx = ftext->lines_count;
                } else {
                        if(ftext->lines_group[gr_idx] == NULL){
                                PERR("gr_idx is NULL: %hu", gr_idx);
                                return NULL;
                        }
                        start = ftext->lines_group[gr_idx]->prev;
                        ln_idx = ftext->group_size * gr_idx;
                }
                foreach_in_list_reverse(line, start){
                        if(line->type != main_editor_line_type_LINE)
                                continue;
                        if(ln_idx == idx){
                                PFUNC_END();
                                return line;
                        }
                        ln_idx -= 1;
                }
        }
        if(line == NULL){
                PERR("fault: %u - %u", ln_idx, idx);
        }
        
        return NULL;
}

char insert_lines_by_pos_down_FileText(FileText * ftext, FilePos * pos, Line * line)
{
        /*
                Вставляет группу линий line в позицию pos
                В FilePos должны быть заполнены:
                        line
                Линии встанут перед pos
                Например:
                вставить        l1-l2-l3 в L2 из L1-L2-L3-L4
                получится       L1-l1-l2-l3-L2-L3-L4
                Изменяется размер файла
                Возвращает:
                        OK      0
                        ERR     -1
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
                                PERR("fault insert item: %u", ftext->lines_count);
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
                        PERR("fault insert item: %u", ftext->lines_count);
                        return -1;
                }
                ftext->lines_count += 1;
                ftext->esize += tmp_line->len;
        }
        
        PFUNC_END();
        return create_lines_groups(ftext);
}

char insert_lines_by_pos_up_FileText(FileText * ftext, FilePos * pos, Line * line)
{
        /*
                Вставляет группу линий line в позицию pos
                В FilePos должны быть заполнены:
                        line
                Линии встанут за pos
                Например:
                вставить        l1-l2-l3 в L2 из L1-L2-L3-L4
                получится       L1-L2-l1-l2-l3-L3-L4
                Изменяется размер файла
                Возвращает:
                        OK      0
                        ERR     -1
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
                                PERR("fault insert item: %u", ftext->lines_count);
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
                        PERR("fault insert item: %u", ftext->lines_count);
                        return -1;
                }
                ftext->lines_count += 1;
                ftext->esize += tmp_line->len;
        }
        
        PFUNC_END();
        return create_lines_groups(ftext);
}

char cut_lines_by_pos_FileText(FileText * ftext, FilePos * pos)
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
                Изменяется размер файла
                Возвращает:
                        OK      0
                        ERR     -1
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
        Line * tmp_line;
        FilePos * fe_pos;
        Line new_lines_end;
        
        new_lines_end.prev = NULL;
        new_lines_end.next = NULL;
        new_lines_end.list_item_type = list_item_type_NOT_ERASE;
        new_lines_end.type = main_editor_line_type_LINE_END;
        
        Line * _cut_lines_by_pos_FileText(FileText * ftext, FilePos * pos)
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
                        new_line = read_line_str((const unsigned char *)pos->line->data + pos->ch_idx, pos->line->len - pos->ch_idx, ftext->eol_chs, ftext->eol_chs_len
#ifdef DBG_ALLOC_MEM
                                                        , &ftext->alloc_mem
#endif
                                                );
                        if(0 != erase_from_Line(ftext, pos->line, pos->ch_idx, new_line->len, 1)){
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
        first_line = _cut_lines_by_pos_FileText(ftext, pos);
        if(0 != insert_ListItem_offset_down((ListItem *)&new_lines_end, (ListItem *)first_line)){
                PERR("fault insert item");
                free_lines_Line(ftext, first_line);
                return -1;
        }

        foreach_in_list(fe_pos, pos->next){
                /*Разрезаем остальные строки*/
                new_line = _cut_lines_by_pos_FileText(ftext, fe_pos);
                if(0 != insert_ListItem_offset_down((ListItem *)&new_lines_end, (ListItem *)new_line)){
                        PERR("fault insert item");
                        free_lines_Line(ftext, first_line);
                        return -1;
                }
        }
        if(new_lines_end.prev != NULL)
                new_lines_end.prev->next = NULL;

        new_line = first_line->next;
        foreach_in_list(fe_pos, pos->next){
                /*Вставляем строки*/
                if(new_line != NULL){
                        tmp_line = new_line;
                        new_line = new_line->next;
                        if(0 != erase_ListItem((ListItem *)tmp_line)){
                                free_lines_Line(ftext, first_line);
                                break;
                        }
                        if(0 != insert_ListItem_offset_up((ListItem *)fe_pos->line, (ListItem *)tmp_line)){
                                free_lines_Line(ftext, first_line);
                                break;
                        }
                        ftext->lines_count += 1;
                        ftext->esize += tmp_line->len;
                } else
                        break;
        }
        /*Вставляем первую строку*/
        if(0 != erase_ListItem((ListItem *)first_line)){
                free_lines_Line(ftext, first_line);
                return create_lines_groups(ftext);
        }
        if(0 != insert_ListItem_offset_up((ListItem *)pos->line, (ListItem *)first_line)){
                free_lines_Line(ftext, first_line);
                return create_lines_groups(ftext);
        }
        ftext->lines_count += 1;
        ftext->esize += first_line->len;
        
        PFUNC_END();
        return create_lines_groups(ftext);
}

char fill_pos_FileText(FileText * ftext, FilePos * pos, Line * line, unsigned int ln_idx, bytes_t ch_idx, bytes_t len)
{
        /*
                Функция заполняет структуру FilePos
                Если line == NULL, то ищет строку по ln_idx
                Возвращает:
                        OK      0
                        ERR     -1
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
                pos->line = get_line_by_idx_FileText(ftext, ln_idx);
                if(pos->line == NULL){
                        PERR("line not found: %u", ln_idx);
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

char edit_lines_by_pos_FileText(FileText * ftext, FilePos * pos, const unsigned char * data, bytes_t data_len)
{
        /*
                Функция редактирует строки
                pos - позиция символа, с которого начнется редактирование
                В структуре pos должны быть заполнены поля:
                        pos->line
                        pos->ch_idx
                        pos->len
                pos->len - длина заменяемых данных, данные начиная с pos (включая этот символ) и заканчивая pos->len-1 будут заменены
                        при pos->len == 0, данные data будут записаны перед символом pos
                data - добавляемые данные
                        при data == NULL, данные начиная с pos (включая этот символ) и заканчивая pos->len-1 будут удалены
                data_len - размер добавляемых данных
                        при data_len == 0, данные начиная с pos (включая этот символ) и заканчивая pos->len-1 будут удалены
                
                Изменяется размер файла
                Изменяется mem alloc
                Возвращает:
                        OK      0
                        ERR     -1
                TODO:
                        удалить созданные строки в случае ошибки
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
        if(data_len < 0){
                PERR("unexpexted 'data_len': %lld", (long long)data_len);
                return -1;
        }
        
        Line * new_lines = NULL;
        Line * erase_line;
        Line * tmp_line;
        Line * last_line = NULL;
        Line erases_lines_end;
        bytes_t last_len;
        
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
                        if(0 != cut_lines_by_pos_FileText(ftext, pos)){
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
                /*Скорей-всего происходит вставка данных без удаления*/
                /*Делим первую строку на две*/
                if(0 != cut_lines_by_pos_FileText(ftext, pos)){
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
        if(data_len > 0 && data != NULL){
                /*Вставляем новые данные и строки*/
                /*Формируем строки из текста*/
                new_lines = read_lines_str((const unsigned char *)data, data_len, ftext->eol_chs, ftext->eol_chs_len
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

bytes_t get_data_by_pos_FileText(FileText * ftext, FilePos * pos, unsigned char * data, bytes_t data_len)
{
        /*
                Функция копирует байты в буфер
                pos - позиция символа, с которого начнется копирование (включая символ pos->ch_idx)
                В структуре pos должны быть заполнены поля:
                        pos->line
                        pos->ch_idx
                        pos->len
                pos->len - размер копируемых данных
                data - буфер назначения
                data_len - размер буфера
                
                Возвращает:
                        OK      кол-во записанных символов
                        ERR     -1
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
        if(data == NULL){
                PERR("ptr is NULL");
                return -1;
        }
        if(data_len <= 0){
                PERR("unexpexted 'data_len': %lld", (long long)data_len);
                return -1;
        }
        Line * line;
        bytes_t writed = 0;
        bytes_t offset = 0;
        
        foreach_in_list(line, pos->line){
                if(line->type != main_editor_line_type_LINE)
                        break;
                if(offset >= data_len || writed >= pos->len)
                        return writed;
                bytes_t wr = line->len > (pos->len - writed) ? line->len - (pos->len - writed) : line->len;
                memcpy(data + offset, line->data, wr);
                offset += wr;
                writed += wr;
        }
        
        PFUNC_END();
        return writed;
}