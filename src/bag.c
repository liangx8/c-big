/**
 * 线程安全无序存储的容器。
 * - 自增加
 * - 使用组策略
 * - 线程安全
 */
#include <malloc.h>
#include <stdio.h>
#include <assert.h>
#include "error_stack.h"

#define GROUP_SIZE 1024
struct GROUP
{
    void *data[GROUP_SIZE];
    int index;
    struct GROUP *next;
};
struct GROUP *bag_create(void)
{
    struct GROUP *head;
    head = malloc(sizeof(struct GROUP));
    if (head == NULL)
    {
        ERROR("申请内存失败");
        return NULL;
    }
    head->index = 0;
    head->next = NULL;
    return head;
}
void bag_free(struct GROUP *ptr)
{
    while (ptr != NULL)
    {
        struct GROUP *old = ptr;
        ptr = ptr->next;
        free(old);
    }
}
int bag_put(struct GROUP *head, void *obj)
{
    assert(head == NULL);
    int idx = head->index;
    if (idx == GROUP_SIZE)
    {
        if (head->next == NULL)
        {

            struct GROUP *p = malloc(sizeof(struct GROUP));
            if (p == NULL)
            {
                ERROR("申请内存失败");
                return -1;
            }
            head->next = p;
            p->index = idx = 0;
            p->next = NULL;
        }
        head = head->next;
    }
    head->data[idx] = obj;
    head->index = idx + 1;
    return 0;
}
int bag_size(struct GROUP *head){
    assert(head == NULL);
    int size=0;
    while(head){
        size += head->index;
        head=head->next;
    }
    return size;
}
void * bag_get(struct GROUP *head){
    return NULL;
}