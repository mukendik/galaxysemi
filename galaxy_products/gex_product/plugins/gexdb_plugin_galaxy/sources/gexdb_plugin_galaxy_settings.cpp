#include <QCryptographicHash>
#include <QSqlRecord>
// Local includes
#include "gexdb_plugin_galaxy.h"
#include "gexdb_plugin_galaxy_cfgwizard.h"
#include "gex_constants.h"
#include "gex_shared.h"
#include <gqtl_log.h>
#include "consolidation_tree.h"
#include "consolidation_tree_replies.h"
#include "db_architecture.h"
#include "query_engine.h"

//////////////////////////////////////////////////////////////////////
// Load plugin settings from file
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::ConnectToCorporateDb()
{
    // Check database connection
    if(!GexDbPlugin_Base::ConnectToCorporateDb())
        return false;

    if (!m_pclDatabaseConnector)
        return false;

    // Update Database Prefix for table access
    m_strPrefixDB = m_strSuffixDB = "";
    if(m_pclDatabaseConnector->IsOracleDB())
    {
        m_strPrefixDB = m_pclDatabaseConnector->m_strSchemaName+".";
    }
    if(m_pclDatabaseConnector->m_bUseQuotesInSqlQueries)
    {
        m_strPrefixDB += "\"";
        m_strSuffixDB += "\"";
    }

    // Load Db Version info
    QString             strQuery;
    QSqlQuery           clQuery(QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    QString             strDbVersionNb;
    unsigned int lPreviousVersionBuild = m_uiDbVersionBuild;
    m_uiDbVersionBuild = 0;
    m_uiDbVersionMajor = 0;
    m_uiDbVersionMinor = 0;
    m_strDbVersionName = "";
    mBackgroundTransferActivated = false;
    mBackgroundTransferInProgress = 0;

    strQuery = "SELECT db_version_name, db_version_nb, db_version_build ";
    strQuery+= "FROM "+NormalizeTableName("global_info",false);
    if(clQuery.exec(strQuery) && clQuery.first())
    {
        m_uiDbVersionBuild = clQuery.value(2).toUInt();
        // Before the B15, db_version_nb = 19 => 1.9
        // After the B15, db_version_nb = 109 => 1.09
        // 2 last char is for minor
        int nNbChar = 1;
        if(m_uiDbVersionBuild >= 15)
            nNbChar = 2;

        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        // REVERT UPDATE FOR PREVIEW V6.5
        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        // First preview B15 changes the version_nb to 200
        //		'GEXDB V2.00 B15 (MySQL)', '200', '15'
        // New preview B15 have to change the version_nb to 109 to use the compatibility
        //		'GEXDB V1.09 B15 (MySQL)', '109', '15',
        // Have to update Db with this new version_nb for all B15
        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        m_strDbVersionName = clQuery.value(0).toString();
        strDbVersionNb = clQuery.value(1).toString();
        if(m_strDbVersionName.startsWith("TDR V2.00 B15 ") && (strDbVersionNb == "200") && (m_uiDbVersionBuild == 15))
        {
            // Old preview
            // Update the DB
            strQuery = "UPDATE "+NormalizeTableName("global_info",false)+" SET ";
            strQuery+= "db_version_name='"+m_strDbVersionName.replace("V2.00 B15","V1.09 B15")+"', ";
            strQuery+= "db_version_nb=109 ";
            clQuery.exec(strQuery);

            strDbVersionNb = "109";

        }
        m_uiDbVersionMinor = strDbVersionNb.right(nNbChar).toUInt();
        m_uiDbVersionMajor = strDbVersionNb.left(strDbVersionNb.length()-nNbChar).toUInt();

        m_strDbStatus = "";
        strQuery = "SELECT db_status ";
        strQuery+= "FROM "+NormalizeTableName("global_info",false);
        if(clQuery.exec(strQuery) && clQuery.first())
            m_strDbStatus = clQuery.value(0).toString();

        if(m_uiDbVersionBuild < 18)
        {
            // MySql
            //  database name    gexdb_v1
            //  schema name      gexdb_v1 (schema=database)
            //  user name        gexdb_v1_user
            // Oracle
            //  database name    GALAXY or ORCL (=SID)
            //  schema name      gexdb_v1
            //  user name        gexdb_v1_user
            QString tdrType = m_pclDatabaseConnector->m_strSchemaName + GEXDB_YM_PROD_TDR_KEY;
            QByteArray hash = QCryptographicHash::hash(tdrType.toLatin1(),QCryptographicHash::Md5);
            m_strDbHashType = QString(hash.toHex());
            m_eDbType = eProdTdrDb;
        }
        else
        {
            strQuery = "SELECT db_type ";
            strQuery+= "FROM "+NormalizeTableName("global_info",false);
            if(clQuery.exec(strQuery) && clQuery.first())
                m_strDbHashType = clQuery.value(0).toString();

            if((m_uiDbVersionBuild==61)
                    && m_strDbHashType.isEmpty()
                    && (m_eDbType==eUnknown)
                    && m_pclDatabaseConnector->m_strDatabaseName.startsWith("adr"))
            {
                // Fix ADR B61
                UpdateDbSetTdrType(m_pclDatabaseConnector->m_strDatabaseName, eAdrDb, &clQuery);
                m_eDbType = eAdrDb;
            }
        }
    }

    if(IsAdr() || IsLocalAdr())
    {
        // For Adr
        // No SecuredMode
        // No Architecture(QueryEngine)
        // No Metadata
        // But get ADR Status
        // Load some options
        QString lOwnerVersion;
        strQuery = "SELECT table_name FROM information_schema.tables WHERE table_schema='"+m_pclDatabaseConnector->m_strDatabaseName+"'";
        strQuery+= " AND table_name LIKE '%owner'";
        if(clQuery.exec(strQuery))
        {
            QStringList lTables;
            while (clQuery.next())
            {
                lTables << clQuery.value("table_name").toString();
            }
            foreach (QString lTable, lTables)
            {
                strQuery = "SELECT owner_name, version FROM "+lTable;
                if(clQuery.exec(strQuery) && clQuery.first())
                {
                    lOwnerVersion += clQuery.value("owner_name").toString();
                    lOwnerVersion += ": ";
                    lOwnerVersion += clQuery.value("version").toString();
                    lOwnerVersion += "\n";
                }
            }
        }
        if(lOwnerVersion.isEmpty())
            lOwnerVersion = "No agent is managing this database";
        m_strDbVersionName = lOwnerVersion;

        return true;
    }

    if(!m_strDbStatus.isEmpty())
    {
        // Check valid Flag
        unsigned int uiUpdateFlags;
        GetDbUpdateSteps(uiUpdateFlags);
        if(uiUpdateFlags & eUpdateUnknown)
        {
            GSET_ERROR1(GexDbPlugin_Base, eDB_Status, NULL, GetDbStatus().toLatin1().constData());
            return false;
        }
    }

    // Load some options
    strQuery = "SELECT table_name ";
    strQuery+= " FROM information_schema.tables ";
    strQuery+= " WHERE table_schema="+TranslateStringToSqlVarChar(m_pclDatabaseConnector->m_strSchemaName);
    strQuery+= " AND table_name LIKE 'background_transfer%'";
    strQuery+= " LIMIT 1";
    if(clQuery.exec(strQuery) && clQuery.first())
    {
        mBackgroundTransferActivated = true;
        mBackgroundTransferCurrentIndex.clear();

        // Then Check if it is in progress
        strQuery = "SELECT MIN(update_id) AS update_id FROM background_transfer_tables WHERE STATUS <> 'DONE'";
        if(clQuery.exec(strQuery))
        {
            if(clQuery.first())
            {
                mBackgroundTransferInProgress = clQuery.value("update_id").toInt();
            }
            else
            {
                mBackgroundTransferInProgress = 0;
            }
        }
    }

    // For MySql, partitioning available only when all tables was convert to InnoDb
    if(m_pclDatabaseConnector->IsMySqlDB() && m_pclDatabaseConnector->m_bPartitioningDb)
    {
        // Check if have InnoDb option
        QString                 strEngine,strFormat;
        if(GetStorageEngineName(strEngine,strFormat) && (strEngine.toUpper() != "MYISAM"))
        {
            m_pclDatabaseConnector->m_bTransactionDb = true;
            m_pclDatabaseConnector->m_bPartitioningDb = true;
        }
        else
            m_pclDatabaseConnector->m_bPartitioningDb = false;
    }

    if(!IsYmProdTdr())
        m_pclDatabaseConnector->m_bTransactionDb = false;

    if(!LoadGlobalOptions())
        GSLOG(SYSLOG_SEV_ERROR, "Unable to load database options!");
    if(!LoadIncrementalUpdateNames())
        GSLOG(SYSLOG_SEV_ERROR, "Unable to load database incremental update names!");

    // Database restriction security
    if(!GetSecuredMode(m_strSecuredMode))
    {
        GSLOG(SYSLOG_SEV_ERROR, "Unable to check database restriction security!");
        return false;
    }

    if (lPreviousVersionBuild != m_uiDbVersionBuild)
    {
        if (!LoadDatabaseArchitecture())
        {
            GSLOG(SYSLOG_SEV_ERROR, "Unable to load database architecture!");
            return false;
        }
        if (!LoadMetaData())
            return false;

        return LoadQueryEngine();
    }

    return true;
}

//////////////////////////////////////////////////////////////////////
// Load settings from dom elt
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::LoadSettingsFromDom(const QDomElement &node)
{

    if (!GexDbPlugin_Base::LoadSettingsFromDom(node))
        return false;

    QDomElement nodePluginConfig = node.firstChildElement("PluginConfig");
    if (nodePluginConfig.isNull())
        return false;

    QDomElement nodeOptions = nodePluginConfig.firstChildElement("Options");
    if (!nodeOptions.isNull())
    {
        LoadStartupTypeFromDom(nodeOptions);
        LoadAdrNameFromDOM(nodeOptions);
    }
    else
    {
        GSLOG(SYSLOG_SEV_ERROR, "Marker <Options> not found while loading Plugin.");
        return false;
    }
    return true;
}

bool GexDbPlugin_Galaxy::LoadStartupTypeFromDom(const QDomElement &node)
{
    QDomElement elt = node.firstChildElement("StartupType");
    if (!elt.isNull())
    {
        m_bAutomaticStartup = (elt.text().trimmed() != "0");
    }
    else
    {
        GSLOG(SYSLOG_SEV_ERROR, "Marker <StartupType> not found while loading Plugin.");
        return false;
    }

    return true;
}


bool GexDbPlugin_Galaxy::LoadAdrNameFromDOM(const QDomElement &node)
{
    QDomElement elt = node.firstChildElement("AdrLink");
    if (!elt.isNull() && elt.text().contains("@"))
    {
        SetAdrLinkName(elt.text().trimmed()) ;
    }
    else
    {
        GSLOG(SYSLOG_SEV_ERROR, "Marker <AdrLink> not found while loading Plugin.");
        return false;
    }

    return true;
}

//////////////////////////////////////////////////////////////////////
// Load plugin settings from file
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::LoadSettings(QFile *pSettingsFile)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString("Load settings from %1").arg( pSettingsFile->fileName()).toLatin1().constData());
    GexDbPlugin_Base::LoadSettings(pSettingsFile);

    // Load options
    // Assign file I/O stream
    QString		strString;
    QString		strKeyword;
    QString		strParameter;
    QTextStream hFile(pSettingsFile);

    // Rewind file first
    pSettingsFile->reset();

    // Search for marker used by this object's settings
    while(!hFile.atEnd())
    {
        strString = hFile.readLine().trimmed();
        if(strString.toLower() == "<options>")
            break;
    }

    while(!hFile.atEnd())
    {
        // Read line.
        strString = hFile.readLine().trimmed();
        strKeyword = strString.section('=',0,0);
        strParameter = strString.section('=',1);

        if(strString.toLower() == "</options>")
            return true;

        if(strKeyword.toLower() == "startuptype")
            m_bAutomaticStartup = (strParameter.trimmed() != "0");
        if(strKeyword.toLower() == "adrlink" && strParameter.contains("@"))
            SetAdrLinkName(strParameter.trimmed());
    }

    return false;
}

// TODO: move most meta-data from static to dynamic
//       o write them into metadata table, except if a  row exists pointing to the same field
//       o remove them from static load
//       o only fields absolutely needed by GEX (product, lot_id, start_t...) should remain in static mode
//       o data from static table must be read-only (not possible to overwrite from dynamic data)

//////////////////////////////////////////////////////////////////////
// Load meta-data (mapping + links)
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::LoadMetaData()
{
    // Check database connection
    if(!GexDbPlugin_Base::ConnectToCorporateDb())
        return false;

    GSLOG(SYSLOG_SEV_INFORMATIONAL, "Load meta data...");

    // Clear meta-data maps
    m_mapFields_GexToRemote_Et.clear();
    m_mapLinks_Remote_Et.clear();
    m_mapFields_GexToRemote_Wt.clear();
    m_mapLinks_Remote_Wt.clear();
    m_mapFields_GexToRemote_Ft.clear();
    m_mapLinks_Remote_Ft.clear();

    if(IsAdr() || IsLocalAdr())
    {
        // For Adr
        // No MetaData
        return true;
    }

    // Load static meta-data
    LoadStaticMetaData();

    // Load dynamic meta-data
    return LoadDynamicMetaData();
}

//////////////////////////////////////////////////////////////////////
// Load meta-data (mapping + links)
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::LoadMetaData(unsigned int uiDbVersion_Build)
{
    qDebug("GexDbPlugin_Galaxy::LoadMetaData");

    // Check database connection
    if(!GexDbPlugin_Base::ConnectToCorporateDb())
        return false;

    // Clear meta-data maps
    m_mapFields_GexToRemote_Et.clear();
    m_mapLinks_Remote_Et.clear();
    m_mapFields_GexToRemote_Wt.clear();
    m_mapLinks_Remote_Wt.clear();
    m_mapFields_GexToRemote_Ft.clear();
    m_mapLinks_Remote_Ft.clear();

    // Load static meta-data
    LoadStaticMetaData(uiDbVersion_Build);

    // Load dynamic meta-data
    return LoadDynamicMetaData(uiDbVersion_Build);
}

//////////////////////////////////////////////////////////////////////
// Load dynamic meta-data from meta-data tables (mapping + links)
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::LoadDynamicMetaData()
{
    // Check database connection
    if(!GexDbPlugin_Base::ConnectToCorporateDb())
        return false;

    return LoadDynamicMetaData(m_uiDbVersionBuild);
}

