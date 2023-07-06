
#include <jansson.h>
#include <malloc.h>
#include <stdio.h>
#include <stdint.h>

#include "timestamp.h"
#include "error_stack.h"
#pragma pack(push,4)
struct STATUS
{
    const char *src;
    const char *dst;
    const char *preform_src;
    const char *preform_dst;
    uint64_t   step1time;
    uint64_t   step2time;
    // 准备要处理的分区,快速排序中partition的结果,必须成对出现
    int64_t    *scope;
    int        scope_cnt;
    // 0 建立目标文件阶段, 1 在０阶段中断，２在目标文件上排序
    int        step;
    json_t *root;
};
#pragma pack(pop)
#define get_string(json, key) json_string_value(json_object_get(json, key))
#define get_int(json, key) json_integer_value(json_object_get(json, key))

const char *default_src = "big/qq-test.bin";
const char *default_dst = "big/qq-sorted.bin";
/**
 * @brief
 * 从jansson 中获取存由malloc获取的值(例如 char *)时,得到的值依赖json_t的reference count,
 * 如果值到了0，获取的值就会失效
 * @param fname 读取文件名
 */
struct STATUS *status_file_load_or_new(const char *fname)
{

    json_t *obj;
    json_error_t error;
    struct STATUS *sta = malloc(sizeof(struct STATUS));
    sta->scope = NULL;
    sta->root = NULL;

    obj = json_load_file(fname, 0, &error);
    if (obj == NULL)
    {
        sta->src           = default_src;
        sta->dst           = default_dst;
        sta->step1time     = 0;
        sta->step          = 0;
        printf("读状态文件错误(%s)\n使用缺省配置\n", error.text);
//        sta->root=json_object();
        return sta;
    }

    sta->root = obj;

    sta->src            = get_string(obj, "src");
    sta->dst            = get_string(obj, "dst");
    sta->step1time      = get_int   (obj, "step1time");
    sta->step2time      = get_int   (obj, "step2time");
    sta->step           = get_int   (obj, "step");
    switch (sta->step)
    {
    case 0:
        sta->scope=NULL;
        break;
    case 1:
    {
        json_t *ary = json_object_get(obj, "scope");
        size_t size = json_array_size(ary);
        if(size!=3){
            printf("状态文件内容不完整，对数据从新排序\n");
            sta->step=0;
        }
        sta->scope=malloc(sizeof(int64_t)*3);
        json_t *json = json_array_get(ary, 0);
        sta->scope[0] = json_integer_value(json);
        json = json_array_get(ary, 1);
        sta->scope[1] = json_integer_value(json);
        json = json_array_get(ary, 2);
        sta->scope[2] = json_integer_value(json);
    }
    break;
    case 2:
    {
        json_t *ary = json_object_get(obj, "scope");
        size_t size = json_array_size(ary);
        if(size & 1){
            printf("状态文件内容不完整，对数据从新排序\n");
            sta->step=0;
            break;
        }
        sta->scope_cnt = size;
        sta->scope = malloc(sizeof(int64_t) * size);
        for (size_t ix = 0; ix < size; ix++)
        {
            json_t *json = json_array_get(ary, ix);
            sta->scope[ix] = json_integer_value(json);
        }
    }
    default:
    break;
    }
    return sta;
}
void status_print(struct STATUS *stu)
{
    char tmstr[30];
    timestamp_str(tmstr,stu->step1time);
    printf("Status -------------------------------------------\n");
    printf("       src: %s\n       dst: %s\n      step: %d\n", stu->src, stu->dst,stu->step);
    printf("step1 time: %s\n",tmstr);
    if(stu->step==1){
        printf("scope [%10ld,%10ld,%10ld]\n", stu->scope[0],stu->scope[1],stu->scope[2]);
    } 
    if(stu->step==2){
        if(stu->step2time){
            timestamp_str(tmstr,stu->step1time);
            printf("step2 time: %s\n",tmstr);
        }
        for (int ix = 0; ix < stu->scope_cnt; ix+=2)
        {
            printf("scope [%010ld,%010ld]\n", stu->scope[ix],stu->scope[ix+1]);
        }

    }
    printf("--------------------------------------------------\n");
}
/**
 * @param stu struct status
 * @param fname file name of json
 * @returns -1 if something wrong
 */
int status_save(struct STATUS *stu, const char *fname)
{
    json_t *obj = json_object();
    json_object_set_new(obj, "src",       json_string(stu->src));
    json_object_set_new(obj, "dst",       json_string(stu->dst));
    json_object_set_new(obj, "step1time", json_integer(stu->step1time));
    json_object_set_new(obj, "step2time", json_integer(stu->step2time));
    json_object_set_new(obj, "step",      json_integer(stu->step));
    if (stu->scope_cnt)
    {
        json_t *ary = json_array();
        for (int ix = 0; ix < stu->scope_cnt; ix++)
        {
            json_array_append_new(ary, json_integer(stu->scope[ix]));
        }
        json_object_set_new(obj, "scope", ary);
    }
    // json_object_set_new(obj, "total", json_integer(stu->total));

    if (json_dump_file(obj, fname, 0))
    {
        ERROR_BY_ERRNO();
        return -1;
    }
    json_decref(obj);
    return 0;
}
void status_free(struct STATUS *sta)
{
    if (sta->root)
        json_decref(sta->root);
    free(sta);
}