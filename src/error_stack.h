#ifndef ERROR_STACK_H_J348982394JK
#define ERROR_STACK_H_J348982394JK

void error_stack(const char *file,int line,const char* msg);
void error_stack_by_errno(const char *file,int line);
void print_error_stack(void *out);

#define ERROR(m) error_stack(__FILE__,__LINE__,m)
#define ERROR_BY_ERRNO() error_stack_by_errno(__FILE__,__LINE__)

#endif