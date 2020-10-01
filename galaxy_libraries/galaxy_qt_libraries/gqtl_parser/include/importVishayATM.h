#ifndef GEX_IMPORT_VISHAY_ATM_H
#define GEX_IMPORT_VISHAY_ATM_H

#include "parserBase.h"
#include "parameterDictionary.h"
#include "importConstants.h"
#include "stdf.h"
#include "stdfrecords_v4.h"
#include "queryable_by_test_number_store.h"

#include <time.h>
#include <qmap.h>
#include <qdatetime.h>
#include <qstringlist.h>
#include <QTextStream>

namespace GS
{
namespace Parser
{

struct VishayATMBinning
{
    int         mSoftBinNum;    // Soft Bin number
    QString     mName;          // Bin name
    bool        mPass;          // Bin cat
    int         mCount;         // Bin Summary count
};

class VishayATMtoSTDF : public ParserBase
{
public:

    /**
    * \fn VishayATMtoSTDF()
    * \brief constructure
    */
    VishayATMtoSTDF();

    /**
     * \fn static bool	IsCompatible(const char *aFileName)
     * \brief static function called by the import all to check if the input file is compatible with the
     *         Vishay ATM format.
     * \param aFileName is the input file, contains the teradyne ASCII file.
     * \return true if the input file is coompatible with the csv skyworks format. Otherwise return false.
     */
    static bool	IsCompatible(const QString &aFileName);

private:
    /**
     * \fn bool ConvertoStdf(const char *aInputFileName, QString& aFileNameSTDF)
     * \brief Read the teradyne ASCII file.
     * \param aInputFileName is the input file, contains the Csv file.
     * \param aFileNameSTDF is the output file, contains the stdf file.
     * \return true if the file has been correctly read. Otherwise return false.
     */
    bool ConvertoStdf(const QString& aInputFileName, QString& aFileNameSTDF);


    /**
     * \fn bool WriteStdfFile(QTextStream *aInputFile, const char *aFileNameSTDF)
     * \brief Write the stdf file.
     * \param aFileName is the input file, contains the csv file.
     * \param aFileNameSTDF is the output file, contains the stdf file.
     * \return true if the stdf file has been correctly written. Otherwise return false.
     */
    bool WriteStdfFile(const QString &aFileName, const QString& aFileNameSTDF);

    void  SpecificReadLine (QString &aString);

    /**
     * \fn bool EmptyLine(const QString& line)
     * \brief Check if th.e line is empty
     * \param line is the input line.
     * \return true if the line is empty.
     */
    bool EmptyLine(const QString& aLine);

    /**
     * \fn GetInputFiles(const QString &aSpektraDatalogFileName, QString &aPromisFilePath, QString &aBinMapFilePath)
     * \brief Find the path of the promis file and the bin mapping one
     * \param aInputFileName is the input file.
     * \param aPromisFilePath is the path of the promis file (output).
     * \param aBinMapFilePath is the path of the bin mapping file (output).
     * \return true if the input files has been correctly red.
     */
    bool GetInputFiles(const QString &aInputFileName, QString &aPromisFilePath, QString &aBinMapFilePath);

    bool ReadPromisFile(const QString &aPromisFile, const QString &aPromisKey, const QString &aConverterExternalFilePath);
    bool ReadBinMappingFile(const QString &aInputFilePath);
    bool GetBinMappingFileName(const QString &aPromisDataFilePath, const QString &aCategory, QString &lBinMapFileName);
    bool IsInTheFinalTestBinMapStore(int aNumber);
    bool IsInTheSortEntriesBinMapStore(int aNumber);
    void ProcessBinMappingItemByTestName(const Qx::BinMapping::BinMapItemBase &aBinMapItem, int aBinCount);
    void ProcessBinMappingItemByTestNumber(const Qx::BinMapping::BinMapItemBase &aBinMapItem, int aBinCount);

    void WriteSBR(VishayATMBinning &lItem, GQTL_STDF::StdfParse &aStdfParser);
    void WriteHBR(VishayATMBinning &lItem, GQTL_STDF::StdfParse &aStdfParser);

    QString mDateCode;
    QString mPackage;
    QString mProductID;
    QString mProcID;
    QString mSiteLocation;
    QString mTesterType;
    QString mPackageType;

    QScopedPointer< Qx::BinMapping::QueryableByTestNumber > mBinMapSortEntriesStore;
    QScopedPointer< Qx::BinMapping::QueryableByTestNumber > mBinMapFinalTestStore;

    QMap<std::string, VishayATMBinning> mVishayATMBinPerTestName;
    QMap<int, VishayATMBinning> mVishayATMBinPerBinNumber;

    GQTL_STDF::Stdf_SBR_V4 mSBRRecord;
    GQTL_STDF::Stdf_HBR_V4 mHBRRecord;
};

}
}
#endif
