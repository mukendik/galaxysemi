#ifndef IMPORVANGUARD_H
#define IMPORVANGUARD_H

#include "gqtl_global.h"
#include "parserBase.h"
#include "stdfparse.h"

namespace GS
{
namespace Parser
{

class VanguardPCMToStdf : public ParserBase
{
public:

    VanguardPCMToStdf();
    ~VanguardPCMToStdf();

    /**
     * \fn static bool	IsCompatible(const QString &FileName)
     * \brief static function called by the import all to check if the input file is compatible with the
     *         TriQuintRF format.
     * \param FileName is the input file, contains the TriQuintRF ascii file.
     * \return true if the input file is compatible with the TriQuintRF TriQuintRF format. Otherwise return false.
     */
    static bool IsCompatible(const QString &aFileName);

    /**
     * \fn bool ConvertoStdf(const QString &TriQuintRFFileName, QString &StdfFileName)
     * \brief Read the TriQuintRF ascii file.
     * \param fileName is the input file, contains the TriQuintRF file.
     * \param stdfFileName is the output file, contains the stdf file.
     * \return true if the file has been successfully read. Otherwise return false.
     */
    bool ConvertoStdf(const QString &aFileName, QString &aStdfFileName);

protected:
    /**
     * \fn bool ReadInputFile
     * \brief Read the values section
     * \param fileName is the input file.
     * \return true if the read finishes with success.
     */
    bool        ReadInputFile               (const QString &aFileName);

    /**
     * \fn bool WriteStdfFiles
     * \brief Write the stdf files
     * \param inputFileName is the input file name.
     * \param StdfFileName is the output file name.
     * \return true if the writing finishes with success.
     */
    bool        WriteStdfFiles              (const QString &aInputFileName, const QString& aStdfFileName);

    /**
     * \fn void SpecificReadLine
     * \brief Read a line.
     * \param line is the line to read.
     */
    void        SpecificReadLine(QString &aLine);

private:
    bool CloseFileWithError(QFile &aFile);

    /**
     * \fn bool WriteSummaries
     * \brief Write the summary of the stdf files: SBR, HBR, WRR and MRR
     * \param lStdfParser is the stdf file parser.
     * \param lPassBin is the number of pass parts.
     * \param lFailBin is the number of fail parts.
     * \param lPartCount is the number of parts.
     * \return true if the writing finishes with success.
     */
    bool WriteSummaries(GQTL_STDF::StdfParse &aStdfParser, int aPartCount);

    GQTL_STDF::Stdf_MIR_V4 mMIRRecord;
    QList<ParserParameter>  mParameterList;     /// \param List of Parameters in Wafer

};
}
}


#endif // IMPORVANGUARD_H
