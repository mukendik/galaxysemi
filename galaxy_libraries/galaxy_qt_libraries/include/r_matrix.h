#ifndef R_MATRIX_H
#define R_MATRIX_H

/*! \class RMatrix
 * \brief
 *
 */

#include "r_object.h"

namespace GS
{
namespace SE
{

class RMatrixPrivate;

class RMatrix: public RObject
{
public:
    enum Type
    {
        M_INT = 0,
        M_STD = 1,
        M_DOUBLE = 2
    };
    /// \brief Constructor
    RMatrix();
    /// \brief Constructor
    RMatrix(GSexp* sexp);
    /// \brief Destructor
    ~RMatrix();
    /// \brief allocate and install empty matrix in R mem
    bool Build(QString name, int rows, int cols);
    /// \brief allocate, install and fill matrix in R mem
    bool Build(QString name, int rows, int cols, double** data);
    /// \brief fill data into matrix
    bool Fill(int row, int col, double value);
    /// \brief return selected item if exists
    double GetItem(int row, int col, bool &ok);
    /// \brief return rows number
    int GetRows();
    /// \brief return cols number
    int GetCols();
    /// \brief return size in mB according to its attributes
    int GetMemSize();
    /// \brief return size in mB according to dims and only double matrix are supported at the moment
    static int GetMemSize(int rows, int cols, Type type);

private:
    Q_DECLARE_PRIVATE_D(mPrivate, RMatrix)
    /// \brief Constructor
    RMatrix(RMatrixPrivate & lPrivateData);
    /// \brief Constructor
    RMatrix(RMatrixPrivate & lPrivateData, GSexp* sexp);

    /// \brief allocate mem for matrix in R mem
    bool Allocate(int rows, int cols);
};

} // namespace SE
} // namespace GS


#endif // R_MATRIX_H
