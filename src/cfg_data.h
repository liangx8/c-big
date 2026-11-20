#ifndef CFG_DATA_H
#define CFG_DATA_H
#include "strlist.h"
struct CFG_DATA{
    const char* logname;
    const char* wallpaper_path;
    strlist *list;
    int wallpaper_count_total;
    int status;
};
#define STATUS_OK 1
#define STATUS_BAD -1

struct CFG_DATA load_cfg(const char *cfgname);
void free_cfg(struct CFG_DATA cfg);
#endif
