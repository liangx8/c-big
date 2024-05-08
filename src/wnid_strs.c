#include <wchar.h>
#include <stdint.h>
#include "werror.h"
#include "wnid_strs.h"

/**
 * @brief 读取字符串,src中消耗的数字要根据,结果是否大于127来是否会多一个字节
 * 例如：返回值是5,src就被消耗了　5 * 2 + 1=11,如果是128,消耗了128 *2 +2=258
 * @param dst 存放结果，必须是有效内存空间
 * @param src 数据库中的裸数据，每个字符都是用２个字节保存的unicode.开始的1/2个字节是字符串的长度
 * @return dst中存放字符的数量(不包含'\0')
*/
uint64_t load_wstr(wchar_t *dst,const char *src){
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
    if(dst==NULL){
        return cc;
    }
    scon ++;
    for (uint16_t ix=0;ix<cc;ix++){
        uint16_t u16=*((uint16_t *)(src + scon +ix *2));
        dst[ix]=u16;
    }
    dst[cc]=0;
    return pair(cc,scon + cc *2);
}
struct wstrs* load_wstrs(const char *src)
{
    WERROR(L"功能没实现","");
    return NULL;
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
            WERROR(L"数据不完整","");
            return -1;
        }
    }
    return calc;

}