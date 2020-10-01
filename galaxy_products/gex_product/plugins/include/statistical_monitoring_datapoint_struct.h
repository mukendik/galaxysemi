#ifndef SM_DATAPOINT_STRUCT_H
#define SM_DATAPOINT_STRUCT_H

#include <QString>
#include <QHash>

struct StatMonDataPoint{
    QString mProduct;
    QString mLot;
    QString mSublot;
    QString mWafer;
    int mSplitlot;
    unsigned int mNumExecutions;
    unsigned int mNumFails;
    unsigned int mNumGood;
    double mValue;

    StatMonDataPoint(QString aProduct,
                     QString aLot,
                     QString aSublot,
                     QString aWafer,
                     int aSplitlot,
                     unsigned int aNumExecutions,
                     unsigned int aNumFails,
                     unsigned int aNumGood,
                     double aValue)
        : mProduct(aProduct),
          mLot(aLot),
          mSublot(aSublot),
          mWafer(aWafer),
          mSplitlot(aSplitlot),
          mNumExecutions(aNumExecutions),
          mNumFails(aNumFails),
          mNumGood(aNumGood),
          mValue(aValue)
    {
    }
};

#endif // SM_DATAPOINT_STRUCT_H
