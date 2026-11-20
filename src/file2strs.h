#ifndef FILE2STRS_H
#define FILE2STRS_H

#include <stddef.h>

struct STRINGS {
    const char **strs;
    size_t str_count;
    void *buffer;
    size_t buffer_size;
    const char *errmsg; /**< 非空时表示转换失败原因；成功时为NULL */
};

typedef struct STRINGS STRINGS;

typedef int (*strings_callback)(size_t seq,
                                const char *line,
                                void *payload);

typedef int (*line_filter)(size_t line_number,
                           const char *line);
/* 0: skip this line and continue; non-zero: add this line and continue. */

struct STRINGS *block_to_strings(const char *block,
                                 size_t block_size,
                                 line_filter filter);
struct STRINGS *file_to_strings(const char *filename,
                                line_filter filter);
void strings_destroy(struct STRINGS *strings);
int strings_iterate(const struct STRINGS *strings,
                    strings_callback callback,
                    void *payload);

#endif
