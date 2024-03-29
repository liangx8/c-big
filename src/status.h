#ifndef DATA_H_JDUI34JK3L3
#define DATA_H_JDUI34JK3L3
#pragma pack(push,4)
struct STATUS
{
    const char *src;
    const char *dst;
    const char *preform_src;
    const char *preform_dst;
    const void *payload;
    // 上次用的时间
    unsigned long step1time;
    unsigned long step2time;
    // 准备要处理的分区,快速排序中partition的结果,必须成对出现
    long *scope;
    int scope_cnt;
    int step;
};
#pragma pack(pop)

struct STATUS *status_file_load_or_new(const char *);
void status_free(struct STATUS *);
int status_save(struct STATUS *, const char *);
void status_print(struct STATUS *stu);

#define NO_ACTION     0
#define SHOW_PROGRESS 1
#define SORTING_BREAK 2
#define SHOW_PARTITION 3
#define SORTING_DONE  4
#endif