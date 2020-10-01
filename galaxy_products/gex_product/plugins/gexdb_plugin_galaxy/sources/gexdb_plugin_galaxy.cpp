// gexdb_plugin_galaxy.cpp: implementation of the GexDbPlugin_Galaxy class.
// ----------------------------------------------------------------------------------------------------------
// Copyright Galaxy
// This computer program is protected by copyright law
// and international treaties. Unauthorized reproduction or
// distribution of this program, or any portion of it,may
// result in severe civil and criminal penalties, and will be
// prosecuted to the maximum extent possible under the low.
// ----------------------------------------------------------------------------------------------------------
//
// Notes: defines the entry point for the DLL application.
//
// ----------------------------------------------------------------------------------------------------------

// Local includes
#include "gexdb_plugin_galaxy.h"
#include "gexdb_plugin_option.h"
#include "gexdb_plugin_galaxy_cfgwizard.h"
#include "rdb_options_widget.h"
#include "gex_constants.h"
#include "consolidation_tree.h"
#include <gqtl_log.h>
#include "abstract_query_progress.h"
#include "consolidation_center.h"

// Standard includes

// Qt includes
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QSqlQuery>
#include <QSqlError>
#include <QHostAddress>
/*nclude <qlocale.h>*/

// Galaxy modules includes
#include <stdf.h>
#include <stdfparse.h>
#include <gqtl_sysutils.h>
#include <gqtl_log.h>

////////////////////////////////////////////////////////////////////////////////////
// Constants and Macro definitions
////////////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------------------------------------------
// Only declare a DLL entry point under Windows.
// Under UNIX system, nothing needed for a shared library.
// ----------------------------------------------------------------------------------------------------------
#if defined(_WIN32)
#	define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#	include <windows.h>

BOOL APIENTRY DllMain( HANDLE /*hModule*/,
                       DWORD  ul_reason_for_call,
                       LPVOID /*lpReserved*/
                       )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return true;
}
#endif // defined(_WIN32)

extern "C" GEXDB_PLUGIN_API GexDbPlugin_Base *gexdb_plugin_getobject(
        const QString & strHostName,
        const QString & strApplicationPath,
        const QString & strUserProfile,
        const QString & strLocalFolder,
        const char *gexLabelFilterChoices[],
        CGexSkin * pGexSkin, const GexScriptEngine* gse,
        const bool bCustomerDebugMode)
{
    GexDbPlugin_Galaxy *pObject = new GexDbPlugin_Galaxy(strHostName, strApplicationPath, strUserProfile, strLocalFolder,
                                                         gexLabelFilterChoices, bCustomerDebugMode, pGexSkin, gse);
    return (GexDbPlugin_Base *)pObject;
}

extern "C" GEXDB_PLUGIN_API void gexdb_plugin_releaseobject(GexDbPlugin_Base *pObject)
{
    if (pObject)
        delete pObject;
}

//////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////
GexDbPlugin_Galaxy::GexDbPlugin_Galaxy(const QString & strHostName,
                                       const QString & strApplicationPath,
                                       const QString & strUserProfile,
                                       const QString & strLocalFolder,
                                       const char *gexLabelFilterChoices[],
                                       const bool bCustomerDebugMode,
                                       CGexSkin * pGexSkin,
                                       const GexScriptEngine *gse,
                                       GexDbPlugin_Connector *pclDatabaseConnector)
    : GexDbPlugin_Base(strHostName, strApplicationPath, strUserProfile, strLocalFolder,
          gexLabelFilterChoices, bCustomerDebugMode, pGexSkin, gse, pclDatabaseConnector)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString("new Galaxy Plugin on %1").arg( strHostName).toLatin1().constData());
    m_eDbType = eUnknown;
    mHttpChannel = NULL;
}

GexDbPlugin_Galaxy::~GexDbPlugin_Galaxy()
{
    GSLOG(SYSLOG_SEV_DEBUG, QString("will delete RunInfoArray : %1")
          .arg( m_pRunInfoArray?"yes":"no").toLatin1().constData());
    // Free ressources
    if(m_pRunInfoArray)
    {
        delete [] m_pRunInfoArray;
        m_pRunInfoArray=0;
    }
    if(mRdbOptionsWidget)
    {
        delete mRdbOptionsWidget;
        mRdbOptionsWidget = NULL;
    }
    if (m_pConsolidationTree)
    {
        delete m_pConsolidationTree;
        m_pConsolidationTree = NULL;
    }

    if (mConsolidationCenter)
    {
        delete mConsolidationCenter;
        mConsolidationCenter = NULL;
    }

    if(mHttpChannel)
    {
        mHttpChannel->deleteLater();
    }
}

