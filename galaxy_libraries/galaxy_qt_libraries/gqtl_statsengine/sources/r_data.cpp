
#include "r_data.h"
#include "r_object.h"
#include "r_matrix.h"
#include "r_vector.h"
#include "Rinternals.h"
#include "Rembedded.h"
#include "stats_data_private.h"

namespace GS
{
namespace SE
{

class RDataPrivate:public StatsDataPrivate
{
public:
    RDataPrivate()
        :StatsDataPrivate(){}

    QMap<StatsData::DataUse, RObject*> mMemMap; ///< holds pointer of allocated object to R mem

private:
    RDataPrivate& operator=(const RDataPrivate&);
    RDataPrivate(const RDataPrivate&);

};


RData::RData()
    : StatsData(* new RDataPrivate)
{

}

RData::RData(RDataPrivate &lPrivateData)
    :StatsData(lPrivateData)
{

}

RData::~RData()
{
    CleanAllAllocatedData();
}

bool RData::Insert(RObject *object, StatsData::DataUse use)
{
    Q_D(RData);

    if (d->mMemMap.contains(use))
    {
        RObject* lRObject = d->mMemMap.take(use);
        if (lRObject && !lRObject->IsNull())
        {
            delete lRObject;
            lRObject = 0;
        }
    }

    d->mMemMap.insert(use, object);

    return true;
}

RObject *RData::GetRObject(StatsData::DataUse use)
{
    Q_D(RData);
    if (!d->mMemMap.contains(use))
    {
        d->mErrors.append(QString("Data id %1 not stored").arg(use));
        return 0;
    }

    return d->mMemMap.value(use);
}

RVector *RData::AllocateVector(StatsData::DataUse use, int size, int type)
{
    QString lMemName;
    if (!AutoBuildVarName(use, lMemName))
        return 0;

    return AllocateVector(use, lMemName, size, type);
}

RVector *RData::AllocateVector(StatsData::DataUse use, QString name, int size, int type)
{
    Q_D(RData);

    RVector::Type lType = static_cast<RVector::Type>(type);
    // check is enough free mem
//    QMap<QString, QVariant> lMemInfo = CGexSystemUtils::GetMemoryInfo(false, true);
//    int lFreeRam = lMemInfo.value("freeram").toInt(); // supposed to be in kB
//    if (lFreeRam > 0)
//    {
//        // ensure that it doesn't use more than 50% of the free mem
//        if (RVector::GetMemSize(size, lType) > (lFreeRam / 2))
//        {
//            d->mErrors.append(QString("Not enough free memory to allocate vector: %1").arg(name));
//            return 0;
//        }
//    }

    RVector* lVector = new RVector();
    if (!lVector->Build(name, size, lType))
    {
        d->mErrors.append(QString("Unable to build vector: %1").arg(lVector->GetLastError()));
        delete lVector;
        lVector = 0;
        return 0;
    }

    // usefull ??
    if( d->mMemMap.contains(use))
        delete d->mMemMap[use];

    d->mMemMap.insert(use, lVector);

    return lVector;
}

RVector *RData::AllocateVector(StatsData::DataUse use, int size, double *vector)
{
    QString lMemName;
    if (!AutoBuildVarName(use, lMemName))
        return 0;

    return AllocateVector(use, lMemName, size, vector);
}

RVector *RData::AllocateVector(StatsData::DataUse use, QString name, int size, double *vector)
{
    Q_D(RData);

    RVector* lVector = new RVector();
    if (!lVector->Build(name, size, vector))
    {
        d->mErrors.append(QString("Unable to build vector: %1").arg(lVector->GetLastError()));
        delete lVector;
        lVector = 0;
        return 0;
    }

    d->mMemMap.insert(use, lVector);

    return lVector;
}

RMatrix* RData::AllocateMatrix(StatsData::DataUse use, int rows, int cols)
{
    QString lMemName;
    if (!AutoBuildVarName(use, lMemName))
        return 0;

    return AllocateMatrix(use, lMemName, rows, cols);
}

RMatrix* RData::AllocateMatrix(StatsData::DataUse use, QString name, int rows, int cols)
{
    Q_D(RData);

    // check is enough free mem
//    QMap<QString, QVariant> lMemInfo = CGexSystemUtils::GetMemoryInfo(false, true);
//    int lFreeMem = lMemInfo.value("freeram").toInt(); // supposed to be in mB
//    if (lFreeMem > 0)
//    {
//        // ensure that it doesn't use more than 50% of the free mem
//        if (RMatrix::GetMemSize(rows, cols, RMatrix::M_DOUBLE) > (lFreeMem / 2))
//        {
//            d->mErrors.append(QString("Not enough free memory to allocate vector: %1").arg(name));
//            return 0;
//        }
//    }

    RMatrix* lMatrix = new RMatrix();
    if (!lMatrix->Build(name, rows, cols))
    {
        d->mErrors.append(QString("Unable to build matrix: %1").arg(lMatrix->GetLastError()));
        delete lMatrix;
        lMatrix = 0;
        return 0;
    }

    d->mMemMap.insert(use, lMatrix);

    return lMatrix;
}

RMatrix* RData::AllocateMatrix(StatsData::DataUse use, QString name, int rows, int cols, double** matrix)
{
    Q_D(RData);

    // check is enough free mem
//    QMap<QString, QVariant> lMemInfo = CGexSystemUtils::GetMemoryInfo(false, true);
//    int lFreeMem = lMemInfo.value("freeram").toInt(); // supposed to be in mB
//    if (lFreeMem > 0)
//    {
//        // ensure that it doesn't use more than 50% of the free mem
//        if (RMatrix::GetMemSize(rows, cols, RMatrix::M_DOUBLE) > (lFreeMem / 2))
//        {
//            d->mErrors.append(QString("Not enough free memory to allocate vector: %1").arg(name));
//            return 0;
//        }
//    }

    RMatrix* lMatrix = new RMatrix();
    if (!lMatrix->Build(name, rows, cols, matrix))
    {
        d->mErrors.append(QString("Unable to build matrix: %1").arg(lMatrix->GetLastError()));
        delete lMatrix;
        lMatrix = 0;
    }
    else
        d->mMemMap.insert(use, lMatrix);

    return lMatrix;
}

bool RData::CleanAllocatedData(RObject *object)
{
    Q_D(RData);
    SEXP res = Rf_lang2(
                    Rf_install("try"),                             // avoid crash
                    Rf_lang2(
                        Rf_install("rm"),
                        Rf_lang1( Rf_install(object->GetName().toLatin1().data()))));
    if (!res)
    {
        d->mErrors.append(QString("Unable to clean var %1").arg(object->GetName()));
        return false;
    }

# ifdef QT_DEBUG
    printf("Succesfully cleaned object:      %s \n", object->GetName().toLatin1().data());
#endif

    return true;
}

bool RData::CleanAllAllocatedData()
{
    Q_D(RData);
//    printf("Clean R memory\n");
//    printf("Status:\n");
//    Rf_eval( Rf_lang2( Rf_install("print"), Rf_lang1(Rf_install("ls"))) , R_GlobalEnv );

    bool lSuccess = true;

    // Clean loaded allocated data
    while(!d->mMemMap.isEmpty() && lSuccess)
    {
        RObject* lRObject = d->mMemMap.take(d->mMemMap.begin().key());
        if (lRObject && !lRObject->IsNull())
        {
            delete lRObject;
            lRObject = 0;
        }
        else
        {
            d->mErrors.append("empty struct to delete");
            lSuccess = false;
        }
    }
//    printf("Status:\n");
//    Rf_eval( Rf_lang2( Rf_install("print"), Rf_lang1(Rf_install("ls"))) , R_GlobalEnv );
//    printf("Clean R memory finished\n");

    return lSuccess;
}

bool RData::AutoBuildVarName(StatsData::DataUse use, QString &name)
{
    Q_D(RData);
    if (use == MVGROUP_IN_1)
        name = "mvgroup_in_1";
    else if (use == MVOUTLIER_IN_1)
        name = "mvoutlier_in_1";
    else if (use == MVOUTLIER_IN_2)
        name = "mvoutlier_in_2";
    else if (use == SHAPEIDENTIFIER_IN)
        name = "shape_identifier_in";
    else if (use == SHAPEIDENTIFIER_OUT)
        name = "shape_identifier_out";
    else
    {
        d->mErrors.append("Unknown purpose of allocated data");
        return false;
    }

    return true;
}


bool RData::IsAllocated(StatsData::DataUse use) const
{
    // check via
    return d_func()->mMemMap.contains(use);
}

RMatrix* RData::AllocateMatrix(StatsData::DataUse use, int rows, int cols, double **matrix)
{
    QString lMemName;
    if (!AutoBuildVarName(use, lMemName))
        return 0;

    return AllocateMatrix(use, lMemName, rows, cols, matrix);
}

} // namespace SE
} // namespace GS

