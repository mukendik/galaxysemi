#ifdef GCORE15334

#include <QTextStream>
#include <QFile>
#include <QSharedPointer>
#include <QThread>
#include <QElapsedTimer>
#include <gqtl_log.h>

#include "station.h"
#include "pat_outlier_finder_ft.h"
#include "pat_info.h"
#include "pat_options.h"
#include "pat_recipe_io.h"

#define GTM_RECIPE_VERSION_SUPPORTED    1.0f

//#include "DebugMemory.h" // must be the last include


// Constructor: Tester Station data
CStation::CStation(QObject* parent):QObject(parent), mPatInfo(0)
{
    GSLOG(SYSLOG_SEV_NOTICE, "new Station...");

    SessionDate = QDateTime::currentDateTime();
    LastHandshake = QDateTime::currentDateTime();

    // Set station status
    mStationStatus = STATION_ENABLED;

    // Create PAT structurs that will hold all receipe info, limits, etc...and used by PAT libraries
    //GSLOG(5, "new CPatInfo...");
    mPatInfo = new CPatInfo(this);

    Clear();
}

///////////////////////////////////////////////////////////
// Destructor: Tester station data
CStation::~CStation()
{
    GSLOG(5, QString("Destroy Station %1 parent=%2...").arg(mStationInfo.iStation).arg(this->parent()->objectName())
          .toLatin1().data() );

    if(mPatInfo)
    {
        delete mPatInfo;
        mPatInfo = NULL;
    }
}

CPatInfo *CStation::GetPatInfo()
{
    return mPatInfo;
}

COptionsPat &CStation::GetPatOptions()
{
    QMutexLocker lLocker(&mMutex);
    return mPatInfo->GetRecipeOptions();
}

unsigned int CStation::GetPatDefinitionsCount()
{
    return mPatInfo->GetUnivariateRules().count();
}

tdPATDefinitions &CStation::GetPatDefinitions()
{
    return mPatInfo->GetUnivariateRules();
}

bool CStation::ReadPatRecipe(const QString &RecipeFile, QString &ErrorMessage)
{
    GSLOG(SYSLOG_SEV_NOTICE, QString("Read Pat Recipe from file '%1'").arg(RecipeFile).toLatin1().data() );

    // Check if file name provided
    if(RecipeFile.isEmpty() == true)
    {
        ErrorMessage = "*PAT Error* No recipe file defined.";
        return false;
    }

    // Check if file exists
    if(QFile::exists(RecipeFile) == false)
    {
        ErrorMessage = "*PAT Error* Recipe file not found: " + RecipeFile;
        return false;
    }

    // Open PAT recipe file
    QFile file;
    file.setFileName(RecipeFile);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        ErrorMessage = QString("Error opening recipe file %1.").arg(RecipeFile);
        return false;
    }
    QString lRecipeString = file.readAll();
    file.close();

    return ReadPatRecipe(lRecipeString.toLatin1().constData(), ErrorMessage);
}

bool CStation::ReadPatRecipe(const char * RecipeBuffer, QString &ErrorMessage)
{
    GSLOG(SYSLOG_SEV_NOTICE, QString("Read Pat Recipe from buffer").toLatin1().data() );

    // Check if buffer not empty
    if(!RecipeBuffer || (strlen(RecipeBuffer)==0))
    {
        ErrorMessage = "*PAT Error* NULL or empty recipe buffer provided.";
        return false;
    }

    // Read recipe
    QString lRecipeString = RecipeBuffer;
    QTextStream lRecipeStream(&lRecipeString, QIODevice::ReadOnly);
    QSharedPointer<GS::Gex::PATRecipeIO> lRecipeIO(GS::Gex::PATRecipeIO::CreateRecipeIo(lRecipeStream));
    if (lRecipeIO.isNull())
    {
        ErrorMessage = "Bad recipe: Cannot find a matching RecipeIO for this recipe";
        return false;
    }

    lRecipeIO->SetReadOption("TESTNAME_REMOVE_SEQUENCER");
    if (lRecipeIO->Read(RecipeBuffer, mPatInfo->GetRecipe()) == false)
    {
        ErrorMessage = "Bad recipe: " + lRecipeIO->GetErrorMessage();
        return false;
    }

    if (mPatInfo->GetRecipeOptions().GetRecipeType() != GS::Gex::PAT::RecipeFinalTest)
    {
        ErrorMessage = "Bad recipe: ";
        switch(mPatInfo->GetRecipeOptions().GetRecipeType())
        {
            default:
            case GS::Gex::PAT::RecipeUnknown:
                ErrorMessage += "Unknown type";
                break;

            case GS::Gex::PAT::RecipeWaferSort:
                ErrorMessage += "Wafer Sort type";
                break;
        }

        ErrorMessage += " found instead of Final Test type.";
        return false;
    }

#if 0
    // Do not check recipe version.
    // Accept all recipes successfully read by the recipeIO (including legacy CSV recipes, provided they specify
    // FT testing stage, which is checked above.
    if (mPatInfo->cOptionsPat.GetRecipeVersion().isEmpty())
    {
        ErrorMessage = "Bad recipe: Recipe format is too old. No format versioning found inside.";
        return false;
    }
    else
    {
        bool    lSucceed        = true;
        float   lRecipeVersion  = mPatInfo->cOptionsPat.GetRecipeVersion().toFloat(&lSucceed);

        if (lRecipeVersion != GTM_RECIPE_VERSION_SUPPORTED)
        {
            ErrorMessage = QString("Bad recipe: Recipe format %1 is not supported.").arg(lRecipeVersion);
            ErrorMessage += QString(" Only version %1 is supported").arg(GTM_RECIPE_VERSION_SUPPORTED);
            return false;
        }
    }
#endif

    return true;
}