//////////////////////////////////////////////////////////////////////
// Load dynamic meta-data from meta-data tables (mapping + links)
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::LoadDynamicMetaData(unsigned int uiDbVersion_Build)
{
    // Check database connection
    if(!GexDbPlugin_Base::ConnectToCorporateDb())
        return false;

    // Load Mapping from GEXDB database
    QString           strQuery, strMetaName, strGexName, strFullFieldName, strLinkName, strLinkName2;
    QString           strNormalizedName;
    GexDbPlugin_Query clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    clGexDbQuery.setForwardOnly(true);

    // FOR ET, WT DO THE SAME ACTION
    for(int nTestingStage=0; nTestingStage<3; nTestingStage++)
    {
        switch(nTestingStage)
        {
        case 0 :
            // FOR WAFER TEST
            SetTestingStage(eWaferTest);
            break;
        case 1 :
            // FOR ELECT TEST
            SetTestingStage(eElectTest);
            break;
        default:
            // FOR FINAL TEST
            SetTestingStage(eFinalTest);
            break;
        }

        // Get the list of MetaData from this GexDb (Static)
        // To check if the MetaData already exists (for MetaData creation)
        QMap<QString, QString> mapStaticMetaData;
        QStringList         lstValues;
        GetRdbFieldsList(m_strTestingStage,lstValues,"*","*","*","*","*","*",true);
        // For case insensitive
        while(!lstValues.isEmpty())
        {
            strMetaName = lstValues.takeFirst();
            mapStaticMetaData[strMetaName.toUpper()] = strMetaName;
        }

        // Query ET mapping
        strMetaName = strGexName = strFullFieldName = strLinkName = strLinkName2 = "";
        Query_Empty();
        m_strlQuery_Fields.append("Field|"+m_strPrefixTable+"_metadata_mapping.meta_name");
        m_strlQuery_Fields.append("Field|"+m_strPrefixTable+"_metadata_mapping.gex_name");
        m_strlQuery_Fields.append("Field|"+m_strPrefixTable+"_metadata_mapping.gexdb_table_name");
        m_strlQuery_Fields.append("Field|"+m_strPrefixTable+"_metadata_mapping.gexdb_field_fullname");
        m_strlQuery_Fields.append("Field|"+m_strPrefixTable+"_metadata_mapping.gexdb_link_name");
        m_strlQuery_Fields.append("Field|"+m_strPrefixTable+"_metadata_mapping.gex_display_in_gui");
        if(uiDbVersion_Build > 9)
        {
            m_strlQuery_Fields.append("Field|"+m_strPrefixTable+"_metadata_mapping.bintype_field");
            m_strlQuery_Fields.append("Field|"+m_strPrefixTable+"_metadata_mapping.time_field");
            m_strlQuery_Fields.append("Field|"+m_strPrefixTable+"_metadata_mapping.custom_field");
            m_strlQuery_Fields.append("Field|"+m_strPrefixTable+"_metadata_mapping.fact_field");
            m_strlQuery_Fields.append("Field|"+m_strPrefixTable+"_metadata_mapping.consolidated_field");
            m_strlQuery_Fields.append("Field|"+m_strPrefixTable+"_metadata_mapping.er_display_in_gui");
            m_strlQuery_Fields.append("Field|"+m_strPrefixTable+"_metadata_mapping.numeric_field");
        }
        if(uiDbVersion_Build > 11)
            m_strlQuery_Fields.append("Field|"+m_strPrefixTable+"_metadata_mapping.az_field");
        Query_BuildSqlString(strQuery);
        if(!clGexDbQuery.Execute(strQuery))
            return true;

        QString lTableName, lBin;
        bool lInGui, lTime, lCustom, lFact, lCons, lInErGui, lNum, lAz, lStaticData;

        // Read extracted ET mapping data
        GexDbPlugin_Mapping_Field	clFieldMapping;
        while(clGexDbQuery.Next())
        {
            strNormalizedName.clear();

            lInGui = lTime = lCustom = lFact = lCons = lInErGui = lNum = lAz = false;
            lCustom = lCons = lInErGui = true;
            lBin = "N";
            lStaticData = false;

            strMetaName = clGexDbQuery.value("meta_name").toString().simplified();
            strGexName = clGexDbQuery.value("gex_name").toString();
            lTableName = clGexDbQuery.value("gexdb_table_name").toString();
            strFullFieldName = clGexDbQuery.value("gexdb_field_fullname").toString();
            strLinkName = clGexDbQuery.value("gexdb_link_name").toString();

            lInGui = (clGexDbQuery.value("gex_display_in_gui").toString().toUpper() == "Y");
            if(uiDbVersion_Build > 9)
            {
                lBin = clGexDbQuery.value("bintype_field").toString().toUpper();
                lTime = (clGexDbQuery.value("time_field").toString().toUpper() == "Y");
                lCustom = (clGexDbQuery.value("custom_field").toString().toUpper() == "Y");
                lFact = (clGexDbQuery.value("fact_field").toString().toUpper() == "Y");
                lCons = (clGexDbQuery.value("consolidated_field").toString().toUpper() == "Y");
                lInErGui = (clGexDbQuery.value("er_display_in_gui").toString().toUpper() == "Y");
                lNum = (clGexDbQuery.value("numeric_field").toString().toUpper() == "Y");
            }
            if(uiDbVersion_Build > 11)
                lAz = (clGexDbQuery.value("az_field").toString().toUpper() == "Y");

            if(mapStaticMetaData.contains(strMetaName.toUpper()))
            {
                // Use the good case
                // Use the Gex Normalized KeyWord
                strMetaName = mapStaticMetaData[strMetaName.toUpper()];
                GSLOG(SYSLOG_SEV_ERROR, QString("Static MetaData already exist %1. The latest will be used.")
                      .arg( strMetaName.toLatin1().constData()).toLatin1().constData());
                // For static Meta-Data
                // allow only GUI management
                lStaticData = true;
                strGexName = (*m_pmapFields_GexToRemote)[strMetaName].m_strGexName;
                lTableName = (*m_pmapFields_GexToRemote)[strMetaName].m_strSqlTable;
                strFullFieldName = (*m_pmapFields_GexToRemote)[strMetaName].m_strSqlFullField;
                strNormalizedName = (*m_pmapFields_GexToRemote)[strMetaName].m_strNormalizedName;
                strLinkName = (*m_pmapFields_GexToRemote)[strMetaName].m_strSqlLinkName;
                lBin = (*m_pmapFields_GexToRemote)[strMetaName].m_sBinType;
                lTime = (*m_pmapFields_GexToRemote)[strMetaName].m_bTime;
                lCustom = (*m_pmapFields_GexToRemote)[strMetaName].m_bCustom;
                lFact = (*m_pmapFields_GexToRemote)[strMetaName].m_bFact;
                lCons = (*m_pmapFields_GexToRemote)[strMetaName].m_bConsolidated;
                lNum = (*m_pmapFields_GexToRemote)[strMetaName].m_bNumeric;
                lAz = (*m_pmapFields_GexToRemote)[strMetaName].m_bAZ;
            }

            // strNormalizedName is set if it is a Gex MetaData
            // Check if we already have an entry pointing to same DB field.
            // If so, remove existing entry, so it gets replaced with dynamic entry.
            if(strNormalizedName.isEmpty()
                    && !strFullFieldName.isEmpty()
                    && m_pmapFields_GexToRemote->ContainsSqlFullField(strFullFieldName, clFieldMapping))
            {
                // If alreay have an entry
                strNormalizedName =	strMetaName.simplified().toLower();
                strNormalizedName.replace(QRegExp("[^0-9a-zA-Z_]"), "_");
                // Check if it is not a reserved MySql KeyWord (to avoid Query error
                if(IsReservedWord(strNormalizedName))
                    strNormalizedName += "_";
            }

            // Add the entry
                (*m_pmapFields_GexToRemote)[strMetaName] =
                        GexDbPlugin_Mapping_Field(strMetaName, strGexName, lTableName, strFullFieldName, strLinkName,
                                                  lInGui, lBin,
                                                  lTime, lCustom,
                                                  lFact, lCons,
                                                  lInErGui, lNum,
                                                  lAz, lStaticData
                                                  );

            if (strNormalizedName.isEmpty() == false)
                (*m_pmapFields_GexToRemote)[strMetaName].m_strNormalizedName = strNormalizedName;
        }

        // Query ET Test condition meta-data
        if (LoadDynamicTCMetaData((TestingStage) m_eTestingStage, uiDbVersion_Build) == false)
            return false;

        // Query ET links
        strMetaName = strGexName = strLinkName = strLinkName2 = "";
        Query_Empty();
        m_strlQuery_Fields.append("Field|"+m_strPrefixTable+"_metadata_link.link_name");
        m_strlQuery_Fields.append("Field|"+m_strPrefixTable+"_metadata_link.gexdb_table1_name");
        m_strlQuery_Fields.append("Field|"+m_strPrefixTable+"_metadata_link.gexdb_field1_fullname");
        m_strlQuery_Fields.append("Field|"+m_strPrefixTable+"_metadata_link.gexdb_table2_name");
        m_strlQuery_Fields.append("Field|"+m_strPrefixTable+"_metadata_link.gexdb_field2_fullname");
        m_strlQuery_Fields.append("Field|"+m_strPrefixTable+"_metadata_link.gexdb_link_name");
        Query_BuildSqlString(strQuery);
        if(!clGexDbQuery.Execute(strQuery))
            return true;

        // Read extracted ET link data
        GexDbPlugin_Mapping_LinkMap lMapLinksEt;
        while(clGexDbQuery.Next())
        {
            strLinkName2 = "";
            strLinkName = clGexDbQuery.value("link_name").toString();
            if(!clGexDbQuery.value("gexdb_link_name").isNull())
                strLinkName2 = clGexDbQuery.value("gexdb_link_name").toString();
            // remove duplicate before use of ::unite()
            if ((*m_pmapLinks_Remote).contains(strLinkName))
                (*m_pmapLinks_Remote).remove(strLinkName);
            // Check if we already have an entry with the same link
            // If so, just update other fields
            // Else, add the entry
            lMapLinksEt[strLinkName] = GexDbPlugin_Mapping_Link(
                        strLinkName,
                        clGexDbQuery.value("gexdb_table1_name").toString(),
                        clGexDbQuery.value("gexdb_field1_fullname").toString(),
                        clGexDbQuery.value("gexdb_table2_name").toString(),
                        clGexDbQuery.value("gexdb_field2_fullname").toString(),
                        strLinkName2);
        }
        // update ET links map
        (*m_pmapLinks_Remote).unite(lMapLinksEt);
        // 6408 update db architecture with external tables
        UpdateDbArchitectureWithExternalLink((*m_pmapLinks_Remote), (TestingStage) m_eTestingStage);
    }


    // List of available filters
    GexDbPlugin_Mapping_FieldMap::Iterator itMapping;
    m_strlLabelFilterChoices_Et.clear();
    m_strlLabelFilterChoices_Wt.clear();
    m_strlLabelFilterChoices_Ft.clear();
    m_strlLabelFilterChoices_Consolidated_Et.clear();
    m_strlLabelFilterChoices_Consolidated_Wt.clear();
    m_strlLabelFilterChoices_Consolidated_Ft.clear();
    for(itMapping = m_mapFields_GexToRemote_Et.begin(); itMapping != m_mapFields_GexToRemote_Et.end(); itMapping++)
    {
        if((*itMapping).m_bDisplayInQueryGui) {
            m_strlLabelFilterChoices_Et.append((*itMapping).m_strMetaDataName);
            if((*itMapping).m_bConsolidated) {
                m_strlLabelFilterChoices_Consolidated_Et.append((*itMapping).m_strMetaDataName);
            }
        }
    }
    for(itMapping = m_mapFields_GexToRemote_Wt.begin(); itMapping != m_mapFields_GexToRemote_Wt.end(); itMapping++)
    {
        if((*itMapping).m_bDisplayInQueryGui) {
            m_strlLabelFilterChoices_Wt.append((*itMapping).m_strMetaDataName);
            if((*itMapping).m_bConsolidated) {
                m_strlLabelFilterChoices_Consolidated_Wt.append((*itMapping).m_strMetaDataName);
            }
        }
    }
    for(itMapping = m_mapFields_GexToRemote_Ft.begin(); itMapping != m_mapFields_GexToRemote_Ft.end(); itMapping++)
    {
        if((*itMapping).m_bDisplayInQueryGui) {
            m_strlLabelFilterChoices_Ft.append((*itMapping).m_strMetaDataName);
            if((*itMapping).m_bConsolidated) {
                m_strlLabelFilterChoices_Consolidated_Ft.append((*itMapping).m_strMetaDataName);
            }
        }
    }

    // List of available filters at wafer level
    m_strlLabelFilterChoices_WaferLevel_Et.clear();
    m_strlLabelFilterChoices_WaferLevel_Wt.clear();
    for(itMapping = m_mapFields_GexToRemote_Et.begin(); itMapping != m_mapFields_GexToRemote_Et.end(); itMapping++)
    {
        if((*itMapping).m_bDisplayInQueryGui && m_mapLinks_Remote_Et.IsLinkedToTable((*itMapping), "et_wafer_info"))
            m_strlLabelFilterChoices_WaferLevel_Et.append((*itMapping).m_strMetaDataName);
    }
    for(itMapping = m_mapFields_GexToRemote_Wt.begin(); itMapping != m_mapFields_GexToRemote_Wt.end(); itMapping++)
    {
        if((*itMapping).m_bDisplayInQueryGui && m_mapLinks_Remote_Wt.IsLinkedToTable((*itMapping), "wt_wafer_info"))
            m_strlLabelFilterChoices_WaferLevel_Wt.append((*itMapping).m_strMetaDataName);
    }

    // List of available filters at lot level
    m_strlLabelFilterChoices_LotLevel_Et.clear();
    m_strlLabelFilterChoices_LotLevel_Wt.clear();
    m_strlLabelFilterChoices_LotLevel_Ft.clear();
    for(itMapping = m_mapFields_GexToRemote_Et.begin(); itMapping != m_mapFields_GexToRemote_Et.end(); itMapping++)
    {
        if((*itMapping).m_bDisplayInQueryGui && m_mapLinks_Remote_Et.IsLinkedToTable((*itMapping), "et_lot"))
            m_strlLabelFilterChoices_LotLevel_Et.append((*itMapping).m_strMetaDataName);
    }
    for(itMapping = m_mapFields_GexToRemote_Wt.begin(); itMapping != m_mapFields_GexToRemote_Wt.end(); itMapping++)
    {
        if((*itMapping).m_bDisplayInQueryGui && m_mapLinks_Remote_Wt.IsLinkedToTable((*itMapping), "wt_lot"))
            m_strlLabelFilterChoices_LotLevel_Wt.append((*itMapping).m_strMetaDataName);
    }
    for(itMapping = m_mapFields_GexToRemote_Ft.begin(); itMapping != m_mapFields_GexToRemote_Ft.end(); itMapping++)
    {
        if((*itMapping).m_bDisplayInQueryGui && m_mapLinks_Remote_Ft.IsLinkedToTable((*itMapping), "ft_lot"))
            m_strlLabelFilterChoices_LotLevel_Ft.append((*itMapping).m_strMetaDataName);
    }

    return true;
}

bool GexDbPlugin_Galaxy::UpdateDbArchitectureWithExternalLink(
        const GexDbPlugin_Mapping_LinkMap &linksMap,
        GexDbPlugin_Galaxy::TestingStage testingStage)
{
    GexDbPlugin_Mapping_LinkMap lLinksMap = linksMap;
    QString lTestingStage;
    switch(testingStage)
    {
    case eElectTest: lTestingStage = "e-test";
        break;
    case eWaferTest: lTestingStage = "w-test";
        break;
    case eFinalTest: lTestingStage = "f-test";
        break;
    default:         lTestingStage = "any";
        break;
    }

    // first search multiple dependencies on one table
    QMapIterator<QString, GexDbPlugin_Mapping_Link> it(lLinksMap);
    while (it.hasNext())
    {
        it.next();
        GexDbPlugin_Mapping_Link lCurrentLink = it.value();
        if (lCurrentLink.m_strSqlTable2Link.trimmed().isEmpty())
        {
            QList<GexDbPlugin_Mapping_Link> lPredLinks;
            // get all predecessor link of selected link
            lPredLinks = lLinksMap.GetLinkPredecessorList(lCurrentLink);
            int lPredCount = lPredLinks.count() + 1; // to include current link
            // if more than one predecessor and not with same table
            if ((lPredCount > 1) &&
                    (lPredCount > lLinksMap.GetLinksWithSameTables(lCurrentLink).count()))
            {
                bool lStartFromSameTable = true;
                foreach(const GexDbPlugin_Mapping_Link &lValue, lLinksMap)
                {
                    if (lValue.m_strSqlTable1 != lCurrentLink.m_strSqlTable1)
                    {
                        lStartFromSameTable = false;
                        break;
                    }
                }
                if (lStartFromSameTable)
                {
                    lPredLinks.prepend(lCurrentLink);
                    // dependency of several tables on one table detected
                    mDbArchitecture->AddMultipleTableLinkFromLegacy(
                                lPredLinks,
                                lTestingStage);
                    foreach(const GexDbPlugin_Mapping_Link &lLink, lPredLinks)
                        lLinksMap.remove(lLink.m_strLinkName);
                }
            }
        }
    }
    // then search simple dependencies on one table
    it.toFront();
    while (it.hasNext())
    {
        it.next();
        GexDbPlugin_Mapping_Link lCurrentLink = it.value();
        // search all multiple link on the same pair
        QList<GexDbPlugin_Mapping_Link> lIncludedLinks;
        lIncludedLinks = lLinksMap.GetLinksWithSameTables(lCurrentLink);
        if (lIncludedLinks.count() == 1)
        {
            mDbArchitecture->AddSimpleLinkFromLegacy(
                        lCurrentLink,
                        lTestingStage);
            lLinksMap.remove(lCurrentLink.m_strLinkName);
        }
        else if (lIncludedLinks.count() > 1)
        {
            mDbArchitecture->AddMultipleKeyLinkFromLegacy(
                        lIncludedLinks,
                        lTestingStage);
            foreach(const GexDbPlugin_Mapping_Link &lLink, lIncludedLinks)
                lLinksMap.remove(lLink.m_strLinkName);
        }
        else
        {
            // TODO 6408 add gslog
            return false;
        }
        // reset iterator after item removal
        it = QMapIterator<QString, GexDbPlugin_Mapping_Link>(lLinksMap);
    }



    return true;
}

