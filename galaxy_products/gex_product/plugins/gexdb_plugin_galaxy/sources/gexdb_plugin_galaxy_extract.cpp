// gexdb_plugin_galaxy_extraction.cpp: implementation of the data extraction functions of GexDbPlugin_Galaxy class.
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
#include "gexdb_plugin_galaxy_splitlot_info.h"
#include "test_filter.h"
#include "gexdb_plugin_base.h"
#include "gex_constants.h"
#include "gex_shared.h"
#include "abstract_query_progress.h"
#include "xychart_data.h"

// Standard includes
#include <math.h>

// Qt includes
#include <QSqlRecord>
#include <QStringList>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlDriver>
#include <QSqlResult>
#include <QSqlError>
#include <QDateTime>
#include <QMap>
#include <QHash>
#include <QDir>
#include <QApplication>

// Galaxy modules includes
#include <stdf.h>
#include <stdfparse.h>
#include <gqtl_sysutils.h>
#include <gqtl_utils.h>
#include <gqtl_log.h>

////////////////////////////////////////////////////////////////////////////////////
// GexDbPlugin_Galaxy class: database plugin class for GEXDB database type
////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Init variables used for data extraction
///////////////////////////////////////////////////////////
void GexDbPlugin_Galaxy::Init_Extraction()
{
    m_pRunInfoArray = NULL;
    m_uiSplitlotNbRuns = 0;
#if PROFILERON
    m_bProfilerON = true;
#else
    m_bProfilerON = false;
#endif
}


bool GexDbPlugin_Galaxy::CreateTestlist(
        const GexDbPlugin_Galaxy_SplitlotInfo & clSplitlotInfo,
        GexDbPlugin_Galaxy_TestFilter & clTestFilter,
        const GexDbPlugin_Filter & cFilters)
{
    QString	strQuery, strCondition, strFieldSpec;
    if (!m_pclDatabaseConnector)
        return false;

    GexDbPlugin_Query		clGexDbQuery_Tests(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    GexDbPlugin_TestInfo	*lTestInfo=0;
    unsigned int			uiFlags, uiTestNb;

    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Create Test list for splitlot %1, tests %2 filtering on %3")
          .arg(clSplitlotInfo.m_lSplitlotID)
          .arg(clTestFilter.getCleanTestNblist())
          .arg(cFilters.strlQueryFilters.join(","))
          .toLatin1().data() );

    if(m_bCustomerDebugMode)
    {
        m_clExtractionPerf.Start();
    }

    // First clear the list
    m_clTestList.ClearData(true);

    QMap<QString, GexDbPlugin_Mapping_Field> conditions_filters;

    if(m_pmapFields_GexToRemote == NULL)
    {
        GSLOG(SYSLOG_SEV_ERROR, "m_pmapFields_GexToRemote NULL");
        return false;
    }

    foreach(const QString &s, cFilters.strlQueryFilters)
    {
        if (s.isEmpty())
            continue;
        QStringList	strlElements = s.split("=");
        if (strlElements.size()!=2)
            continue;
        QString		strField = strlElements[0];

        GexDbPlugin_Mapping_FieldMap::Iterator itMapping = m_pmapFields_GexToRemote->find(strField);
        if(itMapping == m_pmapFields_GexToRemote->end())
            continue;

        GexDbPlugin_Mapping_Field mf=itMapping.value();
        if (!mf.isTestCondition())
            continue;

        conditions_filters.insert(s, mf);
        GSLOG(SYSLOG_SEV_NOTICE, QString("Create test list : test condition filter found on : %1")
              .arg( s).toLatin1().constData() );
    }

    // If test list is empty and no conditions filters, do not create anything
    if(clTestFilter.isEmpty()&&conditions_filters.isEmpty())
        return true;

    // Query PTEST_INFO for specified Splitlot
    Query_Empty();
    strFieldSpec = "Field|";
    strFieldSpec += m_strTablePrefix;
    m_strlQuery_Fields.append(strFieldSpec + "ptest_info.ptest_info_id");
    m_strlQuery_Fields.append(strFieldSpec + "ptest_info.tnum");
    m_strlQuery_Fields.append(strFieldSpec + "ptest_info.tname");
    m_strlQuery_Fields.append(strFieldSpec + "ptest_info.units");
    strCondition = "Expression|" + m_strTablePrefix;
    strCondition += "ptest_info.test_flags|(";
    strCondition += m_strTablePrefix + "ptest_info.test_flags)";
    m_strlQuery_Fields.append(strCondition);
    strCondition = "Expression|" + m_strTablePrefix;
    strCondition += "ptest_info.ll|(";
    strCondition += m_strTablePrefix + "ptest_info.ll+0)";
    m_strlQuery_Fields.append(strCondition);
    strCondition = "Expression|" + m_strTablePrefix;
    strCondition += "ptest_info.spec_ll|(";
    strCondition += m_strTablePrefix + "ptest_info.spec_ll+0)";
    m_strlQuery_Fields.append(strCondition);
    strCondition = "Expression|" + m_strTablePrefix;
    strCondition += "ptest_info.hl|(";
    strCondition += m_strTablePrefix + "ptest_info.hl+0)";
    m_strlQuery_Fields.append(strCondition);
    strCondition = "Expression|" + m_strTablePrefix;
    strCondition += "ptest_info.spec_hl|(";
    strCondition += m_strTablePrefix + "ptest_info.spec_hl+0)";
    m_strlQuery_Fields.append(strCondition);
    m_strlQuery_Fields.append(strFieldSpec + "ptest_info.testseq");
    m_strlQuery_Fields.append(strFieldSpec + "ptest_info.res_scal");
    m_strlQuery_Fields.append(strFieldSpec + "ptest_info.ll_scal");
    m_strlQuery_Fields.append(strFieldSpec + "ptest_info.hl_scal");
    strCondition = m_strTablePrefix + "ptest_info.splitlot_id|Numeric|" + QString::number(clSplitlotInfo.m_lSplitlotID);
    m_strlQuery_ValueConditions.append(strCondition);

    // Narrow on tests specified in testlist (testlist format is "10,20-30,40-50,65")
    if(!clTestFilter.extractPatTests() && !clTestFilter.extractAllTests())
        Query_AddTestlistCondition(clTestFilter, 'P', eOnTestInfo);

    // add sort instruction
    //m_strlQuery_OrderFields.append(m_strTablePrefix + "ptest_info.testseq");
    //m_strlQuery_OrderFields.append(m_strTablePrefix + "ptest_info.tnum");

    bool b=false;
    if (!conditions_filters.isEmpty())
    {
        m_strlQuery_LinkConditions.append(m_strTablePrefix + "ptest_info.splitlot_id|" + m_strTablePrefix + "test_conditions.splitlot_id");
        m_strlQuery_LinkConditions.append(m_strTablePrefix + "ptest_info.ptest_info_id|" + m_strTablePrefix + "test_conditions.test_info_id");

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

        m_strlQuery_ValueConditions.append( m_strTablePrefix + "test_conditions.test_type|String|P" );
        b=Query_BuildSqlString_UsingJoins(strQuery, false);
    }
    else
        b=Query_BuildSqlString(strQuery, false);

    if (!b)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Create test list : Build sql string failed");
        return false;
    }

    clGexDbQuery_Tests.setForwardOnly(true);
    if(!clGexDbQuery_Tests.Execute(strQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery_Tests.lastError().text().toLatin1().constData());
        return false;
    }

    // Add all retrieved tests to testlist
    GS::QtLib::Range gexRange(clTestFilter.getCleanTestNblist());
    if(clTestFilter.getCleanTestNblist().isEmpty())
        // Clear the gexRange, as by default if constructed with an empty string,
        // it creates a range including all values
        gexRange.Clear();

    // Extract all multi limits once for this splitlot
    QMap<int, QList<QVariantMap> > lPtestMultiLimits;
    if (!ExtractMultiLimits(lPtestMultiLimits, clSplitlotInfo.m_lSplitlotID, m_strTablePrefix))
        return false;

    while(clGexDbQuery_Tests.Next())
    {
        uiTestNb = clGexDbQuery_Tests.value(1).toUInt();
        uiFlags = clGexDbQuery_Tests.value(4).toUInt();

        // Check if current parameter should be extracted
        if(clTestFilter.extractAllTests() ||
                gexRange.Contains(uiTestNb) ||
                (clTestFilter.extractPatTests() && (uiFlags & FLAG_TESTINFO_PAT_ACTIVE)))
        {
            try
            {
                lTestInfo = new GexDbPlugin_TestInfo;
            }
            catch(const std::bad_alloc& e)
            {
                GSLOG(SYSLOG_SEV_CRITICAL, "Memory allocation exception caught");
                return false;
            }
            catch(...)
            {
                GSLOG(SYSLOG_SEV_CRITICAL, "Exception caught ");
                return false;
            }
            // Add this test
            lTestInfo->Reset();
            lTestInfo->m_cTestType							= 'P';
            lTestInfo->m_lTestID							= clGexDbQuery_Tests.value(0).toLongLong();
            lTestInfo->m_uiTestNumber						= uiTestNb;
            lTestInfo->m_strTestName						= clGexDbQuery_Tests.value(2).toString();
            lTestInfo->m_pTestInfo_PTR						= new GexDbPlugin_TestInfo_PTR;
            lTestInfo->m_pTestInfo_PTR->m_strTestUnit		= clGexDbQuery_Tests.value(3).toString();
            if(!(uiFlags & FLAG_TESTINFO_LL_STRICT))
                lTestInfo->m_pTestInfo_PTR->m_uiStaticFlags |= 0x40;
            if(!(uiFlags & FLAG_TESTINFO_HL_STRICT))
                lTestInfo->m_pTestInfo_PTR->m_uiStaticFlags |= 0x80;
            if(!clGexDbQuery_Tests.isNull(5))
            {
                lTestInfo->m_pTestInfo_PTR->m_bHasLL		= true;
                lTestInfo->m_pTestInfo_PTR->m_fLL			= clGexDbQuery_Tests.value(5).toDouble();
                if(!clGexDbQuery_Tests.isNull(6))
                {
                    lTestInfo->m_pTestInfo_PTR->m_bHasSpecLL	= true;
                    lTestInfo->m_pTestInfo_PTR->m_fSpecLL		= clGexDbQuery_Tests.value(6).toDouble();
                }
            }
            if(!clGexDbQuery_Tests.isNull(7))
            {
                lTestInfo->m_pTestInfo_PTR->m_bHasHL		= true;
                lTestInfo->m_pTestInfo_PTR->m_fHL			= clGexDbQuery_Tests.value(7).toDouble();
                if(!clGexDbQuery_Tests.isNull(8))
                {
                    lTestInfo->m_pTestInfo_PTR->m_bHasSpecHL	= true;
                    lTestInfo->m_pTestInfo_PTR->m_fSpecHL		= clGexDbQuery_Tests.value(8).toDouble();
                }
            }
            lTestInfo->m_uiTestSeq							= clGexDbQuery_Tests.value(9).toUInt();
            if(!clGexDbQuery_Tests.isNull(10))
                lTestInfo->m_pTestInfo_PTR->m_nResScal		= clGexDbQuery_Tests.value(10).toInt();
            if(!clGexDbQuery_Tests.isNull(11))
                lTestInfo->m_pTestInfo_PTR->m_nLlScal		= clGexDbQuery_Tests.value(11).toInt();
            if(!clGexDbQuery_Tests.isNull(12))
                lTestInfo->m_pTestInfo_PTR->m_nHlScal		= clGexDbQuery_Tests.value(12).toInt();

            // check if multi limits map has some records for this test
            if (lPtestMultiLimits.contains(lTestInfo->m_lTestID))
            {
                // Add TNAM and TNUM to each variant map
                QList<QVariantMap> lMultiLimits = lPtestMultiLimits.value(lTestInfo->m_lTestID);
                for(int lIt=0; lIt<lMultiLimits.size(); ++lIt)
                {
                    lMultiLimits[lIt].insert("TNUM", QVariant(lTestInfo->m_uiTestNumber));
                    lMultiLimits[lIt].insert("TNAME", QVariant(lTestInfo->m_strTestName));
                }

                lTestInfo->m_pTestInfo_PTR->mMultiLimits = lMultiLimits;
            }

            m_clTestList.Insert(lTestInfo);
        }
    }

    // Don't retrieve MPTests and FTests for E-Test testing stage
    if(m_strTestingStage == GEXDB_PLUGIN_GALAXY_ETEST)
        return true;

    // Query MPTEST_INFO for specified Splitlot
    Query_Empty();
    strFieldSpec = "Field|";
    strFieldSpec += m_strTablePrefix;
    m_strlQuery_Fields.append(strFieldSpec + "mptest_info.mptest_info_id");
    m_strlQuery_Fields.append(strFieldSpec + "mptest_info.tnum");
    m_strlQuery_Fields.append(strFieldSpec + "mptest_info.tname");
    m_strlQuery_Fields.append(strFieldSpec + "mptest_info.tpin_arrayindex");
    m_strlQuery_Fields.append(strFieldSpec + "mptest_info.units");
    strCondition = "Expression|" + m_strTablePrefix;
    strCondition += "mptest_info.test_flags|(";
    strCondition += m_strTablePrefix + "mptest_info.test_flags)";
    m_strlQuery_Fields.append(strCondition);
    strCondition = "Expression|" + m_strTablePrefix;
    strCondition += "mptest_info.ll|(";
    strCondition += m_strTablePrefix + "mptest_info.ll+0)";
    m_strlQuery_Fields.append(strCondition);
    strCondition = "Expression|" + m_strTablePrefix;
    strCondition += "mptest_info.spec_ll|(";
    strCondition += m_strTablePrefix + "mptest_info.spec_ll+0)";
    m_strlQuery_Fields.append(strCondition);
    strCondition = "Expression|" + m_strTablePrefix;
    strCondition += "mptest_info.hl|(";
    strCondition += m_strTablePrefix + "mptest_info.hl+0)";
    m_strlQuery_Fields.append(strCondition);
    strCondition = "Expression|" + m_strTablePrefix;
    strCondition += "mptest_info.spec_hl|(";
    strCondition += m_strTablePrefix + "mptest_info.spec_hl+0)";
    m_strlQuery_Fields.append(strCondition);
    m_strlQuery_Fields.append(strFieldSpec + "mptest_info.testseq");
    m_strlQuery_Fields.append(strFieldSpec + "mptest_info.res_scal");
    m_strlQuery_Fields.append(strFieldSpec + "mptest_info.ll_scal");
    m_strlQuery_Fields.append(strFieldSpec + "mptest_info.hl_scal");
    strCondition = m_strTablePrefix + "mptest_info.splitlot_id|Numeric|" + QString::number(clSplitlotInfo.m_lSplitlotID);
    m_strlQuery_ValueConditions.append(strCondition);

    // Narrow on tests specified in testlist (testlist format is "10,20-30,40-50,65")
    if(!clTestFilter.extractPatTests() && !clTestFilter.extractAllTests())
        Query_AddTestlistCondition(clTestFilter, 'M', eOnTestInfo);

    // add sort instruction
    //m_strlQuery_OrderFields.append(m_strTablePrefix + "mptest_info.testseq");
    //m_strlQuery_OrderFields.append(m_strTablePrefix + "mptest_info.tnum");

    if (!conditions_filters.isEmpty())
    {
        m_strlQuery_LinkConditions.append(m_strTablePrefix + "mptest_info.splitlot_id|" + m_strTablePrefix + "test_conditions.splitlot_id");
        m_strlQuery_LinkConditions.append(m_strTablePrefix + "mptest_info.mptest_info_id|" + m_strTablePrefix + "test_conditions.test_info_id");

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

        m_strlQuery_ValueConditions.append( m_strTablePrefix + "test_conditions.test_type|String|M" );
        b=Query_BuildSqlString_UsingJoins(strQuery, false);
    }
    else
        b=Query_BuildSqlString(strQuery, false);

    clGexDbQuery_Tests.setForwardOnly(true);
    if(!clGexDbQuery_Tests.Execute(strQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery_Tests.lastError().text().toLatin1().constData());
        return false;
    }

    //qDebug("GexDbPlugin_Galaxy::CreateTestlist: %d mptest_info retrieved", clGexDbQuery_Tests.size());
    // Add all retrieved tests to testlist
    while(clGexDbQuery_Tests.Next())
    {
        uiTestNb = clGexDbQuery_Tests.value(1).toUInt();
        uiFlags = clGexDbQuery_Tests.value(5).toUInt();

        // If PAT tests to be extracted, and test not in list, and PAT flag not set, do not add this test
        if(clTestFilter.extractPatTests() && !gexRange.Contains(uiTestNb) && ((uiFlags & FLAG_TESTINFO_PAT_ACTIVE)==0))
            continue;

        // Add this test
        try
        {
            lTestInfo = new GexDbPlugin_TestInfo;
        }
        catch(const std::bad_alloc& e)
        {
            GSLOG(SYSLOG_SEV_CRITICAL, "Memory allocation exception caught");
            return false;
        }
        catch(...)
        {
            GSLOG(SYSLOG_SEV_CRITICAL, "Exception caught ");
            return false;
        }

        lTestInfo->Reset();
        lTestInfo->m_cTestType							= 'M';
        lTestInfo->m_lTestID							= clGexDbQuery_Tests.value(0).toLongLong();
        lTestInfo->m_uiTestNumber						= uiTestNb;
        lTestInfo->m_strTestName						= clGexDbQuery_Tests.value(2).toString();
        lTestInfo->m_pTestInfo_MPR						= new GexDbPlugin_TestInfo_MPR;
        lTestInfo->m_pTestInfo_MPR->m_nTpin_ArrayIndex	= clGexDbQuery_Tests.value(3).toInt();
        lTestInfo->m_pTestInfo_MPR->m_strTestUnit		= clGexDbQuery_Tests.value(4).toString();
        if(!(uiFlags & FLAG_TESTINFO_LL_STRICT))
            lTestInfo->m_pTestInfo_MPR->m_uiStaticFlags |= 0x40;
        if(!(uiFlags & FLAG_TESTINFO_HL_STRICT))
            lTestInfo->m_pTestInfo_MPR->m_uiStaticFlags |= 0x80;
        if(!clGexDbQuery_Tests.isNull(6))
        {
            lTestInfo->m_pTestInfo_MPR->m_bHasLL		= true;
            lTestInfo->m_pTestInfo_MPR->m_fLL			= clGexDbQuery_Tests.value(6).toDouble();
            if(!clGexDbQuery_Tests.isNull(7))
            {
                lTestInfo->m_pTestInfo_MPR->m_bHasSpecLL	= true;
                lTestInfo->m_pTestInfo_MPR->m_fSpecLL		= clGexDbQuery_Tests.value(7).toDouble();
            }
        }
        if(!clGexDbQuery_Tests.isNull(8))
        {
            lTestInfo->m_pTestInfo_MPR->m_bHasHL		= true;
            lTestInfo->m_pTestInfo_MPR->m_fHL			= clGexDbQuery_Tests.value(8).toDouble();
            if(!clGexDbQuery_Tests.isNull(9))
            {
                lTestInfo->m_pTestInfo_MPR->m_bHasSpecHL	= true;
                lTestInfo->m_pTestInfo_MPR->m_fSpecHL		= clGexDbQuery_Tests.value(9).toDouble();
            }
        }
        lTestInfo->m_uiTestSeq							= clGexDbQuery_Tests.value(10).toUInt();
        if(!clGexDbQuery_Tests.isNull(11))
            lTestInfo->m_pTestInfo_MPR->m_nResScal		= clGexDbQuery_Tests.value(11).toInt();
        if(!clGexDbQuery_Tests.isNull(12))
            lTestInfo->m_pTestInfo_MPR->m_nLlScal		= clGexDbQuery_Tests.value(12).toInt();
        if(!clGexDbQuery_Tests.isNull(13))
            lTestInfo->m_pTestInfo_MPR->m_nHlScal		= clGexDbQuery_Tests.value(13).toInt();

        // GCORE-555: static (multi) limits not supported for MPTEST

        m_clTestList.Insert(lTestInfo);
    }

    // Query FTEST_INFO for specified Splitlot
    Query_Empty();
    strFieldSpec = "Field|";
    strFieldSpec += m_strTablePrefix;
    m_strlQuery_Fields.append(strFieldSpec + "ftest_info.ftest_info_id");
    m_strlQuery_Fields.append(strFieldSpec + "ftest_info.tnum");
    m_strlQuery_Fields.append(strFieldSpec + "ftest_info.tname");
    m_strlQuery_Fields.append(strFieldSpec + "ftest_info.testseq");
    strCondition = m_strTablePrefix + "ftest_info.splitlot_id|Numeric|" + QString::number(clSplitlotInfo.m_lSplitlotID);
    m_strlQuery_ValueConditions.append(strCondition);

    // Narrow on tests specified in testlist (testlist format is "10,20-30,40-50,65")
    if(!clTestFilter.extractPatTests() && !clTestFilter.extractAllTests())
        Query_AddTestlistCondition(clTestFilter, 'F', eOnTestInfo);

    // add sort instruction
    //m_strlQuery_OrderFields.append(m_strTablePrefix + "mptest_info.testseq");
    //m_strlQuery_OrderFields.append(m_strTablePrefix + "mptest_info.tnum");

    if (!conditions_filters.isEmpty())
    {
        m_strlQuery_LinkConditions.append(m_strTablePrefix + "ftest_info.splitlot_id|" + m_strTablePrefix + "test_conditions.splitlot_id");
        m_strlQuery_LinkConditions.append(m_strTablePrefix + "ftest_info.ftest_info_id|" + m_strTablePrefix + "test_conditions.test_info_id");

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

        m_strlQuery_ValueConditions.append( m_strTablePrefix + "test_conditions.test_type|String|F" );
        b=Query_BuildSqlString_UsingJoins(strQuery, false);
    }
    else
        b=Query_BuildSqlString(strQuery, false);

    clGexDbQuery_Tests.setForwardOnly(true);
    if(!clGexDbQuery_Tests.Execute(strQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery_Tests.lastError().text().toLatin1().constData());
        return false;
    }

    GSLOG(SYSLOG_SEV_DEBUG, QString("%1 ftest_info retrieved").arg( clGexDbQuery_Tests.size()).toLatin1().constData());
    // Add all retrieved tests to testlist
    while(clGexDbQuery_Tests.Next())
    {
        uiTestNb = clGexDbQuery_Tests.value(1).toUInt();
        uiFlags = 0;

        // If PAT tests to be extracted, and test not in list, and PAT flag not set, do not add this test
        if(clTestFilter.extractPatTests() && !gexRange.Contains(uiTestNb) && ((uiFlags & FLAG_TESTINFO_PAT_ACTIVE)==0))
            continue;

        // Add this test
        try
        {
            lTestInfo = new GexDbPlugin_TestInfo;
        }
        catch(const std::bad_alloc& e)
        {
            GSLOG(SYSLOG_SEV_ERROR, "Memory allocation exception caught ");
            return false;
        }
        catch(...)
        {
            GSLOG(SYSLOG_SEV_ERROR, "Exception caught");
            return false;
        }

        lTestInfo->Reset();
        lTestInfo->m_cTestType						= 'F';
        lTestInfo->m_lTestID						= clGexDbQuery_Tests.value(0).toLongLong();
        lTestInfo->m_uiTestNumber					= uiTestNb;
        lTestInfo->m_strTestName					= clGexDbQuery_Tests.value(2).toString();
        lTestInfo->m_pTestInfo_FTR					= new GexDbPlugin_TestInfo_FTR;
        lTestInfo->m_uiTestSeq						= clGexDbQuery_Tests.value(3).toUInt();

        m_clTestList.Insert(lTestInfo);
    }

    // Debug mode??
    if(m_bCustomerDebugMode)
    {
        // Stop partial timer
        m_clExtractionPerf.Stop(clGexDbQuery_Tests.m_ulRetrievedRows_Cumul, clGexDbQuery_Tests.m_fTimer_DbQuery_Cumul, clGexDbQuery_Tests.m_fTimer_DbIteration_Cumul);

#ifndef NO_DUMP_PERF
        // Dump testlist
        m_clTestList.GestTestListString(strQuery);
        strQuery = "TestList DUMP:\n" + strQuery;
        WriteDebugMessageFile(strQuery);
        // Write partial performance
        WritePartialPerformance((char*)"CreateTestlist()");
#endif
    }

    return true;
}

