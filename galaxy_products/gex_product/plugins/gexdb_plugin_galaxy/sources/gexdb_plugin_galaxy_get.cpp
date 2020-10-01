#include <QString>
#include <QSqlError>
#include <QCryptographicHash>
#include "gexdb_plugin_base.h"
#include "gexdb_plugin_galaxy.h"
#include "gexdb_plugin_galaxy_defines.h"
#include "gex_shared.h"
#include <gqtl_log.h>

QString GexDbPlugin_Galaxy::GetMatchingLotWafer(
        const QString TrackingLotID,
        const int TrackingWaferNB,
        const QString targetedTestingStage,
        QString &LotID, QString &WaferID)
{
    if (targetedTestingStage!=GEXDB_PLUGIN_GALAXY_WTEST && targetedTestingStage!=GEXDB_PLUGIN_GALAXY_ETEST)
    {
        return "error : TestingStage not supported for this method ! not yet implemented or no way to retrieve WaferID";
    }

    if (!SetTestingStage(targetedTestingStage))
    {
        GSLOG(SYSLOG_SEV_WARNING, QString("error setting testingStage %1")
              .arg( targetedTestingStage).toLatin1().constData());
        return "error setting TestingStage";
    }

    // TD-78

    QString strQuery;
    Query_Empty();

    m_strlQuery_Fields.append("Field|"+m_strTablePrefix+SPLITLOT_TN+".lot_id");

    if (!Query_AddValueCondition(GEXDB_PLUGIN_DBFIELD_TRACKING_LOT, TrackingLotID))
        return "error : can't Query_AddValueCondition(...) !";

    if (!Query_BuildSqlString(strQuery, true))
        return "error : can't Query_BuildSqlString !"; // distinct ?

    GexDbPlugin_Query	clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    clGexDbQuery.setForwardOnly(true);
    if(!clGexDbQuery.Execute(strQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        return QString("error : when executing query '%1' : %2").arg(strQuery.replace('\n',' '))
                .arg(clGexDbQuery.lastError().text());
    }

    if (clGexDbQuery.size()==0)		//if(!clGexDbQuery.Next())
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, "error : no Lot found for this TrackingLot in this TestingStage !", strQuery.toLatin1().constData() );
        return "error : no Lot found for this TrackingLot in this TestingStage !";
    }

    while (clGexDbQuery.next())  	//for (int i=0; i<clGexDbQuery.size(); i++) //numRowsAffected
    {
        LotID=clGexDbQuery.value(0).toString();
    }

    strQuery.clear();
    Query_Empty();
    // we cant use 'like' command because it returns '18' when '%8'
    //strQuery.append(" where wafer_id like '%"+TrackingWaferID+"'");
    //strQuery.append(" where LPAD(wafer_id,2,'0') = LPAD('" + TrackingWaferID + "',2,'0')");

    m_strlQuery_Fields.append("Field|"+m_strTablePrefix+SPLITLOT_TN+".wafer_id");

    if (!Query_AddValueCondition(GEXDB_PLUGIN_DBFIELD_WAFER_NB, QString::number(TrackingWaferNB)))
        return "error : can't Query_AddValueCondition(...) !";

    if (!Query_AddValueCondition(m_gexLabelFilterChoices[GEX_QUERY_FILTER_LOT], LotID))
        return "error : can't Query_AddValueCondition(...) !";

    if (!Query_BuildSqlString(strQuery, true))
        return "error : can't Query_BuildSqlString !"; // distinct ?

    GSLOG(SYSLOG_SEV_INFORMATIONAL, strQuery.replace('\n', ' ').toLatin1().data());

    if(!clGexDbQuery.Execute(strQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        return QString("error : when executing query '%1' : %2").arg(strQuery.replace('\n',' '))
                .arg(clGexDbQuery.lastError().text());
    }

    if (clGexDbQuery.size()==0)		//if(!clGexDbQuery.Next())
        return QString(
                    "error : no Wafer found for TrackingWaferNB '%1' in Lot '%2' in %3 TestingStage (%4)")
                .arg(TrackingWaferNB).arg(LotID).arg(targetedTestingStage)
                .arg(strQuery.replace('\n',' '));

    while (clGexDbQuery.next())  	//for (int i=0; i<clGexDbQuery.size(); i++) //numRowsAffected
    {
        WaferID=clGexDbQuery.value(0).toString();
    }

    GSLOG(SYSLOG_SEV_NOTICE, QString("TLot %1 and TWafer %2 at %3 is %4 %5")
           .arg(TrackingLotID).arg(TrackingWaferNB)
           .arg(targetedTestingStage)
           .arg(LotID).arg(WaferID)
           .toLatin1().data()
           );

    return "ok";
}

