#include "JSClasses.h"

JSBinDescription::JSBinDescription(QObject *parent)
    : QObject(parent), mNumber(1), mName("Test"), mCategory('P')
{
}

JSBinDescription::JSBinDescription(const JSBinDescription &other)
    : QObject(other.parent())
{
    *this = other;
}

JSBinDescription::~JSBinDescription()
{
}

int JSBinDescription::GetNumber() const
{
    return mNumber;
}

QString JSBinDescription::GetName() const
{
    return mName;
}

QString JSBinDescription::GetCategory() const
{
    return mCategory;
}

void JSBinDescription::SetNumber(int lNumber)
{
    mNumber = lNumber;
}

void JSBinDescription::SetName(const QString &lName)
{
    mName = lName;
}

void JSBinDescription::SetCategory(const QString &lCategory)
{
    mCategory = lCategory;
}

JSBinDescription &JSBinDescription::operator =(const JSBinDescription &other)
{
    if (this != &other)
    {
        mNumber     = other.mNumber;
        mName       = other.mName;
        mCategory   = other.mCategory;
    }

    return *this;
}

JSWaferMap::JSWaferMap(QObject *parent)
    : QObject(parent), mLowX(-32768), mLowY(-32768), mHighX(-32768), mHighY(-32768), mArray(NULL)
{
}

JSWaferMap::JSWaferMap(const JSWaferMap &other)
    : QObject(other.parent())
{
    mLowX   = other.mLowX;
    mLowY   = other.mLowY;
    mHighX  = other.mHighX;
    mHighY  = other.mHighY;
    mArray  = new int[GetSizeX()*GetSizeY()];

    if (other.mArray)
        memcpy(mArray, other.mArray, GetSizeX()*GetSizeY()*sizeof(int));
}

JSWaferMap::~JSWaferMap()
{
    if (mArray)
    {
        delete [] mArray;
        mArray = NULL;
    }
}

bool JSWaferMap::Create(int lLowX, int lLowY, int lHighX, int lHighY)
{
    if (mArray == NULL)
    {
        mLowX   = lLowX;
        mLowY   = lLowY;
        mHighX  = lHighX;
        mHighY  = lHighY;

        int lSize = GetSizeX()*GetSizeY();

        mArray = new int[lSize];

        for (int lIdx = 0; lIdx < lSize; ++lIdx)
            mArray[lIdx] = -1;

        return true;
    }
    else
        return false;
}

int JSWaferMap::GetLowX() const
{
    return mLowX;
}

int JSWaferMap::GetLowY() const
{
    return mLowY;
}

int JSWaferMap::GetHighX() const
{
    return mHighX;
}

int JSWaferMap::GetHighY() const
{
    return mHighY;
}

int JSWaferMap::GetSizeX() const
{
    return (mHighX - mLowX +1);
}

int JSWaferMap::GetSizeY() const
{
    return (mHighY - mLowY +1);
}

int JSWaferMap::GetBinAt(int lX, int lY) const
{
    int nIndex;

    if (indexFromCoord(nIndex, lX, lY))
    {
        return mArray[nIndex];
    }
    else
        return -1;
}

void JSWaferMap::SetBinAt(int lBin, int lX, int lY)
{
    int nIndex;

    if (indexFromCoord(nIndex, lX, lY))
    {
        mArray[nIndex] = lBin;
    }
}

bool JSWaferMap::indexFromCoord(int& nIndex, int nDieX, int nDieY) const
{
    if (isValidCoord(nDieX, nDieY))
    {
        int nXOffset = nDieX - mLowX;
        int nYOffset = nDieY - mLowY;

        nIndex = nXOffset + (GetSizeX() * nYOffset);
        return true;
    }

    return false;
}

bool JSWaferMap::isValidCoord(int nDieX, int nDieY) const
{
    return isValidXCoord(nDieX) && isValidYCoord(nDieY);
}

///////////////////////////////////////////////////////////
bool JSWaferMap::isValidXCoord(int nDieX) const
{
    return ((nDieX >= mLowX) && (nDieX <= mHighX));
}

///////////////////////////////////////////////////////////
bool JSWaferMap::isValidYCoord(int nDieY) const
{
    return ((nDieY >= mLowY) && (nDieY <= mHighY));
}

//JSWaferMap::JSWaferMap(const JSWaferMap &other)
//{
//}

//JSBinPrecedence::JSBinPrecedence(QObject *parent)
//    : QObject(parent)
//{

//}

//JSBinPrecedence::~JSBinPrecedence()
//{
//}

//JSBinDescription JSBinPrecedence::ComputePrecedence(JSBinDescription lFromMap, JSBinDescription lFromSTDF)
//{
//}