bool CStation::ComputeDynamicPatLimits(QList<int> & SitesToCompute, QList<int> & SitesToUse)
{
    if (SitesToCompute.isEmpty())
        return false;

    QElapsedTimer lElTimer;
    lElTimer.start();

    GS::Gex::PATOutlierFinderFT lPATEngine(mPatInfo, &cTestData.mSites);

    bool lResult=false;
    if(SitesToUse.isEmpty())
    {
        QString lSites;
        foreach (int s, SitesToCompute) { lSites.append(QString::number(s)+" "); }
        GSLOG(SYSLOG_SEV_NOTICE, QString("ComputeDynamicPatLimits for station %1 (%2, sublot %3) on %4 site(s): %5...")
              .arg(mStationInfo.iStation)
              .arg( Get(CStationInfo::sTesterNamePropName).toString() )
              .arg( Get(CStationInfo::sSublotPropName).toString() )
              .arg(SitesToCompute.count())
              .arg(lSites)
              .toLatin1().constData() );
        lResult=lPATEngine.ComputePatLimits(SitesToCompute, false);
    }
    else
    {
        GSLOG(SYSLOG_SEV_NOTICE, QString("ComputeDynamicPatLimits for station %1 on %2 sites using %3 sites...")
              .arg(mStationInfo.iStation).arg(SitesToCompute.count()).arg(SitesToUse.count()).toLatin1().constData());
        lResult=lPATEngine.ComputeMultiSiteDynamicLimits(SitesToCompute, SitesToUse);
    }
    GSLOG(SYSLOG_SEV_NOTICE,
          QString("ComputeDynamicPatLimits done in %1 msecs").arg(lElTimer.elapsed()).toLatin1().data() );
    return lResult;
}

bool CStation::ComputeDynamicPatLimits(GS::Gex::SiteTestResults* Site)
{
    // Create a list of site numbers with just specified site
    QList<int>  lSitesToCompute;
    QList<int>  lSitesToUse;
    lSitesToCompute.append(Site->SiteNb());

    return ComputeDynamicPatLimits(lSitesToCompute, lSitesToUse);
}

bool CStation::ComputeTestStats(GS::Gex::SiteTestResults* Site)
{
    //GSLOG(7, "ComputeTestStats...");

    QList<int>  lSiteNumbersList;
    lSiteNumbersList.append(Site->SiteNb());

    GS::Gex::PATOutlierFinderFT lPATEngine(mPatInfo, &cTestData.mSites);

    return lPATEngine.ComputeTestStats(lSiteNumbersList);
}

QVariant CStation::Get(const char* lPropName)
{
    QMutexLocker lML(&mMutex);
    return mStationInfo.property(lPropName);
}

CStation::stationStatus CStation::GetStationStatus()
{
    return mStationStatus;
} // 6935: get station status

QString CStation::SetInfo(PT_GNM_Q_INIT pMsg_Q_INIT)
{
    if (!pMsg_Q_INIT)
        return "error: message null";

    QMutexLocker lML(&mMutex);

    mStationInfo.iStation	= pMsg_Q_INIT->mStationNb;			// Station#
    mStationInfo.setProperty(CStationInfo::sTesterNamePropName, QString(pMsg_Q_INIT->mNodeName) );			// Tester name
    mStationInfo.setProperty(CStationInfo::sTesterTypePropName,	QString(pMsg_Q_INIT->mTesterType) );         // Tester type
    mStationInfo.setProperty(CStationInfo::sJobNamePropName, QString(pMsg_Q_INIT->mTestJobName) ); // Test program name

    return "ok";
}

