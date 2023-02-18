#ifndef DATA_H_JDUI34JK3L3
#define DATA_H_JDUI34JK3L3
#include <stdint.h>
struct STATUS
{
    const char *src;
    const char *dst;
    uint64_t step1time;
    uint32_t pivot_value;
    int64_t pivot_index;
    int64_t total;
};
int config_parse(struct STATUS *,const char *);
#endif