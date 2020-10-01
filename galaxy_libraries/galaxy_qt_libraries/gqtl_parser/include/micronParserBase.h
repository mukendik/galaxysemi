#ifndef MICRONPARSERBASE_H
#define MICRONPARSERBASE_H

#include <qmap.h>
#include <qdatetime.h>
#include <qstringlist.h>
#include <QTextStream>

#include "importConstants.h"
#include "stdf.h"
#include "parserBase.h"
#include "stdfparse.h"

namespace GS
{
namespace Parser
{

class MicronParserBase : public ParserBase
{
public:
    /**
    * \fn MicronParserBase()
    * \brief constructor
    */
    MicronParserBase(ParserType lType, const QString &lName);
    /**
    * \fn ~MicronParserBase()
    * \brief destructor
    */
    virtual ~MicronParserBase();

    enum  ErrCodesMicron
    {
        ErrUncompressFail           = errParserSpecific + 1,    /// \param Fail to uncompress Micron input file
        ErrMissingMandatoryFile     = errParserSpecific + 2     /// \param At least on mandatory file is missing in the archive

    };

    void    SpecificReadLine           (QString &line) { line = line.simplified();}

protected:

    /**
     * \fn bool ReadHeaderSection
     * \brief Read the header section
     * \param lInputFileStream is the input stream.
     * \return true if the read finish with success.
     */
    bool ReadHeaderSection(QTextStream &lInputFileStream);

    bool ReadTestsDefinitionSection(QTextStream &inputTestsDefName,
                                    ParameterDictionary &parameterDirectory,
                                    QList< QPair<unsigned int, QString> >& testList,
                                    const QString keyWord);

    /**
     * \fn bool ReadSummarySection
     * \brief Read the summary section
     * \param lInputFileStream is the input stream.
     * \param lSBRRecordList the list of the found SBR
     * \param lHBRRecordList the map of the found <HBR_Number, HBR_record>
     * \return true if the read finish with success.
     */
    bool ReadSummarySection( QTextStream &lInputFileStream,
                             QList<GQTL_STDF::Stdf_SBR_V4*>& lSBRRecordList,
                             QMap<unsigned short, GQTL_STDF::Stdf_HBR_V4*>& lHBRRecordList);


    /**
     * \fn bool ReadDieDataSection
     * \brief Read the three sections of die data
     * \param lInputFile is the input stream.
     * \param dieData the list of the found die data. The QPair contains the coordinate of x and y
     * \param writeRecord indicates if we have to read the PTR, PIR and PRR record or just to fill the die data list
     * \param keyWord the key to define the beginning of the section (e.g. ^, fdd, sfdd)
     * \param lStdfFile the stdf output
     * \return true if the read finish with success.
     */
    bool ReadDieDataSection( QTextStream &lInputFile,
                             QHash< QPair<qint16, qint16>, QStringList > &dieData,
                             bool writeRecord,
                             const QString keyWord,
                             GS::StdLib::Stdf& lStdfFile,
                             QList< QPair<unsigned int, QString> >& testList);


    QHash<QString, unsigned short>  mPatBins;

    GQTL_STDF::Stdf_MIR_V4          mMIRRecord;
    GQTL_STDF::Stdf_MRR_V4          mMRRRecord;
    GQTL_STDF::Stdf_SDR_V4          mSDRRecord;
    GQTL_STDF::Stdf_WIR_V4          mWIRRecord;
    GQTL_STDF::Stdf_WRR_V4          mWRRRecord;
    GQTL_STDF::Stdf_WCR_V4          mWCRRecord;

};


}
}
#endif // MICRONPARSERBASE_H
