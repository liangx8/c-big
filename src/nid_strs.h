#ifndef STRS_H_KKDW323JJJ3K3
#define STRS_H_KKDW323JJJ3K3
struct STRS{
    int size;
    const char *strs[];
};
struct STRS *strs_load(const unsigned char *,void *);

#define pa1(p) (unsigned int)(p>>32)
#define pa2(p) (unsigned int)(p & 0xffffffff)


unsigned long tr_str(char *dst,const char *src);
int chinaid(char *dst,uint64_t id);
int calc_strss(const char *buf,int);
#endif