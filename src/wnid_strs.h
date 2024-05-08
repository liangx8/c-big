#ifndef WNID_STRS_H_K34H4GHHJ435
#define WNID_STRS_H_K34H4GHHJ435
struct wstrs{
    long cnt;
    const wchar_t *str[];
};

#define pair(p1,p2) ((((uint64_t)(p1)) << 32) + (p2))

#define pa1(p) (uint32_t)(p>>32)
#define pa2(p) (uint32_t)(p & 0xffffffff)
uint64_t load_wstr(wchar_t *,const char *);

struct wstrs* load_wstrs(const char *);

#endif