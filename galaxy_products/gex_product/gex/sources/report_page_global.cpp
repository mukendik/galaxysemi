/////////////////////////////////////////////////////////////////////////////
// Creates HTML 'Global' page.
/////////////////////////////////////////////////////////////////////////////

#include <time.h>

#if defined unix || __MACH__
#include <stdlib.h>
#include <unistd.h>
#endif

#include "browser_dialog.h"
#include "report_build.h"
#include "report_options.h"
#include "gex_oem_constants.h"	// Examinator OEM release specifics.
#include "gex_shared.h"
#include "gex_file_in_group.h"
#include "gex_group_of_files.h"
#include "gex_report.h"
#include <gqtl_log.h>
#include "product_info.h"
#include "engine.h"
#include "csl/csl_engine.h"
#include "message.h"
#include "plugin_base.h"


// Galaxy QT libraries
#include "gqtl_sysutils.h"

#define GEX_PAGEGLOBAL_MAX_LOTS_DISPLAYED	500

// main.cpp
extern GexMainwindow *	pGexMainWindow;

/////////////////////////////////////////////////////////////////////////////
// Prepares the report section to be written (.CSV & .HTML)
/////////////////////////////////////////////////////////////////////////////
int	CGexReport::PrepareSection_Global(BOOL /*bValidSection*/)
{
    char	szString[2048];

    QString strOutputFormat=m_pReportOptions->GetOption("output", "format").toString();
    // Creates the 'Global' page & header
    if (strOutputFormat=="CSV")
    {
        // Generating .CSV report file.
        fprintf(hReportFile,"\n---- Global Info ----\n\n");
    }
    else
    if (m_pReportOptions->isReportOutputHtmlBased())
            //strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
    {
        // Generating HTML report file (but not MyReport from Template)
        if(((strOutputFormat=="HTML"))
           && m_pReportOptions->strTemplateFile.isEmpty())
        {
            // Open <stdf-filename>/report/global.htm
            sprintf(szString,"%s/pages/global.htm",m_pReportOptions->strReportDirectory.toLatin1().constData());
            hReportFile = fopen(szString,"wt");
            if(hReportFile == NULL)
                return GS::StdLib::Stdf::ReportFile;
        }

        // HTML code.
        WriteHeaderHTML(hReportFile,"#000000");	// Default: Text is Black

        // Title + bookmark (unless MyReport which provides its own custom title)
        if(m_pReportOptions->strTemplateFile.isEmpty())
            WriteHtmlSectionTitle(hReportFile,"all_globalinfo","Global Information");

        fprintf(hReportFile,"</font></p>\n");
        fprintf(hReportFile,"<table border=\"0\" width=\"750\" cellspacing=\"1\">\n");
    }
    return GS::StdLib::Stdf::NoError;
}

/////////////////////////////////////////////////////////////////////////////
// closes the report section just written (.CSV & .HTML)
/////////////////////////////////////////////////////////////////////////////
int	CGexReport::CloseSection_Global(void)
{
    QString of=m_pReportOptions->GetOption("output", "format").toString();
    if (of=="CSV")
    {
        // Generating .CSV report file: no specific task to perform when section written !
    }
    else if(m_pReportOptions->isReportOutputHtmlBased())	//(of=="HTML"||of=="DOC"||of=="PDF"||of=="PPT"||of=="INTERACTIVE")
    {
        // close page.
        if(of=="HTML")
        {
            // Close index page (unless creating one flat HTML file with all sections (eg: when creating a Word document)
            fprintf(hReportFile,C_HTML_FOOTER,
                    // Writes HTML footer Application name/web site string
                    GS::Gex::Engine::GetInstance().Get("AppFullName").toString().toLatin1().data() );
            fprintf(hReportFile,"</body>\n");
            fprintf(hReportFile,"</html>\n");

            // Close report file...unlerss flat HTML with multiple sections (eg: MyReport)
            if(m_pReportOptions->strTemplateFile.isEmpty())
                CloseReportFile();

            // Write page break (ignored if not writing a flat HTML document)
            WritePageBreak();
        }
    }
    return GS::StdLib::Stdf::NoError;
}

/////////////////////////////////////////////////////////////////////////////
// Convert a number into HH:mm:sec string
/////////////////////////////////////////////////////////////////////////////
char*	CGexReport::HMSString(int lElapsedTime,int lMillisec,char *szString)
{
    int	lHour,lMin,lSec;
    char	*szHours,*szMin,*szSec;
    char	szMillisec[10];
    // Check if valide time
    if(lElapsedTime < 0)
    {
        strcpy(szString,GEX_NA);
        return szString;
    }

    if(lMillisec > 0)
        sprintf(szMillisec,".%02d",lMillisec/10);
    else
        *szMillisec = 0;

    // Convert number of seconds into Hours, minutes, seconds
    lHour = lElapsedTime / 3600;
    lMin = (lElapsedTime -(3600*lHour)) / 60;
    lSec = (lElapsedTime -(3600*lHour)-(60*lMin));

    // Check if string must include an 's' or not...just to be a purist !
    if(lHour > 1)
        szHours = GEX_T("hours");
    else
        szHours = GEX_T("hour");
    if(lMin > 1)
        szMin = GEX_T("minutes");
    else
        szMin = GEX_T("minute");
    if(lSec > 1)
        szSec = GEX_T("seconds");
    else
        szSec = GEX_T("second");

    if(lHour)
      sprintf(szString,"%d %s %02d %s %02d%s %s",lHour,szHours,lMin,szMin,lSec,szMillisec,szSec);
    else
    {
        // No hours !
        if(lMin)  // but...Minutes
          sprintf(szString,"%d %s %02d%s %s",lMin,szMin,lSec,szMillisec,szSec);
        else	  // No minutes neither
          sprintf(szString,"%d%s %s",lSec,szMillisec,szSec);
    }
    return szString;
}

/////////////////////////////////////////////////////////////////////////////
// Creates the .CSV or HTML file, with all STDF information.
/////////////////////////////////////////////////////////////////////////////
void	CGexReport::WriteMirProcessingTime(void)
{
    char	szString[256];

    int iElapsedProcessTime = m_tProcessTime.elapsed();	// compute elapsed time to process file

    // convert processing time (in seconds) to string.
    HMSString(iElapsedProcessTime/1000,iElapsedProcessTime % 1000,szString);

    // In order to show a funny comment, we need to pick a random #
    srand( (unsigned)time( NULL ) );
    int i= rand() % 18;

    switch(i)
    {
    case 0: strcat(szString," - Better show microseconds in the next release!");
        break;
    case 1: strcat(szString," - Just shattered the world record!");
        break;
    case 2: strcat(szString," - That's prodigious speed...can only come from Quantix!");
        break;
    case 3: strcat(szString," - Because Quantix excels in optimization!");
        break;
    case 4: strcat(szString," - What speed, no comment needed!");
        break;
    case 5: strcat(szString," - Looking for faster processing? Check Quantix's next  release.");
        break;
    case 6: strcat(szString," - Now, that's lightning speed!");
        break;
    case 7: strcat(szString," - Speeeeeeedy!");
        break;
    case 8: strcat(szString," - No time for a coffee break!");
        break;
    case 9: strcat(szString," - Fasten your seatbelt!");
        break;
    case 10: strcat(szString," - That was fast!");
        break;
    case 11: strcat(szString," - Too fast for words!");
        break;
    case 12: strcat(szString," - Now that's fast!");
        break;
    case 13: strcat(szString," - Please place your tray in its upright and locked position!");
        break;
    case 14: strcat(szString," - Whoa, did you see how fast that was?");
        break;
    case 15: strcat(szString," - Screaming fast!");
        break;
    case 16: strcat(szString," - Smokin'");
        break;
    case 17: strcat(szString," - Holy smokes!");
        break;
    }

    WriteInfoLine("Processing time",szString);
}

