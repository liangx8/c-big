#include <wchar.h>
#include <dirent.h>
#include "strlist.h"
#include "log.h"
struct _source{
    DIR *top;
    strlist *sl;
};
void auto_cleanup(struct _source *source) {
    if (source->top != NULL) {
        closedir(source->top);
    }
    if(source->sl != NULL) {
        strlist_destory(source->sl);
    }
}
int rename_action(int argc, char *argv[]) {
    if(argc < 3) {
        wprintf(L"Usage: rename <top_path> <prefix>\n");
        return -1;
    }
    logname="/tmp/test.log";
    const char *toppath = argv[1];
    //const char *prefix = argv[2];
    __attribute__((cleanup(auto_cleanup))) struct _source source;
    source.sl = NULL;
    source.top = opendir(toppath);

    if(source.top == NULL) {
        wprintf(L"Failed to open directory: %s\n", toppath);
        return -1;
    }
    source.sl=strlist_create();
    if(source.sl == NULL) {
        wprintf(L"Failed to allocate memory\n");
        return -1;
    }
    struct dirent *entry;
    while((entry = readdir(source.top)) != NULL) {
        if(entry->d_type == DT_REG) { // Only rename regular files
            if(strlist_add(source.sl,entry->d_name)){
                wprintf(L"strlist_add() error\n");
                return -1;
            }
        }
    }
    int size=strlist_size(source.sl);
    for(int ix=0;ix<size;ix++){
        log_info(L"%s/%s\n",toppath,strlist_get(source.sl,ix));
    }
    log_info(L"total files %d\n",size);
    return 0;
}