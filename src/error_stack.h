#ifndef ERROR_STACK_H_J348982394JK
#define ERROR_STACK_H_J348982394JK

void error_stack(const char *file,int line,const char* msg);
void error_stack_by_errno(const char *file,int line);
void print_error_stack(void *out);
void print_current_error(const char *);
void error_stack_v(const char *file,int line,const char* fmt, ...);
int has_error(void);

#define ERROR_WRAP() error_stack(__FILE__,__LINE__,"wrap only")
#define ERROR(m) error_stack(__FILE__,__LINE__,m)
#define ERRORV(fmt,args...) error_stack_v(__FILE__,__LINE__,fmt,args)
#define ERROR_BY_ERRNO() error_stack_by_errno(__FILE__,__LINE__)



#define CP_MSG(fmt,args...) wprintf(L"%s(%3d): " fmt ,__FILE__,__LINE__,args)
#define CHECKPOINT() CP_MSG("%ls (%d)\n",L"check point", __COUNTER__)
#endif