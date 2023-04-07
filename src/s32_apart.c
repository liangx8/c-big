#include <stdio.h>
#include <malloc.h>
#include "error_stack.h"
#include "status.h"
#include "run_status.h"

#define UNIT_SIZE 12
#define BUF_CNT 100000
/**
 * 把记录分开2部分，第一部分的值小于分界位置的值，第二部分大于等于
 */
int64_t apart32(const char *src, const char *dst, struct run_status *rs)
{
    FILE *fsrc, *fdst;
    fsrc = fopen(src, "r");
    char pivot[12];
    char *buf;
    if (fsrc == NULL)
    {
        ERROR_BY_ERRNO();
        return -1;
    }
    if (fseek(fsrc, 0, SEEK_END))
    {
        ERROR_BY_ERRNO();
        fclose(fsrc);
        return -1;
    }
    long size = ftell(fsrc);
    if (size < 0)
    {
        ERROR_BY_ERRNO();
        fclose(fsrc);
        return -1;
    }
    rs->size = size / UNIT_SIZE;
    rs->pt_left = 0;
    rs->pt_right = rs->size;
    rs->pt_cnt = 0;
    rewind(fsrc);
    clearerr(fsrc);
    buf = malloc(BUF_CNT * UNIT_SIZE);
    if (buf == NULL)
    {
        ERROR_BY_ERRNO();
        fclose(fsrc);
        return -1;
    }
    fread(pivot, UNIT_SIZE, 1, fsrc);
    if (ferror(fsrc))
    {
        ERROR_BY_ERRNO();
        fclose(fsrc);
        return -1;
    }
    fdst = fopen(dst, "w");
    if (fdst == NULL)
    {
        ERROR_BY_ERRNO();
        fclose(fsrc);
        free(buf);
        return -1;
    }
    uint32_t pv = *((uint32_t *)pivot);
    int64_t cnt = 0;
    while (!feof(fsrc))
    {
        int btotal = fread(buf, UNIT_SIZE, BUF_CNT, fsrc);
        uint32_t *p12 = buf;
        if (ferror(fsrc))
        {
            ERROR_BY_ERRNO();
            break;
        }
        int64_t left, right;
        left = rs->pt_left;
        right = rs->pt_right;
        for (int ix = 0; ix < btotal; ix++)
        {
            if (left + 1 == right)
            {
                fseek(fdst, left * UNIT_SIZE, SEEK_SET);
                fwrite(pivot, UNIT_SIZE, 1, fdst);
                free(buf);
                fclose(fsrc);
                fclose(fdst);
                return left;
            }
            p12 += ix * 3;
            if (*p12 < pv)
            {
                save to left
                        left +
                    1
            }
            else
            {
                // save to right
                // right - 1
            }
        }
    }
    free(buf);
    fclose(fsrc);
    fclose(fdst);
    return -1;
}
