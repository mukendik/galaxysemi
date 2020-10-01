#ifndef PAT_NNR_SUMMARY_H
#define PAT_NNR_SUMMARY_H

#include <QMap>
#include <QString>

class CNNR_Rule;

namespace GS
{
namespace Gex
{

class PATNNRTestSummary
{
public:

    PATNNRTestSummary();
    PATNNRTestSummary(int lTestNumber, int lPinmap, const QString& lTestName);
    PATNNRTestSummary(const PATNNRTestSummary& other);
    ~PATNNRTestSummary();

    PATNNRTestSummary&  operator=(const PATNNRTestSummary& other);
    bool                operator<(const PATNNRTestSummary& other) const;

    void                AddOutlier(const QString& lRuleName, int lCount);

    int                 GetTestNumber() const;
    int                 GetPinmap() const;
    const QString&      GetTestName() const;
    int                 GetOutlierCount() const;
    const QMap<QString, int> &GetRuleSummary() const;

    void                SetTestNumber(int lTestNumber);
    void                SetPinmap(int lPinmap);
    void                SetTestName(const QString& lTestName);

protected:

    int                 mTestNumber;
    int                 mPinmap;
    QString             mTestName;

    QMap<QString, int>  mRuleSummary;
};

}
}
#endif // PAT_NNR_SUMMARY_H
