// -------------------------------------------------------------------------- //
// lssqldrivers.cpp
// -------------------------------------------------------------------------- //
#include <QSqlDatabase>
#include <QStringList>
#include <iostream>
#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__)
# include "dlfcn_win32.h"
# define LIBEXT "dll"
#else
# include <dlfcn.h>
# if defined(__APPLE__) && defined(__MACH__)
#  define LIBEXT "dylib"
# else
#  define LIBEXT "so"
# endif
#endif

// -------------------------------------------------------------------------- //
// printStringList
// -------------------------------------------------------------------------- //
void printStringList(const QStringList& list)
{
    QStringList::const_iterator iter;

    for (iter  = list.begin();
         iter != list.end(); ++iter)
    {
        std::cout << qPrintable(*iter)
                  << std::endl;
    }
}

// -------------------------------------------------------------------------- //
// main
// -------------------------------------------------------------------------- //
int main(int , char** , char* arge[])
{
    QStringList env;
    for (unsigned int i = 0; arge[i] != NULL; ++i)
    {
        env << arge[i];
    }
    env.sort();
    printStringList(env);
    std::cout << std::endl;

#   if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__)
    void* mysql =
        dlopen("c:/cygwin/home/gexprod/tmp/sqldrivers/qsqlmysql."LIBEXT,
               RTLD_LAZY);
#   else
    void* mysql =
        dlopen("sqldrivers/libqsqlmysql."LIBEXT,
               RTLD_LAZY);
#   endif
    if (mysql == NULL)
    {
        std::cout << dlerror()
                  << std::endl;
    }

#   if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__)
    void* oci =
        dlopen("c:/cygwin/home/gexprod/tmp/sqldrivers/qsqloci."LIBEXT,
               RTLD_LAZY);
#   else
    void* oci =
        dlopen("sqldrivers/libqsqloci."LIBEXT,
               RTLD_LAZY);
#   endif
    if (oci == NULL)
    {
        std::cout << dlerror()
                  << std::endl;
    }

    QStringList drivers = QSqlDatabase::drivers();
    printStringList(drivers);

    if (mysql != NULL && dlclose(mysql) != 0)
    {
        std::cout << dlerror()
                  << std::endl;
    }
    if (oci != NULL && dlclose(oci) != 0)
    {
        std::cout << dlerror()
                  << std::endl;
    }

    return EXIT_SUCCESS;
}
