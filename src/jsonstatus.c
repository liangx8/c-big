#include <json-c/json.h>
#include <malloc.h>
#include <stdio.h>
#include "status.h"
#include "error_stack.h"

const char *default_src = "/home/com/big-data/qq-phone.bin";
const char *default_dst = "/home/tec/big/qq-sorted.bin";
int config_parse(struct STATUS *cfg, const char *cfg_file)
{
    /* 小内存的申请，忽略内存不足的错误 */
    struct json_object *obj=json_object_from_file(cfg_file);
    if (obj == NULL)
    {
        cfg->src = default_src;
        cfg->dst = default_dst;
        cfg->step1time=0;
        printf("使用缺省配置\n");
        return 0;
    }
    struct json_object *val;
    json_object_object_get_ex(obj,"dst",&val);
    cfg->dst=json_object_get_string(val);
    json_object_put(obj);
    return 0;
}
/**
 * @param stu struct status
 * @param fname file name of json
 * @returns -1 if something wrong
*/
int status_save(struct STATUS *stu,const char *fname){
    struct json_object *obj=json_object_new_object();
    struct json_object *val=json_object_new_string(stu->dst);
    json_object_object_add(obj,"dst",val);
    val=json_object_new_string(stu->src);
    json_object_object_add(obj,"src",val);
    val=json_object_new_int64(stu->step1time);
    json_object_object_add(obj,"step1time",val);
    if(json_object_to_file(fname,obj)==-1){
        ERROR(json_util_get_last_err());
        return -1;
    }
    json_object_put(obj);
    return 0;
}