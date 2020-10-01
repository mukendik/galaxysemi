#ifndef GEX_IMPORT_SKYWORKSIFF_H
#define GEX_IMPORT_SKYWORKSIFF_H

#include <qmap.h>
#include <qdatetime.h>
#include <qstringlist.h>
#include <QTextStream>
#include <list>

#include "importConstants.h"
#include "stdf.h"
#include "parserBase.h"
#include "stdfparse.h"
#include "gs_types.h"


namespace GS
{
namespace Parser
{

class SkyworksIFFToSTDF : public ParserBase
{
public:
    /**
    * \fn SkyworksIFFToSTDF()
    * \brief constructor
    */
    SkyworksIFFToSTDF();
    /**
    * \fn ~SkyworksIFFToSTDF()
    * \brief destructor
    */
    ~SkyworksIFFToSTDF();

    enum  ErrCodesIFF
    {
        UnknownDataType     = errParserSpecific + 1,    /// \param Unable to detect IFF Data type
        MissingWaferConfig,                             /// \param Wafermap configuration section is missing
    };


    /**
     * \fn static bool	IsCompatible(const QString &FileName)
     * \brief static function called by the import all to check if the input file is compatible with the
     *         SkyworksIFF format.
     * \param FileName is the input file, contains the SkyworksIFF ascii file.
     * \return true if the input file is compatible with the SkyworksIFF SkyworksIFF format. Otherwise return false.
     */
    static bool IsCompatible(const QString &FileName);


protected:
    /**
     * \fn std::string GetErrorMessage()
     * \brief Getter
     * \return The error message corresponding to the given error code
     */
     std::string GetErrorMessage(const int ErrorCode) const;

private:

    /**
     * \fn bool ReadSkyworksIFFFile(const QString &SkyworksIFFFileName, const QString &StdfFileName)
     * \brief Read the SkyworksIFF ascii file.
     * \param SkyworksIFFFileName is the input file, contains the SkyworksIFF file.
     * \param StdfFileName is the output file, contains the stdf file.
     * \return true if the file has been successfully read. Otherwise return false.
     */
    gsbool ConvertoStdf(const QString &SkyworksIFFFileName,  QString &StdfFileName);

    /**
     * \fn bool WriteStdfFile(QTextStream &SkyworksIFFFile, const QString &StdfFileName)
     * \brief Write the stdf file.
     * \param lStreamFile is the input stream, contains the SkyworksIFF file.
     * \param StdfFileName is the output file, contains the stdf file.
     * \return true if the stdf file has been successfully written. Otherwise return false.
     */
    gsbool WriteStdfFile(QTextStream &lStreamFile, const QString &StdfFileName);


    /**
     * \fn bool ReadDieData
     * \brief Read the three sections of die data
     * \param fileName is the input file.
     * \param dieData the list of the found die data. The QPair contains the coordinate of x and y
     * \param writeRecord indicates if we have to read the PTR, PIR and PRR record or just to fill the die data list
     * \param keyWord the key to define the beginning of the section (e.g. ^, fdd, sfdd)
     * \param lStdfFile the stdf output
     * \return true if the read finish with success.
     */
    gsbool ReadDieData(QString fileName,
                     QHash<QPair<qint16, qint16>, QStringList> &dieData,
                     bool writeRecord,
                     const QString keyWord,
                     GS::StdLib::Stdf& lStdfFile,
                     QList< QPair<unsigned int, QString> >& testList);


    /**
     * \fn bool ReadPatSection
     * \brief Read the PAT section
     * \param lInputFileStream is the input file.
     * \return true if the read finish with success.
     */
    gsbool ReadPatSection(QTextStream &lInputFileStream);

    /**
     * \fn bool ReadHeaderSection
     * \brief Read the header section
     * \param lInputFileStream is the input file.
     * \return true if the read finish with success.
     */
    gsbool ReadHeaderSection(QTextStream &lInputFileStream);

    /**
     * \fn bool ReadEquipConfigSection
     * \brief Read the equip config section
     * \param lInputFileStream is the input file.
     * \return true if the read finish with success.
     */
    gsbool ReadEquipConfigSection(QTextStream &lInputFileStream);

    /**
     * \fn bool ReadWaferMapConfigSection
     * \brief Read the wafer map configuration  section
     * \param lInputFileStream is the input file.
     * \return true if the read finish with success.
     */
    gsbool ReadWaferMapConfigSection(QTextStream &lInputFileStream);

    /**
     * \fn bool ReadLimitsSection
     * \brief Read the limit section
     * \param lInputFileStream is the input file.
     * \return true if the read finish with success.
     */
    gsbool ReadLimitsSection(QTextStream &lInputFileStream);

    void WriteHBR(GS::StdLib::Stdf& stdfFile, int binNumber, int totalBins, bool isPassed);
    void WriteSBR(GS::StdLib::Stdf& stdfFile, int binNumber, int totalBins, bool isPassed);

    /**
     * \fn bool ReadBinDefinitonsSection
     * \brief Read the Bin definition section
     * \param lInputFileStream is the input file.
     * \return true if the read finish with success.
     */
    gsbool ReadBinDefinitonsSection(QTextStream &lInputFileStream);

    /**
     * \fn gsbool AddMetaDataToDTR
     * \brief Add a DTR which contains a Meta Data
     * \param key is the key to insert in the DTR.
     * \param fieldValue the value of the key
     * \return true if write is correct.
     */
    gsbool AddMetaDataToDTR(const QString &key, QString fieldValue);

    QString GetNotNAString(QString string);

    QMap<QString, int>  mMonth;
    GQTL_STDF::Stdf_MIR_V4          mMIRRecord;
    GQTL_STDF::Stdf_MRR_V4          mMRRRecord;
    GQTL_STDF::Stdf_SDR_V4          mSDRRecord;
    GQTL_STDF::Stdf_WIR_V4          mWIRRecord;
    GQTL_STDF::Stdf_WRR_V4          mWRRRecord;
    GQTL_STDF::Stdf_PCR_V4          mPCRRecord;
    GQTL_STDF::Stdf_WCR_V4          mWCRRecord;
    GQTL_STDF::Stdf_PRR_V4          mPRRRecord;
    GQTL_STDF::Stdf_PIR_V4          mPIRRecord;
    GQTL_STDF::Stdf_SBR_V4          mSBR;
    GQTL_STDF::Stdf_HBR_V4          mHBR;
    GQTL_STDF::Stdf_DTR_V4          mDTRRecord;
    GQTL_STDF::Stdf_PTR_V4          mPTRRecord;
    QList<GQTL_STDF::Stdf_DTR_V4*>  mDTRRecords;
    QList<ParserParameter>          mTestList;          /// \param The list of parameters

    QMap<int,ParserBinning>         mSoftBinning;       /// \param List of soft Bin tables.
    QMap<int,ParserBinning>         mHardBinning;		/// \param List of hard Bin tables.
    QList<gsuint16>                 mFailHBin;          /// \param The list of pass hard bins
    QList<gsuint16>                 mFailSBin;          /// \param The list of pass shoft bins
    QString                         mIFFType;           /// \param Contains a string which represents the Tesing stage FT, WP or PCM
    time_t                          mFinishTime;         //// \param Finish time
};
}
}

#endif
