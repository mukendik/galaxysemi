///////////////////////////////////////////////////////////
// report_readfile : Extracts data from STDF file
///////////////////////////////////////////////////////////
#include <math.h>
#if defined unix || __MACH__
#include <ctype.h>
#include <stdlib.h>
#endif

#include "report_build.h"
#include "report_options.h"
#include "browser_dialog.h"
#include "gex_constants.h"	// Constants shared in modules
#include "gex_file_in_group.h"
#include "cbinning.h"
#include "scripting_io.h"
#include "cstats.h"
#include "cpart_info.h"
#include "gex_oem_constants.h"
#include "gex_report.h"
#include "import_constants.h"
#include "stdfrecords_v4.h"
#include "stdf_head_and_site_number_decipher.h"
#include <gqtl_log.h>
#include "gexabstractdatalog.h"
#include "gex_group_of_files.h"
#include "product_info.h"
#include "engine.h"
#include <gqtl_global.h>
#include "gexperformancecounter.h"
#include "message.h"
#include "gs_types.h"

// main.cpp
extern CGexReport*		gexReport;			// Handle to report class
extern void				WriteDebugMessageFile(const QString & strMessage);

// report_build.cpp
extern CReportOptions	ReportOptions;		// Holds options (report_build.h)

// In cstats.cpp
extern double			ScalingPower(int iPower);


///////////////////////////////////////////////////////////
// Builds string showing the sites to process
///////////////////////////////////////////////////////////
QString CGexFileInGroup::BuildSitesToProcess(void)
{
    QString strBuildSitesString;

    if(iProcessSite <0)
        strBuildSitesString = "All sites";
    else
    {
        strBuildSitesString = "Site# " + QString::number(iProcessSite);
        strBuildSitesString += " (or all sites if data do not include site info)";
    }

    return strBuildSitesString;
}

///////////////////////////////////////////////////////////
// Builds string showing the parts to process
///////////////////////////////////////////////////////////
QString CGexFileInGroup::BuildPartsToProcess(void)
{
    QString strString;

    int	iPartsToProcess = iProcessBins;//pReportOptions->iProcessBins;

    switch(iPartsToProcess)
    {
        case GEX_PROCESSPART_ALL:	// Range= All bins
        case GEX_PROCESSPART_GOOD:	// Range= Good bins.
        case GEX_PROCESSPART_FAIL:	// Range = All bins EXCEPT Bin 1
        case GEX_PROCESSPART_ODD:	// Odd parts (1,3,5...)
        case GEX_PROCESSPART_EVEN:	// Even parts (2,4,6,...)
        case GEX_PROCESSPART_FIRSTINSTANCE:
        case GEX_PROCESSPART_LASTINSTANCE:
        case GEX_PROCESSPART_NO_SAMPLES:
            strString = ProcessPartsItems[iPartsToProcess];
            break;
        case GEX_PROCESSPART_SBINLIST: // Range of Soft-Bin is custom:
        case GEX_PROCESSPART_EXSBINLIST: // All bins except given Range of Soft Bins
        case GEX_PROCESSPART_HBINLIST: // Range of Hard-Bin is custom:
        case GEX_PROCESSPART_EXHBINLIST: // All bins except given Range of Hard Bins
        case GEX_PROCESSPART_PARTLIST: // Part or parts range
        case GEX_PROCESSPART_EXPARTLIST: // All parts except given parts range
            if (pGexRangeList)
                strString = pGexRangeList->BuildListString((char*)ProcessPartsItems[iPartsToProcess]);							// List of Ranges to process (parts/bins,...)
            break;

        case GEX_PROCESSPART_PARTSINSIDE:	// Parts inside...
        case GEX_PROCESSPART_PARTSOUTSIDE:	// Parts outside...
            if (m_pFilterRangeCoord)
                strString = m_pFilterRangeCoord->buildProcessString();
            break;
    }
    // return (char *)strString.latin1();
    return strString;
}

///////////////////////////////////////////////////////////
// Builds string showing wafermap size
///////////////////////////////////////////////////////////
char	*CGexFileInGroup::BuildWaferMapSize(char *szString)
{
    if(getWaferMapData().GetDiameter() <= 0)
    {
        // Information not available !
        strcpy(szString,GEX_NA);
        return szString;
    }

    sprintf(szString,"%g",getWaferMapData().GetDiameter());

    switch(getWaferMapData().bWaferUnits)
    {
        default:
        case 0:	// Unknown units...don't show anything!
            break;
        case 1:	// Inches
            strcat(szString," inches");
            break;
        case 2:	// Centimeters
            strcat(szString," cm");
            break;
        case 3:	// Millimeters
            strcat(szString," mm");
            break;
        case 4:	// mils
            strcat(szString," mils");
            break;
    }
    return szString;
}

///////////////////////////////////////////////////////////
// Builds string showing die size
///////////////////////////////////////////////////////////
char	*CGexFileInGroup::BuildDieSize(char *szString)
{
    if((getWaferMapData().GetDieHeight() <= 0) || (getWaferMapData().GetDieWidth() <= 0))
    {
        // Information not available !
        strcpy(szString,GEX_NA);
        return szString;
    }

    char	szWidth[50];
    sprintf(szWidth,"width: %g",getWaferMapData().GetDieWidth());
    switch(getWaferMapData().bWaferUnits)
    {
        default:
        case 0:	// Unknown units...don't show anything!
            break;
        case 1:	// Inches
            strcat(szWidth," inches");
            break;
        case 2:	// Centimeters
            strcat(szWidth," cm");
            break;
        case 3:	// Millimeters
            strcat(szWidth," mm");
            break;
        case 4:	// mils
            strcat(szWidth," mils");
            break;
    }

    char	szHeight[50];
    sprintf(szHeight," / height: %g",getWaferMapData().GetDieHeight());
    switch(getWaferMapData().bWaferUnits)
    {
        default:
        case 0:	// Unknown units...don't show anything!
            break;
        case 1:	// Inches
            strcat(szHeight," inches");
            break;
        case 2:	// Centimeters
            strcat(szHeight," cm");
            break;
        case 3:	// Millimeters
            strcat(szHeight," mm");
            break;
        case 4:	// mils
            strcat(szHeight," mils");
            break;
    }
    strcpy(szString,szWidth);
    strcat(szString,szHeight);
    return szString;
}

char * CGexFileInGroup::FormatTestResult(CTest *ptTestCell, double lfValue, int iPower, bool bUseOutputFormat /*= true*/)
{
    GEX_BENCHMARK_METHOD(QString("FormatTestResult"));

    GEX_ASSERT(this != NULL);
    GEX_ASSERT(ptTestCell);

    static	char szBuffer[50]="";
    char	cScaleUnits=' ';
    char	cSpace=' ';
    double	lfTemp=0;

    // Overload result-scale with HighLimit scale
    //if(ReportOptions.iSmartScaling == GEX_UNITS_RESCALE_TO_LIMITS)
    if (m_eScalingMode==SCALING_TO_LIMITS)
        iPower = ptTestCell->hlm_scal;

    if(fabs(lfValue)>= C_INFINITE)
    {
        if (m_OutputFormat == GEX_OPTION_OUTPUT_CSV)
            strcpy(szBuffer,"n/a,n/a");
        else
            strcpy(szBuffer,GEX_NA);
        return szBuffer;
    }

    if(lfValue && (m_eScalingMode==SCALING_SMART)	//ReportOptions.iSmartScaling == GEX_UNITS_RESCALE_SMART
       && ptTestCell->szTestUnits[0] != '%'
       && ptTestCell->szTestUnits[0] != 0)
    {
        lfTemp = fabs(lfValue);
        if(iPower == 2)
            lfTemp *=1e2;
        else
        {
            iPower = -((int) log10(lfTemp));	// Get power of 10
            iPower += (lfTemp >= 1) ? 0 : 1;
        }

        if(iPower > -12 && iPower < 15)
        {
            // If power is not multiple of 3, make it so...
            if(iPower % 3 > 0)
                iPower += 3-(iPower % 3);
            else if (iPower % 3 < 0)
                iPower -= (iPower % 3);
        }
    }

    // If value NOT to be normalized, scale to appropriate factor.
    //if(ReportOptions.iSmartScaling != GEX_UNITS_RESCALE_NORMALIZED)
    if (m_eScalingMode!=SCALING_NORMALIZED)
    {
        switch(iPower)
        {
            default:
                lfValue *=GS_POW(10.0,iPower);
                break;

            case 0:
                break;

            case 253:	// for unsigned -3
            case -3:
                cScaleUnits = 'K';
                lfValue *=1e-3;
                break;
            case 250:	// for unsigned -6
            case -6:
                cScaleUnits = 'M';
                lfValue *=1e-6;
                break;
            case 247:	// for unsigned -9
            case -9:
                cScaleUnits = 'G';
                lfValue *=1e-9;
                break;
            case 244:	// for unsigned -13
            case -12:
                cScaleUnits = 'T';
                lfValue *=1e-12;
                break;
            case 2:
                if(ptTestCell->szTestUnits[0] != '%')
                    cScaleUnits = '%';
                lfValue *=1e2;
                break;
            case 3:
                cScaleUnits = 'm';
                lfValue *=1e3;
                break;
            case 6:
                cScaleUnits = 'u';
                lfValue *=1e6;
                break;
            case 9:
                cScaleUnits = 'n';
                lfValue *=1e9;
                break;
            case 12:
                cScaleUnits = 'p';
                lfValue *=1e12;
                break;
            case 15:
                cScaleUnits = 'f';
                lfValue *=1e15;
                break;
        }
    }

    // Do not add the ',' between value and units when we format the test result for display it in the intercatives tables
    if (bUseOutputFormat && (m_OutputFormat == GEX_OPTION_OUTPUT_CSV))	//if (of=="CSV") //if (ReportOptions.iOutputFormat == GEX_OPTION_OUTPUT_CSV)
        cSpace = ',';	// Under spreadsheet editor, ensures units are not in same column as data.
    else
        cSpace = ' ';

    sprintf(szBuffer,"%g",lfValue);
    QString strFormatDouble = gexReport->getNumberFormat()->formatNumericValue(lfValue, true, QString(szBuffer));
    sprintf(szBuffer, "%s%c%c%s", strFormatDouble.toLatin1().constData(), cSpace,cScaleUnits,ptTestCell->szTestUnits);

    return szBuffer;
}

/////////////////////////////////////////////////////////////////////////////
// Format result to be scaled to correct unit: eg. Mv, or uA, etc...
// return: none, but pointer to value passed changes value (scales it)
/////////////////////////////////////////////////////////////////////////////
void CGexFileInGroup::FormatTestResultNoUnits(double *ptlfValue, int nResScale)
{
    // If we have to keep limits in normalized format, do not rescale!
    //if(ReportOptions.iSmartScaling == GEX_UNITS_RESCALE_NORMALIZED)
    if (m_eScalingMode == SCALING_NORMALIZED)
        return;

    switch(nResScale)
    {
        default:
            *ptlfValue *= GS_POW(10.0,nResScale);
            return;

        case 0:
            return;
        case 254:
        case -2:	// '%'
            *ptlfValue *=1e-2;
            return;
        case 253:	// for unsigned -3
        case -3:
            *ptlfValue *=1e-3;
            return;
        case 250:	// for unsigned -6
        case -6:
            *ptlfValue *=1e-6;
            return;
        case 247:	// for unsigned -9
        case -9:
            *ptlfValue *=1e-9;
            return;
        case 244:	// for unsigned -13
        case -12:
            *ptlfValue *=1e-12;
            return;
        case 2:	// '%'
            *ptlfValue *=1e2;
            return;
        case 3:
            *ptlfValue *=1e3;
            return;
        case 6:
            *ptlfValue *=1e6;
            return;
        case 9:
            *ptlfValue *=1e9;
            return;
        case 12:
            *ptlfValue *=1e12;
            return;
        case 15:
            *ptlfValue *=1e15;
            return;
    }
}


/////////////////////////////////////////////////////////////////////////////
// constructor
/////////////////////////////////////////////////////////////////////////////
CPinmap::CPinmap()
{
    strChannelName="";	// Pinmap channel name
    strPhysicName="";
    strLogicName="";
    bHead=0;
    m_site=0;
}

/////////////////////////////////////////////////////////////////////////////
// Find and/or create Pinmap entry in the Pinmap List.
/////////////////////////////////////////////////////////////////////////////
int	CGexFileInGroup::FindPinmapCell(CPinmap **ptPinmapCellFound,int iPinmapIndex)
{
    CPinmap		*ptNewCell;
    CPinmap		*ptPinmapCell;
    CPinmap		*ptPrevPinmapCell;

    // Pinmap must be a positive number!
    if(iPinmapIndex < 0)
        return -1;

    ptPinmapCell = mStdfDataFileParent->getPinmap() ;
    if(ptPinmapCell == NULL)
    {
        // First Pinmap cell : list is currently empty.
        ptNewCell = ptPinmapCell = new CPinmap;
        mStdfDataFileParent->setPinmap(ptPinmapCell);
        ptNewCell->iPinmapIndex = iPinmapIndex;
        mStdfDataFileParent->getPinmap()->ptNextPinmap  = NULL;
        *ptPinmapCellFound = mStdfDataFileParent->getPinmap();	// Pointer to cell just created
        return 1;	// Success
    }

    // Loop until test cell found, or end of list reached.
    ptPinmapCell = mStdfDataFileParent->getPinmap();
    ptPrevPinmapCell = NULL;
    while(ptPinmapCell != NULL)
    {
        if(ptPinmapCell->iPinmapIndex > iPinmapIndex)
            break;
        else
            if(ptPinmapCell->iPinmapIndex == iPinmapIndex)
            {
                // Entry already exists...return pointer to the cell.
                *ptPinmapCellFound = ptPinmapCell;	// pointer to matching pinmap cell
                return 1;
            }
        ptPrevPinmapCell = ptPinmapCell;
        ptPinmapCell = ptPinmapCell->ptNextPinmap;
    };

    // Pinmap not in list: insert in list.
    ptNewCell = new CPinmap;
    ptNewCell->iPinmapIndex = iPinmapIndex;
    ptNewCell->ptNextPinmap  = NULL;

    if(ptPrevPinmapCell == NULL)
    {
        // This cell becomes head of list
        ptNewCell->ptNextPinmap = mStdfDataFileParent->getPinmap();
        mStdfDataFileParent->setPinmap(ptNewCell);
    }
    else
    {
        // Insert cell in list
        ptPrevPinmapCell->ptNextPinmap = ptNewCell;
        ptNewCell->ptNextPinmap  = ptPinmapCell;
    }
    *ptPinmapCellFound = ptNewCell;	// pointer to new pinmap cell created
    return 1;
}

/////////////////////////////////////////////////////////////////////////////
// Removes spaces in string
/////////////////////////////////////////////////////////////////////////////
void CGexFileInGroup::RemoveComas(char *szString)
{
    char *ptChar;

    // Replaces comas ',' by ';'.
    do
    {
        ptChar = strchr(szString,',');
        if(ptChar != NULL) *ptChar = ';';
    }
    while(ptChar != NULL);
}


/////////////////////////////////////////////////////////////////////////////
// Read STDF MRR record
/////////////////////////////////////////////////////////////////////////////
void CGexFileInGroup::ReadMRR(void)
{
    // MRR
    if(lPass == 2)
        return;

    // Flags that this file is complete (as the MRR is ALWAYS the last record in a STDF file).
    getMirDatas().bMrrRecord = true;

    switch(StdfRecordHeader.iStdfVersion)
    {
    default :
            break;
        case GEX_STDFV4:
        {
            long data;
            StdfFile.ReadDword(&data);	// End_T
            getMirDatas().lEndT = (time_t)data;
            break;
        }
    }
}


/////////////////////////////////////////////////////////////////////////////
// Read STDF WCR record
/////////////////////////////////////////////////////////////////////////////
void CGexFileInGroup::ReadWCR(void)
{
    // WCR
    BYTE	bPOS_X;
    BYTE    bPOS_Y;
    GS::Gex::WaferCoordinate lCenterDie;

    // Ignore WCR in pass 2
    if(lPass == 2)
        return;

    if(StdfFile.ReadFloat(&fData) != GS::StdLib::Stdf::NoError)
        return;	// WaferSize.
    getWaferMapData().SetDiameter(fData);
    if(StdfFile.ReadFloat(&fData) != GS::StdLib::Stdf::NoError)
        return;	// DIE Heigth in WF_UNITS.
    getWaferMapData().SetDieHeight(fData);
    if(StdfFile.ReadFloat(&fData) != GS::StdLib::Stdf::NoError)
        return;	// DIE Width in WF_UNITS.
    getWaferMapData().SetDieWidth(fData);
    if(StdfFile.ReadByte(&bData) != GS::StdLib::Stdf::NoError)
        return;	// WF_UNITS
    getWaferMapData().bWaferUnits = bData;
    if(StdfFile.ReadByte(&bData) != GS::StdLib::Stdf::NoError)
        return; // WF_FLAT
    getWaferMapData().cWaferFlat_Stdf = bData;
    if(StdfFile.ReadWord(&wData) != GS::StdLib::Stdf::NoError)
        return; // CENTER_X
    lCenterDie.SetX(wData);
    getWaferMapData().SetCenterDie(lCenterDie);
    if(StdfFile.ReadWord(&wData) != GS::StdLib::Stdf::NoError)
        return; // CENTER_Y
    lCenterDie.SetY(wData);
    getWaferMapData().SetCenterDie(lCenterDie);
    if(StdfFile.ReadByte(&bPOS_X) != GS::StdLib::Stdf::NoError)
        return; // POS_X
    getWaferMapData().cPos_X = bPOS_X;
    if(StdfFile.ReadByte(&bPOS_Y) != GS::StdLib::Stdf::NoError)
        return; // POS_Y
    getWaferMapData().cPos_Y = bPOS_Y;
}

/////////////////////////////////////////////////////////////////////////////
// Read STDF WRR record
/////////////////////////////////////////////////////////////////////////////
void CGexFileInGroup::ReadWRR(void)
{
    // WIR
    if(lPass == 2)
        return;

    if(StdfRecordHeader.iStdfVersion == 3)
    {
        // STDF V3
        StdfFile.ReadDword((long *)&getWaferMapData().lWaferEndTime);	// FINISH_T
        StdfFile.ReadByte(&bData);	// Head
        StdfFile.ReadByte(&bData);	// pad
    }
    else
    {
        // STDF V4
        StdfFile.ReadByte(&bData);	// Head
        StdfFile.ReadByte(&bData);	// test site
        long lData;
        StdfFile.ReadDword(&lData);	// FINISH_T
        getWaferMapData().lWaferEndTime = (time_t)lData;
    }
    // STDF V3 or V4

    // Overwrite MIR Time/date data with Wafer Time info (other wise all wafers in file would have same Time info!).
    getMirDatas().lEndT = getWaferMapData().lWaferEndTime;

    // PART_CNT
    if(StdfFile.ReadDword(&lData) != GS::StdLib::Stdf::NoError)
        return;

    if((lData >= 0) && (lData != GS_MAX_UINT32))
    {
        if((getPcrDatas().lPartCount <= 0) || (getPcrDatas().lPartCount == GS_MAX_UINT32))
            getPcrDatas().lPartCount = lData;
        else if (getPcrDatas().bFirstPcrRecord)
        {
            // We have already seen another WRR in the file...merge data
            getPcrDatas().lPartCount += lData;
        }
    }

    // RTST_CNT
    if(StdfFile.ReadDword(&lData) != GS::StdLib::Stdf::NoError)
        return;

    if((lData >= 0) && (lData != GS_MAX_UINT32))
    {
        if((getPcrDatas().lRetestCount <= 0) || (getPcrDatas().lRetestCount == GS_MAX_UINT32))
            getPcrDatas().lRetestCount = lData;
        else if (getPcrDatas().bFirstPcrRecord)
        {
            // We have already seen another WRR in the file...merge data
            getPcrDatas().lRetestCount += lData;
        }
    }

    // ABRT_CNT
    if(StdfFile.ReadDword(&lData) != GS::StdLib::Stdf::NoError)
        return;

    if((lData >= 0) && (lData != GS_MAX_UINT32))
    {
        if((getPcrDatas().lAbortCount <= 0) || (getPcrDatas().lAbortCount == GS_MAX_UINT32))
            getPcrDatas().lAbortCount = lData;
        else if (getPcrDatas().bFirstPcrRecord)
        {
            // We have already seen another WRR in the file...merge data
            getPcrDatas().lAbortCount += lData;
        }
    }

    // GOOD_CNT
    if(StdfFile.ReadDword(&lData) != GS::StdLib::Stdf::NoError)
        return;

    if((lData >= 0) && (lData != GS_MAX_UINT32))
    {
        if((getPcrDatas().lGoodCount <= 0) || (getPcrDatas().lGoodCount == GS_MAX_UINT32))
            getPcrDatas().lGoodCount = lData;
        else if (getPcrDatas().bFirstPcrRecord)
        {
            // We have already seen another WRR in the file...merge data
            getPcrDatas().lGoodCount += lData;
        }
    }

    // FUNC_CNT
    if(StdfFile.ReadDword(&lData) != GS::StdLib::Stdf::NoError)
        return;

    if((lData >= 0) && (lData != GS_MAX_UINT32))
    {
        if((getPcrDatas().lFuncCount <= 0) || (getPcrDatas().lFuncCount == GS_MAX_UINT32))
            getPcrDatas().lFuncCount = lData;
        else if (getPcrDatas().bFirstPcrRecord)
        {
            // We have already seen another WRR in the file...merge data
            getPcrDatas().lFuncCount += lData;
        }
    }

    // Wafer ID
    if(ReadStringToField(getWaferMapData().szWaferID) < 0)
    {
        // Incomplete record: use n/a WaferID
        // case 7850
        sprintf(getWaferMapData().szWaferID, "%s", "n/a");
        //sprintf(getWaferMapData().szWaferID, "%02d", uiAutoWaferID);
    }
}

/////////////////////////////////////////////////////////////////////////////
// Read STDF PCR record
/////////////////////////////////////////////////////////////////////////////
void CGexFileInGroup::ReadPCR(void)
{
    BYTE	bHeadNumber;
    // PCR
    if(lPass == 2)
        return;

    switch(StdfRecordHeader.iStdfVersion)
    {
    default :
            break;
        case GEX_STDFV4:
            if(getPcrDatas().bFirstPcrRecord == true)
            {
                // First PCR record...reset all PCR related variables. WRR may have loaded them (in case no PCR present in file!).
                getPcrDatas().bFirstPcrRecord = false;
                getPcrDatas().lPartCount	 = 0;
                /* HTH case 3828: reset these variables to -1, not to 0
            MirData.lRetestCount = 0;
            MirData.lAbortCount	 = 0;
            MirData.lGoodCount   = 0;
            MirData.lFuncCount   = 0;
            */
                getPcrDatas().lRetestCount = -1;
                getPcrDatas().lAbortCount	 = -1;
                getPcrDatas().lGoodCount   = -1;
                getPcrDatas().lFuncCount   = -1;
            }

            StdfFile.ReadByte(&bHeadNumber);	// Head: 255=Merged sites info
            StdfFile.ReadByte(&bData);			// test site

            unsigned short siteNumber = bData;
            if(StdfFile.GetHeadAndSiteNumberDecipheringMode() != GS::StdLib::no_deciphering)
            {
                siteNumber = GQTL_STDF::HeadAndSiteNumberDecipher::DecipherSiteNumberForAdvantest(bHeadNumber, bData);
                bHeadNumber = GQTL_STDF::HeadAndSiteNumberDecipher::DecipherHeadNumberForAdvantest(bHeadNumber);
            }


            if((iProcessSite >=0) && ((siteNumber != iProcessSite) || (bHeadNumber == 255)))
                break;	// Site filter not matching.

            // If PCR of merged sites found, and filter=all sites, ignore other PCRs!
            // Sometimes PCRs of specific sites appear AFTER PCR of merged sites!
            if(getPcrDatas().bMergedPcrDone == true)
                break;	// Ignore all other PCRs for this file since we have read the 'Merged' record!
            if(StdfFile.ReadDword(&lData)  != GS::StdLib::Stdf::NoError)
                break;// PART_CNT
            if(bHeadNumber == 255)
            {
                // Tells we will ignore any future PCRs...as we only want the ALL sites!
                if(iProcessSite<0)
                    getPcrDatas().bMergedPcrDone = true;
                getPcrDatas().lPartCount = lData; // This is a merge of all sites
            }
            else
                getPcrDatas().lPartCount+= lData; // Cumulate all sites.

            if(StdfFile.ReadDword(&lData)  != GS::StdLib::Stdf::NoError)
                break;// RTST_CNT

            if (lData >= 0 && lData != GS_MAX_UINT32)
            {
                if(bHeadNumber == 255)
                    getPcrDatas().lRetestCount = lData; // This is a merge of all sites
                else
                {
                    if (getPcrDatas().lRetestCount == -1)
                        getPcrDatas().lRetestCount = lData;
                    else
                        getPcrDatas().lRetestCount+= lData; // Cumulate all sites.
                }
            }

            if(StdfFile.ReadDword(&lData)  != GS::StdLib::Stdf::NoError)
                break;// ABRT_CNT

            if (lData >= 0 && lData != GS_MAX_UINT32)
            {
                if(bHeadNumber == 255)
                    getPcrDatas().lAbortCount = lData; // This is a merge of all sites
                else
                {
                    if (getPcrDatas().lAbortCount == -1)
                        getPcrDatas().lAbortCount = lData;
                    else
                        getPcrDatas().lAbortCount+= lData; // Cumulate all sites.
                }
            }

            if(StdfFile.ReadDword(&lData)  != GS::StdLib::Stdf::NoError)
                break;// GOOD_CNT

            if (lData >= 0 && lData != GS_MAX_UINT32)
            {
                if(bHeadNumber == 255)
                    getPcrDatas().lGoodCount = lData; // This is a merge of all sites
                else
                {
                    if (getPcrDatas().lGoodCount == -1)
                        getPcrDatas().lGoodCount = lData;
                    else
                        getPcrDatas().lGoodCount+= lData; // Cumulate all sites.
                }
            }

            if(StdfFile.ReadDword(&lData)  != GS::StdLib::Stdf::NoError)
                break;// FUNC_CNT

            if (lData >= 0 && lData != GS_MAX_UINT32)
            {
                if(bHeadNumber == 255)
                    getPcrDatas().lFuncCount = lData; // This is a merge of all sites
                else
                {
                    if (getPcrDatas().lFuncCount == -1)
                        getPcrDatas().lFuncCount = lData;
                    else
                        getPcrDatas().lFuncCount+= lData; // Cumulate all sites.
                }
            }
            break;
    }
}

/////////////////////////////////////////////////////////////////////////////
// Read STDF GDR record
// Examinator accepts a scripting language in the GDR record!
/////////////////////////////////////////////////////////////////////////////
void CGexFileInGroup::ReadGDR(void)
{
    GQTL_STDF::Stdf_GDR_V4 gdr;
    gdr.Read( StdfFile );

    // modify the deciphering mode for encountered site number in other records
    GQTL_STDF::HeadAndSiteNumberDecipher::SetDecipheringModeInStdf( gdr, StdfFile );
}

