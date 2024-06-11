
#include <time.h>
#include <stdio.h>
#include <wchar.h>
#include <errno.h>
#include "werror.h"
#define NANO_MASK 1000000000
//int asprintf(char **strp, const char *fmt, ...);
/**
毫秒ms，微秒us，纳秒ns
struct timespec中的tv_nsec最大值为 999999999,即单位是纳秒


*/

/**
 * @brief 获得当前时间
 * @param tm 必须是非空的指针，函数不对该指针做非空判断
 * @return 0失败,非0，当前实际戳
 */
long now(){
    struct timespec ts;
    if(clock_gettime(CLOCK_REALTIME,&ts)==-1){
        WERRNO(errno);
        return 0;
    }
    return (ts.tv_sec *NANO_MASK) + ts.tv_nsec;
}
const char *fmt_dhms="%dday %02d:%02d:%02d %ld";
const char *fmt_hms="%02d:%02d:%02d %ld";
/**
 * @param buf 
*/
int timestamp_str(char *buf,long tm){
    long nsec=tm % NANO_MASK;
    tm =tm / NANO_MASK;
    int sec =tm % 60;
    tm = tm / 60; // min
    int min=tm % 60;
    tm = tm /60 ; // hour
    int hour= tm % 24;
    tm = tm /24;
    if(tm){
        return sprintf(buf,fmt_dhms,tm,hour,min,sec,nsec);
    } else {
        return sprintf(buf,fmt_hms,hour,min,sec,nsec);
    }

}


// void find_max(void){
//     struct timespec ts;
//     clock_gettime(CLOCK_REALTIME,&ts);
//     time_t start=ts.tv_sec;
//     time_t sc=0;
//     time_t nsec_max=ts.tv_nsec;
//     while(sc<60){
//         clock_gettime(CLOCK_REALTIME,&ts);
//         sc=ts.tv_sec-start;
//         if(nsec_max < ts.tv_nsec){
//             nsec_max=ts.tv_nsec;
//         }
//     }
//     printf("max nono-second%ld\n",nsec_max);
// }
/*
golang 2217.266157861s = 2217266157861(0x204 3f5c9925)
分析: 1s = 1000000000
            999999999
*/