#ifndef IMPORT_FET_TEST_SUMMARY_H
#define IMPORT_FET_TEST_SUMMARY_H

#include "parserBase.h"
#include "queryable_by_test_number_store.h"
#include "queryable_by_test_name_store.h"
#include "promis_interpreter_base.h"
#include <QScopedPointer>

namespace GS
{
namespace Parser
{

struct CGFetTestSummaryBinning
{
    int         HardBinNum;    // Hard Bin number
    int         SoftBinNum;    // Soft Bin number
    QString     Name;          // Bin name
    bool        Pass;          // Bin cat
    int         Count;         // Bin Summary count
};

class FetTestSummaryToStdf : public ParserBase
{
public :
    static bool IsCompatible( const QString &aFilePath );
    FetTestSummaryToStdf();

private :
    QString GetRealExternalFilePath( const QString &aFileName );
    bool ConvertoStdf(const QString &aInputFilePath, QString &aOutputFilePath);
    bool ReadBinmapFiles();
    bool ReadFetTestSummaryFile(const QString &aFilePath);
    bool ReadPromisDataFile();
    bool WriteStdfFile(const QString &aStdfFilePath);
    bool GetBinMappingFileName(const QString &aCategory, QString &lBinMapFileName);
    void ProcessBinMappingItemByTestNumber(const Qx::BinMapping::BinMapItemBase &aBinMapItem, int aBinCount);
    void ProcessBinMappingItemByTestName(const Qx::BinMapping::BinMapItemBase &aBinMapItem, int aBinCount);
    bool ExtractTestNumber(int &aOutTestNumber, QString &aStringToExtract);
    void WriteSBR(CGFetTestSummaryBinning &lItem, StdLib::Stdf &aStdfFile);
    void WriteHBR(CGFetTestSummaryBinning &lItem, StdLib::Stdf &aStdfFile);

    QString mDataFilePath;
    QMap<std::string, CGFetTestSummaryBinning> mFetTestSummaryBinPerTestName;
    bool mHaveBinmapFile;
    QString mPromisLotId;
    QString mLotId;
    QString mSubLotId;
    QString mNodeNam;
    QString mHandId;
    QString mCardId;
    QString mDibId;
    QString mLoadId;
    QString mCableId;
    int mTotalGoodParts;
    int mTotalParts;
    gstime mStopTime;
    QMap<int, CGFetTestSummaryBinning> mFetTestSummaryBinPerTestNumber;
    GQTL_STDF::Stdf_SBR_V4 mSBRRecord;
    GQTL_STDF::Stdf_HBR_V4 mHBRRecord;
    QString mFacilityId;
    QString mSiteLocation;
    QString mTesterType;
    QString mDateCode;
    QString mPackage;
    QString mProcId;
    QString mPackageType;
    QString mPromisLotId_D2;
    QString mGeometryName_D2;
    QString mPromisLotId_D3;
    QString mGeometryName_D3;
    QString mPromisLotId_D4;
    QString mGeometryName_D4;
    QString mProductId;
    QString mOperator;
    QString mProber;

    QScopedPointer<Qx::BinMapping::QueryableByTestNumber>   mBinMapFinalTestStore;
    QScopedPointer<Qx::BinMapping::QueryableByTestName>     mBinMapSortEntriesStore;
    QScopedPointer<Qx::BinMapping::PromisInterpreterBase>   mPromisInterpreter;

    bool ExtractTestNumberList(QStringList &aOutTestNumbers, QString &aStringToExtract);
};

}
}

#endif // IMPORT_FET_TEST_SUMMARY_H
