#include <jansson.h>
#include "timestamp.h"
#include "werror.h"
#include "breakinfo_struct.h"

#define GET_STRING(obj,key) json_string_value(json_object_get(obj,key))
#define GET_INT(obj,key)    json_integer_value(json_object_get(obj,key))
struct _bi {
    struct break_info data;
    json_t *root;
};

void break_info_print(const struct break_info *bi)
{
    char tmstr[64];
    if(bi->time1){
        timestamp_str(tmstr,bi->time1);
        wprintf(L"第一阶段用时:%s\n",tmstr);
    }
    if(bi->time2){
        timestamp_str(tmstr,bi->time2);
        wprintf(L"第二阶段用时:%s\n",tmstr);
    }
    for(int ix=0;ix<bi->scope_cnt;ix++){
        wprintf(L"%ld\n",bi->scope[ix]);
    }

}

const char* key_index     = "index";
const char* key_scope     = "scope";
const char* key_time1     = "time1";
const char* key_time2     = "time2";

/**
 * @brief 读取或者新建
 */
struct _bi *break_info_load(const char *name)
{

    struct _bi *bi=malloc(sizeof(struct _bi));
    if(name== NULL){
        bi->root=json_object();
        return bi;
    }
    json_t *json;
    json_error_t error;
    json=json_load_file(name,0,&error);
    if (json == NULL){
        wprintf(L"读json文件错误:%s\n",error.text);
        WERR_NO_MSG(errno,L"file:%s:json error:%s",name,error.text);
        free(bi);
        return NULL;
    }
    
    bi->data.indexname = GET_STRING(json,key_index);
    bi->data.time1     = GET_INT(json,key_time1);
    bi->data.time2     = GET_INT(json,key_time2);
    json_t *ary=json_object_get(json,key_scope);
    int size=json_array_size(ary);
    if(size){
        bi->data.scope_cnt=size;
        bi->data.scope=malloc(sizeof(long)*size);
        for(int ix=0;ix<size;ix++){
            bi->data.scope[ix]=json_integer_value(json_array_get(ary,ix));
        }
    }
    bi->root=json;
    return bi;
}
int break_info_free(struct _bi *bi)
{
    if(bi->data.scope_cnt){
        free(bi->data.scope);
    }
    json_decref(bi->root);
    free(bi);
    return 0;
}

int break_info_save(struct _bi *bi,const char *fname)
{
    json_object_set_new(bi->root,key_index    ,json_string(bi->data.indexname));
    if(bi->data.time1){
        json_object_set_new(bi->root,key_time1    ,json_integer(bi->data.time1));
    }
    if(bi->data.time2){
        json_object_set_new(bi->root,key_time2    ,json_integer(bi->data.time2));
    }
    if(bi->data.scope_cnt){
        json_t *ary=json_array();
        for(int ix=0;ix<bi->data.scope_cnt;ix++){
            json_array_append_new(ary,json_integer(bi->data.scope[ix]));
        }
        json_object_set_new(bi->root,key_scope,ary);
    }

    if(json_dump_file(bi->root,fname,JSON_INDENT(4))){
        WERR_NO_MSG(errno,L"保存中断信息出错%s",fname);
        return -1;
    }
    return 0;
}