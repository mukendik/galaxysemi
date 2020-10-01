#ifndef GEX_IMPORT_CSV_SKYWORKS_H
#define GEX_IMPORT_CSV_SKYWORKS_H

#include <time.h>

#include <qmap.h>
#include <qdatetime.h>
#include <qstringlist.h>
#include <QTextStream>

#include "parserBase.h"
#include "parameterDictionary.h"
#include "importConstants.h"
#include "stdf.h"
#include "stdfrecords_v4.h"

// Predefined parameter fields.
#define	CSV_RAW_DIEX		2
#define	CSV_RAW_DIEY		3
#define	CSV_RAW_TIME		5
#define	CSV_RAW_TOTAL_TESTS	6
#define	CSV_RAW_OPERATOR	8
#define	CSV_RAW_LOT_ID		9
#define	CSV_RAW_SITE		15
#define	CSV_RAW_HBIN		17
#define	CSV_RAW_SBIN		18
#define CSV_RAW_FIRST_DATA  19


namespace GS
{
namespace Parser
{
class CGCSVSkyworkstoSTDF : public ParserBase
{
public:

    /**
    * \fn CGCSVSkyworkstoSTDF()
    * \brief constructure
    */
    CGCSVSkyworkstoSTDF();
    /**
    * \fn ~CGCSVSkyworkstoSTDF()
    * \brief destructure
    */
    ~CGCSVSkyworkstoSTDF();

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
     * \return true if the stdf file has been correctly written. Otherwise return false.
     */
    bool WriteStdfFile(QTextStream& hCsvFile, const QString& lFileNameSTDF);

    void  SpecificReadLine (QString &strString);

    /**
     * \fn bool EmptyLine(const QString& line)
     * \brief Check if th.e line is empty
     * \param line is the input line.
     * \return true if the line is empty.
     */
    bool EmptyLine(const QString& line);
    int     mLastError;			// Holds last error ID

    ParserParameter *       mCGCsvParameter;            // List of Parameters tables.
    unsigned                mTotalParameters;			// Holds the total number of parameters / tests in each part tested
    time_t                  mStartTime;                 // start time
    QStringList             mFullCsvParametersList;     // Complete list of ALL CSV parameters known.
};

}
}
#endif
