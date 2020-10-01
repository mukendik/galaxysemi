#include "vfei_config.h"
#include "gqtl_log.h"

#include <QTextStream>
#include <QFile>

namespace GS
{
namespace Gex
{

VFEIConfig::VFEIConfig()
{
    Clear();
}

VFEIConfig::VFEIConfig(const VFEIConfig &config)
{
    *this = config;
}

VFEIConfig::~VFEIConfig()
{

}

VFEIConfig &VFEIConfig::operator =(const VFEIConfig &config)
{
    if (this != &config)
    {
        mSocketPort             = config.mSocketPort;
        mMaxAllowedInstances    = config.mMaxAllowedInstances;
        mDeleteInput            = config.mDeleteInput;
        mProdRecipeFolder       = config.mProdRecipeFolder;
        mEngRecipeFolder        = config.mEngRecipeFolder;
        mInputSTDFFolder        = config.mInputSTDFFolder;
        mInputMapFolder         = config.mInputMapFolder;
        mOutputSTDFFolder       = config.mOutputSTDFFolder;
        mOutputMapFolder        = config.mOutputMapFolder;
        mOutputReportFolder     = config.mOutputReportFolder;
    }

    return *this;
}

bool VFEIConfig::LoadFromFile(const QString &lFileName, QString& lErrorMessage)
{
    // Check the file exists and can be opened
    QFile file(lFileName);
    if (file.open(QIODevice::ReadOnly) == false)
    {
        lErrorMessage = "Failed to open VFEI config file: " + lFileName;
        // Failed opening VFEI configuration file
        return false;
    }

    // Clear some parameters
    Clear();

    // Trace info
    QTextStream lTxtStream(&file);
    QString     lLine;
    QString     lKeyword;
    QString     lValue;
    int         lLineNumber = 0;
    bool        lOk         = false;

    GSLOG(SYSLOG_SEV_INFORMATIONAL,
           QString("Reading VFEI config file: %1").arg(lFileName).toLatin1().constData());

    do
    {
        // Read line
        lLine = lTxtStream.readLine().trimmed();

        // Keep track of line#
        lLineNumber++;

        if(lLine.isEmpty() || lLine.startsWith("#") || lLine.startsWith(";"))
            goto next_line;

        lKeyword    = lLine.section("=", 0, 0);
        lValue      = lLine.section("=", 1);

        if(lKeyword.compare("Port",Qt::CaseInsensitive) == 0)
        {
            mSocketPort = lValue.toInt(&lOk);

            if(!lOk || mSocketPort < 1)
            {
                lErrorMessage = "Failed parsing 'Port' value.";
                lErrorMessage += " (Line# " + QString::number(lLineNumber) + ")";
                return false;
            }
        }
        else
        if(lKeyword.compare("MaxInstances",Qt::CaseInsensitive) == 0)
        {
            mMaxAllowedInstances = lValue.toInt(&lOk);
            if(!lOk || (mMaxAllowedInstances < 1))
            {
                lErrorMessage = "Failed parsing 'MaxInstances' value.";
                lErrorMessage += " (Line# " + QString::number(lLineNumber) + ")";
                return false;
            }
        }
        else
        // # Location holding production recipes
        if(lKeyword.compare("ProdRecipeFolder",Qt::CaseInsensitive) == 0)
        {
            // Ensure folder name doesn't ends with a / or "\\"
            if(lValue.endsWith("/") || lValue.endsWith("\\"))
                lValue.truncate(lValue.length()-1);

            // Recipe path
            mProdRecipeFolder = lValue;

            // Check if folder exists
            if(QFile::exists(mProdRecipeFolder) == false)
            {
                lErrorMessage = "'ProdRecipeFolder' path specified doesn't exist.";
                lErrorMessage += " (Line# " + QString::number(lLineNumber) + ")";
                return false;
            }
        }
        else
        // # Location holding Engineering recipes
        if(lKeyword.compare("EngRecipeFolder",Qt::CaseInsensitive) == 0)
        {
            // Ensure folder name doesn't ends with a / or "\\"
            if(lValue.endsWith("/") || lValue.endsWith("\\"))
                lValue.truncate(lValue.length()-1);

            // Recipe path
            mEngRecipeFolder = lValue;

            // Check if folder exists
            if(QFile::exists(mEngRecipeFolder) == false)
            {
                lErrorMessage = "'EngRecipeFolder' path specified doesn't exist.";
                lErrorMessage += " (Line# " + QString::number(lLineNumber) + ")";
                return false;
            }
        }
        else
        // # Defines where Input (STDF) files are stored
        if(lKeyword.compare("InputStdfFolder",Qt::CaseInsensitive) == 0)
        {
            // Ensure folder name doesn't ends with a / or "\\"
            if(lValue.endsWith("/") || lValue.endsWith("\\"))
                lValue.truncate(lValue.length()-1);

            // Recipe path
            mInputSTDFFolder = lValue;

            // Check if folder exists
            if(QFile::exists(mInputSTDFFolder) == false)
            {
                lErrorMessage = "'InputStdfFolder' path specified doesn't exist.";
                lErrorMessage += " (Line# " + QString::number(lLineNumber) + ")";
                return false;
            }
        }
        else
        // # Defines where Input (MAP) files are stored
        if(lKeyword.compare("InputMapFolder",Qt::CaseInsensitive) == 0)
        {
            // Ensure folder name doesn't ends with a / or "\\"
            if(lValue.endsWith("/") || lValue.endsWith("\\"))
                lValue.truncate(lValue.length()-1);

            // Recipe path
            mInputMapFolder = lValue;

            // Check if folder exists
            if(QFile::exists(mInputMapFolder) == false)
            {
                lErrorMessage = "'InputMapFolder' path specified doesn't exist.";
                lErrorMessage += " (Line# " + QString::number(lLineNumber) + ")";
                return false;
            }
        }
        else
        // # Defines if Input files to be erased after processing
        if(lKeyword.compare("DeleteInput",Qt::CaseInsensitive) == 0)
        {
            // Delete input files?
            mDeleteInput = (lValue.toInt(&lOk) == 1);
            if(!lOk)
            {
                lErrorMessage = "Failed parsing 'DeleteInput' value.";
                lErrorMessage += " (Line# " + QString::number(lLineNumber) + ")";
                return false;
            }
        }
        else
        // # Defines where output STDF files are stored
        if(lKeyword.compare("OutputStdfFolder",Qt::CaseInsensitive) == 0)
        {
            // Ensure folder name doesn't ends with a / or "\\"
            if(lValue.endsWith("/") || lValue.endsWith("\\"))
                lValue.truncate(lValue.length()-1);

            // Output path
            mOutputSTDFFolder = lValue;

            // Check if folder exists
            if(QFile::exists(mOutputSTDFFolder) == false)
            {
                lErrorMessage = "'OutputStdfFolder' path specified doesn't exist.";
                lErrorMessage += " (Line# " + QString::number(lLineNumber) + ")";
                return false;
            }
        }
        else
        // # Defines where output MAP files are stored
        if(lKeyword.compare("OutputMapFolder",Qt::CaseInsensitive) == 0)
        {
            // Ensure folder name doesn't ends with a / or "\\"
            if(lValue.endsWith("/") || lValue.endsWith("\\"))
                lValue.truncate(lValue.length()-1);

            // Output path
            mOutputMapFolder = lValue;

            // Check if folder exists
            if(QFile::exists(mOutputMapFolder) == false)
            {
                lErrorMessage = "'OutputMapFolder' path specified doesn't exist.";
                lErrorMessage += " (Line# " + QString::number(lLineNumber) + ")";
                return false;
            }
        }
        else
        // # Defines where output REPORT files are stored
        if(lKeyword.compare("OutputReportFolder",Qt::CaseInsensitive) == 0)
        {
            // Ensure folder name doesn't ends with a / or "\\"
            if(lValue.endsWith("/") || lValue.endsWith("\\"))
                lValue.truncate(lValue.length()-1);

            // Output path
            mOutputReportFolder = lValue;

            // Check if folder exists
            if(QFile::exists(mOutputReportFolder) == false)
            {
                lErrorMessage = "'OutputReportFolder' path specified doesn't exist.";
                lErrorMessage += " (Line# " + QString::number(lLineNumber) + ")";
                return false;
            }
        }
        else
        // # Defines where output files created moved on error situation.
        if(lKeyword.compare("ErrorFolder",Qt::CaseInsensitive) == 0)
        {
            // Ensure folder name doesn't ends with a / or "\\"
            if(lValue.endsWith("/") || lValue.endsWith("\\"))
                lValue.truncate(lValue.length()-1);

            // Error path
            mErrorFolder = lValue;

            // Check if folder exists
            if(QFile::exists(mErrorFolder) == false)
            {
                lErrorMessage = "'ErrorFolder' path specified doesn't exist.";
                lErrorMessage += " (Line# " + QString::number(lLineNumber) + ")";
                return false;
            }
        }
        else
        {
            // Unknown line
            lErrorMessage = "invalid/unknown string: " + lLine;
            lErrorMessage += " (Line# " + QString::number(lLineNumber) + ")";
            return false;
        }
next_line:;
    }
    while(lTxtStream.atEnd() == false);

    return IsValid(lErrorMessage);
}

bool VFEIConfig::IsValid(QString &lErrorMessage) const
{
    // Check if all parameters defined
    if(mSocketPort <= 0)
    {
        // Socket port# missing
        lErrorMessage = "Invalid/missing 'Port' field";
        return false;
    }
    if(mMaxAllowedInstances <= 0)
    {
        // Maximum concurrent instances allowed
        lErrorMessage = "Invalid/missing 'MaxInstances' field";
        return false;
    }
    if(mProdRecipeFolder.isEmpty())
    {
        // Folder definition
        lErrorMessage = "Invalid/missing 'ProdRecipeFolder' field";
        return false;
    }
    if(mEngRecipeFolder.isEmpty())
    {
        // Folder definition
        lErrorMessage = "Invalid/missing 'EngRecipeFolder' field";
        return false;
    }
    if(mInputSTDFFolder.isEmpty())
    {
        // Folder definition
        lErrorMessage = "Invalid/missing 'InputStdfFolder' field";
        return false;
    }
    if(mInputMapFolder.isEmpty())
    {
        // Folder definition
        lErrorMessage = "Invalid/missing 'InputMapFolder' field";
        return false;
    }
    if(mOutputSTDFFolder.isEmpty())
    {
        // Folder definition
        lErrorMessage = "Invalid/missing 'OutputStdfFolder' field";
        return false;
    }
    if(mOutputMapFolder.isEmpty())
    {
        // Folder definition
        lErrorMessage = "Invalid/missing 'OutputMapFolder' field";
        return false;
    }
    if(mOutputReportFolder.isEmpty())
    {
        // Folder definition
        lErrorMessage = "Invalid/missing 'OutputReportFolder' field";
        return false;
    }
    if(mErrorFolder.isEmpty())
    {
        // Folder definition
        lErrorMessage = "Invalid/missing 'ErrorFolder' field";
        return false;
    }

    return true;
}

int VFEIConfig::GetSocketPort() const
{
    return mSocketPort;
}

int VFEIConfig::GetMaxAllowedInstances() const
{
    return mMaxAllowedInstances;
}

bool VFEIConfig::GetDeleteInput() const
{
    return mDeleteInput;
}

const QString &VFEIConfig::GetProdRecipeFolder() const
{
    return mProdRecipeFolder;
}

const QString &VFEIConfig::GetEngRecipeFolder() const
{
    return mEngRecipeFolder;
}

const QString &VFEIConfig::GetInputSTDFFolder() const
{
    return mInputSTDFFolder;
}

const QString &VFEIConfig::GetInputMapFolder() const
{
    return mInputMapFolder;
}

const QString &VFEIConfig::GetOutputSTDFFolder() const
{
    return mOutputSTDFFolder;
}

const QString &VFEIConfig::GetOutputMapFolder() const
{
    return mOutputMapFolder;
}

const QString &VFEIConfig::GetOutputReportFolder() const
{
    return mOutputReportFolder;
}

const QString &VFEIConfig::GetErrorFolder() const
{
    return mErrorFolder;
}

void VFEIConfig::Clear()
{
    mSocketPort             = -1;		// Socket port#
    mMaxAllowedInstances    = -1;		// Maximum concurrent instances allowed
    mEngRecipeFolder        = "";       // Engineering recipe location
    mProdRecipeFolder       = "";       // Production recipe location
    mInputSTDFFolder        = "";
    mInputMapFolder         = "";
    mOutputSTDFFolder       = "";
    mOutputMapFolder        = "";
    mOutputReportFolder     = "";
    mErrorFolder            = "";
    mDeleteInput            = false;
}

}   // namespace Gex
}   // namespace GS
