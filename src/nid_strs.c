#include <malloc.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "error_stack.h"

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
/**
 * @brief 转换uint64_t成为身份证号
 * @param dst, 转换结果存放身份证号的内存空间指针,必须是非空值
 * @param id,  uint64_t身份证号存放的数字形式
 * @return alway 0
*/
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
/**
 * @brief 把储存unicode的内存变成multbyte(utf-8)
 * @param dst, 储存multbyte的内存区域，最后会加0字最后
 * @param src, 以长度开始，小于128为1字节,否则为2字节,跟随保存的是unicode16的字节
*/
uint64_t tr_str(char *dst,const char *src){
    uint32_t dcnt=0; // dst counter
    uint32_t scon=0; // src consumer
    uint16_t cc=0;
    if (*src & 0x80){
        uint8_t u8 =*(src + scon);
        cc= ( u8 & 0x7f) * 256;
        scon ++;
        u8 =*(src + scon);
        cc += u8;
    } else {
        uint8_t u8 =*(src + scon);
        cc = u8;
    }
    scon ++;
    for (int ix=0;ix<cc;ix++){
        int cnt=wctomb(dst+dcnt,*((uint16_t *)(src+scon)));
        if (cnt<0){
            ERROR("是错误吗?");
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
 * @brief 把src中的数据转换成struct STRS,结构
 * @param src 中保存的是只有２字节的wide char, 开始的２个字节是整个字符串组的大小(不包含本身)
 * @param cum 在src消耗的字节
 * @return 返回的结构是在一整快内存申请中。只需要直接free就可以
*/
struct strs *strs_load(const uint8_t *src,uint16_t *cum)
{
    // 此方法有coredump 发生
    uint16_t slen= *((uint16_t *)src);
    int scnt=2;

    
    // 统计字符串
    int cnt=0;
    while(scnt<((int)slen+2)){
        uint16_t u16;
        uint8_t u8;

        if(*(src + scnt) & 0x80){
            u8 = *(src + scnt) & 0x7f;
            u16=((uint16_t)u8)*256;
            scnt ++;
            u8=*(src + scnt);
            u16 += u8;
        } else {
            u8=*(src + scnt);
            u16=(uint16_t)u8;
        }
        scnt ++;
        cnt++;
        // unicode格式保存，2字节的保存长度

        scnt +=u16*2;
        //printf("%s(%d)scnt:%d, %d\n",__FILE__,__LINE__,scnt,u16*2);
    }
    //CP_MSG("check point - slen:%d,%d\n",slen,slen>500);
    if(scnt - slen != 2){
        ERRORV("字符串数据错误(scnt:%d,slen:%d)",scnt,slen);
        return NULL;
    }
    // 需要保留的空间,简单的认为muti-byte会占用3个字节
    int expected_size=(slen * 3 / 2) + (cnt+1) * sizeof(void*);
    // if (slen > 500){
    //     ERRORV("源长度:%d,cnt:%d,expected_size:%d\n",slen,cnt,expected_size);
    //     return NULL;
    // }
    char *dst=malloc(expected_size);
    if(dst == NULL){
        ERROR_BY_ERRNO();
        return NULL;
    }
    struct strs *sts=(struct strs *)dst;
    int dcnt=sizeof(void*)*(cnt+1);
    sts->num=cnt;
    scnt=2; // 重置源索引值
    for(int idx=0;idx<cnt;idx++){
        uint64_t pa=tr_str(dst + dcnt,(char *)(src+scnt));
        if (pa==0){
            ERROR_WRAP();
            free(dst);
            return NULL;
        }
        //CP_MSG("%2d[%3d][%s]\n",idx,dcnt,dst+dcnt);
        sts->ss[idx]=dst + dcnt;
        dcnt += pa1(pa);
        scnt += pa2(pa);
        if(dcnt >= expected_size){
            ERROR("程序设计错误");
            free(dst);
            return NULL;
        }
    }
    if ((scnt - slen) != 2){
        free(dst);
        ERROR("字符串数据错误");
        return NULL;
    }
    //hex(dst,expected_size);
    *cum=scnt;
    return sts;
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
            ERROR("数据不完整\n");
            return -1;
        }
    }
    return calc;

}