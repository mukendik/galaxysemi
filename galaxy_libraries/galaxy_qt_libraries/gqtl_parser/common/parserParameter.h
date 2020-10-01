#ifndef PARSERPARAMETER_H
#define PARSERPARAMETER_H

#include "multi_limit_item.h"
#include <QList>

#include <QString>
#include <QMap>

namespace GS
{
namespace Parser
{

typedef QMap<int, double> fValueOfSiteMap;

class ParserParameter
{
public:
    ParserParameter();

    ///\brief Getters
    long    GetTestNumber() const;
    QString	GetTestName() const;
    QString	GetTestUnits() const;
    int     GetResultScale() const;
    double	GetLowLimit() const;
    double	GetHighLimit() const;
    bool	GetValidLowLimit() const;
    bool	GetValidHighLimit() const;
    double	GetLowSpecLimit() const;
    double	GetHighSpecLimit() const;
    bool	GetValidLowSpecLimit() const;
    bool	GetValidHighSpecLimit() const;
    double	GetTestValue() const;
    double  GetTestValueForSite(const int aSite);
    bool    HasTestValueForSite(const int aSite);
    bool	GetStaticHeaderWritten() const;
    bool	GetStaticMultiLimitsWritten() const;
    unsigned long GetExecCount() const;
    unsigned long GetExecFail() const;
    float   GetExecTime() const;
    QString GetTestTxt() const;
    int     GetSpecificFlags() const;
    bool    IsParametricTest() const;

    ///\brief Setters
    void SetTestNumber(const long);
    void SetTestName(const QString);
    void SetTestUnit(const QString);
    void SetResultScale(int resScale);
    void SetLowLimit(const double);
    void SetHighLimit(const double);
    void SetValidLowLimit(const bool);
    void SetValidHighLimit(const bool);
    void SetLowSpecLimit(const double);
    void SetHighSpecLimit(const double);
    void SetValidLowSpecLimit(const bool);
    void SetValidHighSpecLimit(const bool);
    void SetTestValue(const double);
    void SetTestValueForSite(const int aSite, const double aValue);
    void SetStaticHeaderWritten(const bool);
    void SetStaticMultiLimitsWritten(const bool);
    void SetExecCount(const unsigned long);
    void SetExecFail(const unsigned long);
    void SetExecTime(const float);
    void SetTestTxt(const QString);
    void SetSpecificFlags(int flags);
    void SetIsParamatricTest(bool);

    /// !brief Increment the number of fail tests
    void IncrementFailTest();
    /// !brief Increment the number of execution tests
    void IncrementExecTest();

    enum MultiLimit
    {
        KeepDuplicateMultiLimit = 0,
        SkipDuplicateMultiLimit
    };

    void  AddMultiLimitItem(const GS::Core::MultiLimitItem& lMLS, MultiLimit keepDuplicate = SkipDuplicateMultiLimit);
    int   GetMultiLimitCount() const;
    const GS::Core::MultiLimitItem&    GetMultiLimitSetAt(int lIndex) const;
    GS::Core::MultiLimitItem &GetMultiLimit(int lIndex);

protected:
    long            mTestNumber;
    QString         mTestName;              ///\param Parameter name. E.g: "Idd_Total_Shutdown"
    QString         mTestUnits;             ///\param Parameter units,E.g: "A"
    int             mResScale;              ///\param Result sclaing factor
    double          mLowLimit;              ///\param Parameter Low limit, E.g: 0.00004
    double          mHighLimit;             ///\param Parameter High limit, E.g: -0.00004
    bool            mValidLowLimit;         ///\param Low limit defined
    bool            mValidHighLimit;        ///\param High limit defined
    double          mLowSpecLimit;          ///\param Parameter Low Spec limit, E.g: 0.00004
    double          mHighSpecLimit;         ///\param Parameter High Spec limit, E.g: -0.00004
    bool            mValidLowSpecLimit;     ///\param Low limit defined
    bool            mValidHighSpecLimit;    ///\param High limit defined
    double          mTestValue;             ///\param Parameter result
    fValueOfSiteMap mTestValueBySite;       ///\param Parameter result stored by site
    bool            mStaticHeaderWritten;   ///\param 'true' after first STDF PTR static header data written.
    bool            mStaticMultiLimitsWritten; ///\param 'true' after first STDF PTR static multi limit data written.
    unsigned long   mExecCount;             ///\param Number of execution of the test.
    unsigned long   mExecFail;              ///\param Number of fail of this test.
    float           mExecTime;              ///\param Test execution time
    QString         mTestTxt;               ///\param Test descr ipt ion text or label

    QList<GS::Core::MultiLimitItem>   mMultiLimits;
    int             mSpecificFlags;         ///\param Custom flags defined for specific parsers
    int             mIsParametricTest;      ///\param True if test is parametric
};

}
}

#endif // PARSERPARAMETER_H
