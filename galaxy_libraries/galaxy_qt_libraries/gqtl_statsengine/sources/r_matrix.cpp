
#include "r_matrix.h"
#include "r_object.h"
#include "r_object_private.h"
#include "r_protected_object.h"
#include "g_sexp.h"

namespace GS
{
namespace SE
{

class RMatrixPrivate: public RObjectPrivate
{
public:
    RMatrixPrivate()
        :RObjectPrivate(), mCMatrix(0){}

    int mRows;          ///< holds number of rows
    int mCols;          ///< holds number of cols
    double* mCMatrix;   ///< holds pointer to matrix in R mem
    QStringList mErrors;///< holds errors
};

RMatrix::RMatrix()
    :RObject(* new RMatrixPrivate)
{
}

RMatrix::RMatrix(GSexp *sexp)
    :RObject(* new RMatrixPrivate, sexp)
{
    Q_D(RMatrix);
    if (!Rf_isMatrix(sexp->mSexp))
        return;
    double* lMat= REAL(sexp->mSexp);
    if (!lMat)
        return;

    d->mCMatrix = lMat;
    d->mCols = Rf_ncols(sexp->mSexp);
    d->mRows = Rf_nrows(sexp->mSexp);
}

RMatrix::RMatrix(RMatrixPrivate &lPrivateData)
    :RObject(lPrivateData)
{

}

RMatrix::RMatrix(RMatrixPrivate &lPrivateData, GSexp *sexp)
    :RObject(lPrivateData, sexp)
{

}

RMatrix::~RMatrix()
{

}

bool RMatrix::Build(QString name, int rows, int cols)
{
    Q_D(RMatrix);
    if (!Allocate(rows, cols))
        return false;
    // assign this data in the global environment
    if (!Protect(name))
        return false;
    // init to 0
    for( int i=0; i <d->mRows; i++)
        for(int j = 0; j < d->mCols; j++)
            Fill(i, j, R_NaN);

    return true;
}

bool RMatrix::Build(QString name, int rows, int cols, double **data)
{
    Q_D(RMatrix);
    if (data == 0)
        return false;
    if (!Allocate(rows, cols))
        return false;
    // assign this data in the global environment
    if (!Protect(name))
        return false;

    // copy data
    for( int lRow=0; lRow < d->mRows; ++lRow)
    {
        if (data[lRow] == 0)
            return false;
        for(int lCol = 0; lCol < d->mCols; ++lCol)
            Fill(lRow, lCol, data[lRow][lCol]);
    }

    return true;
}

bool RMatrix::Allocate(int rows, int cols)
{
    Q_D(RMatrix);

//    SEXP lSEXPMatrix = Rf_allocMatrix(REALSXP, rows, cols);

    SEXP lCall = PROTECT(Rf_lang4(
                             Rf_install("matrix"),
                                 Rf_ScalarReal(0),
                                 Rf_ScalarInteger(rows),
                                 Rf_ScalarInteger(cols)
                                 ));

    UNPROTECT(1);

    if (!lCall)
    {
        d->mErrors.append(QString("Error when creating data matrix"));
        return false;
    }
    int lErrorOccurred;
    SEXP lSEXPMatrix = PROTECT(R_tryEvalSilent(lCall, R_GlobalEnv, &lErrorOccurred));
    UNPROTECT(1);

    if (lErrorOccurred)
    {
        d->mErrors.append(QString("Unable to Allocate matrix: %1").arg(QString(R_curErrorBuf())));
        return false;
    }

    double* lMat= REAL(lSEXPMatrix);
    if (!lMat)
    {
        d->mErrors.append(QString("Error while retrieving created data matrix"));
        return false;
    }
    d->mGSexp->mSexp = lSEXPMatrix;
    d->mCMatrix = lMat;
    d->mCols = cols;
    d->mRows = rows;

    return true;
}

bool RMatrix::Fill(int row, int col, double value)
{
    Q_D(RMatrix);
    if (!d->mCMatrix)
        return false;
    d->mCMatrix[row + d->mRows*col] = value;
    return true;
}

double RMatrix::GetItem(int row, int col, bool &ok)
{
    Q_D(RMatrix);
    ok = false;

    if (row >= d->mRows || col >= d->mCols)
        return 0;

    if (!d->mCMatrix)
        return 0.0;

    ok = true;
    return d->mCMatrix[row + d->mRows*col];
}

int RMatrix::GetRows()
{
    Q_D(RMatrix);
    return d->mRows;
}

int RMatrix::GetCols()
{
    Q_D(RMatrix);
    return d->mCols;
}

int RMatrix::GetMemSize()
{
    Q_D(RMatrix);

    return GetMemSize(d->mRows, d->mCols, RMatrix::M_DOUBLE);
}

int RMatrix::GetMemSize(int rows, int cols, Type type)
{
    int lSize = -1;

    // only double is handled at the moment !
    if (type == RMatrix::M_DOUBLE)
        lSize = (long) rows * cols * sizeof(double) / (1024 * 1024);

//#ifdef QT_DEBUG
//    printf("matrix size %dX%d: %d mB\n", rows, cols, lSize);
//    fflush(stdout);
//#endif

    return lSize;
}

} // namespace SE
} // namespace GS