/////////////////////////////////////////////////////////////////////////////
// Checks if GEX accepts the STDF file created by the given tester platform
/////////////////////////////////////////////////////////////////////////////
int	CGexReport::CheckForTesterPlatform(char *szExecType,char *szExecVer,char *szTesterType,int iStdfAtrTesterBrand)
{
    int		iStatus;
    int		iMajorRelease,iMinorRelease;
    QString strExecType(szExecType);
    QString strExecVersion(szExecVer);
    QString strTesterType(szTesterType);

    // Check if this file is marked to be rejected.
    if(iStdfAtrTesterBrand == -2)
        return GS::StdLib::Stdf::PlatformRestriction;

    switch(GS::LPPlugin::ProductInfo::getInstance()->getProductID())
    {
        default:
            return GS::StdLib::Stdf::NoError;
            break;

        case GS::LPPlugin::LicenseProvider::eTerOEM:  // OEM-Examinator for Teradyne
            {
                QStringList lTesters  = QStringList() << "IGXL" << "IG-XL" << "Teradyne" << "J750" << "J-750" <<
                                                         "Eagle" << "EWS" << "Flex" << "uFlex" << "microflex" <<
                                                         "integra" << "integraFlex" << "ultraFlex" << "Jaguar" <<
                                                         "ETS-800" << "ETS-88" << "Tiger" << "Catalyst" << "ETS364" <<
                                                         "ETS-364" << "ETS800" << "ETS88" <<
                                                         "A500" << "A510" << "A520" << "A530" << "A540" << "A550" <<
                                                         "A560" << "A570" << "A580" << "A590" << "ETS-200" <<
                                                         "ETS200" << "ETS-200T" << "ETS200T" << "EST-200-T-FT" <<
                                                         "ETS200TFT" << "ETS-300" << "ETS300" << "ETS-364" <<
                                                         "ETS364" << "ETS-500" << "ETS500" << "ETS-600" <<
                                                         "ETS600" << "ETS-564" << "ETS564" << "ETS-88TH" << "ETS88TH";

                QRegExp lTesterRegExp(lTesters.join("|"));
                lTesterRegExp.setCaseSensitivity(Qt::CaseInsensitive);

                if (strTesterType.contains(lTesterRegExp))
                {
                    return GS::StdLib::Stdf::NoError;
                }
            }
            break;

        case GS::LPPlugin::LicenseProvider::eLtxcOEM	:// OEM-Examinator for LTXC
            // If ASL series
            if(strTesterType.startsWith("ASL") && strExecType.startsWith("visualATE"))
            {
                // Clean the Revision string in case it starts with string "Rev" or Revision"
/*                strExecVersion.replace("Revision","", Qt::CaseInsensitive);
                strExecVersion.replace("Rev","", Qt::CaseInsensitive);
                strExecVersion = strExecVersion.trimmed();
                if(strExecVersion.isEmpty())
                    break;	// Missing OS revision...error

                // Check if this release accepts this version of Credence OS...
                // Check that OS version is in format X.Y
                iStatus = sscanf(strExecVersion.toLatin1().constData(),"%d%*c%d",&iMajorRelease,&iMinorRelease);
                if(iStatus != 2)
                    break;	// OS version string not valid
                if(iMajorRelease > GEX_OEM_ASL_OSVERSION)
                    break;	// OS version is more recent than the one accepted by Examinator: user needs to upgrade Examinator!*/
                return GS::StdLib::Stdf::NoError;	// Valid OS version!
            }

            // If STDF file created by Galaxy DL4_to_STDF converter (DL4: Old credence legacy format), accept it.
            if(strTesterType.startsWith("DL4_Source:", Qt::CaseInsensitive))
            {
                strExecVersion = strExecVersion.trimmed();
                if(strExecVersion.startsWith("Galaxy Converter"))
                    return GS::StdLib::Stdf::NoError;	// Valid OS version!
            }

            if(iStdfAtrTesterBrand == GS::LPPlugin::LicenseProvider::eLtxcOEM)
                return GS::StdLib::Stdf::NoError;	// Accept this OS as we found the STDF.ATR signature when reading data file in Pass1.

            // ExecType normaly is 'enVision'...but as operators can change it, it is no longer checked.
            if(strExecType.startsWith("enVision", Qt::CaseInsensitive) &&
               strTesterType.startsWith("Fusion", Qt::CaseInsensitive))
                return GS::StdLib::Stdf::NoError;

            if (strExecType.startsWith("Unison", Qt::CaseInsensitive) &&
                (strTesterType.startsWith("Fusion", Qt::CaseInsensitive) ||
                 strTesterType.contains("EX", Qt::CaseInsensitive) ||
                 strTesterType.contains("MX", Qt::CaseInsensitive) ||
                 strTesterType.contains("LX", Qt::CaseInsensitive) ||
                 strTesterType.contains("PA", Qt::CaseInsensitive) ||
                 strTesterType.contains("CX", Qt::CaseInsensitive) ||
                 strTesterType.contains("DMD", Qt::CaseInsensitive) ||
                 strTesterType.contains("Diamond", Qt::CaseInsensitive)))
                return GS::StdLib::Stdf::NoError;

            // If no signature found in ATR, then need at least default MIR fields...
            if(strTesterType.compare("D-10", Qt::CaseInsensitive) == 0 &&
               strExecType.compare("dmd_exe", Qt::CaseInsensitive) == 0)
                return GS::StdLib::Stdf::NoError;

            break;	// Refuse to analyze files .

        case GS::LPPlugin::LicenseProvider::eSzOEM:	// Examinator: Only SZ STDF files allowed
            if(strExecType.startsWith("Space Ver.") != true)
                break;	// Invalid string found.

            // Extract Space version from string: 'Space Ver. 6.08'
            strExecType = strExecType.section(' ',2);

            // Check if this release accepts this version of Credence SZ OS...
            // Check that OS version is in format 'Space Ver. 6.08'
            iStatus = sscanf(strExecType.toLatin1().constData(),"%d%*c%d",&iMajorRelease,&iMinorRelease);
            if(iStatus != 2)
                break;	// OS version string not valid

            // Check OS version is more recent than the one accepted by Examinator: user needs to upgrade Examinator!
            if(iMajorRelease > GEX_OEM_SZ_OSVERSION)
                break;
            return GS::StdLib::Stdf::NoError;	// Valid OS version!

//        case GEX_DATATYPE_ALLOWED_TERADYNE:	// Examinator: Only Teradyne STDF files allowed
//            if(strExecType.startsWith("Softlink Easylink") == true)
//                return GS::StdLib::Stdf::NoError;
//            if(strExecType.startsWith("Softlink SUPERZ") == true)
//                return GS::StdLib::Stdf::NoError;
//            if(strExecType.startsWith("IMAGE") == true)
//            {
//                // A5XX / Catalyst tester platform....
//                if(strTesterType == "Catalyst")
//                    return GS::StdLib::Stdf::NoError;
//                if(strTesterType == "A585")
//                    return GS::StdLib::Stdf::NoError;
//                if(strTesterType == "A565")
//                    return GS::StdLib::Stdf::NoError;
//                if(strTesterType == "A530")
//                    return GS::StdLib::Stdf::NoError;
//                if(strTesterType == "A5xx")
//                    return GS::StdLib::Stdf::NoError;
//            }
//            if(strExecType.startsWith("ig900plus") == true)

//            {
//                // J971 tester platform....
//                if(strTesterType == "j971")
//                    return GS::StdLib::Stdf::NoError;
//            }
//            break;

/* =======
Agilent can be:
            <ExecType>		<TesterType>
            "HPSmartTest"	"hp830000"
            "ADE"			"HP9490"
            NULL			93000-SOC

Eagle can be:
            "ETS Test Executive"	"ETS500D" , "ETS300",
*/
    }

    // This STDF file is not allowed!
    return GS::StdLib::Stdf::PlatformRestriction;
}

