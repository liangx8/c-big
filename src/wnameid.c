#include <stdio.h>
#include <wchar.h>
#include <malloc.h>
#include "werror.h"
#include "options.h"
#include "app_path.h"
#include "breakinfo.h"
#ifndef NULL
#define NULL (void *)0
#endif

void * new_wnameid(const char *);
int wnid_close(void *);
const char *nid_file   ="/home/com/big-data/chinaid/nameid.bin";
extern const struct ENTITY chnid_index_entity;

int sort_big(const void *,void *,const struct APP_PATH *);
int sort_part(const struct ENTITY *,const struct APP_PATH *);
int wnid_list(struct APP_PATH *ap,const struct ENTITY *ent,long offset,int limit);
int app_wnameid(const struct OPTION *opt)
{
    const char *nsrc;
    if (opt->srcname == NULL){
        wprintf(L"使用缺省文件名%s\n",nid_file);
        nsrc=nid_file;
    } else {
        nsrc=opt->srcname;
    }
    werror_init();
    struct APP_PATH *ap=path_info_new(nsrc);
    path_info_dst(ap,"id.idx");
    wprintf(L"%lx\n      源:%s\n    索引:%s\n配置文件:%s\n",ap,ap->preform_src,ap->preform_dst,ap->preform_break_info);

    wprintf(L"读文件简信:\033[0;36m%s\033[0m\n",ap->preform_src);
    wprintf(L"      源:%s\n    索引:%s\n配置文件:%s\n",ap->preform_src,ap->preform_dst,ap->preform_break_info);
    wprintf(L"排序数据库\033[0;36m%s\033[0m\n",ap->preform_src);
    
    
    switch(opt->action){
        case TEST_PART:
            break;
        case LIST:
            if(wnid_list(ap,&chnid_index_entity,opt->offset,opt->limit)){
                goto return_in_error;
            }
            break;
        case RESORT:
            {
            void *res=new_wnameid(ap->preform_src);
            if (res == NULL){
                goto return_in_error;
            }
            if (sort_big(&chnid_index_entity,res,ap)){
                wnid_close(res);
                goto return_in_error;
            }
            wnid_close(res);
            }
            break;
        default:
            if(sort_part(&chnid_index_entity,ap)){
                goto return_in_error;
            }
    }
    path_info_free(ap);
    return 0;
    return_in_error:
    path_info_free(ap);
    return -1;
}

