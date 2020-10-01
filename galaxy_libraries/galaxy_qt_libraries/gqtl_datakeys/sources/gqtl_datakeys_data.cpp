#include "gqtl_datakeys_data.h"

namespace GS
{
namespace QtLib
{

DataKeysData::DataKeysData(const QString &keyName, const QString &description, const QString &stdfField,
                           const QString &type, const QString &dataType, const QVariant &lDefaultValue)
    :mKeyName(keyName),mDescription(description),mStdfField(stdfField),mType(type),mDataType(dataType),mDefaultValue(lDefaultValue)
{

}

DataKeysData::DataKeysData()
{

}

DataKeysData::DataKeysData(const DataKeysData &dataKeys)
{
     *this = dataKeys;
}

DataKeysData &DataKeysData::operator=(const DataKeysData &dataKeys)
{
    if (this != &dataKeys)
    {
        mKeyName        = dataKeys.mKeyName;
        mDescription    = dataKeys.mDescription;
        mStdfField      = dataKeys.mStdfField;
        mType           = dataKeys.mType;
        mDataType       = dataKeys.mDataType;
        mDefaultValue   = dataKeys.mDefaultValue;
    }

    return (*this);
}

DataKeysData::~DataKeysData()
{

}
QString DataKeysData::GetKeyName() const
{
    return mKeyName;
}
QString DataKeysData::GetDescription() const
{
    return mDescription;
}
QString DataKeysData::GetStdfField() const
{
    return mStdfField;
}

void DataKeysData::SetStdfField(const QString &value)
{
    mStdfField = value;
}
QString DataKeysData::GetDataType() const
{
    return mDataType;
}

void DataKeysData::SetDataType(const QString &value)
{
    mDataType = value;
}

QVariant DataKeysData::GetDefaultValue() const
{
    return mDefaultValue;
}

void DataKeysData::SetDefaultValue(const QVariant &value)
{
    mDefaultValue = value;
}

QString DataKeysData::GetType() const
{
    return mType;
}

void DataKeysData::SetType(const QString &value)
{
    mType = value;
}

void DataKeysData::SetDescription(const QString &value)
{
    mDescription = value;
}


void DataKeysData::SetKeyName(const QString &value)
{
    mKeyName = value;
}


}
}