//////////////////////////////////////////////////////////////////////
// Return TestID for soecified test number, or -1 if test not found
//////////////////////////////////////////////////////////////////////
qint64 GexDbPlugin_Galaxy::GetTestID(unsigned int uiTestNum)
{
    GexDbPlugin_TestInfo *pTestInfo = m_clTestList.FindTestByNb(uiTestNum);
    if(pTestInfo)
        return pTestInfo->m_lTestID;

    return -1;
}

bool
GexDbPlugin_Galaxy::
WriteStaticTestInfo(GQTL_STDF::StdfParse& lStdfParse,
                    const GexDbPlugin_Galaxy_SplitlotInfo& /*clSplitlotInfo*/)
{
    GQTL_STDF::Stdf_PTR_V4	lPTR;
    GQTL_STDF::Stdf_MPR_V4	clMPR;
    GQTL_STDF::Stdf_FTR_V4	clFTR;
    BYTE			bOptFlag;
    int				nMPR_MaxArrayIndex;

    GexDbPlugin_TestInfoContainer				*pContainer;
    GexDbPlugin_TestInfo						*pTestInfo, *pMPR_RefTest;
    QList<GexDbPlugin_TestInfo*>				list_pMPRTestInfos;

    // Check if some tests in list
    if(m_clTestList.m_uiNbTests == 0)
        return true;

    // Write a dummy PIR
    WritePIR(lStdfParse, 255);

    // Write all PTR/MPR/FTR s
    pContainer = m_clTestList.m_pFirstTest;
    while(pContainer)
    {
        pTestInfo = pContainer->m_pTestInfo;
        if (!pTestInfo)
        {
            GSLOG(SYSLOG_SEV_ERROR, "pTestInfo NULL");
            break;
        }

        if(pTestInfo->m_cTestType == 'P')
        {
            // Set PTR fields
            bOptFlag = 0;
            if(!pTestInfo->m_pTestInfo_PTR->m_bHasLL)
                bOptFlag |= 0x40;
            if(!pTestInfo->m_pTestInfo_PTR->m_bHasHL)
                bOptFlag |= 0x80;
            lPTR.SetTEST_NUM(pTestInfo->m_uiTestNumber);
            lPTR.SetHEAD_NUM(1);
            lPTR.SetSITE_NUM(255);
            lPTR.SetTEST_FLG(0x12);		// Test not executed
            lPTR.SetPARM_FLG(pTestInfo->m_pTestInfo_PTR->m_uiStaticFlags);
            lPTR.SetRESULT(0.0F);
            lPTR.SetTEST_TXT(pTestInfo->m_strTestName);
            lPTR.SetALARM_ID("");
            lPTR.SetOPT_FLAG(bOptFlag);
            if(pTestInfo->m_pTestInfo_PTR->m_nResScal == GEXDB_INVALID_SCALING_FACTOR)
                lPTR.SetRES_SCAL((pTestInfo->m_pTestInfo_PTR->m_strTestUnit == "%") ? 2:0);
            else
                lPTR.SetRES_SCAL(pTestInfo->m_pTestInfo_PTR->m_nResScal);
            if(pTestInfo->m_pTestInfo_PTR->m_nLlScal == GEXDB_INVALID_SCALING_FACTOR)
                lPTR.SetLLM_SCAL((pTestInfo->m_pTestInfo_PTR->m_strTestUnit == "%") ? 2:0);
            else
                lPTR.SetLLM_SCAL(pTestInfo->m_pTestInfo_PTR->m_nLlScal);
            if(pTestInfo->m_pTestInfo_PTR->m_nHlScal == GEXDB_INVALID_SCALING_FACTOR)
                lPTR.SetHLM_SCAL((pTestInfo->m_pTestInfo_PTR->m_strTestUnit == "%") ? 2:0);
            else
                lPTR.SetHLM_SCAL(pTestInfo->m_pTestInfo_PTR->m_nHlScal);
            lPTR.SetLO_LIMIT(pTestInfo->m_pTestInfo_PTR->m_fLL);
            lPTR.SetHI_LIMIT(pTestInfo->m_pTestInfo_PTR->m_fHL);
            lPTR.SetUNITS(pTestInfo->m_pTestInfo_PTR->m_strTestUnit);
            lPTR.SetC_RESFMT("");
            lPTR.SetC_LLMFMT("");
            lPTR.SetC_HLMFMT("");
            lPTR.SetLO_SPEC(pTestInfo->m_pTestInfo_PTR->m_fSpecLL);
            lPTR.SetHI_SPEC(pTestInfo->m_pTestInfo_PTR->m_fSpecHL);

            // Write record
            lStdfParse.WriteRecord(&lPTR);
            lPTR.Reset();

            // Write multi limits in DTR if any
            for(int lIt = 0; lIt < pTestInfo->m_pTestInfo_PTR->mMultiLimits.size(); ++lIt)
            {
                GQTL_STDF::Stdf_DTR_V4 lDTR;
                lDTR.SetTEXT_DAT(QJsonObject::fromVariantMap(
                                     pTestInfo->m_pTestInfo_PTR->mMultiLimits.at(lIt)));
                // Write record
                lStdfParse.WriteRecord(&lDTR);
            }

            pTestInfo->m_bStaticInfoWrittenToStdf = true;
            pContainer = pContainer->m_pNextTest;
        }
        else if(pTestInfo->m_cTestType == 'M')
        {
            pMPR_RefTest = pTestInfo;

            // Set test_flags
            bOptFlag = 0;
            if(!pTestInfo->m_pTestInfo_MPR->m_bHasLL)
                bOptFlag |= 0x40;
            if(!pTestInfo->m_pTestInfo_MPR->m_bHasHL)
                bOptFlag |= 0x80;

            // Save all MPR's with same test nb and test name
            //pMPR_Tests.append(pTestInfo);
            list_pMPRTestInfos.append(pTestInfo);

            nMPR_MaxArrayIndex = pTestInfo->m_pTestInfo_MPR->m_nTpin_ArrayIndex;
            pTestInfo->m_bStaticInfoWrittenToStdf = true;
            pContainer = pContainer->m_pNextTest;
            while(pContainer)
            {
                pTestInfo = pContainer->m_pTestInfo;
                if((pTestInfo->m_cTestType != 'M') || (pTestInfo->m_uiTestNumber != pMPR_RefTest->m_uiTestNumber) || (pTestInfo->m_strTestName != pMPR_RefTest->m_strTestName))
                    break;
                //pMPR_Tests.append(pTestInfo);
                list_pMPRTestInfos.append(pTestInfo);

                nMPR_MaxArrayIndex = qMax(nMPR_MaxArrayIndex, pTestInfo->m_pTestInfo_MPR->m_nTpin_ArrayIndex);
                pTestInfo->m_bStaticInfoWrittenToStdf = true;
                pContainer = pContainer->m_pNextTest;
            }

            // Set MPR fields
            clMPR.SetTEST_NUM(pMPR_RefTest->m_uiTestNumber);
            clMPR.SetHEAD_NUM(1);
            clMPR.SetSITE_NUM(255);
            clMPR.SetTEST_FLG(0x10);		// Test not executed
            clMPR.SetPARM_FLG(pMPR_RefTest->m_pTestInfo_MPR->m_uiStaticFlags);
            clMPR.SetRTN_ICNT(nMPR_MaxArrayIndex+1);
            clMPR.SetRSLT_CNT(nMPR_MaxArrayIndex+1);

            foreach(pTestInfo, list_pMPRTestInfos)
                //for (pTestInfo=pMPR_Tests.first(); pTestInfo; pTestInfo=pMPR_Tests.next())
            {
                clMPR.SetRTN_STAT(pTestInfo->m_pTestInfo_MPR->m_nTpin_ArrayIndex, 0);
                clMPR.SetRTN_RSLT(pTestInfo->m_pTestInfo_MPR->m_nTpin_ArrayIndex, 0.0F);
                clMPR.SetRTN_INDX(pTestInfo->m_pTestInfo_MPR->m_nTpin_ArrayIndex, 0);
            }
            clMPR.SetTEST_TXT(pMPR_RefTest->m_strTestName);
            clMPR.SetALARM_ID("");
            clMPR.SetOPT_FLAG(bOptFlag);
            if(pMPR_RefTest->m_pTestInfo_MPR->m_nResScal == GEXDB_INVALID_SCALING_FACTOR)
                clMPR.SetRES_SCAL((pMPR_RefTest->m_pTestInfo_MPR->m_strTestUnit == "%") ? 2:0);
            else
                clMPR.SetRES_SCAL(pMPR_RefTest->m_pTestInfo_MPR->m_nResScal);
            if(pMPR_RefTest->m_pTestInfo_MPR->m_nLlScal == GEXDB_INVALID_SCALING_FACTOR)
                clMPR.SetLLM_SCAL((pMPR_RefTest->m_pTestInfo_MPR->m_strTestUnit == "%") ? 2:0);
            else
                clMPR.SetLLM_SCAL(pMPR_RefTest->m_pTestInfo_MPR->m_nLlScal);
            if(pMPR_RefTest->m_pTestInfo_MPR->m_nHlScal == GEXDB_INVALID_SCALING_FACTOR)
                clMPR.SetHLM_SCAL((pMPR_RefTest->m_pTestInfo_MPR->m_strTestUnit == "%") ? 2:0);
            else
                clMPR.SetHLM_SCAL(pMPR_RefTest->m_pTestInfo_MPR->m_nHlScal);
            clMPR.SetLO_LIMIT(pMPR_RefTest->m_pTestInfo_MPR->m_fLL);
            clMPR.SetHI_LIMIT(pMPR_RefTest->m_pTestInfo_MPR->m_fHL);
            clMPR.SetSTART_IN(0.0F);
            clMPR.SetINCR_IN(0.0F);
            clMPR.SetUNITS(pMPR_RefTest->m_pTestInfo_MPR->m_strTestUnit);
            clMPR.SetUNITS_IN("");
            clMPR.SetC_RESFMT("");
            clMPR.SetC_LLMFMT("");
            clMPR.SetC_HLMFMT("");
            clMPR.SetLO_SPEC(pMPR_RefTest->m_pTestInfo_MPR->m_fSpecLL);
            clMPR.SetHI_SPEC(pMPR_RefTest->m_pTestInfo_MPR->m_fSpecHL);

            // Write record
            lStdfParse.WriteRecord(&clMPR);

            list_pMPRTestInfos.clear();
            //pMPR_Tests.clear();
            clMPR.Reset();
        }
        else if(pTestInfo->m_cTestType == 'F')
        {
            // Set FTR fields
            clFTR.SetTEST_NUM(pTestInfo->m_uiTestNumber);
            clFTR.SetHEAD_NUM(1);
            clFTR.SetSITE_NUM(255);
            clFTR.SetTEST_FLG(0x10);		// Test not executed
            clFTR.SetOPT_FLAG((BYTE)0xff);	// All corresponding fields invalid
            clFTR.SetCYCL_CNT(0);
            clFTR.SetREL_VADR(0);
            clFTR.SetREPT_CNT(0);
            clFTR.SetNUM_FAIL(0);
            clFTR.SetXFAIL_AD(0);
            clFTR.SetYFAIL_AD(0);
            clFTR.SetVECT_OFF(0);
            clFTR.SetRTN_ICNT(0);
            clFTR.SetPGM_ICNT(0);
            clFTR.SetFAIL_PIN();
            clFTR.SetVECT_NAM("");
            clFTR.SetTIME_SET("");
            clFTR.SetOP_CODE("");
            clFTR.SetTEST_TXT(pTestInfo->m_strTestName);

            // Write record
            lStdfParse.WriteRecord(&clFTR);
            clFTR.Reset();

            pTestInfo->m_bStaticInfoWrittenToStdf = true;
            pContainer = pContainer->m_pNextTest;
        }
        else
            pContainer = pContainer->m_pNextTest;
    }

    // Write a dummy PRR
    GexDbPlugin_RunInfo clPart;
    clPart.m_nSiteNb = 255;
    clPart.m_nSoftBin = clPart.m_nHardBin = 65535;
    clPart.m_nX = clPart.m_nY = -32768;
    WritePRR(lStdfParse, &clPart);

    return true;
}


