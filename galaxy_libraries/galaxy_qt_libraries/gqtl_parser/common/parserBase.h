#ifndef PARSERBASE_H
#define PARSERBASE_H

#include <QString>
#include <QStringList>
#include <QDate>
#include <QTextStream>

#include "stdfparse.h"
#include "parserAbstract.h"
#include "importConstants.h"
#include "parserBinning.h"
#include "parserParameter.h"
#include "parameterDictionary.h"
#include "gs_types.h"

namespace GS
{
namespace Parser
{

class ParserBase : public ParserAbstract
{
public:
    /**
     * \fn ParserBase(ParserType lType, const QString& lName)
     * \brief Constructor with parameters
     * \param lType : The type of the parser
     *        lName : The name of the parser
     */
    ParserBase(ParserType lType, const QString& lName);
    /**
     * \fn ~ParserBase()
     * \brief Destractor
     */
    virtual ~ParserBase();


    /**
     * \fn ConverterStatus Convert(const char* fileName)
     * \brief Convert the input file
     * \param fileName: Input file name
     * \return The status of the convertion.
     */
    ConverterStatus Convert(const std::string &fileName,  std::string &stdfFileName);

    /**
     * \fn bool Convert(const char *originalFileName, const char *stdfFileName)
     * \brief Convert the input file to stdf format. Check if the input file respects the expiration date
     * \param originalFileName: The original file (input file)
     *        stdfFileName: The output file which is in stdf format
     * \return true if the convertion is done with success, otherwise false.
     */
     bool Convert(const QString &originalFileName, QString &stdfFileName);

    /**
     * \brief Getters
     */
    ParserType                      GetType() const;
    std::string                     GetName() const;
    const std::list<std::string> &  GetExtensions() const;
    virtual bool                    IsCompressedFormat() const;

    /**
     * \fn IsCompatibleMapUpdater Convert()
     * \return Return true if the parser is compatoble with the map updater. (Default = false)
     */
    virtual bool IsCompatibleMapUpdater() const { return false;}


    /**
     * \fn void SetProgressHandler(ProgressHandlerAbstract *progress)
     * \brief Set the progress handler
     * \param progress: The progress handler
     */
    void SetProgressHandler(ProgressHandlerAbstract *progress);

    void SetParameterFolder(const std::string& parameterFolder);

    /**
     * \fn QStringList GetListStdfFiles()
     * \brief Getter
     * \return Returns the list of the list of output files
     */
    std::list<std::string> GetListStdfFiles() const;

    /**
     * \fn int GetLastError(std::string & ErrorMsg)
     * \brief return the last error code and message.
     * \return last error code.
     */
    virtual int     GetLastError(std::string & ErrorMsg) const;


    enum  errCodes
    {
        errNoError,                         /// \param No erro (default)
        errOpenFail,                        /// \param Failed Opening input file
        errMissingData,                     /// \param Missing mandatory info
        errInvalidFormatParameter,          /// \param Invalid input format
        errLicenceExpired,                  /// \param File date out of Licence window!
        errWriteSTDF,                       /// \param Failed creating STDF output file
        errInvalidExpirationDate,           /// \param Invalid expiration date provided
        errMissingConvertFunction,          /// \param Virtual convert function is not implemented for this parser
        errReadPromisFile,                  /// \param Custom PROMIS file defined, but wafer entry missing...
        errReadBinMapFile,                  /// \param Binning map file defined, but error loading it...
        errConverterExternalFile,           /// \param Converter External file not found, but error loading it...
        errParserSpecific           = 100   /// \param Use this one as lower error code for a specific parser
    };

protected:
    /**
     * \fn std::string GetErrorMessage(const int ErrorCode)
     * \brief Getter
     * \return The error message corresponding to the given error code
     */
    virtual std::string GetErrorMessage(const int ErrorCode) const;


    GQTL_STDF::StdfParse        mStdfParse;
    ParameterDictionary         mParameterDirectory;
    QString                     mParameterFolder;   /// \param The folder that will contain the parameter directory
    ParserType                  mType;              /// \param The type of the parser
    gstime                      mStartTime;         /// \param Start time
    int                         mLastError;         /// \param Holds last error ID
    QString                     mLastErrorMessage;  /// \param Holds last error info
    QString						mExternalFilePath;	/// xml file containing bin map file path and promis file path
    QString						mPromisFilePath;
    QString						mBinMapFilePath;
    QString						appendBinMappingExceptionInfo(void);
    QString						appendPromisExceptionInfo(void);

    void            UpdateProgressMessage   (const QString &message);
    void            UpdateProgressBar       (const QTextStream& hFile);
    void            UpdateProgressBar       ();
    void            StartProgressStatus     (const long totalSize, const QString &message);
    void            StopProgressBar         ();


    /**
     * \fn QString ReadLine
     * \brief Read a line from the file and update the progress bar
     */
    QString         ReadLine                (QTextStream& hFile);

    virtual bool    ConvertoStdf            (const QString&, QString&) { return false;}

    /**
     * \fn bool SpecificReadLine
     * \brief To be implemented if a specific traitment is done on the line after calling readLine
     */
    virtual void    SpecificReadLine           (QString &) {}

    QStringList                     mOutputFiles;   /// \param The list of stdf output files

    /**
     * \fn bool RemoveOutputFiles()
     * \brief Remove all files from the stdf output files
     * \return True if the files have been successfully removed.
     */
    bool RemoveOutputFiles();

    /**
     * \fn gsbool AddMetaDataToDTR(QString key, QString fieldValue, GQTL_STDF::Stdf_DTR_V4* lDTRRecord);
     * \brief Add a meta data into a DTR
     * \return true if the the meta data has been correctly writen
     */
    gsbool AddMetaDataToDTR(const QString &key, const QString &fieldValue, GQTL_STDF::Stdf_DTR_V4* lDTRRecord);

    /**
     * \fn gsbool AddMetaDataToDTR(QString key, QString fieldValue, bool Append, QString Sep, GQTL_STDF::Stdf_DTR_V4* lDTRRecord);
     * \brief Add a meta data into a DTR
     * \return true if the the meta data has been correctly writen
     */
    gsbool AddMetaDataToDTR(const QString &key,
                            const QString &fieldValue,
                            GQTL_STDF::Stdf_DTR_V4* lDTRRecord,
                            bool aConcat,
                            const QString &aSep);
    bool getOptDefaultBinValuesfromExternalFile(const QString& aFile, QString& aStrErrorMsg);
    stdf_type_u2       mDefaultBinNumber;
    QString            mDefaultBinName;
    bool               mDefaultBinSet;

private:

    QString                             mName;              /// \param The name of the parser
    QStringList                         mExtension;         /// \param Supported extentions
    ProgressHandlerAbstract*            mProgressStatus;    /// \param The handler of the convertion progess

    mutable std::list<std::string>      mSTDExtensions;     /// \param Supported extensions std style

    // For ProgressBar
    int         mProgressStep;                          /// \param The progress step
    int         mTotalSize;                             /// \param The totl element to process
    qint64      mNextFilePos;                           /// \param The next position in the input file
};

}
}
#endif // PARSERBASE_H
