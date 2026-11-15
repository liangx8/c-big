#include "strlist.h"

#include <stdlib.h>
#include <string.h>
#define STRS_CAPACITY 100
struct strlist {
    size_t strs_capacity;
    size_t strs_top;
    size_t buff_capacity;
    size_t buff_top;
    const char **strs;
    char buffer[];
};

strlist *strlist_create(void) {
    int cap=sizeof(strlist) + INIT_CAPACITY;
    struct strlist *list = malloc(cap);
    if (!list) {
        return NULL;
    }
    list->strs = (const char **)malloc(STRS_CAPACITY*sizeof(void *));
    if(list->strs == NULL){
        free(list);
        return NULL;
    }
    list->strs_capacity = STRS_CAPACITY;
    list->buff_capacity = INIT_CAPACITY;
    list->buff_top = 0;
    list->strs_top = 0;
	return list;
}

int strlist_add(strlist *list, const char *value) {
    // length include terminating byte('\n')
    size_t len=strlen(value)+1;
    size_t after_len=list->buff_top + len;
    if(after_len >= list->buff_capacity){
        // allocate memory,暂时错误退出
        return -1;
    }
    char *str_ptr=&(list->buffer[list->buff_top]);
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
    free(list);
}