QString GexDbPlugin_Galaxy::WriteTestResults(
        GQTL_STDF::StdfParse & clStdfParse,
        const GexDbPlugin_Galaxy_SplitlotInfo & clSplitlotInfo,
        GexDbPlugin_Galaxy_TestFilter & clTestFilter,
        QMap< QPair<int, int>, QMap<QString, QVariant> > */*xys=NULL*/,
        bool autowrite_parts_with_same_runid/*=false*/,
        const int &sampling
        )
{
    QString						strCondition, strFieldSpec, strDbField, strDbTable, strTableAlias, strMessage;
    QString						strQuery_PTR, strQuery_MPR, strQuery_FTR;
    QString						strQuery_PTR_NoIndex, strQuery_MPR_NoIndex, strQuery_FTR_NoIndex;

    if (!m_pclDatabaseConnector)
        return "error : no DatabaseConnector";
    GexDbPlugin_Query			clGexDbQuery_PTR(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    GexDbPlugin_Query			clGexDbQuery_MPR(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    GexDbPlugin_Query			clGexDbQuery_FTR(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    int							nCurrentItem=-1, nRunID=0, nRunID_PTR=0, nRunID_MPR=0, nRunID_FTR=0;
    GexDbPlugin_RunInfo			*pCurrentPart=NULL;
    GexDbPlugin_TestInfo		*pTestInfo=NULL;
    unsigned int				uiNbRuns=0, uiNbTestResults=0, uiFlags=0, uiNbRuns_Ignored=0;
    bool						bStatus_PTR, bStatus_MPR, bStatus_FTR, bSkipIndexDialog=false;

    // only used in gui exec but anyway
    QMessageBox::StandardButton	buttonNoIndexAnswer;

    int							nTpin_PmrIndex;
    unsigned int				uiDynamicFlags;
    float						fResult=0.f;

    QString r=QString("Write Test Results for splitlot %1, %2 runs on %3 tests at sampling %4...")
            .arg(clSplitlotInfo.m_lSplitlotID).arg(m_nRunInfoArray_NbItems)
            .arg(m_clTestList.m_uiNbTests).arg(sampling);
    GSLOG(SYSLOG_SEV_INFORMATIONAL, r.toLatin1().data());
    if (mQueryProgress)
    {
        mQueryProgress->AddLog(r);
        QCoreApplication::processEvents();
    }

    if(m_bCustomerDebugMode)
    {
        m_clExtractionPerf.Start();
    }

    // A162
    bool ExtractPartsIfNoResults=false;
    if (m_clOptions.mOptions.value( GS::DbPluginGalaxy::RdbOptions::mOptionExtractPartsIfNoResults).canConvert(QVariant::Bool))
        ExtractPartsIfNoResults=m_clOptions.mOptions.value( GS::DbPluginGalaxy::RdbOptions::mOptionExtractPartsIfNoResults).toBool();
    else
    {
        GSLOG(SYSLOG_SEV_ERROR, (QString("Cannot interpret option %1 : %2")
                                 .arg(GS::DbPluginGalaxy::RdbOptions::mOptionExtractPartsIfNoResults.toLatin1().data())
                                 .arg(m_clOptions.mOptions.value( GS::DbPluginGalaxy::RdbOptions::mOptionExtractPartsIfNoResults)
                                      .toString().toLatin1().data())).toLatin1().constData()

              );
    }
    if( m_clOptions.mOptions.value(GS::DbPluginGalaxy::RdbOptions::mOptionExtractRawData).toBool() //m_clOptions.m_bExtractRawData
            && !clTestFilter.isEmpty() && (m_clTestList.m_uiNbTests > 0))
    {
        GSLOG(SYSLOG_SEV_DEBUG, QString("Write Test Results : extracting raw data for %1 tests ...")
              .arg( m_clTestList.m_uiNbTests).toLatin1().constData());

        // Before to extract the result, check for each type of test if we need to extract something
        bool bStatus = true;
        bool bExtractResults_PTR = true;
        bool bExtractResults_MPR = true;
        bool bExtractResults_FTR = true;
        if(!clTestFilter.extractAllTests())
        {
            if(m_clTestList.getTestIdList_PTR().isEmpty())
                bExtractResults_PTR = false;
            if(m_clTestList.getTestIdList_MPR().isEmpty())
                bExtractResults_MPR = false;
            if(m_clTestList.getTestIdList_FTR().isEmpty())
                bExtractResults_FTR = false;
        }
        // Don't retrieve MPTests and FTests for E-Test testing stage
        if(m_strTestingStage == GEXDB_PLUGIN_GALAXY_ETEST)
        {
            bExtractResults_MPR = false;
            bExtractResults_FTR = false;
        }


        if(mBackgroundTransferActivated
                && (mBackgroundTransferInProgress > 0))
        {
            if(bExtractResults_PTR)
            {
                if(!BackgroundTransferLazyMode(m_strTablePrefix + "ptest_results",clSplitlotInfo.m_lSplitlotID))
                    return "error : PTR query execution failed";
            }
            if(bExtractResults_FTR)
            {
                if(!BackgroundTransferLazyMode(m_strTablePrefix + "ftest_results",clSplitlotInfo.m_lSplitlotID))
                    return "error : FTR query execution failed";
            }
            if(bExtractResults_MPR)
            {
                if(!BackgroundTransferLazyMode(m_strTablePrefix + "mptest_results",clSplitlotInfo.m_lSplitlotID))
                    return "error : MPR query execution failed";
            }
        }

        if(bExtractResults_PTR)
        {
            // Create query to extract PTest results
            Query_Empty();
            strFieldSpec = "Field|" + m_strTablePrefix;
            Query_NormalizeToken(m_strTablePrefix + "ptest_results.ptest_info_id", strDbField, strDbTable);
            strTableAlias = m_strTablePrefix + "ptest_results";
            m_mapQuery_TableAliases[strDbTable] = strTableAlias;
            m_strlQuery_Fields.append(strFieldSpec + "ptest_results.ptest_info_id");
            m_strlQuery_Fields.append(strFieldSpec + "ptest_results.run_id");
            m_strlQuery_Fields.append(strFieldSpec + "ptest_results.result_flags");
            strCondition = "Expression|" + m_strTablePrefix;
            strCondition += "ptest_results.value|(";
            strCondition += m_strTablePrefix + "ptest_results.value+0)";
            m_strlQuery_Fields.append(strCondition);
            strCondition =  m_strTablePrefix + "ptest_results.splitlot_id|Numeric|" + QString::number(clSplitlotInfo.m_lSplitlotID);
            m_strlQuery_ValueConditions.append(strCondition);
            if (sampling>1)
            {
                m_strlQuery_ValueConditions.append( m_strTablePrefix + QString("ptest_results.run_id=mod(run_id, %1)|Numeric|0" ).arg(sampling) );
            }

            // Narrow on tests specified in testlist, except if all tests should be extracted
            if(!clTestFilter.extractAllTests())
                Query_AddTestlistCondition(clTestFilter, 'P');
            // add sort instruction
            m_strlQuery_OrderFields.append(m_strTablePrefix + "ptest_results.run_id");
            // Build query
            bStatus=Query_BuildSqlString(strQuery_PTR, false, strTableAlias);
            if(m_pclDatabaseConnector->IsOracleDB())
                bStatus=Query_BuildSqlString(strQuery_PTR_NoIndex, false);
            if (!bStatus)
                GSLOG(SYSLOG_SEV_ERROR, "Cannot build PTR query");

            // Optimize iteration through query results specifying we will use only forward direction
            clGexDbQuery_PTR.setForwardOnly(true);
        }

        // Create query to extract MPTest results
        Query_Empty();
        if(bExtractResults_MPR)
        {
            strFieldSpec = "Field|" + m_strTablePrefix;
            Query_NormalizeToken(m_strTablePrefix + "mptest_results.mptest_info_id", strDbField, strDbTable);
            strTableAlias = m_strTablePrefix + "mptest_results";
            m_mapQuery_TableAliases[strDbTable] = strTableAlias;
            m_strlQuery_Fields.append(strFieldSpec + "mptest_results.mptest_info_id");
            m_strlQuery_Fields.append(strFieldSpec + "mptest_results.run_id");
            m_strlQuery_Fields.append(strFieldSpec + "mptest_results.result_flags");
            strCondition = "Expression|" + m_strTablePrefix;
            strCondition += "mptest_results.value|(";
            strCondition += m_strTablePrefix + "mptest_results.value+0)";
            m_strlQuery_Fields.append(strCondition);
            m_strlQuery_Fields.append(strFieldSpec + "mptest_results.tpin_pmrindex");
            strCondition =  m_strTablePrefix + "mptest_results.splitlot_id|Numeric|" + QString::number(clSplitlotInfo.m_lSplitlotID);
            m_strlQuery_ValueConditions.append(strCondition);

            if (sampling>1)
            {
                m_strlQuery_ValueConditions.append( m_strTablePrefix + QString("mptest_results.run_id=mod(run_id, %1)|Numeric|0" ).arg(sampling) );
            }

            // Narrow on tests specified in testlist, except if all tests should be extracted
            if(!clTestFilter.extractAllTests())
                Query_AddTestlistCondition(clTestFilter, 'M');
            // add sort instruction
            m_strlQuery_OrderFields.append(m_strTablePrefix + "mptest_results.run_id");
            // Build query
            bStatus=Query_BuildSqlString(strQuery_MPR, false, strTableAlias);
            if(m_pclDatabaseConnector->IsOracleDB())
                Query_BuildSqlString(strQuery_MPR_NoIndex, false);
            if (!bStatus)
                GSLOG(SYSLOG_SEV_ERROR, "Cannot build sql MPR query");

            // Optimize iteration through query results specifying we will use only forward direction
            // Following line has been removed for MySQL, to avoid MySQL error when embedding 2 QSqlQuery queries, and having the ForwardOnly mode activated:
            // In MySQL, if several queries are executed, only the last one executed can have the ForwardOnly mode set
            // Commands out of sync; you can't run this command now QMYSQL3: Unable to execute query
            if(m_pclDatabaseConnector->m_strDriver != "QMYSQL3")
                clGexDbQuery_MPR.setForwardOnly(true);
        }

        // Create query to extract FTest results
        Query_Empty();
        if(bExtractResults_FTR)
        {
            strFieldSpec = "Field|" + m_strTablePrefix;
            Query_NormalizeToken(m_strTablePrefix + "ftest_results.ftest_info_id", strDbField, strDbTable);
            strTableAlias = m_strTablePrefix + "ftest_results";
            m_mapQuery_TableAliases[strDbTable] = strTableAlias;
            m_strlQuery_Fields.append(strFieldSpec + "ftest_results.ftest_info_id");
            m_strlQuery_Fields.append(strFieldSpec + "ftest_results.run_id");
            strCondition = "Expression|" + m_strTablePrefix;
            strCondition += "ftest_results.result_flags|(";
            strCondition += m_strTablePrefix + "ftest_results.result_flags)";
            m_strlQuery_Fields.append(strCondition);
            m_strlQuery_Fields.append(strFieldSpec + "ftest_results.vect_nam");
            m_strlQuery_Fields.append(strFieldSpec + "ftest_results.vect_off");
            strCondition =  m_strTablePrefix + "ftest_results.splitlot_id|Numeric|" + QString::number(clSplitlotInfo.m_lSplitlotID);
            m_strlQuery_ValueConditions.append(strCondition);

            if (sampling>1)
            {
                m_strlQuery_ValueConditions.append( m_strTablePrefix + QString("ftest_results.run_id=mod(run_id, %1)|Numeric|0" ).arg(sampling) );
            }

            // Narrow on tests specified in testlist, except if all tests should be extracted
            if(!clTestFilter.extractAllTests())
                Query_AddTestlistCondition(clTestFilter, 'F');
            // add sort instruction
            m_strlQuery_OrderFields.append(m_strTablePrefix + "ftest_results.run_id");
            // Build query
            bStatus=Query_BuildSqlString(strQuery_FTR, false, strTableAlias);
            if(m_pclDatabaseConnector->IsOracleDB())
                bStatus=Query_BuildSqlString(strQuery_FTR_NoIndex, false);
            if (!bStatus)
                GSLOG(SYSLOG_SEV_ERROR, "Cannot build FTR sql string");
            // Optimize iteration through query results specifying we will use only forward direction
            // Following line has been removed for MySQL, to avoid MySQL error when embedding 2 QSqlQuery queries, and having the ForwardOnly mode activated:
            // In MySQL, if several queries are executed, only the last one executed can have the ForwardOnly mode set
            // Commands out of sync; you can't run this command now QMYSQL3: Unable to execute query
            if(m_pclDatabaseConnector->m_strDriver != "QMYSQL3")
                clGexDbQuery_FTR.setForwardOnly(true);
        }
        bStatus = true;

        // Don't retrieve MPTests and FTests for E-Test testing stage
        if(m_strTestingStage != GEXDB_PLUGIN_GALAXY_ETEST)
        {
            // Execute FTR query
            if(bExtractResults_FTR)
                bStatus = clGexDbQuery_FTR.Execute(strQuery_FTR);
            if(!bStatus && m_pclDatabaseConnector->IsOracleDB()
                    && clGexDbQuery_FTR.lastError().text().contains("ORA-01502", Qt::CaseInsensitive))
            {
                if(m_bSkipAllIndexDialogs || bSkipIndexDialog)
                    bStatus = clGexDbQuery_FTR.Execute(strQuery_FTR_NoIndex);
                else
                {
                    QString logMessage(
                                "Some required indexes are in an unusable state.\n"
                                "The extraction may take far more time than usual");
                    strMessage = logMessage + ".\nDo you want to continue ?";
                    if (!mGexScriptEngine->property("GS_DAEMON").toBool())
                    {
                        buttonNoIndexAnswer = QMessageBox::warning(
                                    mParentWidget,m_strPluginName,strMessage,
                                    QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::No, QMessageBox::No);
                        if (buttonNoIndexAnswer == QMessageBox::No)
                        {
                            m_bAbortForUnusableIndex = true;
                        }
                        else
                        {
                            m_bSkipAllIndexDialogs =
                                    (buttonNoIndexAnswer == QMessageBox::YesToAll);
                            bSkipIndexDialog =
                                    (buttonNoIndexAnswer == QMessageBox::Yes);
                            bStatus =
                                    clGexDbQuery_FTR.Execute(strQuery_FTR_NoIndex);
                        }
                    }
                    else
                    {
                        GSLOG(SYSLOG_SEV_WARNING,
                              logMessage.toLatin1().constData());
                        m_bAbortForUnusableIndex = true;
                    }
                }
            }
            if(!bStatus)
            {
                GSLOG(SYSLOG_SEV_ERROR, clGexDbQuery_FTR.lastError().text().toLatin1().constData());
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery_FTR.toLatin1().constData(),
                            clGexDbQuery_FTR.lastError().text().toLatin1().constData());
                return "error : FTR query execution failed";
            }

            // Execute MPR query
            if(bExtractResults_MPR)
                bStatus = clGexDbQuery_MPR.Execute(strQuery_MPR);
            if(!bStatus && m_pclDatabaseConnector->IsOracleDB() && clGexDbQuery_MPR.lastError().text().contains("ORA-01502", Qt::CaseInsensitive))
            {
                if(m_bSkipAllIndexDialogs || bSkipIndexDialog)
                    bStatus = clGexDbQuery_MPR.Execute(strQuery_MPR_NoIndex);
                else
                {
                    QString logMessage(
                                "Some required indexes are in an unusable state.\n"
                                "The extraction may take far more time than usual");
                    strMessage = logMessage + ".\nDo you want to continue ?";

                    if (!mGexScriptEngine->property("GS_DAEMON").toBool())
                    {
                        buttonNoIndexAnswer = QMessageBox::warning(
                                    mParentWidget,m_strPluginName,strMessage,
                                    QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::No,QMessageBox::No);
                        if (buttonNoIndexAnswer == QMessageBox::No)
                        {
                            m_bAbortForUnusableIndex = true;
                        }
                        else
                        {
                            m_bSkipAllIndexDialogs =
                                    (buttonNoIndexAnswer == QMessageBox::YesToAll);
                            bSkipIndexDialog =
                                    (buttonNoIndexAnswer == QMessageBox::Yes);
                            bStatus =
                                    clGexDbQuery_MPR.Execute(strQuery_MPR_NoIndex);
                        }
                    }
                    else
                    {
                        GSLOG(SYSLOG_SEV_WARNING,
                              logMessage.toLatin1().constData());
                        m_bAbortForUnusableIndex = true;
                    }
                }
            }
            if(!bStatus)
            {
                GSLOG(SYSLOG_SEV_ERROR, clGexDbQuery_MPR.lastError().text().toLatin1().constData());
                GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery_MPR.toLatin1().constData(),
                            clGexDbQuery_MPR.lastError().text().toLatin1().constData());
                return "error : MPR query execution failed";
            }
        }

        // Execute PTR query
        if(bExtractResults_PTR)
            bStatus = clGexDbQuery_PTR.Execute(strQuery_PTR);
        if(!bStatus && m_pclDatabaseConnector->IsOracleDB() && clGexDbQuery_PTR.lastError().text().contains("ORA-01502", Qt::CaseInsensitive))
        {
            if(m_bSkipAllIndexDialogs || bSkipIndexDialog)
                bStatus = clGexDbQuery_PTR.Execute(strQuery_PTR_NoIndex);
            else
            {
                QString logMessage("Some required indexes are in an unusable state.\n"
                                   "The extraction may take far more time than usual");
                strMessage = logMessage + ".\nDo you want to continue ?";
                if (!mGexScriptEngine->property("GS_DAEMON").toBool())
                {
                    buttonNoIndexAnswer = QMessageBox::warning(
                                mParentWidget,m_strPluginName,strMessage,
                                QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::No,QMessageBox::No);
                    if (buttonNoIndexAnswer == QMessageBox::No)
                    {
                        m_bAbortForUnusableIndex = true;
                    }
                    else
                    {
                        m_bSkipAllIndexDialogs =
                                (buttonNoIndexAnswer == QMessageBox::YesToAll);
                        bSkipIndexDialog =
                                (buttonNoIndexAnswer == QMessageBox::Yes);
                        bStatus =
                                clGexDbQuery_PTR.Execute(strQuery_PTR_NoIndex);
                    }
                }
                else
                {
                    GSLOG(SYSLOG_SEV_WARNING,
                          logMessage.toLatin1().constData());
                    m_bAbortForUnusableIndex = true;
                }
            }
        }

        if(!bStatus)
        {
            GSLOG(SYSLOG_SEV_ERROR, clGexDbQuery_PTR.lastError().text().toLatin1().data());
            GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery_PTR.toLatin1().constData(), clGexDbQuery_PTR.lastError().text().toLatin1().constData());
            return "error : PTR query execution failed";
        }

        // Iterate through test results
        bStatus_MPR = bStatus_FTR = false;
        bStatus_PTR = clGexDbQuery_PTR.Next();
        if(m_strTestingStage != GEXDB_PLUGIN_GALAXY_ETEST)
        {
            bStatus_MPR = clGexDbQuery_MPR.Next();
            bStatus_FTR = clGexDbQuery_FTR.Next();
        }
        while(bStatus_PTR || bStatus_MPR || bStatus_FTR)
        {
            // Check for user abort
            if(mQueryProgress->IsAbortRequested())
            {
                GSET_ERROR0(GexDbPlugin_Base, eDB_Abort, NULL);
                return "error : abort requested";
            }

            // Get RunIds
            if(bStatus_PTR)
            {
                nRunID_PTR = clGexDbQuery_PTR.value(1).toInt();
                nRunID = nRunID_PTR;
            }
            if(bStatus_MPR)
            {
                nRunID_MPR = clGexDbQuery_MPR.value(1).toInt();
                nRunID = nRunID_MPR;
            }
            if(bStatus_FTR)
            {
                nRunID_FTR = clGexDbQuery_FTR.value(1).toInt();
                nRunID = nRunID_FTR;
            }

            if(bStatus_PTR)
                nRunID = qMin(nRunID, nRunID_PTR);
            if(bStatus_MPR)
                nRunID = qMin(nRunID, nRunID_MPR);
            if(bStatus_FTR)
                nRunID = qMin(nRunID, nRunID_FTR);

            // Check if we have some part results
            if(nRunID < m_nRunInfoArray_NbItems)
            {
                // Any PIR/PRR to write?
                if(nCurrentItem == -1)
                {
                    nCurrentItem = 1;
                    // Write all parts with no test results
                    while(nCurrentItem < nRunID)
                    {
                        // Done : replaced with a function GetPart(int desired_run_id);
                        pCurrentPart = GetPart(nCurrentItem); // was pCurrentPart = m_pRunInfoArray + nCurrentItem;
                        if(!pCurrentPart)
                        {
                            // There is a bug during the insertion that insert null PTR results for RunId=1
                            // even if the RunId is invalid and not inserted in the run table
                            // We can have RunId 1 in the PTest_results table and not in the Run table

                            // check if the PTR result is valid
                            if(!clGexDbQuery_PTR.isNull(3))
                            {
                                strMessage = "CurrentPart NULL and PTR result NULL for runID "
                                        + QString::number(nCurrentItem);
                                GSLOG(SYSLOG_SEV_ERROR,
                                      strMessage.toLatin1().constData());
                                GSET_ERROR1(GexDbPlugin_Base, eInternal_MissingRun, NULL, nCurrentItem);
                                return "error : " + strMessage;
                            }
                            // Else just count nb of ignored runs
                            uiNbRuns_Ignored++;
                        }
                        else if(!pCurrentPart->m_bPartExcluded)
                        {
                            uiNbRuns++;
                            bool b=false;
                            if(ExtractPartsIfNoResults && !clGexDbQuery_PTR.isNull(3)) // A162 : check me with Sandrine or Bernard
                            {
                                b=WritePart(clStdfParse, pCurrentPart, autowrite_parts_with_same_runid);
                                if (!b)
                                    GSLOG(SYSLOG_SEV_WARNING, QString("WritePart failed for part %1")
                                          .arg( pCurrentPart->m_strPartID).toLatin1().constData());
                            }
                            m_clTestList.ResetDynamicTestInfo(); // Move me upper ?
                            // Update query progress dialog
                            if (mQueryProgress)
                                mQueryProgress->SetRetrievedResults(uiNbRuns, m_uiTotalTestResults);
                        }
                        else
                        {
                            // Else just count nb of ignored runs
                            uiNbRuns_Ignored++;
                        }

                        nCurrentItem++;
                    }
                    nCurrentItem = nRunID;
                    pCurrentPart = GetPart(nCurrentItem);	//pCurrentPart = m_pRunInfoArray + nCurrentItem;
                    if(!pCurrentPart)
                    {
                        // check if the PTR result is valid
                        if(!clGexDbQuery_PTR.isNull(3))
                        {
                            strMessage = "CurrentPart NULL and PTR result NULL for runID " + QString::number(nCurrentItem);
                            GSLOG(SYSLOG_SEV_ERROR,
                                  strMessage.toLatin1().constData());
                            GSET_ERROR1(GexDbPlugin_Base, eInternal_MissingRun, NULL, nCurrentItem);
                            return "error : " + strMessage;
                        }
                        // Else just count nb of ignored runs
                        uiNbRuns_Ignored++;
                    }
                    else if(!pCurrentPart->m_bPartExcluded)
                    {
                        uiNbRuns++;
                        // Removed following block because will be done in WritePart()
                        //WritePIR(clStdfParse, pCurrentPart->m_nSiteNb);
                        //pCurrentPart->m_bWrittenToStdf = true;
                        m_clTestList.ResetDynamicTestInfo();
                        // Update query progress dialog
                        if (mQueryProgress)
                            mQueryProgress->SetRetrievedResults(uiNbRuns, m_uiTotalTestResults);
                    }
                    else
                    {
                        // Else just count nb of ignored runs
                        uiNbRuns_Ignored++;
                    }
                } // CurrentItem=-1
                else if(nCurrentItem != nRunID)
                {
                    if(pCurrentPart && !pCurrentPart->m_bPartExcluded)
                    {
                        WritePart(clStdfParse, pCurrentPart, autowrite_parts_with_same_runid);
                        m_clTestList.ResetDynamicTestInfo();
                    }
                    nCurrentItem++;
                    // Write all parts with no test results
                    while(nCurrentItem < nRunID)
                    {
                        pCurrentPart=GetPart(nCurrentItem);	//pCurrentPart = m_pRunInfoArray + nCurrentItem;
                        if(!pCurrentPart)
                        {
                            // check if the PTR result is valid
                            if(!clGexDbQuery_PTR.isNull(3))
                            {
                                strMessage = "CurrentPart NULL and PTR result NULL for runID " + QString::number(nCurrentItem);
                                GSLOG(SYSLOG_SEV_ERROR,
                                      strMessage.toLatin1().constData());
                                GSET_ERROR1(GexDbPlugin_Base, eInternal_MissingRun, NULL, nCurrentItem);
                                return "error : " + strMessage;
                            }
                            // Else just count nb of ignored runs
                            uiNbRuns_Ignored++;
                        }
                        else if(!pCurrentPart->m_bPartExcluded)
                        {
                            uiNbRuns++;
                            if(ExtractPartsIfNoResults)
                                WritePart(clStdfParse, pCurrentPart, autowrite_parts_with_same_runid);
                            m_clTestList.ResetDynamicTestInfo();
                            // Update query progress dialog
                            if (mQueryProgress)
                                mQueryProgress->SetRetrievedResults(uiNbRuns, m_uiTotalTestResults);
                        }
                        else
                        {
                            // Else just count nb of ignored runs
                            uiNbRuns_Ignored++;
                        }

                        nCurrentItem++;
                    }
                    nCurrentItem = nRunID;
                    pCurrentPart = GetPart(nCurrentItem);	//pCurrentPart = m_pRunInfoArray + nCurrentItem
                    if(!pCurrentPart)
                    {
                        // check if the PTR result is valid
                        if(!clGexDbQuery_PTR.isNull(3))
                        {
                            strMessage = "CurrentPart NULL and PTR result NULL for runID " + QString::number(nCurrentItem);
                            GSLOG(SYSLOG_SEV_ERROR,
                                  strMessage.toLatin1().constData());
                            GSET_ERROR1(GexDbPlugin_Base, eInternal_MissingRun, NULL, nCurrentItem);
                            return "error : " + strMessage;
                        }

                        // Else just count nb of ignored runs
                        uiNbRuns_Ignored++;
                    }
                    else if(!pCurrentPart->m_bPartExcluded)
                    {
                        uiNbRuns++;
                        // Remove following block (will be done in WritePart())
                        //WritePIR(clStdfParse, pCurrentPart->m_nSiteNb);
                        //pCurrentPart->m_bWrittenToStdf = true;
                        m_clTestList.ResetDynamicTestInfo();
                        // Update query progress dialog
                        if (mQueryProgress)
                            mQueryProgress->SetRetrievedResults(uiNbRuns, m_uiTotalTestResults);
                    }
                    else
                    {
                        // Else just count nb of ignored runs
                        uiNbRuns_Ignored++;
                    }
                } // CurrentItem != runID

                // Save current test results
                if(pCurrentPart && !pCurrentPart->m_bPartExcluded)
                {
                    // PTR?
                    if(bStatus_PTR && (nRunID_PTR == nRunID))
                    {
                        pTestInfo = m_clTestList.FindTestByID_PTR(clGexDbQuery_PTR.value(0).toLongLong());
                        if(pTestInfo)
                        {
                            pTestInfo->m_bTestExecuted = true;
                            pTestInfo->m_bHaveSamples = true;
                            if(pTestInfo->m_pTestInfo_PTR)
                            {
                                uiDynamicFlags = 0;
                                fResult = std::numeric_limits<double>::quiet_NaN();

                                uiFlags = clGexDbQuery_PTR.value(2).toUInt();
                                if((uiFlags & FLAG_TESTRESULT_PASS) == 0)
                                    uiDynamicFlags |= 0x80;
                                if((uiFlags & FLAG_TESTRESULT_INVALID_PASSFLAG) != 0)
                                    uiDynamicFlags |= 0x40;
                                if((uiFlags & FLAG_TESTRESULT_ALARM) != 0)
                                    uiDynamicFlags |= 0x01;

                                if(((uiFlags & FLAG_TESTRESULT_INVALID_RESULT) != 0)
                                        || (clGexDbQuery_PTR.isNull(3)))
                                    uiDynamicFlags |= 0x02;
                                else
                                    fResult = clGexDbQuery_PTR.value(3).toDouble();

                                pTestInfo->m_pTestInfo_PTR->m_fResultList.append(fResult);
                                pTestInfo->m_pTestInfo_PTR->m_uiDynamicFlagsList.append(uiDynamicFlags);
                            }
                            if(m_bCustomerDebugMode)
                                uiNbTestResults++;
                            m_uiTotalTestResults++;
                        }
                    }

                    // MPR?
                    if(bStatus_MPR && (nRunID_MPR == nRunID))
                    {
                        pTestInfo = m_clTestList.FindTestByID_MPR(clGexDbQuery_MPR.value(0).toLongLong());
                        if(pTestInfo)
                        {
                            pTestInfo->m_bTestExecuted = true;
                            pTestInfo->m_bHaveSamples = true;
                            if(pTestInfo->m_pTestInfo_MPR)
                            {
                                uiDynamicFlags = 0;
                                fResult = std::numeric_limits<double>::quiet_NaN();

                                uiFlags = clGexDbQuery_MPR.value(2).toUInt();
                                if((uiFlags & FLAG_TESTRESULT_PASS) == 0)
                                    uiDynamicFlags |= 0x80;
                                if((uiFlags & FLAG_TESTRESULT_INVALID_PASSFLAG) != 0)
                                    uiDynamicFlags |= 0x40;
                                if((uiFlags & FLAG_TESTRESULT_ALARM) != 0)
                                    uiDynamicFlags |= 0x01;


                                if(((uiFlags & FLAG_TESTRESULT_INVALID_RESULT) != 0)
                                        || clGexDbQuery_MPR.isNull(3))
                                {
                                    // TEST_FLG
                                    // bit 1: Reserved for future use. Must be zero.
                                    // The value in the RESULT field is not valid DOESN'T EXIST for MPR
                                    // uiDynamicFlags |= 0x02;
                                }
                                else
                                    fResult = clGexDbQuery_MPR.value(3).toDouble();

                                nTpin_PmrIndex = clGexDbQuery_MPR.value(4).toInt();

                                pTestInfo->m_pTestInfo_MPR->m_fResultList.append(fResult);
                                pTestInfo->m_pTestInfo_MPR->m_uiDynamicFlagsList.append(uiDynamicFlags);
                                if(nTpin_PmrIndex > -1)
                                    pTestInfo->m_pTestInfo_MPR->m_nTpin_PmrIndexList.append(nTpin_PmrIndex);
                            }
                            if(m_bCustomerDebugMode)
                                uiNbTestResults++;
                            m_uiTotalTestResults++;
                        }
                    }

                    // FTR?
                    if(bStatus_FTR && (nRunID_FTR == nRunID))
                    {
                        pTestInfo = m_clTestList.FindTestByID_FTR(clGexDbQuery_FTR.value(0).toLongLong());
                        if(pTestInfo)
                        {
                            pTestInfo->m_bTestExecuted = true;
                            pTestInfo->m_bHaveSamples = true;
                            if(pTestInfo->m_pTestInfo_FTR)
                            {
                                uiDynamicFlags = 0;

                                uiFlags = clGexDbQuery_FTR.value(2).toUInt();
                                if(clSplitlotInfo.m_strIncremental_Update.toLower().indexOf("update_ftestresults_flags") == -1)
                                {
                                    if((uiFlags & FLAG_TESTRESULT_PASS) == 0)
                                        uiDynamicFlags |= 0x80;
                                    if((uiFlags & FLAG_TESTRESULT_INVALID_PASSFLAG) != 0)
                                        uiDynamicFlags |= 0x40;
                                    if((uiFlags & FLAG_TESTRESULT_ALARM) != 0)
                                        uiDynamicFlags |= 0x01;
                                    if((uiFlags & FLAG_TESTRESULT_INVALID_RESULT) != 0)
                                        uiDynamicFlags |= 0x02;
                                }
                                else
                                    // At the begining of GexDb, the FTR.Test_flags was directly saved in the DB
                                    uiDynamicFlags = uiFlags;

                                pTestInfo->m_pTestInfo_FTR->m_uiDynamicFlagsList.append(uiDynamicFlags);
                                pTestInfo->m_pTestInfo_FTR->m_strVectorNameList.append(clGexDbQuery_FTR.value(3).toString());
                                pTestInfo->m_pTestInfo_FTR->m_uiVectorOffsetList.append(clGexDbQuery_FTR.value(4).toUInt());
                            }
                            if(m_bCustomerDebugMode)
                                uiNbTestResults++;
                            m_uiTotalTestResults++;
                        }
                    }
                } // if (pCurrentPart && runid != -1)
            }

            if(bStatus_PTR && (nRunID_PTR == nRunID))
                bStatus_PTR = clGexDbQuery_PTR.Next();
            if(bStatus_MPR && (nRunID_MPR == nRunID))
                bStatus_MPR = clGexDbQuery_MPR.Next();
            if(bStatus_FTR && (nRunID_FTR == nRunID))
                bStatus_FTR = clGexDbQuery_FTR.Next();
        }

        // Write last PRR (always have a result for this one)
        if((nCurrentItem != -1) && pCurrentPart && !pCurrentPart->m_bPartExcluded)
        {
            // WritePart() instead of following block
            WritePart(clStdfParse, pCurrentPart, autowrite_parts_with_same_runid);
            m_clTestList.ResetDynamicTestInfo();
        }
    } // if extraction raw data

    // Write parts with no test executed
    m_clTestList.ResetDynamicTestInfo();

    if (uiNbRuns < m_uiSplitlotNbRuns
            && ExtractPartsIfNoResults    // A162
            )
    {
        GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Writing %1 parts not found in runs (probably no test results)...")
              .arg( m_uiSplitlotNbRuns-uiNbRuns).toLatin1().constData());
        // Ignored runs will be re-computed here
        uiNbRuns_Ignored = 0;

        // Check for user abort
        if(mQueryProgress->IsAbortRequested())
        {
            GSET_ERROR0(GexDbPlugin_Base, eDB_Abort, NULL);
            return QString("Error : abort requested");
        }

        for(nCurrentItem=0; nCurrentItem<m_nRunInfoArray_NbItems; nCurrentItem++)
        {
            pCurrentPart = GetPart(nCurrentItem);	//pCurrentPart = m_pRunInfoArray + nCurrentItem;
            if (!pCurrentPart ) // GEX_ASSERT(pCurrentPart!=NULL);
            {
                uiNbRuns_Ignored++;
                continue;
            }
            else if(pCurrentPart->m_bPartExcluded)
            {
                uiNbRuns_Ignored++;
                continue;
            }

            if(!pCurrentPart->m_bPartExcluded && (pCurrentPart->m_bWrittenToStdf == false))
            {
                uiNbRuns++;
                WritePart(clStdfParse, pCurrentPart, autowrite_parts_with_same_runid);
                // Update query progress dialog
                if (mQueryProgress)
                    mQueryProgress->SetRetrievedResults(uiNbRuns, m_uiTotalTestResults, true);
            }
        }
    }

    // Update query progress dialog (force data update)
    if (mQueryProgress)
        mQueryProgress->SetRetrievedResults(uiNbRuns, m_uiTotalTestResults, true);

    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("WriteTestResults : %1 runs written to STDF file, %2 runs ignored.").arg(uiNbRuns).arg(uiNbRuns_Ignored)
          .toLatin1().constData());

    if(m_bCustomerDebugMode)
    {
        // Stop partial timer
        m_clExtractionPerf.Stop(clGexDbQuery_PTR.m_ulRetrievedRows_Cumul+clGexDbQuery_MPR.m_ulRetrievedRows_Cumul+clGexDbQuery_FTR.m_ulRetrievedRows_Cumul,
                                clGexDbQuery_PTR.m_fTimer_DbQuery_Cumul+clGexDbQuery_MPR.m_fTimer_DbQuery_Cumul+clGexDbQuery_FTR.m_fTimer_DbQuery_Cumul,
                                clGexDbQuery_PTR.m_fTimer_DbIteration_Cumul+clGexDbQuery_MPR.m_fTimer_DbIteration_Cumul+clGexDbQuery_FTR.m_fTimer_DbIteration_Cumul,
                                uiNbRuns, uiNbTestResults);

        // Write partial performance
        WritePartialPerformance((char*)"WriteTestResults()");
    }

    return "ok";
}


