/******************************************************************************!
 * \file dlfcn_win32.h
 ******************************************************************************/
#ifndef DLFCN_WIN32_H
#define DLFCN_WIN32_H

#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__)
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
# include <errno.h>
# define dlopen(filename, flag) (void*) LoadLibrary(filename)
# define dlsym(handle, symbol) (void*) GetProcAddress((HMODULE) handle, symbol)
# define dlclose(handle) ! FreeLibrary((HMODULE) handle)
const char* dlerror()
{
    static char buff[256];
    if (GetLastError() == 0) {
        return NULL;
    }
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM |
                  FORMAT_MESSAGE_IGNORE_INSERTS,
                  NULL,
                  GetLastError(),
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  buff,
                  sizeof buff,
                  NULL);
    return buff;
}
#endif

#endif
