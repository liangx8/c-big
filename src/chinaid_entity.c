#include "entity.h"

int id_lt(const unsigned long *l,const unsigned long *r)
{
    return *l < *r;
}
int id_cmp(const unsigned long *l,const unsigned long *r)
{
    unsigned long lv=*l;
    unsigned long rv=*r;
    if(lv<rv) return -1;
    if(lv>rv) return 1;
    return 0;
}
long id_index(const char *fname,char **buf)
{
    return 0;
}
const struct Entity chinaid_entity={
    (int  (*)(const void*,const void*)) id_lt,
    (int  (*)(const void*,const void*)) id_cmp,
    (long (*)(const char*,void **))     id_index,
    {99,104,105,110,97,105,100,32,32,32,32,32,32,32,32,32},
    16
};
