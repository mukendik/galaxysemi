#ifndef GEX_IMPORT_EGL_SKYWORKS_H
#define GEX_IMPORT_EGL_SKYWORKS_H

#include <time.h>

#include <qmap.h>
#include <qdatetime.h>
#include <qstringlist.h>
#include <QTextStream>

#include "stdf.h"
#include "stdfrecords_v4.h"
#include "parserBase.h"


namespace GS
{
namespace Parser
{
// Predefined parameter fields.
#define SKYNP_RAW_FIRST_DATA        17
#define SKYNP_POSX_NUMBER_RAW_DATA  7
#define SKYNP_POSY_NUMBER_RAW_DATA  8
#define SKYNP_DUT_NUMBER_RAW_DATA   9

class CGSkyNPCsvtoSTDF : public ParserBase
{
public:

    /**
    * \fn CGEGLSkyworkstoSTDF()
    * \brief constructor
    */
    CGSkyNPCsvtoSTDF();
    /**
    * \fn ~CGEGLSkyworkstoSTDF()
    * \brief destructor
    */
    ~CGSkyNPCsvtoSTDF();

    /**
     * \fn static bool	IsCompatible(const QString &FileName)
     * \brief static function called by the import all to check if the input file is compatible with the
     *         csv skyworks format.
     * \param FileName is the input file, contains the Skyworks ascii file.
     * \return true if the input file is compatible with the csv skyworks format. Otherwise return false.
     */
    static bool	IsCompatible(const QString &FileName);

    enum  ErrCodesSkyNP
    {
        InvalidReticlePosX  = errParserSpecific + 1,    /// \param Pox X value is not a number
        InvalidReticlePosY,                             /// \param Pox Y value is not a number
        InvalidDUTNumber                                /// \param Invalid DUT Number
    };

protected:
    /**
     * \fn std::string GetErrorMessage()
     * \brief Getter
     * \return The error message corresponding to the given error code
     */
     std::string GetErrorMessage(const int ErrorCode) const;

private:
    /**
     * \fn bool ReadCsvFile(const QString &CsvFileName, const QString &StdfFileName)
     * \brief Read the Skyworks ascii file.
     * \param CsvFileName is the input file, contains the Csv file.
     * \param StdfFileName is the output file, contains the stdf file.
     * \return true if the file has been successfully read. Otherwise return false.
     */
    bool ConvertoStdf(const QString &CsvFileName,  QString &StdfFileName);

    /**
     * @brief IsCompatibleMapUpdater
     * @return true if this file format is also compatible for the map updater
     */
    bool    IsCompatibleMapUpdater() const {return true;}

    /**
     * \fn bool WriteStdfFile(QTextStream &CsvFile, const QString &StdfFileName)
     * \brief Write the stdf file.
     * \param CsvFile is the input file, contains the csv file.
     * \param StdfFileName is the output file, contains the stdf file.
     * \return true if the stdf file has been successfully written. Otherwise return false.
     */
    bool WriteStdfFile(QTextStream &CsvFile, const QString &StdfFileName);


    void SpecificReadLine (QString &strString);

    /**
     * \fn bool WriteReticleDTR(const QString& field, GS::StdLib::Stdf& lStdfFile, const gsint16 xCoord, const gsint16 yCoord)
     * \brief Write the DTR into the stdf file.
     * \param fields is the list of fields on the current row.
     * \param lStdfFile is the output file, contains the stdf file.
     * \param xCoord is the x coordinate of the die.
     * \param yCoord is the y coordinate of the die.
     * \return true if the DTR has been successfully written. Otherwise return false.
     */
    bool WriteReticleDTR(const QStringList& fields, GS::StdLib::Stdf& lStdfFile, const gsint16 xCoord, const gsint16 yCoord);


    /**
     * \fn bool EmptyLine(const QString& line)
     * \brief Check if th.e line is empty
     * \param line is the input line.
     * \return true if the line is empty.
     */
    bool EmptyLine(const QString& line);

    ParserParameter             *mParameters;       /// \param List of Parameters tables.
    unsigned                    mTotalParameters;	/// \param Holds the total number of parameters / tests in each part tested
    bool                        mHasSiteNumber;     /// \param true if the file contains the site number's column
    int                         mSwBinCell;         /// \param Contains the location of the sofware bin cell
    QMap<int,ParserBinning *>	mSoftBinning;       /// \param List of soft Bin tables.
    QMap<int,ParserBinning *>	mHardBinning;		/// \param List of hard Bin tables.
};

}
}
#endif
