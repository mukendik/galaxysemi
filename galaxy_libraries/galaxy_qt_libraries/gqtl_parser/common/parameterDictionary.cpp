#include <QTextStream>
#include <QFile>

#include "importConstants.h"
#include "parameterDictionary.h"

namespace GS
{
namespace Parser
{

ParameterDictionary::ParameterDictionary():mFileName(GEX_CSV_PARAMETERS), mNewParameterFound(false)
{
}

ParameterDictionary::ParameterDictionary(const QString& paramRepositoryName):mParamRepositoryName(paramRepositoryName)
{
    mNewParameterFound = false;
    mFileName = GEX_CSV_PARAMETERS;
}

ParameterDictionary::ParameterDictionary(const QString& fileName, const QString& paramRepositoryName):
    mFileName(fileName), mParamRepositoryName(paramRepositoryName)
{
    mNewParameterFound = false;
}


int ParameterDictionary::GetTestNumber(const QString& parameterName) const
{
    return mFullParametersList.indexOf(parameterName);
}

QStringList ParameterDictionary::GetFullParametersList() const
{
    return mFullParametersList;
}

bool ParameterDictionary::GetNewParameterFound() const
{
    return mNewParameterFound;
}


//int ParameterDictionary::InsertParameter(const QString& parameterName)
//{

//}

//////////////////////////////////////////////////////////////////////
// Load CSV Parameter table from DISK
//////////////////////////////////////////////////////////////////////
void ParameterDictionary::LoadParameterIndexTable(void)
{
    QString	lDictionaryFileName;
    QString	lStrString;

//    // The set is done in the class EnginePrivate with Set("UserFolder", mUserFolder);
//    lStrCsvTableFile  = GS::Gex::Engine::GetInstance().Get("UserFolder").toString();
    lDictionaryFileName = mParamRepositoryName + mFileName;

    // Open CSV Parameter table file
    QFile lFile(lDictionaryFileName);
    if(!lFile.open(QIODevice::ReadOnly))
        return;

    // Assign file I/O stream
    QTextStream lDictionaryFile(&lFile);

    // Skip comment lines
    do
    {
        lStrString = lDictionaryFile.readLine();
    }
    while((lStrString.indexOf("----------------------") < 0) && (lDictionaryFile.atEnd() == false));

    // Read lines
    mFullParametersList.clear();
    lStrString = lDictionaryFile.readLine();
    while (lStrString.isNull() == false)
    {
        // Save Parameter name in list
        mFullParametersList.append(lStrString);
        // Read next line
        lStrString = lDictionaryFile.readLine();
    };

    // Close file
    lFile.close();
}

//////////////////////////////////////////////////////////////////////
// Save CSV Parameter table to DISK
//////////////////////////////////////////////////////////////////////
void ParameterDictionary::DumpParameterIndexTable(void)
{
    QString		lDictionaryFileName;
    int			nIndex;

    lDictionaryFileName = mParamRepositoryName + mFileName;

    // Open CSV Parameter table file
    QFile lDictionaryFile(lDictionaryFileName);
    if(!lDictionaryFile.open( QIODevice::WriteOnly ))
        return;

    // Assign file I/O stream
    QTextStream lDictionaryStream(&lDictionaryFile);

    // First few lines are comments:
    lDictionaryStream << "############################################################" << endl;
    lDictionaryStream << "# DO NOT EDIT THIS FILE!" << endl;
    lDictionaryStream << "# Quantix Examinator: Parameters detected" << endl;
    lDictionaryStream << "# www.mentor.com" << endl;
    lDictionaryStream << "# Quantix Examinator reads and writes into this file..." << endl;
    lDictionaryStream << "-----------------------------------------------------------" << endl;

    // Write lines
    // pFullCsvParametersList.sort();
    for(nIndex = 0; nIndex < mFullParametersList.count(); nIndex++)
    {
        // Write line
        lDictionaryStream << mFullParametersList[nIndex] << endl;
    };

    // Close file
    lDictionaryFile.close();
}

//////////////////////////////////////////////////////////////////////
// If Examinator doesn't have this CSV parameter in his dictionnary, have it added.
//////////////////////////////////////////////////////////////////////
long ParameterDictionary::UpdateParameterIndexTable(const QString &paramName)
{
    // Check if the table is empty...if so, load it from disk first!
    if(mFullParametersList.isEmpty() == true)
    {
        // Load CSV parameter table from disk...
        LoadParameterIndexTable();
    }

    int lIndex = mFullParametersList.indexOf(paramName) + 1;

    // Check if Parameter name already in table...if not, add it to the list
    // the new full list will be dumped to the disk at the end.
    if(lIndex < 1)
    {
        // Update list
        mFullParametersList.append(paramName);

        lIndex = mFullParametersList.count();

        // Set flag to force the current table to be updated on disk
        mNewParameterFound = true;
    }

    return lIndex;
}



}
}
