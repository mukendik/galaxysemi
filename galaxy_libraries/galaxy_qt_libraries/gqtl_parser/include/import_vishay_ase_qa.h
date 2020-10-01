#ifndef IMPORT_VISHAY_ASE_QA_H
#define IMPORT_VISHAY_ASE_QA_H

#include "parserBase.h"
#include "promis_interpreter_base.h"
#include "queryable_by_test_number_store.h"

#include <QVector>
#include <QScopedPointer>
#include <QFileInfo>

namespace GS
{
namespace Parser
{

class VishayASEQAtoSTDF : public ParserBase
{
public:

    enum  ErrCodesVishayASEQA
    {
        errNonMappedFailingTest  = errParserSpecific + 1, /// \param Failing test is not mapped
        errReadBinDefault,
        errUndefinedBinDefault
    };


    /**
    * \fn VishayASEQAtoSTDF()
    * \brief constructor
    */
    VishayASEQAtoSTDF();

    /**
     * \fn static bool	IsCompatible(const char *aFileName)
     * \brief static function called by the import all to check if the input file is compatible with the
     *         Vishay ASE QA format.
     * \param   aFileName is the input file
     * \return  true if the input file is compatible with the Vishay ASE QA format. Otherwise return false.
     */
    static bool	IsCompatible(const QString &aFileName);

private:

    static QVector<QString> mCompatibleKeys;

    QList<ParserParameter>      mParameterList;
    QMap<int, int>              mTestNumberToTestIndex;
    QMap<int, int>              mTestColumnToTestIndex;
    QMap<int, stdf_type_u2>     mTestColumnToBinNumber;
    QMap<int, ParserBinning>    mBinning;                   /// \param List of Bin tables.
    QScopedPointer< Qx::BinMapping::QueryableByTestNumber>  mBinMapFinalTestStore;
    QScopedPointer<Qx::BinMapping::PromisInterpreterBase>   mPromisInterpreter;

    int          mMinTestColumn;
    int          mMaxTestColumn;
    stdf_type_u1 mStationNumber;
    QString mDataFilePath;
    QFileInfo mDataFileInfo;
    QString mFacilityId;
    QString mSiteLocation;
    QString mTesterType;
    QString mDateCode;
    QString mPackage;
    QString mProcId;
    QString mPackageType;
    QString mProductId;
    QString mJobRev;
    QString mJobName;
    QString mLotId;
    QString mSublotId;
    QString mOperatorName;
    QString mHandlerId;
    QString mNodeName;

    static bool ReadHeader(QMap<QString, QString> &aHeaders, QTextStream& aTxtSTream, QString &aErrorMessage);
    static void InitCompatibleKeys();

    /**
     * \fn bool ConvertoStdf(const char *aInputFileName, QString& aFileNameSTDF)
     * \brief Read the Vishay ASE QA file.
     * \param aInputFileName is the input file, contains the .log2 file.
     * \param aFileNameSTDF is the output file, contains the stdf file.
     * \return true if the file has been correctly read. Otherwise return false.
     */
    bool ConvertoStdf(const QString& aInputFileName, QString& aFileNameSTDF);

    /**
    * \fn std::string GetErrorMessage()
    * \brief Getter
    * \return The error message corresponding to the given error code
    */
    std::string GetErrorMessage(const int ErrorCode) const;

    bool ParseTestDefinitionLine(const QString& aLine);
    bool ParsePartLine(const QString& aLine, GQTL_STDF::StdfParse& aStdfParse);
    bool ProcessHeader(QTextStream& aTxtStream);
    bool ProcessTestsDefinitions(QTextStream& aTxtStream);
    bool ProcessParts(QTextStream& aTxtStream, GQTL_STDF::StdfParse& aStdfParse);
    bool ReadBinMappingFile();
    bool ReadPromisDataFile();
    bool ReadDefaultBinMapping();
    bool WriteHBR(GQTL_STDF::StdfParse& aStdfParse);
    bool WriteMIR(GQTL_STDF::StdfParse& aStdfParse);
    bool WriteMRR(GQTL_STDF::StdfParse& aStdfParse);
    bool WritePIR(GQTL_STDF::StdfParse& aStdfParse);
    bool WritePRR(GQTL_STDF::StdfParse& aStdfParse, stdf_type_b1 aFlag, stdf_type_u2 aTestExecuted,
                  stdf_type_u2 aBin, stdf_type_u4 aTestTime, stdf_type_cn aPartId);
    bool WriteTestResults(GQTL_STDF::StdfParse& aStdfParse, QMap<int, QPair<QString, bool> > aExecutedTest);
    bool WriteSBR(GQTL_STDF::StdfParse& aStdfParse);
    bool WriteSDR(GQTL_STDF::StdfParse& aStdfParse);
    bool WriteTSR(GQTL_STDF::StdfParse& aStdfParse);
    bool WriteOutputStdf(QTextStream& aTxtStream, const QString& aFileNameSTDF);
};

}
}

#endif // IMPORT_VISHAY_ASE_QA_H
