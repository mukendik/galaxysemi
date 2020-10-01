#ifndef IMPORT_STDF_H
#define IMPORT_STDF_H

#include "vishay_stdf_record_overwrite.h"
#include "parserBase.h"

namespace GS
{
namespace Parser
{

class STDFtoSTDF : public ParserBase
{
public:
    STDFtoSTDF();

    /**
     * \fn static bool	IsCompatible(const char *aFileName)
     * \brief static function called by the import all to check if the input file is compatible with the
     *         Stdf format.
     * \param aFileName is the input file, contains the stdf file.
     * \return true if the input file is coompatible with the csv skyworks format. Otherwise return false.
     */
    static bool	IsCompatible(const QString &aFileName);

private:
    /**
     * \fn bool ConvertoStdf(const char *aInputFileName, QString& aFileNameSTDF)
     * \brief Read the stdf file.
     * \param aInputFileName is the input file, contains the stdf file.
     * \param aFileNameSTDF is the output file, contains the stdf file.
     * \return true if the file has been correctly read. Otherwise return false.
     */
    bool ConvertoStdf(const QString& aInputFileName, QString& aFileNameSTDF);

    /**
     * \fn bool WriteStdfFile(QTextStream *aInputStdfFile, const char *aFileNameSTDF)
     * \brief Write the stdf file.
     * \param aInputStdfFile is the input file, contains the stdf file.
     * \param aFileNameSTDF is the output file, contains the stdf file.
     * \return true if the stdf file has been correctly written. Otherwise return false.
     */
    bool WriteStdfFile(const QString &aInputStdfFile, const QString& aOutputStdfFile);

    /**
     * @brief WriteMappedHbrSbrList write SBR and HBR records
     * @param aWriter ref to the object used to write output stdf
     * @param aErrorMsg
     * @return false if there are issue when writting records, else true
     */
    bool WriteMappedHbrSbrList(GQTL_STDF::StdfParse& aWriter, QString& aErrorMsg);

    /**
     * @brief StdfValidityCheck Check stdf type match specified type in external files
     * @return true if OK
     */
    bool StdfValidityCheck(const QString& aInputStdfFile);
    bool StdfValidityCheckWIR(GQTL_STDF::StdfParse& aReader, QString& aErrorMsg);
    bool StdfValidityCheckPIR(QString& aErrorMsg);

