#ifndef BREAKINFO_H_JKDJK34J4JK32
#define BREAKINFO_H_JKDJK34J4JK32
#include "breakinfo_struct.h"
struct break_info *break_info_load(const char *);
int break_info_save(struct break_info *,const char *);
void break_info_print(const struct break_info *);
int break_info_free(struct break_info *);
#endif