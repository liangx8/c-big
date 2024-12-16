#include <stdio.h>
#include <wchar.h>

/**
 * @brief 身份证号码解码
 */
void id_str(long id,wchar_t str[])
{
    int xsign=id & 1;
    swprintf(str,19,L"%ld",id>>1);
    if(xsign){
        str[17]=L'X';
    }
}