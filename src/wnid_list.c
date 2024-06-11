#include <stdio.h>
#include "wnid_meta.h"
#include "entity.h"
#include "app_path.h"

struct WNAMEID* new_wnameid(const char *);
int wnid_close(struct WNAMEID *);
int wnid_list(struct APP_PATH *ap,const struct ENTITY *ent,long offset,int limit)
{
    FILE *fidx=fopen(ap->preform_dst,"r");
    const int US=ent->unitsize;
    char idx[US];
    struct WNAMEID *wnid=new_wnameid(ap->preform_src);
    if (wnid == NULL){
        return -1;
    }
    fseek(fidx,(offset+1) * ent->unitsize,SEEK_SET);
    for(int ix=0;ix<limit;ix++){
        fread(&idx[0],ent->unitsize,1,fidx);
        ent->print(offset+ix,idx,wnid);
    }

    fclose(fidx);
    wnid_close(wnid);
    return 0;
}

//