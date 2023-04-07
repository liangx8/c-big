#ifndef DATA_H_JDUI34JK3L3
#define DATA_H_JDUI34JK3L3
#include <stdint.h>
struct STATUS
{
    const char *src;
    const char *dst;
    unsigned long step1time;
    long pivot_index;
    long total;
    unsigned long pivot_value;
    
};

struct STATUS *status_load_file_or_new(const char *);
void status_free(struct STATUS *);

void status_print(struct STATUS *stu);
#endif