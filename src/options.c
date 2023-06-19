#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include "options.h"
const char *usage_str="%s [option]\n\
    -h        show this page and exit\n\
    -b        resort database\n\
    -G <n>    generate n records of testing data\n\
    -f        Find record\n\
    -t        Ordered testing of database\n\
    -T        Unit test\n\
    -l <n>    Limit of list record\n\
    -o <n>    Offset of list record\n";
void usage(const char *me){
    printf(usage_str,me);
}
void parse(int argc, char *const argv[], struct OPTION *data)
{
    int opt;
    data->limit=20;
    data->offset=0;
    data->action=SORT;
    while ((opt = getopt(argc, argv, "hG:ftTl:o:b")) != -1)
    {
        switch (opt)
        {
        case 'b':
            data->action=RESORT;
            break;
        case 'T':
            data->action = UNIT_TEST;
            break;
        case 't':
            data->action = TEST;
            break;
        case 'l':
            data->action = LIST;
            data->limit = atoi(optarg);
            if (data->limit==0)data->limit=20;
            break;
        case 'o':
            data->action = LIST;
            data->offset = atoi(optarg);
            break;
        case 'f':
            data->target=atoi(optarg);
            data->action=FIND;
            break;
        case 'G':
            // 生成测试数据
            data->action=GEN_TEST;
            data->offset=atoi(optarg);
            break;
        case 'h':
        default:
            data->action=-1;
            break;
        }
    }
}