#include "WindowsZLevelCompare.h"

#include "QDetachableTabWindow.h"

bool WindowsZLevelCompare::comp
    ( QDetachableTabWindow *a, QDetachableTabWindow *b )
{
    return a->mZLevel < b->mZLevel;
}

bool WindowsZLevelCompare::equiv
    ( QDetachableTabWindow *a, QDetachableTabWindow *b )
{
    return a->mZLevel == b->mZLevel;
}

bool WindowsZLevelCompare::operator ()
    ( QDetachableTabWindow *a, QDetachableTabWindow *b )
{
    return comp( a, b );
}
