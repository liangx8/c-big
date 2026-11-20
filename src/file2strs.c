#define _POSIX_C_SOURCE 200809L

/**
 * 读文本文件到内存，每行以字符串的形式保持，并且在内存中保持8字节对齐
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "file2strs.h"

#define ALIGN_UP_8(x) (((x) + 7) & ~((size_t)7))

static struct STRINGS *strings_error(const char *errmsg)
{
    struct STRINGS *result = calloc(1, sizeof(*result));
    if (result) {
        result->errmsg = errmsg;
    }
    return result;
}

/**
 * 依次迭代所有字符串。
 * 回调返回非0时立即停止，并将该返回值返回给调用者；全部迭代完则返回0。
 */
int strings_iterate(const struct STRINGS *strings,
                    strings_callback callback,
                    void *payload)
{
    if (!strings || !callback) {
        return -1;
    }

    for (size_t i = 0; i < strings->str_count; i++) {
        int result = callback(i,strings->strs[i], payload);
        if (result != 0) {
            return result;
        }
    }

    return 0;
}

struct STRINGS *block_to_strings(const char *block,
                                 size_t block_size,
                                 line_filter filter)
{
    if (block_size == 0) {
        struct STRINGS *result = calloc(1, sizeof(*result));
        if (!result) {
            return strings_error("out of memory");
        }
        return result;
    }
    if (!block) {
        return strings_error("block is NULL");
    }

    struct STRINGS *result = calloc(1, sizeof(*result));
    if (!result) {
        return strings_error("out of memory");
    }

    size_t strs_capacity = 32;
    size_t buffer_capacity = 1024;
    result->strs = malloc(strs_capacity * sizeof(*result->strs));
    result->buffer = malloc(buffer_capacity);
    if (!result->strs || !result->buffer) {
        free(result->strs);
        free(result->buffer);
        free(result);
        return strings_error("out of memory");
    }

    size_t line_start = 0;
    size_t line_number = 0;

    for (size_t i = 0; i <= block_size; i++) {
        if (i == block_size && i > 0 && block[i - 1] == '\n') {
            break;
        }
        if (i != block_size && block[i] != '\n') {
            continue;
        }

        size_t line_end = i;
        if (line_end > line_start && block[line_end - 1] == '\r') {
            line_end--;
        }

        size_t line_length = line_end - line_start;
        if (line_length == SIZE_MAX || line_length + 1 > SIZE_MAX - 7) {
            strings_destroy(result);
            return strings_error("size overflow");
        }

        size_t storage_size = ALIGN_UP_8(line_length + 1);
        if (result->buffer_size > SIZE_MAX - storage_size) {
            strings_destroy(result);
            return strings_error("size overflow");
        }
        size_t needed = result->buffer_size + storage_size;

        if (needed > buffer_capacity) {
            size_t new_capacity = buffer_capacity;
            while (new_capacity < needed) {
                if (new_capacity > SIZE_MAX / 2) {
                    new_capacity = needed;
                    break;
                }
                new_capacity *= 2;
            }

            char *old_buffer = (char *)result->buffer;
            char *new_buffer = malloc(new_capacity);
            if (!new_buffer) {
                strings_destroy(result);
                return strings_error("out of memory");
            }
            memcpy(new_buffer, old_buffer, result->buffer_size);
            for (size_t j = 0; j < result->str_count; j++) {
                size_t offset = (const char *)result->strs[j] - old_buffer;
                result->strs[j] = new_buffer + offset;
            }
            free(old_buffer);
            result->buffer = new_buffer;
            buffer_capacity = new_capacity;
        }

        char *line_copy = (char *)result->buffer + result->buffer_size;
        memcpy(line_copy, block + line_start, line_length);
        line_copy[line_length] = '\0';

        line_number++;
        int keep_line = filter ? filter(line_number, line_copy) != 0 : 1;

        if (keep_line) {
            if (result->str_count == strs_capacity) {
                if (strs_capacity > SIZE_MAX / 2 ||
                    strs_capacity * 2 > SIZE_MAX / sizeof(*result->strs)) {
                    strings_destroy(result);
                    return strings_error("too many lines");
                }
                size_t new_capacity = strs_capacity * 2;
                void *new_strs = realloc(result->strs,
                                         new_capacity * sizeof(*result->strs));
                if (!new_strs) {
                    strings_destroy(result);
                    return strings_error("out of memory");
                }
                result->strs = new_strs;
                strs_capacity = new_capacity;
            }

            result->strs[result->str_count++] = line_copy;
            result->buffer_size = needed;
        }

        if (i == block_size) {
            break;
        }
        line_start = i + 1;
    }

    return result;
}

void strings_destroy(struct STRINGS *strings)
{
    if (!strings) {
        return;
    }
    free(strings->strs);
    free(strings->buffer);
    free(strings);
}

struct STRINGS *file_to_strings(const char *filename,
                                line_filter filter)
{
    if (!filename) {
        return strings_error("filename is NULL");
    }

    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        return strings_error("cannot open file");
    }

    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        return strings_error("cannot seek file");
    }

    long raw_size = ftell(fp);
    if (raw_size < 0) {
        fclose(fp);
        return strings_error("cannot determine file size");
    }
    if (fseek(fp, 0, SEEK_SET) != 0) {
        fclose(fp);
        return strings_error("cannot seek file");
    }

    if (raw_size == 0) {
        fclose(fp);
        return block_to_strings(NULL, 0, filter);
    }

    char *raw_buf = malloc((size_t)raw_size);
    if (!raw_buf) {
        fclose(fp);
        return strings_error("out of memory");
    }

    if (fread(raw_buf, 1, (size_t)raw_size, fp) != (size_t)raw_size) {
        free(raw_buf);
        fclose(fp);
        return strings_error("cannot read file");
    }
    fclose(fp);

    struct STRINGS *result = block_to_strings(raw_buf,
                                              (size_t)raw_size,
                                              filter);
    free(raw_buf);
    return result;
}