/////////////////////////////////////////////////////////////////////////////
// Write Group/File detailed Global Info for: 'Examinator' module. on the
// Global page
/////////////////////////////////////////////////////////////////////////////
void CGexReport::WriteGlobalPageExaminatorGroupDetailed(CGexGroupOfFiles *pGroup, CGexFileInGroup *pFile)
{
    char	szString[2048]="";
    float	fData=0.0f;
    long	lTestDuration=0;
//    int		nInvalidData = (int)4294967295UL;
    QString lOutputFormat=m_pReportOptions->GetOption("output", "format").toString();

    // Separator line using '-'
    WriteInfoLine(NULL,GEX_T("-"));
    WriteInfoLine("File name", pFile->strFileName.toLatin1().constData());
    WriteInfoLine("Tests mapping file", pFile->strMapTests.toLatin1().constData());
    WriteInfoLine("Setup time",TimeStringUTC(pFile->getMirDatas().lSetupT));
    WriteInfoLine("Start time",TimeStringUTC(pFile->getMirDatas().lStartT));
    WriteInfoLine("End time",TimeStringUTC(pFile->getMirDatas().lEndT));
    // Checks if can compute test duration. Use Start_t if exist, otherwise, use Setup_t if exist
    lTestDuration = -1;
    if((pFile->getMirDatas().lEndT > 0) && (pFile->getMirDatas().lSetupT > 0) && (pFile->getMirDatas().lEndT > pFile->getMirDatas().lSetupT))
        lTestDuration = pFile->getMirDatas().lEndT - pFile->getMirDatas().lSetupT;
    if((pFile->getMirDatas().lEndT > 0) && (pFile->getMirDatas().lStartT > 0) && (pFile->getMirDatas().lEndT > pFile->getMirDatas().lStartT))
        lTestDuration = pFile->getMirDatas().lEndT - pFile->getMirDatas().lStartT;
    WriteInfoLine("Test duration",HMSString(lTestDuration,0,szString));
    WriteInfoLine("Product",pFile->getMirDatas().szPartType);
    WriteInfoLine("Program",pFile->getMirDatas().szJobName);
    WriteInfoLine("Revision",pFile->getMirDatas().szJobRev);
    WriteInfoLine("Lot",pFile->getMirDatas().szLot);
    WriteInfoLine("Sub-Lot",pFile->getMirDatas().szSubLot);
    WriteInfoLine("WaferID",pFile->getWaferMapData().szWaferID);

    if (lOutputFormat=="PPT" || lOutputFormat=="ODP")
    {
        WritePageBreak(); // 7131 : there was another bug here...
        WriteHtmlOpenTable(98, 0);
    }

    // Parts to process
    WriteInfoLine("Parts processed",(pFile->BuildPartsToProcess()).toLatin1().data());

    // Testing on sites
    WriteInfoLine("Data from Sites",pFile->BuildSitesToProcess().toLatin1().data());

    // Device test time info (GOOD parts).
    if(pGroup->cMergedData.lMergedTestTimeParts_Good != 0 && pGroup->cMergedData.lMergedAverageTestTime_Good != 0)
    {
        // Compute test time in seconds.
        double fValue = ((double)(pGroup->cMergedData.lMergedAverageTestTime_Good)/(double)(pGroup->cMergedData.lMergedTestTimeParts_Good))/1000.0;
        sprintf(szString,"%.3lf sec. (excludes tester idle time)",fValue);
    }
    else
        sprintf(szString,GEX_NA);
    WriteInfoLine("Test time (GOOD parts)",szString);

    // Device test time info (ALL parts).
    if(pGroup->cMergedData.lMergedTestTimeParts_All != 0 && pGroup->cMergedData.lMergedAverageTestTime_All != 0)
    {
        // Compute test time in seconds.
        double fValue = ((double)(pGroup->cMergedData.lMergedAverageTestTime_All)/(double)(pGroup->cMergedData.lMergedTestTimeParts_All))/1000.0;
        sprintf(szString,"%.3lf sec. (excludes tester idle time)",fValue);
    }
    else
        sprintf(szString,GEX_NA);
    WriteInfoLine("Test time (ALL parts)",szString);

    // Average testing time / device
    if((pFile->getPcrDatas().lPartCount) && (pFile->getPcrDatas().lPartCount != GS_MAX_UINT32) && (lTestDuration>0))
    {
        fData = (float)lTestDuration/(float)(pFile->getPcrDatas().lPartCount);
        sprintf(szString,"%.3f sec. / device (includes tester idle time between parts)",fData);
    }
    else
        sprintf(szString,GEX_NA);
    WriteInfoLine("Average test time",szString);

    // Total parts tested
    if((pFile->getPcrDatas().lPartCount) && (pFile->getPcrDatas().lPartCount != GS_MAX_UINT32))
    {
        sprintf(szString,"%ld - Includes parts retested (if any)",pFile->getPcrDatas().lPartCount);
    }
    else
        sprintf(szString,GEX_NA);
    WriteInfoLine("Total parts tested",szString);

    // If Gross Die count enabled....
    if(pGroup->cMergedData.grossDieCount() > 0)
    {
        sprintf(szString,"%d", pGroup->cMergedData.grossDieCount());
        WriteInfoLine("Gross Die count",szString);
    }

    // Good parts
    if((pFile->getPcrDatas().lPartCount) && (pFile->getPcrDatas().lPartCount != GS_MAX_UINT32) && (pFile->getPcrDatas().lGoodCount>= 0))
    {
        if(pGroup->cMergedData.grossDieCount() > 0)
        {
            fData = (float)(100.0*pFile->getPcrDatas().lGoodCount)/pGroup->cMergedData.grossDieCount();
            sprintf(szString,"%d (%.2f%%) - Based on GrossDie count",pFile->getPcrDatas().lGoodCount,fData);
        }
        else
        {
            fData = (float)(100.0*pFile->getPcrDatas().lGoodCount)/pFile->getPcrDatas().lPartCount;
            sprintf(szString,"%d (%.2f%%) - Includes parts retested (if any)",pFile->getPcrDatas().lGoodCount,fData);
        }
    }
    else
        sprintf(szString,GEX_NA);
    WriteInfoLine("Good parts (Yield)",szString);

    // Bad parts
    if((pFile->getPcrDatas().lPartCount) && (pFile->getPcrDatas().lPartCount != GS_MAX_UINT32)  && (pFile->getPcrDatas().lGoodCount>= 0))
    {
        if(pGroup->cMergedData.grossDieCount() > 0)
        {
            fData = (float)(100.0*(pFile->getPcrDatas().lPartCount-pFile->getPcrDatas().lGoodCount))/pGroup->cMergedData.grossDieCount();
            sprintf(szString,"%ld (%.2f%%) - Based on GrossDie count",pFile->getPcrDatas().lPartCount-pFile->getPcrDatas().lGoodCount,fData);
        }
        else
        {
            fData = (float)(100.0*(pFile->getPcrDatas().lPartCount-pFile->getPcrDatas().lGoodCount))/pFile->getPcrDatas().lPartCount;
            sprintf(szString,"%ld (%.2f%%) - Includes parts retested (if any)",pFile->getPcrDatas().lPartCount-pFile->getPcrDatas().lGoodCount,fData);
        }
    }
    else
        sprintf(szString,GEX_NA);
    WriteInfoLine("Bad parts (Yield loss)",szString);

    // Parts retested.
    if((pFile->getPcrDatas().lPartCount > 0) && (pFile->getPcrDatas().lPartCount != GS_MAX_UINT32) && (pFile->getPcrDatas().lRetestCount>0))
    {
        fData = (float)(100.0*pFile->getPcrDatas().lRetestCount)/pFile->getPcrDatas().lPartCount;
        sprintf(szString,"%d (%.2f%%)",pFile->getPcrDatas().lRetestCount,fData);
    }
    else
        sprintf(szString,GEX_NA);
    WriteInfoLine("Parts retested",szString);

    // Parts aborted.
    if((pFile->getPcrDatas().lPartCount) && (pFile->getPcrDatas().lPartCount != GS_MAX_UINT32) && (pFile->getPcrDatas().lAbortCount>=0))
    {
        fData = (float)(100.0*pFile->getPcrDatas().lAbortCount)/pFile->getPcrDatas().lPartCount;
        sprintf(szString,"%d (%.2f%%)",pFile->getPcrDatas().lAbortCount,fData);
    }
    else
        sprintf(szString,GEX_NA);
    WriteInfoLine("Parts aborted",szString);

    // Functional parts tested
    if(pFile->getPcrDatas().lFuncCount >=0)
        sprintf(szString,"%d",pFile->getPcrDatas().lFuncCount);
    else
        strcpy(szString,GEX_NA);
    if(pFile->getPcrDatas().lFuncCount != GS_MAX_UINT32)
        WriteInfoLine("Functional parts tested",szString);

    if (lOutputFormat=="PPT" || lOutputFormat=="ODP")
    {
        WritePageBreak(); // 7131 : there was another bug here...
        WriteHtmlOpenTable(98, 0);
    }

    // Separator line using '-'
    WriteInfoLine(NULL,GEX_T("-"));
    /*if(pFile->StdfRecordHeader.iStdfVersion == 3)
        WriteInfoLine("STDF Version",GEX_T("3.0"));
    else
        if(pFile->StdfRecordHeader.iStdfVersion == 4)*/
            WriteInfoLine("STDF Version",GEX_T("4.0"));
    WriteInfoLine("Tester name",pFile->getMirDatas().szNodeName);
    WriteInfoLine("Tester type",pFile->getMirDatas().szTesterType);
    sprintf(szString,"%d",pFile->getMirDatas().bStation & 0xff);
    WriteInfoLine("Station",szString);
    WriteInfoLine("Part type",pFile->getMirDatas().szPartType);
    WriteInfoLine("Operator",pFile->getMirDatas().szOperator);
    WriteInfoLine("Exec_type",pFile->getMirDatas().szExecType);
    WriteInfoLine("Exec_version",pFile->getMirDatas().szExecVer);
    WriteInfoLine("TestCode",pFile->getMirDatas().szTestCode);
    WriteInfoLine("Test Temperature",pFile->getMirDatas().szTestTemperature);
    WriteInfoLine("User Text",pFile->getMirDatas().szUserText);
    WriteInfoLine("Aux_file",pFile->getMirDatas().szAuxFile);
    WriteInfoLine("Package type",pFile->getMirDatas().szPkgType);
    WriteInfoLine("Per_freq",pFile->getMirDatas().szperFrq);
    WriteInfoLine("Spec_name",pFile->getMirDatas().szSpecName);
    WriteInfoLine("Spec_version",pFile->getMirDatas().szSpecVersion);
    WriteInfoLine("Family ID",pFile->getMirDatas().szFamilyID);
    WriteInfoLine("Date code",pFile->getMirDatas().szDateCode);
    WriteInfoLine("Design Rev",pFile->getMirDatas().szDesignRev);
    WriteInfoLine("Facility ID",pFile->getMirDatas().szFacilityID);
    WriteInfoLine("Floor ID",pFile->getMirDatas().szFloorID);
    WriteInfoLine("Proc ID",pFile->getMirDatas().szProcID);
    WriteInfoLine("Flow ID",pFile->getMirDatas().szFlowID);
    WriteInfoLine("Setup ID",pFile->getMirDatas().szSetupID);
    WriteInfoLine("Eng ID",pFile->getMirDatas().szEngID);
    WriteInfoLine("ROM code",pFile->getMirDatas().szROM_Code);
    WriteInfoLine("Serial #",pFile->getMirDatas().szSerialNumber);
    WriteInfoLine("Super user name",pFile->getMirDatas().szSuprName);
    WriteInfoLine("Handler/Prober",pFile->getMirDatas().szHandlerProberID);

    // List of handler/proer, etc per site
    if(pFile->m_pSiteEquipmentIDMap != NULL)
    {
        GP_SiteDescriptionMap::const_iterator it;
        int	iSite=0;
        GP_SiteDescription	cSiteDescription;
        for ( it=pFile->m_pSiteEquipmentIDMap->begin(); it!=pFile->m_pSiteEquipmentIDMap->end(); ++it )
        {
            // Focus is on Soft bins
            iSite = it.key();
            cSiteDescription = it.value();

            // Write details
            WriteInfoLine(NULL,GEX_T("-"));
            sprintf(szString,"Site# %d",iSite);
            WriteInfoLine("Site details",szString);
            if(!cSiteDescription.m_strHandlerContactorID.isEmpty())
                WriteInfoLine("Handler contractor", cSiteDescription.m_strHandlerContactorID.toLatin1().constData());
            if(!cSiteDescription.m_strHandlerType.isEmpty())
                WriteInfoLine("Handler type", cSiteDescription.m_strHandlerType.toLatin1().constData());
            if(!cSiteDescription.m_strHandlerProberID.isEmpty())
                WriteInfoLine("Handler/Prober", cSiteDescription.m_strHandlerProberID.toLatin1().constData());
            if(!cSiteDescription.m_strProbeCardType.isEmpty())
                WriteInfoLine("Probe card type", cSiteDescription.m_strProbeCardType.toLatin1().constData());
            if(!cSiteDescription.m_strProbeCardID.isEmpty())
                WriteInfoLine("Probe card", cSiteDescription.m_strProbeCardID.toLatin1().constData());
            if(!cSiteDescription.m_strLoadBoardType.isEmpty())
                WriteInfoLine("Loadboard type", cSiteDescription.m_strLoadBoardType.toLatin1().constData());
            if(!cSiteDescription.m_strLoadBoardID.isEmpty())
                WriteInfoLine("Loadboard", cSiteDescription.m_strLoadBoardID.toLatin1().constData());
            if(!cSiteDescription.m_strDibBoardID.isEmpty())
                WriteInfoLine("Dib Board", cSiteDescription.m_strDibBoardID.toLatin1().constData());
            if(!cSiteDescription.m_strInterfaceCableID.isEmpty())
                WriteInfoLine("Interface cable", cSiteDescription.m_strInterfaceCableID.toLatin1().constData());
            if(!cSiteDescription.m_strLaserID.isEmpty())
                WriteInfoLine("Laser", cSiteDescription.m_strLaserID.toLatin1().constData());
            if(!cSiteDescription.m_strExtraEquipmentID.isEmpty())
                WriteInfoLine("Extra equipment", cSiteDescription.m_strExtraEquipmentID.toLatin1().constData());

        }
    }
}


