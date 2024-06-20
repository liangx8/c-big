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
    const uint8_t *base=(const uint8_t *)(&src->data[src->idx]);
    int idx=0;
    int cnt =0;
    if( *base & 0x80){
        uint8_t u8=*base;
        cnt= (u8 & 0x7f) << 8;
        idx ++;
    }
    uint8_t u8=*(base+idx);
    idx ++;
    cnt += u8;
    if(dst){
        for(int ix=0;ix<cnt;ix++){
            const uint8_t *p8=base +idx +ix *2;
            const uint16_t *p16=(const uint16_t *)p8;
            uint16_t u16=*p16;
            dst[ix]=(wchar_t)u16;
            //wprintf(L"%lx\n",(long)&dst[ix]);
        }
        dst[cnt]=0;
        //CP_MSG(L"%d\n",cnt);
        //CP_MSG(L"%d/%d/%d\n",src->idx,idx,cnt);
    }
    src->idx +=idx + cnt * 2;
    return cnt * sizeof(wchar_t);
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
uint16_t *strpos=NULL;
/**
 * @brief 读取源数据中的字符串组
 * @param src 源数据是字符串组数据，格式：开始２个字节为组占用的字节数，src->idx会被修改
 * @return 返回地址是一个整块，能用free直接释放
*/
struct wstrs* load_wstrs(struct BUFFER *src)
{
    char *base=(char *)(&src->data[src->idx]);
    if(strpos==NULL){
        strpos=malloc(sizeof(uint16_t)*STRARRAY_MAX);
    }
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
            CP_MSG(L"内部错误%d/%d/%d[%2x,%2x]\n",num,sidx,offset_end,src->data[src->idx],src->data[src->idx+1]);

            return NULL;
        }
    }
    
    // 需求的内存大小= 字串数sizeof(long) + 字串指针数组strcnt * sizeof(void *) + 数组的０结尾strcnt * sizeof(wchar_t) + sizeof(wchar_t) * chcnt
    //统计有几个字符串
    //CP_MSG(L"==>字符串:%d,字符:%d\n",strcnt,chcnt);
    char *buf=malloc(sizeof(long)+strcnt * sizeof(void *) + strcnt * sizeof(wchar_t) + sizeof(wchar_t) * chcnt);
    const int buf_offset=sizeof(long) + strcnt * sizeof(void *);
    struct wstrs *wss=(struct wstrs *)buf;
    wss->cnt=strcnt;

    sidx=2;
    int strcnt1=0;
    while(1){
        int num=0;
        if(*(base+sidx) & 0x80){
            num = (*(base+sidx) & 0x7f) << 8;
            sidx ++;
        }
        num += *(base + sidx);
        sidx ++;
        char *p8=(char *)(buf + buf_offset + strpos[strcnt1]);
        wchar_t *wstr=(wchar_t *)p8;
        p8=base + sidx;
        
        for(int ix=0;ix<num;ix++){
            wchar_t *pwc=wstr+ix;
            const uint16_t *p16=(const uint16_t *)(p8+ix*2);
            uint16_t wc=*p16;
            *pwc=(wchar_t)wc;
            //wprintf(L"%lx:%lc,%4x\n",(long)pwc,(wchar_t)wc,wc);
        }
        
        *(wstr+num)=L'\0';
        wss->str[strcnt1]=wstr;
        //CP_MSG(L"字符串%2d有个字符%d（%ls）\n",strcnt1,num,wss->str[strcnt1]);

        sidx +=num * 2;
        strcnt1 ++;
        if(sidx == offset_end + 2){
            // 此时 strcnt1 一定等于strcnt
            if(strcnt1 != strcnt){
                CP_MSG(L"错误%d,%d\n",strcnt1,strcnt);
                return NULL;
            }
            break;
        }
        if(sidx > offset_end + 2){
            CP_MSG(L"内部错误%d/%d/%d\n",num,sidx,offset_end);
            return NULL;
        }
    }


    src->idx += offset_end+2;
    return wss;
}