///////////////////////////////////////////////////////////
// Init some variables ...
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::Init()
{
    if (!GexDbPlugin_Base::Init())
        return false;

    if (!InitCore())
        return false;

    if (!InitGui())
        return false;

    return true;
}

bool GexDbPlugin_Galaxy::InitCore()
{
    // Variables init
    m_strPluginName = GEXDB_PLUGIN_GALAXY_NAME;
    //m_pCStdfParse = NULL;
    m_bEnableAutomaticIncrementalUpdates = true;
    m_bAutomaticStartup = true;
    mTdrLinkName = "";
    mAdrLinkName = "";

    m_pConsolidationTree    = NULL;
    mBackgroundTransferActivated = false;
    mBackgroundTransferInProgress = 0;
    mBackgroundTransferCurrentIndex.clear();

    mpDbKeysEngine = NULL;
    m_uiDbVersionBuild = m_uiDbVersionMajor = m_uiDbVersionMinor = 0;
    m_mapDbStatusMessage[GEXDB_DB_UPDATE_DB] = "Database needs to be update";
    m_mapDbStatusMessage[GEXDB_DB_UPDATE_CONS_TREE] = "Consolidation tree needs to be update";
    m_mapDbStatusMessage[GEXDB_DB_UPDATE_CONS_TRIGGERS] = "Consolidation triggers needs to be update";
    m_mapDbStatusMessage[GEXDB_DB_UPDATE_CONS_TABLES] = "Consolidation tables needs to be update";
    m_mapDbStatusMessage[GEXDB_DB_UPDATE_CONS_PROCEDURES] = "Consolidation procedures needs to be update";
    m_mapDbStatusMessage[GEXDB_DB_UPDATE_CONS_OLD] = "Consolidation process needs to be update";
    m_mapDbStatusMessage[GEXDB_DB_UPDATE_INDEXES] = "Indexes need to be update";

    m_pConsolidationTree = new ConsolidationTree(this);

    if (!m_pConsolidationTree)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Failed to create Consolidation Tree instance");
        return false;
    }

    Init_Extraction();

    if (!InitStdfFields())
        return false;

    return true;
}

bool GexDbPlugin_Galaxy::InitGui()
{
    mRdbOptionsWidget = NULL;
    mConsolidationCenter = NULL;

    if (mGexScriptEngine->property("GS_DAEMON").toBool())
        GSLOG(SYSLOG_SEV_DEBUG, "Daemon mode no gui object to init");
    return true;
}

bool GexDbPlugin_Galaxy::IsTestingStage_FinalTest(const QString & strTestingStage)
{
    return (strTestingStage == GEXDB_PLUGIN_GALAXY_FTEST);
}

bool GexDbPlugin_Galaxy::IsTestingStage_Foundry(const QString & strTestingStage)
{
    return (strTestingStage == GEXDB_PLUGIN_GALAXY_ETEST);
}

bool GexDbPlugin_Galaxy::GetTestingStageName_Foundry(QString & strTestingStage)
{
    strTestingStage = GEXDB_PLUGIN_GALAXY_ETEST;
    return true;
}

