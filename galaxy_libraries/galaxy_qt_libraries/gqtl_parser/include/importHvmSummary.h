#ifndef IMPORTHVMSUMMARY_H
#define IMPORTHVMSUMMARY_H

#include "parserBase.h"
#include "stdf.h"
#include "queryable_by_bin_name_store.h"
#include "promis_interpreter_base.h"

#include <QMap>
#include <QScopedPointer>

namespace GS
{
namespace Parser
{

class ImportHvmSummary: public ParserBase
{
public:
    ImportHvmSummary(): ParserBase(typeHvmSummary, "HvmSummary"){ mHaveBinmapFile = false; mStartTime = mStopTime = 0; }

    bool    ConvertoStdf(const QString &aOriginalFileName, QString &aStdfFileName);

    static bool IsCompatible(const QString& aFileName);

private:

    bool ComputeBinning(int aBinNum, const QString& aBinName, int aBinCount);
    ParserBinning * MakeBinning(int aBinNum, const QString &aBinName, int aBinCount, bool aPass);
    void CreateOrUpdateBinning(QMap<QString, ParserBinning *>& aBinnings, const QString& aBinKey, int aBinNum,
                               const QString& aBinName, int aBinCount, bool aPass);
    bool ReadHvmSummaryFile(const QString& szFileName);
    bool ReadPromisHvmDataFile();
    bool ReadHvmBinmapFile();
    bool WriteStdfFile(const QString& strFileNameSTDF);

    QString     mDataFilePath;

    bool    mHaveBinmapFile;          // true if have binmap file present

    long    mStartTime;               //Beginning Time:
    long    mStopTime;                //Ending Time:
    QString mLotID;                 //Lot,944
    QString mSubLotID;              //PromisLotId
    QString mDateCod;
    QString mPkgTyp;
    QString mNodeNam;
    QString mPartTyp;
    QString mFacilId;
    QString mProcId;
    QString mTestCod;
    QString mExtraId;

    QMap<QString, ParserBinning *> mSoftBinning;    /// \param List of soft Bin tables.
    QMap<QString, ParserBinning *> mHardBinning;    /// \param List of hard Bin tables.
    QScopedPointer< Qx::BinMapping::QueryableByBinName > mBinMapStore;
    QScopedPointer< Qx::BinMapping::PromisInterpreterBase > mPromisInterpreter;
};

}//namespace GS
}//namespace Parser



#endif // IMPORTHVMSUMMARY_H
