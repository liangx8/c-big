#include <stdio.h>
#include <wchar.h>
#include <sqlite3.h>
#include "log.h"

int callback(void *arg,int row,char **p1,char **p2){
    wprintf(L"row:%d\n",row);
    return 0;
}
int test_sqlite3(void)
{
    sqlite3 *db;
    char *errmsg;
    if(sqlite3_open("/home/ram/big/sql.db",&db)==SQLITE_OK){
//    if(sqlite3_open_v2("/home/ram/big/sql.db",&db,SQLITE_OPEN_READONLY,NULL)==SQLITE_OK){
        int res=sqlite3_exec(db,"CREATE TABLE IF NOT EXISTS t_big (keyid INTEGER PRIMARY KEY,idxfield INTEGER,rec INTEGER);",callback,(void*)0,&errmsg);
        if(res!=SQLITE_OK){
            log_err(L"error:%s\n",errmsg);
            sqlite3_free(errmsg);
        }
        sqlite3_close(db);
    } else {
        return -1;
    }
    return 0;
}