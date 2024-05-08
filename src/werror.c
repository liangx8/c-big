#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <wchar.h>
#include <malloc.h>
//#include <stddef.h> // SIZE_MAX

struct runtime_werror{
    const wchar_t *file;
    const wchar_t *msg;
    struct runtime_werror *wrap;
    int line;
} *root;


void werror_init(void){
    root=NULL;
}
wchar_t wbuf[1024];

void werror_no_v(const wchar_t *file,int line,int errcode,const wchar_t *fmt, ...)
{
    va_list ap;
    va_start(ap,fmt);
    size_t fmtnum=vswprintf(&wbuf[0],1024,fmt,ap);
    va_end(ap);
    if(fmtnum>0){
        size_t codenum=0;
        const char *errmsg;
        if(errcode){
            errmsg=strerror(errcode);
            codenum=mbstowcs(NULL,errmsg,0);
            if(codenum <0){
                perror("internal error");
                return ;
            }
        }

        // fmtnum + '(' + codenum + ')' + '\0'
        wchar_t *vmsg=malloc((fmtnum+codenum+1+2) * sizeof(wchar_t));
        va_start(ap,fmt);
        size_t num=vswprintf(vmsg,fmtnum+1,fmt,ap);
        va_end(ap);
        if(num != fmtnum){
            perror("internal error");
            return ;
        }
        if(errcode){
            vmsg[fmtnum]=L'(';
            num = mbstowcs(vmsg+fmtnum+1,errmsg,codenum);
            if(num != codenum){
                perror("internal error");
                return ;
            }
            vmsg[fmtnum+codenum+1]=L')';
            vmsg[fmtnum+codenum+2]=0;
        }
        struct runtime_werror *werr=malloc(sizeof(struct runtime_werror));
        werr->file=file;
        werr->line=line;
        werr->wrap=root;
        werr->msg=vmsg;
        root=werr;
        return ;
    }
    perror("error that can't be handled. werror.c(2999)");
    return ;
}
void werror_print(void)
{
    struct runtime_werror *pe=root;
    int indent=0;
    while (pe!=NULL){
        for (int ix=0;ix<indent;ix++){
            fputwc(L' ',stdout);
        }
        wprintf(L"%ls(%d):%ls\n",pe->file,pe->line,pe->msg);
        pe=pe->wrap;
        indent +=2;
    }
}
void werror_clean(void)
{
    struct runtime_werror *pe=root;
    while (pe!=NULL){
        struct runtime_werror *next=pe->wrap;
        free(pe);
        pe=next;
    }
}

