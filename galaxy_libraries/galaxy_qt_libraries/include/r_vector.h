#ifndef R_VECTOR_H
#define R_VECTOR_H

/*! \class RVector
 * \brief
 *
 */

#include <QList>
#include "r_object.h"

namespace GS
{
namespace SE
{

class GSexp;
class RVectorPrivate;

class RVector: public RObject
{
public:
    enum Type
    {
        V_INT = 0,
        V_STD = 1,
        V_DOUBLE = 2
    };
    /// \brief Constructor
    RVector();
    /// \brief Constructor
    RVector(GSexp* sexp);

    /// \brief Copy constructor
    RVector(const RVector& other);
    /// \brief Assignment operator
    RVector &operator=(const RVector &other);
    /// \brief Destructor
    ~RVector();
    /// \brief allocate and install empty vector in R mem
    bool Build(QString name, int size, Type type);
    /// \brief allocate and install empty vector in R mem
    bool Build(QString name, int size, double* data);
    /// \brief fill data into vector
    bool FillDouble(int index, double value);
    /// \brief fill data into vector
    bool FillInt(int index, int value);
    /// \brief fill data into vector
    bool FillStd(int index, const QString& value);
    /// \brief fill data into vector
    bool FillStd(int index, const RVector& value );
    /// \brief fill data into vector
    bool FillStd(int index, double value );
    /// \brief return integer item at selected index
    int GetIntegerItem(int index, bool& ok);
    /// \brief return integer in vector at index in vector at index...
    int GetIntegerItem(QList<int> indexes, bool& ok);
    /// \brief return double item at selected index
    double GetDoubleItem(int index, bool& ok);
    /// \brief return double in vector at index in vector at index...
    double GetDoubleItem(QList<int> indexes, bool& ok);
    /// \brief return string item at selected index
    std::string GetStringItem(int index, bool& ok);
    /// \brief return string in vector at index in vector at index...
    std::string GetStringItem(QList<int> indexes, bool& ok);
    /// \brief return item at selected index
    GSexp *GetItem(int index, bool& ok);
    /// \brief return item in vector at index in vector at index...
    GSexp *GetItem(QList<int> indexes, bool& ok);
    /// \brief return size of vector
    int GetSize();
    /// \brief return size of vector at index in vector at index...
    int GetSize(QList<int> indexes);
    /// \brief true if vector is integer vector
    bool IsIntVector();
    /// \brief true if vector is double vector
    bool IsDoubleVector();
    /// \brief true if vector is STR vector
    bool IsStandardVector();
    /// \brief true if vector is empty
    bool IsEmpty();
    /// \brief return size in mB according to its attributes
    int GetMemSize();
    /// \brief return size in mB according to size and type
    static int GetMemSize(int size, Type type);

private:
    Q_DECLARE_PRIVATE_D(mPrivate, RVector)
    /// \brief Constructor
    RVector(RVectorPrivate & lPrivateData);
    /// \brief Constructor
    RVector(RVectorPrivate & lPrivateData, GSexp* sexp);

    /// \brief allocate mem for matrix in R mem
    bool Allocate(int size, Type type);
    /// \brief return item in vector at index in vector at index...
    GSexp* GetItem(GSexp* sexp, QList<int> indexes, bool& ok);
    /// \brief return size of vector at index in vector at index...
    int GetSize(GSexp* sexp, QList<int> indexes);
};

} // namespace SE
} // namespace GS

#endif // R_VECTOR_H