QString GexDbPlugin_Galaxy::GetSplitlotsList(
        QString LotID,
        QString WaferID,
        QString TS, QList<int> &l)
{
    QString strQuery; //strSubQuery, strFieldSpec;
    Query_Empty();

    if (!SetTestingStage(TS))
        return QString("error : unsupported testing stage '%s'").arg(TS);

    m_strlQuery_Fields.append("Field|"+m_strTablePrefix+SPLITLOT_TN+".splitlot_id");

    if (!Query_AddValueCondition(m_gexLabelFilterChoices[GEX_QUERY_FILTER_LOT], LotID))
        return "error : can't Query_AddValueCondition(...) !";
    if (!Query_AddValueCondition(m_gexLabelFilterChoices[GEX_QUERY_FILTER_WAFERID], WaferID))
        return "error : can't Query_AddValueCondition(...) !";

    if (!Query_BuildSqlString(strQuery, true))
        return "error : can't Query_BuildSqlString !"; // distinct ?

    GexDbPlugin_Query	clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    clGexDbQuery.setForwardOnly(true);
    if(!clGexDbQuery.Execute(strQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        return QString("error : when executing query '%1'").arg(strQuery.replace('\n',' '));
    }

    if (clGexDbQuery.size()==0) 	//if(!clGexDbQuery.Next())
        return QString("error : no %1 SplitLot found for this lot-wafer !").arg(TS);

    while (clGexDbQuery.next())  	//for (int i=0; i<clGexDbQuery.size(); i++) //numRowsAffected
    {
        l.push_back(clGexDbQuery.value(0).toInt());
    }

    return "ok";
}


bool GexDbPlugin_Galaxy::GetDistinctXY(
        const QString &Lot,
        const QString &Wafer,
        GexDbPlugin_Filter &cFilters,
        QMap< QPair<int, int>, QMap<QString, QVariant> > &xys)
{
    GSLOG(SYSLOG_SEV_DEBUG, (QString("L=%1 W=%2 already %3 XY")
          .arg( Lot)
          .arg(Wafer)
          .arg( xys.size() )).toLatin1().constData());

    if (!SetTestingStage(GEXDB_PLUGIN_GALAXY_WTEST))
        return "error";

    QString lQuery, lSubQuery;
    Query_Empty();

    // Add conditions on Lot and Wafer
    m_strlQuery_ValueConditions.append(m_strTablePrefix + SPLITLOT_TN + ".valid_splitlot|NotString|N");
    m_strlQuery_ValueConditions.append(m_strTablePrefix + SPLITLOT_TN + ".lot_id|String|"+Lot);
    m_strlQuery_ValueConditions.append(m_strTablePrefix + SPLITLOT_TN + ".wafer_id|String|"+Wafer);

    // Add extra filters
    if (!Query_AddFilters(cFilters))
        return false;

    // First extract the needed parts
    m_strlQuery_Fields.append("field|"+m_strTablePrefix + SPLITLOT_TN + ".start_t");
    m_strlQuery_Fields.append("field|"+m_strTablePrefix + RUN_TN + ".part_retest_index");
    m_strlQuery_Fields.append("field|"+m_strTablePrefix + SPLITLOT_TN + ".splitlot_id");
    m_strlQuery_Fields.append("field|"+m_strTablePrefix + RUN_TN + ".run_id");
    m_strlQuery_Fields.append("field|"+m_strTablePrefix + RUN_TN + ".part_x");
    m_strlQuery_Fields.append("field|"+m_strTablePrefix + RUN_TN + ".part_y");
    m_strlQuery_Fields.append("field|"+m_strTablePrefix + RUN_TN + ".sbin_no");
    m_strlQuery_Fields.append("field|"+m_strTablePrefix + RUN_TN + ".hbin_no");
    m_strlQuery_Fields.append("field|"+m_strTablePrefix + RUN_TN + ".part_id");

    // Construct query from table and conditions
    if(!Query_BuildSqlString(lSubQuery,false))
        return false;

    // Then agregate to extract the last PART
    lQuery = "SELECT ";
    if(m_pclDatabaseConnector->IsOracleDB())
        lQuery+= "MAX(T.start_t+T.part_retest_index||':'||T.splitlot_id||':'||T.run_id||':'||T.part_x||':'||T.part_y||':'||T.sbin_no||':'||T.hbin_no||':'||T.part_id) " ;
    else
        lQuery+= "MAX( CONCAT(T.start_t+T.part_retest_index, ':', T.splitlot_id, ':', T.run_id, ':', T.part_x, ':', T.part_y, ':', T.sbin_no,':',T.hbin_no,':',T.part_id) ) ";
    lQuery+= "FROM ";
    lQuery+= "( ";
    lQuery+= lSubQuery;
    lQuery+= ") T ";
    lQuery+= "GROUP BY T.part_x, T.part_y";

    GexDbPlugin_Query clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    if(!clGexDbQuery.Execute(lQuery))
    {

        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, lQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        return false;
    }

    // For all splitlots
    while(clGexDbQuery.Next())
    {
        QString lKey    = clGexDbQuery.value(0).toString();
        if (lKey.isEmpty())
        {
            GSET_ERROR1(GexDbPlugin_Base, eDB_NoResult, NULL, lQuery.toLatin1().constData());
            return false;
        }

        QStringList sl = lKey.split(':');

        QPair< int, int > p;
        p.first =sl.at(3).toInt();
        p.second=sl.at(4).toInt();

        QMap< QString, QVariant> a;
        a.insert("splitlot_id", sl.at(1));
        a.insert("run_id", sl.at(2));
        a.insert("sbin_no", sl.at(5));
        a.insert("hbin_no", sl.at(6));
        a.insert("part_id", sl.at(7));

        xys.insert(p,a);
    }

    return true;
}

GexDbPlugin_RunInfo* GexDbPlugin_Galaxy::GetPart(int desired_run_id)
{
    GexDbPlugin_RunInfo* ri=NULL;
    if (desired_run_id<m_nRunInfoArray_NbItems)
    {
        ri = m_pRunInfoArray + desired_run_id;
        if ( ri->m_nRunID==desired_run_id )
            return ri;
    }

    for (int i=0; i<m_nRunInfoArray_NbItems; i++)
    {
        ri = m_pRunInfoArray + i;
        if (ri && ri->m_nRunID==desired_run_id)
            return ri;
    }

    return NULL;
}


QString GexDbPlugin_Galaxy::GetETestSiteConfig(const QString &lotid, const int &wafer_number, QString site_config)
{
    // TD-78

    if (!SetTestingStage(GEXDB_PLUGIN_GALAXY_ETEST))
        return "error : cant set GEXDB_PLUGIN_GALAXY_ETEST testing stage !";

    QString q="select site_config from et_wafer_info where lot_id='$LOTID' and wafer_nb=$WAFERNB";
    q=q.replace("$LOTID", lotid);
    q=q.replace("$WAFERNB", QString("%1").arg(wafer_number) );

    Query_Empty();
    GexDbPlugin_Query clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    if(!clGexDbQuery.Execute(q))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, q.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        return QString("error : %1 %2").arg(clGexDbQuery.lastError().text()).arg(q);
    }

    if (clGexDbQuery.size()==0)
        return "error : lot-wafer not found !";

    // Should be only one but anyway
    while(clGexDbQuery.Next())
    {
        site_config=clGexDbQuery.value(0).toString();
    }

    return "ok";
}

void GexDbPlugin_Galaxy::GetSupportedTestingStages(QString & strSupportedTestingStages)
{
    strSupportedTestingStages = GEXDB_PLUGIN_GALAXY_FTEST;
    strSupportedTestingStages += ";";
    strSupportedTestingStages += GEXDB_PLUGIN_GALAXY_WTEST;
    strSupportedTestingStages += ";";
    strSupportedTestingStages += GEXDB_PLUGIN_GALAXY_ETEST;
}

