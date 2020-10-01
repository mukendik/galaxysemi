#ifndef IMPORTINPHIPESTRO
#define IMPORTINPHIPESTRO

#include <qstringlist.h>
#include <QTextStream>

#include "stdf.h"
#include "stdfrecords_v4.h"
#include "parserBase.h"
#include "gs_types.h"

namespace GS
{
namespace Parser
{

// Predefined parameter fields.
#define INPHI_PRESTO_WS_FIRST_RAW_DATA  20
#define INPHI_PRESTO_FT_FIRST_RAW_DATA  18

class InphiPrestotoSTDF : public ParserBase
{
public:

    /**
    * \fn InphiPrestotoStdf()
    * \brief constructor
    */
    InphiPrestotoSTDF();

    /**
    * \fn ~InphiPrestotoStdf()
    * \brief destructor
    */
    ~InphiPrestotoSTDF();

    /**
     * \fn static bool	IsCompatible(const QString &FileName)
     * \brief static function called by the import all to check if the input file is compatible with the
     *         csv skyworks format.
     * \param FileName is the input file, contains the Skyworks ascii file.
     * \return true if the input file is compatible with the csv skyworks format. Otherwise return false.
     */
    static bool	IsCompatible(const QString &FileName);

private:
    /**
     * \fn bool ReadCsvFile(const QString &CsvFileName, const QString &StdfFileName)
     * \brief Read the Skyworks ascii file.
     * \param CsvFileName is the input file, contains the Csv file.
     * \param StdfFileName is the output file, contains the stdf file.
     * \return true if the file has been successfully read. Otherwise return false.
     */
    bool ConvertoStdf(const QString &CsvFileName, QString &StdfFileName);

    /**
     * \fn bool WriteStdfFile(QTextStream &CsvFile, const QString &StdfFileName)
     * \brief Write the stdf file.
     * \param CsvFile is the input file, contains the csv file.
     * \param StdfFileName is the output file, contains the stdf file.
     * \return true if the stdf file has been successfully written. Otherwise return false.
     */
    bool WriteStdfFile(QTextStream &CsvFile, const QString &StdfFileName);


    /**
     * \fn bool EmptyLine(const QString& line)
     * \brief Check if the line is empty
     * \param line is the input line.
     * \return true if the line is empty.
     */
    bool EmptyLine(const QString& line);

    /**
     * \fn void SkipEmptyLine(const QString& line)
     * \brief Check if the line is empty according to detection rules
     * \param line is the input line.
     */
    void    SpecificReadLine (QString &line);

    /**
     * \fn gsbool IsAtEndOfFile(const QTextStream& lCsvFile)
     * \brief Check if file is at the end of the file
     * \param lCsvFile is the input stream.
     * \return true if the file is at the end.
     */
    gsbool IsNotAtEndOfFile(const QTextStream& lCsvFile);

    /**
     * \fn gstime_t GetDate(const QString date, const QString time)
     * \brief Convert the date and time from string to time_t
     * \param date the date.
     * \param time the time
     * \param daysToAdd number of days to add (>0) or to remove (<0)
     * \return a time from the date and time, or 0 if date or time is invalid
     */
    gstime_t GetDate(const QString date, const QString time, const int daysToAdd=0);

    /**
     * \brief return empty string if the parameter string is emty. Otherwise, return the parameter string.
     */
    QString GetNotNAString(const QString string);

    ParserParameter           *mTestList;      /// \param List of Parameters tables.
    unsigned                  mTotalParameters;/// \param Holds the total number of parameters / tests in each part tested
    QMap<int,ParserBinning *> mSoftBinning;    /// \param List of soft Bin tables.
    short                     mFirstTestRaw;   /// \param Contains the index of the first test raw
    unsigned short            mShift;          /// \param Contains the count if the header contains multiple Source_Lot, else 0
    QMap<QString, int>        mFieldPos;       /// \param Contains the position of mandatory fields
    gsbool                    mIsWaferSortFile;/// \param contains true if the file is a wafer sort

};

}
}

#endif // IMPORTINPHIPESTRO
