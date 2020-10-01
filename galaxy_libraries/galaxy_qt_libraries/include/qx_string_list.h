#ifndef QX_STRING_LIST_H
#define QX_STRING_LIST_H

#include <QStringList>

namespace Qx
{

class QxStringList : public QStringList
{
public :
    typedef QStringList::const_iterator const_iterator;

public :
    QxStringList();
    explicit QxStringList( const QStringList &aStringList );
    virtual ~QxStringList();

    bool hasCommonContentWith( const QxStringList &aOther );
};

}

#endif // QX_STRING_LIST_H
