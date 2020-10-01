#ifndef PAT_MV_RULE_H
#define PAT_MV_RULE_H

#include "pat_global.h"
#include <QList>
#include <QMetaType>

namespace GS
{
namespace Gex
{

class PATMultiVariateRule
{
public:
    class MVTestData
    {
    public:
        MVTestData(const QString &,int,int);
        virtual ~MVTestData();
        MVTestData(const MVTestData &);
        MVTestData& operator=(const MVTestData& lOther);
        bool operator==(const MVTestData& lOther);

        QString GetTestName() const;
        void SetTestName(const QString &);

        int GetTestNumber() const;
        void SetTestNumber(int);

        int GetPinIdx() const;
        void SetPinIdx(int);

    private:
        QString mTestName;
        int mTestNumber;
        int mPinIdx;
    };

    PATMultiVariateRule();
    PATMultiVariateRule(const PATMultiVariateRule& lOther);
    bool operator==(const PATMultiVariateRule& lOther);
    virtual ~PATMultiVariateRule();

    PATMultiVariateRule&	operator=(const PATMultiVariateRule& lOther);

    enum RuleType
    {
        Generated,
        Manual
    };

    void            AddTestData(const MVTestData &data);
    int             RemoveAllTestData(const MVTestData &data);

    bool            GetEnabled() const;
    int             GetBin() const;
    double          GetCustomDistance() const;
    int             GetPrincipalComponents() const;
    const QString&  GetName() const;
    PAT::OutlierDistance    GetOutlierDistanceMode() const;
    QString         GetOutlierDistanceModeString() const;
    RuleType        GetType() const;
    QString         GetTypeString() const;
    const QList<MVTestData> &GetMVTestData() const;

    void            SetEnabled(bool lEnable);
    void            SetBin(int lBin);
    void            SetCustomDistance(double lDistance);
    void            SetPrincipalComponents(int lMaxComponents);
    void            SetName(const QString& lName);
    void            SetOutlierDistanceMode(PAT::OutlierDistance lMode);
    void            SetRule(RuleType lType);

private:

    bool                    mEnabled;
    int                     mBin;
    double                  mCustomDistance;
    int                     mPrincipalComponents;
    QString                 mName;
    PAT::OutlierDistance    mOutlierDistanceMode;
    RuleType                mType;
    QList<MVTestData>       mTestData;

};

}   // namespace Gex
}   // namespace GS

Q_DECLARE_METATYPE(GS::Gex::PATMultiVariateRule::RuleType)
Q_DECLARE_METATYPE(GS::Gex::PAT::OutlierDistance)


#endif // PAT_MV_RULE_H
