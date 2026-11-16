#define _POSIX_C_SOURCE 200809L
#include "strlist.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#define STRS_CAPACITY 512
struct _strlist {
    size_t strs_capacity;
    size_t strs_top;
    size_t buff_capacity;
    size_t buff_top;
    const char **strs;
    char *buffer;
};

strlist *strlist_create(void) {
    struct _strlist *list = malloc(sizeof(struct _strlist));
    if (!list) {
        return NULL;
    }
    list->strs = (const char **)malloc(STRS_CAPACITY*sizeof(void *));
    if(list->strs == NULL){
        free(list);
        return NULL;
    }
    list->buffer=malloc(INIT_CAPACITY);
    if(list->buffer == NULL){
        free(list->strs);
        free(list);
        return NULL;
    }
    list->strs_capacity = STRS_CAPACITY;
    list->buff_capacity = INIT_CAPACITY;
    list->buff_top = 0;
    list->strs_top = 0;
    srand((unsigned)time(NULL));
	return list;
}

int strlist_shuffle(strlist *list)
{
    for (size_t ix = list->strs_top; ix > 1; ix--) {
        size_t other = (size_t)rand() % ix;
        const char *tmp = list->strs[ix - 1];
        list->strs[ix - 1] = list->strs[other];
        list->strs[other] = tmp;
    }
    return 0;
}
int strlist_add(strlist *list, const char *value) {
    // length include terminating byte('\n')
    size_t len=strlen(value)+1;
    // 增加后的位置
    size_t after_len=list->buff_top + len;
    // 把位置8字节对齐
    if(after_len & 0x7){
        after_len = (after_len + 0x08) & ~0x7;
    }
    if(after_len >= list->buff_capacity){
        size_t new_cap=list->buff_capacity+10240;
        void *tmp=realloc(list->buffer,new_cap);
        if(tmp==NULL){
            return -1;
        }
        list->buffer=tmp;
        list->buff_capacity=new_cap;
    }
    if(list->strs_top==list->strs_capacity){
        size_t new_cap=list->strs_capacity+1024;
        void *tmp=realloc(list->strs,sizeof(void *)*new_cap);
        if(tmp==NULL){
            return -1;
        }
        list->strs_capacity=new_cap;
        list->strs=tmp;
    }
    char *str_ptr=list->buffer+list->buff_top;
    memcpy(str_ptr,value,len);
    *(list->strs+list->strs_top)=str_ptr;
    list->strs_top ++;
    list->buff_top = after_len;
    return 0;
}

void strlist_clear(strlist *list) {
}

const char *strlist_get(const strlist *list, int index) {
    if(index >= list->strs_top){
        return NULL;
    }
    return *(list->strs + index);
}
size_t strlist_size(const strlist *list) {
    return list->strs_top;
}

void strlist_destory(strlist *list)
{
    free(list->strs);
    free(list->buffer);
    free(list);
}

strlist    *all_line(FILE *file){
    if(file == NULL) {
        return NULL;
    }

    strlist *list = strlist_create();
    if(list == NULL) {
        return NULL;
    }

    char *line = NULL;
    size_t capacity = 0;
    ssize_t length;
    while((length = getline(&line, &capacity, file)) != -1) {
        if(length > 0 && line[length - 1] == '\n') {
            line[--length] = '\0';
            if(length > 0 && line[length - 1] == '\r') {
                line[--length] = '\0';
            }
        }
        if(strlist_add(list, line) != 0) {
            free(line);
            strlist_destory(list);
            return NULL;
        }
    }
    free(line);

    if(ferror(file) || !feof(file)) {
        strlist_destory(list);
        return NULL;
    }
    return list;
}