//////////////////////////////////////////////////////////////////////
// Call plugin configuration wizard
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::ConfigWizard()
{
    GexDbPlugin_Connector clDatabaseConnector("Original");
    // Create Connector, unless not privately owned
    if(m_bPrivateConnector)
    {
        // Create a new connector?
        if(!m_pclDatabaseConnector)
        {
            m_pclDatabaseConnector = new GexDbPlugin_Connector(m_strPluginName, this);
            if(mWizardGuiOptionsMap["allowed_database_type"].contains("adr"))
            {
                // For AdrType
                m_pclDatabaseConnector->m_bAdminUser = true;
                m_pclDatabaseConnector->m_strHost_Name = "localhost";
                m_pclDatabaseConnector->m_strHost_Unresolved = "localhost";
                m_pclDatabaseConnector->m_strHost_IP = "127.0.0.1";
                m_pclDatabaseConnector->m_strDriver = "QMYSQL3";
                m_pclDatabaseConnector->m_strDatabaseName = "adr";
                m_pclDatabaseConnector->m_strUserName_Admin = "adr";
                m_pclDatabaseConnector->m_strPassword_Admin = "adradmin";
                m_pclDatabaseConnector->m_strUserName = "adr_user";
                m_pclDatabaseConnector->m_strPassword = "adruser";
                m_pclDatabaseConnector->m_strSchemaName = "adr";
                m_pclDatabaseConnector->m_uiPort = 3306;
            }
            else
            {
                m_pclDatabaseConnector->m_bAdminUser = true;
                m_pclDatabaseConnector->m_strHost_Name = "localhost";
                m_pclDatabaseConnector->m_strHost_Unresolved = "localhost";
                m_pclDatabaseConnector->m_strHost_IP = "127.0.0.1";
                m_pclDatabaseConnector->m_strDriver = "QMYSQL3";
                m_pclDatabaseConnector->m_strDatabaseName = "gexdb";
                m_pclDatabaseConnector->m_strUserName_Admin = "gexdb";
                m_pclDatabaseConnector->m_strPassword_Admin = "gexadmin";
                m_pclDatabaseConnector->m_strUserName = "gexdb_user";
                m_pclDatabaseConnector->m_strPassword = "gexuser";
                m_pclDatabaseConnector->m_strSchemaName = "gexdb";
                m_pclDatabaseConnector->m_uiPort = 3306;
            }
        }
    }

    // Copy the original connector to check modification
    if(m_pclDatabaseConnector)
        clDatabaseConnector = (*m_pclDatabaseConnector);

    // Create Wizard
    GSLOG(SYSLOG_SEV_DEBUG, " creating config wizard GUI...");
    GexDbPlugin_Galaxy_CfgWizard clWizard(m_strHostName,
                                          m_strApplicationPath,
                                          m_strLocalFolder,
                                          m_bCustomerDebugMode,
                                          mWizardGuiOptionsMap,
                                          this,m_pGexSkin,
                                          mParentWidget);

    clWizard.Set(*m_pclDatabaseConnector, m_bAutomaticStartup);

    GSLOG(SYSLOG_SEV_INFORMATIONAL, "Executing config wizard...");

    // Do not use the Connection Timeout option
    //int nConnectionTimeout = m_nConnectionTimeOut;
    //m_nConnectionTimeOut = 0;						// Disabled the Connection Timeout option
    bool bStatus = (clWizard.exec() == QDialog::Accepted);
    //m_nConnectionTimeOut = nConnectionTimeout;		// Restore the Connection Timeout option

    if(bStatus && (!clWizard.IsReadOnly() || clWizard.IsOpenModeCreation()))
    {

        // Retrieve values from wizard
        bool bAutomaticStartup = m_bAutomaticStartup;

        clWizard.Get(*m_pclDatabaseConnector, &m_bAutomaticStartup);

        ConnectToCorporateDb();

        if (clWizard.IsOpenModeCreation())
            return true;

        //case 5332 : are those checks really necessary ?
        bool bUpdated = false;
        if(bAutomaticStartup != m_bAutomaticStartup)
            bUpdated = true;
        if(clDatabaseConnector.m_strPluginName != m_pclDatabaseConnector->m_strPluginName)
            bUpdated = true;
        if(clDatabaseConnector.m_strDriver != m_pclDatabaseConnector->m_strDriver)
            bUpdated = true;
        if(clDatabaseConnector.m_uiPort != m_pclDatabaseConnector->m_uiPort)
            bUpdated = true;
        if(clDatabaseConnector.m_strHost_Unresolved != m_pclDatabaseConnector->m_strHost_Unresolved)
            bUpdated = true;
        if(clDatabaseConnector.m_strHost_Name != m_pclDatabaseConnector->m_strHost_Name)
            bUpdated = true;
        if(clDatabaseConnector.m_strHost_IP != m_pclDatabaseConnector->m_strHost_IP)
            bUpdated = true;
        if(clDatabaseConnector.m_strDatabaseName != m_pclDatabaseConnector->m_strDatabaseName)
            bUpdated = true;
        if(clDatabaseConnector.m_strSchemaName != m_pclDatabaseConnector->m_strSchemaName)
            bUpdated = true;
        if(clDatabaseConnector.m_strUserName_Admin != m_pclDatabaseConnector->m_strUserName_Admin)
            bUpdated = true;
        if(clDatabaseConnector.m_strUserName != m_pclDatabaseConnector->m_strUserName)
            bUpdated = true;
        if(clDatabaseConnector.m_strPassword_Admin != m_pclDatabaseConnector->m_strPassword_Admin)
            bUpdated = true;
        if(clDatabaseConnector.m_strPassword != m_pclDatabaseConnector->m_strPassword)
            bUpdated = true;
        return bUpdated;
    }

    return false;
}

