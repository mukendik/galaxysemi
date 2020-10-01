#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>


/*
	Comment me
	Add this file in your project to prevent some link issues at least on VisualStudio
*/

extern "C"
{
    int snprintf(char *s1, size_t size, const char * fmt, ...)
    {
        va_list ap;
        va_start( ap, fmt );
        int r=_snprintf(s1, size, fmt, ap);
        va_end( ap );
        return r;
    }
    int vsnprintf(char *s1, size_t size, const char *fmt, va_list ap)
    {
        int r=_vsnprintf(s1, size, fmt, ap);
        return r;
    }
	long strtoi64(const char* x, char** y, int z)
    {
		return strtol(x, y, z);
    }
	void ___chkstk_ms()
	{
	}
}
