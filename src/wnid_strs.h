#ifndef WNID_STRS_H_K34H4GHHJ435
#define WNID_STRS_H_K34H4GHHJ435
#include <wchar.h>
struct BUFFER{
    int size;
    int idx;
    const char data[];
};

struct wstrs{
    long cnt;
    const wchar_t *str[];
};

int load_wstr(wchar_t *,struct BUFFER *);

struct wstrs* load_wstrs(struct BUFFER *);



#endif