void GexDbPlugin_Galaxy::GetTdrTypeName(QString & strTdrType)
{
    strTdrType = "Not recognized";
    if (!m_strDbHashType.isEmpty())
    {
        if (IsCharacTdr())
            strTdrType = GEXDB_CHAR_TDR_NAME;
        else if (IsManualProdTdr())
            strTdrType = GEXDB_MAN_PROD_TDR_NAME;
        else if (IsYmProdTdr())
            strTdrType = GEXDB_YM_PROD_TDR_NAME;
        else if (IsAdr())
            strTdrType = GEXDB_ADR_NAME;
        else if (IsLocalAdr())
            strTdrType = GEXDB_ADR_LOCAL_NAME;
    }
}

void GexDbPlugin_Galaxy::LoadRecordedTdrTypeIfEmpty()
{
    QString lFieldName = "db_type_recorded";

    if (m_strDbHashType.isEmpty() && mGexInfoMap.contains(lFieldName))
        m_strDbHashType = mGexInfoMap.value(lFieldName);
}


bool GexDbPlugin_Galaxy::IsCharacTdr()
{
    QString dbTdrType;
    dbTdrType = m_pclDatabaseConnector->m_strSchemaName;
    dbTdrType += GEXDB_CHAR_TDR_KEY;
    QByteArray dbHash = QCryptographicHash::hash(dbTdrType.toLatin1(),QCryptographicHash::Md5);
    QString hashName = dbHash.toHex();
    if (m_strDbHashType == hashName)
    {
        m_eDbType = eCharacTdrDb;
        return true;
    }
    if(m_eDbType == eCharacTdrDb)
        return true;

    return false;
}

bool GexDbPlugin_Galaxy::IsManualProdTdr()
{
    QString dbTdrType;
    dbTdrType = m_pclDatabaseConnector->m_strSchemaName;
    dbTdrType += GEXDB_MAN_PROD_TDR_KEY;
    QByteArray dbHash = QCryptographicHash::hash(dbTdrType.toLatin1(),QCryptographicHash::Md5);
    QString hashName = dbHash.toHex();
    if (m_strDbHashType == hashName)
    {
        m_eDbType = eManualProdDb;
        return true;
    }
    if(m_eDbType == eManualProdDb)
        return true;

    return false;
}

bool GexDbPlugin_Galaxy::IsYmProdTdr()
{
    QString dbTdrType;
    dbTdrType = m_pclDatabaseConnector->m_strSchemaName;
    dbTdrType += GEXDB_YM_PROD_TDR_KEY;
    QByteArray dbHash = QCryptographicHash::hash(dbTdrType.toLatin1(),QCryptographicHash::Md5);
    QString hashName = dbHash.toHex();
    if (m_strDbHashType == hashName)
    {
        m_eDbType = eProdTdrDb;
        return true;
    }
    if(m_eDbType == eProdTdrDb)
        return true;

    return false;
}

bool GexDbPlugin_Galaxy::IsAdr()
{
    QString dbTdrType;
    dbTdrType = m_pclDatabaseConnector->m_strSchemaName;
    dbTdrType += GEXDB_ADR_KEY;
    QByteArray dbHash = QCryptographicHash::hash(dbTdrType.toLatin1(),QCryptographicHash::Md5);
    QString hashName = dbHash.toHex();
    if (m_strDbHashType == hashName)
    {
        m_eDbType = eAdrDb;
        return true;
    }
    if(m_eDbType == eAdrDb)
        return true;

    return false;
}

bool GexDbPlugin_Galaxy::IsLocalAdr()
{
    QString dbTdrType;
    dbTdrType = m_pclDatabaseConnector->m_strSchemaName;
    dbTdrType += GEXDB_ADR_LOCAL_KEY;
    QByteArray dbHash = QCryptographicHash::hash(dbTdrType.toLatin1(),QCryptographicHash::Md5);
    QString hashName = dbHash.toHex();
    if (m_strDbHashType == hashName)
    {
        m_eDbType = eAdrLocalDb;
        return true;
    }
    if(m_eDbType == eAdrLocalDb)
        return true;

    return false;
}

bool GexDbPlugin_Galaxy::IsReportsCenterSupported()
{
    if (m_uiDbVersionBuild >= GEXDB_DB_VERSION_BUILD_B28)
        return true;

    return false;
}


bool GexDbPlugin_Galaxy::GetTotalSize(int &size)
{
    size=0;

    if (!m_pclDatabaseConnector)
    {
        GSLOG(SYSLOG_SEV_ERROR, " m_pclDatabaseConnector NULL !");
        return false;
    }
    if (!m_pclDatabaseConnector->IsConnected())
    {
        GSLOG(SYSLOG_SEV_ERROR, " m_pclDatabaseConnector not connected !");
        return false;
    }


    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Get total size of  %1")
          .arg( m_pclDatabaseConnector->m_strConnectionName).toLatin1().constData());

    if(!m_pclDatabaseConnector->IsMySqlDB() && !m_pclDatabaseConnector->IsOracleDB())
        return false;

    QSqlQuery	clQuery(QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    QString strQuery;
    if (m_pclDatabaseConnector->IsMySqlDB())
    {
        // USE A MYSQL SCHEDULER TO COMPUTE THE DATABASE SIZE
        // CHECK IF THE SCHEDULER IS ON AND IF THE EVENT IS DEFINED
        // THE SCHEDULER IS ACTIVE IF THE OPTION 'GEXDB_MYSQL_SIZE' exists
        // IF NOT, USE THE OLD PROCESS
        strQuery = "SELECT (option_value+0.0) FROM global_options WHERE option_name='GEXDB_MYSQL_SIZE'";
        if(!clQuery.exec(strQuery) || !clQuery.first())
        {
            // SCHEDULER IS NOT ACTIVE
            // USE THE OLD PROCESS
            strQuery = "SELECT SUM(DATA_LENGTH+INDEX_LENGTH)/(1024*1024) ";
            strQuery+= "FROM information_schema.TABLES ";
            strQuery+= "WHERE TABLE_SCHEMA='";
            strQuery+=  m_pclDatabaseConnector->m_strSchemaName;
            strQuery+= "'";

            // Optimization div by 4 or 8 the execution time
            strQuery = "SELECT SUM(DATA_LENGTH+INDEX_LENGTH)/(1024*1024)*1.4 ";
            strQuery+= "FROM information_schema.TABLES ";
            strQuery+= "WHERE TABLE_NAME like '%test%' ";
            strQuery+= "AND TABLE_SCHEMA='";
            strQuery+=  m_pclDatabaseConnector->m_strSchemaName;
            strQuery+= "'";

            // Optimization div by 2 or 4 the execution time
            strQuery = "SELECT SUM(DATA_LENGTH+INDEX_LENGTH)/(1024*1024)*1.02 ";
            strQuery+= "FROM information_schema.TABLES ";
            strQuery+= "WHERE (TABLE_NAME like '%test%' OR TABLE_NAME like '%run') ";
            strQuery+= "AND TABLE_SCHEMA='";
            strQuery+=  m_pclDatabaseConnector->m_strSchemaName;
            strQuery+= "'";

            // Management of the ADR size with 2-4 factor optimization
            strQuery = "SELECT SUM(DATA_LENGTH+INDEX_LENGTH)/(1024*1024) ";
            strQuery+= "FROM information_schema.TABLES ";
            strQuery+= "WHERE (TABLE_NAME like '%test%' OR TABLE_NAME like '%run' OR TABLE_NAME like '%adr%') ";
            strQuery+= "AND TABLE_SCHEMA='";
            strQuery+=  m_pclDatabaseConnector->m_strSchemaName;
            strQuery+= "'";
        }
    }
    else if (m_pclDatabaseConnector->IsOracleDB())
    {
        strQuery = "SELECT SUM(size_in_octet)/(1024*1024) FROM ";
        strQuery+= "( ";
        strQuery+= "SELECT (num_rows*avg_row_len) AS size_in_octet FROM user_tables ";
        strQuery+= " UNION ";
        strQuery+= "SELECT (num_rows*avg_leaf_blocks_per_key) AS size_in_octet FROM user_indexes ";
        strQuery+= ")";
    }
    else
        return false;

    QueryThread clQueryThread;
    clQuery.prepare(strQuery);

    if(!clQueryThread.exec(&clQuery))
        return false;

    if (!clQuery.first())
        return false;

    size = clQuery.value(0).toInt();
    return true;
}

