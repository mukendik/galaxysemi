#include "qx_string_list.h"

namespace Qx
{
QxStringList::QxStringList() :
    QStringList() {}

QxStringList::QxStringList( const QStringList &aStringList ) :
    QStringList( aStringList ) {}

QxStringList::~QxStringList() {}

bool QxStringList::hasCommonContentWith( const QxStringList &aOther )
{
    for( const_iterator lBegin = begin(), lEnd = end(); lBegin != lEnd; ++lBegin )
    {
        if ( ! aOther.contains( *lBegin ) ) return false;
    }

    return true;
}

}