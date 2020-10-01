#ifndef AQUANTIAPCMTOSTDF_H
#define AQUANTIAPCMTOSTDF_H

#include "stdf.h"
#include "parserBase.h"


namespace GS
{
namespace Parser
{


class AquantiaPCMtoSTDF : public ParserBase
{
public:
    AquantiaPCMtoSTDF();

    /**
     * \fn static bool	IsCompatible(const QString &FileName)
     * \brief static function called by the import all to check if the input file is compatible with the
     *         Aquantia Parser for Global Foundries PCM format.
     * \param FileName is the input file, contains the Aquantia Parser for Global Foundries PCM file.
     * \return true if the input file is compatible with the Aquantia Parser for Global Foundries PCM format. Otherwise return false.
     */
    static bool	IsCompatible(const QString &FileName);

private:
    /**
     * \fn bool ReadSkyworksIFFFile(const QString &AquatiaPCMFileName, const QString &StdfFileName)
     * \brief Read the Aquatia PCM file.
     * \param AquatiaPCMFileName is the input file, contains the SkyworksIFF file.
     * \param StdfFileName is the output file, contains the stdf file.
     * \return true if the file has been successfully read. Otherwise return false.
     */
    gsbool ConvertoStdf(const QString &AquatiaPCMFileName,  QString &StdfFileName);

    /**
     * \fn bool ReadLimitHeaderSection
     * \brief Read the limit header section
     * \param lInputFileStream is the input file.
     * \return true if the read finish with success.
     */
    gsbool ReadLimitHeaderSection(QTextStream &lInputFileStream);

    /**
     * \fn bool ReadLimitSection
     * \brief Read the limit section
     * \param lInputFileStream is the input file.
     * \return true if the read finish with success.
     */
    gsbool ReadLimitSection(QTextStream &lInputFileStream);

    /**
     * \fn gsbool ReadWetLotSection
     * \brief Read the Wet Lot section
     * \param lInputFileStream is the input file.
     * \return true if the read finish with success.
     */
    gsbool ReadWetLotSection(QTextStream &lInputFileStream);

    /**
     * \fn gsbool ReadLine
     * \brief Read a line
     * \param inputFile is the input file.
     * \param line is the line to rea.
     * \param tag is the tag of to use to check the before stopping the reading
     * \return true if the read finish with success.
     */
    gsbool ReadLine(QTextStream &inputFile, QString &line, const QString &tag);

    /**
     * \fn void GetDateFromString
     * \brief Convert a string to a date
     * \param dateString is the input string.
     * \param dataTime is the output date
     * \return true if the convert has finished with success.
     */
    void GetDateFromString(QString dateString, QDateTime &dataTime);

    GQTL_STDF::Stdf_MIR_V4          mMIRRecord;
    GQTL_STDF::Stdf_SDR_V4          mSDRRecord;
    GQTL_STDF::Stdf_WIR_V4          mWIRRecord;
    GQTL_STDF::Stdf_PIR_V4          mPIRRecord;
    GQTL_STDF::Stdf_PRR_V4          mPRRRecord;
    GQTL_STDF::Stdf_MRR_V4          mMRRRecord;
    GQTL_STDF::Stdf_WRR_V4          mWRRRecord;
    GQTL_STDF::Stdf_WCR_V4          mWCRRecord;
    QHash<int, ParserParameter>     mTestList; /// \param The list of parameters, the key of the hash is the test number
};

}
}

#endif // AQUANTIAPCMTOSTDF_H
