#pragma once
struct CONF{
    int         uom;
    int         scope_cnt;
    const char *sortname;
    long        scope[];
};

struct CONF *load_config(const char *filename);