///////////////////////////////////////////////////////////
// Get stack of insertion warnings
///////////////////////////////////////////////////////////
void GexDbPlugin_Galaxy::GetWarnings(QStringList & strlWarnings)
{
    strlWarnings.clear();
    strlWarnings = m_strlWarnings;
}

///////////////////////////////////////////////////////////
// Set the LogFile
// strLogFileName contains just the file name
// use the correct Logs folder
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::SetUpdateDbLogFile(QString strLogFileName)
{
    if(strLogFileName.isEmpty())
    {
        m_strUpdateDbLogFile = "";
        return true;
    }

    // Construct file name
    m_strUpdateDbLogFile = m_strLocalFolder + "/GalaxySemi/logs/" + QDate::currentDate().toString(Qt::ISODate);
    QDir d(m_strUpdateDbLogFile);
    if (!d.exists(m_strUpdateDbLogFile))
        if (!d.mkpath(m_strUpdateDbLogFile))
            return false;
    m_strUpdateDbLogFile += QDir::separator() + strLogFileName;
    return true;
}

bool	GexDbPlugin_Galaxy::Reset(const QString &strDB_Folder)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString(" %1").arg( strDB_Folder).toLatin1().constData() );

    if (!m_pclDatabaseConnector)
        return false;

    if (m_pclDatabaseConnector->IsSQLiteDB())
    {
        QString source("plugins/");
        QDir clDir;
        clDir.setPath(source);
        QStringList lstFiles = clDir.entryList( QStringList("*.sqlite"), QDir::Files, QDir::Name);
        if(lstFiles.isEmpty())
        {
            GSLOG(SYSLOG_SEV_ERROR,
                  QString(" Sqlite install script not found (" + source +
                          "*.sqlite) !").toLatin1().constData());
            return false;
        }
        source += lstFiles.last();
        QString dest(strDB_Folder);
        dest+="/gexdb.sqlite";
        if (!QFile::copy( source, dest ))
        {
            GSLOG(SYSLOG_SEV_ERROR, " cant copy sqlite file !");
            return false;
        }
        QFile f(dest);
        if (!f.exists())
        {
            GSLOG(SYSLOG_SEV_ERROR, " cant find new sqlite file !");
            return false;
        }
        if (f.setPermissions( QFile::WriteOther ))
            return true;
        GSLOG(SYSLOG_SEV_ERROR, " cant set permission on new sqlite file !");
        return false;
    }

    return true;
}

// Return supported binning types.
// ConsolidatedType: "Y" or "N"
bool GexDbPlugin_Galaxy::GetSupportedBinTypes(const QString &TestingStage, QStringList & strlBinTypes, const QString &ConsolidatedType/*="Y"*/)
{
    // Clear output
    strlBinTypes.clear();

    // If not consolidated mode, H and S bins are supported
    if(ConsolidatedType.toLower() != "y")
    {
        strlBinTypes << "H" << "S";
        return true;
    }

    // AZ and FT consolidated: H supported, S depends on option
    if((TestingStage.toLower() == QString(GEXDB_PLUGIN_GALAXY_AZ).toLower()) || (TestingStage.toLower() == QString(GEXDB_PLUGIN_GALAXY_FTEST).toLower()))
    {
        strlBinTypes << "H";

        QString strValue;
        if(GetGlobalOptionValue(eBinningFtConsolidateSBin,strValue))
        {
            if(strValue.toUpper() == "TRUE")
                strlBinTypes << "S";
        }
        return true;
    }

    // WT/ET consolidated: H and S supported
    strlBinTypes << "H" << "S";

    return true;
}

