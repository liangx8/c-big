#include <malloc.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "../error_stack.h"

//void hex(const char *,int);

//#define PR(fmt,args...) printf("%s(%3d):" fmt ,__FILE__,__LINE__,args)
#define PR(fmt,args...) do {} while(0);

#define pair(p1,p2) ((((uint64_t)(p1)) << 32) + p2)

#define pa1(p) (uint32_t)(p>>32)
#define pa2(p) (uint32_t)(p & 0xffffffff)

struct strs{
    int num;
    char *ss[];
};
int chinaid(char *dst,uint64_t id)
{
    int isuf=id & 1;
    uint64_t real=id >> 1;
    sprintf(dst,"%lu",real);
    if (isuf){
        dst[17]='X';
    }
    return 0;
}
uint64_t tr_str(char *dst,const char *src){
    uint32_t dcnt=0; // dst counter
    uint32_t scon=0; // src consumer
    uint16_t cc=0;
    if (*src & 0x80){
        cc= (*(src + scon) & 0x7f) * 256;
        scon ++;
        cc += *(src + scon);
    } else {
        cc = *(src + scon);
    }
    scon ++;
    for (int ix=0;ix<cc;ix++){
        int cnt=wctomb(dst+dcnt,*((uint16_t *)(src+scon)));
        if (cnt<0){
            return 0;
        }
        scon +=2;
        dcnt +=cnt;
    }
    *(dst+dcnt)=0;
    dcnt ++;
    return pair(dcnt,scon);
}
/**
 * @brief 把wide char string转换成multi-bytes string
 * @param src 中保存的是只有２字节的wide char, 开始的２个字节是整个字符串组的长度
 * @param cum 在src消耗的字节
*/
struct strs *strs_load(const uint8_t *src,uint16_t *cum)
{
    uint16_t slen= *((uint16_t *)src);
    int scnt=2;

    // 统计字符串
    int cnt=0;
    uint16_t sum=2;
    while(sum<slen){
        uint16_t u16;

        if(*(src + sum) & 0x80){
            u16 = (*(src + sum) & 0x7f) * 256;
            sum ++;
            u16 += *(src + sum);
            sum ++;
        } else {
            u16=*(src + sum);
            sum ++;
        }
        cnt++;
        sum +=u16*2;
        if ((sum-2) > slen){
            ERROR("数据错误");
            return NULL;
        }
    }
    // 需要保留的空间
    int expected_size=(slen * 3 / 2) + (cnt+1) * sizeof(void*);
    PR("expect size:%d, number of strings:%d\n",expected_size,cnt);
    char *dst=malloc(expected_size);
    struct strs *ss=(struct strs *)dst;
    PR("pointer:%lx\n",(long)dst);
    int dcnt=sizeof(void*)*(cnt+1);
    ss->num=cnt;
    int idx=0;
    scnt=2; // 重置源索引值
    PR("base %lx\n",(long)dst);
    while(idx < cnt){
#if 1
        uint64_t pa=tr_str(dst + dcnt,(char *)(src+scnt));
        if (pa==0){
            ERROR("数据错误");
            free(dst);
            return NULL;
        }
        //printf("%2d[%3d][%s]\n",idx,dcnt,dst+dcnt);
        ss->ss[idx]=dst + dcnt;
        dcnt += pa1(pa);
        PR("next %4d %lx\n",dcnt,(long)(&ss->ss[idx]));
        scnt += pa2(pa);
#else
        long addr=(long)(&ss->ss[idx]);
        ss->ss[idx]=(const char *)idx;
        PR("%2d %lx\n",idx,addr);
#endif
        idx ++;
    }
    if ((scnt - slen) != 2){
        free(dst);
        ERROR("字符串数据错误");
        return NULL;
    }
    //hex(dst,expected_size);
    *cum=scnt;
    return (struct strs *)dst;
}
/**
 * @brief 计数有几个字符串组
 * @param buf　输入的数据
 * @param size buf的大小
 * @return　返回　统计值,或者-1意味buf不是正确的数据
*/
int calc_strss(const uint8_t *buf,uint32_t size)
{
    int calc=0;
    uint32_t sum=0;
    const uint8_t *p8;
    while(1){
        p8 = buf + sum;
        const uint16_t *p16=(const uint16_t *)p8;
        uint16_t i16=*p16;
        sum += i16+2;
        calc++;
        if(sum==size){
            break;
        }
        if(sum>size){
            return -1;
        }
    }
    return calc;

}