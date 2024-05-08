#ifndef WERROR_H_HKSDLK34JKK32
#define WERROR_H_HKSDLK34JKK32
#include <wchar.h>

void werror_no_v(const wchar_t *,int,int,const wchar_t *, ...);
void werror_print(void);
void werror_init(void);

#define WIDE2(x) L##x
#define WIDE1(x) WIDE2(x)
#define WFILE WIDE1(__FILE__)

#define WERROR(fmt,args...) werror_no_v(WFILE,__LINE__,0,fmt,args)
#define WERRNO(no) werror_no_v(WFILE,__LINE__,no,L"")
#define WERR_NO_MSG(no,fmt,args...) werror_no_v(WFILE,__LINE__,no,fmt,args)
#endif

#define CP_MSG(fmt,args...) wprintf(L"%s(%3d): " fmt ,__FILE__,__LINE__,args)
//#define ERRORV(fmt,args...) error_stack_v(__FILE__,__LINE__,fmt,args)