bool GexDbPlugin_Galaxy::WriteTestInfo(GQTL_STDF::StdfParse & lStdfParse, const GexDbPlugin_RunInfo *pCurrentPart)
{
    GQTL_STDF::Stdf_PTR_V4	lPTR;
    GQTL_STDF::Stdf_MPR_V4	clMPR;
    GQTL_STDF::Stdf_FTR_V4	clFTR;
    BYTE			bOptFlag;

    QList<GexDbPlugin_TestInfo*>	list_pMPRTestInfos;

    GexDbPlugin_TestInfo			*pTestInfo=0, *pMPR_RefTest=0;
    GexDbPlugin_TestInfoContainer	*pContainer=0, *pContainerRef=0;

    pContainer = m_clTestList.m_pFirstTest;
    while(pContainer)
    {
        pTestInfo = pContainer->m_pTestInfo;

        if (!pTestInfo)
        {
            GSLOG(SYSLOG_SEV_ERROR, "pTestInfo NULL");
            break;
        }

        if(pTestInfo->m_bTestExecuted && (pTestInfo->m_cTestType == 'P'))
        {
            for(int nIndex=0; nIndex<pTestInfo->m_pTestInfo_PTR->m_uiDynamicFlagsList.count(); nIndex++)
            {
                // Set PTR fields
                lPTR.SetTEST_NUM(pTestInfo->m_uiTestNumber);
                lPTR.SetHEAD_NUM(1);
                lPTR.SetSITE_NUM(pCurrentPart->m_nSiteNb);
                lPTR.SetTEST_FLG(pTestInfo->m_pTestInfo_PTR->m_uiDynamicFlagsList.at(nIndex));
                lPTR.SetPARM_FLG(pTestInfo->m_pTestInfo_PTR->m_uiStaticFlags);
                lPTR.SetRESULT(pTestInfo->m_pTestInfo_PTR->m_fResultList.at(nIndex));

                bool lFirstPassForTest = false; // check if static test info already written
                if(!pTestInfo->m_bStaticInfoWrittenToStdf)
                {
                    bOptFlag = 0;
                    if(!pTestInfo->m_pTestInfo_PTR->m_bHasLL)
                        bOptFlag |= 0x40;
                    if(!pTestInfo->m_pTestInfo_PTR->m_bHasHL)
                        bOptFlag |= 0x80;

                    lPTR.SetTEST_TXT(pTestInfo->m_strTestName);
                    lPTR.SetALARM_ID("");
                    lPTR.SetOPT_FLAG(bOptFlag);
                    if(pTestInfo->m_pTestInfo_PTR->m_nResScal == GEXDB_INVALID_SCALING_FACTOR)
                        lPTR.SetRES_SCAL((pTestInfo->m_pTestInfo_PTR->m_strTestUnit == "%") ? 2:0);
                    else
                        lPTR.SetRES_SCAL(pTestInfo->m_pTestInfo_PTR->m_nResScal);
                    if(pTestInfo->m_pTestInfo_PTR->m_nLlScal == GEXDB_INVALID_SCALING_FACTOR)
                        lPTR.SetLLM_SCAL((pTestInfo->m_pTestInfo_PTR->m_strTestUnit == "%") ? 2:0);
                    else
                        lPTR.SetLLM_SCAL(pTestInfo->m_pTestInfo_PTR->m_nLlScal);
                    if(pTestInfo->m_pTestInfo_PTR->m_nHlScal == GEXDB_INVALID_SCALING_FACTOR)
                        lPTR.SetHLM_SCAL((pTestInfo->m_pTestInfo_PTR->m_strTestUnit == "%") ? 2:0);
                    else
                        lPTR.SetHLM_SCAL(pTestInfo->m_pTestInfo_PTR->m_nHlScal);
                    lPTR.SetLO_LIMIT(pTestInfo->m_pTestInfo_PTR->m_fLL);
                    lPTR.SetHI_LIMIT(pTestInfo->m_pTestInfo_PTR->m_fHL);
                    lPTR.SetUNITS(pTestInfo->m_pTestInfo_PTR->m_strTestUnit);
                    lPTR.SetC_RESFMT("");
                    lPTR.SetC_LLMFMT("");
                    lPTR.SetC_HLMFMT("");
                    if(pTestInfo->m_pTestInfo_PTR->m_bHasSpecLL)
                        lPTR.SetLO_SPEC(pTestInfo->m_pTestInfo_PTR->m_fSpecLL);
                    if(pTestInfo->m_pTestInfo_PTR->m_bHasSpecHL)
                        lPTR.SetHI_SPEC(pTestInfo->m_pTestInfo_PTR->m_fSpecHL);

                    pTestInfo->m_bStaticInfoWrittenToStdf = true;
                    lFirstPassForTest = true;
                }

                // Write record
                lStdfParse.WriteRecord(&lPTR);
                lPTR.Reset();

                // Write multi limits in DTR if any
                if (lFirstPassForTest)
                {
                    for(int lIt = 0; lIt < pTestInfo->m_pTestInfo_PTR->mMultiLimits.size(); ++lIt)
                    {
                        GQTL_STDF::Stdf_DTR_V4 lDTR;
                        lDTR.SetTEXT_DAT(QJsonObject::fromVariantMap(
                                             pTestInfo->m_pTestInfo_PTR->mMultiLimits.at(lIt)));
                        // Write record
                        lStdfParse.WriteRecord(&lDTR);
                    }
                }
            }

            pContainer = pContainer->m_pNextTest;
        }
        else if(pTestInfo->m_bTestExecuted && (pTestInfo->m_cTestType == 'M'))
        {
            pMPR_RefTest = pTestInfo;
            pContainerRef = pContainer;
            int nMPR_MaxArrayIndexResult;
            int nMPR_MaxArrayIndexPin;

            for(int nIndex=0; nIndex<pMPR_RefTest->m_pTestInfo_MPR->m_uiDynamicFlagsList.count(); nIndex++)
            {
                // Save all MPR's with same test nb and test name
                //pMPR_Tests.append(pMPR_RefTest);
                list_pMPRTestInfos.append(pMPR_RefTest);

                nMPR_MaxArrayIndexResult = -1;
                if(nIndex < pTestInfo->m_pTestInfo_MPR->m_fResultList.count())
                    nMPR_MaxArrayIndexResult = pMPR_RefTest->m_pTestInfo_MPR->m_nTpin_ArrayIndex;
                nMPR_MaxArrayIndexPin = -1;
                if(nIndex < pTestInfo->m_pTestInfo_MPR->m_nTpin_PmrIndexList.count())
                    nMPR_MaxArrayIndexPin = pMPR_RefTest->m_pTestInfo_MPR->m_nTpin_ArrayIndex;
                pContainer = pContainer->m_pNextTest;
                while(pContainer)
                {
                    pTestInfo = pContainer->m_pTestInfo;
                    if((pTestInfo->m_cTestType!='M') || (pTestInfo->m_uiTestNumber != pMPR_RefTest->m_uiTestNumber) || (pTestInfo->m_strTestName != pMPR_RefTest->m_strTestName))
                        break;
                    //pMPR_Tests.append(pTestInfo);
                    if(pTestInfo->m_bTestExecuted)
                    {
                        list_pMPRTestInfos.append(pTestInfo);
                        if(nMPR_MaxArrayIndexResult > -1)
                            nMPR_MaxArrayIndexResult = qMax(nMPR_MaxArrayIndexResult, pTestInfo->m_pTestInfo_MPR->m_nTpin_ArrayIndex);
                        if(nMPR_MaxArrayIndexPin > -1)
                            nMPR_MaxArrayIndexPin = qMax(nMPR_MaxArrayIndexPin, pTestInfo->m_pTestInfo_MPR->m_nTpin_ArrayIndex);;

                        if(!pTestInfo->m_bStaticInfoWrittenToStdf)
                        {
                            pMPR_RefTest->m_bStaticInfoWrittenToStdf = false;
                            pTestInfo->m_bStaticInfoWrittenToStdf = true;
                        }
                    }
                    else // all pin must be executed
                        break;
                    pContainer = pContainer->m_pNextTest;
                }

                // Set MPR fields
                clMPR.SetTEST_NUM(pMPR_RefTest->m_uiTestNumber);
                clMPR.SetHEAD_NUM(1);
                clMPR.SetSITE_NUM(pCurrentPart->m_nSiteNb);
                clMPR.SetTEST_FLG(pMPR_RefTest->m_pTestInfo_MPR->m_uiDynamicFlagsList.at(nIndex));
                clMPR.SetPARM_FLG(pMPR_RefTest->m_pTestInfo_MPR->m_uiStaticFlags);
                clMPR.SetRTN_ICNT(nMPR_MaxArrayIndexPin+1);
                clMPR.SetRSLT_CNT(nMPR_MaxArrayIndexResult+1);

                //for(pTestInfo=pMPR_Tests.first(); pTestInfo; pTestInfo=pMPR_Tests.next())
                foreach(pTestInfo, list_pMPRTestInfos)
                {
                    clMPR.SetRTN_STAT(pTestInfo->m_pTestInfo_MPR->m_nTpin_ArrayIndex, 0);
                    if(nIndex < pTestInfo->m_pTestInfo_MPR->m_fResultList.count())
                        clMPR.SetRTN_RSLT(pTestInfo->m_pTestInfo_MPR->m_nTpin_ArrayIndex, pTestInfo->m_pTestInfo_MPR->m_fResultList.at(nIndex));
                    if(nIndex < pTestInfo->m_pTestInfo_MPR->m_nTpin_PmrIndexList.count())
                        clMPR.SetRTN_INDX(pTestInfo->m_pTestInfo_MPR->m_nTpin_ArrayIndex, pTestInfo->m_pTestInfo_MPR->m_nTpin_PmrIndexList.at(nIndex));
                }
                clMPR.SetTEST_TXT(pMPR_RefTest->m_strTestName);
                clMPR.SetALARM_ID("");
                if(!pMPR_RefTest->m_bStaticInfoWrittenToStdf)
                {
                    bOptFlag = 0;
                    if(!pMPR_RefTest->m_pTestInfo_MPR->m_bHasLL)
                        bOptFlag |= 0x40;
                    if(!pMPR_RefTest->m_pTestInfo_MPR->m_bHasHL)
                        bOptFlag |= 0x80;

                    clMPR.SetOPT_FLAG(bOptFlag);
                    if(pMPR_RefTest->m_pTestInfo_MPR->m_nResScal == GEXDB_INVALID_SCALING_FACTOR)
                        clMPR.SetRES_SCAL((pMPR_RefTest->m_pTestInfo_MPR->m_strTestUnit == "%") ? 2:0);
                    else
                        clMPR.SetRES_SCAL(pMPR_RefTest->m_pTestInfo_MPR->m_nResScal);
                    if(pMPR_RefTest->m_pTestInfo_MPR->m_nLlScal == GEXDB_INVALID_SCALING_FACTOR)
                        clMPR.SetLLM_SCAL((pMPR_RefTest->m_pTestInfo_MPR->m_strTestUnit == "%") ? 2:0);
                    else
                        clMPR.SetLLM_SCAL(pMPR_RefTest->m_pTestInfo_MPR->m_nLlScal);
                    if(pMPR_RefTest->m_pTestInfo_MPR->m_nHlScal == GEXDB_INVALID_SCALING_FACTOR)
                        clMPR.SetHLM_SCAL((pMPR_RefTest->m_pTestInfo_MPR->m_strTestUnit == "%") ? 2:0);
                    else
                        clMPR.SetHLM_SCAL(pMPR_RefTest->m_pTestInfo_MPR->m_nHlScal);
                    clMPR.SetLO_LIMIT(pMPR_RefTest->m_pTestInfo_MPR->m_fLL);
                    clMPR.SetHI_LIMIT(pMPR_RefTest->m_pTestInfo_MPR->m_fHL);
                    clMPR.SetSTART_IN(0.0F);
                    clMPR.SetINCR_IN(0.0F);
                    clMPR.SetUNITS(pMPR_RefTest->m_pTestInfo_MPR->m_strTestUnit);
                    clMPR.SetUNITS_IN("");
                    clMPR.SetC_RESFMT("");
                    clMPR.SetC_LLMFMT("");
                    clMPR.SetC_HLMFMT("");

                    if(pMPR_RefTest->m_pTestInfo_MPR->m_bHasSpecLL)
                        clMPR.SetLO_SPEC(pMPR_RefTest->m_pTestInfo_MPR->m_fSpecLL);
                    if(pMPR_RefTest->m_pTestInfo_MPR->m_bHasSpecHL)
                        clMPR.SetHI_SPEC(pMPR_RefTest->m_pTestInfo_MPR->m_fSpecHL);

                    pMPR_RefTest->m_bStaticInfoWrittenToStdf = true;
                }
                else
                    clMPR.SetOPT_FLAG(0x30);	// No LL/HL, use the ones in first MPR for same test

                // Write record
                lStdfParse.WriteRecord(&clMPR);
                //pMPR_Tests.clear();
                list_pMPRTestInfos.clear();
                clMPR.Reset();

                if ( (nIndex+1) < pMPR_RefTest->m_pTestInfo_MPR->m_uiDynamicFlagsList.count())
                    pContainer = pContainerRef;
            }
        }
        else if(pTestInfo->m_bTestExecuted && (pTestInfo->m_cTestType == 'F'))
        {

            for(int nIndex=0; nIndex<pTestInfo->m_pTestInfo_FTR->m_uiDynamicFlagsList.count(); nIndex++)
            {
                // Set FTR fields
                clFTR.SetTEST_NUM(pTestInfo->m_uiTestNumber);
                clFTR.SetHEAD_NUM(1);
                clFTR.SetSITE_NUM(pCurrentPart->m_nSiteNb);
                clFTR.SetTEST_FLG(pTestInfo->m_pTestInfo_FTR->m_uiDynamicFlagsList.at(nIndex));
                clFTR.SetOPT_FLAG((BYTE)0xdf);		// Only VECT_OFF is valid
                clFTR.SetCYCL_CNT(0);
                clFTR.SetREL_VADR(0);
                clFTR.SetREPT_CNT(0);
                clFTR.SetNUM_FAIL(0);
                clFTR.SetXFAIL_AD(0);
                clFTR.SetYFAIL_AD(0);
                clFTR.SetVECT_OFF(pTestInfo->m_pTestInfo_FTR->m_uiVectorOffsetList.at(nIndex));
                clFTR.SetRTN_ICNT(0);
                clFTR.SetPGM_ICNT(0);
                clFTR.SetFAIL_PIN();
                clFTR.SetVECT_NAM(pTestInfo->m_pTestInfo_FTR->m_strVectorNameList.at(nIndex));
                if(!pTestInfo->m_bStaticInfoWrittenToStdf)
                {
                    clFTR.SetTIME_SET("");
                    clFTR.SetOP_CODE("");
                    clFTR.SetTEST_TXT(pTestInfo->m_strTestName);

                    pTestInfo->m_bStaticInfoWrittenToStdf = true;
                }

                // Write record
                lStdfParse.WriteRecord(&clFTR);
                clFTR.Reset();
            }
            pContainer = pContainer->m_pNextTest;
        }
        else
            pContainer = pContainer->m_pNextTest;
    }

    return true;
}

