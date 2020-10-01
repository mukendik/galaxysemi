/******************************************************************************!
 * \file importSipex.h
 ******************************************************************************/
#ifndef GEX_IMPORT_SIPEX_H
#define GEX_IMPORT_SIPEX_H

#include "importConstants.h"
#include "stdf.h"
#include "parserBase.h"
#include "stdfparse.h"
#include "gs_types.h"

namespace GS
{
namespace Parser
{

class ParserParameter;
class ParserWafer;

/******************************************************************************!
 * \class SipextoSTDF
 ******************************************************************************/
class SipextoSTDF : public ParserBase
{
    enum eLimitType
    {
        eLowLimit,  // Flag to specify to save the Sipex Parameter LOW limit
        eHighLimit  // Flag to specify to save the Sipex Parameter HIGH limit
    };

public:
    /**
     * \fn SipextoSTDF
     * \brief Constructor
     */
    SipextoSTDF();
    /**
     * \fn ~SipextoSTDF
     * \brief Destructor
     */
    ~SipextoSTDF();

    /**
     * \fn static bool	IsCompatible(const QString &FileName)
     * \brief static function called by the import all to check if the input file is compatible with the
     *         csv Spinstand format.
     * \param FileName is the input file, contains the Spinstand ascii file.
     * \return true if the input file is compatible with the csv Spinstand format. Otherwise return false.
     */
    static bool	IsCompatible(const QString &aFileName);

private:
    /**
     * \fn bool ReadCsvFile(const QString &CsvFileName, const QString &StdfFileName)
     * \brief Read the Spinstand ascii file.
     * \param CsvFileName is the input file, contains the Csv file.
     * \param StdfFileName is the output file, contains the stdf file.
     * \return true if the file has been successfully read. Otherwise return false.
     */
    bool ConvertoStdf(const QString &aCsvFileName, QString &aStdfFileName);

    /**
     * \fn bool WriteStdfFile(QTextStream &CsvFile, const QString &StdfFileName)
     * \brief Write the stdf file.
     * \param CsvFile is the input file, contains the csv file.
     * \param StdfFileName is the output file, contains the stdf file.
     * \return true if the stdf file has been successfully written. Otherwise return false.
     */
    bool WriteStdfFile(QTextStream &aCsvFile, const QString &aStdfFileName) ;



    ParserParameter *FindParameterEntry(int iWaferID,int iSiteID,QString strParamName,QString strUnits="");
    void SaveParameterResult(int iWaferID,int iSiteID,QString strName,QString strUnits,QString strValue);
    void SaveParameterLimit(QString strName,QString strValue,int iLimit);
    void NormalizeLimits(ParserParameter *pParameter);

    QString mProductID;
    QString mLotID;         /// \param CUSTLOT
    QString mProcessID;
    QString mFlowID;
    QString mEngID;
    QString mJobRev;
    QString mJobName;

    QList<ParserWafer*>     mWaferList;
    QMap<QString,QString>   mMetaData;
    QStringList             mCriticalTests;         /// \param List of critical tests
};

}
}

#endif