// Retrieve list of available fields for the given testing stage. output will contains the matching fields.
void GexDbPlugin_Galaxy::GetRdbFieldsList(	const QString &TestingStage, QStringList &output,
                                            const QString &In_GUI,				// Y, N or *
                                            const QString &BinType,				// BinType can be H, S or N (or even *)
                                            const QString &Custom,				// Custom can be Y, N or *
                                            const QString &TimeType,			// TimeType can be Y, N or *
                                            const QString &ConsolidatedType,	// ConsolidatedType can be Y, N or *
                                            const QString &Facts,				// Facts can be Y, N, or *
                                            bool  OnlyStaticMetaData
                                            )
{
    /*
 GSLOG(SYSLOG_SEV_DEBUG, QString(" on %1 with GUI=%2 BinType=%3 Custom=%4 TimeType=%5 Conso=%6 Facts=%7").arg(
     TestingStage.toLatin1().data(), In_GUI.toLatin1().data(), BinType.toLatin1().data(), Custom.toLatin1().data(),
     TimeType.toLatin1().data(), ConsolidatedType.toLatin1().data(), Facts.toLatin1().data() );
 */

    QStringList		strlTestingStages;
    bool			bBinningFieldRequested=(BinType.toLower()=="h" || BinType.toLower()=="s");
    bool			bAtoZ=false, bBinningField=false;
    unsigned int	uiIndex;
    QString			strTestingStage, strMetaDataName;

    // Clear output
    output.clear();

    // Single testing stage, or AZ (all multiple testing stages) ?
    if(TestingStage.toLower() == QString(GEXDB_PLUGIN_GALAXY_AZ).toLower())
    {
        strlTestingStages << GEXDB_PLUGIN_GALAXY_WTEST << GEXDB_PLUGIN_GALAXY_FTEST;
        bAtoZ = true;

        // If 'A to Z', add 'Production stage'
        output.append(GEXDB_PLUGIN_DBFIELD_PRODUCTIONSTAGE);
    }
    else
        strlTestingStages << TestingStage;

    // Iterate over testiung stages
    for(uiIndex=0; uiIndex<(unsigned)strlTestingStages.size(); uiIndex++)
    {
        // Set testingstage
        strTestingStage = strlTestingStages.at(uiIndex);
        if(!SetTestingStage(strTestingStage))
            return;

        // Iterate through Meta-Data
        GexDbPlugin_Mapping_FieldMap::Iterator	itMapping;
        GexDbPlugin_Mapping_Field				clFieldMap;
        for(itMapping = m_pmapFields_GexToRemote->begin(); itMapping != m_pmapFields_GexToRemote->end(); itMapping++)
        {
            clFieldMap = itMapping.value();
            if(OnlyStaticMetaData && !clFieldMap.isStaticMetaData())                        continue;
            bBinningField=(clFieldMap.m_sBinType.toLower()=="h" || clFieldMap.m_sBinType.toLower()=="s");
            if((In_GUI.toLower() == "y") && !clFieldMap.m_bDisplayInERGui)					continue;
            if((In_GUI.toLower() == "n") && clFieldMap.m_bDisplayInERGui)					continue;
            // If BinType field requested, and field is a BinType field, do not check other properties
            if(bBinningFieldRequested && bBinningField)
            {
                if(BinType.toLower() != clFieldMap.m_sBinType.toLower())						continue;
                // Do not include binning fields if AZ
                if(bAtoZ)																		continue;
            }
            else
            {
                if((BinType.toLower() == "n") && bBinningField)									continue;
                if((Custom.toLower() == "y") && !clFieldMap.m_bCustom)							continue;
                if((Custom.toLower() == "n") && clFieldMap.m_bCustom)							continue;
                if((TimeType.toLower() == "y") && !clFieldMap.m_bTime)							continue;
                if((TimeType.toLower() == "n") && clFieldMap.m_bTime)							continue;
                if((ConsolidatedType.toLower() == "y") && !clFieldMap.m_bConsolidated)			continue;
                if((ConsolidatedType.toLower() == "n") && clFieldMap.m_bConsolidated)			continue;
                if((Facts.toLower() == "y") && !clFieldMap.m_bFact)								continue;
                if((Facts.toLower() == "n") && clFieldMap.m_bFact)								continue;
            }

            strMetaDataName = clFieldMap.m_strMetaDataName;

            // A to Z ?
            if(bAtoZ)
            {
                if(!bBinningField && (!clFieldMap.m_bConsolidated || !clFieldMap.m_bAZ))	continue;
                if((strTestingStage != GEXDB_PLUGIN_GALAXY_FTEST) && (clFieldMap.m_bTime))	continue;

                // Prepend with testing stage
                if(!bBinningField && !clFieldMap.m_bTime)
                {
                    if(strTestingStage == GEXDB_PLUGIN_GALAXY_WTEST)
                        strMetaDataName = "WT " + strMetaDataName;
                    if(strTestingStage == GEXDB_PLUGIN_GALAXY_FTEST)
                        strMetaDataName = "FT " + strMetaDataName;
                }
            }

            // order by time
            if (clFieldMap.m_bTime)
                output.prepend(strMetaDataName);
            else
                output.append(strMetaDataName);
        }
    }
}

