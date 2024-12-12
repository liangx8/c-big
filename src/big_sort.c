
typedef int (*compare)(const void *,const void*);

/**
 * @brief sort hurge amount of data in memory
 * @param cfgname 包含有多少单元要排序，单元有多少字节，目标文件名
 * @param cmp compare function
 */
void bigsort(const char *cfgname,compare cmp)
{
}