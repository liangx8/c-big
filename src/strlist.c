#include "strlist.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define STRLIST_ALIGNMENT 8
#define STRLIST_INITIAL_OFFSETS 16
#define STRLIST_INITIAL_BUFFER 1024

#define ALIGN_UP_8(value) \
    (((value) + (STRLIST_ALIGNMENT - 1)) & ~((size_t)(STRLIST_ALIGNMENT - 1)))

struct strlist {
    size_t *offsets;
    size_t offsets_capacity;
    size_t str_count;

    char *buffer;
    size_t buffer_capacity;
    size_t buffer_size;
};

static void ensure_random_seed(void)
{
    static int seeded;
    if (!seeded) {
        srand((unsigned)time(NULL));
        seeded = 1;
    }
}

strlist *strlist_create(void)
{
    strlist *list = calloc(1, sizeof(*list));
    if (!list) {
        return NULL;
    }

    list->offsets = malloc(STRLIST_INITIAL_OFFSETS * sizeof(*list->offsets));
    list->buffer = malloc(STRLIST_INITIAL_BUFFER);
    if (!list->offsets || !list->buffer) {
        free(list->offsets);
        free(list->buffer);
        free(list);
        return NULL;
    }

    list->offsets_capacity = STRLIST_INITIAL_OFFSETS;
    list->buffer_capacity = STRLIST_INITIAL_BUFFER;
    ensure_random_seed();
    return list;
}

static int ensure_offsets_capacity(strlist *list)
{
    if (list->str_count < list->offsets_capacity) {
        return 0;
    }

    if (list->offsets_capacity > SIZE_MAX / 2) {
        return -1;
    }

    size_t capacity = list->offsets_capacity * 2;
    if (capacity > SIZE_MAX / sizeof(*list->offsets)) {
        return -1;
    }

    size_t *new_offsets = realloc(list->offsets,
                                  capacity * sizeof(*list->offsets));
    if (!new_offsets) {
        return -1;
    }

    list->offsets = new_offsets;
    list->offsets_capacity = capacity;
    return 0;
}

static int ensure_buffer_capacity(strlist *list, size_t needed)
{
    if (needed <= list->buffer_capacity) {
        return 0;
    }

    size_t capacity = list->buffer_capacity;
    while (capacity < needed) {
        if (capacity > SIZE_MAX / 2) {
            capacity = needed;
            break;
        }
        capacity *= 2;
    }

    char *new_buffer = realloc(list->buffer, capacity);
    if (!new_buffer) {
        return -1;
    }

    list->buffer = new_buffer;
    list->buffer_capacity = capacity;
    return 0;
}

int strlist_add(strlist *list, const char *value)
{
    if (!list || !value) {
        return -1;
    }

    size_t length = strlen(value);
    if (length == SIZE_MAX || length + 1 > SIZE_MAX - (STRLIST_ALIGNMENT - 1)) {
        return -1;
    }

    size_t storage_size = ALIGN_UP_8(length + 1);
    if (list->buffer_size > SIZE_MAX - storage_size) {
        return -1;
    }

    size_t needed = list->buffer_size + storage_size;
    if (ensure_buffer_capacity(list, needed) != 0) {
        return -1;
    }
    if (ensure_offsets_capacity(list) != 0) {
        return -1;
    }

    size_t offset = list->buffer_size;
    memcpy(list->buffer + offset, value, length + 1);
    list->offsets[list->str_count] = offset;
    list->str_count++;
    list->buffer_size = needed;
    return 0;
}

size_t strlist_size(const strlist *list)
{
    return list ? list->str_count : 0;
}

const char *strlist_get(const strlist *list, size_t index)
{
    if (!list || index >= list->str_count) {
        return NULL;
    }
    return list->buffer + list->offsets[index];
}

int strlist_shuffle(strlist *list)
{
    if (!list) {
        return -1;
    }

    for (size_t i = list->str_count; i > 1; i--) {
        size_t j = (size_t)rand() % i;
        size_t temporary = list->offsets[i - 1];
        list->offsets[i - 1] = list->offsets[j];
        list->offsets[j] = temporary;
    }
    return 0;
}

int strlist_order(strlist *list)
{
    if (!list) {
        return -1;
    }

    for (size_t i = 1; i < list->str_count; i++) {
        size_t key = list->offsets[i];
        size_t j = i;

        while (j > 0) {
            const char *left = list->buffer + list->offsets[j - 1];
            const char *right = list->buffer + key;
            if (strcmp(left, right) <= 0) {
                break;
            }
            list->offsets[j] = list->offsets[j - 1];
            j--;
        }

        list->offsets[j] = key;
    }
    return 0;
}

int strlist_sort(strlist *list)
{
    return strlist_order(list);
}

void strlist_destroy(strlist *list)
{
    if (!list) {
        return;
    }
    free(list->offsets);
    free(list->buffer);
    free(list);
}

