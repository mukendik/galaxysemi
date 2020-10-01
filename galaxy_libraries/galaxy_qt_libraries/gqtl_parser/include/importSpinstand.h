#ifndef GEX_IMPORT_EGL_SPINSTAND_H
#define GEX_IMPORT_EGL_SPINSTAND_H

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
#define SPINSTAND_RAW_FIRST_DATA        1

class CGSpinstandtoSTDF : public ParserBase
{
public:

    /**
    * \fn CGSpinstandtoSTDF()
    * \brief constructor
    */
    CGSpinstandtoSTDF();
    /**
    * \fn ~CGSpinstandtoSTDF()
    * \brief destructor
    */
    ~CGSpinstandtoSTDF();

    /**
     * \fn static bool	IsCompatible(const QString &FileName)
     * \brief static function called by the import all to check if the input file is compatible with the
     *         csv Spinstand format.
     * \param FileName is the input file, contains the Spinstand ascii file.
     * \return true if the input file is compatible with the csv Spinstand format. Otherwise return false.
     */
    static bool	IsCompatible(const QString &FileName);

protected:

private:
    /**
     * \fn bool ReadCsvFile(const QString &CsvFileName, const QString &StdfFileName)
     * \brief Read the Spinstand ascii file.
     * \param CsvFileName is the input file, contains the Csv file.
     * \param StdfFileName is the output file, contains the stdf file.
     * \return true if the file has been successfully read. Otherwise return false.
     */
    bool ConvertoStdf(const QString &CsvFileName, QString &StdfFileName);

    /**
     * \fn bool WriteStdfFile(QTextStream &CsvFile, const QString &StdfFileName)
     * \brief Write the stdf file.
     * \param CsvFile is the input file, contains the csv file.
     * \param StdfFileName is the output file, contains the stdf file.
     * \return true if the stdf file has been successfully written. Otherwise return false.
     */
    bool WriteStdfFile(QTextStream &CsvFile, const QString &StdfFileName);


    void SpecificReadLine (QString &strString);

    ParserParameter                     *mParameters;       /// \param List of Parameters tables.
    int                                 mTotalParameters;	/// \param Holds the total number of parameters / tests in each part tested
    int                                 mBinCell;           /// \param Contains the location of the bin cell
    int                                 mTimeCell;          /// \param Contains the location of the time cell
    QMap<int,ParserBinning *>           mBinning;           /// \param List of hard Bin tables.
    QMap<int,QMap<QString,QString> >    mTestConds;         /// \param List of TestConds value <PosIndex, "CondA=a, CondB=b, ...">
};

}
}
#endif
