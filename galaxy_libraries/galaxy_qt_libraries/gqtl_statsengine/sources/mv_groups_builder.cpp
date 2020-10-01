#include "math.h"
#include "mv_groups_builder.h"
#include "r_engine.h"
#include "Rinternals.h"
#include "r_vector.h"
#include "r_data.h"
#include "g_sexp.h"
#include "r_algo_private.h"
#include "r_matrix.h"
#include <QDebug>

#define MV_GROUP_FILE ":/r_scripts/mv_group_builder.R"

namespace GS
{
namespace SE
{

MVGroupsBuilder::MVGroupsBuilder()
    :RAlgo()
{
    Q_D(RAlgo);
    d->mScripts << MV_GROUP_FILE;
    Init();
}

MVGroupsBuilder::~MVGroupsBuilder()
{

}

void MVGroupsBuilder::InitRequiredData()
{
    Q_D(RAlgo);
    d->mRequiredData.clear();
    d->mRequiredData.insert(StatsData::MVGROUP_IN_1, "mvgroup_in_1");
    d->mRequiredData.insert(StatsData::MVGROUP_OUT_1, "mvgroup_out_1");
}

void MVGroupsBuilder::InitRequiredParameters()
{
    Q_D(RAlgo);
    d->mRequiredParameter.clear();
    d->mRequiredParameter.append(StatsAlgo::CORR_THRESHOLD);
}

int MVGroupsBuilder::GetGroupsSize()
{
    Q_D(RAlgo);
    RObject* lObject = static_cast<RData*>(d->mResults)->GetRObject(StatsData::MVGROUP_OUT_1);
    if (!lObject)
        return -1;

    RVector* lResults = static_cast<RVector*>(lObject);
    if (!lResults)
        return -1;

    return lResults->GetSize(QList<int>());
}

RVector MVGroupsBuilder::GetGroup(int groupId)
{
    Q_D(RAlgo);
    RObject* lObject = static_cast<RData*>(d->mResults)->GetRObject(StatsData::MVGROUP_OUT_1);
    if (!lObject)
        return RVector();
    RVector* lResults = static_cast<RVector*>(lObject);
    if (!lResults)
        return RVector();

    bool lOk = true;
    GSexp* lSexp = lResults->GetItem(groupId, lOk);
    if (!lOk || !lSexp->mSexp || !Rf_isVector(lSexp->mSexp))
    {
        d->mErrors.append(QString("Unable to retrieve group %1").arg(groupId));
        return RVector();
    }

    return RVector(lSexp);
}



QList<QList<int> > MVGroupsBuilder::GetGroups(bool& ok)
{
    Q_D(RAlgo);
    ok = false;
    QList<QList<int> > lGroups;
    RObject* lObject = static_cast<RData*>(d->mResults)->GetRObject(StatsData::MVGROUP_OUT_1);
    if (!lObject)
        return lGroups;

    RVector* lResults = static_cast<RVector*>(lObject);
    if (!lResults)
        return lGroups;

    int lVectSize = lResults->GetSize();
    for (int i = 0; i < lVectSize; ++i)
    {
        lGroups.append(QList<int>());
        QList<int> lIndexes;
        lIndexes.append(i);
        int lVectGroupSize = lResults->GetSize(lIndexes);
        for (int j = 0; j < lVectGroupSize; ++j)
        {
            QList<int> lItemIndexes = lIndexes;
            lItemIndexes.append(j);
            int lTest = lResults->GetIntegerItem(lItemIndexes, ok);
            if (!ok)
                return lGroups;
            lGroups[i].append(lTest);
        }
    }

    ok = true;
    return lGroups;
}

bool MVGroupsBuilder::DoExecute(QMap<Parameter, QVariant> params, StatsData *data)
{
    Q_D(RAlgo);
//#ifdef QT_DEBUG
//    QMap<QString, QVariant> lMemInfo=CGexSystemUtils::GetMemoryInfo(false, true);
//    printf("Group builder start\n");
//    printf("Free mem: %s",lMemInfo.value("freeram").toString().toLatin1().data());
//    printf(" - Used mem:: %s\n",lMemInfo.value("MemUsedByProcess").toString().toLatin1().data());
//#endif

    if (!data)
    {
        d->mErrors.append("No input data");
        return false;
    }

    QString lRFunction = "groupBuilder";
    bool lOk;
    double lThreshold;

    lThreshold = params.value(CORR_THRESHOLD).toDouble(&lOk);
    if (!lOk)
    {
        d->mErrors.append(QString("Parameter corr threshold is not a double value: %1")
                       .arg(params.value(CORR_THRESHOLD).toString()));
        return false;
    }

    //    QString lInputMatrix = d->mRequiredData.value(StatsData::StatsData::MVGROUP_IN_1);
    QString lInputMatrix = static_cast<RData*>(data)->GetRObject(StatsData::MVGROUP_IN_1)->GetName();
    QString lOuputList = d->mRequiredData.value(StatsData::MVGROUP_OUT_1);

    SEXP lCall = PROTECT(Rf_lang2(
                             Rf_install("try"),                             // avoid crash
                             Rf_lang3(
                                 Rf_install(lRFunction.toLatin1().data()),  // function
                                 Rf_install(lInputMatrix.toLatin1().data()),// data
                                 Rf_ScalarReal(lThreshold))));              // threshold
    UNPROTECT(1);

    if (!lCall)
    {
        d->mErrors.append(QString("Error when loading %1 function").arg(lRFunction));
        return false;
    }

    int lErrorOccurred;
    SEXP lRes = PROTECT(R_tryEvalSilent(lCall, R_GlobalEnv, &lErrorOccurred));
    UNPROTECT(1);

    if (lErrorOccurred || !lRes || Rf_inherits(lRes, "try-error"))
    {
        d->mErrors.append(QString("Error when calling %1 function: %2").arg(lRFunction).arg(QString(R_curErrorBuf())));
        return false;
    }

    RVector* lVector = new RVector(new GSexp(lRes));
    if (lVector->IsEmpty())
    {
        d->mErrors.append(QString("No result for this dataset").arg(lRFunction));
        delete lVector;
        lVector = 0;
        return false;
    }

    if (!lVector->Protect(lOuputList))
    {
        d->mErrors.append(QString("Unable to protect result vector").arg(lRFunction));
        delete lVector;
        lVector = 0;
        return false;
    }

    if (!d->mResults)
        d->mResults = new RData();
    RData* lData = static_cast<RData*>(d->mResults);
    lData->Insert(lVector, StatsData::MVGROUP_OUT_1);
//#ifdef QT_DEBUG
//    lMemInfo=CGexSystemUtils::GetMemoryInfo(false, true);
//    printf("Group builder end\n");
//    printf("Free mem: %s",lMemInfo.value("freeram").toString().toLatin1().data());
//    printf(" - Used mem:: %s\n",lMemInfo.value("MemUsedByProcess").toString().toLatin1().data());
//    fflush(stdout);
//#endif
    return true;
}

bool MVGroupsBuilder::CheckInputData(StatsData *data)
{
    Q_D(RAlgo);
    if (!GS::SE::StatsAlgo::CheckInputData(data))
        return false;
#ifdef SIMON
    RMatrix* mMatrix = static_cast<RMatrix*>(static_cast<RData*>(data)->GetRObject(StatsData::MVGROUP_IN_1));
    QList<double> lSums;
    for (int i = 0; i < mMatrix->GetCols(); ++i)
    {
        double lSum = 0;
        bool ok = false;
        for (int j = 0; j < mMatrix->GetRows(); ++j)
        {
            double lItem = mMatrix->GetItem(j,i,ok);
            if (!isnan(lItem) && lItem != R_NaN && ok && lItem != R_NaReal && lItem != R_NegInf && lItem != R_PosInf)
                lSum += lItem;
        }
        lSums.append(lSum);
    }
    qSort(lSums.begin(), lSums.end());
    QString lDebug;
    for (int i = 0; i < lSums.size(); ++i)
    {
        lDebug.append(QString::number(lSums.at(i))).append("\n");
    }
    qDebug() << lDebug;
#endif

    RMatrix* mMatrix = static_cast<RMatrix*>(static_cast<RData*>(data)->GetRObject(StatsData::MVGROUP_IN_1));
    if (mMatrix->GetCols() <= 0)
    {
        d->mErrors.append("No column in allocated matrix");
        return false;
    }

    if (mMatrix->GetRows() <= 0)
    {
        d->mErrors.append("No row in allocated matrix");
        return false;
    }

    return true;
}

} // namespace SE
} // namespace GS