//////////////////////////////////////////////////////////////////////
// Create soft and hard binning maps
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::CreateBinningMaps(const GexDbPlugin_Galaxy_SplitlotInfo & clSplitlotInfo,
                                           bool bClearBinningMaps/*=true*/)
{
    int					nBinning;
    if (!m_pclDatabaseConnector)
        return false;
    GexDbPlugin_Query	clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    QString				strQuery, strCondition, strFieldSpec;

    // Debug mode ??
    if(m_bCustomerDebugMode)
    {
        GSLOG(SYSLOG_SEV_DEBUG, QString("Create bin maps for splitlot %1...")
              .arg( clSplitlotInfo.m_lSplitlotID).toLatin1().constData());
        m_clExtractionPerf.Start();
    }

    ///////////////////////////////////////////////////////////////////////////////////////
    // SOFTWARE BINNING (SBR)
    ///////////////////////////////////////////////////////////////////////////////////////

    // Clear binning map?
    if(bClearBinningMaps)
        m_mapSoftBins.clear();

    //Initialize local software binning map
    Query_Empty();
    strFieldSpec = "Field|";
    strFieldSpec += m_strTablePrefix;
    m_strlQuery_Fields.append(strFieldSpec + "sbin.sbin_no");
    m_strlQuery_Fields.append(strFieldSpec + "sbin.sbin_name");
    m_strlQuery_Fields.append(strFieldSpec + "sbin.sbin_cat");
    strCondition = m_strTablePrefix + "sbin.splitlot_id|Numeric|" + QString::number(clSplitlotInfo.m_lSplitlotID);
    m_strlQuery_ValueConditions.append(strCondition);
    Query_BuildSqlString(strQuery, false);

    clGexDbQuery.setForwardOnly(true);
    if(!clGexDbQuery.Execute(strQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        return false;
    }

    // Iterate through binnings
    while(clGexDbQuery.Next())
    {
        nBinning = clGexDbQuery.value(0).toInt();
        m_mapSoftBins[nBinning].m_nBinNo = nBinning;
        m_mapSoftBins[nBinning].m_nBinCount = 0;
        m_mapSoftBins[nBinning].m_cBinCat = clGexDbQuery.value(2).toString()[0];
        m_mapSoftBins[nBinning].m_strBinName = clGexDbQuery.value(1).toString();
    }

    ///////////////////////////////////////////////////////////////////////////////////////
    // HARDWARE BINNING (HBR)
    ///////////////////////////////////////////////////////////////////////////////////////

    // Clear binning map
    if(bClearBinningMaps)
        m_mapHardBins.clear();

    // Initialize local hardware binning map
    Query_Empty();
    strFieldSpec = "Field|";
    strFieldSpec += m_strTablePrefix;
    m_strlQuery_Fields.append(strFieldSpec + "hbin.hbin_no");
    m_strlQuery_Fields.append(strFieldSpec + "hbin.hbin_name");
    m_strlQuery_Fields.append(strFieldSpec + "hbin.hbin_cat");
    strCondition = m_strTablePrefix + "hbin.splitlot_id|Numeric|" + QString::number(clSplitlotInfo.m_lSplitlotID);
    m_strlQuery_ValueConditions.append(strCondition);
    Query_BuildSqlString(strQuery, false);

    clGexDbQuery.setForwardOnly(true);
    if(!clGexDbQuery.Execute(strQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        return false;
    }

    // Iterate through binnings
    while(clGexDbQuery.Next())
    {
        nBinning = clGexDbQuery.value(0).toInt();
        m_mapHardBins[nBinning].m_nBinNo = nBinning;
        m_mapHardBins[nBinning].m_nBinCount = 0;
        m_mapHardBins[nBinning].m_cBinCat = clGexDbQuery.value(2).toString()[0];
        m_mapHardBins[nBinning].m_strBinName = clGexDbQuery.value(1).toString();
    }

    // Debug mode??
    if(m_bCustomerDebugMode)
    {
        // Stop partial timer
        m_clExtractionPerf.Stop(clGexDbQuery.m_ulRetrievedRows_Cumul, clGexDbQuery.m_fTimer_DbQuery_Cumul, clGexDbQuery.m_fTimer_DbIteration_Cumul);

        // Write partial performance
        WritePartialPerformance((char*)"CreateBinningMaps()");
    }

    return true;
}



//////////////////////////////////////////////////////////////////////
// Get test summary (Parametric tests)
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::GetTestStats_P(const GexDbPlugin_Galaxy_SplitlotInfo & clSplitlotInfo)
{
    QString							strFieldSpec, strQuery, strCondition;
    if (!m_pclDatabaseConnector)
        return false;
    GexDbPlugin_Query				clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    GexDbPlugin_TestInfo_Stats		clTestStats;
    GexDbPlugin_TestInfo			*pTestInfo;

    // Get PTR test summary stats
    Query_Empty();
    strFieldSpec = "Field|";
    strFieldSpec += m_strTablePrefix;
    m_strlQuery_Fields.append(strFieldSpec + "ptest_stats_summary.ptest_info_id");
    m_strlQuery_Fields.append(strFieldSpec + "ptest_stats_summary.site_no");
    m_strlQuery_Fields.append(strFieldSpec + "ptest_stats_summary.exec_count");
    m_strlQuery_Fields.append(strFieldSpec + "ptest_stats_summary.fail_count");
    m_strlQuery_Fields.append(strFieldSpec + "ptest_stats_summary.min_value");
    m_strlQuery_Fields.append(strFieldSpec + "ptest_stats_summary.max_value");
    m_strlQuery_Fields.append(strFieldSpec + "ptest_stats_summary.sum");
    m_strlQuery_Fields.append(strFieldSpec + "ptest_stats_summary.square_sum");
    m_strlQuery_Fields.append(strFieldSpec + "ptest_stats_summary.ttime");
    strCondition = m_strTablePrefix + "ptest_stats_summary.splitlot_id|Numeric|" + QString::number(clSplitlotInfo.m_lSplitlotID);
    m_strlQuery_ValueConditions.append(strCondition);
    // Add filter on site # ?
    if(!clSplitlotInfo.m_strSiteFilterValue.isEmpty())
    {
        strCondition = m_strTablePrefix + "ptest_stats_summary.site_no|Numeric|" + clSplitlotInfo.m_strSiteFilterValue;
        m_strlQuery_ValueConditions.append(strCondition);
    }
    Query_BuildSqlString(strQuery, false);

    //clGexDbQuery.setForwardOnly(true);
    if(!clGexDbQuery.Execute(strQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        return false;
    }

    // Iterate through tests
    unsigned int	uiOptFlag;
    unsigned int	uiExecCount;
    int				nSiteNum;
    while(clGexDbQuery.Next())
    {
        clTestStats.Reset();
        uiOptFlag = 0xff;
        pTestInfo = m_clTestList.FindTestByID_PTR(clGexDbQuery.value(0).toLongLong());
        if(pTestInfo == NULL)
            continue;
        if(clGexDbQuery.isNull(2))
            continue;
        uiExecCount = clGexDbQuery.value(2).toUInt();
        if(uiExecCount == 0)
            continue;

        // Save stats info
        nSiteNum = clGexDbQuery.value(1).toInt();
        clTestStats.m_uiExecCount = uiExecCount;
        if(!clGexDbQuery.isNull(3))
        {
            clTestStats.m_uiFailCount = clGexDbQuery.value(3).toUInt();
            pTestInfo->m_bMinimumStatsFromSummaryAvailable = true;
        }
        if(!clGexDbQuery.isNull(8))
        {
            clTestStats.m_fTestTime = clGexDbQuery.value(8).toDouble();
            uiOptFlag &= 0xfb;
        }
        if(!clGexDbQuery.isNull(4))
        {
            clTestStats.m_fMin = clGexDbQuery.value(4).toDouble();
            uiOptFlag &= 0xfe;
        }
        if(!clGexDbQuery.isNull(5))
        {
            clTestStats.m_fMax = clGexDbQuery.value(5).toDouble();
            uiOptFlag &= 0xfd;
        }
        if(!clGexDbQuery.isNull(6))
        {
            clTestStats.m_fSum = clGexDbQuery.value(6).toDouble();
            uiOptFlag &= 0xef;
        }
        if(!clGexDbQuery.isNull(7))
        {
            clTestStats.m_fSumSquare = clGexDbQuery.value(7).toDouble();
            uiOptFlag &= 0xdf;
        }
        clTestStats.m_uiOptFlag = uiOptFlag;
        if((uiOptFlag & 0x33) == 0)
            pTestInfo->m_bStatsFromSummaryComplete = true;
        pTestInfo->m_mapStatsFromSummary[nSiteNum] = clTestStats;
    }

    // Get PTR test samples stats
    Query_Empty();
    strFieldSpec = "Field|";
    strFieldSpec += m_strTablePrefix;
    m_strlQuery_Fields.append(strFieldSpec + "ptest_stats_samples.ptest_info_id");
    m_strlQuery_Fields.append(strFieldSpec + "ptest_stats_samples.site_no");
    m_strlQuery_Fields.append(strFieldSpec + "ptest_stats_samples.exec_count");
    m_strlQuery_Fields.append(strFieldSpec + "ptest_stats_samples.fail_count");
    m_strlQuery_Fields.append(strFieldSpec + "ptest_stats_samples.min_value");
    m_strlQuery_Fields.append(strFieldSpec + "ptest_stats_samples.max_value");
    m_strlQuery_Fields.append(strFieldSpec + "ptest_stats_samples.sum");
    m_strlQuery_Fields.append(strFieldSpec + "ptest_stats_samples.square_sum");
    strCondition = m_strTablePrefix + "ptest_stats_samples.splitlot_id|Numeric|" + QString::number(clSplitlotInfo.m_lSplitlotID);
    m_strlQuery_ValueConditions.append(strCondition);
    // Add filter on site # ?
    if(!clSplitlotInfo.m_strSiteFilterValue.isEmpty())
    {
        strCondition = m_strTablePrefix + "ptest_stats_samples.site_no|Numeric|" + clSplitlotInfo.m_strSiteFilterValue;
        m_strlQuery_ValueConditions.append(strCondition);
    }
    Query_BuildSqlString(strQuery, false);

    clGexDbQuery.setForwardOnly(true);
    if(!clGexDbQuery.Execute(strQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        return false;
    }

    // Iterate through tests
    while(clGexDbQuery.Next())
    {
        clTestStats.Reset();
        uiOptFlag = 0xff;
        pTestInfo = m_clTestList.FindTestByID_PTR(clGexDbQuery.value(0).toLongLong());
        if(pTestInfo == NULL)
            continue;
        if(clGexDbQuery.isNull(2))
            continue;
        uiExecCount = clGexDbQuery.value(2).toUInt();
        if(uiExecCount == 0)
            continue;

        // Save stats info
        nSiteNum = clGexDbQuery.value(1).toInt();
        clTestStats.m_uiExecCount = uiExecCount;
        if(!clGexDbQuery.isNull(3))
            clTestStats.m_uiFailCount = clGexDbQuery.value(3).toUInt();
        if(!clGexDbQuery.isNull(4))
        {
            clTestStats.m_fMin = clGexDbQuery.value(4).toDouble();
            uiOptFlag &= 0xfe;
        }
        if(!clGexDbQuery.isNull(5))
        {
            clTestStats.m_fMax = clGexDbQuery.value(5).toDouble();
            uiOptFlag &= 0xfd;
        }
        if(!clGexDbQuery.isNull(6))
        {
            clTestStats.m_fSum = clGexDbQuery.value(6).toDouble();
            uiOptFlag &= 0xef;
        }
        if(!clGexDbQuery.isNull(7))
        {
            clTestStats.m_fSumSquare = clGexDbQuery.value(7).toDouble();
            uiOptFlag &= 0xdf;
        }
        clTestStats.m_uiOptFlag = uiOptFlag;
        pTestInfo->m_mapStatsFromSamples[nSiteNum] = clTestStats;
    }

    return true;
}

//////////////////////////////////////////////////////////////////////
// Get test summary (Multi-Parametric tests)
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::GetTestStats_MP(const GexDbPlugin_Galaxy_SplitlotInfo & clSplitlotInfo)
{
    QString							strFieldSpec, strQuery, strCondition;
    if (!m_pclDatabaseConnector)
        return false;
    GexDbPlugin_Query				clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    GexDbPlugin_TestInfo_Stats		clTestStats;
    GexDbPlugin_TestInfo			*pTestInfo;

    // Get MPR test summary stats
    Query_Empty();
    strFieldSpec = "Field|";
    strFieldSpec += m_strTablePrefix;
    m_strlQuery_Fields.append(strFieldSpec + "mptest_stats_summary.mptest_info_id");
    m_strlQuery_Fields.append(strFieldSpec + "mptest_stats_summary.site_no");
    m_strlQuery_Fields.append(strFieldSpec + "mptest_stats_summary.exec_count");
    m_strlQuery_Fields.append(strFieldSpec + "mptest_stats_summary.fail_count");
    m_strlQuery_Fields.append(strFieldSpec + "mptest_stats_summary.min_value");
    m_strlQuery_Fields.append(strFieldSpec + "mptest_stats_summary.max_value");
    m_strlQuery_Fields.append(strFieldSpec + "mptest_stats_summary.sum");
    m_strlQuery_Fields.append(strFieldSpec + "mptest_stats_summary.square_sum");
    m_strlQuery_Fields.append(strFieldSpec + "mptest_stats_summary.ttime");
    strCondition = m_strTablePrefix + "mptest_stats_summary.splitlot_id|Numeric|" + QString::number(clSplitlotInfo.m_lSplitlotID);
    m_strlQuery_ValueConditions.append(strCondition);
    // Add filter on site # ?
    if(!clSplitlotInfo.m_strSiteFilterValue.isEmpty())
    {
        strCondition = m_strTablePrefix + "mptest_stats_summary.site_no|Numeric|" + clSplitlotInfo.m_strSiteFilterValue;
        m_strlQuery_ValueConditions.append(strCondition);
    }
    Query_BuildSqlString(strQuery, false);

    clGexDbQuery.setForwardOnly(true);
    if(!clGexDbQuery.Execute(strQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        return false;
    }

    // Iterate through tests
    unsigned int	uiOptFlag;
    unsigned int	uiExecCount;
    int				nSiteNum;
    while(clGexDbQuery.Next())
    {
        clTestStats.Reset();
        uiOptFlag = 0xff;
        pTestInfo = m_clTestList.FindTestByID_MPR(clGexDbQuery.value(0).toLongLong());
        if(pTestInfo == NULL)
            continue;
        if(clGexDbQuery.isNull(2))
            continue;
        uiExecCount = clGexDbQuery.value(2).toUInt();
        if(uiExecCount == 0)
            continue;

        // Save stats info
        nSiteNum = clGexDbQuery.value(1).toInt();
        clTestStats.m_uiExecCount = uiExecCount;
        if(!clGexDbQuery.isNull(3))
        {
            clTestStats.m_uiFailCount = clGexDbQuery.value(3).toUInt();
            pTestInfo->m_bMinimumStatsFromSummaryAvailable = true;
        }
        if(!clGexDbQuery.isNull(8))
        {
            clTestStats.m_fTestTime = clGexDbQuery.value(8).toDouble();
            uiOptFlag &= 0xfb;
        }
        if(!clGexDbQuery.isNull(4))
        {
            clTestStats.m_fMin = clGexDbQuery.value(4).toDouble();
            uiOptFlag &= 0xfe;
        }
        if(!clGexDbQuery.isNull(5))
        {
            clTestStats.m_fMax = clGexDbQuery.value(5).toDouble();
            uiOptFlag &= 0xfd;
        }
        if(!clGexDbQuery.isNull(6))
        {
            clTestStats.m_fSum = clGexDbQuery.value(6).toDouble();
            uiOptFlag &= 0xef;
        }
        if(!clGexDbQuery.isNull(7))
        {
            clTestStats.m_fSumSquare = clGexDbQuery.value(7).toDouble();
            uiOptFlag &= 0xdf;
        }
        clTestStats.m_uiOptFlag = uiOptFlag;
        if((uiOptFlag & 0x33) == 0)
            pTestInfo->m_bStatsFromSummaryComplete = true;
        pTestInfo->m_mapStatsFromSummary[nSiteNum] = clTestStats;
    }

    // Get MPR test samples stats
    Query_Empty();
    strFieldSpec = "Field|";
    strFieldSpec += m_strTablePrefix;
    m_strlQuery_Fields.append(strFieldSpec + "mptest_stats_samples.mptest_info_id");
    m_strlQuery_Fields.append(strFieldSpec + "mptest_stats_samples.site_no");
    m_strlQuery_Fields.append(strFieldSpec + "mptest_stats_samples.exec_count");
    m_strlQuery_Fields.append(strFieldSpec + "mptest_stats_samples.fail_count");
    m_strlQuery_Fields.append(strFieldSpec + "mptest_stats_samples.min_value");
    m_strlQuery_Fields.append(strFieldSpec + "mptest_stats_samples.max_value");
    m_strlQuery_Fields.append(strFieldSpec + "mptest_stats_samples.sum");
    m_strlQuery_Fields.append(strFieldSpec + "mptest_stats_samples.square_sum");
    strCondition = m_strTablePrefix + "mptest_stats_samples.splitlot_id|Numeric|" + QString::number(clSplitlotInfo.m_lSplitlotID);
    m_strlQuery_ValueConditions.append(strCondition);
    // Add filter on site # ?
    if(!clSplitlotInfo.m_strSiteFilterValue.isEmpty())
    {
        strCondition = m_strTablePrefix + "mptest_stats_samples.site_no|Numeric|" + clSplitlotInfo.m_strSiteFilterValue;
        m_strlQuery_ValueConditions.append(strCondition);
    }
    Query_BuildSqlString(strQuery, false);

    clGexDbQuery.setForwardOnly(true);
    if(!clGexDbQuery.Execute(strQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        return false;
    }

    // Iterate through tests
    while(clGexDbQuery.Next())
    {
        clTestStats.Reset();
        uiOptFlag = 0xff;
        pTestInfo = m_clTestList.FindTestByID_MPR(clGexDbQuery.value(0).toLongLong());
        if(pTestInfo == NULL)
            continue;
        if(clGexDbQuery.isNull(2))
            continue;
        uiExecCount = clGexDbQuery.value(2).toUInt();
        if(uiExecCount == 0)
            continue;

        // Save stats info
        nSiteNum = clGexDbQuery.value(1).toInt();
        clTestStats.m_uiExecCount = uiExecCount;
        if(!clGexDbQuery.isNull(3))
            clTestStats.m_uiFailCount = clGexDbQuery.value(3).toUInt();
        if(!clGexDbQuery.isNull(4))
        {
            clTestStats.m_fMin = clGexDbQuery.value(4).toDouble();
            uiOptFlag &= 0xfe;
        }
        if(!clGexDbQuery.isNull(5))
        {
            clTestStats.m_fMax = clGexDbQuery.value(5).toDouble();
            uiOptFlag &= 0xfd;
        }
        if(!clGexDbQuery.isNull(6))
        {
            clTestStats.m_fSum = clGexDbQuery.value(6).toDouble();
            uiOptFlag &= 0xef;
        }
        if(!clGexDbQuery.isNull(7))
        {
            clTestStats.m_fSumSquare = clGexDbQuery.value(7).toDouble();
            uiOptFlag &= 0xdf;
        }
        clTestStats.m_uiOptFlag = uiOptFlag;
        pTestInfo->m_mapStatsFromSamples[nSiteNum] = clTestStats;
    }

    return true;
}

//////////////////////////////////////////////////////////////////////
// Get test summary (Functional tests)
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::GetTestStats_F(const GexDbPlugin_Galaxy_SplitlotInfo & clSplitlotInfo)
{
    QString							strFieldSpec, strQuery, strCondition;
    if (!m_pclDatabaseConnector)
        return false;
    GexDbPlugin_Query				clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    GexDbPlugin_TestInfo_Stats		clTestStats;
    GexDbPlugin_TestInfo			*pTestInfo;

    // Get test summary records: FTR
    Query_Empty();
    strFieldSpec = "Field|";
    strFieldSpec += m_strTablePrefix;
    m_strlQuery_Fields.append(strFieldSpec + "ftest_stats_summary.ftest_info_id");
    m_strlQuery_Fields.append(strFieldSpec + "ftest_stats_summary.site_no");
    m_strlQuery_Fields.append(strFieldSpec + "ftest_stats_summary.exec_count");
    m_strlQuery_Fields.append(strFieldSpec + "ftest_stats_summary.fail_count");
    m_strlQuery_Fields.append(strFieldSpec + "ftest_stats_summary.ttime");
    strCondition = m_strTablePrefix + "ftest_stats_summary.splitlot_id|Numeric|" + QString::number(clSplitlotInfo.m_lSplitlotID);
    m_strlQuery_ValueConditions.append(strCondition);
    // Add filter on site # ?
    if(!clSplitlotInfo.m_strSiteFilterValue.isEmpty())
    {
        strCondition = m_strTablePrefix + "ftest_stats_summary.site_no|Numeric|" + clSplitlotInfo.m_strSiteFilterValue;
        m_strlQuery_ValueConditions.append(strCondition);
    }
    Query_BuildSqlString(strQuery, false);

    clGexDbQuery.setForwardOnly(true);
    if(!clGexDbQuery.Execute(strQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        return false;
    }

    // Iterate through tests
    unsigned int	uiOptFlag;
    unsigned int	uiExecCount;
    int				nSiteNum;
    while(clGexDbQuery.Next())
    {
        clTestStats.Reset();
        uiOptFlag = 0xff;
        pTestInfo = m_clTestList.FindTestByID_FTR(clGexDbQuery.value(0).toLongLong());
        if(pTestInfo == NULL)
            continue;
        if(clGexDbQuery.isNull(2))
            continue;
        uiExecCount = clGexDbQuery.value(2).toUInt();
        if(uiExecCount == 0)
            continue;

        // Save stats info
        nSiteNum = clGexDbQuery.value(1).toInt();
        clTestStats.m_uiExecCount = uiExecCount;
        if(!clGexDbQuery.isNull(3))
        {
            clTestStats.m_uiFailCount = clGexDbQuery.value(3).toUInt();
            pTestInfo->m_bMinimumStatsFromSummaryAvailable = true;
        }
        if(!clGexDbQuery.isNull(4))
        {
            clTestStats.m_fTestTime = clGexDbQuery.value(4).toDouble();
            uiOptFlag &= 0xfb;
        }
        clTestStats.m_uiOptFlag = uiOptFlag;
        pTestInfo->m_bStatsFromSummaryComplete = false;
        pTestInfo->m_mapStatsFromSummary[nSiteNum] = clTestStats;
    }

    // Get FTR test samples stats
    Query_Empty();
    strFieldSpec = "Field|";
    strFieldSpec += m_strTablePrefix;
    m_strlQuery_Fields.append(strFieldSpec + "ftest_stats_samples.ftest_info_id");
    m_strlQuery_Fields.append(strFieldSpec + "ftest_stats_samples.site_no");
    m_strlQuery_Fields.append(strFieldSpec + "ftest_stats_samples.exec_count");
    m_strlQuery_Fields.append(strFieldSpec + "ftest_stats_samples.fail_count");
    strCondition = m_strTablePrefix + "ftest_stats_samples.splitlot_id|Numeric|" + QString::number(clSplitlotInfo.m_lSplitlotID);
    m_strlQuery_ValueConditions.append(strCondition);
    // Add filter on site # ?
    if(!clSplitlotInfo.m_strSiteFilterValue.isEmpty())
    {
        strCondition = m_strTablePrefix + "ftest_stats_samples.site_no|Numeric|" + clSplitlotInfo.m_strSiteFilterValue;
        m_strlQuery_ValueConditions.append(strCondition);
    }
    Query_BuildSqlString(strQuery, false);

    clGexDbQuery.setForwardOnly(true);
    if(!clGexDbQuery.Execute(strQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        return false;
    }

    // Iterate through tests
    while(clGexDbQuery.Next())
    {
        clTestStats.Reset();
        uiOptFlag = 0xff;
        pTestInfo = m_clTestList.FindTestByID_FTR(clGexDbQuery.value(0).toLongLong());
        if(pTestInfo == NULL)
            continue;
        if(clGexDbQuery.isNull(2))
            continue;
        uiExecCount = clGexDbQuery.value(2).toUInt();
        if(uiExecCount == 0)
            continue;

        // Save stats info
        nSiteNum = clGexDbQuery.value(1).toInt();
        clTestStats.m_uiExecCount = uiExecCount;
        if(!clGexDbQuery.isNull(3))
            clTestStats.m_uiFailCount = clGexDbQuery.value(3).toUInt();
        clTestStats.m_uiOptFlag = uiOptFlag;
        pTestInfo->m_mapStatsFromSamples[nSiteNum] = clTestStats;
    }

    return true;
}

//////////////////////////////////////////////////////////////////////
// Retrieve wafer info for specified splitlot
//////////////////////////////////////////////////////////////////////
GexDbPlugin_Galaxy_WaferInfo *GexDbPlugin_Galaxy::GetWaferInfo(const GexDbPlugin_Galaxy_SplitlotInfo & clSplitlotInfo)
{
    GSLOG(SYSLOG_SEV_DEBUG, (QString("Lot=%1 WaferID=%2 TS = %3")
                             .arg(clSplitlotInfo.m_strLotID.toLatin1().data())
                             .arg(clSplitlotInfo.m_strWaferID.toLatin1().data())
                             .arg(m_strTablePrefix.toLatin1().data() )).toLatin1().constData());
    QString							strQuery, strCondition, strFieldSpec;
    GexDbPlugin_Galaxy_WaferInfo	*pclWaferInfo;
    int								nIndex = 0;

    // Construct query string:
    // SELECT <testnum, testname>
    // FROM <all required tables>
    // WHERE <link conditions> AND <value conditions>	(optional)
    // SELECT: add query fields
    Query_Empty();
    strFieldSpec = "Field|";
    strFieldSpec += m_strTablePrefix;
    m_strlQuery_Fields.append(strFieldSpec + "wafer_info.wafer_id");
    m_strlQuery_Fields.append(strFieldSpec + "wafer_info.fab_id");
    m_strlQuery_Fields.append(strFieldSpec + "wafer_info.frame_id");
    m_strlQuery_Fields.append(strFieldSpec + "wafer_info.mask_id");
    m_strlQuery_Fields.append(strFieldSpec + "wafer_info.wafer_size");
    m_strlQuery_Fields.append(strFieldSpec + "wafer_info.die_ht");
    m_strlQuery_Fields.append(strFieldSpec + "wafer_info.die_wid");
    m_strlQuery_Fields.append(strFieldSpec + "wafer_info.wafer_units");
    m_strlQuery_Fields.append(strFieldSpec + "wafer_info.wafer_flat");
    m_strlQuery_Fields.append(strFieldSpec + "wafer_info.center_x");
    m_strlQuery_Fields.append(strFieldSpec + "wafer_info.center_y");
    m_strlQuery_Fields.append(strFieldSpec + "wafer_info.pos_x");
    m_strlQuery_Fields.append(strFieldSpec + "wafer_info.pos_y");
    m_strlQuery_Fields.append(strFieldSpec + "wafer_info.gross_die");
    m_strlQuery_Fields.append(strFieldSpec + "wafer_info.nb_parts");
    m_strlQuery_Fields.append(strFieldSpec + "wafer_info.nb_parts_good");
    m_strlQuery_Fields.append(strFieldSpec + "wafer_info.wafer_flags");
    m_strlQuery_Fields.append(strFieldSpec + "wafer_info.wafer_nb");
    if(m_strTestingStage == GEXDB_PLUGIN_GALAXY_ETEST)
        m_strlQuery_Fields.append(strFieldSpec + "wafer_info.site_config");
    strCondition = m_strTablePrefix + "wafer_info.lot_id|String|" + clSplitlotInfo.m_strLotID;
    m_strlQuery_ValueConditions.append(strCondition);
    strCondition = m_strTablePrefix + "wafer_info.wafer_id|String|" + clSplitlotInfo.m_strWaferID;
    m_strlQuery_ValueConditions.append(strCondition);

    // Construct query from table and conditions
    Query_BuildSqlString(strQuery);

    // Execute query
    if (!m_pclDatabaseConnector)
        return NULL;

    GexDbPlugin_Query clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    if(!clGexDbQuery.Execute(strQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        return NULL;
    }

    // Create and fill wafer info class
    if(clGexDbQuery.Next())
    {
        pclWaferInfo = new GexDbPlugin_Galaxy_WaferInfo;
        pclWaferInfo->m_strWaferID		= clGexDbQuery.value(nIndex++).toString();
        pclWaferInfo->m_strFabID		= clGexDbQuery.value(nIndex++).toString();
        pclWaferInfo->m_strFrameID		= clGexDbQuery.value(nIndex++).toString();
        pclWaferInfo->m_strMaskID		= clGexDbQuery.value(nIndex++).toString();
        pclWaferInfo->m_fWaferSize		= clGexDbQuery.value(nIndex++).toDouble();
        pclWaferInfo->m_fDieHt			= clGexDbQuery.value(nIndex++).toDouble();
        pclWaferInfo->m_fDieWid			= clGexDbQuery.value(nIndex++).toDouble();
        pclWaferInfo->m_uiWaferUnits	= clGexDbQuery.value(nIndex++).toUInt();
        pclWaferInfo->m_cWaferFlat		= clGexDbQuery.GetChar(nIndex++);
        pclWaferInfo->m_nCenterX		= clGexDbQuery.value(nIndex++).toInt();
        pclWaferInfo->m_nCenterY		= clGexDbQuery.value(nIndex++).toInt();
        pclWaferInfo->m_cPosX			= clGexDbQuery.GetChar(nIndex++);
        pclWaferInfo->m_cPosY			= clGexDbQuery.GetChar(nIndex++);
        pclWaferInfo->m_uiGrossDie		= clGexDbQuery.value(nIndex++).toUInt();
        pclWaferInfo->m_uiNbParts		= clGexDbQuery.value(nIndex++).toUInt();
        pclWaferInfo->m_uiNbParts_Good	= clGexDbQuery.value(nIndex++).toUInt();
        pclWaferInfo->m_uiFlags			= clGexDbQuery.value(nIndex++).toUInt();
        pclWaferInfo->m_nWaferNb = -1;
        if(clGexDbQuery.value(nIndex).isNull())
            pclWaferInfo->m_nWaferNb	= clGexDbQuery.value(nIndex).toInt();
        nIndex++;
        if(m_strTestingStage == GEXDB_PLUGIN_GALAXY_ETEST)
            pclWaferInfo->m_strEtestSiteConfig = clGexDbQuery.value(nIndex++).toString();

        if(m_bCustomerDebugMode)
            clGexDbQuery.DumpPerformance();

        return pclWaferInfo;
    }

    GSLOG(SYSLOG_SEV_NOTICE, "error : no WaferInfo found for this LotWafer !");
    return NULL;
}

//////////////////////////////////////////////////////////////////////
// Return all tests (given filters on other fields)
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::QueryTestlist(GexDbPlugin_Filter & cFilters,
                                       QStringList & cMatchingValues, bool bParametricOnly)
{
    // Debug message
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString(" bParametricOnly:%1")
          .arg( bParametricOnly?"true":"false").toLatin1().constData());

    // First check if GEXDB is up-to-date
    QString			strGexDbVersion, strVersionSupportedByPlugin;
    unsigned int	uiGexDbBuild, uiBuildSupportedByPlugin;
    bool			bDbUpToDate;
    if(!IsDbUpToDateForExtraction(&bDbUpToDate, strGexDbVersion, &uiGexDbBuild, strVersionSupportedByPlugin, &uiBuildSupportedByPlugin))
    {
        strGexDbVersion = GGET_LASTERRORMSG(GexDbPlugin_Base, this);
        GSET_ERROR1(GexDbPlugin_Base, eDB_CheckDbVersion, NULL, strGexDbVersion.toLatin1().constData());
        return false;
    }
    // Allow extraction if compatible
    if(!bDbUpToDate && !IsCompatibleForStdExtraction(uiGexDbBuild, uiBuildSupportedByPlugin))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_VersionMismatch, NULL, strGexDbVersion.toLatin1().constData(), strVersionSupportedByPlugin.toLatin1().constData());
        return false;
    }

    if(cFilters.strDataTypeQuery == GEXDB_PLUGIN_GALAXY_ETEST)
    {
        m_pmapFields_GexToRemote = &m_mapFields_GexToRemote_Et;
        m_pmapLinks_Remote = &m_mapLinks_Remote_Et;
        return GexDbPlugin_Base::QueryTestlist(cFilters, cMatchingValues, bParametricOnly);
    }
    if(cFilters.strDataTypeQuery == GEXDB_PLUGIN_GALAXY_WTEST)
    {
        m_pmapFields_GexToRemote = &m_mapFields_GexToRemote_Wt;
        m_pmapLinks_Remote = &m_mapLinks_Remote_Wt;
        return GexDbPlugin_Base::QueryTestlist(cFilters, cMatchingValues, bParametricOnly);
    }
    if(cFilters.strDataTypeQuery == GEXDB_PLUGIN_GALAXY_FTEST)
    {
        m_pmapFields_GexToRemote = &m_mapFields_GexToRemote_Ft;
        m_pmapLinks_Remote = &m_mapLinks_Remote_Ft;
        return GexDbPlugin_Base::QueryTestlist(cFilters, cMatchingValues, bParametricOnly);
    }

    return false;
}

//////////////////////////////////////////////////////////////////////
// Execute Query and return results
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::QuerySQL(QString & strQuery, QList<QStringList> & listResults)
{
    // Clear returned list of results
    listResults.clear();

    // First check if GEXDB is up-to-date
    QString			strGexDbVersion, strVersionSupportedByPlugin;
    unsigned int	uiGexDbBuild, uiBuildSupportedByPlugin;
    bool			bDbUpToDate;
    if(!IsDbUpToDateForExtraction(&bDbUpToDate, strGexDbVersion, &uiGexDbBuild, strVersionSupportedByPlugin, &uiBuildSupportedByPlugin))
    {
        strGexDbVersion = GGET_LASTERRORMSG(GexDbPlugin_Base, this);
        GSET_ERROR1(GexDbPlugin_Base, eDB_CheckDbVersion, NULL, strGexDbVersion.toLatin1().constData());
        return false;
    }
    if(!bDbUpToDate)
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_VersionMismatch, NULL, strGexDbVersion.toLatin1().constData(), strVersionSupportedByPlugin.toLatin1().constData());
        return false;
    }

    // Check database connection
    if(!ConnectToCorporateDb())
        return false;

    // Execute query
    GexDbPlugin_Query clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    if(m_pclDatabaseConnector->m_strDriver != "QMYSQL3")
    {
        // Following line is not executed with MySQL DB, to avoid MySQL error when embedding 2 QSqlQuery queries, and having the ForwardOnly mode activated:
        // Commands out of sync; you can't run this command now QMYSQL3: Unable to execute query
        clGexDbQuery.setForwardOnly(true);
    }
    if(!clGexDbQuery.Execute(strQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        return false;
    }

    unsigned int	uiNbColumns, uiIndex;
    QStringList		strlRecord;
    QString			strCell;
    while(clGexDbQuery.Next())
    {
        strlRecord.clear();
        uiNbColumns = clGexDbQuery.record().count();
        for(uiIndex=0; uiIndex<uiNbColumns; uiIndex++)
        {
            if(!clGexDbQuery.value(uiIndex).isValid() || clGexDbQuery.value(uiIndex).isNull())
                strlRecord.append(QString("n/a"));
            else
            {
                strCell = clGexDbQuery.value(uiIndex).toString();
                if(strCell == "")
                    strlRecord.append(QString("n/a"));
                else
                    strlRecord.append(strCell);
            }
        }
        listResults.append(strlRecord);
    }

    return true;
}

//////////////////////////////////////////////////////////////////////
// Return all valid splitlots (given filters on several fields)
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::QuerySplitlots(GexDbPlugin_Filter & cFilters,
                                        GexDbPlugin_SplitlotList & clSplitlotList,
                                        bool bPurge)
{
    // Clear returned list of matching splitlots
    clSplitlotList.clear();

    // First check if GEXDB is up-to-date
    QString			strGexDbVersion, strVersionSupportedByPlugin;
    unsigned int	uiGexDbBuild, uiBuildSupportedByPlugin;
    bool			bDbUpToDate;
    if(!IsDbUpToDateForExtraction(&bDbUpToDate, strGexDbVersion, &uiGexDbBuild, strVersionSupportedByPlugin, &uiBuildSupportedByPlugin))
    {
        strGexDbVersion = GGET_LASTERRORMSG(GexDbPlugin_Base, this);
        GSET_ERROR1(GexDbPlugin_Base, eDB_CheckDbVersion, NULL, strGexDbVersion.toLatin1().constData());
        return false;
    }
    if(!bDbUpToDate)
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_VersionMismatch, NULL, strGexDbVersion.toLatin1().constData(), strVersionSupportedByPlugin.toLatin1().constData());
        return false;
    }

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
        return false;

    m_strTestingStage = cFilters.strDataTypeQuery;

    // Check database connection
    if(!ConnectToCorporateDb())
        return false;

    // Compute query date constraints
    Query_ComputeDateConstraints(cFilters);

    // Construct SQL query
    QString strQuery;
    if(!ConstructSplitlotQuery(cFilters, strQuery, uiGexDbBuild,true,bPurge))
        return false;

    // Execute query
    GexDbPlugin_Query clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    if(m_pclDatabaseConnector->m_strDriver != "QMYSQL3")
    {
        // Following line is not executed with MySQL DB, to avoid MySQL error when embedding 2 QSqlQuery queries, and having the ForwardOnly mode activated:
        // Commands out of sync; you can't run this command now QMYSQL3: Unable to execute query
        clGexDbQuery.setForwardOnly(true);
    }
    if(!clGexDbQuery.Execute(strQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        return false;
    }

    // Add each Splitlot retrieved to splitlot list
    GexDbPlugin_Galaxy_SplitlotInfo	clGalaxySplitlotInfo;
    GexDbPlugin_SplitlotInfo		*pclSplitlotInfo;
    while(clGexDbQuery.Next())
    {
        // Create new SplitlotInfo object
        pclSplitlotInfo = new GexDbPlugin_SplitlotInfo;

        // Fill SplitlotInfo object
        FillSplitlotInfo(clGexDbQuery, &clGalaxySplitlotInfo, cFilters);
        pclSplitlotInfo->m_lSplitlotID		= clGalaxySplitlotInfo.m_lSplitlotID;			// SPLITLOT_ID
        pclSplitlotInfo->m_strProductName	= clGalaxySplitlotInfo.m_strProductName;		// From PART_TYP, or overloaded by dbkeys
        pclSplitlotInfo->m_strLotID			= clGalaxySplitlotInfo.m_strLotID;				// LOT_ID
        pclSplitlotInfo->m_strSublotID		= clGalaxySplitlotInfo.m_strSublotID;			// SUBLOT_ID
        pclSplitlotInfo->m_uiSetupTime		= clGalaxySplitlotInfo.m_uiSetupTime;			// SETUP_T
        pclSplitlotInfo->m_uiStartTime		= clGalaxySplitlotInfo.m_uiStartTime;			// START_T
        pclSplitlotInfo->m_uiFinishTime		= clGalaxySplitlotInfo.m_uiFinishTime;			// FINISH_T
        pclSplitlotInfo->m_uiStationNb		= clGalaxySplitlotInfo.m_uiStationNb;			// STAT_NUM
        pclSplitlotInfo->m_strTesterName	= clGalaxySplitlotInfo.m_strTesterName;			// TESTER_NAME
        pclSplitlotInfo->m_strTesterType	= clGalaxySplitlotInfo.m_strTesterType;			// TESTER_TYPE
        pclSplitlotInfo->m_uiNbParts		= clGalaxySplitlotInfo.m_uiNbParts;				// NB_PARTS
        pclSplitlotInfo->m_uiNbParts_Good	= clGalaxySplitlotInfo.m_uiNbParts_Good;		// NB_PARTS_GOOD
        pclSplitlotInfo->m_strJobName		= clGalaxySplitlotInfo.m_strJobName;			// JOB_NAME
        pclSplitlotInfo->m_strJobRev		= clGalaxySplitlotInfo.m_strJobRev;				// JOB_REV
        pclSplitlotInfo->m_strOperator		= clGalaxySplitlotInfo.m_strOperator;			// OPER_NAM
        pclSplitlotInfo->m_strExecType		= clGalaxySplitlotInfo.m_strExecType;			// EXEC_TYP
        pclSplitlotInfo->m_strExecVer		= clGalaxySplitlotInfo.m_strExecVer;			// EXEC_VER
        pclSplitlotInfo->m_strTestTemp		= clGalaxySplitlotInfo.m_strTestTemp;			// TST_TEMP
        pclSplitlotInfo->m_strHandlerType	= clGalaxySplitlotInfo.m_strHandlerType;		// HANDLER_TYP
        pclSplitlotInfo->m_strHandlerID		= clGalaxySplitlotInfo.m_strHandlerID;			// HANDLER_ID
        pclSplitlotInfo->m_strCardType		= clGalaxySplitlotInfo.m_strCardType;			// CARD_TYP
        pclSplitlotInfo->m_strCardID		= clGalaxySplitlotInfo.m_strCardID;				// CARD_ID
        pclSplitlotInfo->m_strLoadboardType	= clGalaxySplitlotInfo.m_strLoadboardType;		// LOADBOARD_TYP
        pclSplitlotInfo->m_strLoadboardID	= clGalaxySplitlotInfo.m_strLoadboardID;		// LOADBOARD_ID
        pclSplitlotInfo->m_strDibType		= clGalaxySplitlotInfo.m_strDibType;			// DIB_TYP
        pclSplitlotInfo->m_strDibID			= clGalaxySplitlotInfo.m_strDibID;				// DIB_ID
        pclSplitlotInfo->m_strCableType		= clGalaxySplitlotInfo.m_strCableType;			// CABLE_TYP
        pclSplitlotInfo->m_strCableID		= clGalaxySplitlotInfo.m_strCableID;			// CABLE_ID
        pclSplitlotInfo->m_strContactorType	= clGalaxySplitlotInfo.m_strContactorType;		// CONTACTOR_TYP
        pclSplitlotInfo->m_strContactorID	= clGalaxySplitlotInfo.m_strContactorID;		// CONTACTOR_ID
        pclSplitlotInfo->m_strLaserType		= clGalaxySplitlotInfo.m_strLaserType;			// LASER_TYP
        pclSplitlotInfo->m_strLaserID		= clGalaxySplitlotInfo.m_strLaserID;			// LASER_ID
        pclSplitlotInfo->m_strExtraType		= clGalaxySplitlotInfo.m_strExtraType;			// EXTRA_TYP
        pclSplitlotInfo->m_strExtraID		= clGalaxySplitlotInfo.m_strExtraID;			// EXTRA_ID
        pclSplitlotInfo->m_strDay			= clGalaxySplitlotInfo.m_strDay;				// DAY
        pclSplitlotInfo->m_uiWeekNb			= clGalaxySplitlotInfo.m_uiWeekNb;				// WEEK_NB
        pclSplitlotInfo->m_uiMonthNb		= clGalaxySplitlotInfo.m_uiMonthNb;				// MONTH_NB
        pclSplitlotInfo->m_uiQuarterNb		= clGalaxySplitlotInfo.m_uiQuarterNb;			// QUARTER_NB
        pclSplitlotInfo->m_uiYearNb			= clGalaxySplitlotInfo.m_uiYearNb;				// YEAR_NB
        pclSplitlotInfo->m_strYearAndWeek	= clGalaxySplitlotInfo.m_strYearAndWeek;		// YEAR_AND_WEEK
        pclSplitlotInfo->m_strYearAndMonth	= clGalaxySplitlotInfo.m_strYearAndMonth;		// YEAR_AND_MONTH
        pclSplitlotInfo->m_strYearAndQuarter= clGalaxySplitlotInfo.m_strYearAndQuarter;		// YEAR_AND_QUARTER
        pclSplitlotInfo->m_strFileName		= clGalaxySplitlotInfo.m_strFileName;			// FILE_NAME
        pclSplitlotInfo->m_strFilePath		= clGalaxySplitlotInfo.m_strFilePath;			// FILE_PATH

        // Append to the list
        clSplitlotList.append(pclSplitlotInfo);
    }

    return clSplitlotList.count() > 0;
}

//////////////////////////////////////////////////////////////////////
// Computes data for Production - UPH graph (given filters on several fields)
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::GetDataForProd_UPH(GexDbPlugin_Filter & cFilters, GexDbPlugin_XYChartList & clXYChartList)
{
    // First check if GEXDB is up-to-date
    QString			strGexDbVersion, strVersionSupportedByPlugin;
    unsigned int	uiGexDbBuild, uiBuildSupportedByPlugin;
    bool			bDbUpToDate;
    if(!IsDbUpToDateForExtraction(&bDbUpToDate, strGexDbVersion, &uiGexDbBuild, strVersionSupportedByPlugin, &uiBuildSupportedByPlugin))
    {
        strGexDbVersion = GGET_LASTERRORMSG(GexDbPlugin_Base, this);
        GSET_ERROR1(GexDbPlugin_Base, eDB_CheckDbVersion, NULL, strGexDbVersion.toLatin1().constData());
        return false;
    }
    // Allow extraction if compatible
    if(!bDbUpToDate && !IsCompatibleForStdExtraction(uiGexDbBuild, uiBuildSupportedByPlugin))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_VersionMismatch, NULL, strGexDbVersion.toLatin1().constData(), strVersionSupportedByPlugin.toLatin1().constData());
        return false;
    }

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
        return false;

    m_strTestingStage = cFilters.strDataTypeQuery;

    // Check database connection
    if(!ConnectToCorporateDb())
        return false;

    // Compute query date constraints
    Query_ComputeDateConstraints(cFilters);

    // Construct SQL query for UPH
    QString strQuery;
    if(!ConstructUPHQuery(cFilters, strQuery, clXYChartList.m_strSplitField, clXYChartList.m_strCumulField))
        return false;

    // Save query
    clXYChartList.m_strQuery = strQuery;

    // Execute query
    GexDbPlugin_Query clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    clGexDbQuery.setForwardOnly(true);
    if(!clGexDbQuery.Execute(strQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        return false;
    }

    // Create list of XY UPH ChartData objects
    QString						strSplitName="", strOldSplitName, strLabel;
    double						lfData, lfUphParts, lfUphTime;
    int							nRows=0, nIndex;
    GexDbPlugin_XYChart_Data	*pXYChartData=NULL;
    bool						bCreateNewChart=true;
    while(clGexDbQuery.Next())
    {
        nRows++;

        // Get query data
        nIndex = 0;
        if(!clXYChartList.m_strSplitField.isEmpty())
        {
            strSplitName	= clGexDbQuery.value(nIndex++).toString();
            if(strSplitName != strOldSplitName)
                bCreateNewChart = true;
            strOldSplitName = strSplitName;
        }
        strLabel		= clGexDbQuery.value(nIndex++).toString();
        lfUphParts		= clGexDbQuery.value(nIndex++).toDouble();
        lfUphTime		= clGexDbQuery.value(nIndex++).toDouble();
        if(lfUphTime > 0)
            lfData = floor((lfUphParts*3600.0)/lfUphTime);
        else
        {
            lfUphTime = 0.0;
            lfData = 0.0;
        }

        // Create new chart??
        if(bCreateNewChart)
        {
            // Add previous chart to the list
            if(pXYChartData)
            {
                pXYChartData->Convert();
                clXYChartList.append(pXYChartData);
            }
            pXYChartData = new GexDbPlugin_XYChart_Data;
            bCreateNewChart = false;
            pXYChartData->m_strSplitValue = strSplitName;
        }

        // Add data to chart
        pXYChartData->Add(lfData, strLabel, lfUphParts, lfUphTime);
    }

    // Add previous chart to the list
    if(pXYChartData)
    {
        pXYChartData->Convert();
        clXYChartList.append(pXYChartData);
    }

    // Check if we have some results
    if(nRows == 0)
    {
        GSET_ERROR0(GexDbPlugin_Base, eDB_NoResult_0, NULL);
        return false;
    }

    return true;
}




//////////////////////////////////////////////////////////////////////
// Computes data for Production - Yield graph (given filters on several fields)
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::GetDataForProd_Yield(GexDbPlugin_Filter & cFilters, GexDbPlugin_XYChartList & clXYChartList, int nBinning, bool bSoftBin)
{
    // First check if GEXDB is up-to-date
    QString			strGexDbVersion, strVersionSupportedByPlugin;
    unsigned int	uiGexDbBuild, uiBuildSupportedByPlugin;
    bool			bDbUpToDate;
    if(!IsDbUpToDateForExtraction(&bDbUpToDate, strGexDbVersion, &uiGexDbBuild, strVersionSupportedByPlugin, &uiBuildSupportedByPlugin))
    {
        strGexDbVersion = GGET_LASTERRORMSG(GexDbPlugin_Base, this);
        GSET_ERROR1(GexDbPlugin_Base, eDB_CheckDbVersion, NULL, strGexDbVersion.toLatin1().constData());
        return false;
    }
    // Allow extraction if compatible
    if(!bDbUpToDate && !IsCompatibleForStdExtraction(uiGexDbBuild, uiBuildSupportedByPlugin))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_VersionMismatch, NULL, strGexDbVersion.toLatin1().constData(), strVersionSupportedByPlugin.toLatin1().constData());
        return false;
    }

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
        return false;

    m_strTestingStage = cFilters.strDataTypeQuery;

    // Check database connection
    if(!ConnectToCorporateDb())
        return false;

    // Compute query date constraints
    Query_ComputeDateConstraints(cFilters);

    // Construct SQL query for Yield
    QString strQuery;
    if(!ConstructYieldQuery(cFilters, strQuery, clXYChartList.m_strSplitField, clXYChartList.m_strCumulField, nBinning, bSoftBin))
        return false;

    // Save query
    clXYChartList.m_strQuery = strQuery;

    // Execute query
    GexDbPlugin_Query clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    clGexDbQuery.setForwardOnly(true);
    if(!clGexDbQuery.Execute(strQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        return false;
    }

    // Create list of XY Yield ChartData objects
    QString						strSplitName="", strOldSplitName, strLabel;
    double						lfData, lfYieldParts, lfYieldParts_Matching;
    int							nRows=0, nIndex;
    GexDbPlugin_XYChart_Data	*pXYChartData=NULL;
    bool						bCreateNewChart=true;
    while(clGexDbQuery.Next())
    {
        nRows++;

        // Get query data
        nIndex = 0;
        if(!clXYChartList.m_strSplitField.isEmpty())
        {
            strSplitName	= clGexDbQuery.value(nIndex++).toString();
            if(strSplitName != strOldSplitName)
                bCreateNewChart = true;
            strOldSplitName = strSplitName;
        }
        strLabel				= clGexDbQuery.value(nIndex++).toString();
        lfYieldParts			= clGexDbQuery.value(nIndex++).toDouble();
        lfYieldParts_Matching	= clGexDbQuery.value(nIndex++).toDouble();
        if(lfYieldParts > 0)
            lfData = (lfYieldParts_Matching*100.0)/lfYieldParts;
        else
        {
            lfYieldParts = 0.0;
            lfData = 0.0;
        }

        // Create new chart??
        if(bCreateNewChart)
        {
            // Add previous chart to the list
            if(pXYChartData)
            {
                pXYChartData->Convert();
                clXYChartList.append(pXYChartData);
            }
            pXYChartData = new GexDbPlugin_XYChart_Data;
            bCreateNewChart = false;
            pXYChartData->m_strSplitValue = strSplitName;
        }

        // Add data to chart
        pXYChartData->Add(lfData, strLabel, lfYieldParts, lfYieldParts_Matching);
    }

    // Add previous chart to the list
    if(pXYChartData)
    {
        pXYChartData->Convert();
        clXYChartList.append(pXYChartData);
    }

    // Check if we have some results
    if(nRows == 0)
    {
        GSET_ERROR0(GexDbPlugin_Base, eDB_NoResult_0, NULL);
        return false;
    }

    return true;
}


//////////////////////////////////////////////////////////////////////
// Computes data for Production - Consolidated Yield graph (given filters on several fields)
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::GetDataForProd_ConsolidatedYield(GexDbPlugin_Filter & cFilters, GexDbPlugin_XYChartList & clXYChartList)
{
    // First check if GEXDB is up-to-date
    QString			strGexDbVersion, strVersionSupportedByPlugin;
    unsigned int	uiGexDbBuild, uiBuildSupportedByPlugin;
    bool			bDbUpToDate;
    if(!IsDbUpToDateForExtraction(&bDbUpToDate, strGexDbVersion, &uiGexDbBuild, strVersionSupportedByPlugin, &uiBuildSupportedByPlugin))
    {
        strGexDbVersion = GGET_LASTERRORMSG(GexDbPlugin_Base, this);
        GSET_ERROR1(GexDbPlugin_Base, eDB_CheckDbVersion, NULL, strGexDbVersion.toLatin1().constData());
        return false;
    }
    // Allow extraction if compatible
    if(!bDbUpToDate && !IsCompatibleForStdExtraction(uiGexDbBuild, uiBuildSupportedByPlugin))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_VersionMismatch, NULL, strGexDbVersion.toLatin1().constData(), strVersionSupportedByPlugin.toLatin1().constData());
        return false;
    }

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
        return false;

    m_strTestingStage = cFilters.strDataTypeQuery;

    // Check database connection
    if(!ConnectToCorporateDb())
        return false;

    // Compute query date constraints
    Query_ComputeDateConstraints(cFilters);

    // Construct SQL query for Consolidated Yield
    QString strQuery;
    if(!ConstructConsolidatedYieldQuery(cFilters, strQuery, clXYChartList.m_strSplitField, clXYChartList.m_strCumulField))
        return false;

    // Save query
    clXYChartList.m_strQuery = strQuery;

    // Execute query
    GexDbPlugin_Query clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    clGexDbQuery.setForwardOnly(true);
    if(!clGexDbQuery.Execute(strQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        return false;
    }

    // Create list of XY Yield ChartData objects
    QString						strSplitName="", strOldSplitName, strLabel;
    double						lfData, lfYieldParts, lfYieldParts_Matching;
    int							nRows=0, nIndex;
    GexDbPlugin_XYChart_Data	*pXYChartData=NULL;
    bool						bCreateNewChart=true;
    while(clGexDbQuery.Next())
    {
        nRows++;

        // Get query data
        nIndex = 0;
        if(!clXYChartList.m_strSplitField.isEmpty())
        {
            strSplitName	= clGexDbQuery.value(nIndex++).toString();
            if(strSplitName != strOldSplitName)
                bCreateNewChart = true;
            strOldSplitName = strSplitName;
        }
        strLabel				= clGexDbQuery.value(nIndex++).toString();
        lfYieldParts			= clGexDbQuery.value(nIndex++).toDouble();
        lfYieldParts_Matching	= clGexDbQuery.value(nIndex++).toDouble();
        if(lfYieldParts > 0)
            lfData = (lfYieldParts_Matching*100.0)/lfYieldParts;
        else
        {
            lfYieldParts = 0.0;
            lfData = 0.0;
        }

        // Create new chart??
        if(bCreateNewChart)
        {
            // Add previous chart to the list
            if(pXYChartData)
            {
                pXYChartData->Convert();
                clXYChartList.append(pXYChartData);
            }
            pXYChartData = new GexDbPlugin_XYChart_Data;
            bCreateNewChart = false;
            pXYChartData->m_strSplitValue = strSplitName;
        }

        // Add data to chart
        pXYChartData->Add(lfData, strLabel, lfYieldParts, lfYieldParts_Matching);
    }

    // Add previous chart to the list
    if(pXYChartData)
    {
        pXYChartData->Convert();
        clXYChartList.append(pXYChartData);
    }

    // Check if we have some results
    if(nRows == 0)
    {
        GSET_ERROR0(GexDbPlugin_Base, eDB_NoResult_0, NULL);
        return false;
    }

    return true;
}

//////////////////////////////////////////////////////////////////////
// WYR - Get specific column from data row
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::Wyr_DataRow_Getvalue(
        QString & strValue,
        GexDbPlugin_WyrFormatColumn *pWyrFormatColumn,
        GexDbPlugin_Query & clGexDbQuery)
{
    QStringList				strlSplits;
    QStringList::iterator	itSplit;
    QString					strColumn, strField;

    strValue = "";

    if(pWyrFormatColumn->m_strDataType.toLower().startsWith("date_in"))
    {
        QDate clDate = clGexDbQuery.value(3).toDate();
        strValue = clDate.toString("dd/MM/yyyy");
        return true;
    }
    if(pWyrFormatColumn->m_strDataType.toLower().startsWith("date_out"))
    {
        QDate clDate = clGexDbQuery.value(4).toDate();
        strValue = clDate.toString("dd/MM/yyyy");
        return true;
    }
    if(pWyrFormatColumn->m_strDataType.toLower().startsWith("product_name"))
    {
        strValue = clGexDbQuery.value(5).toString();
        return true;
    }
    if(pWyrFormatColumn->m_strDataType.toLower().startsWith("program_name"))
    {
        strValue = clGexDbQuery.value(6).toString();
        return true;
    }
    if(pWyrFormatColumn->m_strDataType.toLower().startsWith("tester_name"))
    {
        strValue = clGexDbQuery.value(7).toString();
        return true;
    }
    if(pWyrFormatColumn->m_strDataType.toLower().startsWith("lot_id"))
    {
        strValue = clGexDbQuery.value(8).toString();
        return true;
    }
    if(pWyrFormatColumn->m_strDataType.toLower().startsWith("subcon_lot_id"))
    {
        strValue = clGexDbQuery.value(9).toString();
        return true;
    }
    if(pWyrFormatColumn->m_strDataType.toLower().startsWith("user"))
    {
        strField = clGexDbQuery.value(10).toString();
        strlSplits = strField.split(",");
        strColumn = "C" + QString::number(pWyrFormatColumn->m_nColumnNb);
        strColumn += "=";
        for(itSplit = strlSplits.begin(); itSplit != strlSplits.end(); itSplit++)
        {
            if((*itSplit).startsWith(strColumn))
            {
                strValue = (*itSplit).section("=", 1, 1);
                return true;
            }
        }
        return false;
    }
    if(pWyrFormatColumn->m_strDataType.toLower().startsWith("yield"))
    {
        strValue = QString::number(clGexDbQuery.value(11).toDouble(), 'f', 2);
        return true;
    }
    if(pWyrFormatColumn->m_strDataType.toLower().startsWith("parts_received"))
    {
        strValue = QString::number(clGexDbQuery.value(12).toUInt());
        return true;
    }
    if(pWyrFormatColumn->m_strDataType.toLower().startsWith("pretest_rejects"))
    {
        strField = clGexDbQuery.value(14).toString();
        strlSplits = strField.split(",");
        strColumn = "C" + QString::number(pWyrFormatColumn->m_nColumnNb);
        strColumn += "=";
        for(itSplit = strlSplits.begin(); itSplit != strlSplits.end(); itSplit++)
        {
            if((*itSplit).startsWith(strColumn))
            {
                strValue = (*itSplit).section("=", 1, 1);
                return true;
            }
        }
        return false;
    }
    if(pWyrFormatColumn->m_strDataType.toLower().startsWith("parts_tested"))
    {
        strValue = QString::number(clGexDbQuery.value(15).toUInt());
        return true;
    }
    if(pWyrFormatColumn->m_strDataType.toLower().startsWith("parts_pass"))
    {
        strField = clGexDbQuery.value(17).toString();
        strlSplits = strField.split(",");
        strColumn = "C" + QString::number(pWyrFormatColumn->m_nColumnNb);
        strColumn += "=";
        for(itSplit = strlSplits.begin(); itSplit != strlSplits.end(); itSplit++)
        {
            if((*itSplit).startsWith(strColumn))
            {
                strValue = (*itSplit).section("=", 1, 1);
                return true;
            }
        }
        return false;
    }
    if(pWyrFormatColumn->m_strDataType.toLower().startsWith("parts_fail"))
    {
        strField = clGexDbQuery.value(19).toString();
        strlSplits = strField.split(",");
        strColumn = "C" + QString::number(pWyrFormatColumn->m_nColumnNb);
        strColumn += "=";
        for(itSplit = strlSplits.begin(); itSplit != strlSplits.end(); itSplit++)
        {
            if((*itSplit).startsWith(strColumn))
            {
                strValue = (*itSplit).section("=", 1, 1);
                return true;
            }
        }
        return false;
    }
    if(pWyrFormatColumn->m_strDataType.toLower().startsWith("parts_retest"))
    {
        strField = clGexDbQuery.value(21).toString();
        strlSplits = strField.split(",");
        strColumn = "C" + QString::number(pWyrFormatColumn->m_nColumnNb);
        strColumn += "=";
        for(itSplit = strlSplits.begin(); itSplit != strlSplits.end(); itSplit++)
        {
            if((*itSplit).startsWith(strColumn))
            {
                strValue = (*itSplit).section("=", 1, 1);
                return true;
            }
        }
        return false;
    }
    if(pWyrFormatColumn->m_strDataType.toLower().startsWith("insertions"))
    {
        strValue = QString::number(clGexDbQuery.value(22).toUInt());
        return true;
    }
    if(pWyrFormatColumn->m_strDataType.toLower().startsWith("posttest_rejects"))
    {
        strField = clGexDbQuery.value(24).toString();
        strlSplits = strField.split(",");
        strColumn = "C" + QString::number(pWyrFormatColumn->m_nColumnNb);
        strColumn += "=";
        for(itSplit = strlSplits.begin(); itSplit != strlSplits.end(); itSplit++)
        {
            if((*itSplit).startsWith(strColumn))
            {
                strValue = (*itSplit).section("=", 1, 1);
                return true;
            }
        }
        return false;
    }
    if(pWyrFormatColumn->m_strDataType.toLower().startsWith("parts_shipped"))
    {
        strValue = QString::number(clGexDbQuery.value(25).toUInt());
        return true;
    }
    if((m_strTablePrefix == "wt_") && (pWyrFormatColumn->m_strDataType.toLower().startsWith("wafer_id")))
    {
        strValue = clGexDbQuery.value(26).toString();
        return true;
    }

    return false;
}

//////////////////////////////////////////////////////////////////////
// Computes data for WYR - Standard report (given filters on several fields)
//////////////////////////////////////////////////////////////////////
bool GexDbPlugin_Galaxy::GetDataForWyr_Standard(
        GexDbPlugin_Filter & cFilters,
        GexDbPlugin_WyrData & cWyrData)
{
    // First check if GEXDB is up-to-date
    QString			strGexDbVersion, strVersionSupportedByPlugin;
    unsigned int	uiGexDbBuild, uiBuildSupportedByPlugin;
    bool			bDbUpToDate;
    if(!IsDbUpToDateForExtraction(&bDbUpToDate, strGexDbVersion, &uiGexDbBuild, strVersionSupportedByPlugin, &uiBuildSupportedByPlugin))
    {
        strGexDbVersion = GGET_LASTERRORMSG(GexDbPlugin_Base, this);
        GSET_ERROR1(GexDbPlugin_Base, eDB_CheckDbVersion, NULL, strGexDbVersion.toLatin1().constData());
        return false;
    }
    // Allow extraction if compatible
    if(!bDbUpToDate && !IsCompatibleForStdExtraction(uiGexDbBuild, uiBuildSupportedByPlugin))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_VersionMismatch, NULL, strGexDbVersion.toLatin1().constData(), strVersionSupportedByPlugin.toLatin1().constData());
        return false;
    }

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
        return false;

    m_strTestingStage = cFilters.strDataTypeQuery;

    // Check database connection
    if(!ConnectToCorporateDb())
        return false;

    // Get WYR formats
    QString strQuery, strToken, strField, strTable;
    strToken = m_strTablePrefix;
    strToken += "wyr_format.site_name";
    Query_NormalizeToken(strToken, strField, strTable);
    strQuery = "SELECT t.site_name, t.column_id, t.column_nb, t.column_name, t.data_type FROM ";
    strQuery += strTable;
    strQuery += " t WHERE t.column_id >-1 AND NOT t.data_type IS NULL AND lower(t.display)='y' ORDER BY site_name,column_nb";

    // Execute query
    GexDbPlugin_Query clGexDbQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    clGexDbQuery.setForwardOnly(true);
    if(!clGexDbQuery.Execute(strQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        return false;
    }

    // Go through all retrieved formats
    QString									strSiteName, strCurrentSiteName;
    GexDbPlugin_WyrFormat					*pWyrFormat=NULL;
    QHash<QString,GexDbPlugin_WyrFormat*>	cWyrFormatDict;
    int										nIndex;
    unsigned int							uiColumnId, uiColumnNb;
    QString									strColumnName, strDataType;

    while(clGexDbQuery.Next())
    {
        // Get query data
        nIndex = 0;
        strSiteName = clGexDbQuery.value(nIndex++).toString();
        if(strCurrentSiteName.isEmpty())
            strCurrentSiteName = strSiteName;
        if((strSiteName != strCurrentSiteName) && pWyrFormat && (pWyrFormat->count() > 0))
        {
            cWyrFormatDict.insert(strCurrentSiteName, pWyrFormat);
            strCurrentSiteName = strSiteName;
            pWyrFormat = NULL;
        }
        uiColumnId = clGexDbQuery.value(nIndex++).toUInt();
        uiColumnNb = clGexDbQuery.value(nIndex++).toUInt();
        strColumnName = clGexDbQuery.value(nIndex++).toString();
        strDataType = clGexDbQuery.value(nIndex++).toString();

        // Add column
        if(pWyrFormat == NULL)
            pWyrFormat = new GexDbPlugin_WyrFormat;
        pWyrFormat->append(new GexDbPlugin_WyrFormatColumn(strSiteName, uiColumnId, uiColumnNb, strColumnName, strDataType));
    }
    if(pWyrFormat && (pWyrFormat->count() > 0))
    {
        cWyrFormatDict.insert(strCurrentSiteName, pWyrFormat);
        pWyrFormat = NULL;
    }

    // Retrieve WYR data
    // Clear query string
    strQuery = "";

    // Construct query string:
    // SELECT <fields>
    // FROM <all required tables>
    // WHERE <link conditions> AND <value conditions>	(optional)
    // SELECT: add query fields
    QString strFieldSpec;
    Query_Empty();
    strFieldSpec = "Field|";
    strFieldSpec += m_strTablePrefix;
    m_strlQuery_Fields.append(strFieldSpec + "wyr.site_name");
    m_strlQuery_Fields.append(strFieldSpec + "wyr.week_nb");
    m_strlQuery_Fields.append(strFieldSpec + "wyr.year");
    m_strlQuery_Fields.append(strFieldSpec + "wyr.date_in");
    m_strlQuery_Fields.append(strFieldSpec + "wyr.date_out");
    m_strlQuery_Fields.append(strFieldSpec + "wyr.product_name");
    m_strlQuery_Fields.append(strFieldSpec + "wyr.program_name");
    m_strlQuery_Fields.append(strFieldSpec + "wyr.tester_name");
    m_strlQuery_Fields.append(strFieldSpec + "wyr.lot_id");
    m_strlQuery_Fields.append(strFieldSpec + "wyr.subcon_lot_id");
    m_strlQuery_Fields.append(strFieldSpec + "wyr.user_split");
    m_strlQuery_Fields.append(strFieldSpec + "wyr.yield");
    m_strlQuery_Fields.append(strFieldSpec + "wyr.parts_received");
    m_strlQuery_Fields.append(strFieldSpec + "wyr.pretest_rejects");
    m_strlQuery_Fields.append(strFieldSpec + "wyr.pretest_rejects_split");
    m_strlQuery_Fields.append(strFieldSpec + "wyr.parts_tested");
    m_strlQuery_Fields.append(strFieldSpec + "wyr.parts_pass");
    m_strlQuery_Fields.append(strFieldSpec + "wyr.parts_pass_split");
    m_strlQuery_Fields.append(strFieldSpec + "wyr.parts_fail");
    m_strlQuery_Fields.append(strFieldSpec + "wyr.parts_fail_split");
    m_strlQuery_Fields.append(strFieldSpec + "wyr.parts_retest");
    m_strlQuery_Fields.append(strFieldSpec + "wyr.parts_retest_split");
    m_strlQuery_Fields.append(strFieldSpec + "wyr.insertions");
    m_strlQuery_Fields.append(strFieldSpec + "wyr.posttest_rejects");
    m_strlQuery_Fields.append(strFieldSpec + "wyr.posttest_rejects_split");
    m_strlQuery_Fields.append(strFieldSpec + "wyr.parts_shipped");
    if(m_strTablePrefix == "wt_")
        m_strlQuery_Fields.append(strFieldSpec + "wyr.wafer_id");

    // Add fields for the ORDER BY instruction
    m_strlQuery_OrderFields.append(m_strTablePrefix + "wyr.site_name|ASC");
    m_strlQuery_OrderFields.append(m_strTablePrefix + "wyr.week_nb|ASC");
    m_strlQuery_OrderFields.append(m_strTablePrefix + "wyr.year|ASC");
    m_strlQuery_OrderFields.append(m_strTablePrefix + "wyr.lot_id|ASC");
    if(m_strTablePrefix == "wt_")
        m_strlQuery_OrderFields.append(m_strTablePrefix + "wyr.wafer_id|ASC");
    else
        m_strlQuery_OrderFields.append(m_strTablePrefix + "wyr.subcon_lot_id|ASC");

    //////////////////////////////////////////
    // Exit status
    bool						bStatus = false;
    //////////////////////////////////////////
    QString						strDataset, strCurrentDataset="", strDataRow, strValue;
    unsigned int				uiWeekNb, uiYear;
    GexDbPlugin_WyrDataset		*pWyrDataset=NULL;
    GexDbPlugin_WyrFormatColumn	*pWyrFormatColumn;

    // Set filters
    if(!Query_AddFilters(cFilters))
        goto labelExit;

    // Construct query from table and conditions
    Query_BuildSqlString(strQuery, false);

    // Execute query
    clGexDbQuery.setForwardOnly(true);
    if(!clGexDbQuery.Execute(strQuery))
    {
        GSET_ERROR2(GexDbPlugin_Base, eDB_Query, NULL, strQuery.toLatin1().constData(), clGexDbQuery.lastError().text().toLatin1().constData());
        goto labelExit;
    }

    // Go through all retrieved data lines
    while(clGexDbQuery.Next())
    {
        // Get query data
        nIndex = 0;
        strSiteName = clGexDbQuery.value(nIndex++).toString();
        uiWeekNb = clGexDbQuery.value(nIndex++).toUInt();
        uiYear = clGexDbQuery.value(nIndex++).toUInt();
        strDataset = strSiteName + "|";
        strDataset += QString::number(uiWeekNb) + "|";
        strDataset += QString::number(uiYear);

        if(strCurrentDataset.isEmpty())
        {
            strCurrentDataset = strDataset;

            if (cWyrFormatDict.find(strSiteName) != cWyrFormatDict.end())
                pWyrFormat = cWyrFormatDict.find(strSiteName).value();
            else if (cWyrFormatDict.find("default") != cWyrFormatDict.end())
                pWyrFormat = cWyrFormatDict.find("default").value();
            else
            {
                GSLOG(SYSLOG_SEV_ERROR, "No WYR format found!");
                GEX_ASSERT(false);
                pWyrFormat = NULL;
            }

            if(pWyrFormat)
            {
                pWyrDataset = new GexDbPlugin_WyrDataset(strSiteName, uiYear, uiWeekNb);
                //                for(pWyrFormatColumn = pWyrFormat->first(); pWyrFormatColumn; pWyrFormatColumn = pWyrFormat->next())
                foreach(pWyrFormatColumn, *pWyrFormat)
                {
                    if(!pWyrDataset->m_strTitleRow.isEmpty())
                        pWyrDataset->m_strTitleRow += ",";
                    pWyrDataset->m_strTitleRow += pWyrFormatColumn->m_strColumnName;
                    if(pWyrFormatColumn->m_strDataType.toLower().startsWith("date_"))
                        pWyrDataset->m_strTitleRow += " (dd/mm/yyyy)";
                }
            }
            else
            {
                pWyrDataset = new GexDbPlugin_WyrDataset(strSiteName, uiYear, uiWeekNb);
                pWyrDataset->m_strTitleRow = "ERROR: no WYR format found for site ";
                pWyrDataset->m_strTitleRow += strSiteName;
                pWyrDataset->m_strTitleRow += " in table ";
                pWyrDataset->m_strTitleRow += m_strTablePrefix;
                pWyrDataset->m_strTitleRow += "wyr_format!!";
                continue;
            }
        }
        else if((strDataset != strCurrentDataset) && (pWyrDataset->m_strlDataRows.count() > 0))
        {
            cWyrData.append(pWyrDataset);
            strCurrentDataset = strDataset;

            if (cWyrFormatDict.find(strSiteName) != cWyrFormatDict.end())
                pWyrFormat = cWyrFormatDict.find(strSiteName).value();
            else if (cWyrFormatDict.find("default") != cWyrFormatDict.end())
                pWyrFormat = cWyrFormatDict.find("default").value();
            else
            {
                GSLOG(SYSLOG_SEV_ERROR, "No WYR format found!");
                GEX_ASSERT(false);
                pWyrFormat = NULL;
            }

            if(pWyrFormat)
            {
                pWyrDataset = new GexDbPlugin_WyrDataset(strSiteName, uiYear, uiWeekNb);
                //                for(pWyrFormatColumn = pWyrFormat->first(); pWyrFormatColumn; pWyrFormatColumn = pWyrFormat->next())
                foreach(pWyrFormatColumn, *pWyrFormat)
                {
                    if(!pWyrDataset->m_strTitleRow.isEmpty())
                        pWyrDataset->m_strTitleRow += ",";
                    pWyrDataset->m_strTitleRow += pWyrFormatColumn->m_strColumnName;
                    if(pWyrFormatColumn->m_strDataType.toLower().startsWith("date_"))
                        pWyrDataset->m_strTitleRow += " (dd/mm/yyyy)";
                }
            }
            else
            {
                pWyrDataset = new GexDbPlugin_WyrDataset(strSiteName, uiYear, uiWeekNb);
                pWyrDataset->m_strTitleRow = "ERROR: no WYR format found for site ";
                pWyrDataset->m_strTitleRow += strSiteName;
                pWyrDataset->m_strTitleRow += " in table ";
                pWyrDataset->m_strTitleRow += m_strTablePrefix;
                pWyrDataset->m_strTitleRow += "wyr_format!!";
                continue;
            }
        }

        strDataRow = "";

        //        for(pWyrFormatColumn = pWyrFormat->first(); pWyrFormatColumn; pWyrFormatColumn = pWyrFormat->next())
        foreach(pWyrFormatColumn, *pWyrFormat)
        {
            Wyr_DataRow_Getvalue(strValue, pWyrFormatColumn, clGexDbQuery);
            if(!strDataRow.isEmpty())
                strDataRow += ",";
            if(strValue.isEmpty())
                strDataRow += "-";
            else
                strDataRow += strValue;
        }

        pWyrDataset->m_strlDataRows += strDataRow;
    }

    if(pWyrDataset && pWyrDataset->m_strlDataRows.count() > 0)
        cWyrData.append(pWyrDataset);

    bStatus = true;

labelExit:
    while(!cWyrFormatDict.isEmpty())
        delete cWyrFormatDict.take(cWyrFormatDict.begin().key());

    return bStatus;
}

