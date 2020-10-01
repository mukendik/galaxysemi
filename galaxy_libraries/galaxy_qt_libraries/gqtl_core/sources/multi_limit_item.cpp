#include<limits>
#include <math.h>

#include "multi_limit_item.h"

namespace GS
{
namespace Core
{

int MultiLimitItem::sNbMaxLimits = 0;

MultiLimitItem::MultiLimitItem()
    : mHBin(1),
      mSBin(-1),
      mSite(-1),
      mIsValid(true),
      ldFailCount(-1),          // Total failures in production. -1 means not found in summary.
      ldOutliers(0),            // Total outliers
      lfLowLimit(-C_INFINITE),	// Test Low Limit
      lfHighLimit(C_INFINITE),	// Test High Limit
      lfLowLimitOutlier(-C_INFINITE),			// Outlier Low Limit
      lfHighLimitOutlier(C_INFINITE),			// Outlier High Limit
      lfCpk(C_NO_CP_CPK),		// Cpk Built after pass2 completed...
      lfCpkLow(C_NO_CP_CPK),	// Cpk Low Built after pass2 completed...
      lfCpkHigh(C_NO_CP_CPK),	// Cpk High Built after pass2 completed...
      lfCp(C_NO_CP_CPK),		// Cp Built after pass2 completed...
      lfHistogramMin(-C_INFINITE),// Min. of values
      lfHistogramMax(C_INFINITE)// Max. of values
{
    mIsParametricItem = false;
    ldSampleFails = 0;          // Total failures listed in PTR
    // No limits defined
    bLimitFlag= CTEST_LIMITFLG_NOLTL | CTEST_LIMITFLG_NOHTL | CTEST_LIMITFLG_NOLSL | CTEST_LIMITFLG_NOHSL;
    // bit0=1 (no low limit forced), bit1=1 (no high limit forced)
    bLimitWhatIfFlag=CTEST_LIMITFLG_NOLTL | CTEST_LIMITFLG_NOHTL;
    *szLowL= 0;				// Test low Test limit.
    *szHighL= 0;			// Test low Test limit.
}

MultiLimitItem::MultiLimitItem(const MultiLimitItem &other)
{
    *this = other;
}

MultiLimitItem::~MultiLimitItem()
{

}

void MultiLimitItem::UpdateNbMaxLimits(int size)
{
    if(size > sNbMaxLimits)
        ++sNbMaxLimits;
}

bool MultiLimitItem::operator==(const MultiLimitItem &other)
{
    return this->IsEqual(&other);
}

bool FloatIsEqual(float value1, float value2)
{
    //float lMax12 = std::max(value1, value2);
    //return fabs(value1 - value2) <= (std::numeric_limits<float>::epsilon()*lMax12);
    return value1 == value2;
}

bool MultiLimitItem::IsEqual(const MultiLimitItem* other) const
{
    if (!other)
        return false;
    return
        (
            (
                (((lfHighLimit != C_INFINITE) || (other->lfHighLimit != C_INFINITE)) && (FloatIsEqual(lfHighLimit, other->lfHighLimit)))
                ||
                ((lfHighLimit == C_INFINITE) && (other->lfHighLimit == C_INFINITE))
            )
            &&
            (
                (((lfLowLimit != -C_INFINITE) || (other->lfLowLimit != -C_INFINITE)) && (FloatIsEqual(lfLowLimit, other->lfLowLimit)))
                ||
                ((lfLowLimit == -C_INFINITE) && (other->lfLowLimit == -C_INFINITE))
             )
            &&
            (
                (mHBin == other->GetHardBin())
            )
       );
}

MultiLimitItem &MultiLimitItem::operator=(const MultiLimitItem &other)
{
    if (this != &other)
    {
        lfHighLimit     = other.lfHighLimit;
        lfLowLimit      = other.lfLowLimit;
        mSBin           = other.mSBin;
        mHBin           = other.mHBin;
        mSite           = other.mSite;
        mIsValid        = other.mIsValid;
        ldSampleFails   = other.ldSampleFails;
        ldFailCount     = other.ldFailCount;
        ldOutliers      = other.ldOutliers;
        lfLowLimit      = other.lfLowLimit;
        lfHighLimit     = other.lfHighLimit;
        lfLowLimitOutlier   = other.lfLowLimitOutlier;
        lfHighLimitOutlier  = other.lfHighLimitOutlier;
        lfCpk               = other.lfCpk;
        lfCpkLow            = other.lfCpkLow;
        lfCpkHigh           = other.lfCpkHigh;
        lfCp                = other.lfCp;
        lfHistogramMin      = other.lfHistogramMin;
        lfHistogramMax      = other.lfHistogramMax;
        bLimitWhatIfFlag    = other.bLimitWhatIfFlag;
        bLimitFlag          = other.bLimitFlag;
        strcpy(szLowL, other.szLowL);
        strcpy(szHighL, other.szHighL);
    }

    return *this;
}

bool MultiLimitItem::LoadFromJSon(const QJsonObject& limitItem, QString& error)
{
    QJsonValue lJsonValue;
    QString lType;
    double lDoubleValue, lIntPart;

    // Clear Item before to load
    Clear();
    // empty error msg
    error = "";

    // Check if "TYPE" key found
    lJsonValue = limitItem.value("TYPE");
    if((lJsonValue == QJsonValue::Undefined) || (!lJsonValue.isString()))
    {
        error = "Invalid JSON Multi-limit item, missing \"TYPE\" key";
        mIsValid = false;
        return false;
    }

    // Check if supported value for "TYPE" key
    // If so, eventually do some additional checks
    lType = lJsonValue.toString();
    if(lType.toLower() != "ml")
    {
        error = "Invalid JSON Multi-limit item, wrong type: " + lType;
        mIsValid = false;
        return false;
    }

    // Check if we have a 'TNUM' field
    lJsonValue = limitItem.value("TNUM");
    if((lJsonValue == QJsonValue::Undefined))
    {
        error = "Invalid JSON Multi-limit item, missing \"TNUM\" key";
        mIsValid = false;
        return false;
    }

    // Make sure 'TNUM' field is a positive integer
    lDoubleValue = lJsonValue.toDouble();
    if(!lJsonValue.isDouble() || (lDoubleValue < 0.0) || (modf(lDoubleValue, &lIntPart) != 0.0))
    {
        error = "Invalid JSON Multi-limit item, \"TNUM\" key is not a positive integer";
        mIsValid = false;
        return false;
    }

    // If 'TNAME' defined, make sure it is a string
    lJsonValue = limitItem.value("TNAME");
    if((lJsonValue != QJsonValue::Undefined) && !lJsonValue.isString())
    {
        error = "Invalid JSON Multi-limit item, \"TNAME\" key is not a string";
        mIsValid = false;
        return false;
    }

    // If 'SITE' defined, make sure it is a positive integer
    lJsonValue = limitItem.value("SITE");
    if(lJsonValue != QJsonValue::Undefined)
    {
        lDoubleValue = lJsonValue.toDouble();
        if(!lJsonValue.isDouble() || (lDoubleValue < 0.0) || (modf(lDoubleValue, &lIntPart) != 0.0))
        {
            error = "Invalid JSON Multi-limit item, \"SITE\" key is not a positive integer";
            mIsValid = false;
        }
        mSite = static_cast<int>(lDoubleValue);
    }

    // If 'LL' defined, make sure it is a valid double
    bool lLLDefined=false;
    lJsonValue = limitItem.value("LL");
    if(lJsonValue != QJsonValue::Undefined)
    {
        if(lJsonValue.isNull())
        {
           lfLowLimit   = -C_INFINITE;
        }
        else
        {
            if(!lJsonValue.isDouble())
            {
                error = "Invalid JSON Multi-limit item, \"LL\" key is not a valid double";
                mIsValid = false;
            }
            lLLDefined = true;
            lfLowLimit = lJsonValue.toDouble();
        }
    }
    // If 'HL' defined, make sure it is a valid double
    bool lHLDefined=false;
    lJsonValue = limitItem.value("HL");
    if(lJsonValue != QJsonValue::Undefined)
    {
        if(lJsonValue.isNull())
        {
            lfHighLimit  = C_INFINITE;
        }
        else
        {
            if(!lJsonValue.isDouble())
            {
                error = "Invalid JSON Multi-limit item, \"HL\" key is not a valid double";
                mIsValid = false;
            }
            lHLDefined = true;
            lfHighLimit = lJsonValue.toDouble();
        }
    }
    // If LL & HL defined, make sure LL<=HL
    if(lLLDefined && lHLDefined && (lfLowLimit > lfHighLimit))
    {
        error = "Invalid JSON Multi-limit item, \"LL\" > \"HL\"";
        mIsValid = false;
    }
    // If 'HBIN' defined, make sure it is a valid positive integer
    bool lHBINDefined = false;
    lJsonValue = limitItem.value("HBIN");
    if(lJsonValue != QJsonValue::Undefined)
    {
        lDoubleValue = lJsonValue.toDouble();
        if(!lJsonValue.isDouble() || (lDoubleValue < 0.0) || (modf(lDoubleValue, &lIntPart) != 0.0))
        {
            error = "Invalid JSON Multi-limit item, \"HBIN\" key is not a positive integer";
            mIsValid = false;
        }
        lHBINDefined = true;
        mHBin = static_cast<int>(lDoubleValue);
    }
    // If 'SBIN' defined, make sure it is a valid positive integer
    bool lSBINDefined=false;
    lJsonValue = limitItem.value("SBIN");
    if(lJsonValue != QJsonValue::Undefined)
    {
        lDoubleValue = lJsonValue.toDouble();
        if(!lJsonValue.isDouble() || (lDoubleValue < 0.0) || (modf(lDoubleValue, &lIntPart) != 0.0))
        {
            error = "Invalid JSON Multi-limit item, \"SBIN\" key is not a positive integer";
            mIsValid = false;
        }
        lSBINDefined = true;
        mSBin = static_cast<int>(lDoubleValue);
    }
    // Make sure we have at least one 'LL', 'HL', 'HBIN' or 'SBIN' key
    if(!lLLDefined && !lHLDefined && !lHBINDefined && !lSBINDefined)
    {
        error = "Invalid JSON Multi-limit item, at least one of the following keys must be defined "\
                "[\"LL\", \"HL\", \"HBIN\", \"SBIN\"]";
        mIsValid = false;
    }

    return mIsValid;
}


bool MultiLimitItem::CreateJsonFromMultiLimit(QJsonObject& limitItem, const int testNum)
{
    limitItem.insert("TYPE", QJsonValue(QString("ML")));
    limitItem.insert("TNUM", QJsonValue(testNum));

    if (IsValidHardBin())
       limitItem.insert("HBIN", QJsonValue(mHBin));

    if (IsValidSoftBin())
       limitItem.insert("SBIN", QJsonValue(mSBin));

    if (IsValidLowLimit())
        limitItem.insert("LL", QJsonValue(lfLowLimit));

    if (IsValidHighLimit())
        limitItem.insert("HL", QJsonValue(lfHighLimit));

    return true;
}

double MultiLimitItem::GetLowLimit() const
{
    return lfLowLimit;
}

double MultiLimitItem::GetHighLimit() const
{
    return lfHighLimit;
}

int MultiLimitItem::GetSoftBin() const
{
    return mSBin;
}

int MultiLimitItem::GetHardBin() const
{
    return mHBin;
}

int MultiLimitItem::GetSite() const
{
    return mSite;
}


void MultiLimitItem::SetLowLimit(double lowLimit)
{
    lfLowLimit = lowLimit;
}

void MultiLimitItem::SetHighLimit(double highLimit)
{
    lfHighLimit = highLimit;
}

void MultiLimitItem::SetSoftBin(int softBin)
{
    mSBin = softBin;
}

void MultiLimitItem::SetHardBin(int hardBin)
{
    mHBin = hardBin;
}

void MultiLimitItem::SetSite(int site)
{
    mSite = site;
}

void MultiLimitItem::Clear()
{
    lfLowLimit          = -C_INFINITE;
    lfHighLimit         = C_INFINITE;
    mHBin               = 1;
    mSBin               = -1;
    mSite               = -1;
    mIsValid            = true;
    ldFailCount         = -1;
    ldOutliers          = 0;
    lfLowLimitOutlier   = -C_INFINITE;
    lfHighLimitOutlier  = C_INFINITE;
    lfCpk               = C_NO_CP_CPK;
    lfCpkLow            = C_NO_CP_CPK;
    lfCpkHigh           = C_NO_CP_CPK;
    lfCp                = C_NO_CP_CPK;
    lfHistogramMin      = -C_INFINITE;
    lfHistogramMax      = C_INFINITE;
    ldSampleFails       = 0;          // Total failures listed in PTR
    bLimitFlag          = CTEST_LIMITFLG_NOLTL | CTEST_LIMITFLG_NOHTL | CTEST_LIMITFLG_NOLSL | CTEST_LIMITFLG_NOHSL;
    bLimitWhatIfFlag    = CTEST_LIMITFLG_NOLTL | CTEST_LIMITFLG_NOHTL;
    *szLowL= 0;				// Test low Test limit.
    *szHighL= 0;			// Test low Test limit.
}

bool MultiLimitItem::IsValidLowLimit() const
{
    return (lfLowLimit != -C_INFINITE);
}

bool MultiLimitItem::IsValidHighLimit() const
{
    return (lfHighLimit != C_INFINITE);
}

bool MultiLimitItem::IsValidHardBin() const
{
    return (mHBin >= 0);
}

bool MultiLimitItem::IsValidSoftBin() const
{
    return (mSBin >= 0);
}

bool MultiLimitItem::IsValidSite() const
{
    return (mSite >= 0);
}

bool MultiLimitItem::IsValid() const
{
    return mIsValid;
}

void MultiLimitItem::SetValid(bool valid)
{
    mIsValid = valid;
}

QString MultiLimitItem::ExtractTestName(const QJsonObject &limitItem)
{
    QJsonValue lJsonValue;

    // Check if "TNAME" key found
    lJsonValue = limitItem.value("TNAME");
    if((lJsonValue == QJsonValue::Undefined) || (!lJsonValue.isString()))
    {
        return QString();
    }

    return lJsonValue.toString();
}

int MultiLimitItem::ExtractTestNumber(const QJsonObject &limitItem)
{
    QJsonValue lJsonValue;
    double lDoubleValue, lIntPart;

    // Check if we have a 'TNUM' field
    lJsonValue = limitItem.value("TNUM");
    if((lJsonValue == QJsonValue::Undefined))
    {
        return -1;
    }

    // Make sure 'TNUM' field is a positive integer
    lDoubleValue = lJsonValue.toDouble();
    if(!lJsonValue.isDouble() || (lDoubleValue < 0.0) || (modf(lDoubleValue, &lIntPart) != 0.0))
    {
        return -1;
    }

    return static_cast<int>(lDoubleValue);
}

int MultiLimitItem::ExtractSiteNumber(const QJsonObject &limitItem)
{
    QJsonValue lJsonValue;
    double lDoubleValue, lIntPart;

    // Check if we have a 'SITE' field
    lJsonValue = limitItem.value("SITE");
    if((lJsonValue == QJsonValue::Undefined))
    {
        return -1;
    }

    // Make sure 'SITE' field is a positive integer
    lDoubleValue = lJsonValue.toDouble();
    if(!lJsonValue.isDouble() || (lDoubleValue < 0.0) || (modf(lDoubleValue, &lIntPart) != 0.0))
    {
        return -1;
    }

    return static_cast<int>(lDoubleValue);
}

bool MultiLimitItem::isParametricItem() const
{
    return mIsParametricItem;
}

void MultiLimitItem::setIsParametricItem(bool isParametricItem)
{
    mIsParametricItem = isParametricItem;
}

}
}
