#include <sqlite3.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
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
    sqlite3_stmt *stmt;
    const char *rest;
    sqlite3_prepare(db,"INSERT INTO t_big (idxfield,rec) values (?,?);",-1,&stmt,&rest);\
    sqlite3_bind_int64(stmt,1,10000000);
    sqlite3_bind_int64(stmt,2,0);
    sqlite3_step(stmt);
    sqlite3_reset(stmt);
    sqlite3_bind_int64(stmt,1,20000000);
    sqlite3_bind_int64(stmt,2,1);
    sqlite3_step(stmt);

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}