#define _POSIX_C_SOURCE 200809L


#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>
#include <wchar.h>
#include "strlist.h"

static char *join_path(const char *dirname, const char *name)
{
    size_t dirname_length = strlen(dirname);
    size_t name_length = strlen(name);
    int needs_separator = dirname_length > 0 && dirname[dirname_length - 1] != '/';

    if (dirname_length > SIZE_MAX - name_length - (size_t)needs_separator - 1) {
        return NULL;
    }

    size_t length = dirname_length + name_length + (size_t)needs_separator + 1;
    char *path = malloc(length);
    if (!path) {
        return NULL;
    }

    memcpy(path, dirname, dirname_length);
    size_t offset = dirname_length;
    if (needs_separator) {
        path[offset++] = '/';
    }
    memcpy(path + offset, name, name_length + 1);
    return path;
}

int walk_directory(const char *dirname, strlist *list)
{
    if (!dirname || !list) {
        return -1;
    }

    DIR *directory = opendir(dirname);
    if (!directory) {
        return -1;
    }

    int result = 0;
    for (;;) {
        errno = 0;
        struct dirent *entry = readdir(directory);
        if (!entry) {
            if (errno != 0) {
                result = -1;
            }
            break;
        }

        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char *path = join_path(dirname, entry->d_name);
        if (!path) {
            result = -1;
            break;
        }

        struct stat file_status;
        if (stat(path, &file_status) == 0 && S_ISREG(file_status.st_mode)) {
            if (strlist_add(list, entry->d_name) != 0) {
                free(path);
                result = -1;
                break;
            }
        }
        free(path);
    }

    closedir(directory);
    return result;
}

int rename_action(int argc, char *argv[])
{
    if (argc==1) {
        return -1;
    }

    const char *dirname=argv[1];

    strlist *list = strlist_create();
    if (!list) {
        return -1;
    }

    int result = walk_directory(dirname, list);
    size_t size=strlist_size(list);
    for(size_t ix=0;ix<size;ix++){
        wprintf(L"%s\n",strlist_get(list,ix));
    }
    wprintf(L"总共有%d\n",size);
    strlist_destroy(list);
    return result;
}