QString CStation::SetProdInfo(PT_GNM_PRODINFO pMsg_PRODINFO)
{
    if (!pMsg_PRODINFO)
        return "error: prodinfo null";

    QMutexLocker lML(&mMutex);

    GSLOG(5, QString("Setting prod info: SplitlotID=%1 ...").arg(pMsg_PRODINFO->mSplitlotID).toLatin1().data() );

    mStationInfo.setProperty(CStationInfo::sProductPropName, QString(pMsg_PRODINFO->szProductID) ); // Product name
    mStationInfo.setProperty(CStationInfo::sLotPropName, QString(pMsg_PRODINFO->szLotID) ); // Lot ID
    mStationInfo.setProperty(CStationInfo::sSublotPropName, QString(pMsg_PRODINFO->szSublotID) ); // Sublot ID
    mStationInfo.setProperty(CStationInfo::sJobRevPropName, QString(pMsg_PRODINFO->szJobRevision) ); // Job Revision
    mStationInfo.setProperty(CStationInfo::sOperatorPropName, QString(pMsg_PRODINFO->szOperatorName) );	// Operator
    if (pMsg_PRODINFO->mSplitlotID<1)
      GSLOG(4, QString("Illegal/abnormal splitlot ID %1").arg(pMsg_PRODINFO->mSplitlotID).toLatin1().data() );
    mStationInfo.setProperty(CStationInfo::sSplitlotIDPropName, QVariant(pMsg_PRODINFO->mSplitlotID));
    mStationInfo.setProperty(CStationInfo::sRetestIndexPropName, QVariant(pMsg_PRODINFO->mRetestIndex));

    return "ok";
}

void CStation::SetStationStatus(CStation::stationStatus lStatus)
{
    mStationStatus=lStatus;
}

CPatDefinition *CStation::GetPatDefinition(long lTestNumber, long lPinmapIndex,
                                           const QString& lTestName)
{
    QMutexLocker lML(&mMutex); // This locker is not responsible for GUI slowdown
    //if (QThread::currentThread()==GUIThread) && mMutex.tryLock()==false)
      //  return 0; // do not slow down GUI process
    return mPatInfo->GetPatDefinition(lTestNumber, lPinmapIndex, lTestName);
}

///////////////////////////////////////////////////////////
// Holds Details about the station & Lot under test

const char* CStationInfo::sProductPropName=(char*)"Product";
const char* CStationInfo::sLotPropName=(char*)"Lot";
const char* CStationInfo::sSublotPropName=(char*)"Sublot";
const char* CStationInfo::sSplitlotIDPropName=(char*)"SplitlotID";
const char* CStationInfo::sJobNamePropName=(char*)"JobName";
const char* CStationInfo::sJobRevPropName=(char*)"JobRev";
const char* CStationInfo::sTesterNamePropName=(char*)"TesterName";
const char* CStationInfo::sTesterTypePropName=(char*)"TesterType";
const char* CStationInfo::sRetestIndexPropName=(char*)"RetestIndex";
const char* CStationInfo::sLotStartedPropName=(char*)"LotStarted";
const char* CStationInfo::sOperatorPropName=(char*)"Operator";

CStationInfo::CStationInfo()
{
    clear();
}

///////////////////////////////////////////////////////////
// Holds Details about the station & Lot under test
void CStationInfo::clear(bool bAllVariables/*=true*/)
{
    QMutexLocker lML(&mMutex);
    GSLOG(5, "StationInfo clear...");
    setProperty(sSublotPropName, "-");// Sublot
    setProperty(sLotStartedPropName, QDateTime::currentDateTime()); //tLotStarted=QDateTime::currentDateTime();

    // Fields not cleared if called from a end-of-lot signal!
    if(bAllVariables)
    {
        iStation=0;	// client Station#
        setProperty(sLotPropName, "-");
        setProperty(sTesterNamePropName, "-"); // Client Tester name
        setProperty(sTesterTypePropName, "-");	// Client Tester type
        setProperty(sJobNamePropName, "-");	// Test program running at station
        setProperty(sJobRevPropName, "-");	// Test program job revision
        setProperty(sProductPropName, "-");	// Program running at station
        setProperty(sOperatorPropName, "-"); // Operator logged-in
    }
}

void CStation::ClearInfo(bool lAllVariables)
{
    mStationInfo.clear(lAllVariables);
}

void CStation::Clear()
{
    GSLOG(5, "Clear station...");
    // Resets PAT buffers.
    if (mPatInfo)
        mPatInfo->clear();
}
#endif
