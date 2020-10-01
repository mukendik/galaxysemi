//////////////////////////////////////////////////////////////////////
// Converts a STDF file to CSV
//////////////////////////////////////////////////////////////////////

#include <gqtl_log.h>
#include "csv_converter_v1x.h"
#include "engine.h"
#include "gex_report.h"
#include "stdfrecords_v4.h"
#include "stdf_head_and_site_number_decipher.h"

// Output File format:
// <Header>
// Parameter , <Parm1>,<Parm2>, ..., <ParmN>, [gex_bin], [gex_time]
// Tests# , 1000,2000,3000,.....,4000,5000,6000
// Unit, <unit1>,<unit1>,..., <unitN>
// HighL, <UpperLimit1>, <UpperLimit2>,..., <UpperLimitN>
// LowL, <LowerLimit1>, <LowerLimit2>,..., <LowerLimitN>
// <Part#>, <Result1>, <Result2>,..., <ResultN>
// ...
// <Header>
// Parameter , <Parm1>,<Parm2>, ..., <ParmN>, [gex_bin], [gex_time]
// ...

// report_build.h
extern CReportOptions	ReportOptions;	// Holds option
extern CGexReport*    gexReport;  // Handle to report class

CsvConverterV1x::CsvConverterV1x(CsvV1Minor minorVersion)
    : AbstractCsvConverter(GALAXY_CSV_VERSION_V1_MAJOR, minorVersion)
{
}

void CsvConverterV1x::Clear(void)
{
    AbstractCsvConverter::Clear();
    m_cTestFlow.clear();
}

CsvConverterV1x::~CsvConverterV1x()
{
}

