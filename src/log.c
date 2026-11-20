#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include <wchar.h>

const char *logname;
void log_info(const wchar_t *fmt, ...)
{
    FILE *lh=fopen(logname,"a");
    if(lh==NULL){
        return;
    }

    time_t now = time(NULL);
    struct tm *local_time = localtime(&now);
    if (local_time == NULL) {
        fclose(lh);
        return;
    }
    fwprintf(lh, L"[%04d-%02d-%02d %02d:%02d:%02d] ",
             local_time->tm_year + 1900, local_time->tm_mon + 1,
             local_time->tm_mday, local_time->tm_hour, local_time->tm_min,
             local_time->tm_sec);

    va_list args;
    va_start(args,fmt);
    vfwprintf(lh,fmt,args);
    va_end(args);
    fwprintf(lh,L"\n");
    fclose(lh);
}