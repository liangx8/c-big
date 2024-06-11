#include <wchar.h>
#include <stdint.h>
#include <malloc.h>
#include "werror.h"
#include "wnid_strs.h"

struct BUFFER *new_buffer(int size){
    struct BUFFER *buf=(struct BUFFER *)malloc(size + sizeof(int)*2);
    buf->idx=0;
    buf->size=size;
    return buf;

}
/**
 * @brief 读取字符串,src中消耗的数字要根据,结果是否大于127来是否会多一个字节
 * 例如：返回值是5,src就被消耗了　5 * 2 + 1=11,如果是128,消耗了128 *2 +2=258
 * @param dst 用于存放结果的内存
 * @param src 的data字段保存数据库中的裸数据，每个字符都是用２个字节保存的unicode.开始的1/2个字节是字符串的长度
 * @return dst中存放字符的数量(不包含'\0')
*/
int load_wstr(wchar_t *dst,struct BUFFER *src)
{
    return -1;
}
/**
 * @brief 计数有几个字符串组
 * @param buf　输入的数据
 * @param size buf的大小
 * @return　返回　统计值,或者-1意味buf不是正确的数据
*/
int calc_strss(const char *buf,uint32_t size)
{
    int calc=0;
    uint32_t sum=0;
    const char *p8;
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
            WERROR(L"数据不完整%d,%d",sum,size);
            return -1;
        }
    }
    return calc;

}
#define STRARRAY_MAX      128
/**
 * @brief 读取源数据中的字符串组
 * @param src 源数据是字符串组数据，格式：开始２个字节为组占用的字节数，src->idx会被修改
 * @return 返回地址是一个整块，能用free直接释放
*/
struct wstrs* load_wstrs(struct BUFFER *src)
{
    const char *base=&src->data[src->idx];
    uint16_t *strpos=malloc(sizeof(uint16_t)*STRARRAY_MAX);
    int offset_end=*((const uint16_t*)base);
    int sidx=2;
    int didx=0;
    int chcnt=0;
    int strcnt=0;
    while(1){
        int num=0;
        if(*(base+sidx) & 0x80){
            num = (*(base+sidx) & 0x7f) << 8;
            sidx ++;
        }
        num += *(base + sidx);
        *(strpos+strcnt)=didx;
        sidx += num * 2 + 1;
        didx += (num + 1) * sizeof(wchar_t);
        chcnt += num;
        strcnt++;
        if(sidx == offset_end + 2){
            break;
        }
        if(sidx > offset_end + 2){
            CP_MSG(L"内部错误%d/%d/%d\n",num,sidx,offset_end);
            return NULL;
        }
    }
    // 需求的内存大小= 字串数sizeof(long) + 字串指针数组strcnt * sizeof(void *) + 数组的０结尾strcnt * sizeof(wchar_t) + sizeof(wchar_t) * chcnt
    //统计有几个字符串
    char *buf=malloc(sizeof(long)+strcnt * sizeof(void *) + strcnt * sizeof(wchar_t) + sizeof(wchar_t) * chcnt);
    const int base_offset=sizeof(long) + strcnt * sizeof(void *);
    struct wstrs *wss=buf;
    wss->cnt=strcnt;
    for(int ix=0;ix<strcnt;ix++){
        char *p8=buf + base_offset + strpos[ix];
        wss->str[ix]=(const wchar_t *)p8;
    }
    sidx=2;
    while(1){
        int num=0;
        if(*(base+sidx) & 0x80){
            num = (*(base+sidx) & 0x7f) << 8;
            sidx ++;
        }
        num += *(base + sidx);

    }


    src->idx += offset_end+2;
    return wss;
}
