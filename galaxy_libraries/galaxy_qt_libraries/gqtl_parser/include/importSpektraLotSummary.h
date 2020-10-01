#ifndef IMPORTSPEKTRALOTSUMMARY_H
#define IMPORTSPEKTRALOTSUMMARY_H

#include "gqtl_global.h"
#include "parserBase.h"
#include "queryable_by_test_number_store.h"
#include "promis_interpreter_base.h"

class CGSpektraLotSummaryBinning
{
public:
    int     mNumber;    // Bin number
    QString mName;      // Bin name
    bool    mPass;      // Bin cat
    int     mCount;     // Bin count

};


namespace GS
{
namespace Parser
{


class SpektraLotSummary : public ParserBase
{
public:

    /**
    * \fn SpektraLotSummary()
    * \brief constructor
    */
    SpektraLotSummary();

    /**
    * \fn ~SpektraLotSummary()
    * \brief destructor
    */
    ~SpektraLotSummary();

    enum  ErrCodesSpektraLotSummary
    {
        errMultiLot  = errParserSpecific + 1,
        errMissingData
    };

    /**
     * \fn bool ConvertoStdf(const QString &aInputFile, QString &aStdfFileName)
     * \brief Read the SpektraLotSummary ASCII file.
     * \param aInputFile is the input file, contains the Spektra Lot Summary file.
     * \param aStdfFileName is the output file, contains the stdf file.
     * \return true if the file has been successfully read. Otherwise return false.
     */
    bool ConvertoStdf(const QString &aInputFile, QString &aStdfFileName);

    /**
     * \fn std::string GetErrorMessage(const int ErrorCode)
     * \brief Getter
     * \return The error message corresponding to the given error code
     */
    std::string GetErrorMessage(const int ErrorCode) const;

    /**
    * @brief inform if the file is compatible with the format handle by the parser
    * @param the file
    */
    static bool	IsCompatible(const QString &FileName);

private:

    bool ReadSpektraLotSummaryFile(QTextStream& aInputTextStream);
    bool ReadPromisSpektraDataFile();
    bool ReadSpektraBinmapFile();

    /**
     * \fn bool WriteStdfFile(QTextStream &aInputTextStream, const QString &aStdfFileName)
     * \brief Write the stdf file.
     * \param aInputTextStream is the input file, contains the Spektra Lot Summary file.
     * \param aStdfFileName is the output file, contains the stdf file.
     * \return true if the stdf file has been successfully written. Otherwise return false.
     */
    bool WriteStdfFile(QTextStream&aInputTextStream, const QString &aStdfFileName);

    QScopedPointer< Qx::BinMapping::QueryableByTestNumber > mBinMapStore;
    QScopedPointer< Qx::BinMapping::PromisInterpreterBase > mPromisInterpreter;
    QMap<int, CGSpektraLotSummaryBinning>                   mMapSpektraSummaryBinning;   // List of Bin tables coming from the input file.
    QMap<int,int>                                           mMapIndexToBinNumber;

    QString     mDataFilePath;

    QString     mJobName;
    QString     mTesterName;
    QString     mPartType;
    QString     mHandlerId;
    QString     mExtraId;
    QString     mGrossDie;
    QString     mFacilId;
    QString     mProcId;
    QString     mTestCod;
    QString     mLotId;                     // LotID
    QString     mSubLotId;                  //PromisLotId
    QString     mWaferId;
    QString     mOperatorId;                // OperatorID
    QString     mComputerId;                // ComputerID
    QString     mTesterType;                // Test Type
    QString     mExecType;                  // OutPut Mode

    int         mFlatNotch;
    int         mDiesUnit;
    float       mDiesXSize;
    float       mDiesYSize;

     GQTL_STDF::Stdf_SBR_V4 mSBRRecord;
     GQTL_STDF::Stdf_HBR_V4 mHBRRecord;


     bool IsInTheBinMapStore(int aNumber);
};

}
}

#endif // IMPORTSPEKTRALOTSUMMARY_H
