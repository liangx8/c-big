#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <malloc.h>
#include <wchar.h>
#include <stdlib.h>
#include "strs.h"
#include "../error_stack.h"
#include "../buffer.h"


struct st_nameid{
    FILE *dbh;
    long cur;
    long end;
    uint64_t lead[2];
    struct STRS **titles;
    int title_cnt;
};
int load_titles(struct st_nameid *nid,const uint8_t *buf,int size)
{
    // buf 会被释放，因此不能在这里被重用
    int strscnt= calc_strss((const char *)buf,size);
    if(strscnt<0){
        ERROR_WRAP();
        return -1;
    }
    nid->titles=malloc(sizeof(struct STRS *)*strscnt);
    uint16_t offset=0;
    for(int ix=0;ix<strscnt;ix++){
        uint16_t len;
        struct STRS *strs=strs_load(buf+offset,&len);
        if (strs==NULL){
            return -1;
        }
        nid->titles[ix]=strs;
        offset +=len;
    }
    if(offset != size){
        ERROR("ＥＲＲＯＲ");
        return -1;
    }
    nid->title_cnt=strscnt;
    return 0;
}
// 文件结尾的12个字节: 4字节记录titles的储存大小，8字节记录总共的记录数
#define MAGIC_NUM_WIDTH 12
off_t filesize(const char *path);
struct st_nameid *new_nameid(const char *dbn)
{
    if(filesize(dbn)<8){
        ERROR("错误的输入文件\n");
        return NULL;
    }

    FILE *fsrc=fopen(dbn,"r");
    if (fsrc==NULL){
        ERROR_BY_ERRNO();
        return NULL;
    }
    fseek(fsrc,-MAGIC_NUM_WIDTH,SEEK_END);
    uint8_t magic[MAGIC_NUM_WIDTH];
    int32_t *p32=(int32_t *)&magic[0];
    size_t sz=fread(&magic[0],MAGIC_NUM_WIDTH,1,fsrc);
    int32_t i32=*p32;
    if(i32 <=0){
        ERROR("读标题数据错误\n");
        fclose(fsrc);
        return NULL;
    }
    struct st_nameid *nid;
    nid=malloc(sizeof(struct st_nameid));
    
    uint8_t *buf=malloc(i32);
    fseek(fsrc,-i32,SEEK_END);
    nid->end=ftell(fsrc);
    CP_MSG("file size:%ld,total:%ld\n",nid->end,*((long*)&magic[4]));

    sz=fread(buf,1,i32-MAGIC_NUM_WIDTH,fsrc);
    if ((i32 - sz) != MAGIC_NUM_WIDTH){
        printf("出错误了%ld,%d\n",sz,i32);
        fclose(fsrc);
        free(buf);
        return NULL;
    }
    if(load_titles(nid,buf,i32-MAGIC_NUM_WIDTH)){
        fclose(fsrc);
        free(buf);
        return NULL;
    }
    nid->dbh=fsrc;
    nid->cur=0L;
    free(buf);
    CP_MSG("title count:%d\n",nid->title_cnt);
    rewind(fsrc);
    if(fread((char *)(&nid->lead[0]),8,2,fsrc)<2){
        fclose(fsrc);
        ERROR("数据库文件不完整");
        return NULL;
    }
    return nid;
}
void nameid_close(struct st_nameid *nid){
    for(int ix=0;ix<nid->title_cnt;ix++){
        free(nid->titles[ix]);
    }
    free(nid->titles);
    fclose(nid->dbh);
}
/*
每个记录有１６个字节开头，定义如下
byte 0 - 7
bit[0:19] 20 当前记录的长度（不包含16字节）
bit[20:54] 35手机
bit[55:61] 7其他字段的内容的标题索引号
bit[62:63] 2性别

byte 8 - 15 身份证号

*/

#define MASK_SIZE   0xfffff
#define MASK_MOBILE 0x7ffffffff
#define MASK_TITLES 0x7f
#define MASK_GENDER 0x3


#define GENDER_UNKNOWN 0
#define GENDER_MALE    (((uint64_t)1) << 62)
#define GENDER_FEMALE  (((uint64_t)2) << 62)
#define SHMASK_GENDER  (((uint64_t)3) << 62)