/////////////////////////////////////////////////////////////////////////////
// Write Group/File summarized Global Info for: 'Examinator' module. on the
// Global page
/////////////////////////////////////////////////////////////////////////////
void
CGexReport::
WriteGlobalPageExaminatorGroupSummarized(CGexGroupOfFiles* /*pGroup*/,
                                         CGexFileInGroup* pFile)
{
    char	szString[2048];
    float	fData;
    long	lTestDuration;

    WriteInfoLine("File name", pFile->strFileName.toLatin1().constData());
    WriteInfoLine("Tests mapping file", pFile->strMapTests.toLatin1().constData());
    // Checks if can compute test duration.
    char szString2[50];
    if(pFile->getMirDatas().lStartT > 0)
        strcpy(szString,TimeStringUTC(pFile->getMirDatas().lStartT));
    if(pFile->getMirDatas().lEndT <= 0 || pFile->getMirDatas().lStartT <= 0)
    {
        lTestDuration = -1;
        strcpy(szString2,GEX_NA);
    }
    else
    {
        lTestDuration = pFile->getMirDatas().lEndT-pFile->getMirDatas().lStartT;
        HMSString(lTestDuration,0,szString2);
    }
    WriteInfoLine("Start time",szString);
    WriteInfoLine("Test duration",szString2);
    WriteInfoLine("Product",pFile->getMirDatas().szPartType);

    // Program / rev.
    WriteInfoLine("Program",pFile->getMirDatas().szJobName);
    WriteInfoLine("Revision",pFile->getMirDatas().szJobRev);

    // Lot / Sub-lot
    WriteInfoLine("Lot",pFile->getMirDatas().szLot);
    WriteInfoLine("Sub-Lot",pFile->getMirDatas().szSubLot);
    WriteInfoLine("WaferID",pFile->getWaferMapData().szWaferID);

    // Parts to process
    WriteInfoLine("Parts processed",(pFile->BuildPartsToProcess()).toLatin1().data());

    // Testing on sites
    WriteInfoLine("Data from Sites",pFile->BuildSitesToProcess().toLatin1().data());

    // Average testing time / device
    // Note: If report created by PAT-Man, this info is incorrect for site#1 as we merge all sites to recreate wafer...so we better not display this info!
    if((pFile->getPcrDatas().lPartCount) && (pFile->getPcrDatas().lPartCount != GS_MAX_UINT32) && (lTestDuration>0))
    {
        fData = (float) lTestDuration/pFile->getPcrDatas().lPartCount;
        sprintf(szString,"%.3f sec. / device (includes tester idle time between parts)",fData);
    }
    else
        sprintf(szString,GEX_NA);
    if(m_pReportOptions->getAdvancedReport() != GEX_ADV_OUTLIER_REMOVAL)
        WriteInfoLine("Average test time",szString);
}


/////////////////////////////////////////////////////////////////////////////
// Write Global page for: 'Examinator' module.
/////////////////////////////////////////////////////////////////////////////
int	CGexReport::WriteGlobalPageExaminator(void)
{
    // If single file analysis
    if (m_pReportOptions->iFiles == 1)
    {
        // Get pointer to first group & first file (we always have them exist)
        CGexGroupOfFiles *pGroup = getGroupsList().isEmpty()?NULL:getGroupsList().first();
        CGexFileInGroup *pFile  = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();
        WriteGlobalPageExaminatorGroupDetailed(pGroup, pFile);
    }
    else
    {
        // Multiple files, and possibly multiple groups...
        QString strOutputFormat=m_pReportOptions->GetOption("output", "format").toString();
        // Close header table.
        if (m_pReportOptions->isReportOutputHtmlBased())
            //(strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
            fprintf(hReportFile,"</table>\n");

        // Get pointer to first group & first file (we always have them exist)
        QListIterator<CGexGroupOfFiles*>	itGroupsList(getGroupsList());
        CGexGroupOfFiles *					pGroup = NULL;
        CGexFileInGroup *					pFile  = NULL;

        while(itGroupsList.hasNext())
        {
            // Write page break between each file group (ignored if not writing a flat HTML document)
            WritePageBreak();

            pGroup	= itGroupsList.next();
            QListIterator<CGexFileInGroup*> itFilesList(pGroup->pFilesList);

            // Display group name
            if (strOutputFormat=="CSV")
            {
                // Generating .CSV report file
                fprintf(hReportFile,"\nGroup name: %s\n",pGroup->strGroupName.toLatin1().constData());
            }
            else
                if (m_pReportOptions->isReportOutputHtmlBased())
                    //(strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
            {
                fprintf(hReportFile,"<p align=\"left\"><font color=\"#006699\" size=\"4\">Group name: %s<br>\n",
                    pGroup->strGroupName.toLatin1().constData());
                fprintf(hReportFile,"<img border=\"0\" src=\"../images/ruler.png\" width=\"616\" height=\"8\"><br>\n");
                fprintf(hReportFile,"</font></p>\n");
                fprintf(hReportFile,"<table border=\"0\" width=\"750\" cellspacing=\"1\">\n");
            }


            // new report option section (case 3635)
            {
                QString strOptionStorageDevice = (m_pReportOptions->GetOption("global_info","detail_level")).toString();

                while (itFilesList.hasNext())
                {
                    pFile = itFilesList.next();

                    if (strOptionStorageDevice == QString("detailed"))
                    {
                        WriteGlobalPageExaminatorGroupDetailed(pGroup, pFile);
                    }
                    else if (strOptionStorageDevice == QString("value"))
                    {
                        WriteGlobalPageExaminatorGroupSummarized(pGroup, pFile);
                    }

                    // Separator line using '-'
                    if(itFilesList.hasNext())
                        WriteInfoLine(NULL,GEX_T("-"));

                }
            }
            // Close Group descriptiontable.
            if (m_pReportOptions->isReportOutputHtmlBased())
                    //(strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
                fprintf(hReportFile,"</table>\n");
        };
    }// Multiple files
    return GS::StdLib::Stdf::NoError;
}


