#ifndef STRS_H_KKDW323JJJ3K3
#define STRS_H_KKDW323JJJ3K3
struct STRS{
    int size;
    const char *strs[];
};
struct STRS *strs_load(const unsigned char *,void *);


int calc_strss(const char *buf,int);
#endif