#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include "options.h"
const char *usage_str="%s [option]\n\
    -h        show this page and exit\n\
    -b        resort database\n\
    -G <n>    generate n records of testing data from exist file\n\
    -n <n>    generate n random records\n\
    -f        Find record\n\
    -t        Ordered testing of database\n\
    -T        Unit test\n\
    -p <n,m>  test partition\n\
    -l <n>    Limit of list record\n\
    -o <n>    Offset of list record\n";
void usage(const char *me){
    printf(usage_str,me);
}

int populate_num(struct OPTION *ptr,char *str)
{
    char buf[16];
    int ix=0;
    int comma=-1;
    while(str[ix]){
        if(str[ix]==','){
            buf[ix]=0;
            ptr->offset=atoi(buf);
            comma=ix;
        }else {
            buf[ix]=str[ix];
        }
        ix++;
    }
    if(comma < 0){
        return -1;
    }
    ptr->limit=atoi(str+comma+1);
    if(ptr->offset >= ptr->limit){
        return -1;
    }
    return 0;

}
void parse(int argc, char *const argv[], struct OPTION *data)
{
    int opt;
    data->limit=20;
    data->offset=0;
    data->action=SORT;
    while ((opt = getopt(argc, argv, "n:hG:f:tT:l:o:bp:d")) != -1)
    {
        switch (opt)
        {
        case 'n':
            data->action = GEN_RAND_TEST;
            data->offset = atoi(optarg);
            break;
        case 'b':
            data->action = RESORT;
            break;
        case 'T':
            data->action = UNIT_TEST;
            if(optarg[0]=='\0'){
                data->testname=NULL;
            } else {
                data->testname=optarg;
            }
            break;
        case 't':
            data->action = TEST;
            break;
        case 'l':
            if(data->action == SORT){
                data->action = LIST;
            }
            data->limit = atoi(optarg);
            if (data->limit==0)data->limit=20;
            break;
        case 'o':
            data->action = LIST;
            data->offset = atoi(optarg);
            break;
        case 'f':
            data->offset=atoi(optarg);
            data->action=FIND;
            break;
        case 'G':
            // 生成测试数据
            data->action=GEN_TEST;
            data->offset=atoi(optarg);
            break;
        case 'p':
            if(populate_num(data,optarg)){
                printf("参数错误");
                data->offset=0;
                data->limit=0;
                data->action=-1;
            }else {
                data->action=TEST_PART;
            }
            break;
        case 'd':
            data->action=DUPLICATE;
            break;
        case 'h':
        default:
            data->action=-1;
            break;
        }
    }
    if(argc>optind){
        data->srcname=argv[optind];
    } else{
        data->srcname=NULL;
    }
}