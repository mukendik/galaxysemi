#ifndef GEX_IMPORT_MICRON_SUM_H
#define GEX_IMPORT_MICRON_SUM_H

#include <qmap.h>
#include <qdatetime.h>
#include <qstringlist.h>
#include <QTextStream>

#include "importConstants.h"
#include "stdf.h"
#include "parserBase.h"
#include "stdfparse.h"
#include "micronParserBase.h"

namespace GS
{
namespace Parser
{

class MicronSumToSTDF : public MicronParserBase
{
public:
    /**
    * \fn MicronSumToSTDF()
    * \brief constructor
    */
    MicronSumToSTDF();
    /**
    * \fn ~MicronSumToSTDF()
    * \brief destructor
    */
    ~MicronSumToSTDF();


    /**
     * \fn static bool	IsCompatible(const QString &FileName)
     * \brief static function called by the import all to check if the input file is compatible with the
     *         Micron format.
     * \param FileName is the input file, contains the Micron ascii file.
     * \return true if the input file is compatible with the Micron Micron format. Otherwise return false.
     */
    static bool IsCompatible(const QString &FileName);

private:

    /**
     * \fn bool ReadMicronFile(const QString &MicronFileName, const QString &StdfFileName)
     * \brief Read the Micron ascii file.
     * \param MicronFileName is the input file, contains the Micron file.
     * \param StdfFileName is the output file, contains the stdf file.
     * \return true if the file has been successfully read. Otherwise return false.
     */
    bool ConvertoStdf(const QString &micronFileName, QString &StdfFileName);

    /**
     * \fn bool WriteStdfFile(QTextStream &MicronFile, const QString &StdfFileName)
     * \brief Write the stdf file.
     * \param MicronFile is the input file, contains the Micron file.
     * \param StdfFileName is the output file, contains the stdf file.
     * \return true if the stdf file has been successfully written. Otherwise return false.
     */
    bool WriteStdfFile(QTextStream& micronSumStreamFile, const QString &StdfFileName);

    QHash<QString, unsigned short> mHbin;
    QHash<QString, unsigned short> mSbin;
};
}
}

#endif