//////////////////////////////////////////////////////////////////////
// Return mapped filter labels
//////////////////////////////////////////////////////////////////////
void GexDbPlugin_Galaxy::GetLabelFilterChoices(const QString & strDataTypeQuery, QStringList & strlLabelFilterChoices)
{
    strlLabelFilterChoices.clear();

    if(strDataTypeQuery == GEXDB_PLUGIN_GALAXY_ETEST)
        strlLabelFilterChoices = m_strlLabelFilterChoices_Et;
    if(strDataTypeQuery == GEXDB_PLUGIN_GALAXY_WTEST)
        strlLabelFilterChoices = m_strlLabelFilterChoices_Wt;
    if(strDataTypeQuery == GEXDB_PLUGIN_GALAXY_FTEST)
        strlLabelFilterChoices = m_strlLabelFilterChoices_Ft;
}

//////////////////////////////////////////////////////////////////////
// Return mapped consolidated filter labels
//////////////////////////////////////////////////////////////////////
void GexDbPlugin_Galaxy::GetConsolidatedLabelFilterChoices(const QString & strDataTypeQuery, QStringList & strlLabelFilterChoices)
{
    strlLabelFilterChoices.clear();

    if(strDataTypeQuery == GEXDB_PLUGIN_GALAXY_ETEST)
        strlLabelFilterChoices = m_strlLabelFilterChoices_Consolidated_Et;
    if(strDataTypeQuery == GEXDB_PLUGIN_GALAXY_WTEST)
        strlLabelFilterChoices = m_strlLabelFilterChoices_Consolidated_Wt;
    if(strDataTypeQuery == GEXDB_PLUGIN_GALAXY_FTEST)
        strlLabelFilterChoices = m_strlLabelFilterChoices_Consolidated_Ft;
}