    bool ProcessFAR(GQTL_STDF::StdfParse& aReader, GQTL_STDF::StdfParse& aWriter, QString& aErrorMsg);
    bool ProcessATR(GQTL_STDF::StdfParse& aReader, GQTL_STDF::StdfParse& aWriter, QString& aErrorMsg);
    bool ProcessMIR(GQTL_STDF::StdfParse& aReader, GQTL_STDF::StdfParse& aWriter, QString& aErrorMsg);
    bool ProcessMRR(GQTL_STDF::StdfParse& aReader, GQTL_STDF::StdfParse& aWriter, QString& aErrorMsg);
    bool ProcessPCR(GQTL_STDF::StdfParse& aReader, GQTL_STDF::StdfParse& aWriter, QString& aErrorMsg);
    bool ProcessHBR(GQTL_STDF::StdfParse& aReader, GQTL_STDF::StdfParse& aWriter, QString& aErrorMsg);
    bool ProcessSBR(GQTL_STDF::StdfParse& aReader, QString& aErrorMsg);
    bool ProcessPMR(GQTL_STDF::StdfParse& aReader, GQTL_STDF::StdfParse& aWriter, QString& aErrorMsg);
    bool ProcessPGR(GQTL_STDF::StdfParse& aReader, GQTL_STDF::StdfParse& aWriter, QString& aErrorMsg);
    bool ProcessPLR(GQTL_STDF::StdfParse& aReader, GQTL_STDF::StdfParse& aWriter, QString& aErrorMsg);
    bool ProcessRDR(GQTL_STDF::StdfParse& aReader, GQTL_STDF::StdfParse& aWriter, QString& aErrorMsg);
    bool ProcessSDR(GQTL_STDF::StdfParse& aReader, GQTL_STDF::StdfParse& aWriter, QString& aErrorMsg);
    bool ProcessWIR(GQTL_STDF::StdfParse& aReader, GQTL_STDF::StdfParse& aWriter, QString& aErrorMsg);
    bool ProcessWRR(GQTL_STDF::StdfParse& aReader, GQTL_STDF::StdfParse& aWriter, QString& aErrorMsg);
    bool ProcessWCR(GQTL_STDF::StdfParse& aReader, GQTL_STDF::StdfParse& aWriter, QString& aErrorMsg);
    bool ProcessPIR(GQTL_STDF::StdfParse& aReader, GQTL_STDF::StdfParse& aWriter, QString& aErrorMsg);
    bool ProcessPRR(GQTL_STDF::StdfParse& aReader, GQTL_STDF::StdfParse& aWriter, QString& aErrorMsg);
    bool ProcessTSR(GQTL_STDF::StdfParse& aReader, GQTL_STDF::StdfParse& aWriter, QString& aErrorMsg);
    bool ProcessPTR(GQTL_STDF::StdfParse& aReader, GQTL_STDF::StdfParse& aWriter, QString& aErrorMsg);
    bool ProcessMPR(GQTL_STDF::StdfParse& aReader, GQTL_STDF::StdfParse& aWriter, QString& aErrorMsg);
    bool ProcessFTR(GQTL_STDF::StdfParse& aReader, GQTL_STDF::StdfParse& aWriter, QString& aErrorMsg);
    bool ProcessBPS(GQTL_STDF::StdfParse& aReader, GQTL_STDF::StdfParse& aWriter, QString& aErrorMsg);
    bool ProcessEPS(GQTL_STDF::StdfParse& aReader, GQTL_STDF::StdfParse& aWriter, QString& aErrorMsg);
    bool ProcessGDR(GQTL_STDF::StdfParse& aReader, GQTL_STDF::StdfParse& aWriter, QString& aErrorMsg);
    bool ProcessDTR(GQTL_STDF::StdfParse& aReader, GQTL_STDF::StdfParse& aWriter, QString& aErrorMsg);
    bool ProcessRESERVED_IMAGE(GQTL_STDF::StdfParse& aReader, GQTL_STDF::StdfParse& aWriter, QString& aErrorMsg);
    bool ProcessRESERVED_IG900(GQTL_STDF::StdfParse& aReader, GQTL_STDF::StdfParse& aWriter, QString& aErrorMsg);
    bool ProcessUNKNOWN(GQTL_STDF::StdfParse& aReader, GQTL_STDF::StdfParse& aWriter, QString& aErrorMsg);
    bool ProcessVUR(GQTL_STDF::StdfParse& aReader, GQTL_STDF::StdfParse& aWriter, QString& aErrorMsg);
    bool ProcessPSR(GQTL_STDF::StdfParse& aReader, GQTL_STDF::StdfParse& aWriter, QString& aErrorMsg);
    bool ProcessNMR(GQTL_STDF::StdfParse& aReader, GQTL_STDF::StdfParse& aWriter, QString& aErrorMsg);
    bool ProcessCNR(GQTL_STDF::StdfParse& aReader, GQTL_STDF::StdfParse& aWriter, QString& aErrorMsg);
    bool ProcessSSR(GQTL_STDF::StdfParse& aReader, GQTL_STDF::StdfParse& aWriter, QString& aErrorMsg);
    bool ProcessCDR(GQTL_STDF::StdfParse& aReader, GQTL_STDF::StdfParse& aWriter, QString& aErrorMsg);
    bool ProcessSTR(GQTL_STDF::StdfParse& aReader, GQTL_STDF::StdfParse& aWriter, QString& aErrorMsg);
    bool WriteATR(GQTL_STDF::StdfParse &aWriter, QString &lErrorMsg);

    VishayStdfRecordOverWrite       mVishayStdfRecordOverWrite; ///< Holds functions to overwrite the stdf with Vishay rules
    VishayStdfRecordOverWrite::Type mStdfDetectedType;          ///< Stores the detected type
    int                             mLastFailedTest;            ///< Stores last failed test for a given part
    QString                         mStdfWaferId;               ///< Stores Wafer Id detected during stdf validity check
};

} // Parser
} // GS
#endif // IMPORT_STDF_H
