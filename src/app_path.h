#ifndef APP_PATH_H_UYE3HJK43KL
#define APP_PATH_H_UYE3HJK43KL
struct APP_PATH{
    const char *dst;
    const char* preform_src;
    char* preform_dst;
    char* preform_break_info;
};

struct APP_PATH *path_info_new(const char *);
void path_info_dst(struct APP_PATH *,const char *);
void path_info_free(struct APP_PATH *);
#endif