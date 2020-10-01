#ifndef GEX_IMPORT_TriQuintDC_H
#define GEX_IMPORT_TriQuintDC_H

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

class TriQuintDCToSTDF : public ParserBase
{
public:
    /**
    * \fn TriQuintDCtoSTDF()
    * \brief constructor
    */
    TriQuintDCToSTDF();
    /**
    * \fn ~TriQuintDCtoSTDF()
    * \brief destructor
    */
    ~TriQuintDCToSTDF();

    /**
     * \fn static bool	IsCompatible(const QString &FileName)
     * \brief static function called by the import all to check if the input file is compatible with the
     *         TriQuintDC format.
     * \param FileName is the input file, contains the TriQuintDC ascii file.
     * \return true if the input file is compatible with the TriQuintDC TriQuintDC format. Otherwise return false.
     */
    static bool IsCompatible(const QString &FileName);

private:

    /**
     * \fn bool ReadTriQuintDCFile(const QString &TriQuintDCFileName, const QString &StdfFileName)
     * \brief Read the TriQuintDC ascii file.
     * \param TriQuintDCFileName is the input file, contains the TriQuintDC file.
     * \param StdfFileName is the output file, contains the stdf file.
     * \return true if the file has been successfully read. Otherwise return false.
     */
    bool ConvertoStdf(const QString &triQuintDCFileName,
                             QString &stdfFileName);

    /**
     * \fn bool WriteStdfFile(QTextStream &TriQuintDCFile, const QString &StdfFileName)
     * \brief Write the stdf file.
     * \param TriQuintDCFile is the input file, contains the TriQuintDC file.
     * \param StdfFileName is the output file, contains the stdf file.
     * \return true if the stdf file has been successfully written. Otherwise return false.
     */
    bool WriteStdfFile(QTextStream &inputFile,
                       const QString &StdfFileName,
                       ParameterDictionary &lParameterDirectory);

    /**
     * \fn bool GetDateFromString(QString dateString, QString timeString, QDateTime& dataTime)
     * \brief Get the dateTime from the date string
     * \param dateString is the date string.
     * \param dataTime is the returned date time.
     * \return True if the convertion has been done successfully.
     */
    bool GetDateFromString(QString dateString, QDateTime& dataTime);

    /**
     * \fn bool WriteStdfFile(StdLib::Stdf &lStdfFile, QStringList& lAllLines)
     * \brief Write one stdf file from the list of all lines
     * \param lStdfFile is the output stdf file.
     * \param lAllLines is the list of file lines.
     * \return True if the file has been correctly writen.
     */
    bool WriteStdfFile(StdLib::Stdf &lStdfFile, QStringList& lAllLines);

    QDate                           mDate;          /// \param The date of the file
    QList<ParserParameter>          mTestParams;    /// \param The list of parameters
};

}
}

#endif