bool GexDbPlugin_Galaxy::GetRdbFieldProperties(const QString& TestingStage, const QString& MetaDataName, QMap<QString,QString>& Properties)
{
    // Clear output
    Properties.clear();


    if(!SetTestingStage(TestingStage))
        return false;

    if(!m_pmapFields_GexToRemote->contains(MetaDataName))
        return false;

    GexDbPlugin_Mapping_Field   lFieldMap;
    lFieldMap = m_pmapFields_GexToRemote->value(MetaDataName);

    Properties["az_field"] = (lFieldMap.m_bAZ?"Y":"N");
    Properties["consolidated_field"] = (lFieldMap.m_bConsolidated?"Y":"N");;
    Properties["custom_field"] = (lFieldMap.m_bCustom?"Y":"N");;
    Properties["er_display_in_gui"] = (lFieldMap.m_bDisplayInERGui?"Y":"N");;
    Properties["gex_display_in_gui"] = (lFieldMap.m_bDisplayInQueryGui?"Y":"N");;
    Properties["fact_field"] = (lFieldMap.m_bFact?"Y":"N");;
    Properties["numeric_field"] = (lFieldMap.m_bNumeric?"Y":"N");;
    Properties["time_field"] = (lFieldMap.m_bTime?"Y":"N");;
    Properties["bintype_field"] = lFieldMap.m_sBinType;
    Properties["gex_name"] = lFieldMap.m_strGexName;
    Properties["meta_name"] = lFieldMap.m_strMetaDataName;
    Properties["normalized_name"] = lFieldMap.m_strNormalizedName;
    Properties["gexdb_field_fullname"] = lFieldMap.m_strSqlFullField;
    Properties["gexdb_link_name"] = lFieldMap.m_strSqlLinkName;
    Properties["gexdb_table_name"] = lFieldMap.m_strSqlTable;

    return true;
}

// Get consolidated field name corresponding to specified decorated field name
bool GexDbPlugin_Galaxy::GetConsolidatedFieldName(const QString & strTestingStage,
                                                  const QString & strDecoratedField,
                                                  QString & strConsolidatedField,
                                                  bool *pbIsNumeric /*= NULL*/,
                                                  bool * pbIsBinning /*= NULL*/,
                                                  bool * pbIsTime /*= NULL*/)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString(" %1").arg( strDecoratedField).toLatin1().constData());

    QString strTS = strTestingStage;
    QString strDF = strDecoratedField.toLower();
    QString strPrefix;

    bool	bAtoZ=false, bBinningField=false;

    // Check if AZ field
    if(strTestingStage.toLower() == QString(GEXDB_PLUGIN_GALAXY_AZ).toLower())
    {
        bAtoZ = true;

        // BG.TODO: use AZ metadata mapping once table exists (az_metadata_mapping)
        if(strDecoratedField == GEXDB_PLUGIN_DBFIELD_PRODUCTIONSTAGE)
        {
            strConsolidatedField = "production_stage";

            if (pbIsNumeric)
                *pbIsNumeric = false;

            if(pbIsBinning)
                *pbIsBinning = false;

            if (pbIsTime)
                *pbIsTime = false;

            return true;
        }

        strTS = GEXDB_PLUGIN_GALAXY_FTEST;
        if(strDF.startsWith("ft ") || strDF.startsWith("wt "))
        {
            strPrefix	= strDF.section(" ",0,0);
            strDF		= strDF.section(" ",1);
            if(strPrefix == "wt")
                strTS = GEXDB_PLUGIN_GALAXY_WTEST;
        }
    }

    // Set testingstage
    SetTestingStage(strTS);

    // Iterate through Meta-Data
    GexDbPlugin_Mapping_FieldMap::Iterator	itMapping;
    GexDbPlugin_Mapping_Field				clFieldMap;
    for(itMapping = m_pmapFields_GexToRemote->begin(); itMapping != m_pmapFields_GexToRemote->end(); itMapping++)
    {
        clFieldMap = itMapping.value();

        // Check if binning field
        bBinningField = (clFieldMap.m_sBinType.toLower()=="h" || clFieldMap.m_sBinType.toLower()=="s");

        if(clFieldMap.m_strMetaDataName.toLower() == strDF)
        {
            if (pbIsNumeric)
                *pbIsNumeric = clFieldMap.m_bNumeric;

            if (pbIsBinning)
                *pbIsBinning = bBinningField;

            if (pbIsTime)
                *pbIsTime = clFieldMap.m_bTime;

            // BG.TODO: remove, once using AZ metadata table
            if(bAtoZ && !clFieldMap.m_bTime && !bBinningField)
            {
                if(strTS == GEXDB_PLUGIN_GALAXY_WTEST)
                    strConsolidatedField = "wt_" + clFieldMap.m_strNormalizedName;
                else
                    strConsolidatedField = "ft_" + clFieldMap.m_strNormalizedName;
            }
            else
                strConsolidatedField = clFieldMap.m_strNormalizedName;

            // If binning field, remove 1st character (h or s), as it is not used in consolidated binning tables.
            if(bBinningField)
                strConsolidatedField = strConsolidatedField.remove(0, 1);

            return true;
        }
    }

    return false;
}