//////////////////////////////////////////////////////////////////////
// Return mapped filter labels at wafer level
//////////////////////////////////////////////////////////////////////
void GexDbPlugin_Galaxy::GetLabelFilterChoicesWaferLevel(const QString & strDataTypeQuery, QStringList & strlLabelFilterChoices)
{
    strlLabelFilterChoices.clear();

    if(strDataTypeQuery == GEXDB_PLUGIN_GALAXY_ETEST)
        strlLabelFilterChoices = m_strlLabelFilterChoices_WaferLevel_Et;
    if(strDataTypeQuery == GEXDB_PLUGIN_GALAXY_WTEST)
        strlLabelFilterChoices = m_strlLabelFilterChoices_WaferLevel_Wt;
}

//////////////////////////////////////////////////////////////////////
// Return mapped filter labels at lot level
//////////////////////////////////////////////////////////////////////
void GexDbPlugin_Galaxy::GetLabelFilterChoicesLotLevel(const QString & strDataTypeQuery, QStringList & strlLabelFilterChoices)
{
    strlLabelFilterChoices.clear();

    if(strDataTypeQuery == GEXDB_PLUGIN_GALAXY_ETEST)
        strlLabelFilterChoices = m_strlLabelFilterChoices_LotLevel_Et;
    if(strDataTypeQuery == GEXDB_PLUGIN_GALAXY_WTEST)
        strlLabelFilterChoices = m_strlLabelFilterChoices_LotLevel_Wt;
    if(strDataTypeQuery == GEXDB_PLUGIN_GALAXY_FTEST)
        strlLabelFilterChoices = m_strlLabelFilterChoices_LotLevel_Ft;
}