/////////////////////////////////////////////////////////////////////////////
// Write Results header
/////////////////////////////////////////////////////////////////////////////
void	CsvConverterV1x::WriteResultsHeader(void)
{
    if(lPass != 2)
        return;

    // assume that result header is written once
    if(!m_bIsResultsHeaderWritten)
        m_bIsResultsHeaderWritten = true;
    else
        return;

    // Write list of tests (Parameters names)
    hCsvTableFile << endl << "Parameter, ";

    // Internal parameters: binning, Die location, Exec Time, etc...
    hCsvTableFile << "SBIN,HBIN,DIE_X,DIE_Y,SITE,TIME,TOTAL_TESTS,LOT_ID,WAFER_ID,";

    // Custom variables
    // Dump all field names
    GSLOG(SYSLOG_SEV_NOTICE,
           QString("Found %1 valid DTR with <@field=value>").arg(m_mapVariableValue.count())
          .toLatin1().data() );

    foreach(QString key, m_mapVariableValue.keys())
        hCsvTableFile << key.section("#SEQ#",1) << ",";

    // See test order defined: sort by test flow ID, or sort by test#...
    if (m_eSortingField == SortOnTestID)
        m_cFullTestFlow.clear();	// Sorting is by test#....so clear FlowID if any exists!

    // If No full flow known (meaning no Bin1 found), then use Test# sorting order!
    CShortTest *ptTestCell;
    if((m_cFullTestFlow.count() == 0) && (ptTestList != NULL))
    {
        // Fill FlowList with Test list!
        ptTestCell = ptTestList;
        while(ptTestCell != NULL)
        {
            // Insert TestCell pointer
            m_cFullTestFlow.append(ptTestCell);

            // Move to next cell available
            ptTestCell = ptTestCell->ptNextTest;
        };
    }

    // Write test name as found in flow.
    // List all tests executed (in same order as data flow) in this run & test_site
    cTestList::iterator it;

//    QString strString;
//    QChar * ptChar;
//    int		iNewLength;
    for(it = m_cFullTestFlow.begin(); it != m_cFullTestFlow.end(); ++it )
    {
        ptTestCell = *it;

        // If ptResultArrayIndexes is not null, this is a Master Multiple Parametric Record.
        // Don't list it
        if (ptTestCell->mResultArrayIndexes.isEmpty())
        {
            hCsvTableFile << ptTestCell->strTestName << ",";
        }

        ptTestCell = ptTestCell->ptNextTest;
    };
    hCsvTableFile << endl;

    // Write list of tests#
    hCsvTableFile << "Tests#,";

    // Internal parameters: binning, Die location, Exec Time, etc...
    hCsvTableFile << ",,,,,,,,,";

    // Custom variables
    // Add separator
    for(int index=0; index < m_mapVariableValue.count(); index++)
        hCsvTableFile << ",";


    for(it = m_cFullTestFlow.begin(); it != m_cFullTestFlow.end(); ++it )
    {
        ptTestCell = *it;

        // If ptResultArrayIndexes is not null, this is a Master Multiple Parametric Record.
        // Don't list it
        if (ptTestCell->mResultArrayIndexes.isEmpty())
        {
            // Write Test#
            hCsvTableFile << ptTestCell->lTestNumber;
            // Write Pinmap Index if any (for Multi-parametric tests)
            if(ptTestCell->lPinmapIndex != GEX_PTEST)
                hCsvTableFile << "." << ptTestCell->lPinmapIndex;
            hCsvTableFile << ",";
        }

        ptTestCell = ptTestCell->ptNextTest;
    };
    hCsvTableFile << endl;

    // Write list of tests UNITS
    hCsvTableFile << "Unit,";
    // Skip Internal parameters: binning, Die location, etc...
    hCsvTableFile << ",,,,,sec.,,,,";

    // Custom variables
    // Add separator
    for(int index=0; index < m_mapVariableValue.count(); index++)
        hCsvTableFile << ",";

    for(it = m_cFullTestFlow.begin(); it != m_cFullTestFlow.end(); ++it )
    {
        ptTestCell = *it;

        // If ptResultArrayIndexes is not null, this is a Master Multiple Parametric Record.
        // Don't list it
        if (ptTestCell->mResultArrayIndexes.isEmpty())
            hCsvTableFile << ptTestCell->szTestUnits << ",";

        ptTestCell = ptTestCell->ptNextTest;
    };
    hCsvTableFile << endl;

    // Write list of tests UPPER Spec Limits
    hCsvTableFile << "HighL,";
    // Skip Internal parameters: binning, Die location, etc...
    hCsvTableFile << ",,,,,,,,,";

    // Custom variables
    // Add separator
    for(int index=0; index < m_mapVariableValue.count(); index++)
        hCsvTableFile << ",";

    for(it = m_cFullTestFlow.begin(); it != m_cFullTestFlow.end(); ++it )
    {
        ptTestCell = *it;

        if (ptTestCell->mResultArrayIndexes.isEmpty())
        {
            if((ptTestCell->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0)
                hCsvTableFile << ptTestCell->lfHighLimit;
            hCsvTableFile << ",";
        }

        ptTestCell = ptTestCell->ptNextTest;
    };
    hCsvTableFile << endl;

    // Write list of tests LOWER Spec Limits
    hCsvTableFile << "LowL,";
    // Skip Internal parameters: binning, Die location, etc...
    hCsvTableFile << ",,,,,,,,,";

    // Custom variables
    // Add separator
    for(int index=0; index < m_mapVariableValue.count(); index++)
        hCsvTableFile << ",";

    for(it = m_cFullTestFlow.begin(); it != m_cFullTestFlow.end(); ++it )
    {
        ptTestCell = *it;

        if (ptTestCell->mResultArrayIndexes.isEmpty())
        {
            if((ptTestCell->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0)
                hCsvTableFile << ptTestCell->lfLowLimit;
            hCsvTableFile << ",";
        }

        ptTestCell = ptTestCell->ptNextTest;
    };
    hCsvTableFile << endl;
}

/////////////////////////////////////////////////////////////////////////////
// Read STDF.MIR record, dump it to CSV file...
/////////////////////////////////////////////////////////////////////////////
bool	CsvConverterV1x::ProcessMIR()	// Extract MIR data
{
    BYTE	bStation;
    char	szIgnore[MIR_STRING_SIZE];
    char	szString[MIR_STRING_SIZE];
    char	szPartType[MIR_STRING_SIZE];
    char	szNodeName[MIR_STRING_SIZE];
    char	szTesterType[MIR_STRING_SIZE];
    char	szJobName[MIR_STRING_SIZE];
    char	szJobRev[MIR_STRING_SIZE];
    char	szSubLot[MIR_STRING_SIZE];
    char	szOperator[MIR_STRING_SIZE];
    char	szExecType[MIR_STRING_SIZE];
    char	szExecVer[MIR_STRING_SIZE];
    char	szTestCode[MIR_STRING_SIZE];
    char	szTemperature[MIR_STRING_SIZE];
    char	szPackageType[MIR_STRING_SIZE];
    char	szFamilyID[MIR_STRING_SIZE];
    char	szFacilityID[MIR_STRING_SIZE];
    char	szFloorID[MIR_STRING_SIZE];
    char	szProcessID[MIR_STRING_SIZE];
    char	szFrequencyStep[MIR_STRING_SIZE];
    char	szSpecName[MIR_STRING_SIZE];
    char	szSpecVersion[MIR_STRING_SIZE];
    char	szFlowID[MIR_STRING_SIZE];
    char	szSetupID[MIR_STRING_SIZE];
    char	szDesignRev[MIR_STRING_SIZE];
    char	szEngID[MIR_STRING_SIZE];
    char	szRomCod[MIR_STRING_SIZE];
    char	szSerielNum[MIR_STRING_SIZE];
    char	szSuperName[MIR_STRING_SIZE];
    unsigned char   cModeCode;
    unsigned char   cRtstCode;
    unsigned char   cProtCode;
    int             iBurnTime = 65535;
    unsigned char   cCmodeCode;

        // MIR found, get data!
        switch(StdfRecordHeader.iStdfVersion)
        {
          case GEX_STDFV4:
            StdfFile.ReadDword((long *)&m_cFileData.m_tSetupTime);			// Setup_T
            StdfFile.ReadDword((long *)&m_cFileData.m_tStartTime);			// Start_T
            StdfFile.ReadByte(&bStation);		// stat #
            StdfFile.ReadByte(&cModeCode);		// mode_code
            StdfFile.ReadByte(&cRtstCode);		// rtst_code
            StdfFile.ReadByte(&cProtCode);		// prot_cod #
            StdfFile.ReadWord(&iBurnTime);		// burn_time
            StdfFile.ReadByte(&cCmodeCode);		// cmode_code
            ReadStringToField(szString);
            m_cFileData.m_strLotID = szString;					// Dataset Lot #

            ReadStringToField(szPartType);
            ReadStringToField(szNodeName);
            ReadStringToField(szTesterType);
            ReadStringToField(szJobName);
            ReadStringToField(szJobRev);
            ReadStringToField(szSubLot);
            ReadStringToField(szOperator);
            ReadStringToField(szExecType);
            ReadStringToField(szExecVer);
            ReadStringToField(szTestCode);		// test-cod
            ReadStringToField(szTemperature);	// test-temperature
            ReadStringToField(szIgnore);		// user-txt
            ReadStringToField(szIgnore);		// aux-file
            ReadStringToField(szPackageType);	// package-type
            ReadStringToField(szFamilyID);		// familyID
            ReadStringToField(szIgnore);		// Date-code
            ReadStringToField(szFacilityID);	// Facility-ID
            ReadStringToField(szFloorID);		// FloorID
            ReadStringToField(szProcessID);		// ProcessID
            ReadStringToField(szFrequencyStep);	// Frequency/Step

            ReadStringToField(szSpecName);		// SPEC_NAM:Test specification name
            ReadStringToField(szSpecVersion);	// SPEC_VER:Test specification name
            ReadStringToField(szFlowID);		// FLOW_ID:
            ReadStringToField(szSetupID);		// SETUP_ID:
            ReadStringToField(szDesignRev);		// DSGN_REV:
            ReadStringToField(szEngID);			// ENG_ID:
            ReadStringToField(szRomCod);		// ROM_COD:
            ReadStringToField(szSerielNum);		// SERL_NUM:
            ReadStringToField(szSuperName);		// SUPR_NAM:
            break;

          default :
            break;
        }

    // Don't write anything from MIR during Pass#1!
    if(lPass != 2)
        return true;

    // CSV Header comment.
    hCsvTableFile << "# Semiconductor Yield Analysis is easy with Quantix! " << endl;
    hCsvTableFile << "# Check latest news: www.mentor.com " << endl;
    hCsvTableFile << "# Created by: "
      << GS::Gex::Engine::GetInstance().Get("AppFullName").toString() << endl;
    hCsvTableFile << "# Examinator Data File: Edit/Add/Remove any data you want! " << endl << endl;

    // Fills CSV file with MIR data read.
    QDateTime CsvDateTime;
    // Set QDateTime object to UTC mode
    CsvDateTime.setTimeSpec(Qt::UTC);

    CsvDateTime.setTime_t(m_cFileData.m_tStartTime);
    hCsvTableFile << "Date," << CsvDateTime.toString("yyyy_MM_dd hh:mm:ss")  << endl;

    CsvDateTime.setTime_t(m_cFileData.m_tSetupTime);
    if(m_cFileData.m_tSetupTime)
        hCsvTableFile << "SetupTime," << CsvDateTime.toString("yyyy_MM_dd hh:mm:ss")  << endl;
    CsvDateTime.setTime_t(m_cFileData.m_tStartTime);
    if(m_cFileData.m_tStartTime)
        hCsvTableFile << "StartTime," << CsvDateTime.toString("yyyy_MM_dd hh:mm:ss")  << endl;
    CsvDateTime.setTime_t(m_cFileData.m_tEndTime);
    if(m_cFileData.m_tEndTime)
        hCsvTableFile << "FinishTime," << CsvDateTime.toString("yyyy_MM_dd hh:mm:ss")  << endl;

    if(*szJobName)
        hCsvTableFile << "ProgramName," << szJobName << endl;
    if(*szJobRev)
        hCsvTableFile << "ProgramRevision," << szJobRev << endl;
    if(m_cFileData.m_strLotID.isEmpty() == false)
        hCsvTableFile << "Lot," << m_cFileData.m_strLotID << endl;
    if(*szSubLot)
        hCsvTableFile << "SubLot," << szSubLot << endl;
    if(m_cFileData.m_strWaferID.isEmpty() == false)
        hCsvTableFile << "Wafer," << m_cFileData.m_strWaferID << endl;
    if(m_cFileData.m_strWaferOrientation.isEmpty() == false)
        hCsvTableFile << "WaferOrientation," << m_cFileData.m_strWaferOrientation << endl;

    if(*szNodeName)
        hCsvTableFile << "TesterName," << szNodeName << endl;
    if(*szTesterType)
        hCsvTableFile << "TesterType," << szTesterType << endl;
    if(*szPartType)
        hCsvTableFile << "Product," << szPartType << endl;
    if(*szOperator)
        hCsvTableFile << "Operator," << szOperator << endl;
    if(*szExecType)
        hCsvTableFile << "ExecType," << szExecType << endl;
    if(*szExecVer)
        hCsvTableFile << "ExecRevision," << szExecVer << endl;
    if(*szTestCode)
        hCsvTableFile << "TestCode," << szTestCode << endl;

    if(cModeCode != ' ')
        hCsvTableFile << "ModeCode," << QString(cModeCode) << endl;
    if(cRtstCode != ' ')
        hCsvTableFile << "RtstCode," << QString(cRtstCode) << endl;
    if(cProtCode != ' ')
        hCsvTableFile << "ProtCode," << QString(cProtCode) << endl;
    if(cCmodeCode != ' ')
        hCsvTableFile << "CmodeCode," << QString(cCmodeCode) << endl;
    if((unsigned int) iBurnTime != 65535)
        hCsvTableFile << "BurnTime," << QString::number(iBurnTime) << endl;

    if(*szTemperature)
        hCsvTableFile << "Temperature," << szTemperature << endl;
    if(*szPackageType)
        hCsvTableFile << "PackageType," << szPackageType << endl;
    if(*szFamilyID)
        hCsvTableFile << "Family," << szFamilyID << endl;
    if(*szFacilityID)
        hCsvTableFile << "Facility," << szFacilityID << endl;
    if(*szFloorID)
        hCsvTableFile << "Floor," << szFloorID << endl;
    if(*szProcessID)
        hCsvTableFile << "Process," << szProcessID << endl;
    if(*szFrequencyStep)
        hCsvTableFile << "FreqStep," << szFrequencyStep << endl;

    if(*szSpecName)
        hCsvTableFile << "SpecName," << szSpecName << endl;
    if(*szSpecVersion)
        hCsvTableFile << "SpecVersion," << szSpecVersion << endl;
    if(*szFlowID)
        hCsvTableFile << "FlowID," << szFlowID << endl;
    if(*szSetupID)
        hCsvTableFile << "SetupID," << szSetupID << endl;
    if(*szDesignRev)
        hCsvTableFile << "DesignRevision," << szDesignRev << endl;
    if(*szEngID)
        hCsvTableFile << "EngineeringLotID," << szEngID << endl;
    if(*szRomCod)
        hCsvTableFile << "RomCode," << szRomCod << endl;
    if(*szSerielNum)
        hCsvTableFile << "TesterSerialNumber," << szSerielNum << endl;
    if(*szSuperName)
        hCsvTableFile << "SupervisorName," << szSuperName << endl;


    // Write results header (unless a SDR record exists...in which case the header is written by the SDR record routine
//    if(m_bSrdRecord == false)
//        WriteResultsHeader();

    return true;	// Success
}

/////////////////////////////////////////////////////////////////////////////
// Read STDF.MRR record, dump it to CSV file...
/////////////////////////////////////////////////////////////////////////////
void	CsvConverterV1x::ProcessMRR(void)
{
    // Ignore MRR during Pass#2!
    if(lPass != 1)
        return;

    StdfFile.ReadDword((long *)&m_cFileData.m_tEndTime);	// End_T
}

/////////////////////////////////////////////////////////////////////////////
// Read STDF.SDR record, dump it to CSV file...
/////////////////////////////////////////////////////////////////////////////
void	CsvConverterV1x::ProcessSDR(void)
{
    // Flags that we have a SDR record!
    //m_bSrdRecord = true;

    // Ignore SDR during Pass#1!
    if(lPass != 2)
        return;

    // SDR
    BYTE				bData;
    int					nSiteCount, nSite;
    char				szString[256];

    // SDR record present only in STDF V4
    if(StdfRecordHeader.iStdfVersion != GEX_STDFV4)
        goto EndOfSdr;

    // Read SDR SDTF V4
    StdfFile.ReadByte(&bData);		// Head nb
    hCsvTableFile << "--- Site details:, Head #" << bData << endl;

    StdfFile.ReadByte(&bData);		// Site group nb
    StdfFile.ReadByte(&bData);		// Site count
    nSiteCount = (int)bData;
    if(nSiteCount != 0)
    {
        // Keep track of site numbers
        hCsvTableFile << "Testing sites,";
        for(nSite=0; nSite<nSiteCount; nSite++)
        {
            StdfFile.ReadByte(&bData);			// Site#
            hCsvTableFile << QString::number(bData) << " ";
        }
        hCsvTableFile << endl;
    }

    if(ReadStringToField(szString) != 1)		// Handler type
        goto EndOfSdr;
    if(*szString)
        hCsvTableFile << "Handler type," << szString << endl;

    // If reach this point then we have at least one SDR valid record, even if nSiteCount states otherwise!
    if(nSiteCount == 0)
    {
        // Overload site count
        nSiteCount = 1;
        // Overload site map ID
        nSite = 0;
    }

    if(ReadStringToField(szString) != 1)		// Handler ID
        goto EndOfSdr;
    if(*szString)
        hCsvTableFile << "Handler ID," << szString << endl;

    if(ReadStringToField(szString) != 1)		// Prober card type
        goto EndOfSdr;
    if(*szString)
        hCsvTableFile << "Probe card type," << szString << endl;

    if(ReadStringToField(szString) != 1)		// Prober card ID
        goto EndOfSdr;
    if(*szString)
        hCsvTableFile << "Probe card ID," << szString << endl;

    if(ReadStringToField(szString) != 1)		// Loadboard type
        goto EndOfSdr;
    if(*szString)
        hCsvTableFile << "Load board type," << szString << endl;

    if(ReadStringToField(szString) != 1)		// Loadboard ID
        goto EndOfSdr;
    if(*szString)
        hCsvTableFile << "Load board ID," << szString << endl;

    if(ReadStringToField(szString) != 1)		// DIBboard type
        goto EndOfSdr;
    if(*szString)
        hCsvTableFile << "DIB board type," << szString << endl;

    if(ReadStringToField(szString) != 1)		// DIBboard ID
        goto EndOfSdr;
    if(*szString)
        hCsvTableFile << "DIB board ID," << szString << endl;

    if(ReadStringToField(szString) != 1)		// Interface cable type
        goto EndOfSdr;
    if(*szString)
        hCsvTableFile << "Interface cable type," << szString << endl;

    if(ReadStringToField(szString) != 1)		// Interface cable ID
        goto EndOfSdr;
    if(*szString)
        hCsvTableFile << "Interface cable ID," << szString << endl;

    if(ReadStringToField(szString) != 1)		// Handler contactor type
        goto EndOfSdr;
    if(*szString)
        hCsvTableFile << "Handler contractor type," << szString << endl;

    if(ReadStringToField(szString) != 1)		// Handler contactor ID
        goto EndOfSdr;
    if(*szString)
        hCsvTableFile << "Handler contractor ID," << szString << endl;

    if(ReadStringToField(szString) != 1)		// Laser type
        goto EndOfSdr;
    if(*szString)
        hCsvTableFile << "Laser type," << szString << endl;

    if(ReadStringToField(szString) != 1)		// Laser ID
        goto EndOfSdr;
    if(*szString)
        hCsvTableFile << "Laser ID," << szString << endl;

    if(ReadStringToField(szString) != 1)		// Extra equipment type
        goto EndOfSdr;
    if(*szString)
        hCsvTableFile << "Extra equipment type," << szString << endl;

    if(ReadStringToField(szString) != 1)		// Extra equipment ID
        goto EndOfSdr;
    if(*szString)
        hCsvTableFile << "Extra equipment ID," << szString << endl;

EndOfSdr:;
}

void	CsvConverterV1x::ProcessPDR(void)	// Extract Parametric Test Definition (STDF V3 only.)
{
}

/////////////////////////////////////////////////////////////////////////////
// Read STDF WIR record
/////////////////////////////////////////////////////////////////////////////
void	CsvConverterV1x::ProcessWIR(void)
{
    // Ignore WIR during Pass#2!
    if(lPass != 1)
        return;

    StdfFile.ReadByte(&bData);	// Head
    StdfFile.ReadByte(&bData);	// pad (stdf V3), test site (stdf V4)
    StdfFile.ReadDword(&lData);	// START_T

    // WaferID
    char	szString[257];	// A STDF string is 256 bytes long max!
    if(ReadStringToField(szString) < 0)
        return;	// Incomplete record...ignore.

    // LTX-STDF bug fix: if empty WaferID, assume it is the same as previous one found!
    if(*szString == 0)
        return;

    m_cFileData.m_strWaferID = szString;

    resetVariables();
}

/////////////////////////////////////////////////////////////////////////////
// Read STDF WCR record
/////////////////////////////////////////////////////////////////////////////
void	CsvConverterV1x::ProcessWCR(void)
{
    // Ignore WCR during Pass#2!
    if(lPass != 1)
        return;
    BYTE	bPOS_X,bPOS_Y;

    if(StdfFile.ReadFloat(&fData)  != GS::StdLib::Stdf::NoError)
        return;	// WaferSize.
    if(StdfFile.ReadFloat(&fData)  != GS::StdLib::Stdf::NoError)
        return;	// DIE Heigth in WF_UNITS.
    if(StdfFile.ReadFloat(&fData)  != GS::StdLib::Stdf::NoError)
        return;	// DIE Width in WF_UNITS.
    if(StdfFile.ReadByte(&bData)  != GS::StdLib::Stdf::NoError)
        return;	// WF_UNITS

    if(StdfFile.ReadByte(&bData)  != GS::StdLib::Stdf::NoError)
        return;// WF_FLAT
    m_cFileData.m_strWaferOrientation = bData;

    StdfFile.ReadWord(&wData);	// CENTER_X
    StdfFile.ReadWord(&wData);	// CENTER_Y
    StdfFile.ReadByte(&bPOS_X);	// POS_X
    StdfFile.ReadByte(&bPOS_Y);	// POS_Y

}

/////////////////////////////////////////////////////////////////////////////
// Read STDF.PTR record, dump it to CSV file...
/////////////////////////////////////////////////////////////////////////////
void	CsvConverterV1x::ProcessPTR(void)	// Extract Test results
{
    QString		strString;
    int			test_flg;
    char			szString[257];	// A STDF string is 256 bytes long max!
    float		fResult;
    double		lfLowLimit=0.0,lfHighLimit=0.0;
    bool			bValideResult;
    // FIXME: not used ?
    //bool bFailResult=false;
    unsigned int	uiTestNumber = 0;
    CShortTest *	ptTestCell;	// Pointer to test cell to receive STDF info.

    // PTR
    switch(StdfRecordHeader.iStdfVersion)
    {

    case GEX_STDFV4:
         StdfFile.ReadDword(&lData);	// test number
         uiTestNumber = lData;

         BYTE lHead = 0, lSite = 0;

         StdfFile.ReadByte(&lHead);		// Head
         StdfFile.ReadByte(&lSite);		// test site

         GQTL_STDF::Stdf_PTR_V4 ptr;
         ptr.SetHEAD_NUM( lHead );
         ptr.SetSITE_NUM( lSite );

         unsigned short siteNumber =
             GQTL_STDF::HeadAndSiteNumberDecipher::GetSiteNumberIn
             ( StdfFile, ptr );

         StdfFile.ReadByte(&bData);		// test_flg
         test_flg = (int) bData & 0xff;
         StdfFile.ReadByte(&bData);		// parm_flg
         StdfFile.ReadFloat(&fResult);	// test result

         // Pass#1: get Test definition (name, limits, units, etc.).
        // Read Test definition: name, Limits, etc...
        if(StdfFile.ReadString(szString)  != GS::StdLib::Stdf::NoError)
        {
            //no error !
            //return;	// text description (test name)
            strString = "";
        }
        else
        {
            // Formats test name (in case too int, or leading spaces)..only take non-empty strings (since at init. time it is already set to empty)
            strString = FormatTestName(szString);
        }

        // Returns pointer to correct cell. If cell doesn't exist ; it's created.
         if(FindTestCell(uiTestNumber, GEX_PTEST, &ptTestCell, true, false, strString) !=1)
             return;	// Error

         // FIXME: not used ?
         //if((test_flg & 0300) == 0200)
         //    bFailResult = true;

         if(test_flg & 2 || ((bData & 07) & (m_eStdfCompliancy==CsvConverterV1x::STRINGENT)) )
             bValideResult = false;
         else
             bValideResult = true;

         // Check if test result is valid
         if(test_flg & 0x10)
             bValideResult = false;

           // Keep track of the testing flow
        if(bValideResult)
            m_cTestFlow[siteNumber].append((CShortTest*)ptTestCell);

        if(lPass == 2)
         {
            if(bValideResult == true)
            {
                // Save test result
                ptTestCell->lfResult[siteNumber] = fResult;	// Save test result for this site#
            }
            return;
         }

         if(StdfFile.ReadString(szString)  != GS::StdLib::Stdf::NoError)
           return;	// alarm_id

         BYTE	bOptFlag;
         if(StdfFile.ReadByte(&bOptFlag) != GS::StdLib::Stdf::NoError)
            return;	// opt_flg

         // If we already have the limits set...ignore any NEW limits defined.
         if((ptTestCell->bLimitFlag & (CTEST_LIMITFLG_NOLTL | CTEST_LIMITFLG_NOHTL)) != (CTEST_LIMITFLG_NOLTL | CTEST_LIMITFLG_NOHTL))
         {
             // Tells current position in STDF record READ
            long lPos = StdfFile.GetReadRecordPos();
            //.. then skip the Limits field...
            lPos += 11;
            StdfFile.SetReadRecordPos(lPos);// Overwrite current READ position in STDF record
            goto ReadTestUnits;
         }

         // If not in 'What-if' mode, save limits flag seen in STDF file
         if((bOptFlag & 0x10) == 0)
         {
             // LowLimit+low_scal flag is valid...
             ptTestCell->bLimitFlag &= ~(CTEST_LIMITFLG_NOLTL);
             if(bOptFlag & 0x10)	// Low Limit in first PTR
               ptTestCell->bLimitFlag |= CTEST_LIMITFLG_NOLTL;
             else if(bOptFlag & 0x40)	// No Low Limit
               ptTestCell->bLimitFlag |= CTEST_LIMITFLG_NOLTL;
         }

         // If not in 'What-if' mode, save limits flag seen in STDF file
         if((bOptFlag & 0x20) == 0)
         {
             // HighLimit+high_scal flag is valid...
             ptTestCell->bLimitFlag &= ~(CTEST_LIMITFLG_NOHTL);
             if(bOptFlag & 0x20)	// High Limit in first PTR
               ptTestCell->bLimitFlag |= CTEST_LIMITFLG_NOHTL;
             else if(bOptFlag & 0x80)	// No High Limit
               ptTestCell->bLimitFlag |= CTEST_LIMITFLG_NOHTL;
         }

         if(StdfFile.ReadByte(&bData) != GS::StdLib::Stdf::NoError)
           return;	// res_scal
         if((bOptFlag & 0x1) == 0)	// res-scal is valid
            ptTestCell->res_scal = bData;

         if(StdfFile.ReadByte(&bData) != GS::StdLib::Stdf::NoError)
           return;	// llm_scal
         if(((bData & 0x50) == 0) && ((bOptFlag & 0x10) == 0))	// llm-scal is valid
            ptTestCell->llm_scal = bData;

         if(StdfFile.ReadByte(&bData) != GS::StdLib::Stdf::NoError)
           return;	// hlm_scal
         if(((bData & 0x120) == 0) && ((bOptFlag & 0x20) == 0))	// hlm-scal is valid
            ptTestCell->hlm_scal = bData;

         if(StdfFile.ReadFloat(&fData) != GS::StdLib::Stdf::NoError)
           return;	// low limit
         if(((ptTestCell->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0) && ((bOptFlag & 0x10) == 0))
            lfLowLimit = M_CHECK_INFINITE((double)fData); // Low limit exists: keep value
         if(StdfFile.ReadFloat(&fData) != GS::StdLib::Stdf::NoError)
         {
             ptTestCell->lfLowLimit = lfLowLimit;
             return;
         }
         if(((ptTestCell->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0) && ((bOptFlag & 0x20) == 0))
            lfHighLimit = M_CHECK_INFINITE((double)fData);// High limit exists: keep value
        // If limits are in wrong order, swap them !
        if(((ptTestCell->bLimitFlag & (CTEST_LIMITFLG_NOLTL | CTEST_LIMITFLG_NOHTL)) == 0) && (lfLowLimit > lfHighLimit))
        {
            ptTestCell->lfHighLimit = lfLowLimit;
            ptTestCell->lfLowLimit = lfHighLimit;
        }
        else
        {
            ptTestCell->lfHighLimit = lfHighLimit;
            ptTestCell->lfLowLimit = lfLowLimit;
        }


ReadTestUnits:
        if(StdfFile.ReadString(szString)  != GS::StdLib::Stdf::NoError)
            return;
        szString[GEX_UNITS-1] = 0;
        if(*szString)
            strcpy(ptTestCell->szTestUnits,szString);
        return;
        }

}

/////////////////////////////////////////////////////////////////////////////
// Read STDF MPR record
/////////////////////////////////////////////////////////////////////////////
void	CsvConverterV1x::ProcessMPR(void)	// Extract Test results (Multiple Parametric)
{
    QString		strTestName;
    char			szString[257];	// A STDF string is 256 bytes long max!
    int			test_flg;
    float		fResult;
    bool			bValideResult;
    // FIXME: not used ?
    //bool bFailResult = false;
    BYTE			bSite;
    int			nPMRcount, nResultcount;
    long			lTestNumber;
    int			iPinmapMode	= 0;
    CShortTest *	ptMPTestCell	= NULL;
    CShortTest *	ptTestCell;	// Pointer to test cell to receive STDF info.

    // MPR
    switch(StdfRecordHeader.iStdfVersion)
    {

        case GEX_STDFV4:
        {
            // Merge Pins under single test name or not?
            if (m_eMPRMergeMode==MERGE)
                iPinmapMode = GEX_PTEST;
            else
                iPinmapMode = GEX_MPTEST;

            StdfFile.ReadDword(&lTestNumber);	// test number

            StdfFile.ReadByte(&bData);		// Head
            StdfFile.ReadByte(&bSite);		// test site

            GQTL_STDF::Stdf_MPR_V4 mpr;
            mpr.SetHEAD_NUM(bData);
            mpr.SetSITE_NUM(bSite);

            unsigned short siteNumber =
                GQTL_STDF::HeadAndSiteNumberDecipher::GetSiteNumberIn
                ( StdfFile, mpr );

            StdfFile.ReadByte(&bData);		// test_flg
            test_flg = (int) bData & 0xff;
            StdfFile.ReadByte(&bData);		// parm_flg
            StdfFile.ReadWord(&wData);		// RTN_ICNT: Count (j) of PMR indexes
            nPMRcount		= wData;
            StdfFile.ReadWord(&wData);		// RSLT_CNT: Count (k) of returned results
            nResultcount	= wData;

            // Check if this test is marked 'fail' (bits 6 cleared, bit 7 set) ?
            // FIXME: not used ?
            //if((test_flg & 0300) == 0200)
            //  bFailResult = true;

            // Check if test result is valid
            if((nResultcount == 0) || (test_flg & 16) || ((bData & 07) & (m_eStdfCompliancy==CsvConverterV1x::STRINGENT)))
                bValideResult = false;
            else
                bValideResult = true;

            // Read array of states( Nibbles: 8 bits = 2 states of 4 bits!).
            int iCount = nPMRcount;
            while(iCount > 0)
            {
                StdfFile.ReadByte(&bData);		// State[j]
                iCount-=2;
            }

            QVector<double> vecDouble;

            int iPinCount = nPMRcount;

            if (nPMRcount == 0)
                iPinCount = nResultcount;

            // Read each result in array...and process it!
            for(iCount = 0; iCount < nResultcount; iCount++)
            {
                StdfFile.ReadFloat(&fResult);	// test result
                // If more results than Pin indexes...ignore the leading results
                if((iCount < iPinCount) || (iPinmapMode == GEX_PTEST))
                {
                    if(bValideResult == true)
                        vecDouble.append(fResult);
                }
            }

            // Read Test name
            if(StdfFile.ReadString(szString)  != GS::StdLib::Stdf::NoError)
                return;	// text description (test name)

            // Formats test name (in case too int, or leading spaces)..only take non-empty strings (since at init. time it is already set to empty)
            strTestName = FormatTestName(szString);

            // Returns pointer to correct cell. If cell doesn't exist ; it's created.
            if(FindTestCell(lTestNumber, iPinmapMode, &ptTestCell, true, false, strTestName) !=1)
                return;	// Error

            // Keep track of the testing flow
            if(bValideResult)
                m_cTestFlow[siteNumber].append((CShortTest*)ptTestCell);

            if (lPass == 2)
            {
              WORD* resultArrayIndexes = NULL;
              if (ptTestCell->mResultArrayIndexes.isEmpty() == false)
              {
                resultArrayIndexes =
                  ptTestCell->mResultArrayIndexes.takeFirst();
              }

                for (int nCount = 0; nCount < vecDouble.count(); ++nCount)
                {
                    // If merging Multiple parametric results.
                    if(iPinmapMode == GEX_PTEST)
                        ptMPTestCell = ptTestCell;
                    else
                    {
                        // DO NOT allow test# mapping as it was already done in above call to 'FindTestCell'
                        if (FindTestCell(lTestNumber, nCount, &ptMPTestCell,
                                         false, false, strTestName) != 1)
                        {
                          delete[] resultArrayIndexes;
                          return;  // Error
                        }
                    }

                    ptMPTestCell->lfResult[siteNumber] = vecDouble.at(nCount);	// Save test result for this site#
                }
                delete[] resultArrayIndexes;
                return;
            }

            ProcessStaticDataMPR(ptTestCell, lTestNumber, nPMRcount);
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
// Read STDF PMR record
/////////////////////////////////////////////////////////////////////////////
void CsvConverterV1x::ProcessPMR(void)
{
    // Only read in PASS 1
    if(lPass == 2)
        return;

    char		szString[257];	// A STDF string is 256 bytes long max!
    CPinmap	*	ptPinmapCell;	// Pointer to a pinmap cell in pinmap list

    switch(StdfRecordHeader.iStdfVersion)
    {
        case GEX_STDFV4:
        {
             // PMR_INDX: Pinmap index
            StdfFile.ReadWord(&wData);

            if(FindPinmapCell(&ptPinmapCell,wData)!=1)
                return;	// some error creating the pinmap cell, then ignore record

            // CHAN_TYP : Channel type
            StdfFile.ReadWord(&wData);

            // CHAN_NAM : Channel Name
            if(StdfFile.ReadString(szString)  != GS::StdLib::Stdf::NoError)
               return;

            ptPinmapCell->strChannelName = FormatTestName(szString);

            // Keep channel name
            mChannelNames.insert(ptPinmapCell->strChannelName);

            // PHY_NAM : Physical Name
            if(StdfFile.ReadString(szString)  != GS::StdLib::Stdf::NoError)
               return;
            ptPinmapCell->strPhysicName = FormatTestName(szString);

            // LOG_NAM : Logical name
            if(StdfFile.ReadString(szString)  != GS::StdLib::Stdf::NoError)
               return;
            ptPinmapCell->strLogicName = FormatTestName(szString);

            // HEAD_NUM : Head number
            StdfFile.ReadByte(&bData);

            GQTL_STDF::Stdf_PMR_V4 pmr;
            pmr.SetHEAD_NUM(bData);

            ptPinmapCell->bHead = bData;

            // SITE_NUM : site number
            StdfFile.ReadByte(&bData);

            pmr.SetSITE_NUM(bData);

            unsigned short siteNumber =
                GQTL_STDF::HeadAndSiteNumberDecipher::GetSiteNumberIn
                ( StdfFile, pmr );

            ptPinmapCell->m_site = siteNumber;

            break;
        }

         default:
            break;
    }
}

/////////////////////////////////////////////////////////////////////////////
// Read STDF DTR record
/////////////////////////////////////////////////////////////////////////////
void CsvConverterV1x::ProcessDTR(void)
{
    char        szString[257];  // A STDF string is 256 bytes long max!
    QString     lTextData;

    switch(StdfRecordHeader.iStdfVersion)
    {
        case GEX_STDFV4:

            // ASCII text string
            if(StdfFile.ReadString(szString)  != GS::StdLib::Stdf::NoError)
               return;
            lTextData = QString(szString).trimmed();

            // Check if found value
            if(lTextData.isEmpty())
                return;

            // Check if it is a valid syntax
            // <@field=value>
            // <@field=site#|value>
            // only one <, one >, one =
            if(lTextData.startsWith("<@")
                    && lTextData.endsWith(">")
                    && (lTextData.count("<") == 1)
                    && (lTextData.count(">") == 1)
                    && (lTextData.count("=") == 1))
            {
                QString lField, lValue;
                int lSite = -1;
                lField = lTextData.section("@",1).section("=",0,0).remove("@").trimmed();
                lValue = lTextData.section("=",1).section(">",0,0).trimmed();

                // Can have the site id
                // <@field=site#|value>
                if(lValue.contains("|"))
                {
                    bool lValid;
                    lSite = lValue.section("|",0,0).trimmed().toInt(&lValid);
                    if(lValid)
                        lValue = lValue.section("|",1).trimmed();
                    else
                        lSite = -1;
                }

                // Allow empty value
                if(lField.isEmpty())
                {
                    // no data
                    GSLOG(SYSLOG_SEV_WARNING,
                           QString("Found empty DTR key. Ignore this record[DTR=%1]").arg(lTextData)
                          .toLatin1().data() );
                    return;
                }
                // Must be between PIR and PRR
                // PIR + PRR found (after the PRR, before the next PIR)
                if(getRecordCount(5,10) == getRecordCount(5,20))
                {
                    // not under PIR/PRR
                    GSLOG(SYSLOG_SEV_WARNING,
                           QString("Found DTR outside a PIR/PRR. Ignore this record[DTR=%1]").arg(lTextData)
                          .toLatin1().data() );
                    return;
                }
                // Case 7542
                // Can be interlaced PIR/PIR/PRR/PRR
                // PIR + PIR (after 2 PIR before PRR)
                if(getRecordCount(5,10) != (getRecordCount(5,20)+1))
                {
                    // interlaced PIR/PRR
                    // in this case Site# is mandatory
                    if(lSite < 0)
                    {
                        GSLOG(SYSLOG_SEV_WARNING,
                              QString("Found DTR into interlaced PIR/PRR without site info (ie <@field=site#|value>)."
                                      " Ignore this record[DTR=%1]").arg(lTextData)
                              .toLatin1().data() );
                        return;
                    }
                }
                else
                {
                    // Standard multi site
                    // Ignore site info, use the PIR site
                    lSite = -1;
                }

                // Store the result
                // Add index to keep the sequence order
                int lIndex = m_mapVariableSeq.count();
                if(m_mapVariableSeq.contains(lField))
                    lIndex = m_mapVariableSeq[lField];
                else
                    m_mapVariableSeq[lField] = lIndex;
                lField = QString::number(lIndex) + "#SEQ#" + lField;
                m_mapVariableValue[lField][lSite] = lValue;
            }
            break;

         default:
            break;
    }
}

void CsvConverterV1x::ProcessGDR(void)
{
    GQTL_STDF::Stdf_GDR_V4 gdr;
    gdr.Read(StdfFile);

    GQTL_STDF::HeadAndSiteNumberDecipher::SetDecipheringModeInStdf( gdr, StdfFile );
}

void CsvConverterV1x::ProcessPRR(void)
{
    int		iBin=0;
    char	szString[1024];
    QString strPartIdToWrite;
    cTestList::iterator it;

    BYTE bHeadNumber = 0, bSiteNumber = 0;
    GQTL_STDF::Stdf_PRR_V4 prr;
    unsigned short siteNumber = 0;

    // PRR
    *szString = 0;	// Reset Part ID.
    switch(StdfRecordHeader.iStdfVersion)
    {
          case GEX_STDFV4:
            StdfFile.ReadByte(&bHeadNumber);	// head number
            StdfFile.ReadByte(&bSiteNumber);	// site number

            prr.SetHEAD_NUM( bHeadNumber );
            prr.SetSITE_NUM( bSiteNumber );

            siteNumber =
                GQTL_STDF::HeadAndSiteNumberDecipher::GetSiteNumberIn
                ( StdfFile, prr );

            PartInfo.bHead =
                GQTL_STDF::HeadAndSiteNumberDecipher::GetHeadNumberIn
                ( StdfFile, prr );

            PartInfo.m_site = siteNumber;

            StdfFile.ReadByte(&bData);	// part flag
            StdfFile.ReadWord(&wData);	// number of tests
            PartInfo.iTestsExecuted = wData;
            StdfFile.ReadWord(&wData);	// HBIN
            PartInfo.iHardBin = wData;
            StdfFile.ReadWord(&wData);	// SBIN
            PartInfo.iSoftBin = wData;
            if(PartInfo.iSoftBin != 65535)
                iBin = PartInfo.iSoftBin;
            else
                iBin = PartInfo.iHardBin;

            // Update part #
            PartInfo.lPartNumber++;

            if(StdfFile.ReadWord(&wData) != GS::StdLib::Stdf::NoError)
                 break;	// DIE X
            PartInfo.iDieX = wData;
            if(StdfFile.ReadWord(&wData) != GS::StdLib::Stdf::NoError)
                 break;	// DIE Y
            PartInfo.iDieY = wData;

            // Read test time, and get test time info if GOOD binning (bit3,4=0)
            if(StdfFile.ReadDword(&lData) != GS::StdLib::Stdf::NoError)
                 break;	// Execution time
            PartInfo.lExecutionTime = lData;

            StdfFile.ReadString(szString);// PART ID.
            PartInfo.setPartID(szString);
            if(PartInfo.getPartID().isEmpty())
                PartInfo.setPartID (QString::number(PartInfo.lPartNumber));
            break;

    default :
      break;
    }

    if(lPass == 1)
    {
        // Keep track of total sites in use
        m_cSitesUsed[PartInfo.m_site] = 1;

        // Keep track of the longest data flow (Bin1 flow) as it is used in pass 2 to display test list.
        if((iBin == 1) && (m_cTestFlow[PartInfo.m_site].count() > m_cFullTestFlow.count()))
        {
            // This test flow is longer than one saved, so update Full flow with this latest list.
            m_cFullTestFlow.clear();
            for ( it = m_cTestFlow[PartInfo.m_site].begin(); it != m_cTestFlow[PartInfo.m_site].end(); ++it )
                m_cFullTestFlow.append(*it);

        }

        goto end_prr;
    }

    // DEBUG: Only dump parts for a given WaferID (Test 521)
    CShortTest *ptTestCell;
#if 0
    bIgnorePart=false;
    bValidWafer=false;
    for ( it = m_cTestFlow[PartInfo.bSite].begin(); it != m_cTestFlow[PartInfo.bSite].end(); ++it )
    {
        ptTestCell = *it;
        if(ptTestCell->lTestNumber == 521)
        {
            bValidWafer = true;
            if((ptTestCell->lfResult.contains(PartInfo.bSite)) && (ptTestCell->lfResult[PartInfo.bSite] != 5))
                bIgnorePart = true;
        }
        // X die corrdinate
        if(ptTestCell->lTestNumber == 541)
            PartInfo.iDieX = (int) ptTestCell->lfResult[PartInfo.bSite];
        // Y die corrdinate
        if(ptTestCell->lTestNumber == 531)
            PartInfo.iDieY = (int) ptTestCell->lfResult[PartInfo.bSite];

    }
    if(bIgnorePart || (bValidWafer == false))
        goto end_prr;
#endif
    // END-DEBUG

    // Make Die location a signed value.
    if(PartInfo.iDieX >= 32768) PartInfo.iDieX -= 65536;
    if(PartInfo.iDieY >= 32768) PartInfo.iDieY -= 65536;

    // Write Run results
    // case 4059, pyc
    strPartIdToWrite = PartInfo.getPartID().replace(",", "_");
    hCsvTableFile << "PID-" << PartInfo.getPartID() << ", ";

    // Write: "SBIN, HBIN, DIE_X, DIE_Y, SITE, Time, TOTAL_TESTS,LOT_ID,WAFER_ID,"
    hCsvTableFile << PartInfo.iSoftBin << ", ";
    hCsvTableFile << PartInfo.iHardBin << ", ";
    if(PartInfo.iDieX != -32768)
        hCsvTableFile << PartInfo.iDieX;
    hCsvTableFile  << ", ";
    if(PartInfo.iDieY != -32768)
        hCsvTableFile << PartInfo.iDieY;
    hCsvTableFile  << ", ";
    hCsvTableFile << PartInfo.m_site << ", ";
    if(PartInfo.lExecutionTime > 0)
        hCsvTableFile << (float)PartInfo.lExecutionTime/1e3;
    hCsvTableFile  << ", ";

    // Test list.
    long lTotalTests;
    lTotalTests=0;
    ptTestCell = ptTestList;
    while(ptTestCell != NULL)
    {
        if(ptTestCell->lfResult.contains(PartInfo.m_site))
            lTotalTests++;
        // Move to next cell available
        ptTestCell = ptTestCell->ptNextTest;
    };
    hCsvTableFile << lTotalTests << ", ";	// Output total tests in sequence.

    // Lot#
    if(m_cFileData.m_strLotID.isEmpty()==false)
        hCsvTableFile  << m_cFileData.m_strLotID;
    hCsvTableFile  << ", ";

    // Wafer#
    if(m_cFileData.m_strWaferID.isEmpty()==false)
        hCsvTableFile  << m_cFileData.m_strWaferID;
    hCsvTableFile  << ", ";

    // Custom variables
    // Dump Result
    int lSite;
    foreach(QString key, m_mapVariableValue.keys())
    {
        lSite = -1;
        if(!m_mapVariableValue[key].contains(lSite))
            lSite = PartInfo.m_site;
        hCsvTableFile << m_mapVariableValue[key][lSite] << ",";
    }

    // Output sorted by test# or flow id
    double lfValue;

    for ( it = m_cFullTestFlow.begin(); it != m_cFullTestFlow.end(); ++it )
    {
        ptTestCell = *it;

        // If ptResultArrayIndexes is not null, this is a Master Multiple Parametric Record.
        // Don't list it
        if (ptTestCell->mResultArrayIndexes.isEmpty())
        {
            if(ptTestCell->lfResult.contains(PartInfo.m_site))
            {
                lfValue = ptTestCell->lfResult[PartInfo.m_site];
                hCsvTableFile << lfValue;
                // Remove this test entry as we're done with it.
                ptTestCell->lfResult.remove(PartInfo.m_site);
            }
            hCsvTableFile << ", ";
        }
    }

    // Reset PartInfo values.
    PartInfo.lExecutionTime = 0;

    // Append new-line character.
    hCsvTableFile << endl;

end_prr:
    // Reset result list.
    for ( it = m_cTestFlow[PartInfo.m_site].begin(); it != m_cTestFlow[PartInfo.m_site].end(); ++it )
    {
        ptTestCell = *it;
        if(ptTestCell->lfResult.contains(PartInfo.m_site))
            ptTestCell->lfResult.remove(PartInfo.m_site);
    }

    // Reset Flow lists
    m_cTestFlow[PartInfo.m_site].clear();
}

/////////////////////////////////////////////////////////////////////////////
// Read STDF TSR record
/////////////////////////////////////////////////////////////////////////////
void	CsvConverterV1x::ProcessTSR(void)
{
    // TSR
    if(lPass == 2)
        return;

    CShortTest* ptTestCell;	// Pointer to test cell to receive STDF info.
    char        szTestName[257];	// A STDF string is 256 bytes long max!
    BYTE        bData;

    switch(StdfRecordHeader.iStdfVersion)
    {

      case GEX_STDFV4:
        StdfFile.ReadByte(&bData);	// head number: 255=Merged sites.
        StdfFile.ReadByte(&bData);	// site number
        StdfFile.ReadByte(&bData);	// test type
        StdfFile.ReadDword(&lData);	// test number
        // Returns pointer to correct cell. If cell doesn't exist DO NOT create it , quietly exit!
        if(FindTestCell(lData,GEX_PTEST,&ptTestCell,false,false) !=1)
             return;	// Error

        StdfFile.ReadDword(&lData);	// executions
        StdfFile.ReadDword(&lData);	//  # of failures
        if(StdfFile.ReadDword(&lData) != GS::StdLib::Stdf::NoError)
             return;	//  # of alarms
        if(StdfFile.ReadString(szTestName) != GS::StdLib::Stdf::NoError)
             return;	// Test Name

        if (m_eTestMergeRule == MERGE_TEST_NUMBER)
        {
            char        szSeqName[257];	// A STDF string is 256 bytes long max!
            QString     lTestName   = FormatTestName(szTestName);   // Test name read from the STFD record

            // Format the test name by removing Pin number and/or squencer name if requested
            formatSequencerAndPinTestName(lTestName);

            // In case test name was not specified in the PTR but is found in
            // the TSR, let's get it !
            // Also, if this string is more detailed, use it instead.
            if((ptTestCell->strTestName.length() == 0)  || (ptTestCell->strTestName.length() < lTestName.length()))
            {
                // If no test name, last chance is to read sequencer name...
                StdfFile.ReadString(szSeqName);	// Read sequencer name
                if(*szSeqName == 0)
                {
                    // If no test name, no sequencer name...then build test name : 'Txxxx'
                    if(lTestName.isEmpty())
                        ptTestCell->strTestName.sprintf("T%d",ptTestCell->lTestNumber);
                    else
                        ptTestCell->strTestName = lTestName;		// Test name exists, just use it.
                }
                else
                {
                    // Test name or sequencer name
                    if(lTestName.isEmpty())
                        ptTestCell->strTestName.sprintf("%s - T%d",szSeqName,ptTestCell->lTestNumber);	// No test name, only sequencer name
                    else
                        ptTestCell->strTestName = lTestName;		// Test name exists, just use it.
                }
            }
        }
        break;
    }
}

void	CsvConverterV1x::ProcessFDR(void)	// Extract Functional Test Definition (STDF V3 only.)
{
}

void	CsvConverterV1x::ProcessFTR(void)	// Extract FTR data
{
    BYTE	bSite,bHead;
    char	szString[257];	// A STDF string is 256 bytes long max!
    bool	bValideResult=false;
    bool	bFailResult=false;
    long	lTestNumber;
    CShortTest	*ptTestCell;	// Pointer to test cell to receive STDF info.

    // FTR
    StdfFile.ReadDword(&lTestNumber);	// test number

    StdfFile.ReadByte(&bHead);			// Head
    StdfFile.ReadByte(&bSite);			// test site

    GQTL_STDF::Stdf_FTR_V4 ftr;
    ftr.SetHEAD_NUM(bHead);
    ftr.SetSITE_NUM(bSite);

    unsigned short siteNumber =
        GQTL_STDF::HeadAndSiteNumberDecipher::GetSiteNumberIn
        ( StdfFile, ftr );

    StdfFile.ReadByte(&bData);	// test_flg
    // Check if this test is marked 'fail' (bits 6 cleared, bit 7 set) ?
    if((bData & 0300) == 0200)
        bFailResult=true;

    // Check if test result is valid
    if((bData & 0100) || (bData & 16))
        bValideResult = false;
    else
        bValideResult = true;

    switch(StdfRecordHeader.iStdfVersion)
    {
    default :
        break;

    case GEX_STDFV4:
         StdfFile.ReadByte(&bData);		// opt_flg
         StdfFile.ReadDword(&lData);	// cycl_cnt
         StdfFile.ReadDword(&lData);	// rel_vadr
         StdfFile.ReadDword(&lData);	// rept_cnt
         StdfFile.ReadDword(&lData);	// num_fail
         StdfFile.ReadDword(&lData);	// xfail_ad
         StdfFile.ReadDword(&lData);	// yfail_ad
         StdfFile.ReadWord(&wData);		// vect_off
         StdfFile.ReadWord(&wData);		// rtn_icnt
         int rtn_icnt = wData;
         if(StdfFile.ReadWord(&wData)  != GS::StdLib::Stdf::NoError)
             return;					// pgm_icnt
         int pgm_icnt = wData;
        int i;
        for(i=0;i<rtn_icnt; i++)
            StdfFile.ReadWord(&wData);	// rtn_indx
        for(i=0;i<(rtn_icnt+1)/2; i++)
            StdfFile.ReadByte(&bData);	// rtn_stat
        for(i=0;i<pgm_icnt; i++)
            StdfFile.ReadWord(&wData);	// pgm_indx
        for(i=0;i<(pgm_icnt+1)/2; i++)
            StdfFile.ReadByte(&bData);	// pgm_stat

        // fail_pin: buffer of bits saved in a string, upto 65,535 bits
         if(StdfFile.ReadWord(&wData)  != GS::StdLib::Stdf::NoError)
             return;
         long lLoop = (long)wData;
         while(lLoop > 0)
         {
            StdfFile.ReadByte(&bData);		// pack of 8bits
            lLoop -= 8;
         };

        StdfFile.ReadString(szString);	// vect_name
        StdfFile.ReadString(szString);	// time_set
        StdfFile.ReadString(szString);	// op_code
        StdfFile.ReadString(szString);	// test_txt: test name
        break;
  }

    // Formats test name (in case too int, or leading spaces)
    QString strString = szString;
    strString = strString.replace(',',';');

    if(FindTestCell(lTestNumber, GEX_PTEST, &ptTestCell, true, false, strString) !=1)
        return;	// Error

     // Keep track of the testing flow
    if(bValideResult)
        m_cTestFlow[siteNumber].append((CShortTest*)ptTestCell);

     if(lPass == 2)
     {
        if(bValideResult == true)
        {
            if(bFailResult)
                ptTestCell->lfResult[siteNumber] = 0;	// Save FAIL result for this site#
            else
                ptTestCell->lfResult[siteNumber] = 1;	// Save PASS result for this site#
        }
        return;
     }

}

void CsvConverterV1x::ProcessStaticDataMPR(CShortTest * ptTestCell, long lTestNumber, int iPMRCount)
{
    char			szString[257];					// A STDF string is 256 bytes long max!
    CShortTest *	ptMPTestCell	= NULL;
    double          lfLowLimit		= 0.0;
    double          lfHighLimit		= 0.0;
    int             iPinIndex		= 0;

    if(StdfFile.ReadString(szString)  != GS::StdLib::Stdf::NoError)
        return;	// alarm_id

    BYTE	bOptFlag;
    if(StdfFile.ReadByte(&bOptFlag) != GS::StdLib::Stdf::NoError)
        return;	// opt_flg

    // If we already have the limits set...ignore any NEW limits defined.
    if((ptTestCell->bLimitFlag & (CTEST_LIMITFLG_NOLTL | CTEST_LIMITFLG_NOHTL)) != (CTEST_LIMITFLG_NOLTL | CTEST_LIMITFLG_NOHTL))
    {
         // Tells current position in STDF record READ
        long lPos = StdfFile.GetReadRecordPos();
        //.. then skip the Limits field...
        lPos += 11;
        StdfFile.SetReadRecordPos(lPos);// Overwrite current READ position in STDF record

        // TEST_UNITS
        if(StdfFile.ReadString(szString)  != GS::StdLib::Stdf::NoError)
            return;

        szString[GEX_UNITS-1] = 0;
        if(*szString)
            strcpy(ptTestCell->szTestUnits,szString);
    }
    else
    {
        // If not in 'What-if' mode, save limits flag seen in STDF file
        if((bOptFlag & 0x10) == 0)
        {
            // LowLimit+low_scal flag is valid...
            ptTestCell->bLimitFlag &= ~(CTEST_LIMITFLG_NOLTL);
            if(bOptFlag & 0x10)	// Low Limit in first MPR
                ptTestCell->bLimitFlag |= CTEST_LIMITFLG_NOLTL;
            else if(bOptFlag & 0x40)	// No Low Limit
                ptTestCell->bLimitFlag |= CTEST_LIMITFLG_NOLTL;
        }

        // If not in 'What-if' mode, save limits flag seen in STDF file
        if((bOptFlag & 0x20) == 0)
        {
            // HighLimit+high_scal flag is valid...
            ptTestCell->bLimitFlag &= ~(CTEST_LIMITFLG_NOHTL);
            if(bOptFlag & 0x20)	// High Limit in first MPR
                ptTestCell->bLimitFlag |= CTEST_LIMITFLG_NOHTL;
            else if(bOptFlag & 0x80)	// No High Limit
                ptTestCell->bLimitFlag |= CTEST_LIMITFLG_NOHTL;
        }

        if(StdfFile.ReadByte(&bData) != GS::StdLib::Stdf::NoError)
            return;	// res_scal
        if((bOptFlag & 0x1) == 0)	// res-scal is valid
            ptTestCell->res_scal = bData;

        if(StdfFile.ReadByte(&bData) != GS::StdLib::Stdf::NoError)
            return;	// llm_scal
        if(((bData & 0x50) == 0) && ((bOptFlag & 0x10) == 0))	// llm-scal is valid
            ptTestCell->llm_scal = bData;

        if(StdfFile.ReadByte(&bData) != GS::StdLib::Stdf::NoError)
            return;	// hlm_scal
        if(((bData & 0x120) == 0) && ((bOptFlag & 0x20) == 0))	// hlm-scal is valid
            ptTestCell->hlm_scal = bData;

        if(StdfFile.ReadFloat(&fData) != GS::StdLib::Stdf::NoError)
            return;	// low limit
        if(((ptTestCell->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0) && ((bOptFlag & 0x10) == 0))
            lfLowLimit = M_CHECK_INFINITE((double)fData); // Low limit exists: keep value
        if(StdfFile.ReadFloat(&fData) != GS::StdLib::Stdf::NoError)
        {
             ptTestCell->lfLowLimit = lfLowLimit;
             return;
        }

        if(((ptTestCell->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0) && ((bOptFlag & 0x20) == 0))
            lfHighLimit = M_CHECK_INFINITE((double)fData);// High limit exists: keep value

        // If limits are in wrong order, swap them !
        if(((ptTestCell->bLimitFlag & (CTEST_LIMITFLG_NOLTL | CTEST_LIMITFLG_NOHTL)) == 0) && (lfLowLimit > lfHighLimit))
        {
            ptTestCell->lfHighLimit = lfLowLimit;
            ptTestCell->lfLowLimit = lfHighLimit;
        }
        else
        {
            ptTestCell->lfHighLimit = lfHighLimit;
            ptTestCell->lfLowLimit = lfLowLimit;
        }

        // START_IN
        if(StdfFile.ReadFloat(&fData) != GS::StdLib::Stdf::NoError)
            return;

        // INCR_IN
        if(StdfFile.ReadFloat(&fData) != GS::StdLib::Stdf::NoError)
            return;

        // Skip Pinmap index list for now...but will be read later in this function!
        // Skip 'nJcount' fields of 2bytes!
        // Saves current READ offset in the STDF record.
        long lRecordReadOffsetRtnIndx = StdfFile.GetReadRecordPos();
        if(StdfFile.SetReadRecordPos(lRecordReadOffsetRtnIndx+2*iPMRCount) != GS::StdLib::Stdf::NoError)
           return;	// Record is not that big !

      // TEST_UNITS
        if(StdfFile.ReadString(szString)  != GS::StdLib::Stdf::NoError)
            return;

        szString[GEX_UNITS-1] = 0;
        if(*szString)
            strcpy(ptTestCell->szTestUnits,szString);

        // Now, create as many test cells as Pins tested in this test!
        // Seek into record offset to read the RTN_INDX table!
        if(StdfFile.SetReadRecordPos(lRecordReadOffsetRtnIndx) != GS::StdLib::Stdf::NoError)
           return;	// Record is not that big !...then we will use the latest pinmap indexes read for that test.

        // If current mode is to merge multiple parametric values...then exit now!
        if(ptTestCell->lPinmapIndex == GEX_PTEST)
        {
          while (ptTestCell->mResultArrayIndexes.isEmpty() == false)
          {
            GSLOG(SYSLOG_SEV_WARNING, "mResultArrayIndexes is not empty");
            delete[] ptTestCell->mResultArrayIndexes.takeFirst();
          }
          ptTestCell->bTestType = 'M';
          return;
        }
        else
        {
          WORD* resultArrayIndexes = new WORD[iPMRCount];
          if (resultArrayIndexes == NULL)
          {
            GSLOG(SYSLOG_SEV_ERROR,
                   "Memory alloc. failure...ignore this MPR");
            return;
          }
          ptTestCell->mResultArrayIndexes.push_back(resultArrayIndexes);

            // RTN_INDX	array
            for(int iCount = 0; iCount < iPMRCount; iCount++)
            {
                StdfFile.ReadWord(&wData);

                    iPinIndex = wData;
                    // save array of pinmap indexes.
                    resultArrayIndexes[iCount] = iPinIndex;

                    // Pass1: Save it into the test master record..Test# mapping enabled.
                    if (FindTestCell(lTestNumber, iCount, &ptMPTestCell,
                                     true, false, ptTestCell->strTestName) != 1)
                    {
                        return;  // Error
                    }
                    ptMPTestCell->bTestType		= 'M';			// Test type: Parametric multi-results

                    CPinmap *ptPinmapCell;
                    QString strTestName;

                    if(FindPinmapCell(&ptPinmapCell,ptMPTestCell->lPinmapIndex)==1)
                    {
                        // Found pinmap details...show them!
                        if(ptPinmapCell->strChannelName.isEmpty() == false)
                        {
                            // Channel name exists, add it to string
                            strTestName = " - ";
                            strTestName += ptPinmapCell->strChannelName;
                        }
                        else if(ptPinmapCell->strLogicName.isEmpty() == false)
                        {
                            // Logical pin name exists, add it to string
                            strTestName = " - ";
                            strTestName += ptPinmapCell->strLogicName;
                        }
                    }

                    // Copy test static info from master test definition
                    ptMPTestCell->strTestName	= ptTestCell->strTestName + strTestName;
                    ptMPTestCell->bLimitFlag	= ptTestCell->bLimitFlag;
                    ptMPTestCell->res_scal		= ptTestCell->res_scal;
                    ptMPTestCell->llm_scal		= ptTestCell->llm_scal;
                    ptMPTestCell->hlm_scal		= ptTestCell->hlm_scal;
                    strcpy(ptMPTestCell->szTestUnits,ptTestCell->szTestUnits);

                    ptMPTestCell->lfLowLimit	= ptTestCell->lfLowLimit;
                    ptMPTestCell->lfHighLimit	= ptTestCell->lfHighLimit;
             }
        }
    }
}

