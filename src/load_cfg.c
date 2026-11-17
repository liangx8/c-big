#include <wchar.h>
#include "tomlc17.h"
#include "cfg_data.h"
struct CFG_DATA load_cfg(const char *cfgname)
{
    struct CFG_DATA cfg;
    toml_result_t result=toml_parse_file_ex(cfgname);
    if(!result.ok){
        wprintf(L"文件格式错误:%s\n",result.errmsg);
        cfg.status=STATUS_BAD;
        return cfg;
    }
    cfg.list=strlist_create();
    toml_datum_t wallpaper_path=toml_seek(result.toptab,"wallpaper.path");
    if(wallpaper_path.type==TOML_STRING){
        strlist_add(cfg.list,wallpaper_path.u.s);
        cfg.wallpaper_path=strlist_get(cfg.list,0);
    }
    // strlist_add(cfg.list,resu)
    
    toml_free(result);
    cfg.status=STATUS_OK;
    return cfg;
}
void free_cfg(struct CFG_DATA cfg){
    strlist_destory(cfg.list);
}