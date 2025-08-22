#include <unistd.h>
#include <stdlib.h>
#include <wchar.h>
#include "options.h"
#include "log.h"
extern char *optarg;
extern int optind, opterr, optopt;

/**
 * @brief 10进制，16进制字符串转化成长整型,16进制以0x开始
 */
unsigned long any2num(const char *str)
{
    const char *ptr=str;
    unsigned long base=0;
    while(*ptr==' ')ptr++;
    if(*ptr!='0') goto dec;
    ptr ++;
    if((*ptr!='x') && (*ptr != 'X')) goto dec;
    while(1){
        ptr++;
        int ch=*ptr;
        if(ch=='\0' || ch == ' '){
            return base;
        }
        base = base << 4;
        if((ch >= '0') && (ch <= '9')){
            base += ch - '0';
            continue;
        }
        if((ch >= 'a') && (ch <= 'f')){
            base += ch - 'a' +10;
            continue;
        }
        if((ch >= 'A') && (ch <= 'F')){
            base += ch - 'a' +10;
            continue;
        }
        return 0;
    }
dec:
    while(1){
        int ch=*ptr;
        if(ch=='\0' || ch == ' '){
            return base;
        }
        base = base * 10;
        if(ch >= '0' && ch <= '9'){
            base += ch -'0';
        } else {
            return 0;
        }
        ptr++;
    }
}

int parse_options(struct OPTIONS *opts,int argc, char **argv)
{
    int opt;
    while((opt=getopt(argc,argv,"t:m:r:s:S:"))!=-1){
        opts->action=opt;
        switch(opt){
            case 't':
            opts->str=optarg;
            break;
            case 'r':
            case 'm':
            opts->num=atoi(optarg);
            break;
            case 's':
            opts->num=any2num(optarg);
            break;
            case 'S':
            opts->num=atoi(optarg);
            break;
            default:
            log_err(L"参数不对\n");
            return -1;
        }
    }
    return 0;
}