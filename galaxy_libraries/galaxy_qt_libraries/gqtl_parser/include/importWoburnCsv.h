#ifndef GEX_IMPORT_Woburn_CSV_H
#define GEX_IMPORT_Woburn_CSV_H

#include <time.h>

#include <qmap.h>
#include <qdatetime.h>
#include <qstringlist.h>
#include <QTextStream>

#include "parserBase.h"
#include "parameterDictionary.h"
#include "importConstants.h"
#include "stdf.h"
#include "parserBinning.h"

#include "stdfrecords_v4.h"
#include "gs_types.h"

// Predefined parameter fields.
#define CSV_WOBURN_RAW_PART_TYP     2
#define	CSV_WOBURN_RAW_LOT_ID		3
#define	CSV_WOBURN_RAW_OPERATOR     4
#define	CSV_WOBURN_RAW_WAFER		5
#define	CSV_WOBURN_RAW_MAPID		6
#define	CSV_WOBURN_RAW_DIEX         7
#define	CSV_WOBURN_RAW_DIEY         8
#define	CSV_WOBURN_RAW_DUT          9
#define	CSV_WOBURN_RAW_NODE_NAME	10
#define	CSV_WOBURN_RAW_TST_TEMP     11
#define	CSV_WOBURN_RAW_JOB_NAM      12
#define	CSV_WOBURN_RAW_SPEC_NAM     13
#define	CSV_WOBURN_RAW_CARD_ID      14
#define	CSV_WOBURN_RAW_DATE         15
#define	CSV_WOBURN_RAW_TIME         16
#define CSV_WOBURN_RAW_FIRST_DATA   17


namespace GS
{
namespace Parser
{
class WoburnCSVtoSTDF : public ParserBase
{
    friend class WoburnHeaderParser;

public:

    /**
    * \fn WoburnCSVtoSTDF()
    * \brief constructure
    */
    WoburnCSVtoSTDF();
    /**
    * \fn ~WoburnCSVtoSTDF()
    * \brief destructure
    */
    ~WoburnCSVtoSTDF();

    /**
     * \fn static bool	IsCompatible(const char *lFileName)
     * \brief static function called by the import all to check if the input file is compatible with the
     *         csv skyworks format.
     * \param lFileName is the input file, contains the teradyne ascii file.
     * \return true if the input file is coompatible with the csv skyworks format. Otherwise return false.
     */
    static bool	IsCompatible(const QString &lFileName);

private:
    /**
     * \fn bool ConvertoStdf(const char *CsvFileName,const char *strFileNameSTDF)
     * \brief Read the teradyne ascii file.
     * \param lCsvFileName is the input file, contains the Csv file.
     * \param lFileNameSTDF is the output file, contains the stdf file.
     * \return true if the file has been correctly read. Otherwise return false.
     */
    bool ConvertoStdf(const QString& lCsvFileName,  QString& lFileNameSTDF);

    /**
     * \fn bool ProcessMultiLimits(const QMap<int, QPair<QString, QString> >& lLimitsCase)
     * \brief Process CSV lines containing limits case informations in order to buils multi-limits set
     * \param lLimitsCase A map which contains low an high limits informations for each case
     * \return true if the multi-limits are correctly parsed. Otherwise return false.
     */
    bool ProcessMultiLimits(const QMap<int, QPair<QString, QString> >& lLimitsCase);

    /**
     * \fn bool WriteStdfFile(QTextStream *hCsvFile, const char *strFileNameSTDF)
     * \brief Write the stdf file.
     * \param hCsvFile is the input file, contains the csv file.
     * \param lFileNameSTDF is the output file, contains the stdf file.
     * \param aCsvFilePath csv input file path
     * \return true if the stdf file has been correctly written. Otherwise return false.
     */
    bool WriteStdfFile(QTextStream& hCsvFile, const QString& lFileNameSTDF, const QString &aCsvFilePath);

    void  SpecificReadLine (QString &strString);

    gsbool AddMetaDataToDTR(QString key, QString fieldValue);
    gsbool GetDate(const QString& dateString, const QString& timeString, time_t& date);

    bool WriteReticleDTR(const QStringList &fields, const gsint16 xCoord, const gsint16 yCoord);

    /**
     * \fn bool EmptyLine(const QString& line)
     * \brief Check if th.e line is empty
     * \param line is the input line.
     * \return true if the line is empty.
     */
    bool EmptyLine(const QString& line);

    QList<ParserParameter>          mParameters;            // List of Parameters tables.
    gsuint8                         mTotalParameters;	   // Holds the total number of parameters / tests in each part tested
    time_t                          mStartTime;            // start time
    QStringList                     mFullCsvParametersList;// Complete list of ALL CSV parameters known.
    QString                         mWaferId;
    gsuint16                        mSwBinCell;
    GQTL_STDF::Stdf_MIR_V4          mMIRRecord;
    GQTL_STDF::Stdf_WCR_V4          mWCRRecord;
    GQTL_STDF::Stdf_WRR_V4          mWRRRecord;
    GQTL_STDF::Stdf_PCR_V4          mPCRRecord;
    QList<GQTL_STDF::Stdf_DTR_V4*>  mDTRMetaDataRecords;
    QMap<int,ParserBinning>         mSoftBins;       /// \param List of soft Bin tables.
    QMap<int,ParserBinning>         mHardBins;		/// \param List of hard Bin tables.
    gsint16                         mPassedTime;
};

}
}
#endif