//////////////////////////////////////////////////////////////////////
// Load Dynamic meta-data for Test Conditions
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::LoadDynamicTCMetaData(TestingStage eTestingStage, unsigned int uiDbVersion_Build)
{
    QString                         tablePrefix;
    GexDbPlugin_Mapping_FieldMap *  pMetadataFieldMap = NULL;
    GexDbPlugin_Mapping_Field       tcMetadataField;

    switch(eTestingStage)
    {
    case    eElectTest: tablePrefix         = GEXDB_PLUGIN_GALAXY_ETEST_TABLE_PREFIX;
        pMetadataFieldMap   = &m_mapFields_GexToRemote_Et;
        break;

    case    eWaferTest: tablePrefix         = GEXDB_PLUGIN_GALAXY_WTEST_TABLE_PREFIX;
        pMetadataFieldMap   = &m_mapFields_GexToRemote_Wt;
        break;

    case    eFinalTest: tablePrefix         = GEXDB_PLUGIN_GALAXY_FTEST_TABLE_PREFIX;
        pMetadataFieldMap   = &m_mapFields_GexToRemote_Ft;
        break;

    default:            GSLOG(SYSLOG_SEV_ERROR, "Unknown testing stage");
        return false;
    }

    if (uiDbVersion_Build >= 18)
    {
        GexDbPlugin_Query     clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
        clGexDbQuery.setForwardOnly(true);

        // Query ET mapping for Test Conditions
        QString metadataName;
        QString fullFieldName;
        QString linkName        = QString("%1test_conditions-%2splitlot").arg(tablePrefix).arg(tablePrefix);
        QString sqlTableName    = QString("%1test_conditions").arg(tablePrefix);
        QString strQuery;

        strQuery = "SELECT * \n";
        strQuery += "FROM "+m_pclDatabaseConnector->m_strSchemaName+"." + sqlTableName +" \n";
        strQuery += "WHERE splitlot_id = 0 and test_info_id = 0 and test_type = '-'";

        // HTH-TOCHECK
        if(!clGexDbQuery.Execute(strQuery))
            return false;

        // Read extracted ET Test conditions
        while(clGexDbQuery.Next())
        {
            QString   fieldName;
            int       fieldIndex = -1;
            bool      lastColumn = false;

            for (int idxField = 1; idxField <= GEXDB_PLUGIN_GALAXY_TEST_CONDITIONS_COLUMNS && !lastColumn; ++idxField)
            {
                fieldName     = "condition_" + QString::number(idxField);
                fieldIndex    = clGexDbQuery.record().indexOf(fieldName);

                if (fieldIndex != -1)
                {
                    if (clGexDbQuery.value(fieldIndex).isNull())
                        lastColumn = true;
                    else
                    {
                        metadataName    = clGexDbQuery.value(fieldIndex).toString();
                        fullFieldName   = sqlTableName + "." + fieldName;

                        GSLOG(SYSLOG_SEV_INFORMATIONAL, (QString("Inserted ET Meta-data %1 [%2]").arg(
                                                             metadataName.toLatin1().constData())
                                                         .arg(fullFieldName.toLatin1().constData())).toLatin1().constData());

                        tcMetadataField = GexDbPlugin_Mapping_Field(metadataName, "",
                                                                    sqlTableName, fullFieldName,
                                                                    linkName,
                                                                    false, "N", false, false, false,
                                                                    false, false, false, false);

                        tcMetadataField.setTestCondition(true);

                        // Insert new metadata field into the map
                        pMetadataFieldMap->insert(metadataName, tcMetadataField);
                    }
                }
            }
        }
    }

    return true;
}

//////////////////////////////////////////////////////////////////////
// Load static meta-data (mapping + links)
//////////////////////////////////////////////////////////////////////
void GexDbPlugin_Galaxy::LoadStaticMetaData()
{
    // Check database connection
    if(!GexDbPlugin_Base::ConnectToCorporateDb())
        return;

    LoadStaticMetaData(m_uiDbVersionBuild);
}

