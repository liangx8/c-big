#include <wchar.h>
#include "log.h"
#include "tomlc17.h"
#include "cfg_data.h"
struct CFG_DATA load_cfg(const char *cfgname)
{
    struct CFG_DATA cfg;
    logname="/tmp/test.log";
    toml_result_t result=toml_parse_file_ex(cfgname);
    if(!result.ok){
        log_info(L"文件格式错误:%s\n",result.errmsg);
        cfg.status=STATUS_BAD;
        return cfg;
    }
    cfg.list=strlist_create();
    toml_datum_t wallpaper_path=toml_seek(result.toptab,"wallpaper.path");
    if(wallpaper_path.type==TOML_STRING){
        log_info(L"wallpaper path:%s",wallpaper_path.u.s);
        strlist_add(cfg.list,wallpaper_path.u.s); // 0
    }
    toml_datum_t log_file=toml_seek(result.toptab,"log.file");
    if(log_file.type==TOML_STRING){
        log_info(L"log file:%s",log_file.u.s);
        strlist_add(cfg.list,log_file.u.s); // 1
    }
    toml_datum_t wallpaper_count_total=toml_seek(result.toptab,"wallpaper.count_total");
    if(wallpaper_count_total.type==TOML_INT64){
        cfg.wallpaper_count_total=wallpaper_count_total.u.int64;
    }
    cfg.wallpaper_path=strlist_get(cfg.list,0);
    cfg.logname=strlist_get(cfg.list,1);
    toml_free(result);
    cfg.status=STATUS_OK;
    return cfg;
}
void free_cfg(struct CFG_DATA cfg){
    strlist_destroy(cfg.list);
}