////////////////////////////////////////////////////////////////////////////////
/// Process some reticle information in a stdf V4 DTR record
////////////////////////////////////////////////////////////////////////////////
bool CGexFileInGroup::ProcessReticleInformationsIn( const GQTL_STDF::Stdf_DTR_V4 &dtr )
{
    // Extract je json object inside the dtr datas
    QJsonObject reticleInJSON = dtr.GetGsJson("reticle");
    if (reticleInJSON.empty())
    {
        return true;
    }

    if (lPass == 1)
    {
        if (reticleInJSON["RETX"].isDouble() == false)
        {
            GSLOG(SYSLOG_SEV_ERROR,
                  QString("Invalid RETX parameter for Reticle DTR: %1").arg(reticleInJSON["RETX"].toString())
                  .toLatin1().constData());
            return false;
        }

        if (reticleInJSON["RETY"].isDouble() == false)
        {
            GSLOG(SYSLOG_SEV_ERROR,
                  QString("Invalid RETY parameter for Reticle DTR: %1").arg(reticleInJSON["RETY"].toString())
                  .toLatin1().constData());
            return false;
        }

        if (reticleInJSON["DIEX"].isDouble() == false)
        {
            GSLOG(SYSLOG_SEV_ERROR,
                  QString("Invalid DIEX parameter for Reticle DTR: %1").arg(reticleInJSON["DIEX"].toString())
                  .toLatin1().constData());
            return false;
        }

        if (reticleInJSON["DIEY"].isDouble() == false)
        {
            GSLOG(SYSLOG_SEV_ERROR,
                  QString("Invalid DIEY parameter for Reticle DTR: %1").arg(reticleInJSON["DIEX"].toString())
                  .toLatin1().constData());
            return false;
        }

        if (reticleInJSON["RETPOSX"].isDouble() == false)
        {
            GSLOG(SYSLOG_SEV_ERROR,
                  QString("Invalid RETPOSX parameter for Reticle DTR: %1").arg(reticleInJSON["RETPOSX"].toString())
                  .toLatin1().constData());
            return false;
        }

        if (reticleInJSON["RETPOSY"].isDouble() == false)
        {
            GSLOG(SYSLOG_SEV_ERROR,
                  QString("Invalid RETPOSY parameter for Reticle DTR: %1").arg(reticleInJSON["RETPOSY"].toString())
                  .toLatin1().constData());
            return false;
        }

        // Coordinates inside the reticle within jthe JSON object
        int lReticleDieX    = reticleInJSON["RETX"].toInt();
        int lReticleDieY    = reticleInJSON["RETY"].toInt();

        // First pass, calculate the size of the reticle
        if (getWaferMapData().GetReticleMinX() > lReticleDieX)
            getWaferMapData().SetReticleMinX(lReticleDieX);
        if (getWaferMapData().GetReticleMaxX() < lReticleDieX)
            getWaferMapData().SetReticleMaxX(lReticleDieX);

        if(getWaferMapData().GetReticleMinY() > lReticleDieY)
            getWaferMapData().SetReticleMinY(lReticleDieY);
        if(getWaferMapData().GetReticleMaxY() < lReticleDieY)
            getWaferMapData().SetReticleMaxY(lReticleDieY);
    }
    else
    {
        // Second pass, save X Y
        CWafMapArray* lWafMapArray = getWaferMapData().getWafMap();
        if (lWafMapArray == NULL)
        {
            GSLOG(SYSLOG_SEV_ERROR, "Failed to get wafermap array");
            return false;
        }

        // Compute die coordinate on wafermap to assign the reticle information to this die.
        int lIndex = -1;
        if (getWaferMapData().indexFromCoord(lIndex, reticleInJSON["DIEX"].toInt(), reticleInJSON["DIEY"].toInt()))
        {
            lWafMapArray[lIndex].SetReticleDieY(reticleInJSON["RETY"].toInt());
            lWafMapArray[lIndex].SetReticleDieX(reticleInJSON["RETX"].toInt());
            lWafMapArray[lIndex].SetReticlePosY(reticleInJSON["RETPOSY"].toInt());
            lWafMapArray[lIndex].SetReticlePosX(reticleInJSON["RETPOSX"].toInt());
        }
        else
        {
            GSLOG(SYSLOG_SEV_ERROR,
                  QString("Invalid coordinate given for reticle [%1,%2]")
                  .arg(reticleInJSON["DIEX"].toInt()).arg(reticleInJSON["DIEY"].toInt()).toLatin1().constData());
            return false;
        }
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////
// Read STDF DTR record
// Examinator accepts a scripting language in the DTR record!
/////////////////////////////////////////////////////////////////////////////
bool CGexFileInGroup::ReadDTR(void)
{
    bool lIsDatalogString=true, lIsGexScriptingCommand=false, lIsFilteredPart;
    CTest	*lTestCell = NULL;	// Pointer to test cell to receive STDF info.

    // ** check if contains a multi limits record
    // First read TSR completely
    GQTL_STDF::Stdf_DTR_V4 lDTR_V4;
    if (lDTR_V4.Read(StdfFile) == false)
    {
        GSLOG(SYSLOG_SEV_ERROR, QString("Error when reading DTR record")
              .toLatin1().constData());
        return false;
    }

    // process reticle infos if any
    if (ProcessReticleInformationsIn( lDTR_V4 ) == false)
        return false;

    QString lCommand = "ML"; // to search Multi-Limit object
    QJsonObject lJson = lDTR_V4.GetGsJson(lCommand);

    if(!lJson.isEmpty() && lPass == 1) // if multi limit JSon oject found!
    {
        QString lError;
        GS::Core::MultiLimitItem* lLimitItem = new GS::Core::MultiLimitItem();
        if (!lLimitItem->LoadFromJSon(lJson, lError) || !lLimitItem->IsValid())
        {
            GSLOG(SYSLOG_SEV_ERROR, QString("Error when reading DTR record: %1 - %2")
                  .arg(lDTR_V4.m_cnTEXT_DAT)
                  .arg(lError)
                  .toLatin1().constData());
            return false;
        }
        if (!lLimitItem->IsValidLowLimit() && !lLimitItem->IsValidHighLimit())
        {
            GSLOG(SYSLOG_SEV_NOTICE, QString("No Valid limit in DTR: %1")
                  .arg(lDTR_V4.m_cnTEXT_DAT)
                  .toLatin1().constData());
            return true;
        }

        int lTestNumber = GS::Core::MultiLimitItem::ExtractTestNumber(lJson);
        QString lTestName = GS::Core::MultiLimitItem::ExtractTestName(lJson);

        // if filter on site, check site value
        if (iProcessSite != -1)
        {
            int lSite = GS::Core::MultiLimitItem::ExtractSiteNumber(lJson);
            // if special site specified in DTR, make sure it matches the selected site
            // if not, do not store the limit
            if ((lSite != -1) && (iProcessSite != lSite))
            {
                return true;
            }
        }

        if (lTestNumber >= 0)
        {
            // find the right test
            if(FindTestCell(lTestNumber, GEX_PTEST, &lTestCell, true, false, lTestName) != 1)
            {
                GSLOG(SYSLOG_SEV_ERROR, QString("Unable to find matching test for %1")
                      .arg(lDTR_V4.m_cnTEXT_DAT)
                      .toLatin1().constData());
                return false;
            }

             if (lTestCell == NULL)
            {
                GSLOG(SYSLOG_SEV_ERROR, QString("Invalid test found for %1")
                      .arg(lDTR_V4.m_cnTEXT_DAT)
                      .toLatin1().constData());
                return false;
            }
        }
        else
        {
            GSLOG(SYSLOG_SEV_ERROR, QString("Invalid test number found for %1")
                  .arg(lDTR_V4.m_cnTEXT_DAT)
                  .toLatin1().constData());
            return false;
        }


    }

    // Change \n to \r so HTML output (if report type) doesn't include empty lines.
    QString lString = lDTR_V4.m_cnTEXT_DAT;
    lString.replace('\n','\r');

    //************************************************
    // Check if scripting line
    // Format: <cmd> set variable = value
    // Ember should modify their test programs to use fommowing command instead:
    lIsGexScriptingCommand = (lString.toLower().startsWith("<cmd>", Qt::CaseInsensitive));
    if(!lIsGexScriptingCommand)
        lIsGexScriptingCommand = (lString.toLower().indexOf("<cmd> gross_die") >= 0);
    if(!lIsGexScriptingCommand)
        lIsGexScriptingCommand = (lString.toLower().indexOf("<cmd> grossdiecount") >= 0);

    // Check if partID filtered.
    lIsFilteredPart = PartProcessed.IsFilteredPart(pReportOptions, 0, false, iProcessBins);

    if(lPass != 2)
        lIsDatalogString = false;	// Datalog is only during pass2!

    // Do not datalog DTR in advanced mode
    if(hAdvancedReport == NULL || GexAbstractDatalog::existInstance())
        lIsDatalogString = false;	// No file to output the DTR string.

    // Check if Datalog is disabled
    if(pReportOptions->getAdvancedReport() != GEX_ADV_DATALOG)
        lIsDatalogString = false;

    // Check if datalog comments are activated for display
    if(!(m_eDatalogTableOptions & ADF_COMMENTS)	)
        lIsDatalogString = false;	// no!

    // If raw data, do not include DTR strings in datalog
    if(pReportOptions->getAdvancedReportSettings() == GEX_ADV_DATALOG_RAWDATA)
        lIsDatalogString = false;


    // If Datalog enabled, and part not filtered...
    if(lIsDatalogString && !lIsFilteredPart)
    {
        // If line is too long, write it over few lines...or remove many spaces if possible...
        if (
            (m_OutputFormat & GEX_OPTION_OUTPUT_HTML_BASED)
            &&
            (lString.length() >= 100) )
        {
            // First: remove convert double spaces to single space.
            lString = lString.simplified();
        }

        // First test datalogged in part, and not RAW datalog.
        if(lTestsInDatalog == 0)
            WriteDatalogPartHeader();

        if(m_OutputFormat == GEX_OPTION_OUTPUT_CSV)
        {
            // Generating .CSV report file.

            // Datalog current part in the .CSV file.
            if(lTestsInDatalog % 2)
            {
                // Test in in second column...so tab until on next line
                fprintf(hAdvancedReport,"\n");
                lTestsInDatalog++;	// Tells we are now on a even count.
            }
            if(!lString.isEmpty())
                fprintf(hAdvancedReport,"%s\n",lString.toLatin1().constData());
        }
        else if (m_OutputFormat & GEX_OPTION_OUTPUT_HTML_BASED)
        {
            // Datalog current part in the HTML datalog file.
            if(lTestsInDatalog % 2)
            {
                // Test in in second column...so tab until on next line
                fprintf(hAdvancedReport,"</tr>\n");
                lTestsInDatalog++;	// Tells we are now on a even count.
            }
            fprintf(hAdvancedReport,"</table>\n");

            // Write text in green (comment)
            if(!lString.isEmpty())
            {
                // Check if color control given in the string!
                if(lString.startsWith("<font_cmd>", Qt::CaseInsensitive))
                {
                    // Extract rgb string. line format: '<font_cmd> #XXYYZZ line_to_print
                    lString = lString.trimmed();
                    QString lLine = lString.mid(11);
                    QString lColor = lLine.mid(0,7);
                    lLine = lString.mid(18);

                    fprintf(hAdvancedReport,
                            "<font color=\"%s\">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;%s</font>\n",
                            lColor.toLatin1().constData(),lLine.toLatin1().constData());
                }
                else
                    fprintf(hAdvancedReport,"&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;%s\n",
                            lString.toLatin1().constData());
            }
            fprintf(hAdvancedReport,"<table border=\"0\" width=\"98%%\" cellspacing=\"0\" cellpadding=\"0\">\n");
        }
    }

    if(!lIsGexScriptingCommand)
    {
        // GCORE-2133
        if (lString.startsWith("/*GalaxySemiconductor*/") || lString.startsWith("/*GalaxySemiconductorJS*/") )
        {
            QVariant lConcatenatedJSDTR = property(sConcatenatedJavaScriptDTRs.toLatin1().data());
            if (lConcatenatedJSDTR.isNull())
            {
                setProperty(sConcatenatedJavaScriptDTRs.toLatin1().data(), QVariant(lString));
            }
            else
            {
                setProperty(sConcatenatedJavaScriptDTRs.toLatin1().data(), QVariant(lConcatenatedJSDTR.toString()+lString) );
            }
        }
        return true;
    }

    // DTR Gex scripting commands
    long	lTestNumber=0;
    double	lfResult;

    // Parse command line (take string after the '<cmd>' prefix)
    QString lAction = lString.mid(6);
    //strAction = strAction.trimmed();
    QString lKeyword = lAction.section(' ',0,0);
    lKeyword = lKeyword.trimmed();

    // Check if PAT datalog string
    if(lKeyword.startsWith("logPAT", Qt::CaseInsensitive))
    {
        // Command line: '<cmd> logPAT <PAT datalog HTML string>'
        if(lPass == 1 && ReportOptions.getAdvancedReport() == GEX_ADV_PAT_TRACEABILITY)
        {
            // Only consider this string during pass1. Ignore'd during pass 2!
            lAction = lAction.mid(7);
            gexReport->strPatTraceabilityReport += lAction;
        }
        return true;
    }

    int  lCommandIndex;

    // Check if gross_die command
    if (ReportOptions.
        GetOption("binning",
                  "total_parts_used_for_percentage_computation").toString() ==
        "gross_die_if_available")
    {
        bool bOK;
        int  nGrossDie;
        if (lKeyword.toLower().startsWith("gross_die", Qt::CaseInsensitive))
        {
            bool bOK;
            int  nGrossDie = lAction.section('=', 1, 1).trimmed().toInt(&bOK);
            if (bOK)
            {
                m_nGrossDieCount = nGrossDie;
                GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("grossdie set").toLatin1().constData());
                return true;
            }
        }

        // Note: Vishay Eagle files have [0,15,"<cmd> GrossDieCount =8290",""]
        // instead of [<cmd> gross_die=8290]
        lCommand    = lString.trimmed().toLower();
        lCommandIndex = lCommand.indexOf("<cmd> gross_die");
        if (lCommandIndex >= 0)
        {
            // Parse command line
            lCommand = lCommand.mid(lCommandIndex);
            nGrossDie  =
                    lCommand.section(QRegExp("[=\"]"), 1, 1).trimmed().toInt(&bOK);
            if (bOK)
            {
                m_nGrossDieCount = nGrossDie;
                GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("grossdie set").toLatin1().constData());
                return true;
            }
        }
        lCommandIndex = lCommand.indexOf("<cmd> grossdiecount");
        if (lCommandIndex >= 0)
        {
            // Parse command line
            lCommand = lCommand.mid(lCommandIndex);
            nGrossDie  =
                    lCommand.section(QRegExp("[=\"]"), 1, 1).trimmed().toInt(
                        &bOK);
            if (bOK)
            {
                m_nGrossDieCount = nGrossDie;
                GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("grossdie set").toLatin1().constData());
                return true;
            }
        }
    }

    // Ignore multi-die command
    lCommandIndex = lCommand.indexOf("<cmd> multi-die");
    if(lCommandIndex >= 0)
        return true;

    // Ignore die-tracking command
    lCommandIndex = lCommand.indexOf("<cmd> die-tracking");
    if(lCommandIndex >= 0)
        return true;

    // Extract <VariableName> & <TestNumber>
    lAction = lAction.section(' ',1);
    // E.g: string format: 'setValue <VariableName>.<TestNumber> = <value>'
    QString lParameter = lAction.section('.',0,0);
    QString lStrTestNumber = lAction.section('.',1);
    lStrTestNumber = lStrTestNumber.section('=',0,0);
    lStrTestNumber = lStrTestNumber.trimmed();
    bool	lIsValid;
    lTestNumber = lStrTestNumber.toLong(&lIsValid);
    if(!lIsValid)
    {
        GSLOG(SYSLOG_SEV_NOTICE,
              QString("Failed to extract test number...string parsed may be corrupted")
              .toLatin1().constData());
        return true;
    }

    // Returns pointer to correct cell. If cell doesn't exist ; its created...Test# mapping enabled.
    if(FindTestCell(lTestNumber,GEX_PTEST, &lTestCell,true,true, lParameter.toLatin1().data()) !=1)
    {
        GSLOG(SYSLOG_SEV_ERROR,
              QString("Error when searching test: %1 - %2")
              .arg(QString::number(lTestNumber))
              .arg(lParameter)
              .toLatin1().constData());
        return true;	// Error
    }

    // Set test type
    GEX_ASSERT(lTestCell->bTestType == 'P');

    if(lKeyword.startsWith("setString", Qt::CaseInsensitive))
    {
        // String variable
    }
    if(lKeyword.startsWith("setValue", Qt::CaseInsensitive))
    {
        // Parametric variable
        QString lValue = lAction.section('=',1,1);
        lValue = lValue.trimmed();
        lfResult = lValue.toDouble(&lIsValid);
        if(!lIsValid)
        {
            GSLOG(SYSLOG_SEV_NOTICE,
                  QString("Failed to extract test result...string parsed may be corrupted")
                  .toLatin1().constData());
            return true;
        }

        // During pass#1: keep track of Min/Max so histogram cells can be computed in pass#2.
        if(lPass == 1)
            UpdateMinMaxValues(lTestCell,lfResult);

        if(lPass == 2)
        {
            // Command must be after a PIR and before a PRR
            GEX_ASSERT(PartProcessed.PIRNestedLevel() != 0);

            // collect sample to allow Interactive Charting (Unless size excees cutoff limit)
            if(pReportOptions->bSpeedCollectSamples)
            {
                if (AdvCollectTestResult(lTestCell, lfResult, CTestResult::statusUndefined, 0, true) == false)
                {
                    GSLOG(SYSLOG_SEV_ERROR, "Failed to collect test result");
                    return false;
                }
            }

#if 0 // Herve Thomy : // OBSOLETE MEMBER Not used anymore,since we hold the runIndex in the CPartBinning class except with MASA plugins
            // Keeps tracking the test# result.
            ptTestCell->ldCurrentSample++;
#endif

            // Build Cpk
            lTestCell->lfSamplesTotal += lfResult;				// Sum of X
            lTestCell->lfSamplesTotalSquare += (lfResult*lfResult);// Sum of X*X


            lTestCell->mHistogramData.AddValue(lfResult);
            if((lfResult >= lTestCell->GetCurrentLimitItem()->lfHistogramMin) && (lfResult <= lTestCell->GetCurrentLimitItem()->lfHistogramMax))
            {
                // Incluse result if it in into the viewing window.
                int lCell;
                if (lTestCell->GetCurrentLimitItem()->lfHistogramMax == lTestCell->GetCurrentLimitItem()->lfHistogramMin)
                {
                    lCell = 0;
                }
                else
                {
                    lCell = (int)( (TEST_HISTOSIZE*(lfResult - lTestCell->GetCurrentLimitItem()->lfHistogramMin)) / (lTestCell->GetCurrentLimitItem()->lfHistogramMax-lTestCell->GetCurrentLimitItem()->lfHistogramMin));
                }

                // Incluse result if it in into the viewing window.
                if( (lCell >= 0) && (lCell < TEST_HISTOSIZE) )
                    lTestCell->lHistogram[lCell]++;
                else if (lCell >= TEST_HISTOSIZE)
                    lTestCell->lHistogram[TEST_HISTOSIZE-1]++;   /// TO REVIEW : is it normal that iCell can exceed than TEST_HISTOSIZE
                else
                {
                    GSLOG(SYSLOG_SEV_ERROR,
                             QString("Invalid value detected to build histogram for test %1 [%2]")
                             .arg(lTestCell->lTestNumber)
                             .arg(lTestCell->strTestName).toLatin1().constData());

                    GSLOG(SYSLOG_SEV_DEBUG, QString("\tCell Found: %1").arg(lCell).toLatin1().constData());
                    GSLOG(SYSLOG_SEV_DEBUG, QString("\tDTR result: %1").arg( lfResult).toLatin1().constData());
                    GSLOG(SYSLOG_SEV_DEBUG, QString("\tHistogram Min: %1")
                          .arg(lTestCell->GetCurrentLimitItem()->lfHistogramMin).toLatin1().constData());
                    GSLOG(SYSLOG_SEV_DEBUG, QString("\tHistogram Max: %1").arg(
                             lTestCell->GetCurrentLimitItem()->lfHistogramMax).toLatin1().constData());
                }
            }
        }
    }
    else
    {
        GSLOG(SYSLOG_SEV_NOTICE,
              QString("Unsupported command line: %1")
              .arg(lKeyword)
              .toLatin1().constData());
        return true;
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////
// Reduces Test name if to long, remove leading spaces, etc...
/////////////////////////////////////////////////////////////////////////////
char* CGexFileInGroup::FormatTestName(char *szString)
{
    if (m_OutputFormat == GEX_OPTION_OUTPUT_CSV)
        RemoveComas(szString); // Remove comas that would corrupt .CSV parsing separator !

    return szString;
}

/////////////////////////////////////////////////////////////////////////////
// Reduces Test name if to long, remove leading spaces, etc...
/////////////////////////////////////////////////////////////////////////////
void CGexFileInGroup::FormatTestName(QString & strString)
{
    if(m_OutputFormat == GEX_OPTION_OUTPUT_CSV)
        strString.replace(',',';');		// Remove comas that would corrupt .CSV parsing separator !
}

/////////////////////////////////////////////////////////////////////////////
// Read STDF PDR record : STDF V3 only (Description of tests).
/////////////////////////////////////////////////////////////////////////////
void CGexFileInGroup::ReadPDR(void)
{
    int		iIndex;
    char	szString[257];	// A STDF string is 256 bytes long max!
    CTest	*ptTestCell;	// Pointer to test cell to receive STDF info.
    BYTE	bOptFlag;
    BOOL	bUseLocalLL=true, bUseLocalHL=true;
    double	lfLowLimit=0.0, lfHighLimit=0.0;

    // PDR
    if(lPass != 1)
        return;

    StdfFile.ReadDword(&lData);	// test number
    // Returns pointer to correct cell. If cell doesn't exist ; its created...Test# mapping enabled.
    if(FindTestCell(lData, GEX_PTEST,&ptTestCell) !=1)
        return;	// Error

    // DESC_FLG: ignore
    StdfFile.ReadByte(&bData);

    // opt_flag
    if(StdfFile.ReadByte(&bData) != GS::StdLib::Stdf::NoError)
        return;
    bOptFlag = bData & 0x03;

    // Force non-strict limits
    ptTestCell->GetCurrentLimitItem()->bLimitFlag |= CTEST_LIMITFLG_LTLNOSTRICT;
    ptTestCell->GetCurrentLimitItem()->bLimitFlag |= CTEST_LIMITFLG_HTLNOSTRICT;

    // Save limit flag
    ptTestCell->GetCurrentLimitItem()->bLimitFlag &= ~(CTEST_LIMITFLG_NOLTL |CTEST_LIMITFLG_NOHTL);	// Erase bit0,bit1
    ptTestCell->GetCurrentLimitItem()->bLimitFlag |= bOptFlag;

    // If What-if has forced a limit, clear the corresponding limit flag
    if((ptTestCell->GetCurrentLimitItem()->bLimitWhatIfFlag & CTEST_LIMITFLG_NOLTL) == 0)
    {
        bUseLocalLL = false;
        ptTestCell->GetCurrentLimitItem()->bLimitFlag &= ~(CTEST_LIMITFLG_NOLTL);
    }
    if((ptTestCell->GetCurrentLimitItem()->bLimitWhatIfFlag & CTEST_LIMITFLG_NOHTL) == 0)
    {
        bUseLocalHL = false;
        ptTestCell->GetCurrentLimitItem()->bLimitFlag &= ~(CTEST_LIMITFLG_NOHTL);
    }

    // res_scal: ignore
    if(StdfFile.ReadByte(&bData) != GS::StdLib::Stdf::NoError)
        return;

    // Get units.
    for(iIndex=0;iIndex<7;iIndex++)
        if(StdfFile.ReadByte((BYTE *)&szString[iIndex]) != GS::StdLib::Stdf::NoError)
            return;
    szString[7] = 0;
    if(*szString)
        strcpy(ptTestCell->szTestUnits,szString);

    // res_ldig: ignore
    if(StdfFile.ReadByte(&bData) != GS::StdLib::Stdf::NoError)
        return;

    // res_rdig: ignore
    if(StdfFile.ReadByte(&bData) != GS::StdLib::Stdf::NoError)
        return;

    // llm_scal
    if(StdfFile.ReadByte(&bData) != GS::StdLib::Stdf::NoError)
        return;
    ptTestCell->llm_scal = bData;

    // hlm_scal
    if(StdfFile.ReadByte(&bData) != GS::StdLib::Stdf::NoError)
        return;
    ptTestCell->hlm_scal = bData;

    // llm_ldig: ignore
    if(StdfFile.ReadByte(&bData) != GS::StdLib::Stdf::NoError)
        return;
    // llm_rdig: ignore
    if(StdfFile.ReadByte(&bData) != GS::StdLib::Stdf::NoError)
        return;
    // hlm_ldig: ignore
    if(StdfFile.ReadByte(&bData) != GS::StdLib::Stdf::NoError)
        return;
    // hlm_rdig: ignore
    if(StdfFile.ReadByte(&bData) != GS::StdLib::Stdf::NoError)
        return;

    // low limit
    if(StdfFile.ReadFloat(&fData) != GS::StdLib::Stdf::NoError)
        return;
    if(bUseLocalLL)
    {
        if(bOptFlag & 0x01)
            lfLowLimit = -C_INFINITE;
        else
            lfLowLimit = M_CHECK_INFINITE((double)fData);	// Low limit exists: keep value
    }

    // high limit
    if(StdfFile.ReadFloat(&fData) != GS::StdLib::Stdf::NoError)
    {
        if(bUseLocalLL)
            ptTestCell->GetCurrentLimitItem()->lfLowLimit = lfLowLimit;
        return;
    }
    if(bUseLocalHL)
    {
        if(bOptFlag & 0x02)
            lfHighLimit = C_INFINITE;
        else
            lfHighLimit = M_CHECK_INFINITE((double)fData);	// High limit exists: keep value
    }

    // Save limits. If limits are in wrong order, swap them !
    if(bUseLocalLL && bUseLocalHL && (lfLowLimit > lfHighLimit))
    {
        ptTestCell->GetCurrentLimitItem()->lfHighLimit = lfLowLimit;
        ptTestCell->GetCurrentLimitItem()->lfLowLimit = lfHighLimit;
    }
    if(bUseLocalLL)
        ptTestCell->GetCurrentLimitItem()->lfLowLimit = lfLowLimit;
    if(bUseLocalHL)
        ptTestCell->GetCurrentLimitItem()->lfHighLimit = lfHighLimit;

    // text description (test name)
    if(StdfFile.ReadString(szString)  != GS::StdLib::Stdf::NoError)
        return;

    // Formats test name (in case to long, or leading spaces)
    FormatTestName(szString);

    ptTestCell->strTestName = szString;
}

/////////////////////////////////////////////////////////////////////////////
// Datalog: Check and perform page break if needed
/////////////////////////////////////////////////////////////////////////////
void CGexFileInGroup::WriteDatalogCheckPageBreak(int /*iSite*/, int /*iHead*/)
{
    // Handle may be NULL if a datalog is requested, while Report output is 'interactive only'!
    if(hAdvancedReport == NULL)
        return;

    // Only applies to standard HTML pages (not FLAT HTML!)
    if(m_OutputFormat != GEX_OPTION_OUTPUT_HTML)
        return;

    // Check if we exceed the allowed lines# per page...if so change to new page
    if(pReportOptions->lAdvancedHtmlLinesInPage < MAX_DATALOG_PERPAGE)
        return;

    char	szString[2048];
    long	iCurrentHtmlPage=pReportOptions->lAdvancedHtmlPages;

    // Close current page, open next page.
    fprintf(hAdvancedReport,"</table>\n");
    fprintf(hAdvancedReport,"<br><br><br><table border=\"0\" width=\"98%%\">\n");
    fprintf(hAdvancedReport,"<tr>\n");

    if(iCurrentHtmlPage == 1)
        strcpy(szString,"advanced");
    else
        sprintf(szString,"advanced%ld",iCurrentHtmlPage);
    fprintf(hAdvancedReport,"<td width=\"50%%\"><a href=\"%s.htm\"><img src=\"../images/prev.png\" alt=\"Previous report page\" border=\"0\" width=\"58\" height=\"31\"></a></td>\n",szString);
    sprintf(szString,"advanced%ld",iCurrentHtmlPage+1);
    fprintf(hAdvancedReport,"<td width=\"50%%\"><p align=\"right\"><a href=\"%s.htm\"><img src=\"../images/next.png\" alt=\"Next report page\" border=\"0\" width=\"51\" height=\"30\"></a></td>\n",
            szString);
    fprintf(hAdvancedReport,"</tr>\n</table>\n");

    // Close page
    fprintf(hAdvancedReport,C_HTML_FOOTER, GS::Gex::Engine::GetInstance().Get("AppFullName").toString().toLatin1().data() );	// Writes HTML footer Application name/web site string
    fprintf(hAdvancedReport,"</body>\n");
    fprintf(hAdvancedReport,"</html>\n");
    fclose(hAdvancedReport);

    // Remove next page...it is the only way to avoid 'Print' include dummy pages!
    sprintf(szString,"%s/pages/advanced%ld.htm",pReportOptions->strReportDirectory.toLatin1().constData(),iCurrentHtmlPage+2);
    remove(szString);

    // Open <stdf-filename>/report/<szPageName>.htm
    sprintf(szString,"%s/pages/advanced%ld.htm",pReportOptions->strReportDirectory.toLatin1().constData(),iCurrentHtmlPage+1);
    hAdvancedReport = fopen(szString,"wt"); //hAdvancedReport is now updated.
    if(hAdvancedReport == NULL)
        return;

    fprintf(hAdvancedReport,"<html>\n");
    fprintf(hAdvancedReport,"<head>\n");
    fprintf(hAdvancedReport,"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=ISO-8859-1\">\n");
    fprintf(hAdvancedReport,"</head>\n");
    // Sets default background color = white, text color given in argument.
    fprintf(hAdvancedReport,"<body bgcolor=\"#FFFFFF\" text=\"#000000\">\n");	// Default color in HTML page!

    fprintf(hAdvancedReport,"<h1 align=\"left\"><font color=\"#006699\">Datalog</font></h1><br>\n");
    fprintf(hAdvancedReport,"<img border=\"0\" src=\"../images/ruler.png\" width=\"616\" height=\"8\"><br>\n");
    fprintf(hAdvancedReport,"</table>\n");
    fprintf(hAdvancedReport,"<table border=\"0\" width=\"98%%\">\n");
    fprintf(hAdvancedReport,"<tr>\n");
    if(iCurrentHtmlPage <= 1)
        strcpy(szString,"advanced");
    else
        sprintf(szString,"advanced%ld",iCurrentHtmlPage);
    fprintf(hAdvancedReport,"<td width=\"33%%\"><a href=\"%s.htm\"><img src=\"../images/prev.png\" alt=\"Previous report page\" border=\"0\" width=\"58\" height=\"31\"></a></td>\n",
            szString);
    fprintf(hAdvancedReport,"<td></td>\n");

    // Middle table cell (empty in header)
    fprintf(hAdvancedReport,"<td width=\"33%%\">&nbsp;</td>\n");

    // 'next' button
    sprintf(szString,"advanced%ld",iCurrentHtmlPage+2);
    fprintf(hAdvancedReport,"<td width=\"33%%\"><p align=\"right\"><a href=\"%s.htm\"><img src=\"../images/next.png\" alt=\"Next report page\" border=\"0\" width=\"51\" height=\"30\"></a></td>\n",szString);
    fprintf(hAdvancedReport,"</tr>\n</table>\n");
    fprintf(hAdvancedReport,"<table border=\"0\" width=\"98%%\"	 cellspacing=\"0\" cellpadding=\"0\">\n");

    // Reset counter
    pReportOptions->lAdvancedHtmlLinesInPage = 0;
    pReportOptions->lAdvancedHtmlPages++;
}

/////////////////////////////////////////////////////////////////////////////
// Datalog par header: tells which part currently dataloged
/////////////////////////////////////////////////////////////////////////////
void CGexFileInGroup::WriteDatalogPartHeader(int iSiteID, int iHead)
{
    // Handle may be NULL if a datalog is requested, while Report output is 'interactive only'!
    if(hAdvancedReport == NULL)
        return;


    if(!pReportOptions)
        return ;

    QDir oTempDir(GS::Gex::Engine::GetInstance().Get("GalaxySemiFolder").toString()
                  + QDir::separator()+ "temp" + QDir::separator()+ "datalog");
    if(iHead !=-1)
    {
        if(!oTempDir.exists())
            if(!oTempDir.mkpath(GS::Gex::Engine::GetInstance().Get("GalaxySemiFolder").toString()
                                + QDir::separator()+ "temp" + QDir::separator()+ "datalog"))
                return;
    }

    QString strTempFile = GS::Gex::Engine::GetInstance().Get("GalaxySemiFolder").toString()
                          + QDir::separator()+ "temp"+ QDir::separator()
                          + "datalog" + QDir::separator()+ QString("datalog_%1_%2").arg(iHead).arg(iSiteID);

    FILE *poTempHandle = 0;

    // Generate the intermidate file in all the case it will be used when calling AdvCollectDatalog and WriteDatalogBinResult
    if(iHead !=-1){
        if(!QFile::exists(strTempFile)){
            poTempHandle = fopen(strTempFile.toLatin1().constData(),"w+");
        }else {
            poTempHandle = fopen(strTempFile.toLatin1().constData(),"a+");
        }

        if(!poTempHandle){
            GSLOG(SYSLOG_SEV_DEBUG,QString("Generating file %1 with fail").arg(strTempFile).toLatin1().constData());
            return;
        }
        fclose(poTempHandle);
        GSLOG(SYSLOG_SEV_DEBUG,QString("Generating file %1 with success").arg(strTempFile).toLatin1().constData());
    }

    // Check if page break needed (HTML only)
    WriteDatalogCheckPageBreak();

    // If RAW datalog, do not write header!
    if(pReportOptions->getAdvancedReportSettings() == GEX_ADV_DATALOG_RAWDATA)
        return;

    if(m_OutputFormat == GEX_OPTION_OUTPUT_CSV)
    {
        // Generating .CSV report file.
        fprintf(hAdvancedReport,"Datalog# %d,Lot: %s,Sublot: %s,Wafer: %s\n",PartProcessed.partNumber(iSiteID),getMirDatas().szLot,getMirDatas().szSubLot,getWaferMapData().szWaferID);
    }
    else if (m_OutputFormat & GEX_OPTION_OUTPUT_HTML_BASED)
    {
        fprintf(hAdvancedReport,"<tr>\n");
        fprintf(hAdvancedReport,"<td width=\"10%%\"><b>Datalog#:</b></td>\n");
        fprintf(hAdvancedReport,"<td width=\"15%%\"><b>%d</b></td>\n",PartProcessed.partNumber(iSiteID));

        if(*getMirDatas().szLot)
            fprintf(hAdvancedReport,"<td width=\"25%%\" align=\"right\"><b>Lot: %s</b></td>\n",getMirDatas().szLot);
        else
            fprintf(hAdvancedReport,"<td width=\"25%%\"></td>\n");

        if(*getMirDatas().szSubLot)
            fprintf(hAdvancedReport,"<td width=\"10%%\" align=\"right\"><b>Sublot: %s</b></td>\n",getMirDatas().szSubLot);
        else
            fprintf(hAdvancedReport,"<td width=\"10%%\"></td>\n");

        if(*getWaferMapData().szWaferID)
            fprintf(hAdvancedReport,"<td width=\"15%%\" align=\"right\"><b>Wafer:</b> %s</td>\n",getWaferMapData().szWaferID);
        else
            fprintf(hAdvancedReport,"<td width=\"15%%\"></td>\n");
        fprintf(hAdvancedReport,"<td width=\"25%%\" align=\"right\"></td>\n");
        fprintf(hAdvancedReport,"</tr>\n");
    }
    lTestsInDatalog+=2;	// Next test result to datalog will start on a even column.
    pReportOptions->lAdvancedHtmlLinesInPage++;	// Update HTML line count.

}

/////////////////////////////////////////////////////////////////////////////
// Datalog test result
/////////////////////////////////////////////////////////////////////////////
void CGexFileInGroup::AdvCollectDatalog(CTest *ptTestCell,float fValue,bool bFailResult,bool bAlarmResult,bool bIsOutlier, int iSiteID, int iHead, int iSite)
{

    // Handle may be NULL if a datalog is requested, while Report output is 'interactive only'!
    if(hAdvancedReport == NULL)
        return;
    if(!pReportOptions)
        return ;

    QDir oTempDir(GS::Gex::Engine::GetInstance().Get("GalaxySemiFolder").toString()
                  + QDir::separator()+ "temp" + QDir::separator()+ "datalog");
    if(!oTempDir.exists())
    {
        if(!oTempDir.mkpath(GS::Gex::Engine::GetInstance().Get("GalaxySemiFolder").toString()
                            + QDir::separator()+ "temp" + QDir::separator()+ "datalog"))
        {
            GSLOG(SYSLOG_SEV_DEBUG,
                  QString("Generating temp file path %1 with fail")
                  .arg(pReportOptions->strReportDirectory + QDir::separator()+ "temp" + QDir::separator()+ "datalog").toLatin1().constData());
            return;
        }

        GSLOG(SYSLOG_SEV_DEBUG,
              QString("Generating temp file path %1 with success")
              .arg(pReportOptions->strReportDirectory + QDir::separator()+ "temp" + QDir::separator()+ "datalog").toLatin1().constData());
    }

    QString strTempFile = GS::Gex::Engine::GetInstance().Get("GalaxySemiFolder").toString()
                          + QDir::separator()+ "temp" + QDir::separator()+ "datalog" +
                          QDir::separator()+ QString("datalog_%1_%2").arg(iHead).arg(iSite);
    FILE *poOriAdvancedHandle = hAdvancedReport; //poOriAdvancedHandle handle to store the original hendle on  hAdvancedReport

    switch(pReportOptions->getAdvancedReportSettings())
    {
        case GEX_ADV_DATALOG_ALL:	// ALL tests
        default:
            if(bIsOutlier == true){
                return;
            }
            break;					// Just continue in this function !
        case GEX_ADV_DATALOG_FAIL:	// Only FAIL tests
            if((bFailResult == false) || (bIsOutlier == true)){
                return;
            }
                //return;				// Test is PASS: do not Datalog !

            break;
        case GEX_ADV_DATALOG_OUTLIER:
            // Swap Outlier flag so we can fall into next case.
            if(bIsOutlier == false)
                bIsOutlier = true;
            else
                bIsOutlier = false;
            // MUST FALL INTO NEXT CASES!
        case GEX_ADV_DATALOG_LIST:	// Tests between given range
        case GEX_ADV_DATALOG_RAWDATA:// Tests between given range: raw data
            if(bIsOutlier == true){
                return;
            }
                //return;
            if(pReportOptions->pGexAdvancedRangeList->IsTestInList(ptTestCell->lTestNumber,ptTestCell->lPinmapIndex))
                break;
            else{
                return;
            }
               // return;	// Test out of limits ! or Test PinmapIndex# out of limit!
    }

    if(lTestsInDatalog == 0)
    {
        hAdvancedReport = poOriAdvancedHandle; //Reset hAdvancedReport to original value in case it is changed
        // First test datalogged in part, and not RAW datalog.
        WriteDatalogPartHeader(iSiteID, iHead);
        poOriAdvancedHandle = hAdvancedReport; //Update poOriAdvancedHandle  in case WriteDatalogPartHeader change the handle hAdvancedReport
    }

    FILE *poTempHandle = 0;
    if(!QFile::exists(strTempFile)){
        poTempHandle = fopen(strTempFile.toLatin1().constData(),"w+");
    }else {
        poTempHandle = fopen(strTempFile.toLatin1().constData(),"a+");
    }
    if(!poTempHandle){
        GSLOG(SYSLOG_SEV_DEBUG,QString("Generating file %1 with fail").arg(strTempFile).toLatin1().constData());
        return;
    }

    GSLOG(SYSLOG_SEV_DEBUG,QString("Generating file %1 with success").arg(strTempFile).toLatin1().constData());
    hAdvancedReport = poTempHandle; //Update hAdvancedReport so datalog will be generated into the temp file poTempHandle

    if(m_OutputFormat == GEX_OPTION_OUTPUT_CSV)
    {
        // Generating .CSV report file.
        // Check if have to display testlabel
        if(m_eDatalogTableOptions&ADF_TEST_NUMBER)
        {
            gexReport->BuildTestNumberString(ptTestCell);
            fprintf(hAdvancedReport,"T%s,",ptTestCell->szTestLabel);
        }

        // Check if have to display testlabel
        if( (m_eDatalogTableOptions&ADF_TEST_NAME) && (pReportOptions->getAdvancedReportSettings()!=GEX_ADV_DATALOG_RAWDATA))
        {
            QString strTestName;
            gexReport->BuildTestNameString(this, ptTestCell, strTestName);
            fprintf(hAdvancedReport,"%s,", strTestName.toLatin1().constData());
        }

        // Check if have to display limits
        if( (m_eDatalogTableOptions&ADF_LIMITS) && (pReportOptions->getAdvancedReportSettings()!=GEX_ADV_DATALOG_RAWDATA))
        {
            if((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0)
            {
                FormatTestLimit(ptTestCell,
                                ptTestCell->GetCurrentLimitItem()->szLowL,
                                ptTestCell->GetCurrentLimitItem()->lfLowLimit,
                                ptTestCell->llm_scal);

                if(m_eDatalogFormat==ONE_ROW)
                {
                    // Display 'strict' or 'not strict' Low limit
                    if(ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_LTLNOSTRICT)
                        fprintf(hAdvancedReport,"%s, <=,",ptTestCell->GetCurrentLimitItem()->szLowL);	// Not a strict limit (i.e: '<=')
                    else
                        fprintf(hAdvancedReport,"%s, <,",ptTestCell->GetCurrentLimitItem()->szLowL);	// strict limit (i.e: '<=')
                }
                else
                    fprintf(hAdvancedReport,"LowL:, %s,",ptTestCell->GetCurrentLimitItem()->szLowL);
            }else{
                if(m_eDatalogFormat==ONE_ROW)
                    fprintf(hAdvancedReport,",,,");
                else
                    fprintf(hAdvancedReport,",,,");
            }

            if((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0)
            {
                FormatTestLimit(ptTestCell,
                                ptTestCell->GetCurrentLimitItem()->szHighL,
                                ptTestCell->GetCurrentLimitItem()->lfHighLimit,
                                ptTestCell->hlm_scal);
                if (m_eDatalogFormat!=ONE_ROW)
                    fprintf(hAdvancedReport,"HighL:, %s,",ptTestCell->GetCurrentLimitItem()->szHighL);
            }else{
                if(m_eDatalogFormat!=ONE_ROW)
                    fprintf(hAdvancedReport,",,,");
            }
        }

        fprintf(hAdvancedReport,"%s,", FormatTestResult(ptTestCell,fValue,ptTestCell->res_scal));
        if(bFailResult == true)
            fprintf(hAdvancedReport," *F,");
        else
            if(bAlarmResult == true)
                fprintf(hAdvancedReport," *A,");
            else
                fprintf(hAdvancedReport," ,");

        // Single row mode: display high limit now.
        if (m_eDatalogFormat==ONE_ROW)
        {
            if((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0)
            {
                // Display 'strict' or 'not strict' Hightlimit
                if(ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_HTLNOSTRICT)
                    fprintf(hAdvancedReport,"<=, %s,",ptTestCell->GetCurrentLimitItem()->szHighL);	// Not a strict limit (i.e: '<=')
                else
                    fprintf(hAdvancedReport,"<, %s,",ptTestCell->GetCurrentLimitItem()->szHighL);	// strict limit (i.e: '<=')
            }else
                fprintf(hAdvancedReport,",,,");
        }

        // If Test is in second column...or we must only create one row
        if((lTestsInDatalog % 2) || (m_eDatalogFormat==ONE_ROW))
            fprintf(hAdvancedReport,"\n");
    }
    else if (m_OutputFormat & GEX_OPTION_OUTPUT_HTML_BASED)
    {
        QString	strBookmark;

        // Check if page break needed
        hAdvancedReport = poOriAdvancedHandle; // Reset hAdvancedReport to it is original value in case we write a page break.
        WriteDatalogCheckPageBreak();
        poOriAdvancedHandle = hAdvancedReport; // Update the poOriAdvancedHandle with the new hAdvancedReport created by WriteDatalogCheckPageBreak
        hAdvancedReport = poTempHandle; // Come back to poTempHandle because we does not finish writing this temp datalog into poTempHandle

        // Bookmark: are in same page if FLAT HTML page is generated
        if(m_OutputFormat == GEX_OPTION_OUTPUT_HTML)
            strBookmark.sprintf("stats%d.htm#StatT",ptTestCell->iHtmlStatsPage);
        else
            strBookmark = "#StatT";	// Test Statistics bookmark header string.

        // Datalog current part in the HTML datalog file.
        if(lTestsInDatalog % 2)
        {

            // Test in in second column
            fprintf(hAdvancedReport,"<td width=\"10%%\"></td>\n");

            // Check if have to display test number
            if(m_eDatalogTableOptions&ADF_TEST_NUMBER)
            {
                if((bFailResult == true) && (ptTestCell->iHtmlStatsPage>0))
                    fprintf(hAdvancedReport,"<td width=\"15%%\"><b><a name=\"T%s\"></a> <a href=\"%s%s\">T%s</a>: ",
                            ptTestCell->szTestLabel,strBookmark.toLatin1().constData(),ptTestCell->szTestLabel,ptTestCell->szTestLabel);
                else
                    fprintf(hAdvancedReport,"<td width=\"15%%\">T%s: ",ptTestCell->szTestLabel);
            }
            else
                fprintf(hAdvancedReport,"<td width=\"15%%\">");

            // Check if have to display testlabel
            if( (m_eDatalogTableOptions&ADF_TEST_NAME) && (pReportOptions->getAdvancedReportSettings()!=GEX_ADV_DATALOG_RAWDATA))
            {
                QString strTestName;
                gexReport->BuildTestNameString(this, ptTestCell, strTestName);
                fprintf(hAdvancedReport,"%s", strTestName.toLatin1().constData());
            }

            // Check if have to display limits
            if( (m_eDatalogTableOptions&ADF_LIMITS) && (pReportOptions->getAdvancedReportSettings()!=GEX_ADV_DATALOG_RAWDATA))
            {
                if((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0)
                {
                    FormatTestLimit(ptTestCell,
                                    ptTestCell->GetCurrentLimitItem()->szLowL,
                                    ptTestCell->GetCurrentLimitItem()->lfLowLimit,
                                    ptTestCell->llm_scal);
                    fprintf(hAdvancedReport,"<br>LowL: %s",ptTestCell->GetCurrentLimitItem()->szLowL);
                }
                if((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0)
                {
                    FormatTestLimit(ptTestCell,
                                    ptTestCell->GetCurrentLimitItem()->szHighL,
                                    ptTestCell->GetCurrentLimitItem()->lfHighLimit,
                                    ptTestCell->hlm_scal);
                    fprintf(hAdvancedReport,"<br>HighL: %s",ptTestCell->GetCurrentLimitItem()->szHighL);
                }
            }
            fprintf(hAdvancedReport,"</td>\n");

            fprintf(hAdvancedReport,"<td width=\"25%%\" align=\"right\">");
            if(bFailResult == true || bAlarmResult == true)
                fprintf(hAdvancedReport,"<font color=\"#FF0000\"><b>");
            fprintf(hAdvancedReport,"%s",FormatTestResult(ptTestCell,fValue,ptTestCell->res_scal));
            if(bFailResult == true)
                fprintf(hAdvancedReport," *F</b></font>");
            else
                if(bAlarmResult == true)
                    fprintf(hAdvancedReport," *A</b></font>");
            fprintf(hAdvancedReport,"</td>\n");
            fprintf(hAdvancedReport,"</tr>\n");
        }
        else
        {
            if(lTestsInDatalog == 0)
            {
                // First test datalogged in part, and not RAW datalog.
                WriteDatalogPartHeader(iSiteID);
            }

            // Test is in first column
            fprintf(hAdvancedReport,"<tr>\n");
            fprintf(hAdvancedReport,"<td width=\"10%%\"></td>\n");

            // Check if have to display test number
            if(m_eDatalogTableOptions&ADF_TEST_NUMBER)
            {
                if((bFailResult == true) && (ptTestCell->iHtmlStatsPage>0))
                    fprintf(hAdvancedReport,"<td width=\"15%%\"><b><a name=\"T%s\"></a> <a href=\"%s%s\">T%s</a></b>: ",
                            ptTestCell->szTestLabel,strBookmark.toLatin1().constData(),ptTestCell->szTestLabel,ptTestCell->szTestLabel);
                else
                    fprintf(hAdvancedReport,"<td width=\"15%%\">T%s: ",ptTestCell->szTestLabel);
            }
            else
                fprintf(hAdvancedReport,"<td width=\"15%%\">");

            // Check if have to display testlabel
            if( (m_eDatalogTableOptions&ADF_TEST_NAME) && (pReportOptions->getAdvancedReportSettings()!=GEX_ADV_DATALOG_RAWDATA))
            {
                if (m_eDatalogFormat==ONE_ROW)
                    fprintf(hAdvancedReport,"</td><td>\n");

                QString strTestName;
                gexReport->BuildTestNameString(this, ptTestCell, strTestName);
                strTestName.replace("<","&lt;");
                strTestName.replace(">","&gt;");
                fprintf(hAdvancedReport,"%s", strTestName.toLatin1().constData());
            }
            if (m_eDatalogFormat==ONE_ROW)
            {
                // In one column mode, use more space to display info!
                fprintf(hAdvancedReport,"</td>\n");
                fprintf(hAdvancedReport,"<td width=\"25%%\" align=\"right\">");
            }

            // Check if have to display limits
            if( (m_eDatalogTableOptions&ADF_LIMITS) && (pReportOptions->getAdvancedReportSettings()!=GEX_ADV_DATALOG_RAWDATA))
            {
                if((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0)
                {
                    FormatTestLimit(ptTestCell,
                                    ptTestCell->GetCurrentLimitItem()->szLowL,
                                    ptTestCell->GetCurrentLimitItem()->lfLowLimit,
                                    ptTestCell->llm_scal);
                    if (m_eDatalogFormat==TWO_ROWS)
                    {
                        // In 2 column mode, use less space to display info (display limits with test name)!
                        fprintf(hAdvancedReport,"<br>LowL: ");
                    }
                    fprintf(hAdvancedReport,"%s  ",ptTestCell->GetCurrentLimitItem()->szLowL);
                    if (m_eDatalogFormat==ONE_ROW)
                    {
                        // Display 'strict' or 'not strict' Low limit
                        if(ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_LTLNOSTRICT)
                            fprintf(hAdvancedReport,"&lt;= &nbsp;&nbsp; </td><td align=\"center\">");	// Not a strict limit (i.e: '<=')
                        else
                            fprintf(hAdvancedReport,"&lt; &nbsp;&nbsp; </td><td align=\"center\">");	// strict limit (i.e: '<=')
                    }
                }else{
                    if(m_eDatalogFormat==TWO_ROWS)
                        fprintf(hAdvancedReport," ");
                    if (m_eDatalogFormat==ONE_ROW)
                    {
                        fprintf(hAdvancedReport," </td><td align=\"center\">");
                    }

                }

                if((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0)
                {
                    FormatTestLimit(ptTestCell,
                                    ptTestCell->GetCurrentLimitItem()->szHighL,
                                    ptTestCell->GetCurrentLimitItem()->lfHighLimit,
                                    ptTestCell->hlm_scal);
                    if(m_eDatalogFormat==TWO_ROWS)
                    {
                        // In 2 column mode, use less space to display info (display limits with test name)!
                        fprintf(hAdvancedReport,"<br>HighL: %s",ptTestCell->GetCurrentLimitItem()->szHighL);
                    }
                }else{
                    if(m_eDatalogFormat==TWO_ROWS)
                        fprintf(hAdvancedReport," ");
                }
            }
            else if (m_eDatalogFormat==ONE_ROW)
            {
                // In one column mode, use more space to display info!
                fprintf(hAdvancedReport,"</td>\n");
                fprintf(hAdvancedReport,"<td align=\"center\">");
            }

            if (m_eDatalogFormat==TWO_ROWS)
            {
                fprintf(hAdvancedReport,"</td>\n");
                fprintf(hAdvancedReport,"<td width=\"25%%\" align=\"right\">");
            }

            if(bFailResult == true || bAlarmResult == true)
                fprintf(hAdvancedReport,"<font color=\"#FF0000\"><b>");
            fprintf(hAdvancedReport,"%s",FormatTestResult(ptTestCell,fValue,ptTestCell->res_scal));
            if(bFailResult == true)
                fprintf(hAdvancedReport," *F</b></font>");
            else
                if(bAlarmResult == true)
                    fprintf(hAdvancedReport," *A</b></font>");
            fprintf(hAdvancedReport,"</td>\n");

            // Single row mode: display high limit now.
            if (m_eDatalogFormat==ONE_ROW)
            {
                if( (m_eDatalogTableOptions&ADF_LIMITS) && ((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0))
                {
                    // Display 'strict' or 'not strict' Hightlimit
                    if(ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_HTLNOSTRICT)
                        fprintf(hAdvancedReport,"<td>&lt;= %s</td>\n",ptTestCell->GetCurrentLimitItem()->szHighL);	// Not a strict limit (i.e: '<=')
                    else
                        fprintf(hAdvancedReport,"<td>&lt; %s</td>\n",ptTestCell->GetCurrentLimitItem()->szHighL);	// strict limit (i.e: '<=')
                }else if((m_eDatalogTableOptions&ADF_LIMITS) && ((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) != 0)){
                    fprintf(hAdvancedReport,"<td> </td>\n");	// Not a strict limit (i.e: '<=')
                }
            }

        }
    }

    // Update HTML line count.
    if(lTestsInDatalog % 2)
        pReportOptions->lAdvancedHtmlLinesInPage++;

    // Keeps track of the number of tests datalogged in part
    lTestsInDatalog++;

    // Check if we must only fill a single ROW.
    if( (m_eDatalogFormat==ONE_ROW) && (lTestsInDatalog % 2) && (m_OutputFormat & GEX_OPTION_OUTPUT_HTML_BASED) )
    {
        // Make sure we only fill one row if this is the setting...
        // Test in in second column...so tab until on next line
        fprintf(hAdvancedReport,"</tr>\n");
        lTestsInDatalog++;
    }

    fclose(poTempHandle); // Finish with poTempHandle close it
    hAdvancedReport = poOriAdvancedHandle; // Restore hAdvancedReport to be used with the rest of the report

}

/////////////////////////////////////////////////////////////////////////////
// Check if test result in an outlier!
/////////////////////////////////////////////////////////////////////////////
bool CGexFileInGroup::PartIsOutlier(CTest *ptTestCell,float fValue)
{
    // Check if we have no outlier removal activated
    if (m_eOutlierRemoveMode==CGexFileInGroup::NONE)
        return false;

    // We have the outlier removal activated, then let's see if test passes!
    if(lPass == 1)
    {
        // Pass 1: we may have to compute the limits...
        if(!ptTestCell->bOutlierLimitsSet)
        {
            // NEED to Compute outlier limits
            // Both limits exist.
            double lfOffset = 0.0;	// Offset to add on both sides of test limits.
            switch(m_eOutlierRemoveMode)
            {
                case CGexFileInGroup::N_IQR:	// IQR filtering can only be done AFTER all data are loaded...
                    return false;

                case CGexFileInGroup::N_SIGMA:
                case CGexFileInGroup::EXCLUDE_N_SIGMA:
                    return false;	// Sigma outlier filter is only activated on PASS2.

                case CGexFileInGroup::NONE:
                    lfOffset = 0.0;
                    break;

                case CGexFileInGroup::N_POURCENT:
                    float r=(m_dOutlierRemovalValue-100.f)/200.f;
                    if (r<0.f) r=0.f; if (r>1.f) r=1.f;
                    lfOffset = r * (ptTestCell->GetCurrentLimitItem()->lfHighLimit-ptTestCell->GetCurrentLimitItem()->lfLowLimit);
                    break;

            }

            if((ptTestCell->GetCurrentLimitItem()->bLimitFlag & (CTEST_LIMITFLG_NOLTL | CTEST_LIMITFLG_NOHTL)) == 0)
            {
                // Test has High and Low limits.
                ptTestCell->GetCurrentLimitItem()->lfLowLimitOutlier = ptTestCell->GetCurrentLimitItem()->lfLowLimit - lfOffset;
                ptTestCell->GetCurrentLimitItem()->lfHighLimitOutlier = ptTestCell->GetCurrentLimitItem()->lfHighLimit + lfOffset;
            }
            else
                if((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0)
                {
                    // HIGH limit only. show outlier = "n/a"
                    ptTestCell->GetCurrentLimitItem()->ldOutliers = -1;
                    ptTestCell->GetCurrentLimitItem()->lfLowLimitOutlier = -C_INFINITE;
                    ptTestCell->GetCurrentLimitItem()->lfHighLimitOutlier = ptTestCell->GetCurrentLimitItem()->lfHighLimit;
                }
                else
                    if((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0)
                    {
                        // LOW limit only. show outlier = "n/a"
                        ptTestCell->GetCurrentLimitItem()->ldOutliers = -1;
                        ptTestCell->GetCurrentLimitItem()->lfLowLimitOutlier = ptTestCell->GetCurrentLimitItem()->lfLowLimit;
                        ptTestCell->GetCurrentLimitItem()->lfHighLimitOutlier = C_INFINITE;
                    }
                    else
                    {
                        // No limits...can't compute outlier!
                        ptTestCell->GetCurrentLimitItem()->lfLowLimitOutlier = -C_INFINITE;
                        ptTestCell->GetCurrentLimitItem()->lfHighLimitOutlier = C_INFINITE;
                    }
            ptTestCell->bOutlierLimitsSet = true;
        }
    }

    // Test if test is an outlier / Inlier...
    switch(m_eOutlierRemoveMode)
    {
        case CGexFileInGroup::N_IQR:	//GEX_OPTION_OUTLIER_IQR:	// IQR filtering can only be done AFTER all data are loaded...
            return false;
        case CGexFileInGroup::N_SIGMA:	//GEX_OPTION_OUTLIER_SIGMA:	// Sigma outlier filter is only activated on PASS2.
            return false;
        case CGexFileInGroup::EXCLUDE_N_SIGMA:	//GEX_OPTION_INLINER_SIGMA:
            if ((fValue > ptTestCell->GetCurrentLimitItem()->lfLowLimitOutlier) && (fValue < ptTestCell->GetCurrentLimitItem()->lfHighLimitOutlier))
            {
                if(lPass == 2)
                    ptTestCell->GetCurrentLimitItem()->ldOutliers++;	// Counts total # of IN-Lier!
                return true;	// Test IS an IN-lier
            }
            break;

        default:
            if((fValue < ptTestCell->GetCurrentLimitItem()->lfLowLimitOutlier) || (fValue>ptTestCell->GetCurrentLimitItem()->lfHighLimitOutlier))
            {
                if(lPass == 2)
                    ptTestCell->GetCurrentLimitItem()->ldOutliers++;	// Counts total # of outliers!
                return true;	// Test IS an outlier
            }
            break;
    }


    if(lPass == 2)
    {
        // Pass#2. If Sigma Outlier enabled, must recompute Min/Max as filter window was not computed during pass2
        switch(m_eOutlierRemoveMode)
        {
            case CGexFileInGroup::N_SIGMA:	//GEX_OPTION_OUTLIER_SIGMA:
            case CGexFileInGroup::EXCLUDE_N_SIGMA:	//GEX_OPTION_INLINER_SIGMA:
                ptTestCell->lfSamplesMin = gex_min(fValue,ptTestCell->lfSamplesMin);
                ptTestCell->lfSamplesMax = gex_max(fValue,ptTestCell->lfSamplesMax);
                break;
            default:
                break;
        }
    }

    return false;	// Test is NOT an outlier
}

/////////////////////////////////////////////////////////////////////////////
// Keep track of last failing test info (for GEX-DRILL module)
/////////////////////////////////////////////////////////////////////////////
void CGexFileInGroup::CountTrackFailingTestInfo(int iSiteID,CTest *ptTestCell,int iPinmapIndex,float fValue)
{
    // Keeps track of last failing test in flow. Reset after each run (in PRR).
    m_mapFailTestPinmap[iSiteID] = iPinmapIndex;
    m_mapfFailingValue[iSiteID] = fValue;
    m_map_ptFailTestCell[iSiteID] = ptTestCell;
}

/////////////////////////////////////////////////////////////////////////////
// Check if test result in valid!
// return: true if count this Test as a Failure, false otherwise
/////////////////////////////////////////////////////////////////////////////
bool CGexFileInGroup::IsTestFail(CTest *ptTestCell,float fValue, CTestResult::PassFailStatus ePassFailStatus, bool& bOutsideLimit)
{
    bOutsideLimit = false;
    bool bLowLimitFail = false, bHighLimitFail = false, bIsTestFail = false;

    bIsTestFail = ptTestCell->isFailingValue(fValue, ePassFailStatus, &bLowLimitFail, &bHighLimitFail);
    bOutsideLimit = bLowLimitFail || bHighLimitFail;

    return bIsTestFail;
}


bool CGexFileInGroup::AdvCollectTestResult(
        CTest *ptTestCell, double lfValue,
        CTestResult::PassFailStatus ePassFailStatus,
        int iSiteID, bool bForceCollection /* =false */)
{
    bool bTestInList;

    // Mark this test is executed in this run...
    ptTestCell->bTestExecuted = true;

    // IF test is in list, or we run Gex-Advanced release (which loads all data in RAM), get data!
    if(pReportOptions->pGexAdvancedRangeList == NULL)
        bTestInList = false;
    else
        bTestInList = pReportOptions->pGexAdvancedRangeList->IsTestInList(ptTestCell->lTestNumber,ptTestCell->lPinmapIndex);

    if(bTestInList || bForceCollection)
    {
        // Don't collect result if it is not in PIR/PRR
        if (PartProcessed.PIRNestedLevel() > 0)
        {
            if (PartProcessed.hasResultsCount())
            {
                // Get the part ID for the given site ID
                int nPartNumber = PartProcessed.partNumber(iSiteID);

                // Valid test!
                if(ptTestCell->m_testResult.count() == 0)
                {
                    if (ptTestCell->ldSamplesExecs<1)
                        return true;

                    QString lTRA=pReportOptions->GetOption("dataprocessing", "test_results_allocation").toString();
                    if (lTRA=="one_for_all")
                    {
                        // Let s allocate all test results in 1 time if not already done
                        if (CTestResult::sResults==0)
                        {
                            // If I remember right : it is new XXX y per x : here parts in Y, tests in X
                            unsigned lTotalNumberOfTest=CTest::GetNumberOfInstances(); // 185 for data samples ?
                            // impossible because X must be constant
                            //CTestResult::sResults=new double[ptTestCell->ldSamplesExecs][lTotalNumberOfTest];
                            CTestResult::sResults=(double**)malloc(ptTestCell->ldSamplesExecs*lTotalNumberOfTest*sizeof(double));
                        }
                    }

                    // If failed allocating the memory buffer, report error and exit
                    QString  lRes=ptTestCell->m_testResult.createResultTable(
                                      ptTestCell->ldSamplesExecs, ptTestCell->hasMultiResult());
                    if (lRes.startsWith("error"))
                    {
                        GSLOG(SYSLOG_SEV_ERROR, QString("Collect test result : createResultTable failed: %1")
                              .arg( lRes).toLatin1().constData());
                        return false; // was just return;...
                    }

                    // Saves first part# ID collected
                    ptTestCell->lFirstPartResult = nPartNumber;
                    // 'true' if this test is collected because it is in the test list.
                    // 'false' means it was collected because Gex is running the Advanced edition.
                    ptTestCell->bTestInList = bTestInList;
                }

                // Get the sample index corresponding to this part number
                int nSampleIndex = PartProcessed.runIndex(nPartNumber);

                if(getMirDatas().lFirstPartResult == 0)
                {
                    // Save current part# in case we marge multiple files in a group.
                    // This allows to know the offset of each data set collected per file
                    getMirDatas().lFirstPartResult = nSampleIndex;
                }

                // convert test result from normalized number to number (eg 1.5 if
                // results is 1.5 mV, etc...)...unless we have a NaN
                if(lfValue != GEX_C_DOUBLE_NAN)
                {
                    if (m_eScalingMode==SCALING_NONE || m_eScalingMode==SCALING_SMART)
                        lfValue /=	ScalingPower(ptTestCell->res_scal);
                    else if (m_eScalingMode==SCALING_TO_LIMITS)
                        lfValue /=	ScalingPower(ptTestCell->hlm_scal);
                }

                // Security check for overflow!
                if(nSampleIndex >= 0 && nSampleIndex < ptTestCell->ldSamplesExecs)
                {
                    // Save value into buffer
                    nSampleIndex += CalculOffsetIndexResult(strFileName);
                    ptTestCell->m_testResult.pushResultAt(nSampleIndex, lfValue);

                    // Save test fail flag
                    ptTestCell->m_testResult.setPassFailStatusAt(nSampleIndex, ePassFailStatus);
                }
                else{
                    if(m_iInvalidPartFoundCount.find(ptTestCell->lTestNumber) == m_iInvalidPartFoundCount.end())
                        m_iInvalidPartFoundCount.insert(ptTestCell->lTestNumber,0);
                    m_iInvalidPartFoundCount[ptTestCell->lTestNumber]+=1;

                    //	GSLOG(SYSLOG_SEV_WARNING,
                    //      QString("nSampleIndex (%1) < 0 OR nSampleIndex (%1) > ptTestCell->ldSamplesExecs (%2)")
                    //		.arg(nSampleIndex).arg(ptTestCell->ldSamplesExecs));
                }

                // Saves last part# ID collected
                ptTestCell->lLastPartResult = nPartNumber;
            }
        }
        else
            GSLOG(SYSLOG_SEV_NOTICE,
                     QString("Test result value %1 not collected for test %2 %3: outside PIR/PRR block")
                     .arg(lfValue)
                     .arg(ptTestCell->lTestNumber)
                     .arg(ptTestCell->strTestName).toLatin1().constData());
    }
    return true;
}

/////////////////////////////////////////////////////////////////////////////
// Collect Binning Trend data over Sub-lots (if such report requested)
// Or for any production yield report
/////////////////////////////////////////////////////////////////////////////
void CGexFileInGroup::AdvCollectBinningTrend(CBinning **ptBinList,int iBinValue,int ldTotalCount)
{
    switch(pReportOptions->getAdvancedReport())
    {
        case GEX_ADV_PROD_YIELD:
            if(pReportOptions->bAdvancedUsingSoftBin)
            {
                // Using SOFT bin
                if(ptBinList != &m_ParentGroup->cMergedData.ptMergedSoftBinList)
                    return;	// Ignore HARD Bins
            }
            else
            {
                // Using HARD bin
                if(ptBinList != &m_ParentGroup->cMergedData.ptMergedHardBinList)
                    return;	// Ignore SOFT Bins
            }
            break;

        case GEX_ADV_TREND:
            switch(pReportOptions->getAdvancedReportSettings())
            {
                case GEX_ADV_TREND_SOFTBIN_SBLOTS: // Collect SOFT bin values (Sub-Lot summary)
                    if(ptBinList != &m_ParentGroup->cMergedData.ptMergedSoftBinList)
                        return;	// Ignore HARD Bins
                    break;
                case GEX_ADV_TREND_HARDBIN_SBLOTS:// Collect HARD bin values (Sub-Lot summary)
                    if(ptBinList != &m_ParentGroup->cMergedData.ptMergedHardBinList)
                        return;	// Ignore SOFT Bins
                    break;
            }
            break;

        default:	// Not in a mode needing to collect bin info.
            return;
    }

    if(lPass == 2)
        return;

    // Update total count of Bins found
    lAdvBinningTrendTotal += ldTotalCount;

    if(!pReportOptions->pGexAdvancedRangeList->IsTestInList(iBinValue,GEX_PTEST))
        return;	// This binning is not part of our list!

    // Update total count of Bins matching our filter
    lAdvBinningTrendTotalMatch += ldTotalCount;
}

/////////////////////////////////////////////////////////////////////////////
// Read STDF PTR record
/////////////////////////////////////////////////////////////////////////////
bool CGexFileInGroup::ReadPTR()
{
    int		test_flg;
    BYTE	param_flg;
    char	szString[257];	// A STDF string is 256 bytes long max!
    float	fResult;
    double	lfLowLimit=-C_INFINITE,lfHighLimit=C_INFINITE;
    BYTE	bOptFlag=0xff;
    BYTE	bSite,bHead,bRes_scal;
    bool	bValideResult=true;
    bool	bTestIsFail=false, bOutsideLimit=false;
    bool	bAlarmResult=false;
    bool	bIgnoreSite=false;
    bool	bIsOutlier;
    bool	bStrictLL=true, bStrictHL=true;
    unsigned uMapIndex;
    bool	bIsNAN=false;

    CTestResult::PassFailStatus	ePassFailStatus	= CTestResult::statusUndefined;
    CTest *						ptTestCell		= NULL;	// Pointer to test cell to receive STDF info.

    // PTR
   /* StdfFile.ReadDword(&lData);	// test number
    StdfFile.ReadByte(&bHead);		// Head
    StdfFile.ReadByte(&bSite);		// test site

    // Check if this PTR is within a valid PIR/PRR block.
    uMapIndex = (bHead << 8) | bSite;	// Compute index for Head&Site...as in multi-head/sites, we have to save multiple results before the PRR
    unsigned short siteNumber = bSite;
    */

    // PTR
    StdfFile.ReadDword(&lData);	// test number
    StdfFile.ReadByte(&bHead);		// Head
    StdfFile.ReadByte(&bSite);		// test site
    unsigned short siteNumber = bSite;


    if(StdfFile.GetHeadAndSiteNumberDecipheringMode() != GS::StdLib::no_deciphering)
    {
        siteNumber = GQTL_STDF::HeadAndSiteNumberDecipher::DecipherSiteNumberForAdvantest(bHead, siteNumber);
        bHead = GQTL_STDF::HeadAndSiteNumberDecipher::DecipherHeadNumberForAdvantest(bHead);
    }

    // Check if this PTR is within a valid PIR/PRR block.
    uMapIndex = (bHead << 16) | siteNumber;	// Compute index for Head&Site...as in multi-head/sites, we have to save multiple results before the PRR

    if (lMappingSiteHead.find(uMapIndex) == lMappingSiteHead.end())
    {
        if (m_eStdfCompliancy == CGexFileInGroup::STRINGENT)
        {
            GS::Gex::Message::warning(
                "",
                QString("Error parsing file %1:\n"
                        "PIR and PRR do not match.\n\n"
                        "You may check the file content using "
                        "'Toolbox->STDF Records Dump'.").
                arg(strFileName));
        }
    }

    if((m_eStdfCompliancy==CGexFileInGroup::STRINGENT) && lMappingSiteHead[uMapIndex] != 1)
        bIgnoreSite = true;

    // Ignore PTR if filtering set to 'Bin data only'
    if(iProcessBins == GEX_PROCESSPART_NO_SAMPLES)
        return true;

    // Check if current PTR (test result) belongs to a part that we filter ?
    // If this part has a binning the User doesn't want to process, then
    // we skip this record...
    if((lPass == 2) && (PartProcessed.IsFilteredPart(pReportOptions, siteNumber, false, iProcessBins) == true))
        return true;

    // Ignore test if we are in 'Stop on Fail' mode, and current site already failed!
    if((lPass == 2) && (m_eFailCountMode == FAILCOUNT_FIRST) && (m_map_ptFailTestCell.contains(siteNumber)))
        return true;

    switch(StdfRecordHeader.iStdfVersion)
    {
    default :
            break;

        case GEX_STDFV4:
            StdfFile.ReadByte(&bData);		// test_flg
            test_flg = (int) bData & 0xff;
            StdfFile.ReadByte(&param_flg);	// parm_flg
            StdfFile.ReadFloat(&fResult, &bIsNAN);// test result

            // Check if test limits are strict
            if(param_flg & 0100)
                bStrictLL = false;
            if(param_flg & 0200)
                bStrictHL = false;

            // Check if Pass/Fail indication is valid (bit 6 cleared)
            // Only if Pass/Fail rule option is checked
            if (m_bUsePassFailFlag && (test_flg & 0x40) == 0x00)
            {
                // If bit 7 set, then test is fail, otherwise test is pass
                ePassFailStatus = (test_flg & 0x80) ? CTestResult::statusFail : CTestResult::statusPass;
            }

            // Check if test result is valid
            if(m_eStdfCompliancy==CGexFileInGroup::STRINGENT)
            {
                // In stringent mode (Data processing options->Handling STDF Compliancy issues = Stringent),
                // consider test result is valid based on STDF specs:
                /////////////////////////////////////////////////////////////////////////////////////////////////////
                // The RESULT value is considered useful only if all the following bits from TEST_FLG and PARM_FLG are 0:
                //
                // TEST_FLG	bit 0 = 0	no alarm
                // bit 1 = 0	value in result field is valid
                // bit 2 = 0	test result is reliable
                // bit 3 = 0	no timeout
                // bit 4 = 0	test was executed
                // bit 5 = 0	no abort
                // PARM_FLG	bit 0 = 0	no scale error
                // bit 1 = 0	no drift error
                // bit 2 = 0	no oscillation
                //
                // If any one of these bits is 1, then the PTR result should not be used.
                /////////////////////////////////////////////////////////////////////////////////////////////////////
                if((test_flg & 077) || (param_flg & 07))
                    bValideResult = false;
                else
                    bValideResult = true;
            }
            else
            {
                // In flexible mode (Data processing options->Handling STDF Compliancy issues = Flexible),
                // consider test result is valid only based on TEST_FLG.bit1
                bValideResult = ((test_flg & 0x12) == 0);
            }

            if(bIsNAN)
                bValideResult = false;

            // Ignore test result if must compute statistics from summary only
            if (m_eStatsComputation == FROM_SUMMARY_ONLY)
                bValideResult = false;

            // Check if test has an alarm
            if(test_flg & 1)
                bAlarmResult = true;

            // Read Test definition: name, Limits, etc...
            if(StdfFile.ReadString(szString)  == GS::StdLib::Stdf::NoError)
            {
                // Replaces '\t' by ' '.
                char *ptChar;
                do
                {
                    ptChar = strchr(szString,'\t');
                    if(ptChar != NULL) *ptChar = ' ';
                }
                while(ptChar != NULL);

                // Replace ',' with ';' for csv output
                FormatTestName(szString);

                if (mTestMergeRule != TEST_MERGE_NUMBER)
                {
                    if (szString[0] != '\0')
                    {
                        if (!mTestCorrespondance.contains((int)lData))
                        {
                            mTestCorrespondance.insert((int)lData, QString(szString));
                        }
                    }
                    else
                    {
                        if (mTestCorrespondance.count() > 0)
                        {
                            QMap<int, QString>::iterator it = mTestCorrespondance.find(lData);
                            if (it != mTestCorrespondance.end())
                            {
                                QString str = QString(*it);
                                strcpy(szString, str.toLatin1().constData());
                            }
                        }
                    }
                }

                // Returns pointer to correct cell. If cell doesn't exist ; its created...Test# mapping enabled.
                if (FindTestCell(lData, GEX_PTEST, &ptTestCell, true,true, szString) !=1)
                    return true;  // Error

                // Formats test name (in case to long, or leading spaces)..only take non-empty strings (since at init. time it is already set to empty)
                if(ptTestCell->strTestName.isEmpty() == true)
                {
                    // Overwrite Cell name only if it was not previously initialized...
                    if(*szString)
                    {
                        ptTestCell->strTestName = szString;

                        // Trim test name to remove unnecessary spaces at
                        // beginning and end (for QA) [BG 08/27/2012]
                        ptTestCell->strTestName =
                            ptTestCell->strTestName.trimmed();
                    }
                }
            }
            else
            {
                if (mTestMergeRule != TEST_MERGE_NUMBER)
                {
                    if (szString[0] == '\0')
                    {
                        QMap<int, QString>::iterator it = mTestCorrespondance.find(lData);
                        if (it != mTestCorrespondance.end())
                        {
                            QString str = QString(*it);
                            strcpy(szString, str.toLatin1().constData());
                        }
                    }

                    if(FindTestCell(lData, GEX_PTEST, &ptTestCell, true, true, szString) !=1)
                        return true;  // Error
                }
                else
                {
                    if(FindTestCell(lData, GEX_PTEST, &ptTestCell) !=1)
                        return true;
                }
            }

            // Add a count for this site
            ptTestCell->addMultiResultCount(siteNumber);

            // alarm_id
            StdfFile.ReadString(szString);

            // OPT_FLAG
            if(StdfFile.ReadByte(&bOptFlag) == GS::StdLib::Stdf::NoError)
            {
                // RES_SCALE
                if(StdfFile.ReadByte(&bRes_scal) == GS::StdLib::Stdf::NoError)
                {
                    // res-scal is valid and never defined before, save it in test definition
                    if(((bOptFlag & 0x1) == 0) && ((ptTestCell->GetCurrentLimitItem()->bLimitFlag & (CTEST_LIMITFLG_NOLTL | CTEST_LIMITFLG_NOHTL)) == (CTEST_LIMITFLG_NOLTL | CTEST_LIMITFLG_NOHTL)))
                        ptTestCell->res_scal = bRes_scal;

                }
            }

            if(bIgnoreSite || ((iProcessSite >=0) && (iProcessSite != (int)siteNumber)) )
            {
                bIgnoreSite = true;
                goto ReadTestDefinitionV4;	// read test static info (but not test limits).
            }

            if((bValideResult == true) && (lPass == 2))
            {
                // PASS2:

                // Check if this test is an outlier
                bIsOutlier = PartIsOutlier(ptTestCell,fResult);

                // Ignore STDF FAIL flag if What-If limits defined!
                if(ptTestCell->GetCurrentLimitItem()->bLimitWhatIfFlag != (CTEST_LIMITFLG_NOLTL | CTEST_LIMITFLG_NOHTL))
                    ePassFailStatus = CTestResult::statusUndefined;

                // Check if test is fail
                bTestIsFail = IsTestFail(ptTestCell, fResult, ePassFailStatus, bOutsideLimit);

                if (bTestIsFail && !bOutsideLimit)
                    m_map_ptFailedTestGoodResult[siteNumber].append(ptTestCell);
                else if (!bTestIsFail && bOutsideLimit)
                    m_map_ptPassedTestBadResult[siteNumber].append(ptTestCell);

                // Keep track of failing count (unless it's an outlier)
                if((bIsOutlier == false) && CountTestAsFailure(siteNumber,ptTestCell,GEX_PTEST,fResult,bTestIsFail) == true)
                {
                    // Update failing count
                    ptTestCell->GetCurrentLimitItem()->ldSampleFails++;
                    ptTestCell->UpdateSampleFails(siteNumber);
                    m_bFailingSiteDetails[siteNumber] |= GEX_RUN_FAIL_TEST;

                    // If failure and What-If limits enabled, make sure to force a failing bin!
                    if(ptTestCell->GetCurrentLimitItem()->bLimitWhatIfFlag != (CTEST_LIMITFLG_NOLTL | CTEST_LIMITFLG_NOHTL))
                        m_bFailingSiteDetails[siteNumber] |= GEX_RUN_FAIL_WHATIFTEST;
                }

                // If failure, make sure we mark it.
                if(bTestIsFail)
                    CountTrackFailingTestInfo(siteNumber,ptTestCell,GEX_PTEST,fResult);

                // Advanced report is Enabled, and test in filter limits.
                switch(pReportOptions->getAdvancedReport())
                {
                    case GEX_ADV_DISABLED:
                    case GEX_ADV_PAT_TRACEABILITY:
                    default:
                        // If Gex-Advanced edition, ALWAYS collect test results!
                        if(bIsOutlier)
                            return true;
                        // collect sample to allow Interactive Charting (Unless size excees cutoff limit)
                        if(pReportOptions->bSpeedCollectSamples)
                        {
                            if (AdvCollectTestResult(ptTestCell, fResult, ePassFailStatus, siteNumber, true) == false)
                                return false;
                        }
                        break;

                    case GEX_ADV_DATALOG:
                        if (GexAbstractDatalog::existInstance())
                        {
                            if (GexAbstractDatalog::isDataloggable(ptTestCell, bTestIsFail, bIsOutlier))
                            {
                                int nRunIndex = PartProcessed.runIndexFromSite(siteNumber);
                                if (nRunIndex >= 0 && nRunIndex < pPartInfoList.count())
                                    GexAbstractDatalog::pushParametricResult(
                                                this, ptTestCell, pPartInfoList.at(nRunIndex), fResult, bTestIsFail, bAlarmResult, siteNumber);
                            }
                        }
                        else
                            AdvCollectDatalog(ptTestCell,fResult,bTestIsFail,bAlarmResult,bIsOutlier, siteNumber, (int)bHead, (int)siteNumber);

                        // Check for outlier AFTER checking for datalog...in case datalog is on outlier!
                        if(bIsOutlier)
                            return true;
                        // collect sample to allow Interactive Charting (Unless size excees cutoff limit)
                        if(pReportOptions->bSpeedCollectSamples)
                        {
                            if (AdvCollectTestResult(ptTestCell, fResult, ePassFailStatus, siteNumber, true) == false)
                                return false;
                        }
                        break;
                    case GEX_ADV_HISTOGRAM:
                    case GEX_ADV_TREND:
                    case GEX_ADV_CANDLE_MEANRANGE:
                    case GEX_ADV_GO_DIAGNOSTICS:	// Optimizer Diagnostic takes ALL Trend results!
                        if(bIsOutlier)
                            return true;
                        if (AdvCollectTestResult(ptTestCell, fResult, ePassFailStatus, siteNumber,
                                                 pReportOptions->bSpeedCollectSamples) == false)
                        {
                            return false;
                        }
                        break;
                    case GEX_ADV_CORRELATION:
                        if(bIsOutlier)
                            return true;
                        if (AdvCollectTestResult(ptTestCell, fResult, ePassFailStatus, siteNumber, pReportOptions->bSpeedCollectSamples) == false)
                            return false;
                        break;
                }

                // If this is an outlier (data to totally IGNORE)...don't take it into account!
                if(bIsOutlier)
                    return true;

#if 0 // Herve Thomy : // OBSOLETE MEMBER Not used anymore,since we hold the runIndex in the CPartBinning class except with MASA plugins
                // Keeps tracking the test# result.
                ptTestCell->ldCurrentSample++;
#endif

                // Build Cpk
                ptTestCell->lfSamplesTotal += fResult;				// Sum of X
                ptTestCell->lfSamplesTotalSquare += (fResult*fResult);// Sum of X*X


                ptTestCell->mHistogramData.AddValue(fResult);
                if((fResult >= ptTestCell->GetCurrentLimitItem()->lfHistogramMin) && (fResult <= ptTestCell->GetCurrentLimitItem()->lfHistogramMax))
                {
                    // Incluse result if it in into the viewing window.
                    int iCell;
                    if (ptTestCell->GetCurrentLimitItem()->lfHistogramMax == ptTestCell->GetCurrentLimitItem()->lfHistogramMin)
                        iCell = 0;
                    else
                    {
                        iCell = (int)( (TEST_HISTOSIZE*(fResult - ptTestCell->GetCurrentLimitItem()->lfHistogramMin))
                                       / (ptTestCell->GetCurrentLimitItem()->lfHistogramMax-ptTestCell->GetCurrentLimitItem()->lfHistogramMin));
                    }

                    // Incluse result if it in into the viewing window.
                    if( (iCell >= 0) && (iCell < TEST_HISTOSIZE) )
                        ptTestCell->lHistogram[iCell]++;
                    else if (iCell >= TEST_HISTOSIZE)
                        ptTestCell->lHistogram[TEST_HISTOSIZE-1]++;   /// TO REVIEW : is it normal that iCell can exceed than TEST_HISTOSIZE
                    else
                    {
                        GSLOG(SYSLOG_SEV_ERROR,
                                 QString("Invalid value detected to build histogram for test %1 [%2]")
                                 .arg(ptTestCell->lTestNumber)
                                 .arg(ptTestCell->strTestName).toLatin1().constData());

                        GSLOG(SYSLOG_SEV_DEBUG, QString("\tCell Found: %1").arg(iCell).toLatin1().constData());
                        GSLOG(SYSLOG_SEV_DEBUG, QString("\tPTR result: %1").arg( fResult).toLatin1().constData());
                        GSLOG(SYSLOG_SEV_DEBUG, QString("\tHistogram Min: %1").arg(
                                 ptTestCell->GetCurrentLimitItem()->lfHistogramMin).toLatin1().constData());
                        GSLOG(SYSLOG_SEV_DEBUG, QString("\tHistogram Max: %1").arg(
                                 ptTestCell->GetCurrentLimitItem()->lfHistogramMax).toLatin1().constData());
                    }
                }
            }
ReadTestDefinitionV4:

            // Test is Parametric
            GEX_ASSERT(ptTestCell->bTestType == 'P');

            if(lPass == 2)
                return true;

            // If we already have the limits set...ignore any NEW limits defined.
            if((ptTestCell->GetCurrentLimitItem()->bLimitFlag & (CTEST_LIMITFLG_NOLTL | CTEST_LIMITFLG_NOHTL))
               != (CTEST_LIMITFLG_NOLTL | CTEST_LIMITFLG_NOHTL))
            {
                lfLowLimit = ptTestCell->GetCurrentLimitItem()->lfLowLimit;
                lfHighLimit = ptTestCell->GetCurrentLimitItem()->lfHighLimit;
            }
            else
            {
                lfLowLimit = -C_INFINITE;
                lfHighLimit = C_INFINITE;
            }

            // Are test limits strict??
            if(!bStrictLL)
                ptTestCell->GetCurrentLimitItem()->bLimitFlag |= CTEST_LIMITFLG_LTLNOSTRICT;
            if(!bStrictHL)
                ptTestCell->GetCurrentLimitItem()->bLimitFlag |= CTEST_LIMITFLG_HTLNOSTRICT;

            // If not in 'What-if' mode, save limits flag seen in STDF file
            if((bOptFlag & 0x10) == 0 && mUsedLimits != CGexFileInGroup::SPEC_LIMIT_ONLY)
            {
                // LowLimit+low_scal flag is valid...
                ptTestCell->GetCurrentLimitItem()->bLimitFlag &= ~(CTEST_LIMITFLG_NOLTL);
                if(bOptFlag & 0x40)	// No Low Limit
                    ptTestCell->GetCurrentLimitItem()->bLimitFlag |= CTEST_LIMITFLG_NOLTL;
            }
            // If What-if has forced a LL
            if((ptTestCell->GetCurrentLimitItem()->bLimitWhatIfFlag & CTEST_LIMITFLG_NOLTL) == 0)
                ptTestCell->GetCurrentLimitItem()->bLimitFlag &= ~(CTEST_LIMITFLG_NOLTL);

            // If not in 'What-if' mode, save limits flag seen in STDF file
            if((bOptFlag & 0x20) == 0 && mUsedLimits != CGexFileInGroup::SPEC_LIMIT_ONLY)
            {
                // HighLimit+high_scal flag is valid...
                ptTestCell->GetCurrentLimitItem()->bLimitFlag &= ~(CTEST_LIMITFLG_NOHTL);
                if(bOptFlag & 0x80)	// No High Limit
                    ptTestCell->GetCurrentLimitItem()->bLimitFlag |= CTEST_LIMITFLG_NOHTL;
            }
            // If What-if has forced a HL
            if((ptTestCell->GetCurrentLimitItem()->bLimitWhatIfFlag & CTEST_LIMITFLG_NOHTL) == 0)
                ptTestCell->GetCurrentLimitItem()->bLimitFlag &= ~(CTEST_LIMITFLG_NOHTL);

            if(StdfFile.ReadByte(&bData) != GS::StdLib::Stdf::NoError)
                goto ExitBeforeLimitsV4;	// llm_scal
            if((bOptFlag & 0x50) == 0)	// llm-scal is valid
                ptTestCell->llm_scal = bData;

            if(StdfFile.ReadByte(&bData) != GS::StdLib::Stdf::NoError)
                goto ExitBeforeLimitsV4;	// hlm_scal
            if((bOptFlag & 0xA0) == 0)	// hlm-scal is valid
                ptTestCell->hlm_scal = bData;

            // Read low and High limits
            if(StdfFile.ReadFloat(&fData) != GS::StdLib::Stdf::NoError)
                goto ExitBeforeLimitsV4;	// low limit
            if(((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0)
                    && ((bOptFlag & 0x10) == 0)
                    && (mUsedLimits != CGexFileInGroup::SPEC_LIMIT_ONLY))
                lfLowLimit = M_CHECK_INFINITE((double)fData); // Low limit exists: keep value

            if(StdfFile.ReadFloat(&fData) != GS::StdLib::Stdf::NoError)
            {
                // if not "Use spec limits only"
                if (mUsedLimits != CGexFileInGroup::SPEC_LIMIT_ONLY)
                    ptTestCell->GetCurrentLimitItem()->lfLowLimit = lfLowLimit;
                goto ExitAfterLimitsV4;	// High limit doesn't exist
            }

            if(((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0)
                    && ((bOptFlag & 0x20) == 0)
                    && (mUsedLimits != CGexFileInGroup::SPEC_LIMIT_ONLY))
                lfHighLimit = M_CHECK_INFINITE((double)fData);// High limit exists: keep value


            // Read test units, C_RESFMT, C_LLMFMT and C_HLMFMT
            if(StdfFile.ReadString(szString)  != GS::StdLib::Stdf::NoError)
                goto CalculateLimits;
            szString[GEX_UNITS-1] = 0;
            // If unit specified (and using printable ascii characters), then use it.
            if(*szString >= ' ')
                strcpy(ptTestCell->szTestUnits, QString(szString).trimmed().toLatin1().constData());

            // C_RESFMT
            if(StdfFile.ReadString(szString)  != GS::StdLib::Stdf::NoError)
                goto CalculateLimits;
            // C_LLMFMT
            if(StdfFile.ReadString(szString)  != GS::StdLib::Stdf::NoError)
                goto CalculateLimits;
            // C_HLMFMT
            if(StdfFile.ReadString(szString)  != GS::StdLib::Stdf::NoError)
                goto CalculateLimits;

            /// These inforamtions have to be in the new limits
            // LO_SPEC
            if(StdfFile.ReadFloat(&fData) != GS::StdLib::Stdf::NoError)
                goto CalculateLimits;
            if(bOptFlag & 4)	// LoSpec is Invalid
                ptTestCell->GetCurrentLimitItem()->bLimitFlag |= CTEST_LIMITFLG_NOLSL;
            else
                ptTestCell->GetCurrentLimitItem()->bLimitFlag &= ~CTEST_LIMITFLG_NOLSL;	// Valid Low Spec limit

            ptTestCell->lfLowSpecLimit = M_CHECK_INFINITE((double)fData);

            // If not "Use standard limits only"
            if(mUsedLimits == CGexFileInGroup::SPEC_LIMIT_IF_ANY)
            {
                // Flag test limit validity depending on spec limits flag
                if ((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLSL) == 0)
                {
                    ptTestCell->GetCurrentLimitItem()->bLimitFlag &= ~CTEST_LIMITFLG_NOLTL;
                    lfLowLimit = M_CHECK_INFINITE((double)fData);

                    // No low specs limits as there are kept in std limits
                    ptTestCell->GetCurrentLimitItem()->bLimitFlag |= CTEST_LIMITFLG_NOLSL;
                }
            }
            else if(mUsedLimits == CGexFileInGroup::SPEC_LIMIT_ONLY)
            {
                lfLowLimit = M_CHECK_INFINITE((double)fData);

                // Flag test limit validity depending on spec limits flag
                if ((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLSL) == 0)
                {
                    ptTestCell->GetCurrentLimitItem()->bLimitFlag &= ~CTEST_LIMITFLG_NOLTL;

                    // No low specs limits as there are kept in std limits
                    ptTestCell->GetCurrentLimitItem()->bLimitFlag |= CTEST_LIMITFLG_NOLSL;
                }
                else
                    ptTestCell->GetCurrentLimitItem()->bLimitFlag |= CTEST_LIMITFLG_NOLTL;
            }

            // HI_SPEC
            if(StdfFile.ReadFloat(&fData) != GS::StdLib::Stdf::NoError)
                goto CalculateLimits;
            if(bOptFlag & 8)	// HiSpec is Invalid
                ptTestCell->GetCurrentLimitItem()->bLimitFlag |= CTEST_LIMITFLG_NOHSL;
            else
                ptTestCell->GetCurrentLimitItem()->bLimitFlag &= ~CTEST_LIMITFLG_NOHSL;	// Valid High Spec limit

            ptTestCell->lfHighSpecLimit = M_CHECK_INFINITE((double)fData);

            // If not "Use standard limits only"
            if(mUsedLimits == CGexFileInGroup::SPEC_LIMIT_IF_ANY)
            {
                // Flag test limit validity depending on spec limits flag
                if ((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHSL) == 0)
                {
                    ptTestCell->GetCurrentLimitItem()->bLimitFlag &= ~CTEST_LIMITFLG_NOHTL;
                    lfHighLimit = M_CHECK_INFINITE((double)fData);

                    // No low specs limits as there are kept in std limits
                    ptTestCell->GetCurrentLimitItem()->bLimitFlag |= CTEST_LIMITFLG_NOHSL;
                }
            }
            else if(mUsedLimits == CGexFileInGroup::SPEC_LIMIT_ONLY)
            {
                lfHighLimit = M_CHECK_INFINITE((double)fData);

                // Flag test limit validity depending on spec limits flag
                if ((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHSL) == 0)
                {
                    ptTestCell->GetCurrentLimitItem()->bLimitFlag &= ~CTEST_LIMITFLG_NOHTL;

                    // No low specs limits as there are kept in std limits
                    ptTestCell->GetCurrentLimitItem()->bLimitFlag |= CTEST_LIMITFLG_NOHSL;
                }
                else
                    ptTestCell->GetCurrentLimitItem()->bLimitFlag |= CTEST_LIMITFLG_NOHTL;
            }

CalculateLimits:

            // Store the test limits for this site
            if (m_ParentGroup->addTestSiteLimits(ptTestCell, iProcessSite, siteNumber, lfLowLimit,
                                                    lfHighLimit, bOptFlag, getMirDatas().lStartT) == false)
                return false;

            // Retrieve the min and max tests limits for this site
            m_ParentGroup->getTestSiteLimits(ptTestCell, iProcessSite, siteNumber, lfLowLimit, lfHighLimit);

            if (bIgnoreSite == false || (ptTestCell->GetCurrentLimitItem()->lfHighLimit == C_INFINITE && ptTestCell->GetCurrentLimitItem()->lfLowLimit == -C_INFINITE))
            {
                // If High limit not forced by What-If...
                if((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0 && ptTestCell->GetCurrentLimitItem()->bLimitWhatIfFlag & CTEST_LIMITFLG_NOHTL)
                    ptTestCell->GetCurrentLimitItem()->lfHighLimit = lfHighLimit;

                // If Low limit not forced by What-If...
                if((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0 && ptTestCell->GetCurrentLimitItem()->bLimitWhatIfFlag & CTEST_LIMITFLG_NOLTL)
                    ptTestCell->GetCurrentLimitItem()->lfLowLimit = lfLowLimit;
            }

            if(m_datasetConfig.testToUpdateList().isActivated())
            {
                GexTestToUpdate *poUpdate = m_datasetConfig.testToUpdateList().getTestToUpdate(ptTestCell->lTestNumber);
                if(poUpdate)
                {
                    ptTestCell->GetCurrentLimitItem()->bLimitWhatIfFlag = CTEST_LIMITFLG_NOLTL | CTEST_LIMITFLG_NOHTL;
                    if(!poUpdate->highLimit().isEmpty())
                    {
                        ptTestCell->GetCurrentLimitItem()->lfHighLimit = poUpdate->highLimit().toDouble();
                        if(poUpdate->source() == "whatif")
                            ptTestCell->GetCurrentLimitItem()->bLimitWhatIfFlag	&= ~CTEST_LIMITFLG_NOHTL;
                    }
                    if(!poUpdate->lowLimit().isEmpty())
                    {
                        ptTestCell->GetCurrentLimitItem()->lfLowLimit = poUpdate->lowLimit().toDouble();
                        if(poUpdate->source() == "whatif")
                            ptTestCell->GetCurrentLimitItem()->bLimitWhatIfFlag	&= ~CTEST_LIMITFLG_NOLTL;
                    }
                }
            }

            // All info read, including test limits
ExitAfterLimitsV4:
            // Avoid failing if LL=HL
            if(ptTestCell->GetCurrentLimitItem()->lfLowLimit == ptTestCell->GetCurrentLimitItem()->lfHighLimit)
                ptTestCell->GetCurrentLimitItem()->bLimitFlag |= (CTEST_LIMITFLG_LTLNOSTRICT | CTEST_LIMITFLG_HTLNOSTRICT);

            // If data result to be ignoreed...
            if(bIgnoreSite)
                return true;
            // We have limits...so check if this is an outlier
            if(PartIsOutlier(ptTestCell,fResult) == true)
                return true;
            // Updates Min/Max values
            if(bValideResult == true)
                UpdateMinMaxValues(ptTestCell,fResult);
            return true;

            // All info read...but not until limits (may have been read in previous PTR)
ExitBeforeLimitsV4:
            // If data result to be ignoreed...
            if(bIgnoreSite)
                return true;

            if (m_ParentGroup->getTestSiteLimits(ptTestCell, iProcessSite, siteNumber, lfLowLimit, lfHighLimit))
            {
                // If High limit not forced by What-If...
                if((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0 && ptTestCell->GetCurrentLimitItem()->bLimitWhatIfFlag & CTEST_LIMITFLG_NOHTL)
                    ptTestCell->GetCurrentLimitItem()->lfHighLimit = lfHighLimit;

                // If Low limit not forced by What-If...
                if((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0 && ptTestCell->GetCurrentLimitItem()->bLimitWhatIfFlag & CTEST_LIMITFLG_NOLTL)
                    ptTestCell->GetCurrentLimitItem()->lfLowLimit = lfLowLimit;

                // Avoid failing if LL=HL
                if(ptTestCell->GetCurrentLimitItem()->lfLowLimit == ptTestCell->GetCurrentLimitItem()->lfHighLimit)
                    ptTestCell->GetCurrentLimitItem()->bLimitFlag |= (CTEST_LIMITFLG_LTLNOSTRICT | CTEST_LIMITFLG_HTLNOSTRICT);
            }
            else
            {
                ptTestCell->GetCurrentLimitItem()->bLimitFlag |= CTEST_LIMITFLG_NOLTL;
                ptTestCell->GetCurrentLimitItem()->bLimitFlag |= CTEST_LIMITFLG_NOHTL;
                ptTestCell->GetCurrentLimitItem()->lfHighLimit	= C_INFINITE;
                ptTestCell->GetCurrentLimitItem()->lfLowLimit	= -C_INFINITE;
            }

            // Check if test is an outlier
            if(PartIsOutlier(ptTestCell,fResult) == true)
                return true;
            // Updates Min/Max values..either because not an outlier, or no limits to check it!
            if(bValideResult == true)
                UpdateMinMaxValues(ptTestCell,fResult);
            break;
    }
    return true;  // ?
}

/////////////////////////////////////////////////////////////////////////////
// Read STDF PMR (Pin Map Record) record
/////////////////////////////////////////////////////////////////////////////
void CGexFileInGroup::ReadPMR(void)
{
    // Only read in PASS 1
    if(lPass == 2)
        return;

    char szString[257]="";	// A STDF string is 256 bytes long max!
    CPinmap	*ptPinmapCell=0;	// Pointer to a pinmap cell in pinmap list

    switch(StdfRecordHeader.iStdfVersion)
    {
    default :
            break;

        case GEX_STDFV4:
        {
            StdfFile.ReadWord(&wData);	// PMR_INDX: Pinmap index
            if(FindPinmapCell(&ptPinmapCell,wData)!=1)
                return;	// some error creating the pinmap cell, then ignore record

            StdfFile.ReadWord(&wData);	// CHAN_TYP
            if(StdfFile.ReadString(szString)  != GS::StdLib::Stdf::NoError)
                return;	// CHAN_NAM
            ptPinmapCell->strChannelName = FormatTestName(szString);

            if(StdfFile.ReadString(szString)  != GS::StdLib::Stdf::NoError)
                return;	// PHY_NAM
            ptPinmapCell->strPhysicName = FormatTestName(szString);

            if(StdfFile.ReadString(szString)  != GS::StdLib::Stdf::NoError)
                return;	// LOG_NAM
            ptPinmapCell->strLogicName = FormatTestName(szString);

            GQTL_STDF::Stdf_PMR_V4 pmr;

            StdfFile.ReadByte(&bData);	// HEAD_NUM
            pmr.SetHEAD_NUM(bData);
            StdfFile.ReadByte(&bData);	// SITE_NUM
            pmr.SetSITE_NUM(bData);

            unsigned short siteNumber =
                GQTL_STDF::HeadAndSiteNumberDecipher::GetSiteNumberIn
                (StdfFile, pmr);

            BYTE headNumber =
                GQTL_STDF::HeadAndSiteNumberDecipher::GetHeadNumberIn
                (StdfFile, pmr);

            ptPinmapCell->bHead = headNumber;
            ptPinmapCell->m_site = siteNumber;
            break;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
// Read STDF PGR (Pin Group Record) record. STDF V4 only.
/////////////////////////////////////////////////////////////////////////////
void CGexFileInGroup::ReadPGR(void)
{
    char	szString[257];	// A STDF string is 256 bytes long max!

    // Only read in PASS 1
    if(lPass == 2)
        return;

    StdfFile.ReadWord(&wData);	// GRP_INDX
    if(StdfFile.ReadString(szString)  != GS::StdLib::Stdf::NoError)
        return;	// GTP_NAM
    StdfFile.ReadWord(&wData);	// INDX_CNT

    int iCount = wData;
    while(iCount)
    {
        StdfFile.ReadWord(&wData);	// PMR_INDX: index pin belonging to this group
        iCount--;
    };
}

/////////////////////////////////////////////////////////////////////////////
// Read STDF PLR (Pin List Record) record.STDF V4 only.
/////////////////////////////////////////////////////////////////////////////
void CGexFileInGroup::ReadPLR(void)
{
    char	szString[257];	// A STDF string is 256 bytes long max!

    // Only read in PASS 1
    if(lPass == 2)
        return;

    StdfFile.ReadWord(&wData);	// GRP_CNT
    int iGrpCnt,iCount;
    iGrpCnt = iCount = wData;
    while(iCount)
    {
        StdfFile.ReadWord(&wData);	// GRP_INDX
        iCount--;
    };

    iCount = iGrpCnt;
    while(iCount)
    {
        StdfFile.ReadWord(&wData);	// GRP_MODE
        iCount--;
    };

    iCount = iGrpCnt;
    while(iCount)
    {
        StdfFile.ReadByte(&bData);	// GRP_RADX
        iCount--;
    };

    iCount = iGrpCnt;
    while(iCount)
    {
        StdfFile.ReadString(szString);	// PGM_CHAR
        iCount--;
    };
    iCount = iGrpCnt;
    while(iCount)
    {
        StdfFile.ReadString(szString);	// RTN_CHAR
        iCount--;
    };
    iCount = iGrpCnt;
    while(iCount)
    {
        StdfFile.ReadString(szString);	// PGM_CHAL
        iCount--;
    };
    iCount = iGrpCnt;
    while(iCount)
    {
        StdfFile.ReadString(szString);	// RTN_CHAL
        iCount--;
    };
}

/////////////////////////////////////////////////////////////////////////////
// Read STDF MPR Static information at end of the record, then seek back !
/////////////////////////////////////////////////////////////////////////////
// Read  (located at the end of the record)
int	CGexFileInGroup::ReadStaticDataMPR(CTest ** ptParamTestCell,
                                       unsigned long lTestNumber,
                                       long iPinmapMergeIndex,
                                       BYTE bSite,
                                       long nJcount,
                                       long nKcount,
                                       int *piCustomScaleFactor,
                                       bool bStrictLL, bool bStrictHL)
{
    long lRecordReadOffset          = 0;
    long lRecordReadOffsetRtnIndx   = 0;
    bool bIgnoreSite                = false;

    // If more results than Pin indexes...ignore the leading results
    if(iPinmapMergeIndex != GEX_PTEST)
    {
        // Multiple results: we need the individual pinmaps.
        if((nKcount==0) && (nJcount==0))
            return 1;	// No Result index list...then ignore this MPR!
    }

    if(((iProcessSite >=0) && (iProcessSite != (int)bSite)) )
    {
        bIgnoreSite = true;
    }

    CTest *		ptTestCell = NULL;
    CTest	*	ptMPTestCell = NULL;	// Pointer to test cell to create
    double		lfLowLimit=-C_INFINITE, lfHighLimit=C_INFINITE;
    char		szString[257]="";	// A STDF string is 256 bytes long max!
    int			iPinIndex=0,iCount=0,iPinCount=0;
    BYTE		bOptFlag=0;
    BYTE		bRes_scal=0;
    bool        isFirstRtnIndxOccurence = false;

    // Saves current READ offset in the STDF record.
    lRecordReadOffset = StdfFile.GetReadRecordPos();

    // Skip the RTN_RSLT fields ('nKcount' fields of 4bytes to skip)
    // so we point : TEST_TXT
    if(StdfFile.SetReadRecordPos(lRecordReadOffset + 4*nKcount) != GS::StdLib::Stdf::NoError)
        goto exit_function;	// Record is not that big !

    if(StdfFile.ReadString(szString)  != GS::StdLib::Stdf::NoError)
        goto exit_function;	// TEST_TXT text description (test name)

    // Formats test name (in case to long, or leading spaces)
    FormatTestName(szString);

    // Returns pointer to correct cell. If cell doesn't exist ; it's created...Test# mapping enabled.
    if(FindTestCell(lTestNumber, iPinmapMergeIndex, &ptTestCell, true, true,szString) !=1)
        return 0;	// Error

    // Get the pointer on the test cell created
    *ptParamTestCell = ptTestCell;

    if(StdfFile.ReadString(szString)  != GS::StdLib::Stdf::NoError)
        goto exit_function;	// ALARM_ID


    // Optional fields
    *piCustomScaleFactor = 0;
    if(StdfFile.ReadByte(&bOptFlag) != GS::StdLib::Stdf::NoError)
    {
        // Optional field
        // Goto the step Array of PMR indexes for test duplication
        nJcount = 0;
        goto read_RTNINDX;	// OPT_FLG
    }

    // Read result-scale factor in case it is a custom one for this record
    if((StdfFile.ReadByte(&bRes_scal) == GS::StdLib::Stdf::NoError) && (!(bOptFlag & 1)))
    {
        // Compute custom scale factor.: WILL BE INCORRECT ON 1st execution of PASS1
        // ... as ptTestCell->res_scal is not defined yet!...this is why this code
        // appears few lines below as well!
        *piCustomScaleFactor = (int)bRes_scal - ptTestCell->res_scal;
    }

    if((lPass == 2) || (ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_GOTTL))
    {
        // Pass1: ignore test definition limits if already defined!
        // Pass2: only refresh the list of pinmap indexes to use

        // Compute offset to seek to RTN_INDX offset: current offset+18 bytes
        lRecordReadOffsetRtnIndx = StdfFile.GetReadRecordPos()+18;
        goto read_RTNINDX;
    }

    // Are test limits strict??
    if(!bStrictLL)
        ptTestCell->GetCurrentLimitItem()->bLimitFlag |= CTEST_LIMITFLG_LTLNOSTRICT;
    if(!bStrictHL)
        ptTestCell->GetCurrentLimitItem()->bLimitFlag |= CTEST_LIMITFLG_HTLNOSTRICT;

    // If not in 'What-if' mode, save limits flag seen in STDF file
    if((bOptFlag & 0x10) == 0 && mUsedLimits != CGexFileInGroup::SPEC_LIMIT_ONLY)
    {
        // LowLimit+low_scal flag is valid...
        ptTestCell->GetCurrentLimitItem()->bLimitFlag &= ~(CTEST_LIMITFLG_NOLTL);
        if(bOptFlag & 0x10)	// Low Limit in first PTR
            ptTestCell->GetCurrentLimitItem()->bLimitFlag |= CTEST_LIMITFLG_NOLTL;
        else if(bOptFlag & 0x40)	// No Low Limit
            ptTestCell->GetCurrentLimitItem()->bLimitFlag |= CTEST_LIMITFLG_NOLTL;
    }
    // If not in 'What-if' mode, save limits flag seen in STDF file
    if((bOptFlag & 0x20) == 0 && mUsedLimits != CGexFileInGroup::SPEC_LIMIT_ONLY)
    {
        // HighLimit+high_scal flag is valid...
        ptTestCell->GetCurrentLimitItem()->bLimitFlag &= ~(CTEST_LIMITFLG_NOHTL);
        if(bOptFlag & 0x20)	// High Limit in first PTR
            ptTestCell->GetCurrentLimitItem()->bLimitFlag |= CTEST_LIMITFLG_NOHTL;
        else if(bOptFlag & 0x80)	// No High Limit
            ptTestCell->GetCurrentLimitItem()->bLimitFlag |= CTEST_LIMITFLG_NOHTL;
    }
    // If What-if has forced a LL
    if((ptTestCell->GetCurrentLimitItem()->bLimitWhatIfFlag & CTEST_LIMITFLG_NOLTL) == 0)
        ptTestCell->GetCurrentLimitItem()->bLimitFlag &= ~(CTEST_LIMITFLG_NOLTL);
    // If What-if has forced a HL
    if((ptTestCell->GetCurrentLimitItem()->bLimitWhatIfFlag & CTEST_LIMITFLG_NOHTL) == 0)
        ptTestCell->GetCurrentLimitItem()->bLimitFlag &= ~(CTEST_LIMITFLG_NOHTL);

    // If reached this point, it means test limits have been defined!
    if (bIgnoreSite == false)
        ptTestCell->GetCurrentLimitItem()->bLimitFlag |= CTEST_LIMITFLG_GOTTL;

    if((bOptFlag & 0x1) == 0)	// res-scal is valid
    {
        ptTestCell->res_scal = bRes_scal;
        // // Compute custom scale factor as first computation few lines above is incorrect in 1st call in pass#1!
        *piCustomScaleFactor = (int)bRes_scal - ptTestCell->res_scal;
    }

    if(StdfFile.ReadByte(&bData) != GS::StdLib::Stdf::NoError)
    {
        // Optional field
        // Goto the step Array of PMR indexes for test duplication
        nJcount = 0;
        goto read_RTNINDX;
    }
    if(((bData & 0x50) == 0) && ((bOptFlag & 0x10) == 0))	// llm-scal is valid
        ptTestCell->llm_scal = bData;

    if(StdfFile.ReadByte(&bData) != GS::StdLib::Stdf::NoError)
    {
        // Optional field
        // Goto the step Array of PMR indexes for test duplication
        nJcount = 0;
        goto read_RTNINDX;// hlm_scal
    }
    if(((bData & 0x120) == 0) && ((bOptFlag & 0x20) == 0))	// hlm-scal is valid
        ptTestCell->hlm_scal = bData;


    ///////////////////Read Low Limit
    if(StdfFile.ReadFloat(&fData) != GS::StdLib::Stdf::NoError)
    {
        // Optional field
        // Goto the step Array of PMR indexes for test duplication
        nJcount = 0;
        goto read_RTNINDX;// low limit
    }
    if(((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0)
            && ((bOptFlag & 0x10) == 0)
            && (mUsedLimits != CGexFileInGroup::SPEC_LIMIT_ONLY))
        lfLowLimit = M_CHECK_INFINITE((double)fData); // Low limit exists: keep value
    if(StdfFile.ReadFloat(&fData) != GS::StdLib::Stdf::NoError)
    {
        // if not "Use spec limits only"
        if (mUsedLimits != CGexFileInGroup::SPEC_LIMIT_ONLY)
            ptTestCell->GetCurrentLimitItem()->lfLowLimit = lfLowLimit;
        // Optional field
        // Goto the step Array of PMR indexes for test duplication
        nJcount = 0;
        goto read_RTNINDX;// Low limit
    }
    if(((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0)
            && ((bOptFlag & 0x20) == 0)
            && (mUsedLimits != CGexFileInGroup::SPEC_LIMIT_ONLY))
        lfHighLimit = M_CHECK_INFINITE((double)fData);// High limit exists: keep value


    if(StdfFile.ReadFloat(&fData) != GS::StdLib::Stdf::NoError)
    {
        // Optional field
        // Goto the step Array of PMR indexes for test duplication
        nJcount = 0;
        goto CalculateMPLimits;// start_in
    }
    if(bOptFlag & 0x1)
        ptTestCell->lfStartIn = fData;

    if(StdfFile.ReadFloat(&fData) != GS::StdLib::Stdf::NoError)
    {
        // Optional field
        // Goto the step Array of PMR indexes for test duplication
        nJcount = 0;
        goto CalculateMPLimits;// incr_in
    }
    if(bOptFlag & 0x1)
        ptTestCell->lfIncrementIn = fData;

    // Skip Pinmap index list for now...but will be read later in this function!
    // Skip 'nJcount' fields of 2bytes!
    // Saves current READ offset in the STDF record.
    lRecordReadOffsetRtnIndx = StdfFile.GetReadRecordPos();
    if(StdfFile.SetReadRecordPos(lRecordReadOffsetRtnIndx+2*nJcount) != GS::StdLib::Stdf::NoError)
        goto CalculateMPLimits;	// Record is not that big !

    // UNITS
    StdfFile.ReadString(szString);
    szString[GEX_UNITS-1] = 0;
    // If unit specified (and using printable ascii characters), then use it.
    if(*szString >= ' ')
        strcpy(ptTestCell->szTestUnits, QString(szString).trimmed().toLatin1().constData());

    // UNITS_IN
    StdfFile.ReadString(szString);
    szString[GEX_UNITS-1] = 0;
    if(*szString)
        strcpy(ptTestCell->szTestUnitsIn,szString);

    //Read unused info
    if(StdfFile.ReadString(szString)  != GS::StdLib::Stdf::NoError)
        goto CalculateMPLimits; // C_RESFMT
    if(StdfFile.ReadString(szString)  != GS::StdLib::Stdf::NoError)
        goto CalculateMPLimits; // C_LLMFMT
    if(StdfFile.ReadString(szString)  != GS::StdLib::Stdf::NoError)
        goto CalculateMPLimits; // C_HLMFMT

    // LO_SPEC
    if(StdfFile.ReadFloat(&fData) != GS::StdLib::Stdf::NoError)
        goto CalculateMPLimits;

    ptTestCell->lfLowSpecLimit = M_CHECK_INFINITE((double)fData);

    // If not "Use standard limits only"
    if(mUsedLimits == CGexFileInGroup::SPEC_LIMIT_IF_ANY)
    {
        // Flag test limit validity depending on spec limits flag
        if ((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLSL) == 0)
        {
            ptTestCell->GetCurrentLimitItem()->bLimitFlag &= ~CTEST_LIMITFLG_NOLTL;
            lfLowLimit = M_CHECK_INFINITE((double)fData);

            // No low specs limits as there are kept in std limits
            ptTestCell->GetCurrentLimitItem()->bLimitFlag |= CTEST_LIMITFLG_NOLSL;
        }
    }
    else if(mUsedLimits == CGexFileInGroup::SPEC_LIMIT_ONLY)
    {
        lfLowLimit = M_CHECK_INFINITE((double)fData);

        // Flag test limit validity depending on spec limits flag
        if ((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLSL) == 0)
        {
            ptTestCell->GetCurrentLimitItem()->bLimitFlag &= ~CTEST_LIMITFLG_NOLTL;

            // No low specs limits as there are kept in std limits
            ptTestCell->GetCurrentLimitItem()->bLimitFlag |= CTEST_LIMITFLG_NOLSL;
        }
        else
            ptTestCell->GetCurrentLimitItem()->bLimitFlag |= CTEST_LIMITFLG_NOLTL;
    }

    // HI_SPEC
    if(StdfFile.ReadFloat(&fData) != GS::StdLib::Stdf::NoError)
        goto CalculateMPLimits;

    ptTestCell->lfHighSpecLimit = M_CHECK_INFINITE((double)fData);

    // If not "Use standard limits only"
    if(mUsedLimits == CGexFileInGroup::SPEC_LIMIT_IF_ANY)
    {
        // Flag test limit validity depending on spec limits flag
        if ((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHSL) == 0)
        {
            ptTestCell->GetCurrentLimitItem()->bLimitFlag &= ~CTEST_LIMITFLG_NOHTL;
            lfHighLimit = M_CHECK_INFINITE((double)fData);

            // No low specs limits as there are kept in std limits
            ptTestCell->GetCurrentLimitItem()->bLimitFlag |= CTEST_LIMITFLG_NOHSL;
        }
    }
    else if(mUsedLimits == CGexFileInGroup::SPEC_LIMIT_ONLY)
    {
        lfHighLimit = M_CHECK_INFINITE((double)fData);

        // Flag test limit validity depending on spec limits flag
        if ((ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHSL) == 0)
        {
            ptTestCell->GetCurrentLimitItem()->bLimitFlag &= ~CTEST_LIMITFLG_NOHTL;

            // No low specs limits as there are kept in std limits
            ptTestCell->GetCurrentLimitItem()->bLimitFlag |= CTEST_LIMITFLG_NOHSL;
        }
        else
            ptTestCell->GetCurrentLimitItem()->bLimitFlag |= CTEST_LIMITFLG_NOHTL;
    }

CalculateMPLimits:

    if(m_datasetConfig.testToUpdateList().isActivated()){
        GexTestToUpdate *poUpdate = m_datasetConfig.testToUpdateList().getTestToUpdate(ptTestCell->lTestNumber);
        if(poUpdate){
            ptTestCell->GetCurrentLimitItem()->bLimitWhatIfFlag = CTEST_LIMITFLG_NOLTL | CTEST_LIMITFLG_NOHTL;
            if(!poUpdate->highLimit().isEmpty()){
                lfHighLimit = poUpdate->highLimit().toDouble();
                if(poUpdate->source() == "whatif")
                    ptTestCell->GetCurrentLimitItem()->bLimitWhatIfFlag	&= ~CTEST_LIMITFLG_NOHTL;
            }
            if(!poUpdate->lowLimit().isEmpty()){
                lfLowLimit = poUpdate->lowLimit().toDouble();
                if(poUpdate->source() == "whatif")
                    ptTestCell->GetCurrentLimitItem()->bLimitWhatIfFlag	&= ~CTEST_LIMITFLG_NOLTL;
            }
        }
    }

    // Store the test limits for this site
    if (m_ParentGroup->addTestSiteLimits(ptTestCell, iProcessSite, bSite, lfLowLimit, lfHighLimit,
                                         bOptFlag, getMirDatas().lStartT) == false)
        return false;

    // Retrieve the min and max tests limits for this site
    m_ParentGroup->getTestSiteLimits(ptTestCell, iProcessSite, bSite, lfLowLimit, lfHighLimit);

    // If High limit not forced by What-If...
    if (!bIgnoreSite && (ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOHTL) == 0 && ptTestCell->GetCurrentLimitItem()->bLimitWhatIfFlag & CTEST_LIMITFLG_NOHTL)
        ptTestCell->GetCurrentLimitItem()->lfHighLimit = lfHighLimit;

    // If Low limit not forced by What-If...
    if(!bIgnoreSite && (ptTestCell->GetCurrentLimitItem()->bLimitFlag & CTEST_LIMITFLG_NOLTL) == 0 && ptTestCell->GetCurrentLimitItem()->bLimitWhatIfFlag & CTEST_LIMITFLG_NOLTL)
        ptTestCell->GetCurrentLimitItem()->lfLowLimit = lfLowLimit;

    // Avoid failing if LL=HL
    if(ptTestCell->GetCurrentLimitItem()->lfLowLimit == ptTestCell->GetCurrentLimitItem()->lfHighLimit)
        ptTestCell->GetCurrentLimitItem()->bLimitFlag |= (CTEST_LIMITFLG_LTLNOSTRICT | CTEST_LIMITFLG_HTLNOSTRICT);


    // Here, we read the list of indexes: it may be possible that for each run,
    // the list changes for a same test!
read_RTNINDX:
    // Now, create as many test cells as Pins tested in this test!
    // Seek into record offset to read the RTN_INDX table!
    if(StdfFile.SetReadRecordPos(lRecordReadOffsetRtnIndx) != GS::StdLib::Stdf::NoError)
        goto exit_function;	// Record is not that big !...then we will use the latest pinmap indexes read for that test.
    if(ptTestCell->ptResultArrayIndexes != NULL)
        delete [] ptTestCell->ptResultArrayIndexes;	// erase previous buffer if any.
    // If current mode is to merge multiple parametric values...then exit now!
    if(ptTestCell->lPinmapIndex == GEX_PTEST)
    {
        ptTestCell->ptResultArrayIndexes = NULL;
        ptTestCell->bTestType = 'M';
        goto exit_function;
    }

    iPinCount = nJcount;
    isFirstRtnIndxOccurence = false;
    if(nJcount==0)
        iPinCount = nKcount;
    // Stores only first valid occurence of RTN_INDX
    else if (ptTestCell->ptFirstResultArrayIndexes == NULL)
    {
        ptTestCell->ptFirstResultArrayIndexes = new WORD[iPinCount];
        if(ptTestCell->ptFirstResultArrayIndexes == NULL)
            return 0;	// Error: memory alloc. failure...ignore this MPR
        isFirstRtnIndxOccurence = true;
    }

    ptTestCell->ptResultArrayIndexes = new WORD[iPinCount];
    ptTestCell->setPinCount(iPinCount);
    if(ptTestCell->ptResultArrayIndexes == NULL)
        return 0;	// Error: memory alloc. failure...ignore this MPR
    for(iCount=0;iCount<iPinCount;iCount++)
    {
        // Retrieve Pinmap index if any
        wData = iCount + 1; // case 5329 legal range starts at 1
        if(nJcount > 0)
            StdfFile.ReadWord(&wData);
        // Check if we have a reference array
        else if (!isFirstRtnIndxOccurence && ptTestCell->ptFirstResultArrayIndexes)
            wData = ptTestCell->ptFirstResultArrayIndexes[iCount];
        iPinIndex = wData;

        // save array of pinmap indexes.
        ptTestCell->ptResultArrayIndexes[iCount] = iPinIndex;
        if (isFirstRtnIndxOccurence)
            ptTestCell->ptFirstResultArrayIndexes[iCount] = iPinIndex;
        if(lPass == 1)
        {
            // Pass1: Save it into the test master record..Test# mapping enabled.
            if (FindTestCell(lTestNumber, iCount, &ptMPTestCell,
                             true, true,
                             ptTestCell->strTestName.toLatin1().data()) != 1)
            {
                goto exit_function;  // Error
            }
            ptMPTestCell->bTestType = 'M';			// Test type: Parametric multi-results
            // Copy test static info from master test definition
            ptMPTestCell->strTestName = ptTestCell->strTestName;
            ptMPTestCell->GetCurrentLimitItem()->bLimitFlag = ptTestCell->GetCurrentLimitItem()->bLimitFlag;
            ptMPTestCell->res_scal = ptTestCell->res_scal;
            ptMPTestCell->llm_scal = ptTestCell->llm_scal;
            ptMPTestCell->hlm_scal = ptTestCell->hlm_scal;
            ptMPTestCell->lfStartIn = ptTestCell->lfStartIn;
            ptMPTestCell->lfIncrementIn = ptTestCell->lfIncrementIn;
            strcpy(ptMPTestCell->szTestUnits, ptTestCell->szTestUnits);
            strcpy(ptMPTestCell->szTestUnitsIn, ptTestCell->szTestUnitsIn);

            // Retrieve the min and max tests limits for this site
            m_ParentGroup->getTestSiteLimits(ptTestCell, iProcessSite, bSite, lfLowLimit, lfHighLimit);

            // If What-if has forced a LL
            if((ptMPTestCell->GetCurrentLimitItem()->bLimitWhatIfFlag & CTEST_LIMITFLG_NOLTL) == 0)
                ptMPTestCell->GetCurrentLimitItem()->bLimitFlag &= ~(CTEST_LIMITFLG_NOLTL);
            else
                ptMPTestCell->GetCurrentLimitItem()->lfLowLimit = lfLowLimit;

            // If What-if has forced a HL
            if((ptMPTestCell->GetCurrentLimitItem()->bLimitWhatIfFlag & CTEST_LIMITFLG_NOHTL) == 0)
                ptMPTestCell->GetCurrentLimitItem()->bLimitFlag &= ~(CTEST_LIMITFLG_NOHTL);
            else
                ptMPTestCell->GetCurrentLimitItem()->lfHighLimit = lfHighLimit;
        }
    }

    // Keeps track of test result array size
    ptTestCell->lResultArraySize = gex_max(ptTestCell->lResultArraySize,nJcount);

    // MergeMultiParametricTests
    //ptTestCell->lResultArraySize=0;

    // Before returning to MPR processing, seekback into right record offset!
exit_function:
    // 'rewind' offset so we can now read test result fields!
    StdfFile.SetReadRecordPos(lRecordReadOffset);
    return 1;	// OK
}

/////////////////////////////////////////////////////////////////////////////
// For MPR record, may have to create a test name for each test result in array
/////////////////////////////////////////////////////////////////////////////
void CGexFileInGroup::BuildMultiResultParamTestName(CTest *ptTestCell,CTest *ptMPTestCell)
{
    if(ptTestCell->strTestName.isEmpty() == false)
        ptMPTestCell->strTestName = ptTestCell->strTestName;
}

/////////////////////////////////////////////////////////////////////////////
// Read STDF MPR record
/////////////////////////////////////////////////////////////////////////////
bool CGexFileInGroup::ReadMPR()
{
    int		test_flg=0,param_flg=0;
    float	fResult=0;
    BYTE	bSite=0,bHead=0;
    bool	bValideResult=false;
    bool	bTestIsFail=false, bOutsideLimit=false;
    bool	bAlarmResult=false;
    bool	bIsOutlier=false;
    bool	bStrictLL=true, bStrictHL=true;
    CTest	*ptTestCell=NULL;	// Pointer to test cell to receive STDF info.
    CTest	*ptMPTestCell=NULL;	// Pointer to test cell to create
    long	lTestNumber=0;
    int		iPinIndex=0;
    int		iPinmapMergeIndex=0;
    bool	bIsNAN=false;

    CTestResult::PassFailStatus	ePassFailStatus	= CTestResult::statusUndefined;

    int		nJcount=0,nKcount=0,iCount=0;
    int		iCustomScaleFactor;

    // MPR
    StdfFile.ReadDword(&lTestNumber);	// test number
    StdfFile.ReadByte(&bHead);		// Head
    StdfFile.ReadByte(&bSite);		// test site

    unsigned short siteNumber = bSite;
    if(StdfFile.GetHeadAndSiteNumberDecipheringMode() != GS::StdLib::no_deciphering)
    {
        siteNumber = GQTL_STDF::HeadAndSiteNumberDecipher::DecipherSiteNumberForAdvantest(bHead, bSite);
        bHead = GQTL_STDF::HeadAndSiteNumberDecipher::DecipherHeadNumberForAdvantest(bHead);
    }

    // Ignore MPR if filtering set to 'Bin data only'
    if(iProcessBins == GEX_PROCESSPART_NO_SAMPLES)
        return true;

    // Check if current MPR (test result) belongs to a part that we filter ?
    // If this part has a binning the User doesn't want to process, then
    // we skip this record...
    if(lPass == 2 && PartProcessed.IsFilteredPart(pReportOptions, siteNumber, false, iProcessBins) == true)
        return true;

    // Ignore test if we are in 'Stop on Fail' mode, and current site already failed!
    if((lPass == 2) && (m_eFailCountMode == FAILCOUNT_FIRST) && (m_map_ptFailTestCell.contains(siteNumber)))
        return true;

    switch(StdfRecordHeader.iStdfVersion)
    {
            break;

        case GEX_STDFV4:
            StdfFile.ReadByte(&bData);		// test_flg
            test_flg = (int) bData & 0xff;
            StdfFile.ReadByte(&bData);		// parm_flg
            param_flg = (int) bData & 0xff;

            // RTN_ICNT: Count (j) of PMR indexes
            if(StdfFile.ReadWord(&wData) != GS::StdLib::Stdf::NoError)
                return true;
            nJcount = wData;

            // RSLT_CNT: Count (k) of returned results
            if(StdfFile.ReadWord(&wData) != GS::StdLib::Stdf::NoError)
                return true;
            nKcount = wData;

            // Check if test limits are strict
            if(param_flg & 0100)
                bStrictLL = false;
            if(param_flg & 0200)
                bStrictHL = false;

            // Check if this test is marked 'fail' (bits 6 cleared, bit 7 set) ?
            //		if((test_flg & 0300) == 0200)
            //			bFailFlagSet = true;

            // Check if Pass/Fail indication is valid (bit 6 cleared)
            // Only if Pass/Fail rule option is checked
            if (m_bUsePassFailFlag && (test_flg & 0x40) == 0x00)
            {
                // If bit 7 set, then test is fail, otherwise test is pass
                ePassFailStatus = (test_flg & 0x80) ? CTestResult::statusFail : CTestResult::statusPass;
            }

            // Check if test result is valid
            if((test_flg & 16) || ((bData & 07) & (m_eStdfCompliancy==CGexFileInGroup::STRINGENT)))
                bValideResult = false;
            else
                bValideResult = true;

            // Ignore test result if must compute statistics from summary only
            if (m_eStatsComputation == FROM_SUMMARY_ONLY)
                bValideResult = false;

            // Check if test has an alarm
            if(test_flg & 1)
                bAlarmResult = true;

            // Read array of states( Nibbles: 8 bits = 2 states of 4 bits!).
            iCount = nJcount;
            while(iCount>0)
            {
                StdfFile.ReadByte(&bData);		// State[j]
                iCount-=2;
            }

            // Merge Pins under single test name or not?
            if(m_eMPRMergeMode==MERGE)
                iPinmapMergeIndex = GEX_PTEST;	// Merge Multiple parametric pins
            else
                iPinmapMergeIndex = GEX_MPTEST;	// NO Merge Multiple parametric pins

            // Read Static information (located at the end of the record)
            ptTestCell = NULL;
            if(ReadStaticDataMPR(&ptTestCell, lTestNumber, iPinmapMergeIndex, siteNumber, nJcount,nKcount,
                                 &iCustomScaleFactor, bStrictLL, bStrictHL) != 1)
                return true;  // some kind of error: ignore this record

            if((iProcessSite >=0) && (iProcessSite != (int)siteNumber))
                break;	// Site filter not matching.

            // Check if record is very short and doesn't even include the Parameter name
            if(ptTestCell == NULL)
            {
                // No Parameter name found in this record...then rely on Parameter Number only...
                if(FindTestCell(lTestNumber, iPinmapMergeIndex, &ptTestCell) !=1)
                    return true;  // Error
            }

            // Update Test# as mapping is activated...
            lTestNumber = ptTestCell->lTestNumber;

            // If more results than Pin indexes...ignore the leading results
            if(iPinmapMergeIndex != GEX_PTEST)
            {
                // Multiple results: we need the individual pinmaps.
                if((nJcount>0) && (nKcount > nJcount))
                    nKcount = nJcount;
            }
            // Read each result in array...and process it!
            for(iCount=0;iCount<nKcount;iCount++)
            {
                if(iPinmapMergeIndex == GEX_PTEST)
                {
                    // If merging Multiple parametric results.
                    iPinIndex = GEX_PTEST;
                    ptMPTestCell = ptTestCell;
                }
                else
                {
                    // Find Test cell matching current pin result
                    if(ptTestCell->ptResultArrayIndexes)
                        iPinIndex = ptTestCell->ptResultArrayIndexes[iCount];
                    else
                        iPinIndex = 1;

                    // DO NOT allow test# mapping as it was already done in above call to 'FindTestCell'
                    if (FindTestCell(lTestNumber, iCount, &ptMPTestCell,
                                     false, true,
                                     ptTestCell->strTestName.
                                     toLatin1().data()) != 1)
                    {
                        return true;  // Error
                    }
                }

                // Read Pin Parametric test result.
                StdfFile.ReadFloat(&fResult, &bIsNAN);
                if(bIsNAN)
                    bValideResult = false;

                if(bValideResult != true)
                    goto NextTestResult;

                if(lPass == 1)
                {
                    // Check if this test is an outlier
                    if(PartIsOutlier(ptMPTestCell,fResult) == true)
                        goto NextTestResult;

                    // Updates Min/Max values
                    UpdateMinMaxValues(ptMPTestCell,fResult);

                    ptMPTestCell->addMultiResultCount(siteNumber);
                }
                else
                {
                    // PASS2:

                    // Ignore STDF FAIL flag if What-If limits defined!
                    if(ptMPTestCell->GetCurrentLimitItem()->bLimitWhatIfFlag != (CTEST_LIMITFLG_NOLTL | CTEST_LIMITFLG_NOHTL))
                        ePassFailStatus = CTestResult::statusUndefined;

                    // Check if this test is an outlier
                    bIsOutlier = PartIsOutlier(ptMPTestCell,fResult);

                    // Check if test is fail
                    bTestIsFail = IsTestFail(ptTestCell, fResult, ePassFailStatus, bOutsideLimit);

                    // Keep track of failing count (unless it's an outlier)
                    if((bIsOutlier == false) && CountTestAsFailure(siteNumber,ptTestCell,iPinIndex,fResult,bTestIsFail) == true)
                    {
                        // Update failing count
                        ptMPTestCell->GetCurrentLimitItem()->ldSampleFails++;
                        m_bFailingSiteDetails[siteNumber] |= GEX_RUN_FAIL_TEST;

                        // If failure and What-If limits enabled, make sure to force a failing bin!
                        if(ptMPTestCell->GetCurrentLimitItem()->bLimitWhatIfFlag != (CTEST_LIMITFLG_NOLTL | CTEST_LIMITFLG_NOHTL))
                            m_bFailingSiteDetails[siteNumber] |= GEX_RUN_FAIL_WHATIFTEST;
                    }

                    // If failure, make sure we mark it.
                    if(bTestIsFail)
                        CountTrackFailingTestInfo(siteNumber,ptTestCell,iPinIndex,fResult);

                    // Advanced report is Enabled, and test in filter limits.
                    switch(pReportOptions->getAdvancedReport())
                    {
                        case GEX_ADV_DISABLED:
                        case GEX_ADV_PAT_TRACEABILITY:
                        default:
                            // If Gex-Advanced edition, ALWAYS collect test results!
                            if(bIsOutlier)
                                goto NextTestResult;
                            // collect sample to allow Interactive Charting (Unless size excees cutoff limit)
                            if(pReportOptions->bSpeedCollectSamples)
                            {
                                if (AdvCollectTestResult(ptMPTestCell, fResult, ePassFailStatus, siteNumber, true) == false)
                                    return false;
                            }
                            break;

                        case GEX_ADV_DATALOG:
                            if (GexAbstractDatalog::existInstance())
                            {
                                if (GexAbstractDatalog::isDataloggable(ptTestCell, bTestIsFail, bIsOutlier))
                                {
                                    int nRunIndex = PartProcessed.runIndexFromSite(siteNumber);
                                    if (nRunIndex >= 0 && nRunIndex < pPartInfoList.count())
                                        GexAbstractDatalog::pushParametricResult(this, ptTestCell, pPartInfoList.at(nRunIndex), fResult, bTestIsFail, bAlarmResult, siteNumber);
                                }
                            }
                            else
                                AdvCollectDatalog(ptMPTestCell,fResult,bTestIsFail,bAlarmResult, bIsOutlier, siteNumber, (int)bHead, (int)siteNumber);

                            // Check if outlier only after datalog, in case we collect outliers!
                            if(bIsOutlier)
                                goto NextTestResult;
                            // collect sample to allow Interactive Charting (Unless size excees cutoff limit)
                            if(pReportOptions->bSpeedCollectSamples)
                                AdvCollectTestResult(ptMPTestCell, fResult ,ePassFailStatus, siteNumber, true);

                            break;
                        case GEX_ADV_HISTOGRAM:
                        case GEX_ADV_TREND:
                        case GEX_ADV_CANDLE_MEANRANGE:
                        case GEX_ADV_GO_DIAGNOSTICS:	// Optimizer Diagnostic takes ALL Trend results!
                            if(bIsOutlier)
                                goto NextTestResult;
                            AdvCollectTestResult(ptMPTestCell, fResult, ePassFailStatus, siteNumber, pReportOptions->bSpeedCollectSamples);

                            break;
                        case GEX_ADV_CORRELATION:
                            if(bIsOutlier)
                                goto NextTestResult;

                            AdvCollectTestResult(ptMPTestCell, fResult, ePassFailStatus, siteNumber, pReportOptions->bSpeedCollectSamples);
                            break;
                    }
                    // If this is an outlier (data to totally IGNORE)...don't take it into account!
                    if(bIsOutlier)
                        return true;

#if 0 // Herve Thomy : // OBSOLETE MEMBER Not used anymore,since we hold the runIndex in the CPartBinning class except with MASA plugins
                    // Keeps tracking the test# result.
                    ptMPTestCell->ldCurrentSample++;
#endif

                    // Build Cpk
                    ptMPTestCell->lfSamplesTotal += fResult;				// Sum of X
                    ptMPTestCell->lfSamplesTotalSquare += (fResult*fResult);// Sum of X*X

                    ptTestCell->mHistogramData.AddValue(fResult);
                    if((fResult >= ptMPTestCell->GetCurrentLimitItem()->lfHistogramMin) && (fResult <= ptMPTestCell->GetCurrentLimitItem()->lfHistogramMax))
                    {
                        double lfCellHistogramMax = ptMPTestCell->GetCurrentLimitItem()->lfHistogramMax;
                        double lfCellHistogramMin = ptMPTestCell->GetCurrentLimitItem()->lfHistogramMin;

                        int iCell = 0;
                        if(lfCellHistogramMin != lfCellHistogramMax)        // Comment : lfCellHistogramMin <= fResult <= lfCellHistogramMax already tested
                        {
                            iCell = (unsigned int)( (TEST_HISTOSIZE*(fResult - lfCellHistogramMin)) / (lfCellHistogramMax-lfCellHistogramMin));
                        }
                        // else, only one bar iCell = 0

                        // Incluse result if it in into the viewing window.
                        if( (iCell>=0) && (iCell<TEST_HISTOSIZE) )      // check iCell correspond to a valid table index (ptMPTestCell->lHistogram[] size = TEST_HISTOSIZE)
                            ptMPTestCell->lHistogram[iCell]++;
                        else if (iCell >= TEST_HISTOSIZE)
                            ptMPTestCell->lHistogram[TEST_HISTOSIZE-1]++;   /// TO REVIEW : is it normal that iCell can exceed than TEST_HISTOSIZE
                        else
                        {
                            GSLOG(SYSLOG_SEV_ERROR,
                                     QString("Invalid value detected to build histogram for test %1 [%2]")
                                     .arg(ptMPTestCell->lTestNumber)
                                     .arg(ptMPTestCell->strTestName).toLatin1().constData());

                            GSLOG(SYSLOG_SEV_DEBUG, QString("\tCell Found: %1").arg(iCell).toLatin1().constData());
                            GSLOG(SYSLOG_SEV_DEBUG, QString("\tMPR result: %1").arg( fResult).toLatin1().constData());
                            GSLOG(SYSLOG_SEV_DEBUG, QString("\tHistogram Min: %1").arg(
                                     ptMPTestCell->GetCurrentLimitItem()->lfHistogramMin).toLatin1().constData());
                            GSLOG(SYSLOG_SEV_DEBUG, QString("\tHistogram Max: %1").arg(
                                     ptMPTestCell->GetCurrentLimitItem()->lfHistogramMax).toLatin1().constData());
                        }
                    }
                } // In pass2 only
                // Loop over all tests in array.
NextTestResult:;
            }	// Loop for as many test results as Pins in the pin list...

    }// STDF version switch.
    return true;
}

/////////////////////////////////////////////////////////////////////////////
// Read STDF FDR record STDV V3 only
/////////////////////////////////////////////////////////////////////////////
void CGexFileInGroup::ReadFDR(void)
{
}

/////////////////////////////////////////////////////////////////////////////
// Read STDF FTR record
/////////////////////////////////////////////////////////////////////////////
bool CGexFileInGroup::ReadFTR(void)
{
    bool 	bValideResult	= false;
    bool	bFailResult		= false;
    CTest	*ptTestCell		= NULL;			// Pointer to test cell to receive STDF info.
    unsigned uMapIndex;
    float	fResult;

    CTestResult::PassFailStatus	ePassFailStatus	= CTestResult::statusUndefined;

    // Check if FTR processing is disabled
    if(m_eIgnoreFunctionalTests == FTR_DISABLED)
        return true;

    // Ignore FTR if filtering set to 'Bin data only'
    if(iProcessBins == GEX_PROCESSPART_NO_SAMPLES)
        return true;

    switch(StdfRecordHeader.iStdfVersion)
    {
    default :
            break;

#ifdef OBSOLETE_READ_FTR
        case GEX_STDFV4:

            // FTR
            StdfFile.ReadDword(&lTestNumber);	// test number
            StdfFile.ReadByte(&bHead);	// Head
            StdfFile.ReadByte(&bSite);	// test site

            // Check if current FTR (functional test result) belongs to a part that we filter ?
            // If this part has a binning the User doesn't want to process, then
            // we skip this record...
            if(lPass == 2 && PartProcessed.IsFilteredPart(pReportOptions, bSite) == true)
                return;

            // Ignore test if we are in 'Stop on Fail' mode, and current site already failed!
            if((lPass == 2) && (m_eFailCountMode == FAILCOUNT_FIRST) && (m_map_ptFailTestCell.contains(bSite)))
                return;

            if((iProcessSite >=0) && (iProcessSite != (int)bSite))
                break;	// Site filter not matching.

            // Check if this FTR is within a valid PIR/PRR block.
            uMapIndex = (bHead << 8) | bSite;	// Compute index for Head&Site...as in multi-head/sites, we have to save multiple results before the PRR
            if((m_eStdfCompliancy==CGexFileInGroup::STRINGENT) && lMappingSiteHead[uMapIndex] != 1)
                break;

            // test_flg
            StdfFile.ReadByte(&bData);

            // Check if this test is marked 'fail' (bits 6 cleared, bit 7 set) ?
            // Check if Pass/Fail indication is valid (bit 6 cleared)
            if ((bData & 0x40) == 0x00)
            {
                // If bit 7 set, then test is fail, otherwise test is pass
                ePassFailStatus = (bData & 0x80) ? CTestResult::statusFail : CTestResult::statusPass;
                bFailResult		= (ePassFailStatus == CTestResult::statusFail) ? true : false;
            }

            // Check if test result is valid
            if((bData & 0100) || (bData & 16))
                bValideResult = false;
            else
                bValideResult = true;

            // Ignore test result if must compute statistics from summary only
            if(m_eStatsComputation == FROM_SUMMARY_ONLY)
                bValideResult = false;

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

            StdfFile.ReadString(szVector);	// vect_name
            if(*szVector == 0)
                strcpy(szVector,"-");		// Force a name if none defined.

            StdfFile.ReadString(szString);	// time_set
            StdfFile.ReadString(szString);	// op_code
            StdfFile.ReadString(szString);	// test_txt: test name

            // Formats test name (in case to long, or leading spaces)
            FormatTestName(szString);

            // Returns pointer to correct cell. If cell doesn't exist ; its created...Test# mapping enabled.
            if(FindTestCell(lTestNumber,GEX_FTEST,&ptTestCell,true,true,false,szString) !=1)
                return;	// Error

            if(*szString && (ptTestCell->strTestName.isEmpty() == true))
                ptTestCell->strTestName = szString;

            // Test is Functionnal
            ptTestCell->bTestType = 'F';

            StdfFile.ReadString(szString);	// alarm_id
            StdfFile.ReadString(szString);	// prog_txt
            StdfFile.ReadString(szString);	// rslt_txt
            StdfFile.ReadByte(&bData);		// patg_num
            StdfFile.ReadString(szString);	// spin_map

            if (bValideResult)
            {
                ptTestCell->bTestExecuted = true;

                // Update tmp buffer sample count: number of time this test is executed in a run.
                ptTestCell->ldTmpSamplesExecs++;
            }

            if((bValideResult == true) && (lPass == 2))
            {
                // PASS2:

                // Keep track of failing count (unless it's an outlier)
                if(bFailResult)
                {
                    // Update failing count
                    ptTestCell->ldSampleFails++;

                    // Keep failure history.
                    m_bFailingSiteDetails[bSite] |= GEX_RUN_FAIL_TEST;

                    // If failure, make sure we mark it.
                    CountTrackFailingTestInfo(bSite,ptTestCell,GEX_FTEST,0);
                }

                if(bFailResult)
                    fResult = 0.0;	// Functionnal Failure
                else
                    fResult = 1.0;	// Functionnal PAss.

                // Advanced report is Enabled, and test in filter limits.
                switch(pReportOptions->iAdvancedReport)
                {
                    case GEX_ADV_DISABLED:
                    default:
                        break;

                    case GEX_ADV_DATALOG:
                        AdvCollectDatalog(ptTestCell,fResult,bFailResult,false,false, bSite);
                        break;
                }

                // collect sample to allow Interactive Charting (Unless size excees cutoff limit)
                {
                    if(pReportOptions->bSpeedCollectSamples)
                        AdvCollectTestResult(ptTestCell, fResult, ePassFailStatus, bSite, true);
                }

#if 0 // Herve Thomy : // OBSOLETE MEMBER Not used anymore,since we hold the runIndex in the CPartBinning class except with MASA plugins
                // Keeps tracking the test# execution count (pattern).
                ptTestCell->ldCurrentSample++;
#endif

                // Save vector info
                CFunctionalTest cVectorResult;
                if(ptTestCell->mVectors.find(szVector) == ptTestCell->mVectors.end())
                {
                    // First failing vector in this test
                    cVectorResult.lExecs = 1;
                    cVectorResult.lFails = (bFailResult) ? 1: 0;
                    cVectorResult.strVectorName = szVector;
                }
                else
                {
                    // This vector already in list, then increment Exec count & update fail count
                    cVectorResult = ptTestCell->mVectors[szVector];
                    // Increment exec count
                    cVectorResult.lExecs++;
                    // Update fail count
                    if(bFailResult)
                        cVectorResult.lFails++;
                }
                // Save updated vector info
                ptTestCell->mVectors[szVector] = cVectorResult;
            }
            break;
#else
        case GEX_STDFV4:

            // First read TSR completely
            GQTL_STDF::Stdf_FTR_V4 clFTR_V4;

/*
            GQTL_STDF::Stdf_FTR_V4 clFilterFTR;

            if (iProcessSite >= 0)
            {
                clFilterFTR.SetSITE_NUM(iProcessSite & 0xff);
                clFTR_V4.SetFilter(&clFilterFTR);
            }
*/
            if (clFTR_V4.Read(StdfFile) == false)
                return true;

            // read the real site number
            unsigned short siteNumber =
                GQTL_STDF::HeadAndSiteNumberDecipher::GetSiteNumberIn
                ( StdfFile, clFTR_V4 );

            // overwrite head number by deciphering it if needed
            clFTR_V4.SetHEAD_NUM
                (
                    GQTL_STDF::HeadAndSiteNumberDecipher::GetHeadNumberIn
                        ( StdfFile, clFTR_V4 )
                );

            // Check if current FTR (functional test result) belongs to a part that we filter ?
            // If this part has a binning the User doesn't want to process, then
            // we skip this record...
            if(lPass == 2 && PartProcessed.IsFilteredPart(pReportOptions, siteNumber, false, iProcessBins) == true)
                return true;

            // Ignore test if we are in 'Stop on Fail' mode, and current site already failed!
            if((lPass == 2) && (m_eFailCountMode == FAILCOUNT_FIRST) && (m_map_ptFailTestCell.contains(siteNumber)))
                return true;

            if((iProcessSite >=0) && (iProcessSite != (int)siteNumber))
                break;	// Site filter not matching.

            // Check if this FTR is within a valid PIR/PRR block.
            // Compute index for Head&Site...as in multi-head/sites, we have to save multiple results before the PRR
            uMapIndex = (clFTR_V4.m_u1HEAD_NUM << 16) | siteNumber;
            if((m_eStdfCompliancy==CGexFileInGroup::STRINGENT) && lMappingSiteHead[uMapIndex] != 1)
                break;

            // Check if this test is marked 'fail' (bits 6 cleared, bit 7 set) ?
            // Check if Pass/Fail indication is valid (bit 6 cleared)
            if ((clFTR_V4.m_b1TEST_FLG & 0x40) == 0x00)
            {
                // If bit 7 set, then test is fail, otherwise test is pass
                ePassFailStatus = (clFTR_V4.m_b1TEST_FLG & 0x80) ? CTestResult::statusFail : CTestResult::statusPass;
                bFailResult		= (ePassFailStatus == CTestResult::statusFail) ? true : false;
            }

            // Check if test result is valid
            if((clFTR_V4.m_b1TEST_FLG & 0100) || (clFTR_V4.m_b1TEST_FLG & 16))
                bValideResult = false;
            else
                bValideResult = true;

            // Ignore test result if must compute statistics from summary only
            if(m_eStatsComputation == FROM_SUMMARY_ONLY)
                bValideResult = false;

            QString strVectorName	= clFTR_V4.m_cnVECT_NAM;
            QString strTestName		= clFTR_V4.m_cnTEST_TXT.trimmed();

            if(strVectorName.isNull() || strVectorName.isEmpty())
                strVectorName = "-";		// Force a name if none defined.

            // Formats test name (in case to long, or leading spaces)
            FormatTestName(strTestName);

            // Returns pointer to correct cell. If cell doesn't exist ; its created...Test# mapping enabled.
            if(FindTestCell(clFTR_V4.m_u4TEST_NUM, GEX_FTEST, &ptTestCell, true, true, strTestName.toLatin1().data()) !=1)
                return "ok";	// Error

            if(strTestName.isEmpty() == false && (ptTestCell->strTestName.isEmpty() == true))
                ptTestCell->strTestName = strTestName;

            // Test is Functionnal
            GEX_ASSERT(ptTestCell->bTestType == 'F');

            if (bValideResult)
            {
                ptTestCell->bTestExecuted = true;

                // Update tmp buffer sample count: number of time this test is executed in a run.
                ptTestCell->ldTmpSamplesExecs++;
            }

            if((bValideResult == true) && (lPass == 2))
            {
                // PASS2:
                // Keep track of failing count (unless it's an outlier)
                if(bFailResult)
                {
                    // Update failing count
                    ptTestCell->GetCurrentLimitItem()->ldSampleFails++;
                    // Keep failure history.
                    m_bFailingSiteDetails[siteNumber] |= GEX_RUN_FAIL_TEST;
                    // If failure, make sure we mark it.
                    CountTrackFailingTestInfo(siteNumber, ptTestCell, GEX_FTEST, 0);
                }

                if(bFailResult)
                    fResult = 0.0;	// Functionnal Failure
                else
                    fResult = 1.0;	// Functionnal PAss.

                // Save vector info
                CFunctionalTest cVectorResult;
                if(ptTestCell->mVectors.find(strVectorName) == ptTestCell->mVectors.end())
                {
                    // First failing vector in this test
                    cVectorResult.lExecs = 1;
                    cVectorResult.lFails = (bFailResult) ? 1: 0;
                    cVectorResult.strVectorName = strVectorName;
                }
                else
                {
                    // This vector already in list, then increment Exec count & update fail count
                    cVectorResult = ptTestCell->mVectors[strVectorName];
                    // Increment exec count
                    cVectorResult.lExecs++;
                    // Update fail count
                    if(bFailResult)
                        cVectorResult.lFails++;
                }

                GexVectorFailureInfo vectorFailure(clFTR_V4.m_u4CYCL_CNT, clFTR_V4.m_u4REL_VADR);

                // Add vector information on pin failed
                if(pReportOptions->getAdvancedReport() == GEX_ADV_DATALOG
                   || pReportOptions->getAdvancedReport() == GEX_ADV_FUNCTIONAL)
                {
                    if (bFailResult && clFTR_V4.m_u4NUM_FAIL > 0)
                    {
                        uint		uiFailPinCount	= 0;
                        char		cBitField		= 0;
                        CPinmap *	pPinmap			= NULL;

                        // Looking for every pin failed
                        for (uint nIndex = 0; nIndex < clFTR_V4.m_dnFAIL_PIN.m_uiLength && uiFailPinCount < clFTR_V4.m_u4NUM_FAIL; nIndex++)
                        {
                            cBitField = clFTR_V4.m_dnFAIL_PIN.m_pBitField[nIndex];
                            if(!cBitField) continue;

                            for (int nBit = 0; nBit < 8; nBit++)
                            {
                                if (cBitField & 0x01)
                                {
                                    if (FindPinmapCell(&pPinmap, (nIndex*8) + nBit))
                                        vectorFailure.addPinmap(pPinmap);
                                    uiFailPinCount++;
                                }
                                cBitField = cBitField >> 1;
                            }
                        }
                        cVectorResult.m_lstVectorInfo.append(vectorFailure);
                    }
                }

                // Save updated vector info
                ptTestCell->mVectors[strVectorName] = cVectorResult;

                // Advanced report is Enabled, and test in filter limits.
                switch(pReportOptions->getAdvancedReport())
                {
                    case GEX_ADV_DISABLED:
                    default:
                        break;

                    case GEX_ADV_DATALOG:

                        if (GexAbstractDatalog::existInstance())
                        {
                            if (GexAbstractDatalog::isDataloggable(ptTestCell, bFailResult, false))
                            {

                                int nRunIndex = PartProcessed.runIndexFromSite(siteNumber);
                                if (nRunIndex >= 0 && nRunIndex < pPartInfoList.count())
                                    GexAbstractDatalog::pushFunctionalResult(
                                                this, ptTestCell, pPartInfoList.at(nRunIndex),
                                                fResult, bFailResult, strVectorName, vectorFailure, siteNumber);
                            }
                        }
                        else
                            AdvCollectDatalog(ptTestCell, fResult, bFailResult, false, false, siteNumber, (int)clFTR_V4.m_u1HEAD_NUM, (int)siteNumber);
                        break;
                }

                // collect sample to allow Interactive Charting (Unless size excees cutoff limit)
                {
                    if(pReportOptions->bSpeedCollectSamples)
                    {
                        if (AdvCollectTestResult(ptTestCell, fResult, ePassFailStatus, siteNumber, true) == false)
                            return false;
                    }
                }

            }

            break;
#endif // OBSOLETE_READ_FTR
    }
    return true;
}

//----------------------------------------------
bool CGexFileInGroup::UpdateBinResult(CPartInfo *pPartInfo)
{
    bool bIgnorePart;

    // Keep track of binning result of each part.
    // Used during pass 2 for filtering data.
    if(lPass == 1)
        PartProcessed.saveBinning(pPartInfo, iProcessBins);		// Save Part bin result.

    // Check if this part must be filtered. If so, then do not
    // save the Die info.
    bIgnorePart = PartProcessed.IsFilteredPart(pReportOptions, pPartInfo->m_site, false, iProcessBins);

    return bIgnorePart;
}

//----------------------------------------------
// Called while processing PRR
//----------------------------------------------
void CGexFileInGroup::UpdateWaferMap(int iBin, CPartInfo *pPartInfo, long lInternalPartID)
{
    int iSiteID = pPartInfo->m_site;

    // Convert from 'unsigned int' to 'int'
    if(pPartInfo->iDieX >= 32768) pPartInfo->iDieX -= 65536;
    if(pPartInfo->iDieY >= 32768) pPartInfo->iDieY -= 65536;

    // Invalid or No Die location available...if we have to create a StripMap, then build location!
    if(getWaferMapData().bStripMap && ((pPartInfo->iDieX == -32768) || (pPartInfo->iDieY == -32768)))
    {
        pPartInfo->iDieX = lInternalPartID % getWaferMapData().SizeX;
        pPartInfo->iDieY = getWaferMapData().iHighDieY - (lInternalPartID / getWaferMapData().SizeX);
    }

    // Compute index for Head&Site...as in multi-head/sites, we have to save multiple results before the PRR
    unsigned uMapIndex = (pPartInfo->bHead << 16) | iSiteID;

    if (lPass == 1)
    {
        CPartInfo * ptTempPartInfo = NULL;
        if( (PartProcessed.IsFilteredPart(pReportOptions, iSiteID, false, iProcessBins) == false))
        {
            // DRILL: For all these cases: save PArtInfo data.
            ptTempPartInfo = new CPartInfo();
            // Copy data
            *ptTempPartInfo = *pPartInfo;

            // Add buffer to list
            pPartInfoList.append(ptTempPartInfo);
        }
    }
    else if(lPass == 2)
    {
        if(PartProcessed.IsFilteredPart(pReportOptions, iSiteID, false, iProcessBins) == false)
        {
            // If Drilling mode: save PartInfo for all parts, even if no WaferMap XY info valid.
            CPartInfo * ptTempPartInfo = NULL;

            if (PartProcessed.hasResultsCount())
            {
                // Get Global run index (through the whole group)
                int nRunIndex = PartProcessed.runIndexFromSite(iSiteID);

                // Compute local run index (for this file)
                nRunIndex -= PartProcessed.groupRunOffset();

                // Retrieve the corresponding PartInfo for the given local run index
                if (nRunIndex >= 0 && nRunIndex < pPartInfoList.count())
                    ptTempPartInfo	= pPartInfoList[nRunIndex];
                else
                {
                    GSLOG(SYSLOG_SEV_ERROR, QString("Invalid run index found when reading file %1")
                          .arg( strFileNameSTDF).toLatin1().constData());
                    GEX_ASSERT(false);
                }
            }

            // Case 6060
            if (ptTempPartInfo && getWaferMapData().bStripMap)
            {
                ptTempPartInfo->iDieX = pPartInfo->iDieX;
                ptTempPartInfo->iDieY = pPartInfo->iDieY;
            }

            if (ptTempPartInfo)
            {
                // Save complementary info: Last failing test#
                CTest *ptFailTestCell=NULL;
                if(m_map_ptFailTestCell.contains(iSiteID))
                    ptFailTestCell = m_map_ptFailTestCell[iSiteID];

                if(ptFailTestCell != NULL)
                    ptTempPartInfo->iFailTestNumber = ptFailTestCell->lTestNumber;
                else
                    ptTempPartInfo->iFailTestNumber = (unsigned)-1;

                ptTempPartInfo->iFailTestPinmap = m_mapFailTestPinmap[iSiteID];
                ptTempPartInfo->fFailingValue   = m_mapfFailingValue[iSiteID];

                // Save Zonal value if any used
                if(imapZoningTestValue.contains(uMapIndex))
                {
                    ptTempPartInfo->fValue = fmapZoningTestValue[uMapIndex];
                    ptTempPartInfo->iValue = imapZoningTestValue[uMapIndex];
                }

                if (m_map_ptFailedTestGoodResult.contains(iSiteID))
                    ptTempPartInfo->lstFailedTestGoodResult = m_map_ptFailedTestGoodResult[iSiteID];

                if (m_map_ptPassedTestBadResult.contains(iSiteID))
                    ptTempPartInfo->lstPassedTestBadResult	= m_map_ptPassedTestBadResult[iSiteID];

                // Reset Failing Test# after each run (for its given SiteID)
                if(m_map_ptFailTestCell.contains(iSiteID))
                    m_map_ptFailTestCell.remove(iSiteID);
                if(m_mapFailTestNumber.contains(iSiteID))
                    m_mapFailTestNumber.remove(iSiteID);
                if(m_mapFailTestPinmap.contains(iSiteID))
                    m_mapFailTestPinmap.remove(iSiteID);
                if (m_map_ptFailedTestGoodResult.contains(iSiteID))
                    m_map_ptFailedTestGoodResult.clear();
                if (m_map_ptPassedTestBadResult.contains(iSiteID))
                    m_map_ptPassedTestBadResult.clear();
            }
        }
    }

    // Invalid Wafer map data
    if((pPartInfo->iDieX == -32768) || (pPartInfo->iDieY == -32768) || (getWaferMapData().bPartialWaferMap==true))
        goto exit_UpdateWaferMap;

    if(lPass == 1)
    {
        getWaferMapData().iLowDieX = gex_min(getWaferMapData().iLowDieX,pPartInfo->iDieX);
        getWaferMapData().iHighDieX= gex_max(getWaferMapData().iHighDieX,pPartInfo->iDieX);
        getWaferMapData().iLowDieY  = gex_min(getWaferMapData().iLowDieY,pPartInfo->iDieY);
        getWaferMapData().iHighDieY = gex_max(getWaferMapData().iHighDieY,pPartInfo->iDieY);
        getWaferMapData().bWaferMapExists = true;
        // Assume wafermap is a real one if we have more than 1 die in X and Y
        // Summary exists or will be created with wafermap...so no need to use the PRR data !
        //		if((getWaferMapData().iLowDieX != getWaferMapData().iHighDieX) && (getWaferMapData().iLowDieY != getWaferMapData().iHighDieY))
        //			bBinningSummary=true;
        // In case we are forced to build Binning from Samples, overwrite status flag!

        //		if (m_eBinComputation == SAMPLES)
        //			bBinningSummary=false;	// Only use PRR to compute Binning.
    }
    else
    {
        // Pass 2:
        if((getWaferMapData().iHighDieX - getWaferMapData().iLowDieX) >= getWaferMapData().SizeX ||
           (getWaferMapData().iHighDieY - getWaferMapData().iLowDieY) >= getWaferMapData().SizeY)
        {
            getWaferMapData().bPartialWaferMap = true;
            goto exit_UpdateWaferMap;
        }
        switch(pReportOptions->iWafermapType)
        {
            case GEX_WAFMAP_TESTOVERLIMITS:
            case GEX_WAFMAP_STACK_TESTOVERLIMITS:
                // Zoning over test limits
            case GEX_WAFMAP_TESTOVERDATA:
            case GEX_WAFMAP_STACK_TESTOVERDATA:
            case GEX_WAFMAP_TEST_PASSFAIL:
            case GEX_WAFMAP_STACK_TEST_PASSFAIL:
                // Zoning over test values range
                // Change binning to test zoning value...then fall into CASE 0 !
                if(imapZoningTestValue.contains(uMapIndex))
                    iBin = imapZoningTestValue[uMapIndex];		// Parametric result exists
                else
                    iBin  = GEX_WAFMAP_EMPTY_CELL;			// No parametric result fro this run
            case GEX_WAFMAP_SOFTBIN:
            case GEX_WAFMAP_STACK_SOFTBIN:
            case GEX_WAFMAP_ZONAL_SOFTBIN:
                // Regular wafermap.: SOFT BIN
            case GEX_WAFMAP_HARDBIN:
            case GEX_WAFMAP_STACK_HARDBIN:
            case GEX_WAFMAP_ZONAL_HARDBIN:
                // Regular wafermap.: HARD BIN
                if(getWaferMapData().getWafMap()==NULL)
                    break;
                int iCell = ((pPartInfo->iDieX-getWaferMapData().iLowDieX)+((pPartInfo->iDieY-getWaferMapData().iLowDieY)*getWaferMapData().SizeX));
                if(iCell < 0 || iCell >= getWaferMapData().SizeX*getWaferMapData().SizeY)
                    iCell=0;

                // Check if this is a retest...
                if(getWaferMapData().getWafMap()[iCell].getBin() != GEX_WAFMAP_EMPTY_CELL)
                {
                    // If first retest, we have to save the original bin value...
                    if(getWaferMapData().getWafMap()[iCell].getOrgBin() == GEX_WAFMAP_EMPTY_CELL)
                        getWaferMapData().getWafMap()[iCell].setOrgBin (iBin);
                }
                // In all cases (retest or not), save bin result...but if retest, check the retest-policy !
                //if (ReportOptions.GetOption("wafer","retest_policy").toString()=="highest_bin")
                //(ReportOptions.bWafMapRetest_HighestBin == true)
                if (m_eRetestPolicy == HIGHEST_BIN)
                {
                    // Promote the highest bin value.
                    if(getWaferMapData().getWafMap()[iCell].getBin() != GEX_WAFMAP_EMPTY_CELL)
                        getWaferMapData().getWafMap()[iCell].setBin( gex_max(getWaferMapData().getWafMap()[iCell].getBin(),iBin));
                    else
                        getWaferMapData().getWafMap()[iCell].setBin( iBin);
                }
                else
                {
                    // Standard retest policy: keep the latest Bin result
                    getWaferMapData().getWafMap()[iCell].setBin( iBin);
                }
                getWaferMapData().getCellTestCounter()[iCell]++;
                break;
        }
    }

    // Clear Parametric entry if exists
exit_UpdateWaferMap:
    if((!imapZoningTestValue.empty()) && imapZoningTestValue.contains(uMapIndex))
        imapZoningTestValue.remove(uMapIndex);
    if((!fmapZoningTestValue.empty()) && fmapZoningTestValue.contains(uMapIndex))
        fmapZoningTestValue.remove(uMapIndex);

    return ;
}

/////////////////////////////////////////////////////////////////////////////
// Write Datalog Binning Result (HTML mode only).
/////////////////////////////////////////////////////////////////////////////
void CGexFileInGroup::WriteDatalogBinResult(int iBin,CPartInfo *pPartInfo)
{

    QString strTempFile = GS::Gex::Engine::GetInstance().Get("GalaxySemiFolder").toString()
                          + QDir::separator()+ "temp" + QDir::separator()+ "datalog"
                          + QDir::separator()+ QString("datalog_%1_%2").arg(pPartInfo->bHead).arg(pPartInfo->m_site);
    FILE *poOriAdvancedHandle = hAdvancedReport; //Store the hAdvancedReport original handle
     FILE *poTempHandle = 0;
    if(!QFile::exists(strTempFile)){
        poTempHandle = fopen(strTempFile.toLatin1().constData(),"w+");
    }else {
        poTempHandle = fopen(strTempFile.toLatin1().constData(),"a+");
    }
    if(!poTempHandle){
        GSLOG(SYSLOG_SEV_DEBUG,QString("Generating file %1 with fail").arg(strTempFile).toLatin1().constData());
        return;
    }
    GSLOG(SYSLOG_SEV_DEBUG,QString("Generating file %1 with success").arg(strTempFile).toLatin1().constData());
    hAdvancedReport = poTempHandle; // Use the intermidate file to  generate the log


    // Convert from 'unsigned int' to 'int'
    if(pPartInfo->iDieX >= 32768) pPartInfo->iDieX -= 65536;
    if(pPartInfo->iDieY >= 32768) pPartInfo->iDieY -= 65536;

    if (m_OutputFormat == GEX_OPTION_OUTPUT_CSV)
    {
        // Generating .CSV report file.

        // Current Test curror is in second column: skip remaining table cells
        if(lTestsInDatalog % 2)
            fprintf(hAdvancedReport,"\n");

        if(pReportOptions->getAdvancedReportSettings() != GEX_ADV_DATALOG_RAWDATA)
        {
            // If not RAW datalog!
            fprintf(hAdvancedReport,"BIN %d",iBin);

            if(pPartInfo->bPass)
                fprintf(hAdvancedReport," PASSED");
            else
                fprintf(hAdvancedReport," *FAILED*");

            // Die location
            if(m_eDatalogTableOptions&ADF_DIE_XY)
                fprintf(hAdvancedReport,"  [DieX=%d ; DieY=%d]",pPartInfo->iDieX,pPartInfo->iDieY);

            // PartID
            if(pPartInfo->getPartID().isEmpty() == false)
                fprintf(hAdvancedReport,"  - PartID: %s",pPartInfo->getPartID().toLatin1().constData());

            // Testing time
            if(pPartInfo->lExecutionTime > 0)
                fprintf(hAdvancedReport,"  Test Time: %.3g sec.",(float)pPartInfo->lExecutionTime/1000.0);

            fprintf(hAdvancedReport,"\n\n");
        }
    }
    else if(m_OutputFormat & GEX_OPTION_OUTPUT_HTML_BASED)
    {
        if(lTestsInDatalog % 2)
        {
            // Current Test cusror is in second column: skip remaining table cells
            fprintf(hAdvancedReport,"</tr>\n");
        }
        if(pReportOptions->getAdvancedReportSettings() != GEX_ADV_DATALOG_RAWDATA)
        {
            // If not RAW datalog!
            fprintf(hAdvancedReport,"<tr>\n");
            fprintf(hAdvancedReport,"<td width=\"10%%\"></td>\n");

            if(pPartInfo->bPass)
                fprintf(hAdvancedReport,"<td width=\"15%%\"><b>BIN %d - PASSED</b></td>\n",iBin);
            else
                fprintf(hAdvancedReport,"<td width=\"15%%\"><font color=\"#FF0000\"><b>BIN %d *FAILED*</b></font></td>\n",iBin);

            // Die location
            fprintf(hAdvancedReport,"<td width=\"25%%\" align=\"left\">");
            if(m_eDatalogTableOptions&ADF_DIE_XY)
                fprintf(hAdvancedReport,"<b>DieX=%d, DieY=%d</b>  ",pPartInfo->iDieX,pPartInfo->iDieY);

            // PartID
            if(pPartInfo->getPartID().isEmpty() == false)
                fprintf(hAdvancedReport,"(PartID: %s)",pPartInfo->getPartID().toLatin1().constData());

            // Testing time
            if(pPartInfo->lExecutionTime > 0)
                fprintf(hAdvancedReport,"  Test Time: %.3g sec.",(float)pPartInfo->lExecutionTime/1000.0);


            fprintf(hAdvancedReport,"</td>\n");
            fprintf(hAdvancedReport,"<td width=\"10%%\"></td>\n");
            fprintf(hAdvancedReport,"<td width=\"15%%\"></td>\n");
            fprintf(hAdvancedReport,"<td width=\"25%%\" align=\"right\"></td>\n");
            fprintf(hAdvancedReport,"</tr>\n");
        }

        if(m_OutputFormat==GEX_OPTION_OUTPUT_PPT && pReportOptions->lAdvancedHtmlLinesInPage && pReportOptions->lAdvancedHtmlLinesInPage%5 == 0){
            if(m_eDatalogFormat==ONE_ROW){
                // close table
                fprintf(hAdvancedReport,"</table>\n");
                gexReport->WritePageBreak();
                pReportOptions->lAdvancedHtmlLinesInPage = 0;
                // Reopen table + write header
                gexReport->WriteHtmlOpenTable(98,0);	// HTML code to open table, Width 98%, cell spacing=0
            }else if (m_eDatalogFormat==TWO_ROWS){
                // close table
                fprintf(hAdvancedReport,"</table>\n");
                gexReport->WritePageBreak(NULL,false);
                fprintf(hAdvancedReport,"<html>\n");
                fprintf(hAdvancedReport,"<body>\n");
                fprintf(hAdvancedReport,"<table border=\"0\" width=\"98%%\" cellspacing=\"0\" cellpadding=\"0\">\n");
                pReportOptions->lAdvancedHtmlLinesInPage = 0;

            }
        }else{
            fprintf(hAdvancedReport,"</table>\n<br>\n<br>");
            fprintf(hAdvancedReport,"<table border=\"0\" width=\"98%%\" cellspacing=\"0\" cellpadding=\"0\">\n");
        }
    }


    fclose(poTempHandle);//Close the temp value
    hAdvancedReport = poOriAdvancedHandle; // Restore the handle to it is original value

    //Merge Data between temp file and original file;
    dumpTempDatalog(pPartInfo->m_site, pPartInfo->bHead, hAdvancedReport);//This site/head can be closed put the data into the advanced file.
    // Reset test counter.
    lTestsInDatalog = 0;

}

bool CGexFileInGroup::UpdateCustomParameter(bool bIgnorePart,
                                            QString strParameterName,
                                            int iCustomTestNumber,
                                            double lResult,
                                            CPartInfo *pPartInfo)
{
    // If this part is filtered, we must ignore it!
    if (bIgnorePart == true)
        return true;

    // Find pointer to Parameter
    CTest *ptTestCell=0;
    if(FindTestCell(iCustomTestNumber,GEX_PTEST,&ptTestCell,true,true,
                    strParameterName.toLatin1().data()) !=1)
        return false;	// Failed creating/finding test entry.

    // Update SamplesMin,SampleMax variables!
    ptTestCell->lfSamplesMin = gex_min(lResult,ptTestCell->lfSamplesMin);
    ptTestCell->lfSamplesMax = gex_max(lResult,ptTestCell->lfSamplesMax);

    if(lPass == 1)
    {
        // First call: Initializes Parameter label, and misc. flags.
        if(ptTestCell->bTestType == 'P')
        {
            // Set flag to show it's not a standard parameter, but internal
            ptTestCell->bTestType = '-';

            // Parameter name.
            ptTestCell->strTestName = strParameterName;

            // Intialize constant info such as: limits, units, etc...
            switch(iCustomTestNumber)
            {
                case GEX_TESTNBR_OFFSET_EXT_TEMPERATURE:
                    strcpy(ptTestCell->szTestUnits, "°C");
                    break;
                case GEX_TESTNBR_OFFSET_EXT_TTIME:
                    strcpy(ptTestCell->szTestUnits, "sec");
                    ptTestCell->GetCurrentLimitItem()->lfLowLimit = 0.0;
                    ptTestCell->GetCurrentLimitItem()->bLimitFlag &= (~CTEST_LIMITFLG_NOLTL);	// Clear Bit0 (Means LOW limit exists)
                    break;
            }
        }
        // Updates Min/Max values
        UpdateMinMaxValues(ptTestCell,lResult);
    }
    else if(lPass == 2)
    {
        // Pass 2: collect data & build statistics.
        // Advanced report is Enabled, and test in filter limits.
        switch(pReportOptions->getAdvancedReport())
        {
            case GEX_ADV_DISABLED:
            case GEX_ADV_PAT_TRACEABILITY:
            default:
                // collect sample to allow Interactive Charting (Unless size excees cutoff limit)
                if(pReportOptions->bSpeedCollectSamples)
                {
                    if (AdvCollectTestResult(ptTestCell, lResult,
                                             CTestResult::statusUndefined, pPartInfo->m_site, true) == false)
                    {
                        GSLOG(SYSLOG_SEV_ERROR, QString("Failed to update custom parameters").toLatin1().constData());
                        return false;
                    }
                }
                break;

            case GEX_ADV_DATALOG:
                if (GexAbstractDatalog::existInstance())
                {
                    if (GexAbstractDatalog::isDataloggable(ptTestCell, false, false))
                        GexAbstractDatalog::pushParametricResult(this, ptTestCell, pPartInfo, lResult, false, false, pPartInfo->m_site);
                }
                else
                    AdvCollectDatalog(ptTestCell, lResult,false,false,false, pPartInfo->m_site, (int)pPartInfo->bHead, (int)pPartInfo->m_site);

                // collect sample to allow Interactive Charting (Unless size excees cutoff limit)
                if(pReportOptions->bSpeedCollectSamples)
                    AdvCollectTestResult(ptTestCell, lResult, CTestResult::statusUndefined, pPartInfo->m_site, true);
                break;
            case GEX_ADV_HISTOGRAM:
            case GEX_ADV_TREND:
            case GEX_ADV_CANDLE_MEANRANGE:
            case GEX_ADV_GO_DIAGNOSTICS:	// Optimizer Diagnostic takes ALL Trend results!
                AdvCollectTestResult(ptTestCell, lResult, CTestResult::statusUndefined, pPartInfo->m_site, pReportOptions->bSpeedCollectSamples);
                break;
            case GEX_ADV_CORRELATION:
                AdvCollectTestResult(ptTestCell, lResult, CTestResult::statusUndefined, pPartInfo->m_site, pReportOptions->bSpeedCollectSamples);
                break;
        }

#if 0 // Herve Thomy :
        // OBSOLETE MEMBER Not used anymore,since we hold the runIndex in the CPartBinning class except with MASA plugins
        // Keeps tracking the test# result.
        ptTestCell->ldCurrentSample++;
#endif

        // Build Cpk
        ptTestCell->lfSamplesTotal += lResult;					// Sum of X
        ptTestCell->lfSamplesTotalSquare += (lResult*lResult);	// Sum of X*X

        ptTestCell->mHistogramData.AddValue(lResult);
        if((lResult >= ptTestCell->GetCurrentLimitItem()->lfHistogramMin) && (lResult <= ptTestCell->GetCurrentLimitItem()->lfHistogramMax))
        {
            // Incluse result if it in into the viewing window.
            int iCell;
            if (ptTestCell->GetCurrentLimitItem()->lfHistogramMax == ptTestCell->GetCurrentLimitItem()->lfHistogramMin) {
                iCell = 0;
            } else {
                iCell = (int)( (TEST_HISTOSIZE*(lResult - ptTestCell->GetCurrentLimitItem()->lfHistogramMin)) / (ptTestCell->GetCurrentLimitItem()->lfHistogramMax-ptTestCell->GetCurrentLimitItem()->lfHistogramMin));
            }
            // Incluse result if it in into the viewing window.
            if( (iCell >= 0) && (iCell < TEST_HISTOSIZE) )
                ptTestCell->lHistogram[iCell]++;
            else if (iCell >= TEST_HISTOSIZE)
                ptTestCell->lHistogram[TEST_HISTOSIZE-1]++;   /// TO REVIEW : is it normal that iCell can exceed than TEST_HISTOSIZE
            else
            {
                GSLOG(SYSLOG_SEV_ERROR,
                         QString("Invalid value detected to build histogram for test %1 [%2]")
                         .arg(ptTestCell->lTestNumber)
                        .arg( ptTestCell->strTestName).toLatin1().constData());

                GSLOG(SYSLOG_SEV_DEBUG, QString("\tCell Found: %1").arg(iCell).toLatin1().constData());
                GSLOG(SYSLOG_SEV_DEBUG, QString("\tUCP result: %1").arg( lResult).toLatin1().constData());
                GSLOG(SYSLOG_SEV_DEBUG, QString("\tHistogram Min: %1").arg(
                         ptTestCell->GetCurrentLimitItem()->lfHistogramMin).toLatin1().constData());
                GSLOG(SYSLOG_SEV_DEBUG, QString("\tHistogram Max: %1").arg(
                         ptTestCell->GetCurrentLimitItem()->lfHistogramMax).toLatin1().constData());
            }
        }
    }
    return true;
}

/////////////////////////////////////////////////////////////////////////////
// Read STDF PIR record
/////////////////////////////////////////////////////////////////////////////
void CGexFileInGroup::ReadPIR(void)
{
    CTest		*ptTestCell;
    CPartInfo	PartInfo;		// Structure to hold part info: DieXY, Exec time, Tests executed, partID, etc...
    unsigned	uMapIndex;

    BYTE  bSiteNumber = 0;

    // PIR
    switch(StdfRecordHeader.iStdfVersion)
    {
    default :
        break;

    case GEX_STDFV4:
            StdfFile.ReadByte(&PartInfo.bHead) ;
            StdfFile.ReadByte(&bSiteNumber)  ;	// site number
            PartInfo.m_site = bSiteNumber;

            // Clear Run failing type
            m_bFailingSiteDetails[PartInfo.m_site] = 0;

            if(StdfFile.GetHeadAndSiteNumberDecipheringMode() != GS::StdLib::no_deciphering)
            {
                PartInfo.m_site = GQTL_STDF::HeadAndSiteNumberDecipher::DecipherSiteNumberForAdvantest(PartInfo.bHead, PartInfo.m_site);
                PartInfo.bHead = GQTL_STDF::HeadAndSiteNumberDecipher::DecipherHeadNumberForAdvantest(PartInfo.bHead);
            }

            uMapIndex = (PartInfo.bHead << 16) | PartInfo.m_site;	// Compute index for Head&Site...as in multi-head/sites, we have to save multiple results before the PRR


            //        if(lMappingSiteHead.find(uMapIndex) != lMappingSiteHead.end() && lMappingSiteHead[uMapIndex] == 1){
            //            // last PRR missing PIR/PRR block not closed
            //            //show a warning
            //        }
            //        if(lPass == 1){
            //            m_PIRListing.append(sPirPrrMapping());
            //            m_PIRListing.last().bHead = PartInfo.bHead;
            //            m_PIRListing.last().bSite = PartInfo.bSite;
            //            m_PIRListing.last().iIdx = m_iPIRCount;
            //        }
            //
            lMappingSiteHead[uMapIndex]= 1;	// Tells we enter in a PIR/PRR block
            if((iProcessSite >=0) && (iProcessSite != (int)PartInfo.m_site))
                break;

            // If filter is: First/Last testing instance (Wafer sort only)
            // then keep track of Part# for this given site...
            switch(iProcessBins)
            {
                case GEX_PROCESSPART_FIRSTINSTANCE:
                case GEX_PROCESSPART_LASTINSTANCE:

                    // First time call: reset buffer
                    if(PartProcessed.partNumber(PartInfo.m_site) == 1)
                    {
                        m_First_InstanceDies.clear();
                        m_Last_InstanceDies.clear();
                        m_First_Instance.clear();
                        m_Last_Instance.clear();
                    }
            }

            // Initializes the mapping offset to know which site is mapped to which Part#...
            PartProcessed.addSite(PartInfo.m_site);

            // If we reach this point, this means we process this type of part (site# & head#)
            // then do pre-processing cleanup...unless we already did it (in previous PIR if we have multiple PIR nested)
            if(PartProcessed.PIRNestedLevel() == 1)
            {
                ptTestCell = ptTestList;
                // Reset 'tested' flag for each test in the test list
                while(ptTestCell != NULL)
                {
                    ptTestCell->bTestExecuted = false;
                    ptTestCell->ldTmpSamplesExecs=0;
                    ptTestCell = ptTestCell->GetNextTest();
                };
            }

            break;
    }
}


/////////////////////////////////////////////////////////////////////////////
// Read STDF SBR record
/////////////////////////////////////////////////////////////////////////////
void CGexFileInGroup::ReadSBR()
{
    ReadBinningRecord(&m_ParentGroup->cMergedData.ptMergedSoftBinList,&iSoftBinRecords,&bMergeSoftBin);
}

/////////////////////////////////////////////////////////////////////////////
// Read STDF HBR record
/////////////////////////////////////////////////////////////////////////////
void CGexFileInGroup::ReadHBR()
{
    ReadBinningRecord(&m_ParentGroup->cMergedData.ptMergedHardBinList,&iHardBinRecords,&bMergeHardBin);
}

/////////////////////////////////////////////////////////////////////////////
// Read STDF SBR/ HBR record
/////////////////////////////////////////////////////////////////////////////
void CGexFileInGroup::ReadBinningRecord(CBinning **ptBinList,int *piRecords,bool *pbMerge)
{
    char	szString[257];	// A STDF string is 256 bytes long max!
    BYTE bMergedSites;
    BYTE bSites;
    bool	bBreak=false;
    bool	bCreateEntryOnly=false;

    switch(StdfRecordHeader.iStdfVersion)
    {
        default :
            break;

        case GEX_STDFV4:
            StdfFile.ReadByte(&bMergedSites);	// head number: 255=Merged sites
            StdfFile.ReadByte(&bSites);			// site number

            // HBR or SBR is ok here, need just head and site numbers

            unsigned short siteNumber = bSites;
            if(StdfFile.GetHeadAndSiteNumberDecipheringMode() != GS::StdLib::no_deciphering)
            {
                siteNumber = GQTL_STDF::HeadAndSiteNumberDecipher::DecipherSiteNumber( StdfFile, bMergedSites, bSites);
                bMergedSites = GQTL_STDF::HeadAndSiteNumberDecipher::DecipherHeadNumber(StdfFile, bMergedSites);
            }
            // If filter is specific site, only read relevant SBR/HBR
            if((iProcessSite >=0) && ((lPass ==2) || (siteNumber != iProcessSite) || (bMergedSites == 255)))
                bCreateEntryOnly = true;	// Site filter not matching.

            // If looking for merged binning summary
            if(iProcessSite < 0)
            {
                if((lPass == 1) && (((int)bMergedSites) != 255))
                    bBreak = true;	// In pass1, only process HBR/SBR giving merged results
                else
                    if(lPass == 2)
                    {
                        if((*piRecords) && (*pbMerge == false))
                            break;	// In pass2, only take records if we have to merge ourselves...
                        else
                            *pbMerge = true;	// Trigger to merge ourselves!
                    }
            }

            StdfFile.ReadWord(&wData);	// SBIN/HBIN #
            StdfFile.ReadDword(&lData);	// SBIN/HBIN count
            if((unsigned)lData & 0xC0000000)
                break;	// Too high number to be valid...also fixes LTX-Fusion bug!
            StdfFile.ReadByte(&bData);	// Pass/Fail info
            StdfFile.ReadString(szString);	// BIN name label

            // Check if Have to Collect Binning Trend data
            if(	(m_eBinComputation==SUMMARY)
                && (wData != 65535 || (m_eStdfCompliancy==CGexFileInGroup::FLEXIBLE)))
                AdvCollectBinningTrend(ptBinList,wData,lData);

            // Check if should exit now
            if(bBreak)
                break;

            // If only to use PRR to compute Binning., then only create empty Bin celle (so we save its name)
            if(	(m_eBinComputation==SAMPLES)    ||
                (m_eBinComputation==WAFER_MAP)  ||
                bCreateEntryOnly
                )
                lData = -1;

            if (wData != 65535 || (m_eStdfCompliancy==CGexFileInGroup::FLEXIBLE)){
                int iSoftBinValue = wData ,iHardBinValue = wData;
                //bModeUsed to specify if the fct insert a soft bin or a hard bin
                bool bMode = false;

                if(StdfRecordHeader.iRecordSubType == 40)//HBIN
                {
                    iSoftBinValue = m_oSBinHBinMap.key(wData, wData);
                    QList<int> oFoundSoftBin = m_oSBinHBinMap.keys(wData);
                    if(oFoundSoftBin.count() == 1)
                        iSoftBinValue  = oFoundSoftBin.first();
                    else {
                        foreach (int iCSoft, oFoundSoftBin) {
                            if(iProcessBins == GEX_PROCESSPART_SBINLIST &&
                               pGexRangeList->Contains(iCSoft))
                            {
                                iSoftBinValue = iCSoft;
                                break;
                            }
                            if(iProcessBins == GEX_PROCESSPART_EXSBINLIST &&
                               !pGexRangeList->Contains(iCSoft))
                            {
                                iSoftBinValue = iCSoft;
                                break;
                            }
                        }
                    }
                    bMode = true;
                }else{//SBIN
                    iHardBinValue = m_oSBinHBinMap.value(wData, wData);
                    bMode = false;
                }

                AddBinCell(ptBinList,-1,32768,32768,iSoftBinValue,iHardBinValue,bData,lData,true,true,szString, bMode);	// Add Bin inf to list
            }

            // If only to use PRR to compute Binning, exit now.
            if(	(m_eBinComputation==SAMPLES)        ||
                (m_eBinComputation==WAFER_MAP)      ||
                bCreateEntryOnly
                )
                return;

            // Keeps track of total number of records SBR and HBR processed.
            (*piRecords)++;
            break;
    }

    // Flags we have a binning summary, then no need in pass2 to create it from sampled data
    bBinningSummary = true;
}

/////////////////////////////////////////////////////////////////////////////
// Builds a string that include the Test# and PinmapIndex# if any (Multi-result param)
/////////////////////////////////////////////////////////////////////////////
void CGexFileInGroup::BuildTestNumberString(CTest *ptTestCell)
{
    switch(ptTestCell->lPinmapIndex)
    {
        case GEX_PTEST:		// Standard Parametric test: no PinmapIndex#
        case GEX_MPTEST:	// Multi-result parametric test,...but no PinmapIndex defined!
        case GEX_INVALIDTEST:
        case GEX_UNKNOWNTEST:
        case GEX_FTEST:		// Digital test
            sprintf(ptTestCell->szTestLabel,"%u",ptTestCell->lTestNumber);
            break;
        default:			// Multi-result parametric test, PinmapIndex# defined!
            sprintf(ptTestCell->szTestLabel,"%u.%d",ptTestCell->lTestNumber,ptTestCell->lPinmapIndex);
            break;
    }
}



///////////////////////////////////////////////////////////
// Keep track of PartID over samples.
///////////////////////////////////////////////////////////
void	CGexFileInGroup::SaveLotSamplesOffsets(void)
{
    static long	lFileStartSampleID=0;	// Used to keep track of offsetID for each file in same group

    // Keep track of sampleID offset when mergin multiple files in a group.
    if(lFileID==0)
        lFileStartSampleID = 1;
    lFileStartSampleID = gex_max(lFileStartSampleID,getMirDatas().lFirstPartResult);
    getMirDatas().lFirstPartResult = lFileStartSampleID;

    // Keep track of number of samples for each test in this given sub-lot
    CTest *ptTestCell = ptTestList;

    while(ptTestCell != NULL)
    {
        // Update list of Samples offset "ending point" (number of samples) for each sublot: used to draw Sublots markers in trend charts.
        ptTestCell->pSamplesInSublots.append(m_lSamplesInSublot);

        // Move to next test.
        ptTestCell = ptTestCell->GetNextTest();
    };

    m_lSamplesInSublot = 0;
}

bool CGexFileInGroup::UpdateOptions(CReportOptions *ro)
{
    if (!ro)
    {
        GSLOG(SYSLOG_SEV_WARNING, " error : cant update CGexFileInGroup options because ReportOptions is NULL !");
        return false;
    }
    bool ok=true; // if you detect a problem, set ok to false

    // Cache some options to avoid slowdown
    QString of=ro->GetOption("output", "format").toString();
    updateOutputFormat(of);

    //
    QString bc=ro->GetOption("binning","computation").toString();
    if (bc=="wafer_map") m_eBinComputation=CGexFileInGroup::WAFER_MAP;
    else if (bc=="summary") m_eBinComputation=CGexFileInGroup::SUMMARY;
    else if (bc=="samples") m_eBinComputation=CGexFileInGroup::SAMPLES;
    //
    QString sc=ro->GetOption("statistics","computation").toString();
    if (sc=="samples_only") m_eStatsComputation=CGexFileInGroup::FROM_SAMPLES_ONLY;
    else if (sc=="summary_only") m_eStatsComputation=CGexFileInGroup::FROM_SUMMARY_ONLY;
    else if (sc=="samples_then_summary") m_eStatsComputation=CGexFileInGroup::FROM_SAMPLES_THEN_SUMMARY;
    else if (sc=="summary_then_samples") m_eStatsComputation=CGexFileInGroup::FROM_SUMMARY_THEN_SAMPLES;
    //
    QString df=ro->GetOption("adv_datalog", "format").toString();
    if (df=="1row") m_eDatalogFormat=CGexFileInGroup::ONE_ROW;
    else if (df=="2rows") m_eDatalogFormat=CGexFileInGroup::TWO_ROWS;
    //
    QString rp=ro->GetOption("wafer","retest_policy").toString();
    if (rp=="last_bin") m_eRetestPolicy=CGexFileInGroup::LAST_BIN;
    else if (rp=="highest_bin") m_eRetestPolicy=CGexFileInGroup::HIGHEST_BIN;
    //
    QString dpsc=ro->GetOption("dataprocessing", "stdf_compliancy").toString();
    if (dpsc=="stringent") m_eStdfCompliancy=CGexFileInGroup::STRINGENT;
    else if (dpsc=="flexible") m_eStdfCompliancy=CGexFileInGroup::FLEXIBLE;

    QString orm=ro->GetOption("dataprocessing", "data_cleaning_mode").toString();
    if (orm=="none") m_eOutlierRemoveMode=CGexFileInGroup::NONE;
    else if (orm=="n_sigma") m_eOutlierRemoveMode=CGexFileInGroup::N_SIGMA;
    else if (orm=="exclude_n_sigma") m_eOutlierRemoveMode=CGexFileInGroup::EXCLUDE_N_SIGMA;
    else if (orm=="n_pourcent") m_eOutlierRemoveMode=CGexFileInGroup::N_POURCENT;
    else if (orm=="n_iqr")	m_eOutlierRemoveMode=CGexFileInGroup::N_IQR;
    //
     bool orv_ok=false;
    m_dOutlierRemovalValue=ro->GetOption("dataprocessing","data_cleaning_value").toDouble(&orv_ok);
    if (!orv_ok)
    {
        ok=false;
        GSLOG(SYSLOG_SEV_WARNING, " error : cant get Option( dataprocessing,data_cleaning_value ) !");
        WriteDebugMessageFile("CGexFileInGroup::UpdateOptions(): error : cant get Option( dataprocessing,data_cleaning_value ) !");
    }

    QString mpr_mm=ro->GetOption("dataprocessing","multi_parametric_merge_mode").toString();
    if (mpr_mm=="merge") m_eMPRMergeMode=MERGE;
    else if (mpr_mm=="no_merge") m_eMPRMergeMode=NO_MERGE;
    else
    {
        ok=false;
        GSLOG(SYSLOG_SEV_NOTICE, QString(" warning : unknown MPR merge mode : '%1' !").arg(mpr_mm).toLatin1().data() );
        WriteDebugMessageFile("CGexFileInGroup::UpdateOptions : warning : unknown MPR merge mode !");
    }

    QString strTestMergeRule = ro->GetOption("dataprocessing","duplicate_test").toString();
    if (strTestMergeRule == "merge")
        mTestMergeRule = TEST_MERGE_NUMBER;
    else if (strTestMergeRule == "merge_name")
        mTestMergeRule = TEST_MERGE_NAME;
    else if (strTestMergeRule == "no_merge")
        mTestMergeRule = TEST_NO_MERGE;
    else
    {
        ok=false;
        GSLOG(SYSLOG_SEV_NOTICE, QString(" warning : unknown duplicate test merge rule : '%1' !").arg( strTestMergeRule).toLatin1().constData());
        WriteDebugMessageFile("CGexFileInGroup::UpdateOptions : warning : unknown duplicate test merge rule !");
    }

    QString strIgnoreFtrRule = ro->GetOption("dataprocessing", "functional_tests").toString();
    //GSLOG(SYSLOG_SEV_INFORMATIONAL, QString(" IgnoreFtrRule = %1").arg( strIgnoreFtrRule).toLatin1().constData() );
    if (strIgnoreFtrRule == "disabled")
        m_eIgnoreFunctionalTests = FTR_DISABLED;
    else if (strIgnoreFtrRule == "enabled")
        m_eIgnoreFunctionalTests = FTR_ENABLED;
    else
    {
        ok=false;
        GSLOG(SYSLOG_SEV_NOTICE, QString(" warning : unknown ignore functional tests rule : '%1' !").arg( strIgnoreFtrRule).toLatin1().constData());
        WriteDebugMessageFile("CGexFileInGroup::UpdateOptions : warning : unknown ignore functional tests rule !");
    }

    QString strFailCountMode = ro->GetOption("dataprocessing", "fail_count").toString();
    if (strFailCountMode == "all")
        m_eFailCountMode = FAILCOUNT_ALL;
    else if (strFailCountMode == "first")
        m_eFailCountMode = FAILCOUNT_FIRST;
    else
    {
        ok=false;
        GSLOG(SYSLOG_SEV_NOTICE, QString(" warning : unknown fail count rule : '%1' !").arg( strFailCountMode).toLatin1().constData());
        WriteDebugMessageFile("CGexFileInGroup::UpdateOptions : warning : unknown fail count rule !");
    }

    QString strAdvDatalogFieldOptions = (ro->GetOption(QString("adv_datalog"), QString("field"))).toString();
    if(strAdvDatalogFieldOptions.isEmpty())
        m_eDatalogTableOptions = ADF_NO_FIELD;
    else
    {
        m_eDatalogTableOptions = 0;
        QStringList qslAdvDatalogFieldOptions = strAdvDatalogFieldOptions.split(QString("|"));

        if(qslAdvDatalogFieldOptions.contains(QString("comment")))
            m_eDatalogTableOptions = m_eDatalogTableOptions|ADF_COMMENTS;
        if(qslAdvDatalogFieldOptions.contains(QString("test_number")))
            m_eDatalogTableOptions = m_eDatalogTableOptions|ADF_TEST_NUMBER;
        if(qslAdvDatalogFieldOptions.contains(QString("test_name")))
            m_eDatalogTableOptions = m_eDatalogTableOptions|ADF_TEST_NAME;
        if(qslAdvDatalogFieldOptions.contains(QString("limits")))
            m_eDatalogTableOptions = m_eDatalogTableOptions|ADF_LIMITS;
        if(qslAdvDatalogFieldOptions.contains(QString("die_loc")))
            m_eDatalogTableOptions = m_eDatalogTableOptions|ADF_DIE_XY;
    }

    QString s=ro->GetOption("dataprocessing","scaling").toString();
    if (s=="none") m_eScalingMode=SCALING_NONE;
    else if (s=="smart") m_eScalingMode=SCALING_SMART;
    else if (s=="to_limits") m_eScalingMode=SCALING_TO_LIMITS;
    else if (s=="normalized") m_eScalingMode=SCALING_NORMALIZED;
    else
    {
        GSLOG(SYSLOG_SEV_WARNING, QString(" error : unknown option 'scaling' '%1' ").arg( s).toLatin1().constData());
        ok=false;
    }

    QStringList lstVisualOptions = ro->GetOption("wafer", "visual_options").toString().split("|");

    if (lstVisualOptions.contains("all_parts"))
        m_bFullWafermap = true;
    else
        m_bFullWafermap = false;

    // Load Pass/Fail flag option
    QString strPassFailRuleOption = ro->GetOption("dataprocessing", "param_passfail_rule").toString();

    if (strPassFailRuleOption == "passfail_flag")
        m_bUsePassFailFlag = true;
    else if (strPassFailRuleOption == "limits_only")
        m_bUsePassFailFlag = false;
    else
    {
        GSLOG(SYSLOG_SEV_WARNING, QString(" error : unknown option 'param_passfail_rule' '%1' ").arg( strPassFailRuleOption).toLatin1().constData());
        ok=false;
    }

    // Load Pass/Fail flag option
    QString lPartIdentification = ro->GetOption("dataprocessing", "part_identification").toString();

    if (lPartIdentification == "xy")
        mPartIdentifiation = XY;
    else if (lPartIdentification == "part_id")
        mPartIdentifiation = PARTID;
    else if (lPartIdentification == "auto")
        mPartIdentifiation = AUTO;
    else
    {
        GSLOG(SYSLOG_SEV_WARNING,
                 QString(" error : unknown option 'part_identification' '%1' ")
                 .arg(lPartIdentification).toLatin1().constData());
        ok=false;
    }

    bool lInteger = false;
    mHistoBarsCount = ReportOptions.GetOption("adv_histogram", "total_bars").toInt(&lInteger);

    if (lInteger == false)
    {
        GSLOG(SYSLOG_SEV_WARNING,
               " error : Option adv_histogram/total_bars is not a integer value");
        ok = false;
    }

    // Set limit used in analysing files
    QString limit = ro->GetOption("dataprocessing", "used_limits").toString();
    if (limit=="spec_limits_if_any")
        mUsedLimits=CGexFileInGroup::SPEC_LIMIT_IF_ANY;
    else if (limit=="standard_limits_only")
        mUsedLimits=CGexFileInGroup::STANDART_LIMIT_ONLY;
    else if (limit=="spec_limits_only")
        mUsedLimits=CGexFileInGroup::SPEC_LIMIT_ONLY;
    else
    {
        mUsedLimits=CGexFileInGroup::STANDART_LIMIT_ONLY;

        GSLOG(SYSLOG_SEV_WARNING,
              QString(" error : unknown option used_limits' '%1' ")
              .arg(limit).toLatin1().constData());
        ok=false;
    }

    return ok;
}

void CGexFileInGroup::updateOutputFormat(const QString& aOutputFormat)
{
    if (aOutputFormat=="HTML") m_OutputFormat=GEX_OPTION_OUTPUT_HTML;
    else if (aOutputFormat=="CSV")
        m_OutputFormat=GEX_OPTION_OUTPUT_CSV;
    else if (aOutputFormat=="PPT")
        m_OutputFormat=GEX_OPTION_OUTPUT_PPT;
    else if (aOutputFormat=="PDF")
        m_OutputFormat=GEX_OPTION_OUTPUT_PDF;
    else if (aOutputFormat=="DOC")
        m_OutputFormat=GEX_OPTION_OUTPUT_WORD;
    else if (aOutputFormat=="INTERACTIVE")
        m_OutputFormat=GEX_OPTION_OUTPUT_INTERACTIVEONLY;
    else if(aOutputFormat=="ODT")
        m_OutputFormat=GEX_OPTION_OUTPUT_ODT;
}

void CGexFileInGroup::dumpTempDatalog(int iSite, int iHead, FILE *poHandle){

    if(!pReportOptions || !poHandle)
        return ;

    QString strTempFile = GS::Gex::Engine::GetInstance().Get("GalaxySemiFolder").toString()
                          + QDir::separator()+ "temp"  + QDir::separator()+ "datalog"
                          + QDir::separator()+ QString("datalog_%1_%2").arg(iHead).arg(iSite);
    if(!QFile::exists(strTempFile)){
        GSLOG(SYSLOG_SEV_DEBUG,QString("Reading file %1 with fail").arg(strTempFile).toLatin1().constData());
        return ;
    }
    GSLOG(SYSLOG_SEV_DEBUG,QString("Reading file %1 with SUCCESS").arg(strTempFile).toLatin1().constData());
    QFile oFile(strTempFile);
    if(!oFile.open(QIODevice::ReadOnly))
        return ;
    QTextStream oStream;
    oStream.setDevice(&oFile);
    while(!oStream.atEnd()){
        QString strLine = oStream.readLine();
        fprintf(poHandle,"%s\n", strLine.toLatin1().constData());
    }

    oFile.close();
    QFile::remove(strTempFile);
    GSLOG(SYSLOG_SEV_DEBUG,QString("Dumping file %1 with SUCCESS").arg(strTempFile).toLatin1().constData());
}

TestSiteLimits::TestSiteLimits ()
{
    m_dLowLimit = -C_INFINITE;
    m_dHighLimit= C_INFINITE;
    m_iCurrentIdx = m_iLimitIndex++;
    m_lStartT = 0;
}

TestSiteLimits::~TestSiteLimits(){
}

TestSiteLimits::TestSiteLimits(const TestSiteLimits &roOther){
    m_dLowLimit = roOther.m_dLowLimit;
    m_dHighLimit = roOther.m_dHighLimit;
    m_iCurrentIdx = roOther.m_iCurrentIdx;
    m_lStartT = roOther.m_lStartT;
}

void TestSiteLimits::reset(){
    m_iLimitIndex = 0;
}

double TestSiteLimits::getLowLimit()
{
    return m_dLowLimit;
}

void TestSiteLimits::setLowLimit(double val)
{
    m_dLowLimit = val;
}

double TestSiteLimits::getHighLimit()
{
    return m_dHighLimit ;
}

void TestSiteLimits::setHighLimit(double val)
{
    m_dHighLimit = val;
}

int TestSiteLimits::getCurrentIdx()
{
    return m_iCurrentIdx;
}

void TestSiteLimits::setCurrentIdx(int val)
{
    m_iCurrentIdx = val;
}

time_t TestSiteLimits::getStartT()
{
    return m_lStartT;
}

void TestSiteLimits::setStartT(time_t t)
{
    m_lStartT = t;
}

int TestSiteLimits::getLimitIndex()
{
    return m_iLimitIndex;

}

void TestSiteLimits::setLimitIndex(int val)
{
    m_iLimitIndex = val;
}



