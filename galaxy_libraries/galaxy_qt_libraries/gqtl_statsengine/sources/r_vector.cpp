
#include "r_vector.h"
#include "r_object.h"
#include "r_object_private.h"
#include "r_protected_object.h"
#include "g_sexp.h"

namespace GS
{
namespace SE
{

class RVectorPrivate: public RObjectPrivate
{
public:
    RVectorPrivate()
        :RObjectPrivate(), mIntVect(0), mDoubleVect(0), mStdVect(0), mType(RVector::V_INT){}

    RVectorPrivate(GSexp* gsexp)
        :RObjectPrivate(gsexp), mIntVect(0), mDoubleVect(0), mStdVect(0), mType(RVector::V_INT){}

    int*            mIntVect;       ///< holds pointer to int vector if any
    double*         mDoubleVect;    ///< holds pointer to double vector if any
    SEXP*           mStdVect;       ///< holds pointer to standard vector if any
    RVector::Type   mType;          ///< holds vector type
};

RVector::RVector()
    :RObject(* new RVectorPrivate)
{
}

RVector::RVector(GSexp *sexp)
    :RObject(* new RVectorPrivate(sexp))
{
}

RVector::RVector(RVectorPrivate &lPrivateData)
    :RObject(lPrivateData)
{
}

RVector::RVector(RVectorPrivate &lPrivateData, GSexp *sexp)
    :RObject(lPrivateData, sexp)
{
}

RVector::RVector(const RVector &other)
    :RObject(*other.mPrivate)
{

}

RVector &RVector::operator=(const RVector &other)
{
    Q_D(RObject);
    d->mErrors = other.mPrivate->mErrors;
    if (d->mGSexp != NULL)
    {
        delete d->mGSexp;
    }
    d->mGSexp = new GSexp;
    *d->mGSexp = *other.mPrivate->mGSexp;
    d->mProtectedObject= other.mPrivate->mProtectedObject;
    return *this;
}

RVector::~RVector()
{
}

bool RVector::Build(QString name, int size, Type type)
{
    if (!Allocate(size, type))
        return false;
    // assign this data in the global environment
    if (!Protect(name))
        return false;
    // init to NaN
    if (type == V_INT)
    {
        for( int i=0; i < size; i++)
            FillInt(i, R_NaInt);
    }
    else if (type == V_DOUBLE)
    {
        for( int i=0; i < size; i++)
            FillDouble(i, R_NaN);
    }
    else if (type == V_STD)
    {
        for( int i=0; i < size; i++)
            FillStd(i, QString());
    }
    else
        return false;


    return true;
}

bool RVector::Build(QString name, int size, double *data)
{
    if (data == 0)
        return false;
    if (!Allocate(size, V_DOUBLE))
        return false;
    // assign this data in the global environment
    if (!Protect(name))
        return false;
    // copy data
    for( int lIndex=0; lIndex < size; ++lIndex)
    {
        FillDouble(lIndex, data[lIndex]);
    }

    return true;
}

bool RVector::FillDouble(int index, double value)
{
    Q_D(RVector);
    if (!d->mDoubleVect)
        return false;
    d->mDoubleVect[index] = value;
    d->mType = V_DOUBLE;
    return true;
}

bool RVector::FillInt(int index, int value)
{
    Q_D(RVector);
    if (!d->mIntVect)
        return false;
    d->mIntVect[index] = value;
    d->mType = V_INT;
    return true;
}

bool RVector::FillStd(int index, const QString& value)
{
    Q_D(RVector);
    if (!d->mStdVect)
        return false;
    d->mStdVect[index] = Rf_mkString(value.toLatin1().data());
    d->mType = V_STD;
    return true;
}

bool RVector::FillStd(int index, const RVector& value)
{
    Q_D(RVector);
    if (!d->mStdVect)
        return false;
    d->mStdVect[index] = value.GetSexp()->mSexp;
    return true;
}

bool RVector::FillStd(int index, double value)
{
    Q_D(RVector);
    if (!d->mStdVect)
        return false;
    d->mStdVect[index] = Rf_ScalarReal(value);
    return true;
}

bool RVector::Allocate(int size, Type type)
{
    Q_D(RVector);
    SEXP lSEXVector = 0;
    if (type == V_DOUBLE)
    {
        lSEXVector = Rf_allocVector(REALSXP, size);
        d->mDoubleVect = REAL(lSEXVector);
        if (!d->mDoubleVect)
            return false;
    }
    else if (type == V_INT)
    {
        lSEXVector = Rf_allocVector(INTSXP, size);
        d->mIntVect = INTEGER(lSEXVector);
        if (!d->mIntVect)
            return false;
    }
    else if (type == V_STD)
    {
        lSEXVector = Rf_allocVector(VECSXP, size);
        d->mStdVect = STRING_PTR(lSEXVector);
        if (!d->mStdVect)
            return false;
    }
    else
        return false;

    d->mGSexp->mSexp = lSEXVector;

    return true;
}

int RVector::GetIntegerItem(QList<int> indexes, bool &ok)
{
    Q_D(RVector);
    ok = false;
    if (!d->mGSexp)
        return 0;
    GSexp* lSEXPInt = GetItem(d->mGSexp, indexes, ok);

    if (!ok || !lSEXPInt->mSexp || !Rf_isInteger(lSEXPInt->mSexp))
        return 0;

    ok = true;
    return INTEGER(lSEXPInt->mSexp)[0];
}

int RVector::GetIntegerItem(int index, bool &ok)
{
    QList<int> lIndexes;
    lIndexes.append(index);
    return GetIntegerItem(lIndexes, ok);
}

double RVector::GetDoubleItem(QList<int> indexes, bool &ok)
{
    Q_D(RVector);
    ok = false;
    if (!d->mGSexp)
        return 0;

    GSexp* lSEXPReal = GetItem(d->mGSexp, indexes, ok);
    if (!ok || !lSEXPReal->mSexp || !Rf_isNumeric(lSEXPReal->mSexp))
        return 0;

    ok = true;
    return REAL(lSEXPReal->mSexp)[0];
}

std::string RVector::GetStringItem(int index, bool &ok)
{
    QList<int> lIndexes;
    lIndexes.append(index);
    return GetStringItem(lIndexes, ok);
}

std::string RVector::GetStringItem(QList<int> indexes, bool &ok)
{
    Q_D(RVector);
    ok = false;
    if (!d->mGSexp)
        return 0;

    GSexp* lSEXPString = GetItem(d->mGSexp, indexes, ok);
    if (!ok || !lSEXPString->mSexp || !Rf_isString(lSEXPString->mSexp))
        return 0;

    ok = true;
    std::string lRes(CHAR(STRING_ELT(lSEXPString->mSexp, 0)));
    delete lSEXPString;
    return lRes;
}

double RVector::GetDoubleItem(int index, bool &ok)
{
    QList<int> lIndexes;
    lIndexes.append(index);
    return GetDoubleItem(lIndexes, ok);
}

GSexp* RVector::GetItem(QList<int> indexes, bool &ok)
{
    Q_D(RVector);
    ok = false;
    if (!d->mGSexp)
        return 0;

    return GetItem(d->mGSexp, indexes, ok);
}

GSexp* RVector::GetItem(int index, bool &ok)
{
    QList<int> lIndexes;
    lIndexes.append(index);
    return GetItem(lIndexes, ok);
}

int RVector::GetSize(QList<int> indexes)
{
    Q_D(RVector);
    if (!d->mGSexp)
        return -1;

    return GetSize(d->mGSexp, indexes);
}

bool RVector::IsIntVector()
{
    Q_D(RVector);
    if (!d->mGSexp->mSexp && !Rf_isVector(d->mGSexp->mSexp))
        return false;
    // check if is vector of int
    if (TYPEOF(d->mGSexp->mSexp) != INTSXP)
        return false;

    return true;
}

bool RVector::IsDoubleVector()
{
    Q_D(RVector);
    if (!d->mGSexp->mSexp && !Rf_isVector(d->mGSexp->mSexp))
        return false;
    // check if is vector of double
    if (TYPEOF(d->mGSexp->mSexp) != REALSXP)
        return false;

    return true;
}

bool RVector::IsStandardVector()
{
    Q_D(RVector);
    if (!d->mGSexp->mSexp && !Rf_isVector(d->mGSexp->mSexp))
        return false;
    // check if is vector of double
    if (TYPEOF(d->mGSexp->mSexp) != VECSXP)
        return false;

    return true;
}

bool RVector::IsEmpty()
{
    return (GetSize(QList<int>()) == 0);
}

int RVector::GetMemSize()
{
    Q_D(RVector);

    return GetMemSize(GetSize(), d->mType);
}

int RVector::GetMemSize(int size, RVector::Type type)
{
    int lSize = -1;
    if (type == V_INT)
    {
        lSize = (long) size * sizeof(int) / (1024 * 1024);
    }
    else if (type == V_DOUBLE)
    {
        lSize = (long) size * sizeof(double) / (1024 * 1024);
    }
    else if (type == V_STD)
    {
        lSize = (long) size * sizeof(SEXP) / (1024 * 1024);
    }

#ifdef QT_DEBUG
    printf("vector size %d mB\n", lSize);
#endif

    return lSize;
}

int RVector::GetSize()
{
    return GetSize(QList<int>());
}

GSexp* RVector::GetItem(GSexp* sexp, QList<int> indexes, bool &ok)
{
    ok = false;
    if (!sexp->mSexp ||
            !Rf_isVector(sexp->mSexp) ||
            indexes.isEmpty())
        return 0;

    QList<int> lIndexes = indexes;
    int index = lIndexes.takeFirst();

    if (lIndexes.isEmpty())
    {
        if ((index < 0) || (index >= Rf_xlength(sexp->mSexp)))
            return 0;
        int lVectorType = TYPEOF(sexp->mSexp);
        if (lVectorType == VECSXP)
        {
            ok = true;
            return new GSexp(VECTOR_ELT(sexp->mSexp, index));
        }
        else if (lVectorType == INTSXP)
        {
            // TODO: check is it clean?
            SEXP lSEXVector = Rf_allocVector(INTSXP, 1);
            int* lVect = INTEGER(lSEXVector);
            if (!lVect )
                return 0;
            lVect[0] = INTEGER(sexp->mSexp)[index];
            ok = true;
            return new GSexp(lSEXVector);
        }
        else if (lVectorType == REALSXP)
        {
            // TODO: check is it clean?
            SEXP lSEXVector = Rf_allocVector(REALSXP, 1);
            double* lVect = REAL(lSEXVector);
            if (!lVect )
                return 0;
            lVect[0] = REAL(sexp->mSexp)[index];
            ok = true;
            return new GSexp(lSEXVector);
        }
        else if (lVectorType == STRSXP)
        {
            // TODO: check is it clean?
            SEXP lSEXVector = Rf_allocVector(STRSXP, 1);
            SEXP* lVect = STRING_PTR(lSEXVector);
            if (!lVect )
                return 0;
            lVect[0] = STRING_PTR(sexp->mSexp)[index];
            ok = true;
            return new GSexp(lSEXVector);
        }
        else // not handled at the moment
        {
            printf("vector type %d not handled", lVectorType);
            return 0;
        }
    }

    return GetItem(new GSexp(VECTOR_ELT(sexp->mSexp, index)), lIndexes, ok);
}

int RVector::GetSize(GSexp* sexp, QList<int> indexes)
{
    if (!sexp->mSexp || !Rf_isVector(sexp->mSexp))
        return -1;

    if (indexes.isEmpty())
        return Rf_xlength(sexp->mSexp);

    QList<int> lIndexes = indexes;
    int index = lIndexes.takeFirst();
    if ((index < 0) || (index >= Rf_xlength(sexp->mSexp)))
        return -1;

    return GetSize(new GSexp(VECTOR_ELT(sexp->mSexp, index)), lIndexes);
}

} // namespace SE
} // namespace GS
