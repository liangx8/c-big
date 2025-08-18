#include <stdio.h>
#include <wchar.h>
#include <time.h>
#include <stdarg.h>

time_t find_max(void){
    /*此程序的结果接近999999999*/
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME,&ts);
    time_t start=ts.tv_sec;
    time_t sc=0;
    time_t nsec_max=ts.tv_nsec;
    while(sc<60){
        clock_gettime(CLOCK_REALTIME,&ts);
        sc=ts.tv_sec-start;
        if(nsec_max < ts.tv_nsec){
            nsec_max=ts.tv_nsec;
        }
    }
    return nsec_max;
}
void log_init(void)
{
    struct timespec ts;
    struct tm tmv;
    wchar_t timestr[30];
    clock_gettime(CLOCK_REALTIME,&ts);
    gmtime_r(&ts.tv_sec,&tmv);
    swprintf(timestr,30,L"%02d:%02d:%02d %9d\n",tmv.tm_hour,tmv.tm_min,tmv.tm_sec,ts.tv_nsec);
    wprintf(timestr);
}

void log_info(const wchar_t *fmt,...)
{
    struct timespec ts;
    struct tm tmv;
    clock_gettime(CLOCK_REALTIME,&ts);
    localtime_r(&ts.tv_sec,&tmv);
    wprintf(L"[\033[0;35m%02d:%02d:%02d %9d\033[0m] ",tmv.tm_hour,tmv.tm_min,tmv.tm_sec,ts.tv_nsec);
    va_list ap;
    va_start(ap,fmt);
    vwprintf(fmt,ap);
    va_end(ap);
}