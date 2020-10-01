#ifndef GEX_IMPORT_SPEKTRA_DATALOG_H
#define GEX_IMPORT_SPEKTRA_DATALOG_H

#include "parserBase.h"
#include "queryable_by_test_number_store.h"
#include "promis_interpreter_base.h"

#include <QVector>

namespace GS
{
namespace Parser
{
class ParserParameter;

class CGSpektraDatalogtoSTDF : public ParserBase
{
public:
    CGSpektraDatalogtoSTDF();
    ~CGSpektraDatalogtoSTDF();
    bool    ConvertoStdf(const QString& aSpektraDatalogFileName, QString& aFileNameSTDF);
    static bool IsCompatible(const QString &szFileName);

private:

    void NormalizeValue(QString &aValue, QString &aUnit, int &aScaleFactor);

    bool ProcessSpektraDatalogFile(
            const QString &aSpektraDatalogFileName,
            const QString &aPromisFile,
            const QString &aBinMapFile,
            const QString &aFileNameSTDF);

    bool ReadPromisSpektraDataFile(const QString& aFilePath);

    bool ReadSpektraBinmapFile(const QString& aFilePath);

    bool GetInputFiles(const QString& aSpektraDatalogFileName, QString& aPromisFilePath, QString& aBinMapFilePath);

    bool WriteStdfFile(QTextStream &aSpektraDatalogFile, const QString &aFileNameSTDF);

    void WriteFARRecord();
    void WriteMIRRecord();
    void WriteSDRRecord();
    void WriteWIRRecord();
    void WriteDTRRecord();
    void WritePIRRecord();
    void WritePTRRecord(
            ParserParameter &aParam,
            bool aIsTestPass,
            bool aIsStopOnFail,
            float aValue);
    void WriteFTRRecord(const ParserParameter &aParam, bool aIsTestPass, const QString &aValue);
    void WritePRRRecord(
            bool aIsPassStatus,
            int aTotalTests,
            int aBinNumber,
            int aXWafer,
            int aYWafer,
            int aPartNumber,
            int &aTotalGoodBin,
            int &aGoodParts,
            int &aTotalFailBin);
    void WriteWRRRecord(int aTotalGoodBin, int aTotalFailbin);
    void WriteWCRRecord();
    void WriteTSRRecord(const ParserParameter &aParam);
    void WritePCRRecord();
    void WriteMRRRecord();
    void WriteHBRRecord(const ParserBinning &aBin);
    void WriteSBRRecord(const ParserBinning &aBin);

    int         mDataOffset;
    int         mGoodParts;
    int         mFailParts;
    int         mTotalParts;

    QString     mLotId;                 // LotID
    QString     mSubLotId;              //PromisLotId
    int         mWaferId;                 // WaferID
    int         mFlatNotch;
    int         mDiesUnit;
    float       mDiesXSize;
    float       mDiesYSize;
    QString     mProductId;             // ProductID
    QString     mTesterId;              // TesterID
    QString     mTesterType;
    QString     mProgramId;             // ProgramID
    QString     mOperatorId;            // OperatorID
    QString     mStationId;             // StationID
    QString     mJobName;
    QString     mTesterName;
    QString     mPartType;
    QString     mSuprName;
    QString     mHandlerId;
    QString     mExtraId;
    QString     mGrossDie;
    QString     mFacilId;
    QString     mProcId;
    QString     mTestCod;
    long        mStartTime;               // Startup time
    int         mTotalParameters;
    std::vector<ParserParameter>  mParameterList;        // List of Parameters in Wafer

    GQTL_STDF::Stdf_FAR_V4 mFARrecord;
    GQTL_STDF::Stdf_MIR_V4 mMIRRecord;
    GQTL_STDF::Stdf_SDR_V4 mSDRRecord;
    GQTL_STDF::Stdf_WIR_V4 mWIRRecord;
    GQTL_STDF::Stdf_DTR_V4 mDTRRecord;
    GQTL_STDF::Stdf_PIR_V4 mPIRRecord;
    GQTL_STDF::Stdf_PTR_V4 mPTRRecord;
    GQTL_STDF::Stdf_FTR_V4 mFTRRecord;
    GQTL_STDF::Stdf_PRR_V4 mPRRRecord;
    GQTL_STDF::Stdf_WRR_V4 mWRRRecord;
    GQTL_STDF::Stdf_WCR_V4 mWCRRecord;
    GQTL_STDF::Stdf_TSR_V4 mTSRRecord;
    GQTL_STDF::Stdf_HBR_V4 mHBRRecord;
    GQTL_STDF::Stdf_SBR_V4 mSBRRecord;
    GQTL_STDF::Stdf_PCR_V4 mPCRRecord;
    GQTL_STDF::Stdf_MRR_V4 mMRRRecord;

    QScopedPointer< Qx::BinMapping::QueryableByTestNumber > mBinMapStore;
    QScopedPointer< Qx::BinMapping::PromisInterpreterBase > mPromisInterpreter;
    QMap<int, ParserBinning> mSpektraDatalogBinning;

    bool IsInTheBinMapStore(int aNumber);
};

} // namespace Parser
} // namespace GS

#endif
