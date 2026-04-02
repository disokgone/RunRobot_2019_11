#include <stdio.h>
extern void xSay(char *fmt, ...);

char safe_ptr(int line_no, void *ptr, char *info)
{
    if ((size_t) ptr < 0x300000) {
        xSay("%d: ptr $%x is bad (%s)", line_no, ptr, info);
        return(0);
    }
    return(1);
}
