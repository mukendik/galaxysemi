#ifndef GEX_IMPORT_MICRON_H
#define GEX_IMPORT_MICRON_H

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

class MicronToSTDF : public MicronParserBase
{
public:
    /**
    * \fn CGEGLMicrontoSTDF()
    * \brief constructor
    */
    MicronToSTDF();
    /**
    * \fn ~CGEGLMicrontoSTDF()
    * \brief destructor
    */
    ~MicronToSTDF();


    /**
     * \fn static bool	IsCompatible(const QString &FileName)
     * \brief static function called by the import all to check if the input file is compatible with the
     *         Micron format.
     * \param FileName is the input file, contains the Micron ascii file.
     * \return true if the input file is compatible with the Micron Micron format. Otherwise return false.
     */
    static bool IsCompatible(const QString &FileName);

    /**
     * \fn bool CheckMandatoryInputFiles(const QStringList& lInputFiles) const
     * \brief Check all mandatory input files exist in the given files list.
     * \param lInputFiles   List of input files
     * \return true if the list contains all mandatory files. Otherwise return false.
     */
    static bool CheckMandatoryInputFiles(const QStringList& lInputFiles);

    bool        IsCompressedFormat() const;

protected:
    /**
     * \fn std::string GetErrorMessage()
     * \brief Getter
     * \return The error message corresponding to the given error code
     */
     std::string GetErrorMessage(const int ErrorCode) const;

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
    bool WriteStdfFile(const QString &StdfFileName);


    /**
     * \fn bool ReadSummary
     * \brief Read the summary section
     * \param is the input file.
     * \param lSBRRecordList the list of the found SBR
     * \param lHBRRecordList the map of the found <HBR_Number, HBR_record>
     * \return true if the read finish with success.
     */
    bool ReadSummary(const QString &fileName,
                     QList<GQTL_STDF::Stdf_SBR_V4 *> &lSBRRecordList,
                     QMap<unsigned short, GQTL_STDF::Stdf_HBR_V4 *> &lHBRRecordList);



    bool ReadTestsDefinition(const QString &inputTestsDefFile,
                             ParameterDictionary &parameterDirectory,
                             QList< QPair<unsigned int, QString> >& testList,
                             const QString keyWord);

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
    bool ReadDieData(QString fileName,
                     QHash<QPair<qint16, qint16>, QStringList> &dieData,
                     bool writeRecord,
                     const QString keyWord,
                     GS::StdLib::Stdf& lStdfFile,
                     QList< QPair<unsigned int, QString> >& testList);


    /**
     * \fn bool ReadMaps
     * \brief Read the G85 map
     * \param inputFile is the input file.
     * \param lPRRRecordsList a list pf found PRR
     * \param passHBin: a list to contains the pass bins
     * \return true if the read finish with success.
     */
//    bool ReadG85Maps(QTextStream &inputFile,
//                     QList<GQTL_STDF::Stdf_PRR_V4 *> &lPRRRecordsList,
//                     QList<short> &passHBin);


    QHash<QString, unsigned short> mHbin;
    QHash<QString, unsigned short> mSbin;
	QMap<QString, QString>      	mInputFiles;        /// \param List of input files
};
}
}

#endif
