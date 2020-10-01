#ifndef GEX_IMPORT_VISHAY_ASE_H
#define GEX_IMPORT_VISHAY_ASE_H

#include "parserBase.h"
#include "parameterDictionary.h"
#include "importConstants.h"
#include "stdf.h"
#include "stdfrecords_v4.h"
#include "queryable_by_test_name_store.h"
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

struct VishayASEBinning
{
    int         mSoftBinNum;    // Soft Bin number
    QString     mName;          // Bin name
    bool        mPass;          // Bin cat
    int         mCount;         // Bin Summary count
};

class VishayASEtoSTDF : public ParserBase
{
public:

    /**
    * \fn VishayASEtoSTDF()
    * \brief constructure
    */
    VishayASEtoSTDF();

    /**
     * \fn static bool	IsCompatible(const char *aFileName)
     * \brief static function called by the import all to check if the input file is compatible with the
     *         Vishay ASE format.
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

    bool ReadBinMappingFile(const QString &aInputFilePath);

    QScopedPointer< Qx::BinMapping::QueryableByTestNumber> mBinMapSortEntiresStore;
    QScopedPointer< Qx::BinMapping::QueryableByTestNumber> mBinMapFinalTestStore;

    QMap<std::string, VishayASEBinning> mVishayASEBinPerTestName;
    QMap<int, VishayASEBinning> mVishayASEBinPerTestNumber;

    GQTL_STDF::Stdf_SBR_V4 mSBRRecord;
    GQTL_STDF::Stdf_HBR_V4 mHBRRecord;

    QString mDateCode;
    QString mPackage;
    QString mProductID;
    QString mProcID;
    QString mSiteLocation;
    QString mTesterType;
    QString mPackageType;

    bool GetBinMappingFileName(const QString &aPromisDataFilePath, const QString &aCategory, QString &lBinMapFileName);
    bool IsInTheFinalTestBinMapStore(int aNumber);
    bool IsInTheSortEntriesBinMapStore(int aNumber);
    void ProcessBinMappingItemByTestName(const Qx::BinMapping::BinMapItemBase &aBinMapItem, int aBinCount);
    void ProcessBinMappingItemByTestNumber(const Qx::BinMapping::BinMapItemBase &aBinMapItem, int aBinCount);

    void WriteSBR(VishayASEBinning &lItem, GQTL_STDF::StdfParse &aStdfParser);
    void WriteHBR(VishayASEBinning &lItem, GQTL_STDF::StdfParse &aStdfParser);
    bool ReadPromisFile(const QString &aPromisFile, const QString &aPromisKey, const QString &aConverterExternalFilePath);
};

}
}
#endif
