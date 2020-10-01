#include "bin_description.h"

namespace GS
{
namespace Gex
{

BinDescription::BinDescription(QObject *lParent /*=NULL*/)
    : QObject(lParent), mNumber(-1)
{

}

BinDescription::BinDescription(const BinDescription &lOther)
    : QObject(lOther.parent())
{
    *this = lOther;
}

BinDescription::~BinDescription()
{

}

BinDescription &BinDescription::operator=(const BinDescription &lOther)
{
    if (this != &lOther)
    {
        mNumber     = lOther.mNumber;
        mCategory   = lOther.mCategory;
        mName       = lOther.mName;
    }

    return *this;
}

int BinDescription::GetNumber() const
{
    return mNumber;
}

QString BinDescription::GetCategory() const
{
    return mCategory;
}

QString BinDescription::GetName() const
{
    return mName;
}

void BinDescription::SetNumber(int lNumber)
{
    mNumber = lNumber;
}

void BinDescription::SetCategory(const QChar &lCategory)
{
    mCategory = lCategory;
}

void BinDescription::SetName(const QString &lName)
{
    mName = lName;
}

}
}