bool GexDbPlugin_Galaxy::InitRdbOptionsWidget()
{
    if (!mGexScriptEngine->property("GS_DAEMON").toBool())
    {
        if (!mRdbOptionsWidget)
            mRdbOptionsWidget = new GS::DbPluginGalaxy::RdbOptionsWidget(*mGexScriptEngine, NULL,
                                                                         Qt::FramelessWindowHint);
        if (!mRdbOptionsWidget)
        {
            GSLOG(SYSLOG_SEV_ERROR, "Failed to create a new RdbOptionsWidget : mRdbOptionsWidget will be NULL !");
            return false;
        }
    }
    return true;
}

///////////////////////////////////////////////////////////
// Returns a pointer on an options GUI.
///////////////////////////////////////////////////////////
QWidget *GexDbPlugin_Galaxy::GetRdbOptionsWidget()
{
    if (!mRdbOptionsWidget)
        InitRdbOptionsWidget();

    return mRdbOptionsWidget;
}

///////////////////////////////////////////////////////////
// Return a string with plug-in options.
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::GetRdbOptionsString(QString & strRdBOptionsString)
{
    if (!mRdbOptionsWidget)
        return false;
    mRdbOptionsWidget->GetOptionsString(strRdBOptionsString);
    return true;
}

///////////////////////////////////////////////////////////
// Sets the plug-in options GUI according to options string passed as argument.
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::SetRdbOptionsString(const QString & strRdBOptionsString)
{
    // Init plug-in options
    m_clOptions.Init(strRdBOptionsString);

    if (mRdbOptionsWidget)
        mRdbOptionsWidget->SetOptionsString(m_clOptions);
    return true;
}

