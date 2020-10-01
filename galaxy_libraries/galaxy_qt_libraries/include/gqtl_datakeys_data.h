#ifndef DATAKEYS_DATA_H
#define DATAKEYS_DATA_H

#include <QString>
#include <QVariant>

#include "gqtl_datakeys_global.h"

namespace GS
{
namespace QtLib
{


/// \brief The DataKeysData class

class GQTL_DATAKEYSSHARED_EXPORT DataKeysData
{
public:

    /// \brief DataKeysData constructor from parameter
    DataKeysData(const QString &keyName,const QString &description,const QString &stdfField,
                 const QString &type, const QString &dataType, const QVariant& lDefaultValue);
    DataKeysData();
    /// \brief DataKeysData copy constructor
    DataKeysData(const DataKeysData &dataKeys);
    /// \brief DataKeysData assigning operator
    DataKeysData &operator=(const DataKeysData &other);
    /// Destructor
    ~DataKeysData();

    //Getter & Setter
    QString GetKeyName() const;
    void SetKeyName(const QString &value);

    QString GetDescription() const;
    void SetDescription(const QString &value);

    QString GetStdfField() const;
    void SetStdfField(const QString &value);

    QString GetType() const;
    void SetType(const QString &value);

    QString GetDataType() const;
    void SetDataType(const QString &value);

    QVariant GetDefaultValue() const;
    void SetDefaultValue(const QVariant& value);

private:

    QString     mKeyName;
    QString     mDescription;
    QString     mStdfField;
    QString     mType;
    QString     mDataType;
    QVariant    mDefaultValue;
};

}
}

#endif // DATAKEYS_DATA_H
