#ifndef SM_LIMIT_STRUCT_H
#define SM_LIMIT_STRUCT_H

#include <QString>

struct StatMonLimit{
    int mLimitId;
    int mSite;
    QString mStatName;
    double mLowl;
    bool mLowlIsEnabled;
    double mHighl;
    bool mHighlIsEnabled;
    int mCriticity;

    StatMonLimit(){}
    StatMonLimit(int aLimitId,
                int aSite,
                QString aStatName,
                double aLowl,
                bool aLowlIsEnabled,
                double aHighl,
                bool aHighlIsEnabled,
                int aCriticity)
        : mLimitId(aLimitId),
        mSite(aSite),
        mStatName(aStatName),
        mLowl(aLowl),
        mLowlIsEnabled(aLowlIsEnabled),
        mHighl(aHighl),
        mHighlIsEnabled(aHighlIsEnabled),
        mCriticity(aCriticity)
    {

    }
};

#endif // SM_DATAPOINT_STRUCT_H