/////////////////////////////////////////////////////////////////////////////
// Write Global page for: 'ExaminatorDB' module.
/////////////////////////////////////////////////////////////////////////////
int	CGexReport::WriteGlobalPageExaminatorDB(void)
{
    char				szString[2048];
    float				fData;
    GexDatabaseQuery	*pQuery;
    QString				strFiles;
    int					iIndex;
    bool				bValidQuery;
    long				lTestDuration;

    // Handles first & file
    CGexFileInGroup *pFile;
    CGexGroupOfFiles *pGroup;

// Debugging code: To fix customer query problem...
#if 0
    // Empty folder (if exists)
    GS::Gex::Engine::GetInstance().GetDatabaseEngine().DeleteFolderContent("c://gex_trace");

    QDir	cDir;
    QString strDestFile;
    if(!cDir.exists("c:\\gex_trace"))
    {
        if(!cDir.mkdir("c:\\gex_trace"))
        {
            GS::Gex::Message::information(
                szAppFullName,"Failed to create folder 'c:/gex_trace'");
            goto continue_query;
        }
    }

    // Create ASCII file with the list of data files processed in the query.
    FILE *hFile;
    hFile = fopen("c:\\gex_trace\\files.csv","w");
    if(hFile == NULL)
    {
        GS::Gex::Message::information(
            szAppFullName,"Failed to create file 'c:/gex_trace/files.txt'");
        goto continue_query;
    }
    fprintf(hFile,"List of files in query...\nFile,STDF_Name,\n");
    // Get pointer to first group & first file (we always have them exist)
    pGroup = getGroupsList().isEmpty()?NULL:getGroupsList().first();
    pFile  = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();

    int nFilesListIndex = 0;
    while(pFile != NULL)
    {
        fprintf(hFile,"%s , %s\n",pFile->strFileName.toLatin1().constData(),pFile->strFileNameSTDF.toLatin1().constData());

        // Copy file analysed into gex_trace folder
        QFileInfo cFileInfo(pFile->strFileNameSTDF);
        strDestFile = "c:/gex_trace/" + cFileInfo.fileName();

        CGexSystemUtils::CopyFile(pFile->strFileNameSTDF,strDestFile);

        if(nFilesListIndex++ < pGroup->pFilesList.count())
            pFile = pGroup->pFilesList.at(nFileslistIndex);
        else
            pFile = NULL;

    };
    fclose(hFile);

    // Copy script file executed
    CGexSystemUtils::CopyFile(GS::Gex::Engine::GetInstance().GetAssistantScript(),"c:/gex_trace/script.csl");

continue_query:;

#endif

    QString strOutputFormat=m_pReportOptions->GetOption("output", "format").toString();
    // Close header table.
    if (m_pReportOptions->isReportOutputHtmlBased())
            //(strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
        fprintf(hReportFile,"</table>\n");

    // Get pointer to first group & first file (we always have them exist)
    QListIterator<CGexGroupOfFiles*>	itGroupsList(getGroupsList());

    char* tp=NULL;
    // Get pointer to first group & first file (we always have them exist)
    while(itGroupsList.hasNext())
    {
        // Get pointer to Group Query definition
        pGroup		= itGroupsList.next();
        pQuery		= &(pGroup->cQuery);
        bValidQuery = pQuery->strDatabaseLogicalName.isEmpty() ? false: true;

        if(bValidQuery == false)
            goto skipQueryDefinition;

        // Display group name
        if (strOutputFormat=="CSV")
        {
            // Generating .CSV report file
            fprintf(hReportFile, "\nQuery name: %s\n", pQuery->strTitle.toLatin1().constData());
        }
        else
            if (m_pReportOptions->isReportOutputHtmlBased())
                //(strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
        {
            fprintf(hReportFile,"<p align=\"left\"><font color=\"#006699\" size=\"4\">Query name: %s<br>\n",
                pQuery->strTitle.toLatin1().constData());
            fprintf(hReportFile,"<img border=\"0\" src=\"../images/ruler.png\" width=\"616\" height=\"8\"><br>\n");
            fprintf(hReportFile,"</font></p>\n");
            fprintf(hReportFile,"<table border=\"0\" width=\"750\" cellspacing=\"1\">\n");
        }

        // Database name
        if(pQuery->bLocalDatabase)
            sprintf(szString,"[Local] %s",pQuery->strDatabaseLogicalName.toLatin1().constData());
        else
            sprintf(szString,"[Server] %s",pQuery->strDatabaseLogicalName.toLatin1().constData());
        WriteInfoLine("Database",szString);

        // Time period
        tp=(char*)gexLabelTimePeriodChoices[pQuery->iTimePeriod];
        WriteInfoLine("Time Period", tp?tp:"?");

        if(pQuery->iTimePeriod == GEX_QUERY_TIMEPERIOD_CALENDAR)
        {
            // Custom dates picked from Calendar:
            QString strDate;
            strDate = pQuery->calendarFrom.toString(Qt::TextDate);
            WriteInfoLine("...From", strDate.toLatin1().constData());
            strDate = pQuery->calendarTo.toString(Qt::TextDate);
            WriteInfoLine("...To", strDate.toLatin1().constData());
        }
        else if(pQuery->iTimePeriod == GEX_QUERY_TIMEPERIOD_LAST_N_X)
        {
            GSLOG(SYSLOG_SEV_WARNING, "check me");
            QStringList sl=QStringList() << "day(s)" << "week(s)" << "month(s)" << "quarter(s)" << "year(s)";
            WriteInfoLine("Last ",
                QString("%1 %2")
                    .arg( pQuery->iTimeNFactor )
                    .arg( sl.at(pQuery->m_eTimeStep) )
                    .toLatin1().data() );
        }

        // Write filters
        if(pQuery->bExternal && !pQuery->bOfflineQuery && !pQuery->strlSqlFilters.isEmpty())
        {
            QStringList::ConstIterator	it;
            for(it = pQuery->strlSqlFilters.begin(); it != pQuery->strlSqlFilters.end(); it++)
                WriteInfoLine(((*it).section('=', 0, 0).toLatin1().constData()), ((*it).section('=', 1, 1).toLatin1().constData()));
        }
        else if((!pQuery->bExternal || pQuery->bOfflineQuery) && (pQuery->uFilterFlag))
        {
            // Custom filters defined...
            WriteInfoLine("List of filters",(char *)NULL);

            if(pQuery->uFilterFlag & GEX_QUERY_FLAG_BURNIN)
            {
                sprintf(szString,"%d",pQuery->iBurninTime);
                WriteInfoLine((char*)gexLabelFilterChoices[GEX_QUERY_FILTER_BURNIN],szString);
            }
            if(pQuery->uFilterFlag & GEX_QUERY_FLAG_ORIGIN)
                WriteInfoLine(gexLabelFilterChoices[GEX_QUERY_FILTER_ORIGIN], pQuery->strDataOrigin.toLatin1().constData());
            if(pQuery->uFilterFlag & GEX_QUERY_FLAG_FACILITY)
                WriteInfoLine(gexLabelFilterChoices[GEX_QUERY_FILTER_FACILITY], pQuery->strFacilityID.toLatin1().constData());
            if(pQuery->uFilterFlag & GEX_QUERY_FLAG_FAMILY)
                WriteInfoLine(gexLabelFilterChoices[GEX_QUERY_FILTER_FAMILY], pQuery->strFamilyID.toLatin1().constData());
            if(pQuery->uFilterFlag & GEX_QUERY_FLAG_FLOOR)
                WriteInfoLine(gexLabelFilterChoices[GEX_QUERY_FILTER_FLOOR], pQuery->strFloorID.toLatin1().constData());
            if(pQuery->uFilterFlag & GEX_QUERY_FLAG_FREQUENCYSTEP)
                WriteInfoLine(gexLabelFilterChoices[GEX_QUERY_FILTER_FREQUENCYSTEP], pQuery->strFrequencyStep.toLatin1().constData());
            if(pQuery->uFilterFlag & GEX_QUERY_FLAG_LOADBOARDNAME)
                WriteInfoLine(gexLabelFilterChoices[GEX_QUERY_FILTER_LOADBOARDNAME], pQuery->strLoadBoardName.toLatin1().constData());
            if(pQuery->uFilterFlag & GEX_QUERY_FLAG_LOADBOARDTYPE)
                WriteInfoLine(gexLabelFilterChoices[GEX_QUERY_FILTER_LOADBOARDTYPE], pQuery->strLoadBoardType.toLatin1().constData());
            if(pQuery->uFilterFlag & GEX_QUERY_FLAG_DIBNAME)
                WriteInfoLine(gexLabelFilterChoices[GEX_QUERY_FILTER_DIBNAME], pQuery->strDibName.toLatin1().constData());
            if(pQuery->uFilterFlag & GEX_QUERY_FLAG_DIBTYPE)
                WriteInfoLine(gexLabelFilterChoices[GEX_QUERY_FILTER_DIBTYPE], pQuery->strDibType.toLatin1().constData());
            if(pQuery->uFilterFlag & GEX_QUERY_FLAG_LOT)
                WriteInfoLine(gexLabelFilterChoices[GEX_QUERY_FILTER_LOT], pQuery->strLotID.toLatin1().constData());
            if(pQuery->uFilterFlag & GEX_QUERY_FLAG_OPERATOR)
                WriteInfoLine(gexLabelFilterChoices[GEX_QUERY_FILTER_OPERATOR], pQuery->strOperator.toLatin1().constData());
            if(pQuery->uFilterFlag & GEX_QUERY_FLAG_PACKAGE)
                WriteInfoLine(gexLabelFilterChoices[GEX_QUERY_FILTER_PACKAGE], pQuery->strPackageType.toLatin1().constData());
            if(pQuery->uFilterFlag & GEX_QUERY_FLAG_PROBERNAME)
                WriteInfoLine(gexLabelFilterChoices[GEX_QUERY_FILTER_PROBERNAME], pQuery->strProberName.toLatin1().constData());
            if(pQuery->uFilterFlag & GEX_QUERY_FLAG_PROBERTYPE)
                WriteInfoLine(gexLabelFilterChoices[GEX_QUERY_FILTER_PROBERTYPE], pQuery->strProberType.toLatin1().constData());
            if(pQuery->uFilterFlag & GEX_QUERY_FLAG_PRODUCT)
                WriteInfoLine(gexLabelFilterChoices[GEX_QUERY_FILTER_PRODUCT], pQuery->strProductID.toLatin1().constData());
            if(pQuery->uFilterFlag & GEX_QUERY_FLAG_PROGRAMNAME)
                WriteInfoLine(gexLabelFilterChoices[GEX_QUERY_FILTER_PROGRAMNAME], pQuery->strJobName.toLatin1().constData());
            if(pQuery->uFilterFlag & GEX_QUERY_FLAG_PROGRAMREVISION)
                WriteInfoLine(gexLabelFilterChoices[GEX_QUERY_FILTER_PROGRAMREVISION], pQuery->strJobRev.toLatin1().constData());
            if(pQuery->uFilterFlag & GEX_QUERY_FLAG_RETESTNBR)
                WriteInfoLine(gexLabelFilterChoices[GEX_QUERY_FILTER_RETESTNBR], pQuery->strRetestNbr.toLatin1().constData());
            if(pQuery->uFilterFlag & GEX_QUERY_FLAG_SUBLOT)
                WriteInfoLine(gexLabelFilterChoices[GEX_QUERY_FILTER_SUBLOT], pQuery->strSubLotID.toLatin1().constData());
            if(pQuery->uFilterFlag & GEX_QUERY_FLAG_TEMPERATURE)
                WriteInfoLine(gexLabelFilterChoices[GEX_QUERY_FILTER_TEMPERATURE], pQuery->strTemperature.toLatin1().constData());
            if(pQuery->uFilterFlag & GEX_QUERY_FLAG_TESTERNAME)
                WriteInfoLine(gexLabelFilterChoices[GEX_QUERY_FILTER_TESTERNAME], pQuery->strNodeName.toLatin1().constData());
            if(pQuery->uFilterFlag & GEX_QUERY_FLAG_TESTERTYPE)
                WriteInfoLine(gexLabelFilterChoices[GEX_QUERY_FILTER_TESTERTYPE], pQuery->strTesterType.toLatin1().constData());
            if(pQuery->uFilterFlag & GEX_QUERY_FLAG_TESTCODE)
                WriteInfoLine(gexLabelFilterChoices[GEX_QUERY_FILTER_TESTCODE], pQuery->strTestingCode.toLatin1().constData());
            if(pQuery->uFilterFlag & GEX_QUERY_FLAG_PROCESS)
                WriteInfoLine(gexLabelFilterChoices[GEX_QUERY_FILTER_PROCESS], pQuery->strProcessID.toLatin1().constData());
            if(pQuery->uFilterFlag & GEX_QUERY_FLAG_SITENBR)
            {
                sprintf(szString,"%d",pQuery->iSite);
                WriteInfoLine(gexLabelFilterChoices[GEX_QUERY_FILTER_SITENBR],szString);
            }
            if(pQuery->uFilterFlag & GEX_QUERY_FLAG_WAFERID)
                WriteInfoLine(gexLabelFilterChoices[GEX_QUERY_FILTER_WAFERID], pQuery->strWaferID.toLatin1().constData());
            if(pQuery->uFilterFlag & GEX_QUERY_FLAG_USER1)
                WriteInfoLine(gexLabelFilterChoices[GEX_QUERY_FILTER_USER1], pQuery->strUser1.toLatin1().constData());
            if(pQuery->uFilterFlag & GEX_QUERY_FLAG_USER2)
                WriteInfoLine(gexLabelFilterChoices[GEX_QUERY_FILTER_USER2], pQuery->strUser2.toLatin1().constData());
            if(pQuery->uFilterFlag & GEX_QUERY_FLAG_USER3)
                WriteInfoLine(gexLabelFilterChoices[GEX_QUERY_FILTER_USER3], pQuery->strUser3.toLatin1().constData());
            if(pQuery->uFilterFlag & GEX_QUERY_FLAG_USER4)
                WriteInfoLine(gexLabelFilterChoices[GEX_QUERY_FILTER_USER4], pQuery->strUser4.toLatin1().constData());
            if(pQuery->uFilterFlag & GEX_QUERY_FLAG_USER5)
                WriteInfoLine(gexLabelFilterChoices[GEX_QUERY_FILTER_USER5], pQuery->strUser5.toLatin1().constData());
        }
        else
        {
            // No filter defined
            WriteInfoLine("Filters","none");
        }

        // If up to 5 files in the query, give the first 5 names
        strFiles = "";	// Clear the list of files
        for(iIndex = 0;iIndex < std::min(pGroup->pFilesList.size(), 5); iIndex++)
        {
            pFile  = pGroup->pFilesList.at(iIndex);
            if(pFile == NULL)
                break;	// We've reached the end of the files list!

            // Build list of files.
            strFiles += pFile->strFileName;
            if (strOutputFormat=="CSV")
                strFiles += "\n,";
            else
            if (m_pReportOptions->isReportOutputHtmlBased())
                    //(strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
                strFiles += "<br>";
        }
        // If more than 5 files in query, display '...'
        if(pGroup->pFilesList.count() > 5)
            strFiles += "...";

        // Report first few data files
        WriteInfoLine("First few data files matching query", strFiles.toLatin1().constData());

        // Close Query Group description table.
        if (m_pReportOptions->isReportOutputHtmlBased())
                //(strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
            fprintf(hReportFile,"</table>\n");

skipQueryDefinition:

        // Write Lot info matching the Query.
        if(strOutputFormat=="CSV")
        {
            // Generating .CSV report file
            fprintf(hReportFile,"\nLots matching query criteria\n");
            fprintf(hReportFile,"\nDate,Product,Lot,SubLot,WaferID,Retest#,Program,Total parts,Good parts (Yield),Failing parts,Avg test time (GOOD parts),Avg test time (ALL parts),Avg test time (including idle time),UPH\n");
        }
        else
        if(m_pReportOptions->isReportOutputHtmlBased())
                //(strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
        {
            fprintf(hReportFile,"<p align=\"left\"><font color=\"#006699\" size=\"4\">Lots matching query criteria\n");
            fprintf(hReportFile,"</font></p>\n");
            fprintf(hReportFile,"<font size=\"3\">\n");
            fprintf(hReportFile,"<table border=\"0\" width=\"95%%\">\n");
            fprintf(hReportFile,"<tr>\n");
            fprintf(hReportFile,"<td bgcolor=%s align=\"left\"><b>Start date</td>\n",szFieldColor);
            fprintf(hReportFile,"<td bgcolor=%s align=\"left\"><b>Product</td>\n",szFieldColor);
            fprintf(hReportFile,"<td bgcolor=%s align=\"left\"><b>LotID</b></td>\n",szFieldColor);
            fprintf(hReportFile,"<td bgcolor=%s align=\"left\"><b>SubLot</b></td>\n",szFieldColor);
            fprintf(hReportFile,"<td bgcolor=%s align=\"left\"><b>WaferID</b></td>\n",szFieldColor);
            fprintf(hReportFile,"<td bgcolor=%s align=\"center\"><b>Retest#</b></td>\n",szFieldColor);
            fprintf(hReportFile,"<td bgcolor=%s align=\"left\"><b>Program</b></td>\n",szFieldColor);
            fprintf(hReportFile,"<td bgcolor=%s align=\"center\"><b>Total parts</b></td>\n",szFieldColor);
            fprintf(hReportFile,"<td bgcolor=%s align=\"center\"><b>Good parts (Yield)</b></td>\n",szFieldColor);
            fprintf(hReportFile,"<td bgcolor=%s align=\"center\"><b>Failing parts</b></td>\n",szFieldColor);
            fprintf(hReportFile,"<td bgcolor=%s align=\"center\"><b>Avg test time (GOOD parts)</b></td>\n",szFieldColor);
            fprintf(hReportFile,"<td bgcolor=%s align=\"center\"><b>Avg test time (ALL parts)</b></td>\n",szFieldColor);
            fprintf(hReportFile,"<td bgcolor=%s align=\"center\"><b>Avg test time (including idle time)</b></td>\n",szFieldColor);
            fprintf(hReportFile,"<td bgcolor=%s align=\"center\"><b>UPH</b></td>\n",szFieldColor);
            fprintf(hReportFile,"</tr>\n");
        }

        QString strDate,strProduct,strLot,strSublot,strWaferID,strProgram,strTested,strGood,strFail,strRetestNbr;
        QString	strDeviceTestTime_GOOD,strDeviceTestTime_ALL,strAverageTestTime,strUPH;
        long	lTotalFiles=0;
        long	lTotalParts=0;
        long	lTotalGoodParts=0;
        long	lTotalFailParts=0;
        long	lFileCount = pGroup->pFilesList.count();
        long	lTotalTestDuration_Device_Good=0, lTotalTestDuration_Device_All=0, lTotalTestDuration_Avg=0;
        long	lTotalParts_Device_Good=0, lTotalParts_Device_All=0, lTotalParts_Avg=0;

        QListIterator<CGexFileInGroup*> itFilesList(pGroup->pFilesList);

        while (itFilesList.hasNext())
        {
            pFile = itFilesList.next();

            // Compute test times
            lTestDuration = -1;
            if((pFile->getMirDatas().lEndT > 0) && (pFile->getMirDatas().lSetupT > 0) && (pFile->getMirDatas().lEndT > pFile->getMirDatas().lSetupT))
                lTestDuration = pFile->getMirDatas().lEndT - pFile->getMirDatas().lSetupT;
            if((pFile->getMirDatas().lEndT > 0) && (pFile->getMirDatas().lStartT > 0) && (pFile->getMirDatas().lEndT > pFile->getMirDatas().lStartT))
                lTestDuration = pFile->getMirDatas().lEndT - pFile->getMirDatas().lStartT;

            //Device test time (GOOD parts)
            if(pFile->lTestTimeParts_Good > 0 && pFile->lAverageTestTime_Good > 0)
            {
                // Compute test time in seconds.
                fData = ((float)(pFile->lAverageTestTime_Good)/(float)(pFile->lTestTimeParts_Good))/1000.0F;
                strDeviceTestTime_GOOD.sprintf("%.3lf sec",fData);
                lTotalTestDuration_Device_Good += pFile->lAverageTestTime_Good;
                lTotalParts_Device_Good += pFile->lTestTimeParts_Good;
            }
            else
                strDeviceTestTime_GOOD = GEX_NA;

            //Device test time (ALL parts)
            if(pFile->lTestTimeParts_All > 0 && pFile->lAverageTestTime_All > 0)
            {
                // Compute test time in seconds.
                fData = ((float)(pFile->lAverageTestTime_All)/(float)(pFile->lTestTimeParts_All))/1000.0F;
                strDeviceTestTime_ALL.sprintf("%.3lf sec",fData);
                lTotalTestDuration_Device_All += pFile->lAverageTestTime_All;
                lTotalParts_Device_All += pFile->lTestTimeParts_All;
            }
            else
                strDeviceTestTime_ALL = GEX_NA;

            // Average testing time / device (including idle time)
            if((pFile->getPcrDatas().lPartCount) && (pFile->getPcrDatas().lPartCount != GS_MAX_UINT32) && (lTestDuration>0))
            {
                fData = (float)lTestDuration/(float)(pFile->getPcrDatas().lPartCount);
                strAverageTestTime.sprintf("%.3f sec",fData);
                lTotalTestDuration_Avg += lTestDuration;
                lTotalParts_Avg += pFile->getPcrDatas().lPartCount;
                strUPH.sprintf("%ld", 3600*(pFile->getPcrDatas().lPartCount)/lTestDuration);
            }
            else
            {
                strAverageTestTime = GEX_NA;
                strUPH = GEX_NA;
            }

            if(pFile->getMirDatas().lStartT > 0)
            {
                strcpy(szString,TimeStringUTC(pFile->getMirDatas().lStartT));
                strDate = szString;
                strDate = strDate.trimmed();	// remove leading spaces & <LF>
            }
            else
                strDate = GEX_NA;

            // Product (PartType)
            strProduct = pFile->getMirDatas().szPartType;

            // Program
            strProgram = pFile->getMirDatas().szJobName;

            // Retest count
            strRetestNbr = pFile->getMirDatas().bRtstCode;

            // Lot / Sub-lot
            strLot= pFile->getMirDatas().szLot;
            strSublot = pFile->getMirDatas().szSubLot;
            strWaferID = pFile->getWaferMapData().szWaferID;

            // Check if this logical file matches our query criteria
            if((pFile->strWaferToExtract.isEmpty()==false) && (pFile->isMatchingWafer(pFile->strWaferToExtract,strWaferID) == false))
                continue;

            // Total parts tested
            if((pFile->getPcrDatas().lPartCount) && (pFile->getPcrDatas().lPartCount != GS_MAX_UINT32))
            {
                strTested = QString::number(pFile->getPcrDatas().lPartCount);
                lTotalParts += pFile->getPcrDatas().lPartCount;
                lTotalFiles++;	// Keep track of total files data merged.
            }
            else
                strTested = GEX_NA;

            // Good parts
            if((pFile->getPcrDatas().lPartCount) && (pFile->getPcrDatas().lPartCount != GS_MAX_UINT32) && (pFile->getPcrDatas().lGoodCount>= 0))
            {
                fData = (float)(100.0*pFile->getPcrDatas().lGoodCount)/pFile->getPcrDatas().lPartCount;
                sprintf(szString,"%d (%.2f%%)",pFile->getPcrDatas().lGoodCount,fData);
                strGood = szString;
                lTotalGoodParts += pFile->getPcrDatas().lGoodCount;
            }
            else
                strGood = GEX_NA;

            // Bad parts
            if((pFile->getPcrDatas().lPartCount) && (pFile->getPcrDatas().lPartCount != GS_MAX_UINT32)  && (pFile->getPcrDatas().lGoodCount>= 0))
            {
                fData = (float)(100.0*(pFile->getPcrDatas().lPartCount-pFile->getPcrDatas().lGoodCount))/pFile->getPcrDatas().lPartCount;
                sprintf(szString,"%ld (%.2f%%)",pFile->getPcrDatas().lPartCount-pFile->getPcrDatas().lGoodCount,fData);
                strFail = szString;
                lTotalFailParts += pFile->getPcrDatas().lPartCount-pFile->getPcrDatas().lGoodCount;
            }
            else
                strFail = GEX_NA;

            // Dump info into file or HTML page
            if(	m_pReportOptions->isReportOutputHtmlBased()
                    //(strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
                && (lFileCount <= GEX_PAGEGLOBAL_MAX_LOTS_DISPLAYED)
                )
            {
                fprintf(hReportFile,"<tr>\n");
                fprintf(hReportFile,"<td bgcolor=%s align=\"left\">%s</td>\n",szDataColor,strDate.toLatin1().constData());
                fprintf(hReportFile,"<td bgcolor=%s align=\"left\">%s</td>\n",szDataColor,strProduct.toLatin1().constData());
                fprintf(hReportFile,"<td bgcolor=%s align=\"left\">%s</td>\n",szDataColor,strLot.toLatin1().constData());
                fprintf(hReportFile,"<td bgcolor=%s align=\"left\">%s&nbsp;</td>\n",szDataColor,strSublot.toLatin1().constData());
                fprintf(hReportFile,"<td bgcolor=%s align=\"left\">%s&nbsp;</td>\n",szDataColor,strWaferID.toLatin1().constData());
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\">%s&nbsp;</td>\n",szDataColor,strRetestNbr.toLatin1().constData());
                fprintf(hReportFile,"<td bgcolor=%s align=\"left\">%s</td>\n",szDataColor,strProgram.toLatin1().constData());
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\">%s</td>\n",szDataColor,strTested.toLatin1().constData());
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\">%s</td>\n",szDataColor,strGood.toLatin1().constData());
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\">%s</td>\n",szDataColor,strFail.toLatin1().constData());
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\">%s</td>\n",szDataColor,strDeviceTestTime_GOOD.toLatin1().constData());
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\">%s</td>\n",szDataColor,strDeviceTestTime_ALL.toLatin1().constData());
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\">%s</td>\n",szDataColor,strAverageTestTime.toLatin1().constData());
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\">%s</td>\n",szDataColor,strUPH.toLatin1().constData());
                fprintf(hReportFile,"</tr>\n");
            }
            else if(strOutputFormat=="CSV")
            {
                // CSV file
                fprintf(hReportFile,"%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n",
                    strDate.toLatin1().constData(),
                    strProduct.toLatin1().constData(),
                    strLot.toLatin1().constData(),
                    strSublot.toLatin1().constData(),
                    strWaferID.toLatin1().constData(),
                    strRetestNbr.toLatin1().constData(),
                    strProgram.toLatin1().constData(),
                    strTested.toLatin1().constData(),
                    strGood.toLatin1().constData(),
                    strFail.toLatin1().constData(),
                    strDeviceTestTime_GOOD.toLatin1().constData(),
                    strDeviceTestTime_ALL.toLatin1().constData(),
                    strAverageTestTime.toLatin1().constData(),
                    strUPH.toLatin1().constData());
            }
        };

        // If more than 100 files, just write a row saying so
        if(	m_pReportOptions->isReportOutputHtmlBased()
                //(strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
            && (lFileCount > GEX_PAGEGLOBAL_MAX_LOTS_DISPLAYED) )
        {
            fprintf(hReportFile,"<tr>\n");
            fprintf(hReportFile,
                    "<td colspan=8 bgcolor=%s align=\"left\">More than %d lots (%ld) matching query criteria. Only cumulated values reported...</td>\n",
                    szDataColor, GEX_PAGEGLOBAL_MAX_LOTS_DISPLAYED, lFileCount);
            fprintf(hReportFile,"</tr>\n");
        }

        // Write Cumulated count if at least 2 files are listed
        if(lTotalFiles > 1)
        {
            //Device test time (GOOD parts)
            if(lTotalParts_Device_Good > 0)
            {
                // Compute test time in seconds.
                fData = ((float)lTotalTestDuration_Device_Good/(float)lTotalParts_Device_Good)/1000.0F;
                strDeviceTestTime_GOOD.sprintf("%.3lf sec",fData);
            }
            else
                strDeviceTestTime_GOOD = GEX_NA;

            //Device test time (ALL parts)
            if(lTotalParts_Device_All > 0)
            {
                // Compute test time in seconds.
                fData = ((float)lTotalTestDuration_Device_All/(float)lTotalParts_Device_All)/1000.0F;
                strDeviceTestTime_ALL.sprintf("%.3lf sec",fData);
            }
            else
                strDeviceTestTime_ALL = GEX_NA;

            // Average testing time / device (including idle time)
            if(lTotalParts_Avg > 0)
            {
                fData = (float)lTotalTestDuration_Avg/(float)lTotalParts_Avg;
                strAverageTestTime.sprintf("%.3f sec",fData);
                strUPH.sprintf("%ld", 3600*(lTotalParts_Avg)/lTotalTestDuration_Avg);
            }
            else
                strAverageTestTime = GEX_NA;

            strTested = QString::number(lTotalParts);
            fData = (float)(100.0*lTotalGoodParts)/lTotalParts;
            sprintf(szString,"%ld (%.2f%%)",lTotalGoodParts,fData);
            strGood = szString;
            fData = (float)(100.0*(lTotalFailParts))/lTotalParts;
            sprintf(szString,"%ld (%.2f%%)",lTotalFailParts,fData);
            strFail = szString;
            // Dump info into file or HTML page
            if (m_pReportOptions->isReportOutputHtmlBased())
                    //(strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
            {
                fprintf(hReportFile,"<tr>\n");
                fprintf(hReportFile,"<td bgcolor=%s align=\"left\"><b>Cumul...</b></td>\n",szDataColor);
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\"><b>-</b></td>\n",szDataColor);
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\"><b>-</b></td>\n",szDataColor);
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\"><b>-</b></td>\n",szDataColor);
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\"><b>-</b></td>\n",szDataColor);
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\"><b>-</b></td>\n",szDataColor);
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\"><b>-</b></td>\n",szDataColor);
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\"><b>%s</b></td>\n",szDataColor,strTested.toLatin1().constData());
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\"><b>%s</b></td>\n",szDataColor,strGood.toLatin1().constData());
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\"><b>%s</b></td>\n",szDataColor,strFail.toLatin1().constData());
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\"><b>%s</b></td>\n",szDataColor,strDeviceTestTime_GOOD.toLatin1().constData());
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\"><b>%s</b></td>\n",szDataColor,strDeviceTestTime_ALL.toLatin1().constData());
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\"><b>%s</b></td>\n",szDataColor,strAverageTestTime.toLatin1().constData());
                fprintf(hReportFile,"<td bgcolor=%s align=\"center\"><b>%s</b></td>\n",szDataColor,strUPH.toLatin1().constData());
                fprintf(hReportFile,"</tr>\n");
            }
            else
            {
                // CSV file
                fprintf(hReportFile,"Cumul,,,,,%s,%s,%s,%s,%s,%s,%s\n",
                    strTested.toLatin1().constData(),
                    strGood.toLatin1().constData(),
                    strFail.toLatin1().constData(),
                    strDeviceTestTime_GOOD.toLatin1().constData(),
                    strDeviceTestTime_ALL.toLatin1().constData(),
                    strAverageTestTime.toLatin1().constData(),
                    strUPH.toLatin1().constData());
            }
        }

        // Close Group descriptiontable.
        if(m_pReportOptions->isReportOutputHtmlBased())
                //(strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
        {
            fprintf(hReportFile,"</table>\n");
            fprintf(hReportFile,"</font>\n");
        }
    };

    return GS::StdLib::Stdf::NoError;
}