//////////////////////////////////////////////////////////////////////
// Return field name for WaferID
//////////////////////////////////////////////////////////////////////
void GexDbPlugin_Galaxy::GetWaferIdFieldName(QString& /*strFieldName*/)
{
}

//////////////////////////////////////////////////////////////////////
// Return field name for LotID
//////////////////////////////////////////////////////////////////////
void GexDbPlugin_Galaxy::GetLotIdFieldName(QString& /*strFieldName*/)
{
}

bool GexDbPlugin_Galaxy::ExtractMultiLimits(QMap<int, QList<QVariantMap> > &multiLimits,
                                            int splitlotId,
                                            const QString& tablePrefix)
{
    if(m_strTestingStage == GEXDB_PLUGIN_GALAXY_ETEST)
        return true;

    QString lTableName = tablePrefix + "ptest_static_limits";
    QString lNormalizedTable = lTableName;
    Query_NormalizeTableName(lNormalizedTable);
    QString lQueryContent = "SELECT "
            + lTableName + ".ptest_info_id, "
            + lTableName + ".limit_id, "
            + lTableName + ".site_no, "
            + lTableName + ".hbin_no, "
            + lTableName + ".sbin_no, "
            + lTableName + ".hl, "
            + lTableName + ".ll "
                           "FROM " + lNormalizedTable + " "
                                                        "WHERE " +
            lTableName + ".splitlot_id=" + QString::number(splitlotId) + "\n";
    GexDbPlugin_Query lQuery(this, QSqlDatabase::database(m_pclDatabaseConnector->m_strConnectionName));
    if(!lQuery.Execute(lQueryContent))
    {
        // Display error
        GSET_ERROR2(GexDbPlugin_Base,
                    eDB_Query,
                    NULL,
                    lQueryContent.toLatin1().constData(),
                    lQuery.lastError().text().toLatin1().constData());
        return false;
    }

    // iterate on limits
    while (lQuery.next())
    {
        QVariantMap lMultiLimitsMap;
        int lFieldNo = lQuery.record().indexOf("ptest_info_id");
        int lTestInfoId = lQuery.value(lFieldNo).toInt();

        lMultiLimitsMap.insert("TYPE", "ML");
        lFieldNo = lQuery.record().indexOf("site_no");
        QVariant lValue = lQuery.value(lFieldNo);
        if (lValue.isValid() && !lValue.isNull())
            lMultiLimitsMap.insert("SITE", lValue);

        lFieldNo = lQuery.record().indexOf("hbin_no");
        lValue = lQuery.value(lFieldNo);
        if (lValue.isValid() && !lValue.isNull())
            lMultiLimitsMap.insert("HBIN", lValue);

        lFieldNo = lQuery.record().indexOf("sbin_no");
        lValue = lQuery.value(lFieldNo);
        if (lValue.isValid() && !lValue.isNull())
            lMultiLimitsMap.insert("SBIN", lValue);

        lFieldNo = lQuery.record().indexOf("hl");
        lValue = lQuery.value(lFieldNo);
        if (lValue.isValid() && !lValue.isNull())
            lMultiLimitsMap.insert("HL", lValue);

        lFieldNo = lQuery.record().indexOf("ll");
        lValue = lQuery.value(lFieldNo);
        if (lValue.isValid() && !lValue.isNull())
            lMultiLimitsMap.insert("LL", lValue);

        QList<QVariantMap> lLimitList;
        // If already a limits list for this test, use it
        if (multiLimits.contains(lTestInfoId))
        {
            lLimitList = multiLimits.value(lTestInfoId);
        }
        // Fill the limits list for a given test
        lLimitList.append(lMultiLimitsMap);
        multiLimits.insert(lTestInfoId, lLimitList);

    }
    return true;
}
