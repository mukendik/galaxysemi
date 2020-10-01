#ifndef GEX_IMPORT_TriQuintRF_H
#define GEX_IMPORT_TriQuintRF_H

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

class TriQuintRFToSTDF : public ParserBase
{
public:
    /**
    * \fn TriQuintRFtoSTDF()
    * \brief constructor
    */
    TriQuintRFToSTDF();
    /**
    * \fn ~TriQuintRFtoSTDF()
    * \brief destructor
    */
    ~TriQuintRFToSTDF();


    /**
     * \fn static bool	IsCompatible(const QString &FileName)
     * \brief static function called by the import all to check if the input file is compatible with the
     *         TriQuintRF format.
     * \param FileName is the input file, contains the TriQuintRF ascii file.
     * \return true if the input file is compatible with the TriQuintRF TriQuintRF format. Otherwise return false.
     */
    static bool IsCompatible(const QString &FileName);

private:

    /**
     * \fn bool ReadTriQuintRFFile(const QString &TriQuintRFFileName, const QString &StdfFileName)
     * \brief Read the TriQuintRF ascii file.
     * \param TriQuintRFFileName is the input file, contains the TriQuintRF file.
     * \param StdfFileName is the output file, contains the stdf file.
     * \return true if the file has been successfully read. Otherwise return false.
     */
    bool ConvertoStdf(const QString &triQuintRFFileName,  QString &stdfFileName);

    /**
     * \fn bool WriteStdfFile(QTextStream &TriQuintRFFile, const QString &StdfFileName)
     * \brief Write the stdf file.
     * \param TriQuintRFFile is the input file, contains the TriQuintRF file.
     * \param StdfFileName is the output file, contains the stdf file.
     * \return true if the stdf file has been successfully written. Otherwise return false.
     */
    bool WriteStdfFile(QTextStream &inputFile, const QString &StdfFileName);


    void  SpecificReadLine (QString &line);

    GQTL_STDF::Stdf_MIR_V4 mMIRRecord;
    GQTL_STDF::Stdf_MRR_V4 mMRRRecord;
    GQTL_STDF::Stdf_SDR_V4 mSDRRecord;
    GQTL_STDF::Stdf_WIR_V4 mWIRRecord;
    GQTL_STDF::Stdf_WRR_V4 mWRRRecord;
    GQTL_STDF::Stdf_WCR_V4 mWCRRecord;

    QStringList                     mTestNames;
    QStringList                     mTestUnits;
    QDate                           mDate;
    QString                         mProbeId;
    QHash<QString, ParserParameter*> mTestParams;
};

}
}

#endif