/////////////////////////////////////////////////////////////////////////////
// Creates ALL the .CSV or HTML pages for 'Global'
/////////////////////////////////////////////////////////////////////////////
int	CGexReport::CreatePages_Global()
{
    int		iStatus;
    char	szString[2048];

    // Update process bar...
    GS::Gex::Engine::GetInstance().UpdateProgressStatus(false,-1,-1);	// Increment to next step.

    // Create Global Report (MIR info)
    iStatus = PrepareSection_Global(true);
    if(iStatus != GS::StdLib::Stdf::NoError)
    {
        GSLOG(SYSLOG_SEV_WARNING,
              QString("PrepareSection_Global failed (code %1)!").arg( iStatus).toLatin1().constData());
        return iStatus;
    }

    QString strOutputFormat=m_pReportOptions->GetOption("output", "format").toString();
    if (strOutputFormat=="CSV")
        fprintf(hReportFile,"Master info,Value\n");
    // Write Report 'Globals'
    sprintf(szString,"%s - www.mentor.com", GS::Gex::Engine::GetInstance().Get("AppFullName").toString().toLatin1().data() );
    WriteInfoLine("Report from",szString);

    // Write Monitoring script name (if caller is Monitoring)
    if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
        WriteInfoLine("Script executed",
                      GS::Gex::CSLEngine::GetInstance().GetScriptName().toLatin1().constData());

    time_t lCurrentDate = time(NULL);
    WriteInfoLine("Report created",TimeString(lCurrentDate);

    // Size of file(s) processed
    float fKilo = (float) lfTotalFileSize / 1024.0;
    float fMega = fKilo / 1024.0;
    float fGiga = fMega / 1024.0;
    if(fGiga >=1.0)
        sprintf(szString,"%.1f GB", fGiga);
    else
    if(fMega >=1.0)
        sprintf(szString,"%.1f MB (%ld bytes)", fMega,(long)lfTotalFileSize);
    else
        sprintf(szString,"%.1f KB (%ld bytes)", fKilo,(long)lfTotalFileSize);
    WriteInfoLine("Data processed",szString);
    WriteMirProcessingTime();	// Tells processing time + funny comment !
    // Writes 'Coffee' Index: coffe can't cool-down !
    double fSpeed = (lfTotalFileSize)/((double)m_tProcessTime.elapsed());
    sprintf(szString,"%.1f KB/sec", fSpeed);
    WriteInfoLine("Processing speed",szString);

    // Write License expiration date (unless must be hiden. eg: OEM software)
    if(GS::LPPlugin::ProductInfo::getInstance()->getProductID() !=  GS::LPPlugin::LicenseProvider::eLtxcOEM)
    {
        strcpy(szString, GS::Gex::Engine::GetInstance().GetExpirationDate().toString(Qt::TextDate).toLatin1().constData());
        WriteInfoLine("Examinator expires",szString);
    }

    if(GS::LPPlugin::ProductInfo::getInstance()->isExaminatorPRO() ||
       GS::LPPlugin::ProductInfo::getInstance()->isExaminatorTerProPlus() ||
       GS::LPPlugin::ProductInfo::getInstance()->isExaminatorPAT() )
    {
            if(mProcessingFile == false)
                iStatus = WriteGlobalPageExaminatorDB();
            else
                iStatus = WriteGlobalPageExaminator();	// File report while running in Database mode.
    }
    else
    {
        // Examinator report or ExaminatorMonitoring (then one file processed at a time)
        iStatus = WriteGlobalPageExaminator();
    }

    if(iStatus != GS::StdLib::Stdf::NoError)
    {
        GSLOG(SYSLOG_SEV_WARNING,
              QString("WriteGlobalPage failed (code %1) !").arg( iStatus).toLatin1().constData());
        return iStatus;
    }

    // Close Global Info table.
    if(m_pReportOptions->isReportOutputHtmlBased())
    //if (strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
        fprintf(hReportFile,"</table>\n");

    // Write page break (ignored if not writing a flat HTML document)
    WritePageBreak();

    ///////////////////////////////////////////////////
    // Global Options
    ///////////////////////////////////////////////////
    WriteHtmlSectionTitle(hReportFile,"all_globaloptions","Global Options");

    // New table
    //if (strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
    if(m_pReportOptions->isReportOutputHtmlBased())
        fprintf(hReportFile,"<table border=\"0\" width=\"750\" cellspacing=\"1\">\n");

    // List of global options...
    QString strString;
    QString strDuplicateTest = m_pReportOptions->GetOption("dataprocessing", "duplicate_test").toString();

    if(strDuplicateTest == "merge")
        strString = "Always merge tests with identical test number (ignore test name)";
    else if(strDuplicateTest == "merge_name")
        strString = "Always merge tests with identical test name (ignore test#)";
    else
        strString = "Never merge tests with identical test number if test name not matching";
    WriteInfoLine("Test# policy", strString.toLatin1().constData());

    // outlier removal option
    QString orm=m_pReportOptions->GetOption("dataprocessing","data_cleaning_mode").toString();
    double orv=m_pReportOptions->GetOption("dataprocessing","data_cleaning_value").toDouble();
    if (orm=="none") strString = "None (keep all data)";
    else if(orm=="n_pourcent")
    {
        strString=QString("%1% of limits space").arg((int)orv);
        if (orv==100.f) strString.append(" (ignore values outside the limits)");
    }
    else if(orm=="n_sigma")
    {
        strString.sprintf("+/- N*Sigma range, (centered on Mean). With N=%g", orv);
    }
    else if(orm=="n_iqr")
    {	strString.sprintf("IQR based: LL=Q1-N*IQR, HL=Q3+N*IQR. With N=%g", orv);
    }
    else if (orm=="exclude_n_sigma")
    {
        strString.sprintf("Exclude +/- N*Sigma range, (centered on Mean). With N=%g",orv);
    }

    WriteInfoLine("Data Cleaning", strString.toLatin1().constData());

    // Statistics computation
    {
        QString strOptionStorageDevice = (m_pReportOptions->GetOption("statistics","computation")).toString();
        if(strOptionStorageDevice == "samples_only")				// Compute statistics from samples only
        {
            strString = "From samples data only";
        }
        else if(strOptionStorageDevice == "summary_only")			// Compute statistics from summary only
        {
            strString = "From summary data only";
        }
        else if(strOptionStorageDevice == "samples_then_summary")	// Compute statistics from samples (if exists), then summary
        {
            strString = "From samples data (if any), otherwise from summary";
        }
        else if(strOptionStorageDevice == "summary_then_samples")	// Compute statistics from summary (if exists), then samples
        {
            strString = "From summary data (if any), otherwise from samples";
        }
        else
        {
            GSLOG(SYSLOG_SEV_NOTICE, QString("unknown option (statistics,computation) : %1 !")
                  .arg( strOptionStorageDevice).toLatin1().constData());
            GEX_ASSERT(false);
        }
    }

    WriteInfoLine("Statistics computation", strString.toLatin1().constData());

    // Binning computation
    {
        QString strOptionStorageDevice = (m_pReportOptions->GetOption("binning","computation")).toString();
        if(strOptionStorageDevice == "wafer_map")
            strString = "From wafermap / strip map report only (handles retests properly)";
        else if(strOptionStorageDevice == "samples")
            strString = "From samples data only";
        else if(strOptionStorageDevice == "summary")
            strString = "From summary data (if any), otherwise from samples";
        else
        {
            GSLOG(SYSLOG_SEV_NOTICE, QString("unknown option (binning,computation) %1")
                  .arg( strOptionStorageDevice).toLatin1().constData() );
            GEX_ASSERT(false);
            strString = "From summary data (if any), otherwise from samples";
        }
    }
    WriteInfoLine("Binning computation", strString.toLatin1().constData());

    // Cpk formula
    QString f=m_pReportOptions->GetOption("statistics", "cp_cpk_computation").toString();
    if (f=="percentile")
        strString = "Use percentile formula (Cnpk)";
    else
        strString = "Use standard Sigma formula";
    WriteInfoLine("Cp,Cpk computation", strString.toLatin1().constData());

    // Shift variation formula selected: shift over mean
    // Correlation formula type for the Mean: value or limit space.
    {
        QString strOptionStorageDevice= (m_pReportOptions->GetOption("statistics","mean_drift_formula")).toString();
        if ( strOptionStorageDevice == QString("limits") )
            strString = "Percentage of limits space drift";	// Drift formula: % of limits space
        else			// strOptionStorageDevice == QString("value")
            strString = "Percentage of value drift"; // Drift formula: % of value
    }
    WriteInfoLine("Mean drift formula", strString.toLatin1().constData());

    // Close Options table.
    //if (strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
    if (m_pReportOptions->isReportOutputHtmlBased())
        fprintf(hReportFile,"</table>\n");

    iStatus = CloseSection_Global();
    return iStatus;
}

