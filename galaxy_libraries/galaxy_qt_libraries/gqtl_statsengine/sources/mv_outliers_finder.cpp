
#include "mv_outliers_finder.h"
#include "r_engine.h"
#include "Rinternals.h"
#include "r_object.h"
#include "r_data.h"
#include "g_sexp.h"
#include "r_algo_private.h"
#include "math.h"

#define MV_OUTLIERS_FILE ":/r_scripts/mv_outliers_finder.R"
#define MV_GRAPHS_FILE ":/r_scripts/graphs.R"

namespace GS
{
namespace SE
{

MVOutliersFinder::MVOutliersFinder()
    :RAlgo()
{
    Q_D(RAlgo);
    d->mScripts << MV_OUTLIERS_FILE;
    d->mScripts << MV_GRAPHS_FILE;
    Init();
}

MVOutliersFinder::~MVOutliersFinder()
{

}

void MVOutliersFinder::InitRequiredData()
{
    Q_D(RAlgo);
    d->mRequiredData.clear();
    d->mRequiredData.insert(StatsData::MVOUTLIER_IN_1, "mvoutlier_in_1");
    d->mRequiredData.insert(StatsData::MVOUTLIER_OUT_1, "mvoutlier_out_1");
}

void MVOutliersFinder::InitRequiredParameters()
{
    Q_D(RAlgo);
    d->mRequiredParameter.clear();
    d->mRequiredParameter.append(StatsAlgo::SIGMA);
}

bool MVOutliersFinder::DoExecute(QMap<Parameter, QVariant> params, StatsData *data)
{
    Q_D(RAlgo);

//#ifdef QT_DEBUG
//    QMap<QString, QVariant> lMemInfo=CGexSystemUtils::GetMemoryInfo(false, true);
//    printf("Outlier finder start\n");
//    printf("Free mem: %s",lMemInfo.value("freeram").toString().toLatin1().data());
//    printf(" - Used mem:: %s\n",lMemInfo.value("MemUsedByProcess").toString().toLatin1().data());
//#endif
    if (!data)
    {
        d->mErrors.append("No input data");
        return false;
    }

    QString lRFunction = "findOutliers";
    bool lOk;
    double lSigma = params.value(SIGMA).toDouble(&lOk);
    if (!lOk)
    {
        d->mErrors.append(QString("Parameter corr sigma is not a double value: %1")
                       .arg(params.value(SIGMA).toString()));
        return false;
    }

    int lMaxComponent = params.value(MAX_COMPONENT).toInt(&lOk);
    if (params.contains(MAX_COMPONENT) && !lOk)
    {
        d->mErrors.append(QString("Parameter max component is not an integer value: %1")
                       .arg(params.value(MAX_COMPONENT).toString()));
        return false;
    }


//    QString lInputMatrix = d->mRequiredData.value(StatsData::MVOUTLIER_IN_1);
    QString lInputMatrix = static_cast<RData*>(data)->GetRObject(StatsData::MVOUTLIER_IN_1)->GetName();
    QString lOuputList = d->mRequiredData.value(StatsData::MVOUTLIER_OUT_1);

    SEXP lCall = PROTECT(Rf_lang2(
                             Rf_install("try"),                                 // avoid crash
                             Rf_lang4(
                                 Rf_install(lRFunction.toLatin1().data()),      // function
                                 Rf_install(lInputMatrix.toLatin1().data()),    // data
                                 Rf_ScalarReal(lSigma),                         // sigma
                                 Rf_ScalarInteger(lMaxComponent))));            // max component
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
        d->mErrors.append(QString("No result with %1 for this dataset").arg(lRFunction));
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
    lData->Insert(lVector, StatsData::MVOUTLIER_OUT_1);


//#ifdef QT_DEBUG
//    lMemInfo=CGexSystemUtils::GetMemoryInfo(false, true);
//    printf("Outlier finder end\n");
//    printf("Free mem: %s",lMemInfo.value("freeram").toString().toLatin1().data());
//    printf(" - Used mem:: %s\n",lMemInfo.value("MemUsedByProcess").toString().toLatin1().data());
//    fflush(stdout);
//#endif

    return true;
}

RVector MVOutliersFinder::GetOutliers(bool& ok) const
{
    ok = false;
    if (!d_func()->mResults)
        return RVector();
    RObject* lObject = static_cast<RData*>(d_func()->mResults)->GetRObject(StatsData::MVOUTLIER_OUT_1);
    if (!lObject)
        return RVector();
    RVector* lResults = static_cast<RVector*>(lObject);
    if (!lResults)
        return RVector();

    GSexp* lSexp = lResults->GetItem(OUTLIERS, ok);
    if (!ok || !lSexp->mSexp || !Rf_isVector(lSexp->mSexp))
        return RVector();

    return RVector(lSexp);
}

double MVOutliersFinder::GetChi(bool &ok) const
{
    ok = false;
    if (!d_func()->mResults)
        return 0;
    RObject* lObject = static_cast<RData*>(d_func()->mResults)->GetRObject(StatsData::MVOUTLIER_OUT_1);
    if (!lObject)
        return 0;
    RVector* lResults = static_cast<RVector*>(lObject);
    if (!lResults)
        return 0;

    double lChi = lResults->GetDoubleItem(CHI, ok);
    if (!ok)
        return 0;

    return lChi;
}

RVector MVOutliersFinder::GetMD(bool &ok) const
{
    ok = false;
    if (!d_func()->mResults)
        return RVector();
    RObject* lObject = static_cast<RData*>(d_func()->mResults)->GetRObject(StatsData::MVOUTLIER_OUT_1);
    if (!lObject)
        return RVector();
    RVector* lResults = static_cast<RVector*>(lObject);
    if (!lResults)
        return RVector();

    GSexp* lSexp = lResults->GetItem(MAHALANOBISD, ok);
    if (!ok || !lSexp->mSexp || !Rf_isVector(lSexp->mSexp))
        return RVector();

    return RVector(lSexp);
}

RVector MVOutliersFinder::GetMD2(bool &ok) const
{
    ok = false;
    if (!d_func()->mResults)
        return RVector();
    RObject* lObject = static_cast<RData*>(d_func()->mResults)->GetRObject(StatsData::MVOUTLIER_OUT_1);
    if (!lObject)
        return RVector();
    RVector* lResults = static_cast<RVector*>(lObject);
    if (!lResults)
        return RVector();

    GSexp* lSexp = lResults->GetItem(MAHALANOBISD2, ok);
    if (!ok || !lSexp->mSexp || !Rf_isVector(lSexp->mSexp))
        return RVector();

    return RVector(lSexp);
}

RMatrix MVOutliersFinder::GetPCA(bool& ok) const
{
    ok = false;
    if (!d_func()->mResults)
        return RMatrix();
    RObject* lObject = static_cast<RData*>(d_func()->mResults)->GetRObject(StatsData::MVOUTLIER_OUT_1);
    if (!lObject)
        return RMatrix();
    RVector* lResults = static_cast<RVector*>(lObject);
    if (!lResults)
        return RMatrix();

    GSexp* lSexp = lResults->GetItem(PCA, ok);
    if (!ok || !lSexp->mSexp || !Rf_isMatrix(lSexp->mSexp))
        return RMatrix();

    return RMatrix(lSexp);
}

RVector MVOutliersFinder::GetZScores(bool& ok) const
{
    ok = false;
    if (!d_func()->mResults)
        return 0;
    RObject* lObject = static_cast<RData*>(d_func()->mResults)->GetRObject(StatsData::MVOUTLIER_OUT_1);
    if (!lObject)
        return RVector();
    RVector* lResults = static_cast<RVector*>(lObject);
    if (!lResults)
        return RVector();

    GSexp* lSexp = lResults->GetItem(ZSCORES, ok);
    if (!ok || !lSexp->mSexp || !Rf_isVector(lSexp->mSexp))
        return RVector();

    return RVector(lSexp);
}

double MVOutliersFinder::GetSigma(bool &ok) const
{
    ok = false;
    if (!d_func()->mResults)
        return 0;
    RObject* lObject = static_cast<RData*>(d_func()->mResults)->GetRObject(StatsData::MVOUTLIER_OUT_1);
    if (!lObject)
        return 0;
    RVector* lResults = static_cast<RVector*>(lObject);
    if (!lResults)
        return 0;

    double lSigma = lResults->GetDoubleItem(SIGMAVALUE, ok);
    if (!ok)
        return 0;

    return lSigma;
}

RVector MVOutliersFinder::GetNaParts(bool &ok) const
{
    ok = false;
    if (!d_func()->mResults)
        return 0;
    RObject* lObject = static_cast<RData*>(d_func()->mResults)->GetRObject(StatsData::MVOUTLIER_OUT_1);
    if (!lObject)
        return RVector();
    RVector* lResults = static_cast<RVector*>(lObject);
    if (!lResults)
        return RVector();

    GSexp* lSexp = lResults->GetItem(NAROWS, ok);
    if (!ok || !lSexp->mSexp || !Rf_isVector(lSexp->mSexp))
        return RVector();

    return RVector(lSexp);
}

int MVOutliersFinder::GetPcNumber(bool &ok) const
{
    ok = false;
    if (!d_func()->mResults)
        return 0;
    RObject* lObject = static_cast<RData*>(d_func()->mResults)->GetRObject(StatsData::MVOUTLIER_OUT_1);
    if (!lObject)
        return 0;
    RVector* lResults = static_cast<RVector*>(lObject);
    if (!lResults)
        return 0;

    int lPcNumber = lResults->GetIntegerItem(PCNUMBER, ok);
    if (!ok)
        return 0;

    return lPcNumber;
}

bool MVOutliersFinder::BuildZScoresHisto(const QString &filePath,
                                         const QSize &size,
                                         const QString &title,
                                         const QString &xLabel,
                                         const QString &yLabel)
{
    Q_D(RAlgo);
    QString lRFunction = "GSNormHistogram";
    bool lOk = false;
    double lSigma = GetSigma(lOk);

    if (!lOk)
    {
        d->mErrors.append(QString("Unable to retrieve input sigma"));
        return false;
    }

    RVector lZVector = GetZScores(lOk);

    if (!lOk)
    {
        d->mErrors.append(QString("Unable to retrieve ZScores from result"));
        return false;
    }

    if (!lZVector.IsProtected())
        lZVector.Protect("zscore");

    RVector lVectorSize;
    lVectorSize.Build("graphsize", 2, RVector::V_INT);
    lVectorSize.FillInt(0, size.height());
    lVectorSize.FillInt(1, size.width());

    RVector lVectorParameters;
    lVectorParameters.Build("graphparam", 4, RVector::V_STD);
    lVectorParameters.FillStd(0, lVectorSize);
    lVectorParameters.FillStd(1, title);
    lVectorParameters.FillStd(2, xLabel);
    lVectorParameters.FillStd(3, yLabel);

    SEXP lCall = PROTECT(Rf_lang2(
                             Rf_install("try"),                                        // avoid crash
                             Rf_lang5(
                                 Rf_install(lRFunction.toLatin1().data()),             // function
                                 Rf_install(lZVector.GetName().toLatin1().data()),     // data
                                 Rf_ScalarReal(lSigma),                                // sigma
                                 Rf_mkString(filePath.toLatin1().data()),              // filepath
                                 Rf_install(lVectorParameters.GetName().toLatin1().data())// params
                                 )));
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

    if (Rf_asLogical(lRes) != 1)
    {
        d->mErrors.append(QString("Error when building histogram"));
        return false;
    }

    return true;
}

bool MVOutliersFinder::BuildZScoresTrend(const QString &filePath,
                                         const RVector &labels,
                                         const QSize &size,
                                         const QString &title,
                                         const QString &xLabel,
                                         const QString &yLabel)
{
    Q_D(RAlgo);
    QString lRFunction = "GSTrendChart";
    bool lOk = false;
    double lSigma = GetSigma(lOk);

    if (!lOk)
    {
        d->mErrors.append(QString("Unable to retrieve input sigma"));
        return false;
    }

    RVector lZVector = GetZScores(lOk);

    if (!lOk)
    {
        d->mErrors.append(QString("Unable to retrieve ZScores from result"));
        return false;
    }

    if (!lZVector.IsProtected())
        lZVector.Protect("zscore");

    RVector lVectorSize;
    lVectorSize.Build("graphsize", 2, RVector::V_INT);
    lVectorSize.FillInt(0, size.height());
    lVectorSize.FillInt(1, size.width());

    RVector lVectorParameters;
    lVectorParameters.Build("graphparam", 5, RVector::V_STD);
    lVectorParameters.FillStd(0, lVectorSize);
    lVectorParameters.FillStd(1, labels);
    lVectorParameters.FillStd(2, title);
    lVectorParameters.FillStd(3, xLabel);
    lVectorParameters.FillStd(4, yLabel);

    SEXP lCall = PROTECT(Rf_lang2(
                             Rf_install("try"),                                            // avoid crash
                             Rf_lang5(
                                 Rf_install(lRFunction.toLatin1().data()),                 // function
                                 Rf_install(lZVector.GetName().toLatin1().data()),         // data
                                 Rf_ScalarReal(lSigma),                                    // sigma
                                 Rf_mkString(filePath.toLatin1().data()),                  // filepath
                                 Rf_install(lVectorParameters.GetName().toLatin1().data()) // params (size,labels)
                                 )));
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


    if (Rf_asLogical(lRes) != 1)
    {
        d->mErrors.append(QString("Error when building histogram"));
        return false;
    }

    return true;
}

bool MVOutliersFinder::BuildZScoresQQPlot(const QString &filePath,
                                          const QSize &size,
                                          const QString &title,
                                          const QString &xLabel,
                                          const QString &yLabel)
{
    Q_D(RAlgo);
    QString lRFunction = "GSQQPlot";
    bool lOk = false;
    double lSigma = GetSigma(lOk);

    if (!lOk)
    {
        d->mErrors.append(QString("Unable to retrieve input sigma"));
        return false;
    }

    RVector lZVector = GetZScores(lOk);

    if (!lOk)
    {
        d->mErrors.append(QString("Unable to retrieve ZScores from result"));
        return false;
    }

    if (!lZVector.IsProtected())
        lZVector.Protect("zscore");

    RVector lVectorSize;
    lVectorSize.Build("graphsize", 2, RVector::V_INT);
    lVectorSize.FillInt(0, size.height());
    lVectorSize.FillInt(1, size.width());

    RVector lVectorParameters;
    lVectorParameters.Build("graphparam", 4, RVector::V_STD);
    lVectorParameters.FillStd(0, lVectorSize);
    lVectorParameters.FillStd(1, title);
    lVectorParameters.FillStd(2, xLabel);
    lVectorParameters.FillStd(3, yLabel);

    SEXP lCall = PROTECT(Rf_lang2(
                             Rf_install("try"),                                            // avoid crash
                             Rf_lang5(
                                 Rf_install(lRFunction.toLatin1().data()),             // function
                                 Rf_install(lZVector.GetName().toLatin1().data()),     // data
                                 Rf_ScalarReal(lSigma),                                // sigma
                                 Rf_mkString(filePath.toLatin1().data()),              // filepath
                                 Rf_install(lVectorParameters.GetName().toLatin1().data())// params
                                 )));
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

    if (Rf_asLogical(lRes) != 1)
    {
        d->mErrors.append(QString("Error when building histogram"));
        return false;
    }

    return true;
}

bool MVOutliersFinder::BuildCorrelationChart(RMatrix& data,
                                             int col1,
                                             int col2,
                                             const QString& filePath,
                                             const RVector& labels,
                                             const QSize& size,
                                             const QString &title,
                                             const QString &xLabel,
                                             const QString &yLabel,
                                             double chi,
                                             double sigma)
{
    Q_D(RAlgo);
    QString lRFunction = "GSCorrelationChart";

    if (data.IsNull())
        return false;

    if (isinf(chi) || isinf(sigma))
    {
        d->mErrors.append("unhandled infinite value of chi or sigma");
        return false;
    }

    if (!data.IsProtected())
        data.Protect("inmatrix");

    RVector lVectorSize;
    lVectorSize.Build("graphsize", 2, RVector::V_INT);
    lVectorSize.FillInt(0, size.height());
    lVectorSize.FillInt(1, size.width());

    RVector lVectorSelection;
    lVectorSelection.Build("selectedcols", 2, RVector::V_INT);
    lVectorSelection.FillInt(0, col1);
    lVectorSelection.FillInt(1, col2);

    RVector lVectorChiSigma;
    lVectorChiSigma.Build("chi_sigma", 2, RVector::V_DOUBLE);
    lVectorChiSigma.FillDouble(0, chi);
    lVectorChiSigma.FillDouble(1, sigma);

    bool lOk;
    RVector lOutliers = GetOutliers(lOk);
    if (!lOk)
        return false;

    RVector lVectorParameters;
    lVectorParameters.Build("graphparam", 8, RVector::V_STD);
    lVectorParameters.FillStd(0, lVectorSize);
    lVectorParameters.FillStd(1, labels);
    lVectorParameters.FillStd(2, lVectorSelection);
    lVectorParameters.FillStd(3, lOutliers);
    lVectorParameters.FillStd(4, title);
    lVectorParameters.FillStd(5, xLabel);
    lVectorParameters.FillStd(6, yLabel);
    lVectorParameters.FillStd(7, lVectorChiSigma);

    SEXP lCall = PROTECT(Rf_lang2(
                             Rf_install("try"),                             // avoid crash
                             Rf_lang4(
                                 Rf_install(lRFunction.toLatin1().data()),                 // function
                                 Rf_install(data.GetName().toLatin1().data()),             // data
                                 Rf_mkString(filePath.toLatin1().data()),                  // filepath
                                 Rf_install(lVectorParameters.GetName().toLatin1().data()) // params (size,labels,selection,outliers)
                                 )));
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

    if (Rf_asLogical(lRes) != 1)
    {
        d->mErrors.append(QString("Error when building histogram"));
        return false;
    }

    return true;
}

} // namespace SE
} // namespace GS


