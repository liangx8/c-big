#include <stdio.h>
#include <stdint.h>
#include <malloc.h>
#include <stdlib.h>
#include "strs.h"

#define PR(fmt,args...) printf("%s(%3d):" fmt ,__FILE__,__LINE__,args)


uint64_t tr_str(char *,const char *);

int test_tr_str(FILE *fp)
{
    fseek(fp,16,SEEK_SET);
    char *b256=malloc(512);
    char *str=b256+256;
    fread(b256,256,1,fp);
    uint64_t pair=tr_str(str,b256);
    // printf("%x\n",*((uint16_t*)(b256+1)));
    if(pair==0){
        printf("something wrong\n");
        return -1;
    }
    unsigned p1=pa1(pair);
    unsigned p2=pa2(pair);
    printf("目标占用内存空间:汉字*3(utf8) + 1(结尾0),\n源读取内存空间: 1(字长)+2 * 汉字 \n[%s],%d,%d\n",str,p1,p2);
    pair=tr_str(str+p1,b256+p2);
    printf("[%s]",str+p1);
    p1=pa1(pair);
    p2=pa2(pair);
    printf("%d,%d\n",p1,p2);
    free(b256);
    return 0;
}
int test_strs_load(FILE *fp)
{
    uint8_t b4[4];
    uint8_t *ptr,*ptr1;
    fseek(fp,-4,SEEK_END);
    fread(&b4[0],4,1,fp);
    int32_t i32= *((int32_t *)&b4[0]);
    printf("title 占用字节:%d\n",i32);
    fseek(fp,-i32,SEEK_END);
    ptr1=malloc(i32);
    ptr=ptr1;
    if (fread(ptr,i32,1,fp)<1){
        printf("有问题\n");
        return -1;
    }
    uint16_t cons=0;
    uint32_t sum=0;
    while(1){
        ptr += cons;
        struct STRS *ss= strs_load(ptr,&cons);
        if (ss==NULL){
            return -1;
        }
        sum +=cons;
        printf("line in bytes:%d\n",cons);
        if (sum+4==i32){
            break;
        }
        if (sum+4 > i32){
            printf("error\n");
            return -1;
        }
        for(int ix=0;ix<ss->size;ix++){
            printf("%2d[%s]\n",ix,ss->strs[ix]);
        }
        printf("total strings:%d,%lx\n",ss->size,(long)ss->strs);
        free(ss);
    }

    free(ptr1);
    fclose(fp);
    return 0;
}
int test_strs(void *pl)
{
    FILE *fh=fopen("/home/tec/big/nameid.bin","r");
    if (fh==NULL){
        return -1;
    }
    if (test_tr_str(fh)){
        fclose(fh);
        return -1;
    }
    if(test_strs_load(fh)){
        fclose(fh);
        return -1;
    }
    return 0;
}