int nameid_verify(struct st_nameid *nid,struct BUFFER *sbuf)
{
    uint64_t u64=nid->lead[0];
    uint32_t size= u64 & MASK_SIZE;
    int title_idx= (u64  >> 55)& MASK_TITLES;
    char *src=sbuf->data;
    if(title_idx >=nid->title_cnt){
        ERRORV("标题:%d/%d,size:%d\n u64:%lx",title_idx,nid->title_cnt,size,u64);
        return -1;
    }
    fseek(nid->dbh,nid->cur+16,SEEK_SET);
    long num=fread(src,1,size,nid->dbh);
    if ((long)size != num){
        ERROR("internal error");
        return -1;
    }
    char *dst=src + size;
    int scnt=0;
    uint64_t part=tr_str(dst,src);
    if(part==0){
        ERROR_WRAP();
        return -1;
    }
    
    char *addr=dst + pa1(part);
    scnt +=pa2(part);
    part = tr_str(addr,src+scnt);
    if(part==0){
        ERROR_WRAP();
        return -1;
    }
    scnt +=pa2(part);
    if (scnt > 800){
        ERRORV("程序有错误%d",scnt);
        return -1;
    }
    char *sid=addr+ pa1(part);
    chinaid(sid,nid->lead[1]);
//    printf("姓名:%s\t 身份证:\033[0;33;46m%s\033[0m 手机:%ld   \n地址:%s\n",dst,sid,((u64>>20) & MASK_MOBILE),addr);
//    CP_MSG("size:%d,scnt:%d\n",size,scnt);
    struct STRS *other=strs_load((const uint8_t *)src+scnt,&num);
    if (other==NULL){
        ERROR_WRAP();
        return -1;
    }
#if 0
    struct STRS *title=nid->titles[title_idx];
    for (int ix=0;ix<title->size;ix++){
        printf("%s:\033[0;36m%s\033[m\n",title->strs[ix],other->strs[ix]);
    }
#endif
    free(other);
    return 0;

}
int nameid_print1(struct st_nameid *nid,struct BUFFER *sbuf)
{
    uint64_t u64=nid->lead[0];
    uint32_t size= u64 & MASK_SIZE;
    int title_idx= (u64  >> 55)& MASK_TITLES;
    char *src=sbuf->data;
    if(title_idx >=nid->title_cnt){
        ERRORV("标题:%d/%d,size:%d\n u64:%lx",title_idx,nid->title_cnt,size,u64);
        return -1;
    }
    fseek(nid->dbh,nid->cur+16,SEEK_SET);
    long num=fread(src,1,size,nid->dbh);
    if ((long)size != num){
        ERROR("internal error");
        return -1;
    }
    char *dst=src + size;
    uint64_t part=tr_str(dst,src);
    if(part==0){
        ERROR_WRAP();
        return -1;
    }
    char *name=dst;
    char *addr=dst + pa1(part);
    src=src+pa2(part);
    part = tr_str(addr,src);
    if(part==0){
        ERROR_WRAP();
        return -1;
    }
    src=src+pa2(part);
    char *sid=addr+ pa1(part);
    chinaid(sid,nid->lead[1]);
    const char *gen="无";
    if((u64 & SHMASK_GENDER)==GENDER_MALE){
        gen="男";
    }
    if((u64 & SHMASK_GENDER)==GENDER_FEMALE){
        gen="女";
    }
    printf("姓名:%s\t 身份证:\033[0;33;46m%s\033[0m 手机:%ld  性别:%s \n地址:%s\n",name,sid,((u64>>20) & MASK_MOBILE),gen,addr);
    
    struct STRS *other=strs_load((const uint8_t *)src,&num);
    if (other==NULL){
        ERROR_WRAP();
        return -1;
    }
#if 1
    struct STRS *title=nid->titles[title_idx];
    for (int ix=0;ix<title->size;ix++){
        printf("%s:\033[0;36m%s\033[m\n",title->strs[ix],other->strs[ix]);
    }
#endif
    free(other);
    return 0;
}
long nameid_next(struct st_nameid *nid){
    uint64_t u64=nid->lead[0];
    uint32_t size= u64 & MASK_SIZE;
    long next=nid->cur + size+16;
    if (next > nid->end){
        ERROR("数据库文件不完整");
        return -1;
    }
    if (next == nid->end){
        return 0;
    }
    fseek(nid->dbh,next,SEEK_SET);
    if(fread((char *)(&nid->lead[0]),8,2,nid->dbh)<2){
        ERROR("数据库文件不完整");
        return -1;
    }
    nid->cur=next;
    return next;
}