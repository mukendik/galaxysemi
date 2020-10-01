
#include "shape_identifier.h"

#include "g_sexp.h"
#include "r_data.h"
#include "r_vector.h"
#include "r_algo_private.h"

#include <QSize>

#define SHAPE_IDENTIFIER_FILE ":/r_scripts/shape_analysis.R"


namespace GS
{
namespace SE
{

ShapeIdentifier::ShapeIdentifier()
    :RAlgo()
{
    Q_D(RAlgo);
    d->mScripts << SHAPE_IDENTIFIER_FILE;
    Init();
}

ShapeIdentifier::~ShapeIdentifier()
{

}

RVector ShapeIdentifier::GetShapeName(bool &ok) const
{
    ok = false;
    if (!d_func()->mResults)
        return 0;
    RObject* lObject = static_cast<RData*>(d_func()->mResults)->GetRObject(StatsData::SHAPEIDENTIFIER_OUT);
    if (!lObject)
        return RVector();
    RVector* lResults = static_cast<RVector*>(lObject);
    if (!lResults)
        return RVector();

    GSexp* lSexp = lResults->GetItem(SHAPE, ok);
    if (!ok || !lSexp->mSexp || !Rf_isVector(lSexp->mSexp))
        return RVector();

    return RVector(lSexp);
}

RVector ShapeIdentifier::GetConfidenceLevel(bool &ok) const
{
    ok = false;
    if (!d_func()->mResults)
        return 0;
    RObject* lObject = static_cast<RData*>(d_func()->mResults)->GetRObject(StatsData::SHAPEIDENTIFIER_OUT);
    if (!lObject)
        return RVector();
    RVector* lResults = static_cast<RVector*>(lObject);
    if (!lResults)
        return RVector();

    GSexp* lSexp = lResults->GetItem(CONFIDENCE, ok);
    if (!ok || !lSexp->mSexp || !Rf_isVector(lSexp->mSexp))
        return RVector();

    return RVector(lSexp);
}

RVector ShapeIdentifier::GetBimodalSplitValue(bool &ok) const
{
    ok = false;
    if (!d_func()->mResults)
        return 0;
    RObject* lObject = static_cast<RData*>(d_func()->mResults)->GetRObject(StatsData::SHAPEIDENTIFIER_OUT);
    if (!lObject)
        return RVector();
    RVector* lResults = static_cast<RVector*>(lObject);
    if (!lResults)
        return RVector();

    GSexp* lSexp = lResults->GetItem(BIMODAL_SPLIT, ok);
    if (!ok || !lSexp->mSexp || !Rf_isVector(lSexp->mSexp))
        return RVector();

    return RVector(lSexp);
}

bool ShapeIdentifier::BuildHistogram(RVector& data,
                                    const QString &filePath,
                                    const QSize &size,
                                    const QString &title)
{
    Q_D(RAlgo);
    QString lRFunction = "GSShapeNormHistogram";

    if (data.IsNull())
        return false;

    if (!data.IsProtected())
        data.Protect("invector");

    RVector lVectorSize;
    lVectorSize.Build("graphsize", 2, RVector::V_INT);
    lVectorSize.FillInt(0, size.height());
    lVectorSize.FillInt(1, size.width());

    RVector lVectorParameters;
    lVectorParameters.Build("graphparam", 4, RVector::V_STD);
    lVectorParameters.FillStd(0, lVectorSize);
    lVectorParameters.FillStd(1, title);

    SEXP lCall = PROTECT(Rf_lang2(
                             Rf_install("try"),                                             // avoid crash
                             Rf_lang4(
                                 Rf_install(lRFunction.toLatin1().data()),                  // function
                                 Rf_install(data.GetName().toLatin1().data()),              // data
                                 Rf_mkString(filePath.toLatin1().data()),                   // filepath
                                 Rf_install(lVectorParameters.GetName().toLatin1().data())  // params
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

void ShapeIdentifier::InitRequiredData()
{
    Q_D(RAlgo);
    d->mRequiredData.clear();
    d->mRequiredData.insert(StatsData::SHAPEIDENTIFIER_IN, "shape_identifier_in");
    d->mRequiredData.insert(StatsData::SHAPEIDENTIFIER_OUT, "shape_identifier_out");
}

void ShapeIdentifier::InitRequiredParameters()
{
    Q_D(RAlgo);
    d->mRequiredParameter.clear();
    // add here mandatory input parameter to check
}

bool ShapeIdentifier::DoExecute(QMap<StatsAlgo::Parameter, QVariant> params, StatsData *data)
{
    Q_D(RAlgo);
    if (!data)
    {
        d->mErrors.append("No input data");
        return false;
    }

    QString lRFunction = "shape_analysis";

    QString lInputVector = d->mRequiredData.value(StatsData::SHAPEIDENTIFIER_IN);
    QString lOuputList = d->mRequiredData.value(StatsData::SHAPEIDENTIFIER_OUT);

    bool lOk;
    int lDistinctValue = params.value(DISTINCT_VALUES).toInt(&lOk);
    if (params.contains(DISTINCT_VALUES) && !lOk)
    {
        d->mErrors.append(QString("Parameter distinct values is not an integer value: %1")
                       .arg(params.value(DISTINCT_VALUES).toString()));
        return false;
    }


    SEXP lCall = PROTECT(Rf_lang2(
                             Rf_install("try"),                                 // avoid crash
                             Rf_lang3(
                                 Rf_install(lRFunction.toLatin1().data()),      // function
                                 Rf_install(lInputVector.toLatin1().data()),
                                 Rf_ScalarInteger(lDistinctValue)
                                 ))); // data
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
    lData->Insert(lVector, StatsData::SHAPEIDENTIFIER_OUT);

    return true;
}

} // namespace SE
} // namespace GS