//////////////////////////////////////////////////////////////////////
// Load static meta-data (mapping + links)
//////////////////////////////////////////////////////////////////////
void GexDbPlugin_Galaxy::LoadStaticMetaData(unsigned int uiDbVersion_Build)
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // E-Test
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Gex fields <-> Galaxy DB fields mapping
    // 1. Fields exported in the GEX GUI
    m_mapFields_GexToRemote_Et[m_gexLabelFilterChoices[GEX_QUERY_FILTER_ORIGIN]] =
            GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_ORIGIN], m_gexLabelFilterChoices[GEX_QUERY_FILTER_ORIGIN],
                                      "et_splitlot",		"et_splitlot.data_provider",		"",								true, "N", false, false, false, true, true, false, true);
    m_mapFields_GexToRemote_Et[m_gexLabelFilterChoices[GEX_QUERY_FILTER_FACILITY]] =
            GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_FACILITY], m_gexLabelFilterChoices[GEX_QUERY_FILTER_FACILITY],
                                      "et_splitlot",		"et_splitlot.facil_id",				"",								true, "N", false, false, false, true, true, false, true);
    m_mapFields_GexToRemote_Et[m_gexLabelFilterChoices[GEX_QUERY_FILTER_FAMILY]] =
            GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_FAMILY], m_gexLabelFilterChoices[GEX_QUERY_FILTER_FAMILY],
                                      "et_splitlot",		"et_splitlot.famly_id",				"",								true, "N", false, false, false, true, true, false, true);
    //    m_mapFields_GexToRemote_Et[m_gexLabelFilterChoices[GEX_QUERY_FILTER_LOT]] =
    //            GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_LOT], m_gexLabelFilterChoices[GEX_QUERY_FILTER_LOT],
    //                                      "et_lot",			"et_lot.lot_id",					"et_lot-et_splitlot",			true, "N", false, false, false, true, true, false, true);
    m_mapFields_GexToRemote_Et[m_gexLabelFilterChoices[GEX_QUERY_FILTER_LOT]] =
            GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_LOT], m_gexLabelFilterChoices[GEX_QUERY_FILTER_LOT],
                                      "et_splitlot",		"et_splitlot.lot_id",				"",                             true, "N", false, false, false, true, true, false, true);



    m_mapFields_GexToRemote_Et[m_gexLabelFilterChoices[GEX_QUERY_FILTER_OPERATOR]] =
            GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_OPERATOR], m_gexLabelFilterChoices[GEX_QUERY_FILTER_OPERATOR],
                                      "et_splitlot",		"et_splitlot.oper_nam",				"",								true, "N", false, false, false, true, true, false, true);
    m_mapFields_GexToRemote_Et[m_gexLabelFilterChoices[GEX_QUERY_FILTER_PROCESS]] =
            GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_PROCESS], m_gexLabelFilterChoices[GEX_QUERY_FILTER_PROCESS],
                                      "et_splitlot",		"et_splitlot.proc_id",				"",								true, "N", false, false, false, true, true, false, true);
    m_mapFields_GexToRemote_Et[m_gexLabelFilterChoices[GEX_QUERY_FILTER_PRODUCT]] =
            GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_PRODUCT], m_gexLabelFilterChoices[GEX_QUERY_FILTER_PRODUCT],
                                      "et_splitlot",			"et_splitlot.product_name",				"",			true, "N", false, false, false, true, true, false, true);
    m_mapFields_GexToRemote_Et[m_gexLabelFilterChoices[GEX_QUERY_FILTER_PROGRAMNAME]] =
            GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_PROGRAMNAME], m_gexLabelFilterChoices[GEX_QUERY_FILTER_PROGRAMNAME],
                                      "et_splitlot",		"et_splitlot.job_nam",				"",								true, "N", false, false, false, true, true, false, true);
    m_mapFields_GexToRemote_Et[m_gexLabelFilterChoices[GEX_QUERY_FILTER_PROGRAMREVISION]] =
            GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_PROGRAMREVISION], m_gexLabelFilterChoices[GEX_QUERY_FILTER_PROGRAMREVISION],
                                      "et_splitlot",		"et_splitlot.job_rev",				"",								true, "N", false, false, false, true, true, false, true);
    m_mapFields_GexToRemote_Et[m_gexLabelFilterChoices[GEX_QUERY_FILTER_SUBLOT]] =
            GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_SUBLOT], m_gexLabelFilterChoices[GEX_QUERY_FILTER_SUBLOT],
                                      "et_splitlot",		"et_splitlot.sublot_id",			"",								true, "N", false, false, false, true, true, false, true);
    m_mapFields_GexToRemote_Et[m_gexLabelFilterChoices[GEX_QUERY_FILTER_TESTERNAME]] =
            GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_TESTERNAME], m_gexLabelFilterChoices[GEX_QUERY_FILTER_TESTERNAME],
                                      "et_splitlot",		"et_splitlot.tester_name",			"",								true, "N", false, false, false, true, true, false, true);
    m_mapFields_GexToRemote_Et[m_gexLabelFilterChoices[GEX_QUERY_FILTER_TESTERTYPE]] =
            GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_TESTERTYPE], m_gexLabelFilterChoices[GEX_QUERY_FILTER_TESTERTYPE],
                                      "et_splitlot",		"et_splitlot.tester_type",			"",								true, "N", false, false, false, true, true, false, true);
    m_mapFields_GexToRemote_Et[m_gexLabelFilterChoices[GEX_QUERY_FILTER_WAFERID]] =
            GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_WAFERID], m_gexLabelFilterChoices[GEX_QUERY_FILTER_WAFERID],
                                      "et_splitlot",		"et_splitlot.wafer_id",				"",								true, "N", false, false, false, true, true, false, true);
    m_mapFields_GexToRemote_Et[m_gexLabelFilterChoices[GEX_QUERY_FILTER_SITENBR]] =
            GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_SITENBR], m_gexLabelFilterChoices[GEX_QUERY_FILTER_SITENBR],
                                      "et_run",			"et_run.site_no",					"et_run-et_splitlot",			true, "N", false, false, false, false, false, true, false);
    // 2. Fields exported in the GEX GUI, but with no standard GEX correspondance
    m_mapFields_GexToRemote_Et[GEXDB_PLUGIN_DBFIELD_WAFER_NB] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_WAFER_NB, GEXDB_PLUGIN_DBFIELD_WAFER_NB,
                                      "et_splitlot",		"et_splitlot.wafer_nb",				"",								true, "N", false, false, false, false, true, true, false);
    m_mapFields_GexToRemote_Et[GEXDB_PLUGIN_DBFIELD_ETEST_SITE_CONFIG] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_ETEST_SITE_CONFIG, GEXDB_PLUGIN_DBFIELD_ETEST_SITE_CONFIG,
                                      "et_splitlot",		"et_splitlot.site_config",			"",								true, "N", false, false, false, false, true, false, false);
    m_mapFields_GexToRemote_Et[GEXDB_PLUGIN_DBFIELD_DATATYPE] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_DATATYPE, GEXDB_PLUGIN_DBFIELD_DATATYPE,
                                      "et_splitlot",		"et_splitlot.data_type",			"",								true, "N", false, false, false, true, true, false, true);
    m_mapFields_GexToRemote_Et[GEXDB_PLUGIN_DBFIELD_TRACKING_LOT] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_TRACKING_LOT, GEXDB_PLUGIN_DBFIELD_TRACKING_LOT,
                                      "et_lot",			"et_lot.tracking_lot_id",			"et_lot-et_splitlot",			true, "N", false, false, false, true, true, false, true);
    m_mapFields_GexToRemote_Et[GEXDB_PLUGIN_DBFIELD_DAY] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_DAY, GEXDB_PLUGIN_DBFIELD_DAY,
                                      "et_splitlot",		"et_splitlot.day",				"",									true, "N", true, false, false, true, true, false, true);
    m_mapFields_GexToRemote_Et[GEXDB_PLUGIN_DBFIELD_WEEK] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_WEEK, GEXDB_PLUGIN_DBFIELD_WEEK,
                                      "et_splitlot",		"et_splitlot.week_nb",				"",								true, "N", true, false, false, true, true, true, true);
    m_mapFields_GexToRemote_Et[GEXDB_PLUGIN_DBFIELD_MONTH] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_MONTH, GEXDB_PLUGIN_DBFIELD_MONTH,
                                      "et_splitlot",		"et_splitlot.month_nb",				"",								true, "N", true, false, false, true, true, true, true);
    m_mapFields_GexToRemote_Et[GEXDB_PLUGIN_DBFIELD_QUARTER] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_QUARTER, GEXDB_PLUGIN_DBFIELD_QUARTER,
                                      "et_splitlot",		"et_splitlot.quarter_nb",			"",								true, "N", true, false, false, true, true, true, true);
    m_mapFields_GexToRemote_Et[GEXDB_PLUGIN_DBFIELD_YEAR] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_YEAR, GEXDB_PLUGIN_DBFIELD_YEAR,
                                      "et_splitlot",		"et_splitlot.year_nb",				"",								true, "N", true, false, false, true, true, true, true);
    m_mapFields_GexToRemote_Et[GEXDB_PLUGIN_DBFIELD_YEAR_WEEK] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_YEAR_WEEK, GEXDB_PLUGIN_DBFIELD_YEAR_WEEK,
                                      "et_splitlot",		"et_splitlot.year_and_week",			"",								true, "N", true, false, false, true, true, false, true);
    m_mapFields_GexToRemote_Et[GEXDB_PLUGIN_DBFIELD_YEAR_MONTH] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_YEAR_MONTH, GEXDB_PLUGIN_DBFIELD_YEAR_MONTH,
                                      "et_splitlot",		"et_splitlot.year_and_month",			"",								true, "N", true, false, false, true, true, false, true);
    m_mapFields_GexToRemote_Et[GEXDB_PLUGIN_DBFIELD_YEAR_QUARTER] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_YEAR_QUARTER, GEXDB_PLUGIN_DBFIELD_YEAR_QUARTER,
                                      "et_splitlot",		"et_splitlot.year_and_quarter",			"",								true, "N", true, false, false, true, true, false, true);

    // 3. Fields not exported in the GEX GUI, but needed in our queries
    m_mapFields_GexToRemote_Et[GEXDB_PLUGIN_DBFIELD_TESTNAME] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_TESTNAME, GEXDB_PLUGIN_DBFIELD_TESTNAME,
                                      "et_ptest_info",	"et_ptest_info.tname",				"et_ptest_info-et_splitlot",	false, "N", false, false, false, false, false, false, false);
    m_mapFields_GexToRemote_Et[GEXDB_PLUGIN_DBFIELD_TESTNUM] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_TESTNUM, GEXDB_PLUGIN_DBFIELD_TESTNUM,
                                      "et_ptest_info",	"et_ptest_info.tnum",				"et_ptest_info-et_splitlot",	false, "N", false, false, false, false, false, true, false);
    m_mapFields_GexToRemote_Et[GEXDB_PLUGIN_DBFIELD_STARTDATETIME] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_STARTDATETIME, GEXDB_PLUGIN_DBFIELD_STARTDATETIME,
                                      "et_splitlot",		"et_splitlot.start_t",				"",								false, "N", true, false, false, true, false, true, true);
    m_mapFields_GexToRemote_Et[GEXDB_PLUGIN_DBFIELD_HBIN] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_HBIN, GEXDB_PLUGIN_DBFIELD_HBIN,
                                      "et_hbin",			"et_hbin.hbin_no",					"et_hbin-et_splitlot",			false, "H", false, false, false, false, true, true, false);
    m_mapFields_GexToRemote_Et[GEXDB_PLUGIN_DBFIELD_SBIN] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_SBIN, GEXDB_PLUGIN_DBFIELD_SBIN,
                                      "et_sbin",			"et_sbin.sbin_no",					"et_sbin-et_splitlot",			false, "S", false, false, false, false, true, true, false);
    m_mapFields_GexToRemote_Et[GEXDB_PLUGIN_DBFIELD_HBIN_NAME] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_HBIN_NAME, GEXDB_PLUGIN_DBFIELD_HBIN_NAME,
                                      "et_hbin",			"et_hbin.hbin_name",				"et_hbin-et_splitlot",			false, "H", false, false, false, false, false, false, false);
    m_mapFields_GexToRemote_Et[GEXDB_PLUGIN_DBFIELD_SBIN_NAME] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_SBIN_NAME, GEXDB_PLUGIN_DBFIELD_SBIN_NAME,
                                      "et_sbin",			"et_sbin.sbin_name",				"et_sbin-et_splitlot",			false, "S", false, false, false, false, false, false, false);
    m_mapFields_GexToRemote_Et[GEXDB_PLUGIN_DBFIELD_HBIN_PF] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_HBIN_PF, GEXDB_PLUGIN_DBFIELD_HBIN_PF,
                                      "et_hbin",			"et_hbin.hbin_cat",					"et_hbin-et_splitlot",			false, "H", false, false, false, false, true, false, false);
    m_mapFields_GexToRemote_Et[GEXDB_PLUGIN_DBFIELD_SBIN_PF] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_SBIN_PF, GEXDB_PLUGIN_DBFIELD_SBIN_PF,
                                      "et_sbin",			"et_sbin.sbin_cat",					"et_sbin-et_splitlot",			false, "S", false, false, false, false, true, false, false);
    m_mapFields_GexToRemote_Et[GEXDB_PLUGIN_DBFIELD_USER_TXT] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_USER_TXT, GEXDB_PLUGIN_DBFIELD_USER_TXT,
                                      "et_splitlot",		"et_splitlot.user_txt",				"",								false, "N", false, false, false, false, false, false, false);
    m_mapFields_GexToRemote_Et[GEXDB_PLUGIN_DBFIELD_VALID_SPLITLOT] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_VALID_SPLITLOT, GEXDB_PLUGIN_DBFIELD_VALID_SPLITLOT,
                                      "et_splitlot",		"et_splitlot.valid_splitlot",		"",								false, "N", false, false, false, false, false, false, false);
    m_mapFields_GexToRemote_Et[GEXDB_PLUGIN_DBFIELD_PROD_DATA] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_PROD_DATA, GEXDB_PLUGIN_DBFIELD_PROD_DATA,
                                      "et_splitlot",		"et_splitlot.prod_data",			"",								false, "N", false, false, false, false, false, false, false);
    m_mapFields_GexToRemote_Et[GEXDB_PLUGIN_DBFIELD_PARTS] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_PARTS, GEXDB_PLUGIN_DBFIELD_PARTS,
                                      "et_splitlot",		"et_splitlot.nb_parts",				"",								false, "N", false, false, true, true, false, true, true);
    m_mapFields_GexToRemote_Et[GEXDB_PLUGIN_DBFIELD_PARTS_SAMPLE] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_PARTS_SAMPLE, GEXDB_PLUGIN_DBFIELD_PARTS_SAMPLE,
                                      "et_splitlot",		"et_splitlot.nb_parts",				"",								false, "N", false, false, true, true, false, true, true);
    m_mapFields_GexToRemote_Et[GEXDB_PLUGIN_DBFIELD_PARTS_GOOD] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_PARTS_GOOD, GEXDB_PLUGIN_DBFIELD_PARTS_GOOD,
                                      "et_splitlot",		"et_splitlot.nb_parts_good",		"",								false, "N", false, false, true, true, false, true, true);
    m_mapFields_GexToRemote_Et[GEXDB_PLUGIN_DBFIELD_LOT_PARTS] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_LOT_PARTS, GEXDB_PLUGIN_DBFIELD_LOT_PARTS,
                                      "et_lot",			"et_lot.nb_parts",					"et_lot-et_splitlot",			false, "N", false, false, true, true, false, true, true);
    m_mapFields_GexToRemote_Et[GEXDB_PLUGIN_DBFIELD_LOT_PARTS_GOOD] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_LOT_PARTS_GOOD, GEXDB_PLUGIN_DBFIELD_LOT_PARTS_GOOD,
                                      "et_lot",			"et_lot.nb_parts_good",				"et_lot-et_splitlot",			false, "N", false, false, true, true, false, true, true);
    m_mapFields_GexToRemote_Et[GEXDB_PLUGIN_DBFIELD_WAFER_PARTS] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_WAFER_PARTS, GEXDB_PLUGIN_DBFIELD_WAFER_PARTS,
                                      "et_wafer_info",	"et_wafer_info.nb_parts",			"et_wafer_info-et_splitlot",	false, "N", false, false, true, true, false, true, true);
    m_mapFields_GexToRemote_Et[GEXDB_PLUGIN_DBFIELD_WAFER_PARTS_GOOD] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_WAFER_PARTS_GOOD, GEXDB_PLUGIN_DBFIELD_WAFER_PARTS_GOOD,
                                      "et_wafer_info",	"et_wafer_info.nb_parts_good",		"et_wafer_info-et_splitlot",	false, "N", false, false, true, true, false, true, true);
    m_mapFields_GexToRemote_Et[GEXDB_PLUGIN_DBFIELD_WAFER_GROSS_DIE] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_WAFER_GROSS_DIE, GEXDB_PLUGIN_DBFIELD_WAFER_GROSS_DIE,
                                      "et_wafer_info",	"et_wafer_info.gross_die",			"et_wafer_info-et_splitlot",	false, "N", false, false, true, true, false, true, true);

    // Links in Galaxy DB
    m_mapLinks_Remote_Et["et_lot_sbin-et_lot"] =
            GexDbPlugin_Mapping_Link("et_lot_sbin-et_lot", "et_lot_sbin", "et_lot_sbin.lot_id", "et_lot", "et_lot.lot_id", "et_lot_sbin-et_lot-2");
    m_mapLinks_Remote_Et["et_lot_sbin-et_lot-2"] =
            GexDbPlugin_Mapping_Link("et_lot_sbin-et_lot-2", "et_lot_sbin", "et_lot_sbin.product_name", "et_lot", "et_lot.product_name", "et_lot-et_splitlot");
    m_mapLinks_Remote_Et["et_lot_hbin-et_lot"] =
            GexDbPlugin_Mapping_Link("et_lot_hbin-et_lot", "et_lot_hbin", "et_lot_hbin.lot_id", "et_lot", "et_lot.lot_id", "et_lot_hbin-et_lot-2");
    m_mapLinks_Remote_Et["et_lot_hbin-et_lot-2"] =
            GexDbPlugin_Mapping_Link("et_lot_hbin-et_lot-2", "et_lot_hbin", "et_lot_hbin.product_name", "et_lot", "et_lot.product_name", "et_lot-et_splitlot");
    m_mapLinks_Remote_Et["product-et_wafer_info"] =
            GexDbPlugin_Mapping_Link("product-et_wafer_info", "product", "product.product_name", "et_wafer_info", "et_wafer_info.product_name", "et_wafer_info-et_splitlot");
    m_mapLinks_Remote_Et["et_lot-et_splitlot"] =
            GexDbPlugin_Mapping_Link("et_lot-et_splitlot", "et_lot", "et_lot.lot_id", "et_splitlot", "et_splitlot.lot_id", "et_lot-et_splitlot-2");
    m_mapLinks_Remote_Et["et_lot-et_splitlot-2"] =
            GexDbPlugin_Mapping_Link("et_lot-et_splitlot-2", "et_lot", "et_lot.product_name", "et_splitlot", "et_splitlot.product_name", "");
    m_mapLinks_Remote_Et["et_wafer_info-et_lot"] =
            GexDbPlugin_Mapping_Link("et_wafer_info-et_lot", "et_wafer_info", "et_wafer_info.lot_id", "et_lot", "et_lot.lot_id", "et_lot-et_splitlot");
    m_mapLinks_Remote_Et["et_ptest_info-et_splitlot"] =
            GexDbPlugin_Mapping_Link("et_ptest_info-et_splitlot", "et_ptest_info", "et_ptest_info.splitlot_id", "et_splitlot", "et_splitlot.splitlot_id", "");
    m_mapLinks_Remote_Et["et_hbin-et_splitlot"] =
            GexDbPlugin_Mapping_Link("et_hbin-et_splitlot", "et_hbin", "et_hbin.splitlot_id", "et_splitlot", "et_splitlot.splitlot_id", "");
    m_mapLinks_Remote_Et["et_sbin-et_splitlot"] =
            GexDbPlugin_Mapping_Link("et_sbin-et_splitlot", "et_sbin", "et_sbin.splitlot_id", "et_splitlot", "et_splitlot.splitlot_id", "");
    m_mapLinks_Remote_Et["et_parts_stats_samples-et_splitlot"] =
            GexDbPlugin_Mapping_Link("et_parts_stats_samples-et_splitlot", "et_parts_stats_samples", "et_parts_stats_samples.splitlot_id", "et_splitlot", "et_splitlot.splitlot_id", "");
    m_mapLinks_Remote_Et["et_wafer_info-et_splitlot"] =
            GexDbPlugin_Mapping_Link("et_wafer_info-et_splitlot", "et_wafer_info", "et_wafer_info.lot_id", "et_splitlot", "et_splitlot.lot_id", "et_wafer_info-et_splitlot-2");
    m_mapLinks_Remote_Et["et_wafer_info-et_splitlot-2"] =
            GexDbPlugin_Mapping_Link("et_wafer_info-et_splitlot-2", "et_wafer_info", "et_wafer_info.wafer_id", "et_splitlot", "et_splitlot.wafer_id", "");
    m_mapLinks_Remote_Et["et_run-et_splitlot"] =
            GexDbPlugin_Mapping_Link("et_run-et_splitlot", "et_run", "et_run.splitlot_id", "et_splitlot", "et_splitlot.splitlot_id", "");

    if (uiDbVersion_Build >= 18)
    {
        m_mapLinks_Remote_Et["et_test_conditions-et_splitlot"] =
                GexDbPlugin_Mapping_Link("et_test_conditions-et_splitlot", "et_test_conditions","et_test_conditions.splitlot_id", "et_splitlot","et_splitlot.splitlot_id", "");
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Wafer Sort
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Gex fields <-> Galaxy DB fields mapping
    // 1. Fields exported in the GEX GUI
    m_mapFields_GexToRemote_Wt[m_gexLabelFilterChoices[GEX_QUERY_FILTER_BURNIN]] =
            GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_BURNIN],m_gexLabelFilterChoices[GEX_QUERY_FILTER_BURNIN],
                                      "wt_splitlot",		"wt_splitlot.burn_tim",				"",true, "N", false, false, false, true, false, true, true);
    m_mapFields_GexToRemote_Wt[m_gexLabelFilterChoices[GEX_QUERY_FILTER_ORIGIN]] =
            GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_ORIGIN], m_gexLabelFilterChoices[GEX_QUERY_FILTER_ORIGIN],
                                      "wt_splitlot",		"wt_splitlot.data_provider",		"",								true, "N", false, false, false, true, true, false, true);
    m_mapFields_GexToRemote_Wt[m_gexLabelFilterChoices[GEX_QUERY_FILTER_DIBNAME]] =
            GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_DIBNAME], m_gexLabelFilterChoices[GEX_QUERY_FILTER_DIBNAME],
                                      "wt_splitlot",		"wt_splitlot.dib_id",				"",								true, "N", false, false, false, true, true, false, true);
    m_mapFields_GexToRemote_Wt[m_gexLabelFilterChoices[GEX_QUERY_FILTER_DIBTYPE]] =
            GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_DIBTYPE], m_gexLabelFilterChoices[GEX_QUERY_FILTER_DIBTYPE],
                                      "wt_splitlot",		"wt_splitlot.dib_typ",				"",								true, "N", false, false, false, true, true, false, true);
    m_mapFields_GexToRemote_Wt[m_gexLabelFilterChoices[GEX_QUERY_FILTER_FACILITY]] =
            GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_FACILITY], m_gexLabelFilterChoices[GEX_QUERY_FILTER_FACILITY],
                                      "wt_splitlot",		"wt_splitlot.facil_id",				"",								true, "N", false, false, false, true, true, false, true);
    m_mapFields_GexToRemote_Wt[m_gexLabelFilterChoices[GEX_QUERY_FILTER_FAMILY]] =
            GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_FAMILY], m_gexLabelFilterChoices[GEX_QUERY_FILTER_FAMILY],
                                      "wt_splitlot",		"wt_splitlot.famly_id",				"",								true, "N", false, false, false, true, true, false, true);
    m_mapFields_GexToRemote_Wt[m_gexLabelFilterChoices[GEX_QUERY_FILTER_FLOOR]] =
            GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_FLOOR], m_gexLabelFilterChoices[GEX_QUERY_FILTER_FLOOR],
                                      "wt_splitlot",		"wt_splitlot.floor_id",				"",								true, "N", false, false, false, true, true, false, true);
    m_mapFields_GexToRemote_Wt[m_gexLabelFilterChoices[GEX_QUERY_FILTER_FREQUENCYSTEP]] =
            GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_FREQUENCYSTEP], m_gexLabelFilterChoices[GEX_QUERY_FILTER_FREQUENCYSTEP],
                                      "wt_splitlot",		"wt_splitlot.oper_frq",				"",								true, "N", false, false, false, true, true, false, true);
    m_mapFields_GexToRemote_Wt[m_gexLabelFilterChoices[GEX_QUERY_FILTER_LOADBOARDNAME]] =
            GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_LOADBOARDNAME], m_gexLabelFilterChoices[GEX_QUERY_FILTER_LOADBOARDNAME],
                                      "wt_splitlot",		"wt_splitlot.loadboard_id",			"",								true, "N", false, false, false, true, true, false, true);
    m_mapFields_GexToRemote_Wt[m_gexLabelFilterChoices[GEX_QUERY_FILTER_LOADBOARDTYPE]] =
            GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_LOADBOARDTYPE], m_gexLabelFilterChoices[GEX_QUERY_FILTER_LOADBOARDTYPE],
                                      "wt_splitlot",		"wt_splitlot.loadboard_typ",		"",								true, "N", false, false, false, true, true, false, true);
    //    m_mapFields_GexToRemote_Wt[m_gexLabelFilterChoices[GEX_QUERY_FILTER_LOT]] =
    //            GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_LOT], m_gexLabelFilterChoices[GEX_QUERY_FILTER_LOT],
    //                                      "wt_lot",			"wt_lot.lot_id",					"wt_lot-wt_splitlot",			true, "N", false, false, false, true, true, false, true);
    m_mapFields_GexToRemote_Wt[m_gexLabelFilterChoices[GEX_QUERY_FILTER_LOT]] =
            GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_LOT], m_gexLabelFilterChoices[GEX_QUERY_FILTER_LOT],
                                      "wt_splitlot",		"wt_splitlot.lot_id",				"",                             true, "N", false, false, false, true, true, false, true);


    m_mapFields_GexToRemote_Wt[m_gexLabelFilterChoices[GEX_QUERY_FILTER_OPERATOR]] =
            GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_OPERATOR], m_gexLabelFilterChoices[GEX_QUERY_FILTER_OPERATOR],
                                      "wt_splitlot",		"wt_splitlot.oper_nam",				"",								true, "N", false, false, false, true, true, false, true);
    m_mapFields_GexToRemote_Wt[m_gexLabelFilterChoices[GEX_QUERY_FILTER_PACKAGE]] =
            GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_PACKAGE], m_gexLabelFilterChoices[GEX_QUERY_FILTER_PACKAGE],
                                      "wt_splitlot",		"wt_splitlot.pkg_typ",				"",								true, "N", false, false, false, true, true, false, true);
    m_mapFields_GexToRemote_Wt[m_gexLabelFilterChoices[GEX_QUERY_FILTER_PROBERNAME]] =
            GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_PROBERNAME], m_gexLabelFilterChoices[GEX_QUERY_FILTER_PROBERNAME],
                                      "wt_splitlot",		"wt_splitlot.handler_id",			"",								true, "N", false, false, false, true, true, false, true);
    m_mapFields_GexToRemote_Wt[m_gexLabelFilterChoices[GEX_QUERY_FILTER_PROBERTYPE]] =
            GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_PROBERTYPE], m_gexLabelFilterChoices[GEX_QUERY_FILTER_PROBERTYPE],
                                      "wt_splitlot",		"wt_splitlot.handler_typ",			"",								true, "N", false, false, false, true, true, false, true);
    m_mapFields_GexToRemote_Wt[m_gexLabelFilterChoices[GEX_QUERY_FILTER_PROCESS]] =
            GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_PROCESS], m_gexLabelFilterChoices[GEX_QUERY_FILTER_PROCESS],
                                      "wt_splitlot",		"wt_splitlot.proc_id",				"",								true, "N", false, false, false, true, true, false, true);
    m_mapFields_GexToRemote_Wt[m_gexLabelFilterChoices[GEX_QUERY_FILTER_PRODUCT]] =
            GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_PRODUCT], m_gexLabelFilterChoices[GEX_QUERY_FILTER_PRODUCT],
                                      "wt_splitlot",			"wt_splitlot.product_name",				"",			true, "N", false, false, false, true, true, false, true);
    m_mapFields_GexToRemote_Wt[m_gexLabelFilterChoices[GEX_QUERY_FILTER_PROGRAMNAME]] =
            GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_PROGRAMNAME], m_gexLabelFilterChoices[GEX_QUERY_FILTER_PROGRAMNAME],
                                      "wt_splitlot",		"wt_splitlot.job_nam",				"",								true, "N", false, false, false, true, true, false, true);
    m_mapFields_GexToRemote_Wt[m_gexLabelFilterChoices[GEX_QUERY_FILTER_PROGRAMREVISION]] =
            GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_PROGRAMREVISION], m_gexLabelFilterChoices[GEX_QUERY_FILTER_PROGRAMREVISION],
                                      "wt_splitlot",		"wt_splitlot.job_rev",				"",								true, "N", false, false, false, true, true, false, true);
    m_mapFields_GexToRemote_Wt[m_gexLabelFilterChoices[GEX_QUERY_FILTER_RETESTNBR]] =
            GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_RETESTNBR], m_gexLabelFilterChoices[GEX_QUERY_FILTER_RETESTNBR],
                                      "wt_splitlot",		"wt_splitlot.retest_index",			"",								true, "N", false, false, false, false, false, true, false);
    m_mapFields_GexToRemote_Wt[m_gexLabelFilterChoices[GEX_QUERY_FILTER_SUBLOT]] =
            GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_SUBLOT], m_gexLabelFilterChoices[GEX_QUERY_FILTER_SUBLOT],
                                      "wt_splitlot",		"wt_splitlot.sublot_id",			"",								true, "N", false, false, false, true, true, false, true);
    m_mapFields_GexToRemote_Wt[m_gexLabelFilterChoices[GEX_QUERY_FILTER_TEMPERATURE]] =
            GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_TEMPERATURE], m_gexLabelFilterChoices[GEX_QUERY_FILTER_TEMPERATURE],
                                      "wt_splitlot",		"wt_splitlot.tst_temp",				"",								true, "N", false, false, false, true, true, false, true);
    m_mapFields_GexToRemote_Wt[m_gexLabelFilterChoices[GEX_QUERY_FILTER_TESTERNAME]] =
            GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_TESTERNAME], m_gexLabelFilterChoices[GEX_QUERY_FILTER_TESTERNAME],
                                      "wt_splitlot",		"wt_splitlot.tester_name",			"",								true, "N", false, false, false, true, true, false, true);
    m_mapFields_GexToRemote_Wt[m_gexLabelFilterChoices[GEX_QUERY_FILTER_TESTERTYPE]] =
            GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_TESTERTYPE], m_gexLabelFilterChoices[GEX_QUERY_FILTER_TESTERTYPE],
                                      "wt_splitlot",		"wt_splitlot.tester_type",			"",								true, "N", false, false, false, true, true, false, true);
    m_mapFields_GexToRemote_Wt[m_gexLabelFilterChoices[GEX_QUERY_FILTER_TESTCODE]] =
            GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_TESTCODE], m_gexLabelFilterChoices[GEX_QUERY_FILTER_TESTCODE],
                                      "wt_splitlot",		"wt_splitlot.test_cod",				"",								true, "N", false, false, false, true, true, false, true);
    m_mapFields_GexToRemote_Wt[m_gexLabelFilterChoices[GEX_QUERY_FILTER_WAFERID]] =
            GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_WAFERID], m_gexLabelFilterChoices[GEX_QUERY_FILTER_WAFERID],
                                      "wt_splitlot",		"wt_splitlot.wafer_id",				"",								true, "N", false, false, false, true, true, false, true);
    m_mapFields_GexToRemote_Wt[m_gexLabelFilterChoices[GEX_QUERY_FILTER_SITENBR]] =
            GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_SITENBR], m_gexLabelFilterChoices[GEX_QUERY_FILTER_SITENBR],
                                      "wt_parts_stats_samples",	"wt_parts_stats_samples.site_no",			"wt_parts_stats_samples-wt_splitlot",	true, "N", false, false, false, false, false, true, false);
    // 2. Fields exported in the GEX GUI, but with no standard GEX correspondance
    m_mapFields_GexToRemote_Wt[GEXDB_PLUGIN_DBFIELD_WAFER_NB] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_WAFER_NB, GEXDB_PLUGIN_DBFIELD_WAFER_NB,
                                      "wt_splitlot",		"wt_splitlot.wafer_nb",				"",								true, "N", false, false, false, false, true, true, false);
    m_mapFields_GexToRemote_Wt[GEXDB_PLUGIN_DBFIELD_DATATYPE] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_DATATYPE, GEXDB_PLUGIN_DBFIELD_DATATYPE,
                                      "wt_splitlot",		"wt_splitlot.data_type",			"",								true, "N", false, false, false, true, true, false, true);
    m_mapFields_GexToRemote_Wt[GEXDB_PLUGIN_DBFIELD_TRACKING_LOT] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_TRACKING_LOT, GEXDB_PLUGIN_DBFIELD_TRACKING_LOT,
                                      "wt_lot",			"wt_lot.tracking_lot_id",			"wt_lot-wt_splitlot",			true, "N", false, false, false, true, true, false, true);
    m_mapFields_GexToRemote_Wt[GEXDB_PLUGIN_DBFIELD_DAY] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_DAY, GEXDB_PLUGIN_DBFIELD_DAY,
                                      "wt_splitlot",		"wt_splitlot.day",				"",									true, "N", true, false, false, true, true, false, true);
    m_mapFields_GexToRemote_Wt[GEXDB_PLUGIN_DBFIELD_WEEK] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_WEEK, GEXDB_PLUGIN_DBFIELD_WEEK,
                                      "wt_splitlot",		"wt_splitlot.week_nb",				"",								true, "N", true, false, false, true, true, true, true);
    m_mapFields_GexToRemote_Wt[GEXDB_PLUGIN_DBFIELD_MONTH] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_MONTH, GEXDB_PLUGIN_DBFIELD_MONTH,
                                      "wt_splitlot",		"wt_splitlot.month_nb",				"",								true, "N", true, false, false, true, true, true, true);
    m_mapFields_GexToRemote_Wt[GEXDB_PLUGIN_DBFIELD_QUARTER] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_QUARTER, GEXDB_PLUGIN_DBFIELD_QUARTER,
                                      "wt_splitlot",		"wt_splitlot.quarter_nb",			"",								true, "N", true, false, false, true, true, true, true);
    m_mapFields_GexToRemote_Wt[GEXDB_PLUGIN_DBFIELD_YEAR] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_YEAR, GEXDB_PLUGIN_DBFIELD_YEAR,
                                      "wt_splitlot",		"wt_splitlot.year_nb",				"",								true, "N", true, false, false, true, true, true, true);
    m_mapFields_GexToRemote_Wt[GEXDB_PLUGIN_DBFIELD_YEAR_WEEK] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_YEAR_WEEK, GEXDB_PLUGIN_DBFIELD_YEAR_WEEK,
                                      "wt_splitlot",		"wt_splitlot.year_and_week",			"",								true, "N", true, false, false, true, true, false, true);
    m_mapFields_GexToRemote_Wt[GEXDB_PLUGIN_DBFIELD_YEAR_MONTH] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_YEAR_MONTH, GEXDB_PLUGIN_DBFIELD_YEAR_MONTH,
                                      "wt_splitlot",		"wt_splitlot.year_and_month",			"",								true, "N", true, false, false, true, true, false, true);
    m_mapFields_GexToRemote_Wt[GEXDB_PLUGIN_DBFIELD_YEAR_QUARTER] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_YEAR_QUARTER, GEXDB_PLUGIN_DBFIELD_YEAR_QUARTER,
                                      "wt_splitlot",		"wt_splitlot.year_and_quarter",			"",								true, "N", true, false, false, true, true, false, true);
    // 3. Fields not exported in the GEX GUI, but needed in our queries
    m_mapFields_GexToRemote_Wt[GEXDB_PLUGIN_DBFIELD_TESTNAME] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_TESTNAME, GEXDB_PLUGIN_DBFIELD_TESTNAME,
                                      "wt_ptest_info",	"wt_ptest_info.tname",				"wt_ptest_info-wt_splitlot",	false, "N", false, false, false, false, false, false, false);
    m_mapFields_GexToRemote_Wt[GEXDB_PLUGIN_DBFIELD_TESTNUM] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_TESTNUM, GEXDB_PLUGIN_DBFIELD_TESTNUM,
                                      "wt_ptest_info",	"wt_ptest_info.tnum",				"wt_ptest_info-wt_splitlot",	false, "N", false, false, false, false, false, true, false);
    m_mapFields_GexToRemote_Wt[GEXDB_PLUGIN_DBFIELD_TESTNAME_MPR] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_TESTNAME_MPR, GEXDB_PLUGIN_DBFIELD_TESTNAME_MPR,
                                      "wt_mptest_info",	"wt_mptest_info.tname",				"wt_mptest_info-wt_splitlot",	false, "N", false, false, false, false, false, false, false);
    m_mapFields_GexToRemote_Wt[GEXDB_PLUGIN_DBFIELD_TESTNUM_MPR] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_TESTNUM_MPR, GEXDB_PLUGIN_DBFIELD_TESTNUM_MPR,
                                      "wt_mptest_info",	"wt_mptest_info.tnum",				"wt_mptest_info-wt_splitlot",	false, "N", false, false, false, false, false, true, false);
    m_mapFields_GexToRemote_Wt[GEXDB_PLUGIN_DBFIELD_TESTNAME_FTR] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_TESTNAME_FTR, GEXDB_PLUGIN_DBFIELD_TESTNAME_FTR,
                                      "wt_ftest_info",	"wt_ftest_info.tname",				"wt_ftest_info-wt_splitlot",	false, "N", false, false, false, false, false, false, false);
    m_mapFields_GexToRemote_Wt[GEXDB_PLUGIN_DBFIELD_TESTNUM_FTR] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_TESTNUM_FTR, GEXDB_PLUGIN_DBFIELD_TESTNUM_FTR,
                                      "wt_ftest_info",	"wt_ftest_info.tnum",				"wt_ftest_info-wt_splitlot",	false, "N", false, false, false, false, false, true, false);
    m_mapFields_GexToRemote_Wt[GEXDB_PLUGIN_DBFIELD_STARTDATETIME] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_STARTDATETIME, GEXDB_PLUGIN_DBFIELD_STARTDATETIME,
                                      "wt_splitlot",		"wt_splitlot.start_t",				"",								false, "N", true, false, false, true, false, true, true);
    m_mapFields_GexToRemote_Wt[GEXDB_PLUGIN_DBFIELD_HBIN] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_HBIN, GEXDB_PLUGIN_DBFIELD_HBIN,
                                      "wt_hbin",			"wt_hbin.hbin_no",					"wt_hbin-wt_splitlot",			false, "H", false, false, false, false, true, true, false);
    m_mapFields_GexToRemote_Wt[GEXDB_PLUGIN_DBFIELD_SBIN] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_SBIN, GEXDB_PLUGIN_DBFIELD_SBIN,
                                      "wt_sbin",			"wt_sbin.sbin_no",					"wt_sbin-wt_splitlot",			false, "S", false, false, false, false, true, true, false);
    m_mapFields_GexToRemote_Wt[GEXDB_PLUGIN_DBFIELD_HBIN_NAME] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_HBIN_NAME, GEXDB_PLUGIN_DBFIELD_HBIN_NAME,
                                      "wt_hbin",			"wt_hbin.hbin_name",				"wt_hbin-wt_splitlot",			false, "H", false, false, false, false, false, false, false);
    m_mapFields_GexToRemote_Wt[GEXDB_PLUGIN_DBFIELD_SBIN_NAME] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_SBIN_NAME, GEXDB_PLUGIN_DBFIELD_SBIN_NAME,
                                      "wt_sbin",			"wt_sbin.sbin_name",				"wt_sbin-wt_splitlot",			false, "S", false, false, false, false, false, false, false);
    m_mapFields_GexToRemote_Wt[GEXDB_PLUGIN_DBFIELD_HBIN_PF] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_HBIN_PF, GEXDB_PLUGIN_DBFIELD_HBIN_PF,
                                      "wt_hbin",			"wt_hbin.hbin_cat",					"wt_hbin-wt_splitlot",			false, "H", false, false, false, false, true, false, false);
    m_mapFields_GexToRemote_Wt[GEXDB_PLUGIN_DBFIELD_SBIN_PF] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_SBIN_PF, GEXDB_PLUGIN_DBFIELD_SBIN_PF,
                                      "wt_sbin",			"wt_sbin.sbin_cat",					"wt_sbin-wt_splitlot",			false, "S", false, false, false, false, true, false, false);
    m_mapFields_GexToRemote_Wt[GEXDB_PLUGIN_DBFIELD_USER_TXT] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_USER_TXT, GEXDB_PLUGIN_DBFIELD_USER_TXT,
                                      "wt_splitlot",		"wt_splitlot.user_txt",				"",								false, "N", false, false, false, false, false, false, false);
    m_mapFields_GexToRemote_Wt[GEXDB_PLUGIN_DBFIELD_VALID_SPLITLOT] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_VALID_SPLITLOT, GEXDB_PLUGIN_DBFIELD_VALID_SPLITLOT,
                                      "wt_splitlot",		"wt_splitlot.valid_splitlot",		"",								false, "N", false, false, false, false, false, false, false);
    m_mapFields_GexToRemote_Wt[GEXDB_PLUGIN_DBFIELD_PROD_DATA] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_PROD_DATA, GEXDB_PLUGIN_DBFIELD_PROD_DATA,
                                      "wt_splitlot",		"wt_splitlot.prod_data",			"",								false, "N", false, false, false, false, false, false, false);
    m_mapFields_GexToRemote_Wt[GEXDB_PLUGIN_DBFIELD_PARTS] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_PARTS, GEXDB_PLUGIN_DBFIELD_PARTS,
                                      "wt_splitlot",		"wt_splitlot.nb_parts",				"",								false, "N", false, false, true, true, false, true, true);
    m_mapFields_GexToRemote_Wt[GEXDB_PLUGIN_DBFIELD_PARTS_SAMPLE] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_PARTS_SAMPLE, GEXDB_PLUGIN_DBFIELD_PARTS_SAMPLE,
                                      "wt_splitlot",		"wt_splitlot.nb_parts_samples",				"",								false, "N", false, false, true, true, false, true, true);
    m_mapFields_GexToRemote_Wt[GEXDB_PLUGIN_DBFIELD_PARTS_GOOD] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_PARTS_GOOD, GEXDB_PLUGIN_DBFIELD_PARTS_GOOD,
                                      "wt_splitlot",		"wt_splitlot.nb_parts_good",		"",								false, "N", false, false, true, true, false, true, true);
    m_mapFields_GexToRemote_Wt[GEXDB_PLUGIN_DBFIELD_LOT_PARTS] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_LOT_PARTS, GEXDB_PLUGIN_DBFIELD_LOT_PARTS,
                                      "wt_lot",			"wt_lot.nb_parts",					"wt_lot-wt_splitlot",			false, "N", false, false, true, true, false, true, true);
    m_mapFields_GexToRemote_Wt[GEXDB_PLUGIN_DBFIELD_LOT_PARTS_GOOD] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_LOT_PARTS_GOOD, GEXDB_PLUGIN_DBFIELD_LOT_PARTS_GOOD,
                                      "wt_lot",			"wt_lot.nb_parts_good",				"wt_lot-wt_splitlot",			false, "N", false, false, true, true, false, true, true);

    if(uiDbVersion_Build < 16)
    {
        m_mapFields_GexToRemote_Wt[GEXDB_PLUGIN_DBFIELD_WAFER_PARTS] =
                GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_WAFER_PARTS, GEXDB_PLUGIN_DBFIELD_WAFER_PARTS,
                                          "wt_wafer_info",	"wt_wafer_info.nb_parts",			"wt_wafer_info-wt_splitlot",	false, "N", false, false, true, true, false, true, true);
        m_mapFields_GexToRemote_Wt[GEXDB_PLUGIN_DBFIELD_WAFER_PARTS_GOOD] =
                GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_WAFER_PARTS_GOOD, GEXDB_PLUGIN_DBFIELD_WAFER_PARTS_GOOD,
                                          "wt_wafer_info",	"wt_wafer_info.nb_parts_good",		"wt_wafer_info-wt_splitlot",	false, "N", false, false, true, true, false, true, true);
    }
    else
    {
        m_mapFields_GexToRemote_Wt[GEXDB_PLUGIN_DBFIELD_WAFER_PARTS] =
                GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_WAFER_PARTS, GEXDB_PLUGIN_DBFIELD_WAFER_PARTS,
                                          "wt_wafer_consolidation",	"wt_wafer_consolidation.nb_parts",			"wt_wafer_consolidation-wt_splitlot",	false, "N", false, false, true, true, false, true, true);
        m_mapFields_GexToRemote_Wt[GEXDB_PLUGIN_DBFIELD_WAFER_PARTS_GOOD] =
                GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_WAFER_PARTS_GOOD, GEXDB_PLUGIN_DBFIELD_WAFER_PARTS_GOOD,
                                          "wt_wafer_consolidation",	"wt_wafer_consolidation.nb_parts_good",		"wt_wafer_consolidation-wt_splitlot",	false, "N", false, false, true, true, false, true, true);
        m_mapFields_GexToRemote_Wt[GEXDB_PLUGIN_DBFIELD_CONSOLIDATED_DATA_TYPE] =
                GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_CONSOLIDATED_DATA_TYPE, GEXDB_PLUGIN_DBFIELD_CONSOLIDATED_DATA_TYPE,
                                          "wt_wafer_consolidation",	"wt_wafer_consolidation.consolidated_data_type","wt_wafer_consolidation-wt_splitlot",	false, "N", false, false, false, true, true, false, false);
        m_mapFields_GexToRemote_Wt[GEXDB_PLUGIN_DBFIELD_CONSOLIDATION_NAME] =
                GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_CONSOLIDATION_NAME, GEXDB_PLUGIN_DBFIELD_CONSOLIDATION_NAME,
                                          "wt_wafer_consolidation",	"wt_wafer_consolidation.consolidation_name",	"wt_wafer_consolidation-wt_splitlot",	false, "N", false, false, false, true, true, false, false);
        m_mapFields_GexToRemote_Wt[GEXDB_PLUGIN_DBFIELD_CONSOLIDATION_FLOW] =
                GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_CONSOLIDATION_FLOW, GEXDB_PLUGIN_DBFIELD_CONSOLIDATION_FLOW,
                                          "wt_wafer_consolidation",	"wt_wafer_consolidation.consolidation_flow","wt_wafer_consolidation-wt_splitlot",	false, "N", false, false, false, true, true, false, false);
    }

    m_mapFields_GexToRemote_Wt[GEXDB_PLUGIN_DBFIELD_WAFER_GROSS_DIE] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_WAFER_GROSS_DIE, GEXDB_PLUGIN_DBFIELD_WAFER_GROSS_DIE,
                                      "wt_wafer_info",	"wt_wafer_info.gross_die",			"wt_wafer_info-wt_splitlot",	false, "N", false, false, true, true, false, true, true);


    // WYR fields (not exported in GUI)
    m_mapFields_GexToRemote_Wt[GEXDB_PLUGIN_DBFIELD_WYR_SITE] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_WYR_SITE, GEXDB_PLUGIN_DBFIELD_WYR_SITE,
                                      "wt_wyr",	"wt_wyr.site_name",							"",								false, "N", false, false, false, false, false, false, false);
    m_mapFields_GexToRemote_Wt[GEXDB_PLUGIN_DBFIELD_WYR_YEAR] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_WYR_YEAR, GEXDB_PLUGIN_DBFIELD_WYR_YEAR,
                                      "wt_wyr",	"wt_wyr.year",							"",									false, "N", false, false, false, false, false, true, false);
    m_mapFields_GexToRemote_Wt[GEXDB_PLUGIN_DBFIELD_WYR_WEEK] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_WYR_WEEK, GEXDB_PLUGIN_DBFIELD_WYR_WEEK,
                                      "wt_wyr",	"wt_wyr.week_nb",							"",								false, "N", false, false, false, false, false, true, false);

    // Links in Galaxy DB
    m_mapLinks_Remote_Wt["wt_lot_sbin-wt_lot"] =
            GexDbPlugin_Mapping_Link("wt_lot_sbin-wt_lot",					"wt_lot_sbin",				"wt_lot_sbin.lot_id",					"wt_lot",		"wt_lot.lot_id",			"wt_lot_sbin-wt_lot-2");
    m_mapLinks_Remote_Wt["wt_lot_sbin-wt_lot-2"] =
            GexDbPlugin_Mapping_Link("wt_lot_sbin-wt_lot-2",				"wt_lot_sbin",				"wt_lot_sbin.product_name",				"wt_lot",		"wt_lot.product_name",		"wt_lot-wt_splitlot");
    m_mapLinks_Remote_Wt["wt_lot_hbin-wt_lot"] =
            GexDbPlugin_Mapping_Link("wt_lot_hbin-wt_lot",					"wt_lot_hbin",				"wt_lot_hbin.lot_id",					"wt_lot",		"wt_lot.lot_id",			"wt_lot_hbin-wt_lot-2");
    m_mapLinks_Remote_Wt["wt_lot_hbin-wt_lot-2"] =
            GexDbPlugin_Mapping_Link("wt_lot_hbin-wt_lot-2",				"wt_lot_hbin",				"wt_lot_hbin.product_name",				"wt_lot",		"wt_lot.product_name",		"wt_lot-wt_splitlot");
    m_mapLinks_Remote_Wt["product-wt_wafer_info"] =
            GexDbPlugin_Mapping_Link("product-wt_wafer_info",				"product",					"product.product_name",					"wt_wafer_info","wt_wafer_info.product_name","wt_wafer_info-wt_splitlot");
    m_mapLinks_Remote_Wt["wt_lot-wt_splitlot"] =
            GexDbPlugin_Mapping_Link("wt_lot-wt_splitlot",					"wt_lot",					"wt_lot.lot_id",						"wt_splitlot",	"wt_splitlot.lot_id",		"wt_lot-wt_splitlot-2");
    m_mapLinks_Remote_Wt["wt_lot-wt_splitlot-2"] =
            GexDbPlugin_Mapping_Link("wt_lot-wt_splitlot-2",				"wt_lot",					"wt_lot.product_name",					"wt_splitlot",	"wt_splitlot.product_name",		"");
    m_mapLinks_Remote_Wt["wt_wafer_info-wt_lot"] =
            GexDbPlugin_Mapping_Link("wt_wafer_info-wt_lot",				"wt_wafer_info",			"wt_wafer_info.lot_id",					"wt_lot",		"wt_lot.lot_id",			"wt_lot-wt_splitlot");
    m_mapLinks_Remote_Wt["wt_ptest_info-wt_splitlot"] =
            GexDbPlugin_Mapping_Link("wt_ptest_info-wt_splitlot",			"wt_ptest_info",			"wt_ptest_info.splitlot_id",			"wt_splitlot",	"wt_splitlot.splitlot_id",	"");
    m_mapLinks_Remote_Wt["wt_mptest_info-wt_splitlot"] =
            GexDbPlugin_Mapping_Link("wt_mptest_info-wt_splitlot",			"wt_mptest_info",			"wt_mptest_info.splitlot_id",			"wt_splitlot",	"wt_splitlot.splitlot_id",	"");
    m_mapLinks_Remote_Wt["wt_ftest_info-wt_splitlot"] =
            GexDbPlugin_Mapping_Link("wt_ftest_info-wt_splitlot",			"wt_ftest_info",			"wt_ftest_info.splitlot_id",			"wt_splitlot",	"wt_splitlot.splitlot_id",	"");
    m_mapLinks_Remote_Wt["wt_hbin-wt_splitlot"] =
            GexDbPlugin_Mapping_Link("wt_hbin-wt_splitlot",					"wt_hbin",					"wt_hbin.splitlot_id",					"wt_splitlot",	"wt_splitlot.splitlot_id",	"");
    m_mapLinks_Remote_Wt["wt_sbin-wt_splitlot"] =
            GexDbPlugin_Mapping_Link("wt_sbin-wt_splitlot",					"wt_sbin",					"wt_sbin.splitlot_id",					"wt_splitlot",	"wt_splitlot.splitlot_id",	"");
    m_mapLinks_Remote_Wt["wt_parts_stats_samples-wt_splitlot"] =
            GexDbPlugin_Mapping_Link("wt_parts_stats_samples-wt_splitlot",	"wt_parts_stats_samples",	"wt_parts_stats_samples.splitlot_id",	"wt_splitlot",	"wt_splitlot.splitlot_id",	"");
    m_mapLinks_Remote_Wt["wt_wafer_info-wt_splitlot"] =
            GexDbPlugin_Mapping_Link("wt_wafer_info-wt_splitlot", "wt_wafer_info", "wt_wafer_info.lot_id", "wt_splitlot", "wt_splitlot.lot_id", "wt_wafer_info-wt_splitlot-2");
    m_mapLinks_Remote_Wt["wt_wafer_info-wt_splitlot-2"] =
            GexDbPlugin_Mapping_Link("wt_wafer_info-wt_splitlot-2", "wt_wafer_info", "wt_wafer_info.wafer_id", "wt_splitlot", "wt_splitlot.wafer_id", "");
    m_mapLinks_Remote_Wt["wt_run-wt_splitlot"] =
            GexDbPlugin_Mapping_Link("wt_run-wt_splitlot",			"wt_run",		"wt_run.splitlot_id",		"wt_splitlot", "wt_splitlot.splitlot_id",	"");

    if(uiDbVersion_Build > 15)
    {
        m_mapLinks_Remote_Wt["wt_wafer_consolidation-wt_splitlot"] =
                GexDbPlugin_Mapping_Link("wt_wafer_consolidation-wt_splitlot", "wt_wafer_consolidation", "wt_wafer_consolidation.lot_id", "wt_splitlot", "wt_splitlot.lot_id",	"wt_wafer_consolidation-wt_splitlot-2");
        m_mapLinks_Remote_Wt["wt_wafer_consolidation-wt_splitlot-2"] =
                GexDbPlugin_Mapping_Link("wt_wafer_consolidation-wt_splitlot-2", "wt_wafer_consolidation", "wt_wafer_consolidation.wafer_id", "wt_splitlot", "wt_splitlot.wafer_id",	"");
    }

    if (uiDbVersion_Build >= 18)
    {
        m_mapLinks_Remote_Wt["wt_test_conditions-wt_splitlot"] =
                GexDbPlugin_Mapping_Link("wt_test_conditions-wt_splitlot", "wt_test_conditions","wt_test_conditions.splitlot_id", "wt_splitlot","wt_splitlot.splitlot_id", "");
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Final Test
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Gex fields <-> Galaxy DB fields mapping
    // 1. Fields exported in the GEX GUI
    m_mapFields_GexToRemote_Ft[m_gexLabelFilterChoices[GEX_QUERY_FILTER_BURNIN]] =
            GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_BURNIN], m_gexLabelFilterChoices[GEX_QUERY_FILTER_BURNIN],
                                      "ft_splitlot",		"ft_splitlot.burn_tim",				"",								true, "N", false, false, false, true, false, true, true);
    m_mapFields_GexToRemote_Ft[m_gexLabelFilterChoices[GEX_QUERY_FILTER_ORIGIN]] =
            GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_ORIGIN], m_gexLabelFilterChoices[GEX_QUERY_FILTER_ORIGIN],
                                      "ft_splitlot",		"ft_splitlot.data_provider",		"",								true, "N", false, false, false, true, true, false, true);
    m_mapFields_GexToRemote_Ft[m_gexLabelFilterChoices[GEX_QUERY_FILTER_DIBNAME]] =
            GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_DIBNAME], m_gexLabelFilterChoices[GEX_QUERY_FILTER_DIBNAME],
                                      "ft_splitlot",		"ft_splitlot.dib_id",				"",								true, "N", false, false, false, true, true, false, true);
    m_mapFields_GexToRemote_Ft[m_gexLabelFilterChoices[GEX_QUERY_FILTER_DIBTYPE]] =
            GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_DIBTYPE], m_gexLabelFilterChoices[GEX_QUERY_FILTER_DIBTYPE],
                                      "ft_splitlot",		"ft_splitlot.dib_typ",				"",								true, "N", false, false, false, true, true, false, true);
    m_mapFields_GexToRemote_Ft[m_gexLabelFilterChoices[GEX_QUERY_FILTER_FACILITY]] =
            GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_FACILITY], m_gexLabelFilterChoices[GEX_QUERY_FILTER_FACILITY],
                                      "ft_splitlot",		"ft_splitlot.facil_id",				"",								true, "N", false, false, false, true, true, false, true);
    m_mapFields_GexToRemote_Ft[m_gexLabelFilterChoices[GEX_QUERY_FILTER_FAMILY]] =
            GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_FAMILY], m_gexLabelFilterChoices[GEX_QUERY_FILTER_FAMILY],
                                      "ft_splitlot",		"ft_splitlot.famly_id",				"",								true, "N", false, false, false, true, true, false, true);
    m_mapFields_GexToRemote_Ft[m_gexLabelFilterChoices[GEX_QUERY_FILTER_FLOOR]] =
            GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_FLOOR], m_gexLabelFilterChoices[GEX_QUERY_FILTER_FLOOR],
                                      "ft_splitlot",		"ft_splitlot.floor_id",				"",								true, "N", false, false, false, true, true, false, true);
    m_mapFields_GexToRemote_Ft[m_gexLabelFilterChoices[GEX_QUERY_FILTER_FREQUENCYSTEP]] =
            GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_FREQUENCYSTEP], m_gexLabelFilterChoices[GEX_QUERY_FILTER_FREQUENCYSTEP],
                                      "ft_splitlot",		"ft_splitlot.oper_frq",				"",								true, "N", false, false, false, true, true, false, true);
    m_mapFields_GexToRemote_Ft[m_gexLabelFilterChoices[GEX_QUERY_FILTER_LOADBOARDNAME]] =
            GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_LOADBOARDNAME], m_gexLabelFilterChoices[GEX_QUERY_FILTER_LOADBOARDNAME],
                                      "ft_splitlot",		"ft_splitlot.loadboard_id",			"",								true, "N", false, false, false, true, true, false, true);
    m_mapFields_GexToRemote_Ft[m_gexLabelFilterChoices[GEX_QUERY_FILTER_LOADBOARDTYPE]] =
            GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_LOADBOARDTYPE], m_gexLabelFilterChoices[GEX_QUERY_FILTER_LOADBOARDTYPE],
                                      "ft_splitlot",		"ft_splitlot.loadboard_typ",		"",								true, "N", false, false, false, true, true, false, true);
    //    m_mapFields_GexToRemote_Ft[m_gexLabelFilterChoices[GEX_QUERY_FILTER_LOT]] =
    //            GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_LOT], m_gexLabelFilterChoices[GEX_QUERY_FILTER_LOT],
    //                                      "ft_lot",			"ft_lot.lot_id",					"ft_lot-ft_splitlot",			true, "N", false, false, false, true, true, false, true);
    m_mapFields_GexToRemote_Ft[m_gexLabelFilterChoices[GEX_QUERY_FILTER_LOT]] =
            GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_LOT], m_gexLabelFilterChoices[GEX_QUERY_FILTER_LOT],
                                      "ft_splitlot",		"ft_splitlot.lot_id",				"",                             true, "N", false, false, false, true, true, false, true);



    m_mapFields_GexToRemote_Ft[m_gexLabelFilterChoices[GEX_QUERY_FILTER_OPERATOR]] =
            GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_OPERATOR], m_gexLabelFilterChoices[GEX_QUERY_FILTER_OPERATOR],
                                      "ft_splitlot",		"ft_splitlot.oper_nam",				"",								false, "N", false, false, false, true, true, false, true);
    m_mapFields_GexToRemote_Ft[m_gexLabelFilterChoices[GEX_QUERY_FILTER_PACKAGE]] =
            GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_PACKAGE], m_gexLabelFilterChoices[GEX_QUERY_FILTER_PACKAGE],
                                      "ft_splitlot",		"ft_splitlot.pkg_typ",				"",								true, "N", false, false, false, true, true, false, true);
    m_mapFields_GexToRemote_Ft[m_gexLabelFilterChoices[GEX_QUERY_FILTER_PROBERNAME]] =
            GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_PROBERNAME], m_gexLabelFilterChoices[GEX_QUERY_FILTER_PROBERNAME],
                                      "ft_splitlot",		"ft_splitlot.handler_id",			"",								true, "N", false, false, false, true, true, false, true);
    m_mapFields_GexToRemote_Ft[m_gexLabelFilterChoices[GEX_QUERY_FILTER_PROBERTYPE]] =
            GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_PROBERTYPE], m_gexLabelFilterChoices[GEX_QUERY_FILTER_PROBERTYPE],
                                      "ft_splitlot",		"ft_splitlot.handler_typ",			"",								true, "N", false, false, false, true, true, false, true);
    m_mapFields_GexToRemote_Ft[m_gexLabelFilterChoices[GEX_QUERY_FILTER_PROCESS]] =
            GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_PROCESS], m_gexLabelFilterChoices[GEX_QUERY_FILTER_PROCESS],
                                      "ft_splitlot",		"ft_splitlot.proc_id",				"",								true, "N", false, false, false, true, true, false, true);
    m_mapFields_GexToRemote_Ft[m_gexLabelFilterChoices[GEX_QUERY_FILTER_PRODUCT]] =
            GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_PRODUCT], m_gexLabelFilterChoices[GEX_QUERY_FILTER_PRODUCT],
                                      "ft_splitlot",			"ft_splitlot.product_name",				"",			true, "N", false, false, false, true, true, false, true);
    m_mapFields_GexToRemote_Ft[m_gexLabelFilterChoices[GEX_QUERY_FILTER_PROGRAMNAME]] =
            GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_PROGRAMNAME], m_gexLabelFilterChoices[GEX_QUERY_FILTER_PROGRAMNAME],
                                      "ft_splitlot",		"ft_splitlot.job_nam",				"",								true, "N", false, false, false, true, true, false, true);
    m_mapFields_GexToRemote_Ft[m_gexLabelFilterChoices[GEX_QUERY_FILTER_PROGRAMREVISION]] =
            GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_PROGRAMREVISION], m_gexLabelFilterChoices[GEX_QUERY_FILTER_PROGRAMREVISION],
                                      "ft_splitlot",		"ft_splitlot.job_rev",				"",								true, "N", false, false, false, true, true, false, true);
    m_mapFields_GexToRemote_Ft[m_gexLabelFilterChoices[GEX_QUERY_FILTER_RETESTNBR]] =
            GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_RETESTNBR], m_gexLabelFilterChoices[GEX_QUERY_FILTER_RETESTNBR],
                                      "ft_splitlot",		"ft_splitlot.retest_index",			"",								true, "N", false, false, false, false, false, true, false);
    m_mapFields_GexToRemote_Ft[m_gexLabelFilterChoices[GEX_QUERY_FILTER_SUBLOT]] =
            GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_SUBLOT], m_gexLabelFilterChoices[GEX_QUERY_FILTER_SUBLOT],
                                      "ft_splitlot",		"ft_splitlot.sublot_id",			"",								true, "N", false, false, false, true, true, false, true);
    m_mapFields_GexToRemote_Ft[m_gexLabelFilterChoices[GEX_QUERY_FILTER_TEMPERATURE]] =
            GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_TEMPERATURE], m_gexLabelFilterChoices[GEX_QUERY_FILTER_TEMPERATURE],
                                      "ft_splitlot",		"ft_splitlot.tst_temp",				"",								true, "N", false, false, false, true, true, false, true);
    m_mapFields_GexToRemote_Ft[m_gexLabelFilterChoices[GEX_QUERY_FILTER_TESTERNAME]] =
            GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_TESTERNAME], m_gexLabelFilterChoices[GEX_QUERY_FILTER_TESTERNAME],
                                      "ft_splitlot",		"ft_splitlot.tester_name",			"",								true, "N", false, false, false, true, true, false, true);
    m_mapFields_GexToRemote_Ft[m_gexLabelFilterChoices[GEX_QUERY_FILTER_TESTERTYPE]] =
            GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_TESTERTYPE], m_gexLabelFilterChoices[GEX_QUERY_FILTER_TESTERTYPE],
                                      "ft_splitlot",		"ft_splitlot.tester_type",			"",								true, "N", false, false, false, true, true, false, true);
    m_mapFields_GexToRemote_Ft[m_gexLabelFilterChoices[GEX_QUERY_FILTER_TESTCODE]] =
            GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_TESTCODE], m_gexLabelFilterChoices[GEX_QUERY_FILTER_TESTCODE],
                                      "ft_splitlot",		"ft_splitlot.test_cod",				"",								true, "N", false, false, false, true, true, false, true);
    m_mapFields_GexToRemote_Ft[m_gexLabelFilterChoices[GEX_QUERY_FILTER_SITENBR]] =
            GexDbPlugin_Mapping_Field(m_gexLabelFilterChoices[GEX_QUERY_FILTER_SITENBR], m_gexLabelFilterChoices[GEX_QUERY_FILTER_SITENBR],
                                      "ft_parts_stats_samples",	"ft_parts_stats_samples.site_no",		"ft_parts_stats_samples-ft_splitlot", true, "N", false, false, false, false, false, true, false);
    // 2. Fields exported in the GEX GUI, but with no standard GEX correspondance
    m_mapFields_GexToRemote_Ft[GEXDB_PLUGIN_DBFIELD_DATATYPE] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_DATATYPE, GEXDB_PLUGIN_DBFIELD_DATATYPE,
                                      "ft_splitlot",		"ft_splitlot.data_type",			"",								true, "N", false, false, false, true, true, false, true);
    m_mapFields_GexToRemote_Ft[GEXDB_PLUGIN_DBFIELD_TESTINSERTION] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_TESTINSERTION, GEXDB_PLUGIN_DBFIELD_TESTINSERTION,
                                      "ft_splitlot",		"ft_splitlot.test_insertion",			"",								true, "N", false, false, false, false, false, false, false);
    m_mapFields_GexToRemote_Ft[GEXDB_PLUGIN_DBFIELD_TESTFLOW] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_TESTFLOW, GEXDB_PLUGIN_DBFIELD_TESTFLOW,
                                      "ft_splitlot",		"ft_splitlot.test_flow",			"",								true, "N", false, false, false, false, false, false, false);
    m_mapFields_GexToRemote_Ft[GEXDB_PLUGIN_DBFIELD_TRACKING_LOT] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_TRACKING_LOT, GEXDB_PLUGIN_DBFIELD_TRACKING_LOT,
                                      "ft_lot",			"ft_lot.tracking_lot_id",			"ft_lot-ft_splitlot",			true, "N", false, false, false, true, true, false, true);

    m_mapFields_GexToRemote_Ft[GEXDB_PLUGIN_DBFIELD_FTDIETRACE_CONFIG_PRODUCT] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_FTDIETRACE_CONFIG_PRODUCT,GEXDB_PLUGIN_DBFIELD_FTDIETRACE_CONFIG_PRODUCT,
                                      "ft_dietrace_config",	"ft_dietrace_config.product", "",true, "N", false, false, false, false, true, false, false);

    m_mapFields_GexToRemote_Ft[GEXDB_PLUGIN_DBFIELD_FTDIETRACE_CONFIG_LOT_ID] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_FTDIETRACE_CONFIG_LOT_ID,GEXDB_PLUGIN_DBFIELD_FTDIETRACE_CONFIG_LOT_ID,
                                      "ft_dietrace_config",	"ft_dietrace_config.lot_id", "",true, "N", false, false, false, false, true, false, false);

    m_mapFields_GexToRemote_Ft[GEXDB_PLUGIN_DBFIELD_FTDIETRACE_CONFIG_WAFER_ID] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_FTDIETRACE_CONFIG_WAFER_ID,GEXDB_PLUGIN_DBFIELD_FTDIETRACE_CONFIG_WAFER_ID,
                                      "ft_dietrace_config",	"ft_dietrace_config.wafer_id", "",true, "N", false, false, false, false, true, false, false);

    m_mapFields_GexToRemote_Ft[GEXDB_PLUGIN_DBFIELD_FTDIETRACKING_WAFER_DIE_ID] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_FTDIETRACKING_WAFER_DIE_ID,GEXDB_PLUGIN_DBFIELD_FTDIETRACKING_WAFER_DIE_ID,
                                      "ft_die_tracking",	"ft_die_tracking.die_id", "ft_die_tracking-ft_lot",true, "N", false, false, false, true, true, false, true);
    m_mapFields_GexToRemote_Ft[GEXDB_PLUGIN_DBFIELD_FTDIETRACKING_WAFER_PRODUCT_ID] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_FTDIETRACKING_WAFER_PRODUCT_ID, GEXDB_PLUGIN_DBFIELD_FTDIETRACKING_WAFER_PRODUCT_ID,
                                      "ft_die_tracking",	"ft_die_tracking.wt_product_id",	"ft_die_tracking-ft_lot",		true, "N", false, false, false, true, true, false, true);
    m_mapFields_GexToRemote_Ft[GEXDB_PLUGIN_DBFIELD_FTDIETRACKING_WAFER_SUBLOT_ID] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_FTDIETRACKING_WAFER_SUBLOT_ID, GEXDB_PLUGIN_DBFIELD_FTDIETRACKING_WAFER_SUBLOT_ID,
                                      "ft_die_tracking",	"ft_die_tracking.wt_sublot_id",		"ft_die_tracking-ft_lot",		true, "N", false, false, false, true, true, false, true);
    m_mapFields_GexToRemote_Ft[GEXDB_PLUGIN_DBFIELD_FTDIETRACKING_WAFER_TRACKING_LOT_ID] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_FTDIETRACKING_WAFER_TRACKING_LOT_ID, GEXDB_PLUGIN_DBFIELD_FTDIETRACKING_WAFER_TRACKING_LOT_ID,
                                      "ft_die_tracking",	"ft_die_tracking.wt_tracking_lot_id","ft_die_tracking-ft_lot",		true, "N", false, false, false, true, true, false, true);
    m_mapFields_GexToRemote_Ft[GEXDB_PLUGIN_DBFIELD_DAY] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_DAY, GEXDB_PLUGIN_DBFIELD_DAY,
                                      "ft_splitlot",		"ft_splitlot.day",				"",									true, "N", true, false, false, true, true, false, true);
    m_mapFields_GexToRemote_Ft[GEXDB_PLUGIN_DBFIELD_WEEK] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_WEEK, GEXDB_PLUGIN_DBFIELD_WEEK,
                                      "ft_splitlot",		"ft_splitlot.week_nb",				"",								true, "N", true, false, false, true, true, true, true);
    m_mapFields_GexToRemote_Ft[GEXDB_PLUGIN_DBFIELD_MONTH] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_MONTH, GEXDB_PLUGIN_DBFIELD_MONTH,
                                      "ft_splitlot",		"ft_splitlot.month_nb",				"",								true, "N", true, false, false, true, true, true, true);
    m_mapFields_GexToRemote_Ft[GEXDB_PLUGIN_DBFIELD_QUARTER] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_QUARTER, GEXDB_PLUGIN_DBFIELD_QUARTER,
                                      "ft_splitlot",		"ft_splitlot.quarter_nb",			"",								true, "N", true, false, false, true, true, true, true);
    m_mapFields_GexToRemote_Ft[GEXDB_PLUGIN_DBFIELD_YEAR] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_YEAR, GEXDB_PLUGIN_DBFIELD_YEAR,
                                      "ft_splitlot",		"ft_splitlot.year_nb",				"",								true, "N", true, false, false, true, true, true, true);
    m_mapFields_GexToRemote_Ft[GEXDB_PLUGIN_DBFIELD_YEAR_WEEK] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_YEAR_WEEK, GEXDB_PLUGIN_DBFIELD_YEAR_WEEK,
                                      "ft_splitlot",		"ft_splitlot.year_and_week",			"",								true, "N", true, false, false, true, true, false, true);
    m_mapFields_GexToRemote_Ft[GEXDB_PLUGIN_DBFIELD_YEAR_MONTH] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_YEAR_MONTH, GEXDB_PLUGIN_DBFIELD_YEAR_MONTH,
                                      "ft_splitlot",		"ft_splitlot.year_and_month",			"",								true, "N", true, false, false, true, true, false, true);
    m_mapFields_GexToRemote_Ft[GEXDB_PLUGIN_DBFIELD_YEAR_QUARTER] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_YEAR_QUARTER, GEXDB_PLUGIN_DBFIELD_YEAR_QUARTER,
                                      "ft_splitlot",		"ft_splitlot.year_and_quarter",			"",								true, "N", true, false, false, true, true, false, true);
    // 3. Fields not exported in the GEX GUI, but needed in our queries
    m_mapFields_GexToRemote_Ft[GEXDB_PLUGIN_DBFIELD_TESTNAME] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_TESTNAME, GEXDB_PLUGIN_DBFIELD_TESTNAME,
                                      "ft_ptest_info",	"ft_ptest_info.tname",				"ft_ptest_info-ft_splitlot",	false, "N", false, false, false, false, false, false, false);
    m_mapFields_GexToRemote_Ft[GEXDB_PLUGIN_DBFIELD_TESTNUM] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_TESTNUM, GEXDB_PLUGIN_DBFIELD_TESTNUM,
                                      "ft_ptest_info",	"ft_ptest_info.tnum",				"ft_ptest_info-ft_splitlot",	false, "N", false, false, false, false, false, true, false);
    m_mapFields_GexToRemote_Ft[GEXDB_PLUGIN_DBFIELD_TESTNAME_MPR] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_TESTNAME_MPR, GEXDB_PLUGIN_DBFIELD_TESTNAME_MPR,
                                      "ft_mptest_info",	"ft_mptest_info.tname",				"ft_mptest_info-ft_splitlot",	false, "N", false, false, false, false, false, false, false);
    m_mapFields_GexToRemote_Ft[GEXDB_PLUGIN_DBFIELD_TESTNUM_MPR] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_TESTNUM_MPR, GEXDB_PLUGIN_DBFIELD_TESTNUM_MPR,
                                      "ft_mptest_info",	"ft_mptest_info.tnum",				"ft_mptest_info-ft_splitlot",	false, "N", false, false, false, false, false, true, false);
    m_mapFields_GexToRemote_Ft[GEXDB_PLUGIN_DBFIELD_TESTNAME_FTR] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_TESTNAME_FTR, GEXDB_PLUGIN_DBFIELD_TESTNAME_FTR,
                                      "ft_ftest_info",	"ft_ftest_info.tname",				"ft_ftest_info-ft_splitlot",	false, "N", false, false, false, false, false, false, false);
    m_mapFields_GexToRemote_Ft[GEXDB_PLUGIN_DBFIELD_TESTNUM_FTR] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_TESTNUM_FTR, GEXDB_PLUGIN_DBFIELD_TESTNUM_FTR,
                                      "ft_ftest_info",	"ft_ftest_info.tnum",				"ft_ftest_info-ft_splitlot",	false, "N", false, false, false, false, false, true, false);
    m_mapFields_GexToRemote_Ft[GEXDB_PLUGIN_DBFIELD_STARTDATETIME] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_STARTDATETIME, GEXDB_PLUGIN_DBFIELD_STARTDATETIME,
                                      "ft_splitlot",		"ft_splitlot.start_t",				"",								false, "N", true, false, false, true, false, true, true);
    m_mapFields_GexToRemote_Ft[GEXDB_PLUGIN_DBFIELD_HBIN] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_HBIN, GEXDB_PLUGIN_DBFIELD_HBIN,
                                      "ft_hbin",			"ft_hbin.hbin_no",					"ft_hbin-ft_splitlot",			false, "H", false, false, false, false, true, true, false);
    m_mapFields_GexToRemote_Ft[GEXDB_PLUGIN_DBFIELD_SBIN] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_SBIN, GEXDB_PLUGIN_DBFIELD_SBIN,
                                      "ft_sbin",			"ft_sbin.sbin_no",					"ft_sbin-ft_splitlot",			false, "S", false, false, false, false, true, true, false);
    m_mapFields_GexToRemote_Ft[GEXDB_PLUGIN_DBFIELD_HBIN_NAME] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_HBIN_NAME, GEXDB_PLUGIN_DBFIELD_HBIN_NAME,
                                      "ft_hbin",			"ft_hbin.hbin_name",				"ft_hbin-ft_splitlot",			false, "H", false, false, false, false, false, false, false);
    m_mapFields_GexToRemote_Ft[GEXDB_PLUGIN_DBFIELD_SBIN_NAME] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_SBIN_NAME, GEXDB_PLUGIN_DBFIELD_SBIN_NAME,
                                      "ft_sbin",			"ft_sbin.sbin_name",				"ft_sbin-ft_splitlot",			false, "S", false, false, false, false, false, false, false);
    m_mapFields_GexToRemote_Ft[GEXDB_PLUGIN_DBFIELD_HBIN_PF] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_HBIN_PF, GEXDB_PLUGIN_DBFIELD_HBIN_PF,
                                      "ft_hbin",			"ft_hbin.hbin_cat",					"ft_hbin-ft_splitlot",			false, "H", false, false, false, false, true, false, false);
    m_mapFields_GexToRemote_Ft[GEXDB_PLUGIN_DBFIELD_SBIN_PF] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_SBIN_PF, GEXDB_PLUGIN_DBFIELD_SBIN_PF,
                                      "ft_sbin",			"ft_sbin.sbin_cat",					"ft_sbin-ft_splitlot",			false, "S", false, false, false, false, true, false, false);
    m_mapFields_GexToRemote_Ft[GEXDB_PLUGIN_DBFIELD_USER_TXT] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_USER_TXT, GEXDB_PLUGIN_DBFIELD_USER_TXT,
                                      "ft_splitlot",		"ft_splitlot.user_txt",				"",								false, "N", false, false, false, false, false, false, false);
    m_mapFields_GexToRemote_Ft[GEXDB_PLUGIN_DBFIELD_VALID_SPLITLOT] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_VALID_SPLITLOT, GEXDB_PLUGIN_DBFIELD_VALID_SPLITLOT,
                                      "ft_splitlot",		"ft_splitlot.valid_splitlot",		"",								false, "N", false, false, false, false, false, false, false);
    m_mapFields_GexToRemote_Ft[GEXDB_PLUGIN_DBFIELD_PROD_DATA] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_PROD_DATA, GEXDB_PLUGIN_DBFIELD_PROD_DATA,
                                      "ft_splitlot",		"ft_splitlot.prod_data",			"",								false, "N", false, false, false, false, false, false, false);
    m_mapFields_GexToRemote_Ft[GEXDB_PLUGIN_DBFIELD_PARTS] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_PARTS, GEXDB_PLUGIN_DBFIELD_PARTS,
                                      "ft_splitlot",		"ft_splitlot.nb_parts",				"",								false, "N", false, false, true, true, false, true, true);
    m_mapFields_GexToRemote_Ft[GEXDB_PLUGIN_DBFIELD_PARTS_SAMPLE] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_PARTS_SAMPLE, GEXDB_PLUGIN_DBFIELD_PARTS_SAMPLE,
                                      "ft_splitlot",		"ft_splitlot.nb_parts_samples",				"",						false, "N", false, false, true, true, false, true, true);
    m_mapFields_GexToRemote_Ft[GEXDB_PLUGIN_DBFIELD_PARTS_GOOD] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_PARTS_GOOD, GEXDB_PLUGIN_DBFIELD_PARTS_GOOD,
                                      "ft_splitlot",		"ft_splitlot.nb_parts_good",		"",								false, "N", false, false, true, true, false, true, true);
    m_mapFields_GexToRemote_Ft[GEXDB_PLUGIN_DBFIELD_LOT_PARTS] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_LOT_PARTS, GEXDB_PLUGIN_DBFIELD_LOT_PARTS,
                                      "ft_lot",			"ft_lot.nb_parts",					"ft_lot-ft_splitlot",			false, "N", false, false, true, true, false, true, true);
    m_mapFields_GexToRemote_Ft[GEXDB_PLUGIN_DBFIELD_LOT_PARTS_GOOD] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_LOT_PARTS_GOOD, GEXDB_PLUGIN_DBFIELD_LOT_PARTS_GOOD,
                                      "ft_lot",			"ft_lot.nb_parts_good",				"ft_lot-ft_splitlot",			false, "N", false, false, true, true, false, true, true);

    if (uiDbVersion_Build == 27)
    {
        m_mapFields_GexToRemote_Ft[GEXDB_PLUGIN_DBFIELD_SUBLOT_PARTS] =
                GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_SUBLOT_PARTS, GEXDB_PLUGIN_DBFIELD_SUBLOT_PARTS,
                                          "ft_sublot_info",	"ft_sublot_info.nb_parts",			"ft_sublot_info-ft_splitlot",	false, "N", false, false, true, true, false, true, true);
        m_mapFields_GexToRemote_Ft[GEXDB_PLUGIN_DBFIELD_SUBLOT_PARTS_GOOD] =
                GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_SUBLOT_PARTS_GOOD, GEXDB_PLUGIN_DBFIELD_SUBLOT_PARTS_GOOD,
                                          "ft_sublot_info",	"ft_sublot_info.nb_parts_good",		"ft_sublot_info-ft_splitlot",	false, "N", false, false, true, true, false, true, true);
    }
    else if(uiDbVersion_Build >= 28)
    {
        m_mapFields_GexToRemote_Ft[GEXDB_PLUGIN_DBFIELD_SUBLOT_PARTS] =
                GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_SUBLOT_PARTS, GEXDB_PLUGIN_DBFIELD_SUBLOT_PARTS,
                                          "ft_sublot_consolidation",	"ft_sublot_consolidation.nb_parts",             "ft_sublot_consolidation-ft_splitlot",	false, "N", false, false, true, true, false, true, true);
        m_mapFields_GexToRemote_Ft[GEXDB_PLUGIN_DBFIELD_SUBLOT_PARTS_GOOD] =
                GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_SUBLOT_PARTS_GOOD, GEXDB_PLUGIN_DBFIELD_SUBLOT_PARTS_GOOD,
                                          "ft_sublot_consolidation",	"ft_sublot_consolidation.nb_parts_good",        "ft_sublot_consolidation-ft_splitlot",	false, "N", false, false, true, true, false, true, true);
        m_mapFields_GexToRemote_Ft[GEXDB_PLUGIN_DBFIELD_CONSOLIDATED_DATA_TYPE] =
                GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_CONSOLIDATED_DATA_TYPE, GEXDB_PLUGIN_DBFIELD_CONSOLIDATED_DATA_TYPE,
                                          "ft_sublot_consolidation",	"ft_sublot_consolidation.consolidated_data_type","ft_sublot_consolidation-ft_splitlot",	false, "N", false, false, false, true, true, false, false);
        m_mapFields_GexToRemote_Ft[GEXDB_PLUGIN_DBFIELD_CONSOLIDATION_NAME] =
                GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_CONSOLIDATION_NAME, GEXDB_PLUGIN_DBFIELD_CONSOLIDATION_NAME,
                                          "ft_sublot_consolidation",	"ft_sublot_consolidation.consolidation_name",	"ft_sublot_consolidation-ft_splitlot",	false, "N", false, false, false, true, true, false, false);
        m_mapFields_GexToRemote_Ft[GEXDB_PLUGIN_DBFIELD_CONSOLIDATION_FLOW] =
                GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_CONSOLIDATION_FLOW, GEXDB_PLUGIN_DBFIELD_CONSOLIDATION_FLOW,
                                          "ft_sublot_consolidation",	"ft_sublot_consolidation.consolidation_flow","ft_sublot_consolidation-ft_splitlot",	false, "N", false, false, false, true, true, false, false);
    }

    // WYR fields (not exported in GUI)
    m_mapFields_GexToRemote_Ft[GEXDB_PLUGIN_DBFIELD_WYR_SITE] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_WYR_SITE, GEXDB_PLUGIN_DBFIELD_WYR_SITE,
                                      "ft_wyr",	"ft_wyr.site_name",							"",								false, "N", false, false, false, false, false, false, false);
    m_mapFields_GexToRemote_Ft[GEXDB_PLUGIN_DBFIELD_WYR_YEAR] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_WYR_YEAR, GEXDB_PLUGIN_DBFIELD_WYR_YEAR,
                                      "ft_wyr",	"ft_wyr.year",							"",									false, "N", false, false, false, false, false, true, false);
    m_mapFields_GexToRemote_Ft[GEXDB_PLUGIN_DBFIELD_WYR_WEEK] =
            GexDbPlugin_Mapping_Field(GEXDB_PLUGIN_DBFIELD_WYR_WEEK, GEXDB_PLUGIN_DBFIELD_WYR_WEEK,
                                      "ft_wyr",	"ft_wyr.week_nb",							"",								false, "N", false, false, false, false, false, true, false);

    // Links in Galaxy DB
    m_mapLinks_Remote_Ft["product-ft_sublot_info"] =
            GexDbPlugin_Mapping_Link("product-ft_sublot_info", "product", "product.product_name", "ft_sublot_info", "ft_sublot_info.product_name", "ft_sublot_info-ft_splitlot");
    m_mapLinks_Remote_Ft["ft_lot-ft_splitlot"] =
            GexDbPlugin_Mapping_Link("ft_lot-ft_splitlot", "ft_lot", "ft_lot.lot_id", "ft_splitlot", "ft_splitlot.lot_id", "ft_lot-ft_splitlot-2");
    m_mapLinks_Remote_Ft["ft_lot-ft_splitlot-2"] =
            GexDbPlugin_Mapping_Link("ft_lot-ft_splitlot-2", "ft_lot", "ft_lot.product_name", "ft_splitlot", "ft_splitlot.product_name", "");
    m_mapLinks_Remote_Ft["ft_ptest_info-ft_splitlot"] =
            GexDbPlugin_Mapping_Link("ft_ptest_info-ft_splitlot", "ft_ptest_info", "ft_ptest_info.splitlot_id", "ft_splitlot", "ft_splitlot.splitlot_id", "");
    m_mapLinks_Remote_Ft["ft_mptest_info-ft_splitlot"] =
            GexDbPlugin_Mapping_Link("ft_mptest_info-ft_splitlot", "ft_mptest_info", "ft_mptest_info.splitlot_id", "ft_splitlot", "ft_splitlot.splitlot_id", "");
    m_mapLinks_Remote_Ft["ft_ftest_info-ft_splitlot"] =
            GexDbPlugin_Mapping_Link("ft_ftest_info-ft_splitlot", "ft_ftest_info", "ft_ftest_info.splitlot_id", "ft_splitlot", "ft_splitlot.splitlot_id", "");
    m_mapLinks_Remote_Ft["ft_hbin-ft_splitlot"] =
            GexDbPlugin_Mapping_Link("ft_hbin-ft_splitlot", "ft_hbin", "ft_hbin.splitlot_id", "ft_splitlot", "ft_splitlot.splitlot_id", "");
    m_mapLinks_Remote_Ft["ft_sbin-ft_splitlot"] =
            GexDbPlugin_Mapping_Link("ft_sbin-ft_splitlot", "ft_sbin", "ft_sbin.splitlot_id", "ft_splitlot", "ft_splitlot.splitlot_id", "");
    m_mapLinks_Remote_Ft["ft_parts_stats_samples-ft_splitlot"] =
            GexDbPlugin_Mapping_Link("ft_parts_stats_samples-ft_splitlot", "ft_parts_stats_samples", "ft_parts_stats_samples.splitlot_id", "ft_splitlot", "ft_splitlot.splitlot_id", "");
    m_mapLinks_Remote_Ft["ft_die_tracking-ft_lot"] =
            GexDbPlugin_Mapping_Link("ft_die_tracking-ft_lot", "ft_die_tracking", "ft_die_tracking.ft_tracking_lot_id", "ft_lot", "ft_lot.tracking_lot_id", "ft_lot-ft_splitlot");
    m_mapLinks_Remote_Ft["ft_run-ft_splitlot"] =
            GexDbPlugin_Mapping_Link("ft_run-ft_splitlot", "ft_run", "ft_run.splitlot_id", "ft_splitlot", "ft_splitlot.splitlot_id", "");

    if (uiDbVersion_Build >= 18)
    {
        m_mapLinks_Remote_Ft["ft_test_conditions-ft_splitlot"] =
                GexDbPlugin_Mapping_Link("ft_test_conditions-ft_splitlot", "ft_test_conditions","ft_test_conditions.splitlot_id", "ft_splitlot","ft_splitlot.splitlot_id", "");
    }

    if (uiDbVersion_Build >= 27)
    {
        m_mapLinks_Remote_Ft["ft_sublot_info-ft_splitlot"] =
                GexDbPlugin_Mapping_Link("ft_sublot_info-ft_splitlot", "ft_sublot_info", "ft_sublot_info.lot_id", "ft_splitlot", "ft_splitlot.lot_id",	"ft_sublot_info-ft_splitlot-2");
        m_mapLinks_Remote_Ft["ft_sublot_info-ft_splitlot-2"] =
                GexDbPlugin_Mapping_Link("ft_sublot_info-ft_splitlot-2", "ft_sublot_info", "ft_sublot_info.sublot_id", "ft_splitlot", "ft_splitlot.sublot_id",	"");
    }

    if(uiDbVersion_Build >= 28)
    {
        m_mapLinks_Remote_Ft["ft_sublot_consolidation-ft_splitlot"] =
                GexDbPlugin_Mapping_Link("ft_sublot_consolidation-ft_splitlot", "ft_sublot_consolidation", "ft_sublot_consolidation.lot_id", "ft_splitlot", "ft_splitlot.lot_id",	"ft_sublot_consolidation-ft_splitlot-2");
        m_mapLinks_Remote_Ft["ft_sublot_consolidation-ft_splitlot-2"] =
                GexDbPlugin_Mapping_Link("ft_sublot_consolidation-ft_splitlot-2", "ft_sublot_consolidation", "ft_sublot_consolidation.sublot_id", "ft_splitlot", "ft_splitlot.sublot_id",	"");
    }

    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Loaded %1 metadata for FT.").arg( m_mapLinks_Remote_Ft.size()).toLatin1().constData());
}

