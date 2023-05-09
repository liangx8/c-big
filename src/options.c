#include <getopt.h>
#include <stdlib.h>
#include "options.h"

void parse(int argc, char *const argv[], struct OPTION *data)
{
    int opt;
    data->limit=20;
    data->offset=0;
    data->action=SORT;
    while ((opt = getopt(argc, argv, "G:ftTl:o:")) != -1)
    {

        switch (opt)
        {
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
        default:
            data->action=-1;
            break;
        }
    }
}