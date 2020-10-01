#include "gex_shared.h"
#include "db_gexdatabasequery.h"
#include <gqtl_log.h>
#include "stdio.h"

int GexDatabaseQuery::sInstances=0;

///////////////////////////////////////////////////////////
// Query structure constructor
///////////////////////////////////////////////////////////
GexDatabaseQuery::GexDatabaseQuery()
{
    sInstances++;
    clear();
}

GexDatabaseQuery::GexDatabaseQuery(const GexDatabaseQuery & other)
{
    *this = other;
}

GexDatabaseQuery &GexDatabaseQuery::operator =(const GexDatabaseQuery & other)
{
    if (this != &other)
    {
        bLocalDatabase              = other.bLocalDatabase;
        bHoldFileCopy				= other.bHoldFileCopy;
        bCompressed 				= other.bCompressed;
        bSummaryOnly				= other.bSummaryOnly;
        bBlackHole					= other.bBlackHole;
        bExternal					= other.bExternal;
        bOfflineQuery				= other.bOfflineQuery;
        bConsolidatedExtraction 	= other.bConsolidatedExtraction;

        strDatabaseLogicalName      = other.strDatabaseLogicalName;
        strDataTypeQuery            = other.strDataTypeQuery;
        calendarFrom                = other.calendarFrom;
        calendarTo                  = other.calendarTo;
        calendarFrom_Time           = other.calendarFrom_Time;
        calendarTo_Time             = other.calendarTo_Time;
        iTimePeriod                 = other.iTimePeriod;
        iTimeNFactor                = other.iTimeNFactor;
        m_eTimeStep                 = other.m_eTimeStep;
        lMinimumPartsInFile         = other.lMinimumPartsInFile;
        lfQueryFileSize             = other.lfQueryFileSize;
        iBurninTime                 = other.iBurninTime;
        strTitle                    = other.strTitle;
        strDataOrigin               = other.strDataOrigin;
        strDibName                  = other.strDibName;
        strDibType                  = other.strDibType;
        strFacilityID               = other.strFacilityID;
        strFamilyID                 = other.strFamilyID;
        strFloorID                  = other.strFloorID;
        strFrequencyStep            = other.strFrequencyStep;
        strLoadBoardName            = other.strLoadBoardName;
        strLoadBoardType            = other.strLoadBoardType;
        strLotID                    = other.strLotID;
        strOperator                 = other.strOperator;
        strPackageType              = other.strPackageType;
        strProberType               = other.strProberType;
        strProberName               = other.strProberName;
        strProductID                = other.strProductID;
        strJobName                  = other.strJobName;
        strJobRev                   = other.strJobRev;
        strSubLotID                 = other.strSubLotID;
        strTemperature              = other.strTemperature;
        strNodeName                 = other.strNodeName;
        strTesterType               = other.strTesterType;
        strTestingCode              = other.strTestingCode;
        strProcessID                = other.strProcessID;
        strWaferID                  = other.strWaferID;
        strUser1                    = other.strUser1;
        strUser2                    = other.strUser2;
        strUser3                    = other.strUser3;
        strUser4                    = other.strUser4;
        strUser5                    = other.strUser5;
        strRetestNbr                = other.strRetestNbr;
        iProcessData                = other.iProcessData;
        strProcessData              = other.strProcessData;
        iSite                       = other.iSite;
        strMapFile                  = other.strMapFile;
        uFilterFlag                 = other.uFilterFlag;
        strTestList                 = other.strTestList;
        strlSqlFilters              = other.strlSqlFilters;
        mSplitFields                = other.mSplitFields;
        mTestConditions             = other.mTestConditions;
        strOptionsString            = other.strOptionsString;
        m_gexQueries                = other.m_gexQueries;
    }

    return *this;
}

GexDatabaseQuery::~GexDatabaseQuery()
{
    sInstances--;
    //GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("deleting a GexDatabaseQuery...Still %1 instances.").arg( sInstances));
}