///////////////////////////////////////////////////////////
// Get nb of Splitlots matching query
///////////////////////////////////////////////////////////
int GexDbPlugin_Galaxy::GetNbOfSplitlots(GexDbPlugin_Filter &cFilters)
{
    QString strQuery, strSubQuery, strFieldSpec;
    int		nNbSplitlots = 0;

    Query_Empty();

    // Construct subquery
    // SELECT distinct splitlot_id
    // FROM <all required tables>
    // WHERE <link conditions> AND <value conditions>	(optional)
    // Set field
    strFieldSpec = "Field|";
    strFieldSpec += m_strTablePrefix;
    strFieldSpec += "splitlot.splitlot_id";
    m_strlQuery_Fields.append(strFieldSpec);

    // Set filters
    Query_AddFilters(cFilters);

    // Add time period condition
    Query_AddTimePeriodCondition(cFilters);

    // Add min part condition
    Query_AddValueCondition(GEXDB_PLUGIN_DBFIELD_PARTS, QString(">=%1").arg(QString::number(cFilters.mMinimumPartsInFile)));

    // For all query, add a VALID_SPLITLOT<>'N'
    m_strlQuery_ValueConditions.append(NormalizeTableName("_splitlot")+".valid_splitlot|NotString|N");

    // Construct query from table and conditions
    Query_BuildSqlString(strSubQuery, true);

    // Construct query string:
    // SELECT count(*)
    // FROM (subquery)
    strQuery = "SELECT\n";
    strQuery += "count(*)\n";
    strQuery += "FROM\n(";
    strQuery += strSubQuery;

    if (!m_pclDatabaseConnector)
        return -1;

    if(m_pclDatabaseConnector->m_strDriver == "QMYSQL3")
        strQuery += ") AS T1\n";
    else
        strQuery += ")\n";

    // Execute query
    GexDbPlugin_Query	clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    clGexDbQuery.setForwardOnly(true);
    if(!clGexDbQuery.Execute(strQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        return -1;
    }

    if(clGexDbQuery.Next())
    {
        nNbSplitlots = clGexDbQuery.value(0).toInt();
        if(m_bCustomerDebugMode)
            clGexDbQuery.DumpPerformance();
    }

    return nNbSplitlots;
}

QString   GexDbPlugin_Galaxy::GetSplitlotBinCountsPerSite(
        const QString &TestingStage,
        long lSplitlotID,
        QMap<int, GexDbPlugin_BinList> &mapBinsPerSite)
{
    GSLOG(SYSLOG_SEV_NOTICE,
          (QString("Get Splitlot Bin Counts Per Site for TS %1 splitlot %2 (already %3 sites in given map)...")
          .arg(TestingStage.toLatin1().data())
          .arg(lSplitlotID)
          .arg(mapBinsPerSite.size())).toLatin1().constData());

    if (IsTestingStage_Foundry(TestingStage))
    {
        QString e("Get splitlot bin counts per site does support Etest stage.");
        GSLOG(SYSLOG_SEV_ERROR, e.toLatin1().data());
        return "error:"+e;
    }

    char TS_prefix='w';
    if (IsTestingStage_FinalTest(TestingStage))
        TS_prefix='f';

    // get flag
    QString q="select splitlot_flags from "+QString(TS_prefix)+"t_splitlot where splitlot_id="+QString::number(lSplitlotID);
    GexDbPlugin_Query	clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    clGexDbQuery.setForwardOnly(true);
    if(!clGexDbQuery.Execute(q))
    {
        //GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        return "error : cant execute query "+q+" : "+clGexDbQuery.lastError().text();
    }

    if(!clGexDbQuery.Next())
    {
        return "error : Splitlot "+QString::number(lSplitlotID)+" not found in this TS "+TestingStage;
    }

    bool ok=false;
    int splitlot_flags = clGexDbQuery.value(0).toInt(&ok);
    GSLOG(SYSLOG_SEV_NOTICE, (QString("Splitlot %1 has splitlot_flags %2 (%3)")
          .arg( lSplitlotID)
          .arg(splitlot_flags)
          .arg( ok?"":"error")).toLatin1().constData()
            );
    //(splitlot_flags & 4)
    bool useSamples=splitlot_flags & FLAG_SPLITLOT_PARTSFROMSAMPLES;

    GSLOG(SYSLOG_SEV_NOTICE, QString("Will use %1 stats")
          .arg( (useSamples)?"samples":"summary").toLatin1().constData());

    if (useSamples)
    {
        q="select SL.splitlot_id, SL.part_typ, SL.lot_id, SBS.sbin_no, SB.sbin_name, SB.sbin_cat, \
                P.site_no, P.nb_parts as totparts, SBS.nb_parts as totbin, SBS.nb_parts/P.nb_parts as yield \
                from \
                $TSt_splitlot SL \
                inner join $TSt_parts_stats_samples P on P.splitlot_id=SL.splitlot_id \
                inner join $TSt_sbin_stats_samples SBS on SBS.splitlot_id=P.splitlot_id and SBS.site_no=P.site_no \
                inner join $TSt_sbin SB on SB.splitlot_id=SBS.splitlot_id and SB.sbin_no=SBS.sbin_no \
                where SL.splitlot_id='$SPLITLOTID' and P.site_no != -1 and SBS.site_no != -1 \
                order by sbin_no, site_no";
                q.replace("$TS", QString(TS_prefix) );
        q.replace("$SPLITLOTID", QString::number(lSplitlotID) );
    }
    else
    {
        q="select SL.splitlot_id, SL.part_typ, SL.lot_id, SBS.sbin_no, SB.sbin_name, SB.sbin_cat, \
                P.site_no, P.nb_parts as totparts, SBS.bin_count as totbin, SBS.bin_count/P.nb_parts as yield \
                from \
                $TSt_splitlot SL \
                inner join $TSt_parts_stats_summary P on P.splitlot_id=SL.splitlot_id \
                inner join $TSt_sbin_stats_summary SBS on SBS.splitlot_id=P.splitlot_id and SBS.site_no=P.site_no \
                inner join $TSt_sbin SB on SB.splitlot_id=SBS.splitlot_id and SB.sbin_no=SBS.sbin_no \
                where SL.splitlot_id='$SPLITLOTID' and P.site_no != -1 and SBS.site_no != -1 \
                order by sbin_no, site_no";
                q.replace("$TS", QString(TS_prefix) );
        q.replace("$SPLITLOTID", QString::number(lSplitlotID) );
    }

    if(!clGexDbQuery.Execute(q))
    {
        //GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        return "error : cant execute query "+q+" : "+clGexDbQuery.lastError().text();
    }

    while(clGexDbQuery.Next())
    {
        int site=clGexDbQuery.value(6).toInt(&ok);
        if (!ok)
            continue;
        GexDbPlugin_BinInfo* bi=new GexDbPlugin_BinInfo();
        QString cat=clGexDbQuery.value(5).toString();
        QChar c='?';
        if (cat.size()>0) c=cat.at(0);
        bi->m_cBinCat=c;
        bi->m_nBinCount=clGexDbQuery.value(8).toInt();
        bi->m_nBinNo=clGexDbQuery.value(3).toInt();
        bi->m_strBinName=clGexDbQuery.value(4).toString();
        //GSLOG(SYSLOG_SEV_NOTICE, QString("BinInfo site %1 : bin no %2 count %3 : %4").arg( site, bi->m_nBinNo.arg( bi->m_nBinCount, bi));.arg(        //GSLOG(SYSLOG_SEV_NOTICE, "BinInfo site %1 : bin no %2 count %3 : %4").arg( site.arg( bi->m_nBinNo.arg( bi->m_nBinCount, bi));.arg(        //GSLOG(SYSLOG_SEV_NOTICE, "BinInfo site %1 : bin no %2 count %3 : %4").arg( site.arg( bi->m_nBinNo.arg( bi->m_nBinCount, bi));.arg(        //GSLOG(SYSLOG_SEV_NOTICE.arg( "BinInfo site %1 : bin no %2 count %3 : %4").arg( site.arg( bi->m_nBinNo.arg( bi->m_nBinCount, bi));.arg(        //GSLOG(SYSLOG_SEV_NOTICE, "BinInfo site %1 : bin no %2 count %3 : %4").arg( site.arg( bi->m_nBinNo.arg( bi->m_nBinCount, bi));.arg(        //GSLOG(SYSLOG_SEV_NOTICE.arg( "BinInfo site %1 : bin no %2 count %3 : %4").arg( site.arg( bi->m_nBinNo.arg( bi->m_nBinCount, bi));.arg(        //GSLOG(SYSLOG_SEV_NOTICE.arg( "BinInfo site %1 : bin no %2 count %3 : %4").arg( site.arg( bi->m_nBinNo.arg( bi->m_nBinCount, bi));.arg(        //GSLOG(SYSLOG_SEV_NOTICE.arg( "BinInfo site %1 : bin no %2 count %3 : %4").arg( site.arg( bi->m_nBinNo.arg( bi->m_nBinCount.arg( bi));
        //GexDbPlugin_BinList bl=mapBinsPerSite[site]; //mapBinsPerSite.value(site); // value() returns a const, [] a ref
        //bl.append(bi);
        if (mapBinsPerSite.contains(site))
            mapBinsPerSite[site].append(bi);
        else
        {
            GexDbPlugin_BinList bl; //=mapBinsPerSite[site];
//            bl.setAutoDelete(false);
            bl.append(bi);
            mapBinsPerSite.insert(site, bl);
        }
    }

    return "ok : dev validate me";
}

/////////////////////////////////////////////////////////
// Get the list of bining from the database using Filters
// bSoftBin = true => extract soft bin count
// cFilters.bConsolidatedData = false => use sbin_stats table (splitlot level)
// cFilters.strlQueryFilters => List of query filters. Each filter uses syntax <Field>=<Value>
// lGroupBy = field => one value between product_name,lot_id,wafer_id/sublot_id,site_no,bin_no
// mapBins => concatened groupby values separated by #GEX#
// ex1:
//  cFilters.bConsolidatedData = false
//  cFilters.strlQueryFilters = <splitlot_id=13012000154>
//  lGroupBy = site_no
//  bSoftBin = true
//      USE TABLE XT_SBIN_STATS_S
//  mapBins =>  ["1"] = list of bin counts
//              ["2"] = list of bin counts
//  ex2:
//  cFilters.bConsolidatedData = false
//  cFilters.strlQueryFilters = <product_name="product_1">
//  lGroupBy = lot_id,site_no
//  bSoftBin = true
//      USE TABLE XT_SBIN_STATS_S
//  mapBins =>  ["lot_1#GEX#1"] = list of bin counts
//              ["lot_1#GEX#2"] = list of bin counts
//              ["lot_2#GEX#1"] = list of bin counts
//              ["lot_2#GEX#2"] = list of bin counts
//  ex3:
//  cFilters.bConsolidatedData = true
//  cFilters.strlQueryFilters = <product_name="product_1">
//  lGroupBy = lot_id
//  bSoftBin = true
//      USE TABLE XT_LOT_SBIN
//  mapBins =>  ["lot_1"] = list of bin counts
//              ["lot_2"] = list of bin counts
//  ex4:
//  cFilters.bConsolidatedData = true
//  cFilters.strlQueryFilters = <product_name="product_1">
//  lGroupBy = lot_id,site_no
//      ERROR NO SITE_NO INTO CONSOLIDATED TABLES
//  ex5:
//  cFilters.bConsolidatedData = true
//  lGroupBy = product_name
//  bSoftBin = true
//      USE TABLE XT_PRODUCT_SBIN
//  mapBins =>  ["product_1"] = list of bin counts
//              ["product_2"] = list of bin counts
/////////////////////////////////////////////////////////


/*
  Usage example
    // Get the (raw) bin counts per group by for the given filter.
    GexDbPlugin_Filter cFilters(NULL);
    QStringList lGroupBy;
    bool bSoftBin;
    QMap<QString, GexDbPlugin_BinList> mapBinsPerKey;

    QString filter;
    // Test 1
    //cFilters.bConsolidatedData = false;
    //cFilters.strDataTypeQuery = "Final Test";
    //filter = gexLabelFilterChoices[GEX_QUERY_FILTER_LOT];
    //cFilters.strlQueryFilters.append(filter+"=HTOL_repeatability");
    // Test 2
    //cFilters.bConsolidatedData = false;
    //cFilters.strDataTypeQuery = "Final Test";
    //filter = "SplitlotID=1230600305";
    //cFilters.strlQueryFilters.append(filter);
    //lGroupBy << "site_no";
    // Test 3
//    cFilters.bConsolidatedData = true;
//    cFilters.strDataTypeQuery = "Wafer Sort";
//    bSoftBin = false;
//    filter = gexLabelFilterChoices[GEX_QUERY_FILTER_LOT];
//    cFilters.strlQueryFilters.append(filter+"=x29136");
//    filter = gexLabelFilterChoices[GEX_QUERY_FILTER_WAFERID];
//    cFilters.strlQueryFilters.append(filter+"=14");
//    lGroupBy << "lot_id" << "wafer_id";
    // Test 4
//    cFilters.bConsolidatedData = true;
//    cFilters.strDataTypeQuery = "Wafer Sort";
//    bSoftBin = false;
//    filter = gexLabelFilterChoices[GEX_QUERY_FILTER_PRODUCT];
//    cFilters.strlQueryFilters.append(filter+"=ANBAD40BCAA");
//    filter = gexLabelFilterChoices[GEX_QUERY_FILTER_LOT];
//    cFilters.strlQueryFilters.append(filter+"=x29136");
//    filter = gexLabelFilterChoices[GEX_QUERY_FILTER_WAFERID];
//    cFilters.strlQueryFilters.append(filter+"=14");
//    lGroupBy << "lot_id";
    // Test 5
    cFilters.bConsolidatedData = true;
    cFilters.strDataTypeQuery = "Wafer Sort";
    bSoftBin = false;
    filter = gexLabelFilterChoices[GEX_QUERY_FILTER_PRODUCT];
    cFilters.strlQueryFilters.append(filter+"=ANBAD40BCAA");
    lGroupBy << "product_name";

    pDatabaseEntry->m_pExternalDatabase->GetPluginID()
                            ->m_pPlugin->GetBinCounts(cFilters, lGroupBy, bSoftBin, mapBinsPerKey);

  */
QString   GexDbPlugin_Galaxy::GetBinCounts(
        GexDbPlugin_Filter & cFilters,
        QStringList lGroupBy,
        bool bSoftBin,
        QMap<QString, GexDbPlugin_BinList> &mapBinsPerKey)
{
    /*
    GetBinCounts
    From consolidatedData
            From HBin or SBin
            From Lot consolidation or Product consolidation
    Aggregate by site or product or ...
    Group by argument

    CONSOLIDATED DATA
    xt_product_hbin (product_name,bin_no,bin_name,bin_cat,nb_parts)
    xt_lot_hbin (lot_id,hbin_no,hbin_name,hbin_cat,nb_parts)
    xt_wafer_hbin (lot_id,wafer_id,hbin_no,hbin_name,hbin_cat,nb_parts)
        POSSIBLE GROUP BY product_name +- lot_id +- wafer_id

    SPLITLOT DATA
    xt_hbin_stats_samples (splitlot_id,site_no,hbin_no,nb_parts)
    xt_hbin_stats_summary (splitlot_id,site_no,hbin_no,bin_count)
        POSSIBLE GROUP BY site_no

    GROUP BY condition
    splitlot_id  => NO CONSOLIDATED DATA
    site => NO CONSOLIDATED DATA
*/
    QStringList allowedGroupByField = QStringList() << "product_name" << "part_typ" << "lot_id" << "wafer_id" << "sublot_id" << "splitlot_id" << "site_no";
    QStringList groupBy;
    QString field;
    // Check group by criteria
    foreach(field,lGroupBy)
    {
        field = field.toLower();
        if(!allowedGroupByField.contains(field))
            return "error : invalid group by field["+field+"]";

        // map correct field for product
        if(cFilters.bConsolidatedData && (field=="part_typ"))
            field = "product_name";
        if(!cFilters.bConsolidatedData && (field=="product_name"))
            field = "part_typ";

        groupBy << field;
    }

    if(cFilters.bConsolidatedData &&
            (groupBy.contains("site_no") ||
             groupBy.contains("splitlot_id") ||
             groupBy.contains("part_typ")))
        return "error : invalid group by field for consolidated data";

    QString TestingStage = cFilters.strDataTypeQuery;
    GSLOG(SYSLOG_SEV_NOTICE, QString("Get Bin Counts for TS %1 ").arg(TestingStage).toLatin1().constData());

    SetTestingStage(TestingStage);

    if(IsTestingStage_Foundry(TestingStage) &&
            groupBy.contains("site_no"))
        return "error : invalid group by field[site_no] for "+TestingStage;

    // Soft bin or Hard bin counts
    QString bin_type = "h";
    if(bSoftBin) bin_type = "s";

    GexDbPlugin_SplitlotList clSplitlotList;
    foreach(field,cFilters.strlQueryFilters)
    {
        if(field.startsWith("SplitlotID=",Qt::CaseInsensitive))
        {
            if(!m_pmapFields_GexToRemote->contains("SplitlotID"))
            {
                m_pmapFields_GexToRemote->insert("SplitlotID",
                        GexDbPlugin_Mapping_Field("SplitlotID", "SplitlotID", NormalizeTableName("_splitlot"), NormalizeTableName("_splitlot")+".splitlot_id", "",
                                                  false, "N",false, false, false, false, false, true, false));
            }
        }
    }

    // Check if have splitlot_id from cFilter clause
    if(!QuerySplitlots(cFilters,clSplitlotList))
        return "error : no splitlot list";

    if(clSplitlotList.count()==0)
        return "error : no splitlot list";

    // get flag
    GexDbPlugin_Query	clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    clGexDbQuery.setForwardOnly(true);
    QString strQuery;
    //int     site;
    bool    useSamples;
    bool    ok=false;
    qlonglong lSplitlotID;

    // Clear the mapBinsPerSite
    mapBinsPerKey.clear();
    Query_Empty();

    // Set filters
    Query_AddFilters(cFilters);

    if(!cFilters.bConsolidatedData)
    {
        // Retrieve all bin count from splitlot tables
        // Use all splitlot_id selected with the cFilter clause
        foreach(GexDbPlugin_SplitlotInfo* SplitlotInfo, clSplitlotList)
        {
            lSplitlotID = SplitlotInfo->m_lSplitlotID;
            useSamples=true;
            // Check if stats from summary or samples
            // Ignore for Etest - always samples
            if(!IsTestingStage_Foundry(TestingStage))
            {
                strQuery="SELECT splitlot_flags FROM "+NormalizeTableName("_splitlot")+" WHERE splitlot_id="+QString::number(lSplitlotID);
                if(!clGexDbQuery.Execute(strQuery))
                {
                    //GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
                    return "error : cant execute query "+strQuery+" : "+clGexDbQuery.lastError().text();
                }

                if(!clGexDbQuery.Next())
                {
                    return "error : Splitlot "+QString::number(lSplitlotID)+" not found in this TS "+TestingStage;
                }

                int splitlot_flags = clGexDbQuery.value(0).toInt(&ok);
                //(splitlot_flags & 4)
                useSamples=splitlot_flags & FLAG_SPLITLOT_PARTSFROMSAMPLES;

            }

            GSLOG(SYSLOG_SEV_DEBUG, (QString("Splitlot %1 has splitlot_flags=%2")
                  .arg( lSplitlotID)
                  .arg((useSamples)?"samples":"summary")).toLatin1().constData());

            // Add group by fields
            QString splitlotTable;
            QString binDescTable;
            QString binCountTable;
            QString binCountField;
            splitlotTable = NormalizeTableName("_splitlot");
            binDescTable = binCountTable = NormalizeTableName("_"+ bin_type + "bin");
            binCountField = "bin_count";
            // et_sbin.bin_count
            // et_hbin.bin_count
            // ft_sbin_stats_samples.nb_parts
            // ft_hbin_stats_samples.nb_parts
            // wt_sbin_stats_samples.nb_parts
            // wt_hbin_stats_samples.nb_parts
            // ft_sbin_stats_summary.bin_count
            // ft_hbin_stats_summary.bin_count
            // wt_sbin_stats_summary.bin_count
            // wt_hbin_stats_summary.bin_count
            if(!IsTestingStage_Foundry(TestingStage))
            {
                if(useSamples)
                {
                    binCountTable = NormalizeTableName("_"+ bin_type + "bin_stats_samples");
                    binCountField = "nb_parts";
                }
                else
                    binCountTable = NormalizeTableName("_"+ bin_type + "bin_stats_summary");
            }

            QString table;
            foreach(field, groupBy)
            {
                table = splitlotTable;
                if(field == "site_no")
                {
                    table = binCountTable;
                    m_strlQuery_OrderFields.append(table + "."+field);
                }

                m_strlQuery_Fields.append("Field|"+table + "."+field);
                m_strlQuery_GroupFields.append(table + "."+field);
            }

            // Add bin_no group by
            m_strlQuery_Fields.append("Field|"+binCountTable + "."+bin_type + "bin_no");
            m_strlQuery_GroupFields.append(binCountTable + "."+bin_type + "bin_no");
            m_strlQuery_OrderFields.append(binCountTable + "."+bin_type + "bin_no");

            // Add bin description
            m_strlQuery_Fields.append("Field|"+binDescTable+"."+bin_type+"bin_name");
            m_strlQuery_Fields.append("Field|"+binDescTable+"."+bin_type+"bin_cat");
            // Add bin counts
            m_strlQuery_Fields.append("Field|"+binCountTable+"."+binCountField);

            m_strlQuery_ValueConditions.append( m_strTablePrefix + "splitlot.splitlot_id|Numeric|" + QString::number(lSplitlotID));

            if (!Query_BuildSqlString(strQuery, true)) // distinct or not ?
                return "error : failed to build SQL query.";

            if(!clGexDbQuery.Execute(strQuery))
                return "error : cant execute query "+strQuery+" : "+clGexDbQuery.lastError().text();

            int  site;
            int  nIndex;
            bool bAddNewBinInfo = true;
            bool bHaveMergedSite = false;
            GexDbPlugin_BinInfo* bi = NULL;
            QString lKey;
            QString lMergedSiteKey;
            QString lSep;
            while(clGexDbQuery.Next())
            {
                // site info
                // if group by site
                // can have -1 for merge all sites
                // can have 0, 1, 2, ...
                // 256 for no site info
                site = 256;
                nIndex = 0;
                lKey = lSep = "";
                foreach(field, groupBy)
                {
                    if(field == "site_no")
                        site = clGexDbQuery.value(nIndex).toInt();

                    lKey += lSep + clGexDbQuery.value(nIndex).toString();
                    nIndex++;
                    lSep = "#GEX#";
                }
                if(lKey.isEmpty())
                    lKey = "all";

                bAddNewBinInfo = false;
                bi = NULL;
                // Check if already have info from this lKey and this bin
                // Use it to add new count
                if(mapBinsPerKey.contains(lKey))
                {
                    foreach(GexDbPlugin_BinInfo* BinInfo, mapBinsPerKey[lKey])
                    {
                        if(BinInfo->m_nBinNo == clGexDbQuery.value(nIndex).toInt())
                        {
                            bi = BinInfo;
                            break;
                        }
                    }
                }
                // Not found
                // Create new entry
                if(bi == NULL)
                {
                    bAddNewBinInfo = true;
                    bi=new GexDbPlugin_BinInfo();
                }

                bi->m_nBinNo    =   clGexDbQuery.value(nIndex).toInt(); nIndex++;
                bi->m_strBinName=   clGexDbQuery.value(nIndex).toString(); nIndex++;
                QString cat     =   clGexDbQuery.value(nIndex).toString(); nIndex++;
                QChar c='?';
                if (cat.size()>0) c=cat.at(0);
                bi->m_cBinCat   =   c;
                bi->m_nBinCount +=  clGexDbQuery.value(nIndex).toInt();

                // Order by site_no (first is -1 if exists)
                if(site==-1)
                {
                    bHaveMergedSite = true;
                    lMergedSiteKey = lKey;
                }
                if(bHaveMergedSite && site>-1)
                {
                    // From summary table, we can have merge site info (all count) and split site info
                    // ignore merge sites count if have info for each other site
                    mapBinsPerKey.remove(lMergedSiteKey);
                }

                if(!mapBinsPerKey.contains(lKey))
                {
                    GexDbPlugin_BinList bl;
                    //bl.setAutoDelete(false);
                    bl.append(bi);
                    mapBinsPerKey[lKey] = bl;
                }
                if(bAddNewBinInfo)
                    mapBinsPerKey[lKey].append(bi);
            }
        }
    }
    else
    {
        // Consolidated data
        // Add group by fields
        QString consolidatedTable;
        QString binField;
        binField = bin_type;
        // et_lot_hbin.nb_parts
        // et_lot_sbin.nb_parts
        // et_product_hbin.nb_parts
        // et_product_sbin.nb_parts
        // ft_lot_hbin.nb_parts
        // ft_lot_sbin.nb_parts
        // ft_product_hbin.nb_parts
        // ft_product_sbin.nb_parts
        // wt_wafer_hbin.nb_parts
        // wt_wafer_sbin.nb_parts
        // wt_lot_hbin.nb_parts
        // wt_lot_sbin.nb_parts
        // wt_product_hbin.nb_parts
        // wt_product_sbin.nb_parts
        if(groupBy.contains("wafer_id"))
            consolidatedTable = NormalizeTableName("_wafer_"+ bin_type + "bin");
        else if(groupBy.contains("lot_id"))
            consolidatedTable = NormalizeTableName("_lot_"+ bin_type + "bin");
        else
        {
            consolidatedTable = NormalizeTableName("_product_"+ bin_type + "bin");
            binField = "";
        }

        QString table;
        foreach(field, groupBy)
        {
            table = consolidatedTable;
            if(field == "product_name" && (consolidatedTable != NormalizeTableName("_product_"+ bin_type + "bin")))
                table = NormalizeTableName("_lot");
            m_strlQuery_Fields.append("Field|"+consolidatedTable + "."+field);
            m_strlQuery_GroupFields.append(consolidatedTable + "."+field);
        }

        // Add bin_no group by
        m_strlQuery_Fields.append("Field|"+consolidatedTable + "."+binField + "bin_no");
        m_strlQuery_GroupFields.append(consolidatedTable + "."+binField + "bin_no");

        // Add bin description
        m_strlQuery_Fields.append("Field|"+consolidatedTable+"."+binField+"bin_name");
        m_strlQuery_Fields.append("Field|"+consolidatedTable+"."+binField+"bin_cat");
        // Add bin counts
        m_strlQuery_Fields.append("Field|"+consolidatedTable+".nb_parts");

        if (!Query_BuildSqlString(strQuery, true)) // distinct or not ?
            return "error : failed to build SQL query.";

        if(!clGexDbQuery.Execute(strQuery))
            return "error : cant execute query "+strQuery+" : "+clGexDbQuery.lastError().text();

        int  nIndex;
        bool bAddNewBinInfo = true;
        GexDbPlugin_BinInfo* bi = NULL;
        QString lKey;
        QString lSep;
        while(clGexDbQuery.Next())
        {
            nIndex = 0;
            lKey = lSep = "";
            foreach(field, groupBy)
            {
                lKey += lSep + clGexDbQuery.value(nIndex).toString();
                nIndex++;
                lSep = "#GEX#";
            }
            if(lKey.isEmpty())
                lKey = "all";

            bAddNewBinInfo = false;
            bi = NULL;
            // Check if already have info from this lKey and this bin
            // Use it to add new count
            if(mapBinsPerKey.contains(lKey))
            {
                foreach(GexDbPlugin_BinInfo* BinInfo, mapBinsPerKey[lKey])
                {
                    if(BinInfo->m_nBinNo == clGexDbQuery.value(nIndex).toInt())
                    {
                        bi = BinInfo;
                        break;
                    }
                }
            }
            // Not found
            // Create new entry
            if(bi == NULL)
            {
                bAddNewBinInfo = true;
                bi=new GexDbPlugin_BinInfo();
            }

            bi->m_nBinNo    =   clGexDbQuery.value(nIndex).toInt(); nIndex++;
            bi->m_strBinName=   clGexDbQuery.value(nIndex).toString(); nIndex++;
            QString cat     =   clGexDbQuery.value(nIndex).toString(); nIndex++;
            QChar c='?';
            if (cat.size()>0) c=cat.at(0);
            bi->m_cBinCat   =   c;
            bi->m_nBinCount +=  clGexDbQuery.value(nIndex).toInt();

            if(!mapBinsPerKey.contains(lKey))
            {
                GexDbPlugin_BinList bl;
                //bl.setAutoDelete(false);
                bl.append(bi);
                mapBinsPerKey[lKey] = bl;
            }
            if(bAddNewBinInfo)
                mapBinsPerKey[lKey].append(bi);
        }
    }


    return "ok";
}