///////////////////////////////////////////////////////////////////////
// Query: construct query string to retrieve all tests matching filters
///////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::ConstructTestlistQueryBACK(GexDbPlugin_Filter &cFilters, QString & strQuery, QString & strTestNumField, QString & strTestNameField)
{
    QString strDbField;
    QString strDbTable;
    QMap<QString, GexDbPlugin_Mapping_Field> conditions_filters;

    // Check testingstage to query
    if(cFilters.strDataTypeQuery == GEXDB_PLUGIN_GALAXY_ETEST)
    {
        m_pmapFields_GexToRemote = &m_mapFields_GexToRemote_Et;
        m_pmapLinks_Remote = &m_mapLinks_Remote_Et;
        m_strTablePrefix = "et_";
    }
    else if(cFilters.strDataTypeQuery == GEXDB_PLUGIN_GALAXY_WTEST)
    {
        m_pmapFields_GexToRemote = &m_mapFields_GexToRemote_Wt;
        m_pmapLinks_Remote = &m_mapLinks_Remote_Wt;
        m_strTablePrefix = "wt_";
    }
    else if(cFilters.strDataTypeQuery == GEXDB_PLUGIN_GALAXY_FTEST)
    {
        m_pmapFields_GexToRemote = &m_mapFields_GexToRemote_Ft;
        m_pmapLinks_Remote = &m_mapLinks_Remote_Ft;
        m_strTablePrefix = "ft_";
    }
    else
    {
        QString m=QString("Unknown Testing Stage '%1'").arg(cFilters.strDataTypeQuery);
        GSLOG(SYSLOG_SEV_WARNING, m.toLatin1().data());
        if (mQueryProgress)
            mQueryProgress->AddLog(m);
        return false;
    }

    QString lTestInfoTable;
    QString lTestInfoIdField;
    QString lTestType;

    if (strTestNumField == GEXDB_PLUGIN_DBFIELD_TESTNUM)
    {
        lTestInfoTable    = m_strTablePrefix + "ptest_info";
        lTestInfoIdField  = "ptest_info_id";
        lTestType         = "P";
    }
    else if (strTestNumField == GEXDB_PLUGIN_DBFIELD_TESTNUM_MPR)
    {
        lTestInfoTable    = m_strTablePrefix + "mptest_info";
        lTestInfoIdField  = "mptest_info_id";
        lTestType         = "M";
    }
    else if (strTestNumField == GEXDB_PLUGIN_DBFIELD_TESTNUM_FTR)
    {
        lTestInfoTable    = m_strTablePrefix + "ftest_info";
        lTestInfoIdField  = "ftest_info_id";
        lTestType         = "F";
    }
    else
    {
        QString m = QString("Unknown Test number field: '%1'").arg(strTestNumField);
        GSLOG(SYSLOG_SEV_WARNING, m.toLatin1().data());
        if (mQueryProgress)
            mQueryProgress->AddLog(m);
        return false;
    }

    // Looking for test condition filter
    foreach(const QString &filter, cFilters.strlQueryFilters)
    {
        if (filter.isEmpty())
            continue;

        QStringList	strlElements = filter.split("=");

        if (strlElements.size()!=2)
            continue;

        QString   strField = strlElements[0];

        GexDbPlugin_Mapping_FieldMap::Iterator itMapping = m_pmapFields_GexToRemote->find(strField);

        if(itMapping == m_pmapFields_GexToRemote->end())
            continue;

        GexDbPlugin_Mapping_Field mf = itMapping.value();
        if (!mf.isTestCondition())
            continue;

        conditions_filters.insert(filter, mf);
        GSLOG(SYSLOG_SEV_NOTICE, QString("Construct test list: test condition filter found on : %1")
              .arg( filter).toLatin1().constData() );
    }

    // Clear query string
    strQuery = "";

    // Construct query string:
    // SELECT <testnum, testname>
    // FROM <all required tables>
    // WHERE <link conditions> AND <value conditions>	(optional)
    Query_Empty();

    // Set field to query
    Query_AddField(strTestNumField, strDbField, strDbTable);
    Query_AddField(strTestNameField, strDbField, strDbTable);

    // Add link with test_condition table if filter defined on test condition
    if (!conditions_filters.isEmpty())
    {
        m_strlQuery_LinkConditions.append(lTestInfoTable + ".splitlot_id|" + m_strTablePrefix + "test_conditions.splitlot_id");
        m_strlQuery_LinkConditions.append(lTestInfoTable + "." + lTestInfoIdField + "|" + m_strTablePrefix + "test_conditions.test_info_id");

        foreach(const QString &f, conditions_filters.keys())
        {
            QString lValues = f.section("=",1,1);

            if (lValues.isEmpty())
                continue;

            GexDbPlugin_Mapping_Field lMetaData = conditions_filters.value(f);

            if (!lMetaData.m_bNumeric)
                lValues.replace("|",",").replace(",",GEXDB_PLUGIN_DELIMITER_OR);
            else
                lValues.replace("|", ","); // ok for numeric but not string

            QString new_cond = lMetaData.m_strSqlFullField+"|"+QString(lMetaData.m_bNumeric?"Numeric":"String")+"|"+ lValues;
            m_strlQuery_ValueConditions.append(new_cond);
        }

        m_strlQuery_ValueConditions.append( m_strTablePrefix + "test_conditions.test_type|String|" + lTestType);
    }

    // Set filters
    Query_AddFilters(cFilters);

    // Construct query from table and conditions
    Query_BuildSqlString(strQuery);

    return true;
}

qint64 GexDbPlugin_Galaxy::GetAutoIncrementTestNumber(int lTestType,
                                                   const QString& lOriginalTestName)
{
    qint64     lTestNumber = 0;
    QString lTestName   = QString("[%1][%2]")
            .arg(lTestType)
            .arg(NormalizeTestName(lOriginalTestName));

    // Apply the associated TestNumber
    if(mAutoIncrementTestMap.contains(lTestName))
        lTestNumber = mAutoIncrementTestMap[lTestName];
    else
    {
        qint64 lDummyMaxTestNumber = 0;

        if (mAIDummyMaxTestNumber.contains(lTestType))
            lDummyMaxTestNumber = mAIDummyMaxTestNumber.value(lTestType);

        lTestNumber = lDummyMaxTestNumber;
        ++lDummyMaxTestNumber;

        mAutoIncrementTestMap[lTestName]    = lTestNumber;
        mAIDummyMaxTestNumber[lTestType]    = lDummyMaxTestNumber;
    }

    return lTestNumber;
}
