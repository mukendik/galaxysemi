#ifndef R_DATA_H
#define R_DATA_H

/*! \class RData
 * \brief
 *
 */

#include <QString>
#include <QMap>

#include "stats_data.h"

namespace GS
{
namespace SE
{
class RObject;
class RMatrix;
class RVector;
class RDataPrivate;

class RData: public StatsData
{
public:
    /// \brief Constructor
    RData();
    /// \brief Destructor
    ~RData();
    /// \brief Store
    bool Insert(RObject* object, StatsData::DataUse use);
    /// \brief return pointer to selected object if exists
    RObject* GetRObject(StatsData::DataUse use);
    /// VECTOR
    /// \brief allocate empty vector in R mem (name is automatically populated)
    RVector* AllocateVector(DataUse use, int size, int type);
    /// \brief allocate empty named matrix in R mem
    RVector* AllocateVector(DataUse use, QString name, int size, int type);
    /// \brief allocate vector in R mem and fill it with vector (name is automatically populated)
    RVector* AllocateVector(DataUse use, int size, double* vector);
    /// \brief allocate vector in R mem and fill it with vector
    RVector* AllocateVector(DataUse use, QString name, int size, double* vector);

    /// MATRIX
    /// \brief allocate empty matrix in R mem (name is automatically populated)
    RMatrix* AllocateMatrix(DataUse use, int rows, int cols);
    /// \brief allocate empty named matrix in R mem
    RMatrix*  AllocateMatrix(DataUse use, QString name, int rows, int cols);
    /// \brief allocate matrix in R mem and fill it with matrix (name is automatically populated)
    RMatrix* AllocateMatrix(DataUse use, int rows, int cols, double** matrix);
    /// \brief allocate named matrix in R mem and fill it with matrix
    RMatrix *AllocateMatrix(DataUse use, QString name, int rows, int cols, double** matrix);
    /// \brief clean R memory
    bool CleanAllAllocatedData();
    /// \brief remove object from R mem
    bool CleanAllocatedData(RObject* object);

private:
    Q_DECLARE_PRIVATE_D(mPrivate, RData)
    /// \brief Constructor
    RData(RDataPrivate & lPrivateData);
    /// \brief auto compute object name
    bool AutoBuildVarName(DataUse use, QString &name);
    /// \brief check if data is allocated
    bool IsAllocated(StatsData::DataUse use) const;
};

} // namespace SE
} // namespace GS

#endif // R_DATA_H

