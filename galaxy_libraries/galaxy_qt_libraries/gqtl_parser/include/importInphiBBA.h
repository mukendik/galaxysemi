#ifndef IMPORTINPHIBBA
#define IMPORTINPHIBBA

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

class InphiBBASTDF : public ParserBase
{
public:

    /**
    * \fn InphiPrestotoStdf()
    * \brief constructor
    */
    InphiBBASTDF();

    /**
    * \fn ~InphiPrestotoStdf()
    * \brief destructor
    */
    ~InphiBBASTDF();

    /**
     * \fn static bool	IsCompatible(const QString &FileName)
     * \brief static function called by the import all to check if the input file is compatible with the
     *         csv skyworks format.
     * \param FileName is the input file, contains the Skyworks ascii file.
     * \return true if the input file is compatible with the csv skyworks format. Otherwise return false.
     */
    static bool	IsCompatible(const QString &FileName);

    enum  ErrCodesInphiBBA
    {
        InvalidReticlePosX  = errParserSpecific + 1,    /// \param Pox X value is not a number
        InvalidReticlePosY,                             /// \param Pox Y value is not a number
        InvalidPositionInReticle,                       /// \param Invalid position of the die in the reticle
        InvalidKeyword,                                 /// \param Invalid or misplaced keyword detected
        MissingHardBinDefinition                        /// \param Hard binning definitio is missing from file
    };


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
     * \fn gsbool WriteReticleDTR
     * \brief Add a reticle DTR into the stdf file
     * \return true if the the reticle has been correctly writen
     */
    gsbool WriteReticleDTR(const QStringList &fields,
                           GQTL_STDF::StdfParse &lStdfFile);


    /**
     * \fn bool WriteStdfFile(QTextStream &CsvFile, const QString &StdfFileName)
     * \brief Write the stdf file.
     * \param CsvFile is the input file, contains the csv file.
     * \param StdfFileName is the output file, contains the stdf file.
     * \return true if the stdf file has been successfully written. Otherwise return false.
     */
    bool WriteStdfFile(QTextStream &aCsvFile, const QString &StdfFileName);


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

    /**
     * \brief Fill the list of hard bin and soft bin from the header
     */
    gsbool FillBinning(QTextStream &lCsvFile);

    ParserParameter           *mTestList;      /// \param List of Parameters tables.
    QMap<int,ParserBinning *> mSoftBinning;    /// \param List of soft Bin tables.
    QMap<int,ParserBinning *> mHardBinning;    /// \param List of hard Bin tables.
    GQTL_STDF::Stdf_MIR_V4    mMIRRecord;
    GQTL_STDF::Stdf_SDR_V4    mSDRRecord;
    GQTL_STDF::Stdf_MRR_V4    mMRRRecord;
    GQTL_STDF::Stdf_WIR_V4    mWIRRecord;
    GQTL_STDF::Stdf_WRR_V4    mWRRRecord;
};

}
}

#endif // IMPORTINPHIBBA
