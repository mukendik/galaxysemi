#ifndef BIN_DESCRIPTION_H
#define BIN_DESCRIPTION_H

#include <QObject>
#include <QString>

namespace GS
{
namespace Gex
{

class BinDescription : public QObject
{
    Q_OBJECT

public:

    BinDescription(QObject * lParent = NULL);
    BinDescription(const BinDescription& lOther);
    ~BinDescription();

    BinDescription& operator=(const BinDescription& lOther);

    Q_INVOKABLE int             GetNumber() const;
    // JS API cannot handle a QChar, we have to return a QString
    Q_INVOKABLE QString         GetCategory() const;
    Q_INVOKABLE QString         GetName() const;

    Q_INVOKABLE void            SetNumber(int lNumber);
    Q_INVOKABLE void            SetCategory(const QChar& lCategory);
    Q_INVOKABLE void            SetName(const QString& lName);

private:

    int             mNumber;
    QChar           mCategory;
    QString         mName;
};

}
}
#endif // BIN_DESCRIPTION_H
