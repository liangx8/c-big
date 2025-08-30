#include <sqlite3.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "log.h"
extern const char *dbdir;
char sqlstr[256];
int loadsql(void)
{
    char buf[128];
    const char *home=getenv("HOME");
    int namesize=strlen(home);
    memset(buf,0,128);
    strcpy(buf,home);
    buf[namesize]='/';
    strcat(buf,dbdir);
    buf[namesize+4]='/';
    strcat(buf,"script.sql");
    FILE *ss=fopen(buf,"r");
    if(ss==NULL){
        return -1;
    }
    size_t len=fread(sqlstr,1,256,ss);
    sqlstr[len]='\0';
    fclose(ss);
    return 0;
}
sqlite3 *opendb(void)
{
    char buf[128];
    const char *home=getenv("HOME");
    int namesize=strlen(home);
    memset(buf,0,128);
    strcpy(buf,home);
    buf[namesize]='/';
    strcat(buf,dbdir);
    buf[namesize+4]='/';
    strcat(buf,"sql.db");
    sqlite3 *db;
    if(sqlite3_open("/home/ram/big/sql.db",&db)==SQLITE_OK){
        return db;
    }
    log_err(L"打开数据库%s错误\n",buf);
    return NULL;
}
int exec_cb(void *arg,int col,char **p1,char **p2){
    log_info(L"col:%d,%x,%x\n",col,*p1,*p2);
    return 0;
}
void sqlite_exec(int cmd)
{
    sqlite3 *db=opendb();
    char *errmsg;
    if(db==NULL){
        return;
    }
    if(cmd){
        log_info(L"建立数据表\n");
    }else {
        if(loadsql()){
            log_err(L"打开script.sql文件错误\n");
            sqlite3_close(db);
            return;
        }
        int res=sqlite3_exec(db,sqlstr,exec_cb,(void*)0,&errmsg);
        if(res!=SQLITE_OK){
            log_err(L"error:%s\n",errmsg);
            sqlite3_free(errmsg);
        }
    }
    sqlite3_close(db);
}
void sqlite_prepare(int cmd)
{
    sqlite3 *db=opendb();
    if(db==NULL){
        return;
    }
    sqlite3_stmt *begin,*insert,*commit,*rollback;
    const char *rest;
    begin=NULL;
    insert=NULL;
    commit=NULL;
    rollback=NULL;
    FILE *rf=fopen("/home/ram/big/sample.bin","r");
    char b64[8];
    const wchar_t *msg=NULL;
    if(sqlite3_prepare_v2(db,"BEGIN TRANSACTION;",-1,&begin,&rest)){
        msg=L"BEGIN Statement error";
        goto exit;
    }
    if(sqlite3_prepare_v2(db,"COMMIT TRANSACTION;",-1,&commit,&rest)){
        msg=L"COMMIT Statement error";
        goto exit;
    }
    if(sqlite3_prepare_v2(db,"ROLLBACK TRANSACTION;",-1,&rollback,&rest)){
        msg=L"ROLLBACK Statement error";
        goto exit;
    }
    if(sqlite3_prepare_v2(db,"INSERT INTO t_big (highidx,lowidx,rec) values (?,?,?);",-1,&insert,&rest)){
        msg=L"INSERT Statement error";
        goto exit;
    }
    if(sqlite3_step(begin)==SQLITE_ERROR){
        msg=L"执行step(begin)错误";
        goto exit;
    }
    int xcnt=0;
    uint32_t *ptr1=(uint32_t*)&b64[0];
    uint32_t *ptr2=(uint32_t*)&b64[4];

    while(1){
        size_t cnt=fread(b64,1,8,rf);
        long v1=(unsigned int)*ptr1;
        long v2=(unsigned int)*ptr2;
        if(cnt <8){
            break;
        }
        if(sqlite3_bind_int64(insert,1,v2)){
            msg=L"绑定数据错误118";
            sqlite3_step(rollback);
            goto exit;
        }
        if(sqlite3_bind_int64(insert,2,v1)){
            msg=L"绑定数据错误123";
            sqlite3_step(rollback);
            goto exit;
        }
        if(sqlite3_bind_int(insert,3,xcnt)){
            msg=L"绑定数据错误128";
            sqlite3_step(rollback);
            goto exit;
        }
        if(sqlite3_step(insert)==SQLITE_ERROR){
            msg=L"执行step(insert)错误";
            sqlite3_step(rollback);
            goto exit;
        }
        if(sqlite3_reset(insert)){
            msg=L"执行reset(insert)错误";
            sqlite3_step(rollback);
            goto exit;
        }
        xcnt++;
    }
    sqlite3_step(commit);
    log_info(L"total:%d\n",xcnt);
exit:
    if(msg){
        log_err(L"%ls,database msg:%s\n",msg,sqlite3_errmsg(db));
    }
    sqlite3_finalize(begin);
    sqlite3_finalize(insert);
    sqlite3_finalize(commit);
    sqlite3_finalize(rollback);
    sqlite3_close(db);
}