///////////////////////////////////////////////////////////
// Reset Query structure fields.
///////////////////////////////////////////////////////////
void GexDatabaseQuery::clear(void)
{
    uFilterFlag = 0;							// No filter at all.
    iTimePeriod = GEX_QUERY_TIMEPERIOD_TODAY;	// Date window = Today
    iTimeNFactor = 0;
    m_eTimeStep = GexDatabaseQuery::WEEKS;
    iSite = -1;							// No site filtered.
    strWaferID = "";					// Clear WaferID filter.
    lMinimumPartsInFile=0;				// Import all files, no matter how many samples found.
    lfQueryFileSize = 0;				// Amount of disk space occuied by all files matching the query.
    strRetestNbr = "";					// Retest number (0 = first test, 1 = retest#1, etc...)
    strTestList = "";					// Test list filter (only used in RemoteDB query mode).
    strOptionsString = "";				// Remote DB only: plugin options string
    bOfflineQuery = false;				// set to true if query over cache instead of database
    bConsolidatedExtraction = false;	// set to true for consolidation extraction

    m_gexQueries.clear();
    mSplitFields.clear();
    mTestConditions.clear();

    iBurninTime=0;
    strDataOrigin="";
    strDibName="";
    strDibType="";
    strFacilityID="";
    strFamilyID="";
    strFloorID="";
    strFrequencyStep="";
    strLoadBoardName="";
    strLoadBoardType="";
    strLotID="";				// Filter: LotID
    strOperator="";
    strPackageType="";
    strProberType="";
    strProberName="";
    strProductID="";
    strJobName="";
    strJobRev="";
    strSubLotID="";				// Filter: SubLotID
    strTemperature="";
    strNodeName="";
    strTesterType="";
    strTestingCode="";
    strProcessID="";
    strWaferID="";
    strUser1="";
    strUser2="";
    strUser3="";
    strUser4="";
    strUser5="";
    strProcessData="";			// List of parts/bins to process/exclude
    strMapFile="";				// Mapping file.
    uFilterFlag = 0;			// Bits set for filter flags (eg:GEX_QUERY_FLAG_FLOOR | GEX_QUERY_FLAG_PROBERNAME)
    strlSqlFilters.clear();
}

