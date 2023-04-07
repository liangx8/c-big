
#include <jansson.h>
#include <malloc.h>
#include <stdio.h>
#include <stdint.h>

#include "error_stack.h"


struct STATUS
{
    const char *src;
    const char *dst;
    uint64_t step1time;
    int64_t pivot_index;
    int64_t total;
    int64_t *scope; // 准备要处理的分区
    json_t *root;
};


#define get_string(json,key,val,left)  val=json_object_get(json,key); left=json_string_value(val)
#define get_int(json,key,val,left)  val=json_object_get(json,key); left=json_integer_value(val)


const char *default_src = "/home/com/big-data/qq-phone.bin";
const char *default_dst = "/home/tec/big/qq-sorted.bin";
/**
 * 从jansson 中获取存由malloc获取的值(例如 char *)时,得到的值依赖json_t的reference count,
 * 如果值到了0，获取的值就会失效
  * @param fname 读取文件名
 */
struct STATUS * status_load_file_or_new(const char *fname)
{

    json_t *obj,*val;
    json_error_t error;
    struct STATUS *sta=malloc(sizeof(struct STATUS));
    sta->scope=NULL;
    sta->root=NULL;
    

    obj = json_load_file(fname, 0, &error);
    if (obj == NULL)
    {
        sta->src = default_src;
        sta->dst = default_dst;
        sta->step1time = 0;
        printf("读状态文件错误(%s)\n使用缺省配置\n", error.text);
        return sta;
    }
    /*
    json_object_foreach(obj,key,val){

        if (json_is_string(val)){
            printf("key: %s,value: %s\n",key,json_string_value(val));
        } else {
            printf("key: %s\n",key);
        }
        
    }*/

    sta->root=obj;
    
    
    get_string(obj,"src",val,sta->src);
    get_string(obj,"dst",val,sta->dst);
    get_int(obj,"pivot-index",val,sta->pivot_index);
    get_int(obj,"total",val,sta->total);
    return sta;
}
void status_print(struct STATUS *stu){
    printf("src:%s\ndst:%s\npivot-index: %ld, total: %ld\n",stu->src,stu->dst,stu->pivot_index,stu->total);
}
/**
 * @param stu struct status
 * @param fname file name of json
 * @returns -1 if something wrong
 */
int status_save(struct STATUS *stu, const char *fname)
{
    json_t *obj = json_object();
    json_object_set_new(obj, "src", json_string(stu->src));
    json_object_set_new(obj, "dst", json_string(stu->dst));
    json_object_set_new(obj, "step1time", json_integer(stu->step1time));
    json_object_set_new(obj, "pivot-index", json_integer(stu->pivot_index));
    json_object_set_new(obj, "total", json_integer(stu->total));

    if (json_dump_file(obj, fname, 0))
    {
        ERROR_BY_ERRNO();
        return -1;
    }
    json_decref(obj);
    return 0;
}
void status_free(struct STATUS *sta){
    if(sta->root)
        json_decref(sta->root);
    free(sta);
}