QDomElement GexDbPlugin_Galaxy::GetSettingsDom(QDomDocument &doc)
{
    QDomElement eltPluginConfig = GexDbPlugin_Base::GetSettingsDom(doc);
    QDomElement eltOptions = doc.createElement("Options");
    eltOptions.appendChild(GetStartupTypeDom(doc));
    eltPluginConfig.appendChild(eltOptions);
    eltOptions.appendChild(GetAdrLinkDom(doc));
    eltPluginConfig.appendChild(eltOptions);

    return eltPluginConfig;
}

QDomElement GexDbPlugin_Galaxy::GetStartupTypeDom(QDomDocument &doc)
{
    QDomText domText;
    QDomElement domElt = doc.createElement("StartupType");
    if(m_bAutomaticStartup)
        domText = doc.createTextNode("1");
    else
        domText = doc.createTextNode("0");
    domElt.appendChild(domText);

    return domElt;
}

QDomElement GexDbPlugin_Galaxy::GetAdrLinkDom(QDomDocument &doc)
{
    QDomText domText;
    QDomElement domElt = doc.createElement("AdrLink");
    domText = doc.createTextNode(GetAdrLinkName());
    domElt.appendChild(domText);

    return domElt;
}


///////////////////////////////////////////////////////////
// Write settings to file using informations loaded in the class variables
///////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::WriteSettings(QTextStream *phFile)
{
    GEX_ASSERT(phFile);
    //GSLOG(SYSLOG_SEV_DEBUG, QString("Write Settings in TextStream %1").arg( phFile?phFile->device()->objectName()).toLatin1().constData():"NULL !"));

    GexDbPlugin_Base::WriteSettings(phFile);

    // Write options to file...
    *phFile << endl;
    *phFile << "<Options>" << endl;
    *phFile << "StartupType=" << QString::number(m_bAutomaticStartup) << endl;
    if(MustHaveAdrLink())
        *phFile << "AdrLink=" << GetAdrLinkName() << endl;
    *phFile << "</Options>" << endl;

    if(m_pclDatabaseConnector->IsSQLiteDB())
    {
        //if (phFile->device())
        // qDebug("GexDbPlugin_Galaxy::WriteSettings: SQLite : TextStream device = %s", phFile->device()->objectName().toLatin1().data());
        //qDebug("GexDbPlugin_Galaxy::WriteSettings: SQLite : db path is %s", phFile->string()->toLatin1().data());
    }

    // Success
    return true;
}

