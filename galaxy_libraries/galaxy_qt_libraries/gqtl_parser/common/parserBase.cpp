
#include <gqtl_log.h>
#include <QFileInfo>
#include <QCoreApplication>

#include "parserBase.h"
#include "converter_external_file.h"

namespace GS
{
namespace Parser
{

ParserBase::ParserBase(ParserType lType, const QString &lName)
    : ParserAbstract(), mType(lType), mName(lName)
{
    mProgressStatus = 0;
    mOutputFiles.clear();
    mDefaultBinNumber = 999;
    mDefaultBinSet = false;
    mExternalFilePath = "";	/// xml file containing bin map file path and promis file path
    mBinMapFilePath = "";
    mPromisFilePath = "";
}

ParserBase::~ParserBase()
{
}

bool ParserBase::getOptDefaultBinValuesfromExternalFile(const QString& aFile, QString& aStrErrorMsg)
{
    QFileInfo aFileInfo(aFile);
    QStringList lFormatList = ConverterExternalFile::LoadOptionnalDefaultVals(aFileInfo.path(), aStrErrorMsg);

    if(!aStrErrorMsg.isEmpty())
    {
        mLastError= errReadBinMapFile;
        return true;
    }

    //reads optionnal default values for bin name and bin number, if present in ext file
    if(lFormatList.size() == 2 && lFormatList[0].size() > 0 && lFormatList[1].size() > 0)
    {
        bool lOK(false);
        double lDefaultNumber = lFormatList[1].toDouble(&lOK);
        if (!lOK || (lDefaultNumber < 0) || (lDefaultNumber > 32767))
        {
            mLastError= errReadBinMapFile;
            aStrErrorMsg = QString("Invalid default Bin Number %1 in the external file: %2")
                    .arg(lFormatList[1])
                    .arg(ConverterExternalFile::GetExternalFileName(aFileInfo.path()));
            GSLOG(SYSLOG_SEV_ERROR, aStrErrorMsg.toLatin1().constData());
            return false;
        }
        mDefaultBinNumber = static_cast<stdf_type_u2>(lDefaultNumber);
        mDefaultBinName = lFormatList[0];
        mDefaultBinSet = true;
    }
    else
    {
        GSLOG(SYSLOG_SEV_INFORMATIONAL,
              QString("No default bin values found in external file %1. ").arg(aFile).toLatin1().constData());
    }

    return true;
}

ParserType ParserBase::GetType() const
{
    return mType;
}

std::string ParserBase::GetName() const
{
    return mName.toStdString();
}

const std::list<std::string>& ParserBase::GetExtensions() const
{
    if(mSTDExtensions.empty())
{

    for (int lItem = 0; lItem < mExtension.count(); ++lItem)
            mSTDExtensions.push_back(mExtension.at(lItem).toStdString());
    }

    return mSTDExtensions;
}

bool ParserBase::IsCompressedFormat() const
{
    return false;
}

//std::list<std::string> ParserBase::GetListStdfFiles() const
//{
//    return std::list<std::string>();
//}



bool ParserBase::Convert(const QString &originalFileName, QString &stdfFileName)
{
    // No erro (default)
    mLastError = errNoError;
    mLastErrorMessage = "";

    // If STDF file already exists...do not rebuild it...unless dates not matching!
    QFileInfo fInput(originalFileName);
    QFileInfo fOutput(stdfFileName);

    QFile f( stdfFileName );
    if((f.exists() == true) && (fInput.lastModified() < fOutput.lastModified()))
        return true;

    //////////////////////////////////////////////////////////////////////
    // For ProgressBar

    StartProgressStatus(fInput.size() + 1, QString("Converting data from file "+QFileInfo(originalFileName).fileName()+"...") );

    bool lRes = ConvertoStdf(originalFileName, stdfFileName) ;

    StopProgressBar();

    return lRes;
}


QString ParserBase::ReadLine(QTextStream& hFile)
{
    QString strString;
    do
    {
        strString = hFile.readLine();
        SpecificReadLine(strString);
    }
    while(!strString.isNull() && strString.isEmpty() && !hFile.atEnd());
    UpdateProgressBar(hFile);

    return strString;

}

void ParserBase::StopProgressBar()
{
    if (mProgressStatus)
        mProgressStatus->Finish();
}

void ParserBase::UpdateProgressMessage(const QString &message)
{
    if (mProgressStatus != 0)
    {
       mProgressStatus->SetMessage(message.toStdString());
       QCoreApplication::processEvents();
    }
}

void ParserBase::StartProgressStatus(const long totalSize, const QString &message)
{
    mProgressStep   = 0;
    mNextFilePos    = 0;
    mTotalSize      = totalSize;

    if (mProgressStatus != 0)
    {
        mProgressStatus->Start(0, 100);
        mProgressStatus->SetMessage(message.toStdString());
    }
}

///
/// \brief ParserBase::UpdateProgressBar for write
///
void ParserBase::UpdateProgressBar()
{
    if(mProgressStatus != NULL && mTotalSize > 0)
    {
        mProgressStep += 100/mTotalSize + 1;
        mProgressStatus->SetValue(mProgressStep);
    }
}

///
/// \brief ParserBase::UpdateProgressBar for read
///
void ParserBase::UpdateProgressBar(const QTextStream& hFile)
{
    if(mProgressStatus != NULL && mTotalSize > 0)
    {
        while((int)hFile.device()->pos() > mNextFilePos)
        {
            mProgressStep += 100/mTotalSize + 1;
            mNextFilePos  += mTotalSize/100 + 1;
            mProgressStatus->SetValue(mProgressStep);
        }
    }
}

int ParserBase::GetLastError(std::string & ErrorMsg) const
{
    ErrorMsg = GetName();
    ErrorMsg += " - ";
    ErrorMsg += GetErrorMessage(mLastError);

    return mLastError;
}

std::string ParserBase::GetErrorMessage(const int ErrorCode) const
{
    QString lErrorMsg;
    switch(ErrorCode)
    {
        case errNoError:
            lErrorMsg += "No Error";
            break;
        case errOpenFail:
            lErrorMsg += "Failed to open file";
            break;
        case errInvalidFormatParameter:
            lErrorMsg += "Invalid file format";
            break;
        case errMissingData:
            lErrorMsg += "Missing mandatory info";
            break;
        case errWriteSTDF:
            lErrorMsg += "Failed creating temporary file";
            break;
        case errLicenceExpired:
            lErrorMsg += "License has expired or Data file out of date";
            break;
        case errInvalidExpirationDate:
            lErrorMsg += "Invalid expiration date provided to parser";
            break;
        case errMissingConvertFunction:
            lErrorMsg += "Virtual convert function is not implemented for this parser";
            break;
        case errReadPromisFile:
            lErrorMsg += "Failed reading PROMIS data file";
           break;
        case errReadBinMapFile:
            lErrorMsg += "Failed reading BIN mapping file";
           break;
        case errConverterExternalFile:
            lErrorMsg += "Failed loading converter external file";
           break;
        default:
        // Other issue from specific parsers
            lErrorMsg += "Unexpected Error";
            break;
    }

    if(!mLastErrorMessage.isEmpty())
        lErrorMsg += ": "+mLastErrorMessage;

    return lErrorMsg.toStdString();
}


void ParserBase::SetProgressHandler(ProgressHandlerAbstract* progress)
{
    mProgressStatus = progress;
}

void ParserBase::SetParameterFolder(const std::string &parameterFolder)
{
    mParameterFolder = QString::fromStdString(parameterFolder);
    mParameterDirectory.SetRepositoryName(mParameterFolder);
}

//void ParserBase::SetLastError(const QString error)
//{
//    mLastError = error;
//}

ConverterStatus ParserBase::Convert(const std::string &fileName, std::string &stdfFileName)
{
    // BG: For the moment, this method can return only ConvertSuccess or ConvertError, as the underlying
    // Convert() function returns a boolean (true/false).
    // The underlying Convert() function must be modified if we want to be able to also return ConvertWarning.
    // This being said, I think we should only return ConvertSuccess or ConvertError, and include a Warning stack
    // with all encountered warnings.
    bool lStatus;

    QString lStdfFile = stdfFileName.c_str();
    // Convert
    lStatus = Convert(QString(fileName.c_str()), lStdfFile);

    // retrieve the sdfFile name that may have been updated
    stdfFileName = lStdfFile.toStdString();

    // Check status
    return (lStatus==true ? ConvertSuccess:ConvertError);
}


bool ParserBase::RemoveOutputFiles()
{
    for(unsigned short i=0; i<mOutputFiles.size(); ++i)
    {
        QString lFile = mOutputFiles[i];
        if (!QFile::remove(lFile))
        {
            GSLOG(SYSLOG_SEV_ERROR, QString("Impossible to remove the output file %1")
                  .arg(lFile).toLatin1().constData());
            return false;
        }
    }
    return true;
}

gsbool ParserBase::AddMetaDataToDTR(const QString& key, const QString& fieldValue, GQTL_STDF::Stdf_DTR_V4* lDTRRecord)
{
    bool lConcat = false;
    QString lSep = "";
    return AddMetaDataToDTR(key, fieldValue, lDTRRecord, lConcat, lSep);
}

gsbool ParserBase::AddMetaDataToDTR(const QString& key,
                                    const QString& fieldValue,
                                    GQTL_STDF::Stdf_DTR_V4* lDTRRecord,
                                    bool aConcat,
                                    const QString &aSep)
{
    QJsonObject            lMetaDataField;
    lMetaDataField.insert("TYPE", QJsonValue(QString("md")));
    if(aConcat)
    {
        lMetaDataField.insert("ACTION", QJsonValue(QString("concat")));
        if(aSep != ",")
            lMetaDataField.insert("SEP", QJsonValue(aSep));
    }
    lMetaDataField.insert("KEY", QJsonValue(key));
    lMetaDataField.insert("VALUE", QJsonValue(fieldValue));
    lDTRRecord->SetTEXT_DAT(lMetaDataField);
    return true;
}

std::list<std::string> ParserBase::GetListStdfFiles() const
{
    std::list<std::string> lOutputFiles;

    for (int lItem=0; lItem < mOutputFiles.count(); ++lItem)
    {
        lOutputFiles.push_back(mOutputFiles[lItem].toStdString());
    }

    return lOutputFiles;
}

QString	ParserBase::appendBinMappingExceptionInfo()
{
    mLastError = errReadBinMapFile;
    return QString(" " + mBinMapFilePath + " specified in " + mExternalFilePath);
}

QString	ParserBase::appendPromisExceptionInfo()
{
    mLastError = errReadPromisFile;
    return QString(" " + mPromisFilePath + " specified in " + mExternalFilePath);
}

}
}