////////////////////////////////////////////////////////////////////////////////
//------- Extract FILTER name + value  --------
////////////////////////////////////////////////////////////////////////////////
bool GexDatabaseQuery::setQueryFilter(const QString& field, const QString& value)
{
    // Burning time filter
    if(!qstricmp(field.toLatin1().constData(),"dbf_burnin_time"))
    {
        sscanf(value.toLatin1().constData(), "%d", &iBurninTime);
        uFilterFlag |= GEX_QUERY_FLAG_BURNIN;
        return true;
    }
    // Data origin filter
    if(!qstricmp(field.toLatin1().constData(),"dbf_data_origin"))
    {
        strDataOrigin = value;
        uFilterFlag |= GEX_QUERY_FLAG_ORIGIN;
        return true;
    }
    // Facility location filter
    if(!qstricmp(field.toLatin1().constData(),"dbf_facility_id"))
    {
        strFacilityID = value;
        uFilterFlag |= GEX_QUERY_FLAG_FACILITY;
        return true;
    }
    // FamilyID filter
    if(!qstricmp(field.toLatin1().constData(),"dbf_family_id"))
    {
        strFamilyID = value;
        uFilterFlag |= GEX_QUERY_FLAG_FAMILY;
        return true;
    }
    // Floor Location filter
    if(!qstricmp(field.toLatin1().constData(),"dbf_floor_id"))
    {
        strFloorID = value;
        uFilterFlag |= GEX_QUERY_FLAG_FLOOR;
        return true;
    }
    // Testing/step frequency filter
    if(!qstricmp(field.toLatin1().constData(),"dbf_freq_id"))
    {
        strFrequencyStep = value;
        uFilterFlag |= GEX_QUERY_FLAG_FREQUENCYSTEP;
        return true;
    }
    // LoadBoard name filter
    if(!qstricmp(field.toLatin1().constData(),"dbf_loadboard_id"))
    {
        strLoadBoardName = value;
        uFilterFlag |= GEX_QUERY_FLAG_LOADBOARDNAME;
        return true;
    }
    // LoadBoard type (brand/family) filter
    if(!qstricmp(field.toLatin1().constData(),"dbf_loadboard_type"))
    {
        strLoadBoardType = value;
        uFilterFlag |= GEX_QUERY_FLAG_LOADBOARDTYPE;
        return true;
    }
    // DIB name filter
    if(!qstricmp(field.toLatin1().constData(),"dbf_dib_id"))
    {
        strDibName = value;
        uFilterFlag |= GEX_QUERY_FLAG_DIBNAME;
        return true;
    }
    // DIB type (brand/family) filter
    if(!qstricmp(field.toLatin1().constData(),"dbf_dib_type"))
    {
        strDibType = value;
        uFilterFlag |= GEX_QUERY_FLAG_DIBTYPE;
        return true;
    }
    // Lot ID filter
    if(!qstricmp(field.toLatin1().constData(),"dbf_lot_id"))
    {
        strLotID = value;
        uFilterFlag |= GEX_QUERY_FLAG_LOT;
        return true;
    }
    // Operator name filter
    if(!qstricmp(field.toLatin1().constData(),"dbf_operator_name"))
    {
        strOperator = value;
        uFilterFlag |= GEX_QUERY_FLAG_OPERATOR;
        return true;
    }
    // Package type filter
    if(!qstricmp(field.toLatin1().constData(),"dbf_package_type"))
    {
        strPackageType = value;
        uFilterFlag |= GEX_QUERY_FLAG_PACKAGE;
        return true;
    }
    // Prober name filter
    if(!qstricmp(field.toLatin1().constData(),"dbf_prober_id"))
    {
        strProberName = value;
        uFilterFlag |= GEX_QUERY_FLAG_PROBERNAME;
        return true;
    }
    // Prober type (brand) filter
    if(!qstricmp(field.toLatin1().constData(),"dbf_prober_type"))
    {
        strProberType = value;
        uFilterFlag |= GEX_QUERY_FLAG_PROBERTYPE;
        return true;
    }
    // Product name filter
    if(!qstricmp(field.toLatin1().constData(),"dbf_product_id"))
    {
        strProductID = value;
        uFilterFlag |= GEX_QUERY_FLAG_PRODUCT;
        return true;
    }
    // Program name filter
    if(!qstricmp(field.toLatin1().constData(),"dbf_program_name"))
    {
        strJobName = value;
        uFilterFlag |= GEX_QUERY_FLAG_PROGRAMNAME;
        return true;
    }
    // Program revision filter
    if(!qstricmp(field.toLatin1().constData(),"dbf_program_rev"))
    {
        strJobRev = value;
        uFilterFlag |= GEX_QUERY_FLAG_PROGRAMREVISION;
        return true;
    }
    // Sublot ID filter
    if(!qstricmp(field.toLatin1().constData(),"dbf_retest_nbr"))
    {
        strRetestNbr = value;
        uFilterFlag |= GEX_QUERY_FLAG_RETESTNBR;
        return true;
    }
    // Sublot ID filter
    if(!qstricmp(field.toLatin1().constData(),"dbf_sublot_id"))
    {
        strSubLotID = value;
        uFilterFlag |= GEX_QUERY_FLAG_SUBLOT;
        return true;
    }
    // Testing temperature filter
    if(!qstricmp(field.toLatin1().constData(),"dbf_temperature"))
    {
        strTemperature = value;
        uFilterFlag |= GEX_QUERY_FLAG_TEMPERATURE;
        return true;
    }
    // Tester name filter
    if(!qstricmp(field.toLatin1().constData(),"dbf_tester_name"))
    {
        strNodeName = value;
        uFilterFlag |= GEX_QUERY_FLAG_TESTERNAME;
        return true;
    }
    // Tester type (brand/family) filter
    if(!qstricmp(field.toLatin1().constData(),"dbf_tester_type"))
    {
        strTesterType = value;
        uFilterFlag |= GEX_QUERY_FLAG_TESTERTYPE;
        return true;
    }
    // Testing code filter
    if(!qstricmp(field.toLatin1().constData(),"dbf_testing_code"))
    {
        strTestingCode = value;
        uFilterFlag |= GEX_QUERY_FLAG_TESTCODE;
        return true;
    }
    // Process name filter
    if(!qstricmp(field.toLatin1().constData(),"dbf_process_id"))
    {
        strProcessID = value;
        uFilterFlag |= GEX_QUERY_FLAG_PROCESS;
        return true;
    }
    // Tester Hardware testing site# filter
    if(!qstricmp(field.toLatin1().constData(),"dbf_site_nbr"))
    {
        iSite = -1;
        int r=sscanf(value.toLatin1().constData(), "%d", &iSite);
        if (r==EOF)
            GSLOG(SYSLOG_SEV_ERROR, "can't interpret 'dbf_site_nbr'");
        uFilterFlag |= GEX_QUERY_FLAG_SITENBR;
        return true;
    }

    // WaferID name filter
    if(!qstricmp(field.toLatin1().constData(),"dbf_wafer_id"))
    {
        strWaferID = value;
        uFilterFlag |= GEX_QUERY_FLAG_WAFERID;
        return true;
    }

    // User1 name filter
    if(!qstricmp(field.toLatin1().constData(),"dbf_user_1"))
    {
        strUser1 = value;
        uFilterFlag |= GEX_QUERY_FLAG_USER1;
        return true;
    }

    // User2 name filter
    if(!qstricmp(field.toLatin1().constData(),"dbf_user_2"))
    {
        strUser2 = value;
        uFilterFlag |= GEX_QUERY_FLAG_USER2;
        return true;
    }

    // User3 name filter
    if(!qstricmp(field.toLatin1().constData(),"dbf_user_3"))
    {
        strUser3 = value;
        uFilterFlag |= GEX_QUERY_FLAG_USER3;
        return true;
    }

    // User4 name filter
    if(!qstricmp(field.toLatin1().constData(),"dbf_user_4"))
    {
        strUser4 = value;
        uFilterFlag |= GEX_QUERY_FLAG_USER4;
        return true;
    }

    // User5 name filter
    if(!qstricmp(field.toLatin1().constData(),"dbf_user_5"))
    {
        strUser5 = value;
        uFilterFlag |= GEX_QUERY_FLAG_USER5;
        return true;
    }

    // Invalid filter name
    return false;
}
