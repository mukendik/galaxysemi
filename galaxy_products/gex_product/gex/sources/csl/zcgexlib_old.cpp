///////////////////////////////////////////////////////////
// Scripting commands to control GEX.
///////////////////////////////////////////////////////////

#include <qregexp.h>

#include "ZBase.h"  // load ZC_.... defines
#include <sstream>

#ifndef ZC_NATIVECSLLIB
  #include "ZCslWrap.hpp"
#else
  #include "ZCsl.hpp"
#endif

#include <gqtl_sysutils.h>

// Holds the message thrown by the script exception...
#include "../report_build.h"
#include "../interactive_charts.h"		// Layer Style class.
#include "../drill_chart.h"
#include "../script_wizard.h"
#include "../scripting_io.h"
#include "../db_transactions.h"
#include "../gex_web.h"
#include "../plugin_manager.h"
#include "../export_ascii_dialog.h"			// USed for Toolbox: STDF Dump function.
#include "../export_csv.h"
#include "../db_external_database.h"
#include "../classes.h"


// main
extern GexMainwindow	*pGexMainWindow;
extern char	*szAppFullName;
extern CReportOptions	ReportOptions;	// Holds options (report_build.h)
extern CGexReport *gexReport;			// Handle to report class

// In activation_key.cpp
extern QDate	ExpirationDate;		// Date when release expires...

// in patman_lib.cpp
extern int ExtractTestRange(char *szValue,unsigned long *lFromTest,long *lFromPinmapIndex,unsigned long *lToTest,long *lToPinmapIndex);
extern void	ExtractTestCouple(char *szRange,long *lFromTest,long *lFromPinmapIndex,long *lToTest,long *lToPinmapIndex);

// in settings_dialog.cpp
extern QString ImportTestListFromFile(const QString &strTestListFile);
extern bool checkProfessionalFeature(bool bProductionReleaseOnly);

static	GexDatabaseQuery cQuery;		// Holds Query parameters.
static	CSTDFtoASCII *ptStdfToAscii=NULL;	// Toolbox: Dump STDF file
static	ExportAscii_InputFileList	m_pInputFiles;
static int	setQueryFilter(char *szSection, char *szField);

static	CBinColor	cBinColor;			// Holds the Bin# and color.
static	int	m_lInteractiveTestX;		// Test in X to plot (interactive mode only)
static	int	m_lInteractiveTestPmX;		// Test Pinmap in X to plot (interactive mode only)
static	int	m_lGroupX;					// GroupID to use in X
static	int	m_lInteractiveTestY;		// Test in Y to plot (interactive mode only)
static	int	m_lInteractiveTestPmY;		// Test Pinmap in Y to plot (interactive mode only)
static	int	m_lGroupY;					// GroupID to use in X
static	int	m_InteractiveLayer;			// Keeps track of total layers inserted by the scripts.

////////////////////////////////////////////////////////////////////////////////
// Return handle to cQuery local structure.
////////////////////////////////////////////////////////////////////////////////
GexDatabaseQuery *getScriptQuery(void)
{
	return &cQuery;
}

////////////////////////////////////////////////////////////////////////////////
// Update the ASP/HTML page that lists the HTML reports available + return its name
// Call : gexUpdateWebReportsList();
////////////////////////////////////////////////////////////////////////////////
static QString gexUpdateWebReportsList(void)
{
	// Only applies to ExaminatorWeb!
	if(ReportOptions.lProductID != GEX_DATATYPE_ALLOWED_DATABASEWEB)
		return "";

	// Build full path to the HTML page to build
	QString strShowPage = pGexMainWindow->strExaminatorWebUserHome + GEX_DATABASE_WEB_EXCHANGE;
	strShowPage += GEX_HTMLPAGE_WEB_REPORTS;

	pGexMainWindow->cGexWeb->htmlBuildAdminReports(strShowPage);

	// Return path the the ASP/HTML page created.
	return strShowPage;
}

////////////////////////////////////////////////////////////////////////////////
// Update the ASP/HTML page that lists the databases available + return its name
// Call : gexUpdateWebDatabasesList();
////////////////////////////////////////////////////////////////////////////////
static QString gexUpdateWebDatabasesList(void)
{
	// Only applies to ExaminatorWeb!
	if(ReportOptions.lProductID != GEX_DATATYPE_ALLOWED_DATABASEWEB)
		return "";

	// Build full path to the HTML page to build
	QString strShowPage = pGexMainWindow->strExaminatorWebUserHome + GEX_DATABASE_WEB_EXCHANGE;
	strShowPage += GEX_HTMLPAGE_WEB_DATABASES;

	pGexMainWindow->cGexWeb->htmlBuildAdminDatabases(strShowPage);

	// Return path the the ASP/HTML page created.
	return strShowPage;
}

////////////////////////////////////////////////////////////////////////////////
// Function gexConvertSTDF: Convert file to STDF (Examinator-MONITORING ONLY)
// Call : gexConvertSTDF(QString strInputFile);
////////////////////////////////////////////////////////////////////////////////
static ZString gexConvertSTDF(ZCsl* csl)
{
	// Only applies to Examinator-Monitoring!
	if(ReportOptions.lProductID != GEX_DATATYPE_GEX_MONITORING &&
		ReportOptions.lProductID != GEX_DATATYPE_GEX_PATMAN)
		return 0;

	int argc = csl->get("argCount").asInt();

	CGConvertToSTDF StdfConvert;	// To Convert to STDF
	char	*szInputFile;			// Pointer to input file to convert.
	QString	strFileNameSTDF;		// To receive STDF file name created.
	QString	strErrorMessage;
	bool	bFileCreated;

	// Read input file to convert
	szInputFile = (char*) csl->get("input_file").constBuffer();
	
	// build output file name to create
	if(argc == 2)
	{
		// The script has specified a output file...
		strFileNameSTDF = csl->get("output_file").constBuffer();
	}
	else
	{
		strFileNameSTDF = szInputFile;
		strFileNameSTDF += "_gexmo.std";
	}
	
	// Convert file to STDF
	int nConvertStatus = StdfConvert.Convert(true,false,false, true, szInputFile, strFileNameSTDF,"",bFileCreated,strErrorMessage);
	// If conversion failed...return error code
	if(nConvertStatus == CGConvertToSTDF::eConvertError)
		return 0;

	return 1;
}


////////////////////////////////////////////////////////////////////////////////
// Function gexConvertCSV: Convert file to CSV (Examinator-MONITORING ONLY)
// Call : gexConvertCSV(QString strInputFile);
////////////////////////////////////////////////////////////////////////////////
static ZString gexConvertCSV(ZCsl* csl)
{
	// Only applies to Examinator-Monitoring!
	if(ReportOptions.lProductID != GEX_DATATYPE_GEX_MONITORING &&
		ReportOptions.lProductID != GEX_DATATYPE_GEX_PATMAN)
		return 0;

	int argc = csl->get("argCount").asInt();

	CGConvertToSTDF StdfConvert;	// To Convert to STDF
	CSTDFtoCSV		CsvConvert;
	char	*szInputFile;			// Pointer to input file to convert.
	QString	strFileNameSTDF;		// To receive STDF file name created.
	QString	strFileNameCSV;			// To receive CSV file name created.
	QString	strErrorMessage;
	bool	bStdfFileCreated;

	// Read input file to convert
	szInputFile = (char*) csl->get("input_file").constBuffer();
	
	// Intermediate STDF file to create (unless already STDF)
	strFileNameSTDF = szInputFile;
	strFileNameSTDF += "_gexmo.std";
	
	// build output file name to create
	if(argc == 2)
	{
		// The script has specified a output file...
		strFileNameCSV = csl->get("output_file").constBuffer();
	}
	else
	{
		strFileNameCSV = szInputFile;
		strFileNameCSV += "_gexmo.csv";
	}
	
	// Convert file to STDF
	bool bResult;
	int nConvertStatus = StdfConvert.Convert(true,false,false, true, szInputFile, strFileNameSTDF,"",bStdfFileCreated,strErrorMessage);
	// If conversion failed...return error code
	if(nConvertStatus == CGConvertToSTDF::eConvertError)
		return 0;

	// If input file was already a STDF file...
	if(!bStdfFileCreated)
		strFileNameSTDF = szInputFile;

	// STDF convertion successful, so now convert to CSV!
	bResult = CsvConvert.Convert(strFileNameSTDF,strFileNameCSV,&ExpirationDate);

	// Delete intermediate STDF file (if created)
	if(bStdfFileCreated)
		unlink(strFileNameSTDF.latin1());

	// If conversion failed...return error code
	if(bResult == false)
		return 0;

	return 1;
}

////////////////////////////////////////////////////////////////////////////////
// Function gexBuildReport: Generates report (data analysis)
// Call : gexBuildReport(bool ShowReport);
////////////////////////////////////////////////////////////////////////////////
static ZString gexBuildReport(ZCsl* csl)
{
	char	*szShowPage;	// Report page to show once report ready

	// Number of arguments
	int argc = csl->get("argCount").asInt();

	// If a list of Html sections NOT to create is given, get list of flags...
	if(argc <= 0)
		szShowPage = "home";	// if no argument, assume to show HOME page
	else
		szShowPage = (char*) csl->get("show_page").constBuffer();

	if(argc >= 2)
	{
		//	List of HTML sections NOT to create (used when multi-pass involved such as for 'guard-banding'
		int iHtmlSkip = csl->get("sections_ignored").asInt();
		ReportOptions.iHtmlSectionsToSkip = iHtmlSkip;
	}

	// Get report created (some sections may not be created...depends of 'iHtmlSkip')
	gexReport->BuildReport(&ReportOptions);

	ShowReportOnCompletion(szShowPage);

	// If ExaminatorWeb running, update the reports table
	gexUpdateWebReportsList();

	return 1;
}

////////////////////////////////////////////////////////////////////////////////
// ExmaminatorWeb ONLY.
// Function gexWeb: Let's user perform ExaminatorWeb management functions
// Call : gexWeb(action, field, value);
////////////////////////////////////////////////////////////////////////////////
static ZString gexWeb(ZCsl* csl)
{
	char	*szAction;		// Action to perform
	char	*szField="";	// Field name 
	char	*szValue="";	// Field value
	bool	bStatus;

	// If NOT ExaminatorWEB, return!
	if(ReportOptions.lProductID != GEX_DATATYPE_ALLOWED_DATABASEWEB)
		return 1;	// Silent return.

	// Get number of parameters given
	int argc = csl->get("argCount").asInt();

	szAction = (char*) csl->get("action").constBuffer();
	if(argc >= 2)
		szField = (char*) csl->get("field").constBuffer();
	if(argc >= 3)
		szValue = (char*) csl->get("value").constBuffer();

	// Import all files in $home/<szValue> folder
	if(!qstricmp(szAction,"db_import"))
	{
		// Build full path to the import folder (non-recursive: meaning only files in this folder are imported, not the one in sub-folders).
		QStringList strCorruptedFiles,strProcessedFiles;
		QString strShowPage;
		QString strFolder = pGexMainWindow->strExaminatorWebUserHome + "/";
		strFolder += szValue;
		
		// Call import method. ImportFolder(DatabaseLogicName,ImportPath,bRecursive,bDeleteAfterImport)
		QString strErrorMessage;
		bStatus = pGexMainWindow->pDatabaseCenter->ImportFolder(szField,strFolder,false,strProcessedFiles,&strCorruptedFiles,strErrorMessage,true);

		if(bStatus == true)
		{
			// Import SUCCESSFUL
			strShowPage = pGexMainWindow->strExaminatorWebUserHome;
			strShowPage += GEX_HELP_FOLDER;
			strShowPage += GEX_HTMLPAGE_WEB_IMPORT;

			// Update the ASP/HTML page that lists the databases+size
			gexUpdateWebDatabasesList();
		}
		else
		{
			// Import FAILURES: Create HTML report in the $home/user
			strShowPage = pGexMainWindow->strExaminatorWebUserHome;
			strShowPage += GEX_DATABASE_WEB_EXCHANGE;
			strShowPage += GEX_HTMLPAGE_WEB_FAILIMPORT;
			
			pGexMainWindow->cGexWeb->htmlImportFailure(strShowPage,&strCorruptedFiles);
		}

		gexReport->strReportName = strShowPage;
		// Have it shown when script is executed!
		ShowReportOnCompletion("real_html");
		return 1;	// No error.
	}

	// Rebuild databases HTML admin page
	if(!qstricmp(szAction,"db_update_html"))
	{
		// Update the ASP/HTML page that lists the databases available + return its name
		QString strShowPage = gexUpdateWebDatabasesList();

		gexReport->strReportName = strShowPage;
		// Have it shown when script is executed!
		ShowReportOnCompletion("real_html");
		return 1;	// No error.
	}

	// Rebuild reports HTML admin page
	if(!qstricmp(szAction,"rpt_update_html"))
	{
		// Update the ASP/HTML page that lists the reports available + return its name
		QString strShowPage = gexUpdateWebReportsList();

		gexReport->strReportName = strShowPage;
		// Have it shown when script is executed!
		ShowReportOnCompletion("real_html");
		return 1;	// No error.
	}

	// Delete specified report+update Reports table HTML page.
	if(!qstricmp(szAction,"rpt_delete"))
	{
		// Normalise report name to report folder name
		CGexSystemUtils	clSysUtils;
		QString strReportName = szField;
		clSysUtils.NormalizeString(strReportName);

		// Build path to report folder to erase.
		QString strReportPath = pGexMainWindow->strExaminatorWebUserHome;
		strReportPath += GEX_DATABASE_REPORTS;
		strReportPath += strReportName;

		// Delete folder
		pGexMainWindow->pDatabaseCenter->DeleteFolderContent(strReportPath);
		// Remove logical entry from the XML definition file.
		pGexMainWindow->cGexWeb->RemoveReportEntry(szField);

		// Update the ASP/HTML page that lists the reports available + return its name
		QString strShowPage = gexUpdateWebReportsList();

		gexReport->strReportName = strShowPage;
		// Have it shown when script is executed!
		ShowReportOnCompletion("real_html");

		return 1;	// No error.
	}

	// Zip + download a report entry....for now doesn't do anything!
	if(!qstricmp(szAction,"rpt_zip"))
	{
		// Build full path to the HTML page to build
		QString strShowPage = pGexMainWindow->strExaminatorWebUserHome + GEX_DATABASE_WEB_EXCHANGE;
		strShowPage += GEX_HTMLPAGE_WEB_REPORTS;

		gexReport->strReportName = strShowPage;
		// Have it shown when script is executed!
		ShowReportOnCompletion("real_html");

		return 1;	// No error.
	}


	// Create a database entry
	if(!qstricmp(szAction,"db_create"))
	{
		// Fill database creation fields.
		GexDatabaseEntry cDatabaseEntry;
		cDatabaseEntry.bHoldFileCopy = true;
		cDatabaseEntry.bCompressed = false;
		cDatabaseEntry.bLocalDatabase = true;
		cDatabaseEntry.bSummaryOnly = false;
		cDatabaseEntry.bBlackHole = false;
		cDatabaseEntry.bExternal=false;


		cDatabaseEntry.strDatabaseLogicalName = szField;
		cDatabaseEntry.strDatabaseDescription = szValue;
        pGexMainWindow->pDatabaseCenter->SaveDatabaseEntry(&cDatabaseEntry);

		// Update the ASP/HTML page that lists the databases
		gexUpdateWebDatabasesList();

		return 1;	// No error.
	}

	// Delete a database entry
	if(!qstricmp(szAction,"db_delete"))
	{
		pGexMainWindow->pDatabaseCenter->DeleteDatabaseEntry(szField);

		// Update the ASP/HTML page that lists the databases
		gexUpdateWebDatabasesList();

		return 1;	// No error.
	}


	ZTHROWEXC("invalid parameter :"+ZString(szAction));
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Converts string to lowercase
////////////////////////////////////////////////////////////////////////////////
static void ToLower(char *szString)
{
	// Check if valid pointer
	if(szString == NULL)
		return;

	char *ptChar=szString;
	while(*ptChar)
	{
		if(((*ptChar) >= 'A') && ((*ptChar) <= 'Z'))
			*ptChar = ((*ptChar) + 'a' - 'A');
		ptChar++;
	};
}

////////////////////////////////////////////////////////////////////////////////
// Function gexParameter: allows to preset parameter info (limits, markers,...)
////////////////////////////////////////////////////////////////////////////////
static int gexParameterEntry(QString strTest,QString strName,QString strField,QString strValue,QString strLabel,QString &strError)
{
	if(gexReport == NULL)
		return 1;	// Just in case!

	// Test with custom limits for guard banding analysis.
	CTestCustomize* pTest;
	pTest = new CTestCustomize();

	// Save test name
	pTest->strTestName = strName;

	// bLimitWhatIfFlag: bit0=1 (no low limit forced), bit1=1 (no high limit forced)
	pTest->bLimitWhatIfFlag = CTEST_LIMITFLG_NOLTL | CTEST_LIMITFLG_NOHTL;

	// List of tests in this group.
	if(sscanf(strTest.latin1(),"%d%*c%d",&(pTest->lTestNumber),&(pTest->lPinmapIndex)) < 2)
		pTest->lPinmapIndex= GEX_PTEST;

	if(strField.endsWith("Marker",false))
	{
		// Marker value is in format: <Marker_Position> <line_width> <R G B color> <layer#>
		// eg: '10.5 2 255 0 0'
		// note: if RGB colrs are - &-1 -1, then color to be used is same as layer charting color
		double lfPos;
		int	iLine,iR,iG,iB,iLayer=-1;
		if(strValue.startsWith("-1.#IND",false) || strValue.startsWith("-1.#INF",false))
			return 1;	// Invalid marker, ignore it

		if(sscanf(strValue.latin1(),"%lf %d %d %d %d %d",&lfPos,&iLine,&iR,&iG,&iB,&iLayer) < 5)
		{
			strError = "Marker 'value' syntax error";
			return -1;
		}
		pTest->ptMarker = new CTestMarker();
		pTest->ptMarker->lfPos = lfPos;
		pTest->ptMarker->iLine = iLine;
		pTest->ptMarker->cColor = QColor(iR,iG,iB);
		pTest->ptMarker->strLabel = strLabel;
		pTest->ptMarker->iLayer = iLayer;
	}
	else
	if(strField.endsWith("Viewport",false))
	{
		// Viweport value is in format: <LowX> <HighX> [<LowY> <HighY>]
		// eg: '10.5 50.3 -5 -2'
		pTest->ptChartOptions = new CGexTestChartingOptions();

		int iParams = sscanf(strValue.latin1(),"%lf %lf %lf %lf",
			&pTest->ptChartOptions->lfLowX,&pTest->ptChartOptions->lfHighX,
			&pTest->ptChartOptions->lfLowY,&pTest->ptChartOptions->lfHighY);
		
		// Check if valid number of parameters specified.
		if(iParams != 2 &&  iParams != 4)
		{
			strError = "Viewport 'value' syntax error";
			return -1;
		}

		// Set flag 'valid X view port'
		pTest->ptChartOptions->bCustomViewPortX = true;

		// If viewport includes Y scales (for scatter plot only), then save info
		if(iParams == 4)
			pTest->ptChartOptions->bCustomViewPortY = true;
		else
			pTest->ptChartOptions->bCustomViewPortY = false;
	}
	else
	{
		strError = "invalid parameter :" + strField;
		return -1;
	}

	// Add this test to the list of 'test with custom limits/markers,...'
	gexReport->pTestCustomFieldsList.append(pTest);

	return 1;
}

////////////////////////////////////////////////////////////////////////////////
// Function gexParameter: allows to preset parameter info (limits, markers,...)
////////////////////////////////////////////////////////////////////////////////
static ZString gexParameter(ZCsl* csl)
{
	char	*szTest;	// Test# eg: 10.35
	char	*szName;	// Test name
	char	*szField;	// Field. eg: LL or HL, or marker, or viewport
	char	*szValue;	// Field value (eg: limit, or marker color & width)
	char	*szLabel;	// Field label/comment. E.g: maker name if field = 'marker'
	QString	strErrorMsg;

	szTest = (char*) csl->get("parameter").constBuffer();
	szName = (char*) csl->get("name").constBuffer();
	szField = (char*) csl->get("field").constBuffer();
	szValue = (char*) csl->get("value").constBuffer();
	szLabel = (char*) csl->get("label").constBuffer();

	if(gexParameterEntry(szTest,szName,szField,szValue,szLabel,strErrorMsg) < 0)
		ZTHROWEXC(ZString(strErrorMsg.latin1()));

	return 1;
}

////////////////////////////////////////////////////////////////////////////////
// Function gexParameterFile: allows to preset a list of parameter info (limits, markers,...)
// listed in a file
////////////////////////////////////////////////////////////////////////////////
static ZString gexParameterFile(ZCsl* csl)
{
	QString	strString;
	char	*szFileName;	// File name where parameters are listed

	// Get file name
	szFileName = (char*) csl->get("parameter").constBuffer();

	// Check file exists
	if(QFile(szFileName).exists() == false)
		ZTHROWEXC("File doesn't exist :"+ZString(szFileName));

	// Open file and parse it.
    QFile f(szFileName);
    if(!f.open( IO_ReadOnly ))
	{
		strString = "Failed reading file:" + QString(szFileName);
		ZTHROWEXC(ZString(strString.latin1()));
	}

	// Assign file I/O stream
	QString strTestNumber,strTestName,strField,strValue,strLabel,strErrorMsg;
	QTextStream hFile(&f);
	do
	{
		// Read line.
		strString = hFile.readLine();
		strString = strString.stripWhiteSpace();

		// Check if comment line
		if(strString.startsWith("#"))
		{
			// comment line...simply do nothing! Ignore this line!
		}
		else
		if(strString.startsWith("gexParameter"))
		{
			// Parse 'gexParameter' line!
			// Format:   gexParameter('14798','iddq_dev_flash_tst_vCEG PstStrs:92 1','Marker','6.76033e-006 2 255 0 255 1','+N');
			strTestNumber = strString.section('\'',1,1);
			strTestName = strString.section('\'',3,3);
			strField = strString.section('\'',5,5);
			strValue = strString.section('\'',7,7);
			strLabel = strString.section('\'',9,9);

			if(gexParameterEntry(strTestNumber,strTestName,strField,strValue,strLabel,strErrorMsg) < 0)
				ZTHROWEXC(ZString(strErrorMsg.latin1()));

		}
	}
	while(hFile.atEnd() == false);
	return 1;
}

////////////////////////////////////////////////////////////////////////////////
// Function gexToolbox: Toolbox functions 'eg: STDF Dump to ASCII, etc...)
////////////////////////////////////////////////////////////////////////////////
static ZString gexToolbox(ZCsl* csl)
{
	char	*szAction="";	// Toolbox action
	char	*szField="";	// Field. eg: 'name'
	char	*szValue="";	// Field value (eg: 'c:/files/file.stdf')
	char	*szOptional="";	// Optional Field value (eg: 'c:/output-folder')

	// Only valid under Examinator Production, Professional edition.
 	if(checkProfessionalFeature(true) == false)
		return 0;	

	// Create object
	if(ptStdfToAscii == NULL)
	{
		ptStdfToAscii = new CSTDFtoASCII(false);
		// Clear list of input files
		m_pInputFiles.clear();
		// Default: dump ALL records
		ptStdfToAscii->SetProcessRecord(true);
	}

	int argc = csl->get("argCount").asInt();
	szAction = (char*) csl->get("action").constBuffer();
	szField = (char*) csl->get("field").constBuffer();
	if(argc >= 3)
		szValue = (char*) csl->get("value").constBuffer();
	if(argc >= 4)
		szOptional = (char*) csl->get("optional").constBuffer();

	if(!qstricmp(szAction,"stdf_dump"))
	{
		if(!qstricmp(szField,"records"))
		{
			// Define STDF records to dump
			if(!qstricmp(szValue,"clear"))
			{
				// Clear all records
				ptStdfToAscii->SetProcessRecord(false);
				return 1;
			}
			if(!qstricmp(szValue,"all"))
			{
				// Set all records
				ptStdfToAscii->SetProcessRecord(true);
				return 1;
			}
			if(!qstricmp(szValue,"ATR"))
			{
				// Dump of ATR records
				ptStdfToAscii->SetProcessRecord(CStdf_Record_V4::Rec_ATR);
				return 1;
			}
			if(!qstricmp(szValue,"BPS"))
			{
				// Dump of BPS records
				ptStdfToAscii->SetProcessRecord(CStdf_Record_V4::Rec_BPS);
				return 1;
			}
			if(!qstricmp(szValue,"DTR"))
			{
				// Dump of DTR records
				ptStdfToAscii->SetProcessRecord(CStdf_Record_V4::Rec_DTR);
				return 1;
			}
			if(!qstricmp(szValue,"EPS"))
			{
				// Dump of EPS records
				ptStdfToAscii->SetProcessRecord(CStdf_Record_V4::Rec_EPS);
				return 1;
			}
			if(!qstricmp(szValue,"FAR"))
			{
				// Dump of FAR records
				ptStdfToAscii->SetProcessRecord(CStdf_Record_V4::Rec_FAR);
				return 1;
			}
			if(!qstricmp(szValue,"FTR"))
			{
				// Dump of FTR records
				ptStdfToAscii->SetProcessRecord(CStdf_Record_V4::Rec_FTR);
				return 1;
			}
			if(!qstricmp(szValue,"GDR"))
			{
				// Dump of GDR records
				ptStdfToAscii->SetProcessRecord(CStdf_Record_V4::Rec_GDR);
				return 1;
			}
			if(!qstricmp(szValue,"HBR"))
			{
				// Dump of HBR records
				ptStdfToAscii->SetProcessRecord(CStdf_Record_V4::Rec_HBR);
				return 1;
			}
			if(!qstricmp(szValue,"MIR"))
			{
				// Dump of MIR records
				ptStdfToAscii->SetProcessRecord(CStdf_Record_V4::Rec_MIR);
				return 1;
			}
			if(!qstricmp(szValue,"MPR"))
			{
				// Dump of MPR records
				ptStdfToAscii->SetProcessRecord(CStdf_Record_V4::Rec_MPR);
				return 1;
			}
			if(!qstricmp(szValue,"MRR"))
			{
				// Dump of MRR records
				ptStdfToAscii->SetProcessRecord(CStdf_Record_V4::Rec_MRR);
				return 1;
			}
			if(!qstricmp(szValue,"PCR"))
			{
				// Dump of PCR records
				ptStdfToAscii->SetProcessRecord(CStdf_Record_V4::Rec_PCR);
				return 1;
			}
			if(!qstricmp(szValue,"PGR"))
			{
				// Dump of PGR records
				ptStdfToAscii->SetProcessRecord(CStdf_Record_V4::Rec_PGR);
				return 1;
			}
			if(!qstricmp(szValue,"PIR"))
			{
				// Dump of PIR records
				ptStdfToAscii->SetProcessRecord(CStdf_Record_V4::Rec_PIR);
				return 1;
			}
			if(!qstricmp(szValue,"PLR"))
			{
				// Dump of PLR records
				ptStdfToAscii->SetProcessRecord(CStdf_Record_V4::Rec_PLR);
				return 1;
			}
			if(!qstricmp(szValue,"PMR"))
			{
				// Dump of PMR records
				ptStdfToAscii->SetProcessRecord(CStdf_Record_V4::Rec_PMR);
				return 1;
			}
			if(!qstricmp(szValue,"PRR"))
			{
				// Dump of PRR records
				ptStdfToAscii->SetProcessRecord(CStdf_Record_V4::Rec_PRR);
				return 1;
			}
			if(!qstricmp(szValue,"PTR"))
			{
				// Dump of PTR records
				ptStdfToAscii->SetProcessRecord(CStdf_Record_V4::Rec_PTR);
				return 1;
			}
			if(!qstricmp(szValue,"RDR"))
			{
				// Dump of RDR records
				ptStdfToAscii->SetProcessRecord(CStdf_Record_V4::Rec_RDR);
				return 1;
			}
			if(!qstricmp(szValue,"SBR"))
			{
				// Dump of SBR records
				ptStdfToAscii->SetProcessRecord(CStdf_Record_V4::Rec_SBR);
				return 1;
			}
			if(!qstricmp(szValue,"SDR"))
			{
				// Dump of SDR records
				ptStdfToAscii->SetProcessRecord(CStdf_Record_V4::Rec_SDR);
				return 1;
			}
			if(!qstricmp(szValue,"TSR"))
			{
				// Dump of TSR records
				ptStdfToAscii->SetProcessRecord(CStdf_Record_V4::Rec_TSR);
				return 1;
			}
			if(!qstricmp(szValue,"WCR"))
			{
				// Dump of WCR records
				ptStdfToAscii->SetProcessRecord(CStdf_Record_V4::Rec_WCR);
				return 1;
			}
			if(!qstricmp(szValue,"WIR"))
			{
				// Dump of WIR records
				ptStdfToAscii->SetProcessRecord(CStdf_Record_V4::Rec_WIR);
				return 1;
			}
			if(!qstricmp(szValue,"WRR"))
			{
				// Dump of WRR records
				ptStdfToAscii->SetProcessRecord(CStdf_Record_V4::Rec_WRR);
				return 1;
			}
			ZTHROWEXC("invalid record field :"+ZString(szField));
		}
		else
		if(!qstricmp(szField,"style"))
		{
			// DUMP header will display the style selected
			ptStdfToAscii->SetSizeFileLimitOptionText(szValue);

			// Define dump style
			if(!qstricmp(szValue,"short"))
			{
				// Dump in the short form: only record names
				ptStdfToAscii->SetFieldFilter(CStdf_Record_V4::FieldFlag_None);
				return 1;
			}
			if(!qstricmp(szValue,"detailed"))
			{
				// Dump in the detailed form: list all record fields
				ptStdfToAscii->SetFieldFilter(CStdf_Record_V4::FieldFlag_Present);
				return 1;
			}
			ZTHROWEXC("invalid style :"+ZString(szField));
		}
		else
		if(!qstricmp(szField,"size"))
		{
			// Define maximum dump size (0=no limit)
			long	ldSize;
			if(sscanf(szValue,"%ld",&ldSize) != 1)
				ldSize = 0;
			ptStdfToAscii->SetSizeFileLimit(ldSize);
		}
		else
		if(!qstricmp(szField,"consecutive"))
		{
			// Dump consecutive records of same type
			if(!qstricmp(szValue,"yes"))
			{
				// Dump in the short form: only record names
				ptStdfToAscii->SetConsecutiveRecordOption(true);
				return 1;
			}
			if(!qstricmp(szValue,"no"))
			{
				//: DO NOT dump consecutive records of same type
				ptStdfToAscii->SetConsecutiveRecordOption(false);
				return 1;
			}
			ZTHROWEXC("invalid value :"+ZString(szField));
		}
		else
		if(!qstricmp(szField,"file"))
		{
			// Add STDF name to the dump list
			m_pInputFiles.Append(szValue,szOptional);
			return 1;
		}
		else
		if(!qstricmp(szField,"dump"))
		{
			// Trigger dump action now!
			QString strError;
			ExportAscii_FileInfo	*pclFileInfo = m_pInputFiles.FirstStdf(ExportAscii_FileInfo::FileType_STDF_V4);

			// If no file dumped, tell it!
			if(pclFileInfo == NULL)
			{
				strError = "'stdf_dump': No file dumped. Invalid input format?";
				WriteScriptMessage(strError,true);	// Text + EOL
			} 
			
			while(pclFileInfo)
			{
				// Convert files			
				if(pclFileInfo->Convert(*ptStdfToAscii,"",&ExpirationDate) != TRUE)
				{
					// Error Dumping file, display file & error
					ptStdfToAscii->GetLastError(strError);
					QString strMessage;
					strMessage.sprintf("Error dumping STDF file to ASCII.\nFile: %s", strError.latin1());
				   // Send error message to Script Window.
				   WriteScriptMessage(strMessage,true);	// Text + EOL
				}

				pclFileInfo = m_pInputFiles.NextStdf(ExportAscii_FileInfo::FileType_STDF_V4);
			}

			// Clear list of input files
			m_pInputFiles.clear();

			// Delete Dump object.
			delete ptStdfToAscii;
			ptStdfToAscii = NULL;

			return 1;
		}
		ZTHROWEXC("invalid field :"+ZString(szField));

	}
	ZTHROWEXC("invalid action :"+ZString(szAction));

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Function gexFavoriteScripts: Let's user define the GEX list of 'Favorite Scripts'
// Call : gexFavoriteScripts(action [,path, title);
////////////////////////////////////////////////////////////////////////////////
static ZString gexFavoriteScripts(ZCsl* csl)
{
	char	*szAction;	// Action: 'clear' or 'insert'
	char	*szPath;	// Script path
	char	*szTitle;	// Script description Title

	int argc = csl->get("argCount").asInt();

	switch (argc) 
	{
      case 1:
			// If only one argument, must be 'clear'
			szAction = (char*) csl->get("action").constBuffer();
			if(!qstricmp(szAction,"clear"))
			{
				EraseFavoriteList();
				return 1;	// No error.
			}
			ZTHROWEXC("invalid parameter :"+ZString(szAction));
		break;

      case 3:
			// If only one argument, must be 'insert'
			szAction = (char*) csl->get("action").constBuffer();
			szPath   = (char*) csl->get("path").constBuffer();
			szTitle  = (char*) csl->get("title").constBuffer();
			if(!qstricmp(szAction,"insert"))
			{
				InsertFavoriteList(szPath,szTitle);
				return 1;	// No error.
			}
			ZTHROWEXC("invalid parameter :"+ZString(szAction));
			break;

      default:
			ZTHROWEXC("invalid number of parameters");
			break;
   } // switch
	return 0;
} // gexFavoriteScripts

////////////////////////////////////////////////////////////////////////////////
// Function gexFile: Add file to group.
// Call : gexFile(group_id, action, file, site, process [,range,...]);
////////////////////////////////////////////////////////////////////////////////
static ZString gexFile(ZCsl* csl)
{
	long	lGroupID;	// Group_ID to which this file will belong
	char	*szAction;	// Action
	char	*szFile;	// STDF file to process
	char	*szSite;	// Site(s) to process
	char	*szProcess; // Type of parts to process.
	char	*szRangeList;	// Range of parts/bins to process
	char	*szMapTests="";	// .CSV Test number mapping file
	char	*szWaferToExtract=""; // WaferID to extract.
	double	lfTemperature=-1000;	// Testing temperature (if applicable).
	char	*szDatasetName=""; // Dataset name if any associated with file...
	ZString zString;

	int		iProcessSite;
	int		iProcessBins=0;

	lGroupID = (long) csl->get("group_id").asLong();
	szAction = (char*) csl->get("action").constBuffer();
	szFile = (char*) csl->get("file").constBuffer();
	szSite = (char*) csl->get("site").constBuffer();
	szProcess = (char*) csl->get("process").constBuffer();

	if(qstricmp(szAction,"insert"))
		ZTHROWEXC("invalid parameter :"+ZString(szAction));

	// Check sites to process
	if(!qstricmp(szSite,"all"))
		iProcessSite = -1;	// Will process ALL sites
	else
	{
		// Check which specific site# is given
		iProcessSite = (long) csl->get("site").asLong();
	}

	// Check for type of parts to process
	if(!qstricmp(szProcess,"all"))
		iProcessBins = GEX_PROCESSPART_ALL;
	else
	if(!qstricmp(szProcess,"allparts_except"))
		iProcessBins = GEX_PROCESSPART_EXPARTLIST;
	else
	if(!qstricmp(szProcess,"good"))
		iProcessBins = GEX_PROCESSPART_GOOD;
	else
	if(!qstricmp(szProcess,"fails"))
		iProcessBins = GEX_PROCESSPART_FAIL;
	else
	if(!qstricmp(szProcess,"parts"))
		iProcessBins = GEX_PROCESSPART_PARTLIST;
	else
	if(!qstricmp(szProcess,"bins"))
		iProcessBins = GEX_PROCESSPART_SBINLIST;
	else
	if(!qstricmp(szProcess,"allbins_except"))
		iProcessBins = GEX_PROCESSPART_EXSBINLIST;
	else
	if(!qstricmp(szProcess,"hbins"))
		iProcessBins = GEX_PROCESSPART_HBINLIST;
	else
	if(!qstricmp(szProcess,"allhbins_except"))
		iProcessBins = GEX_PROCESSPART_EXHBINLIST;
	else
	if(!qstricmp(szProcess,"odd_parts"))
		iProcessBins = GEX_PROCESSPART_ODD;
	else
	if(!qstricmp(szProcess,"even_parts"))
		iProcessBins = GEX_PROCESSPART_EVEN;
	else
	if(!qstricmp(szProcess,"first_instance"))
		iProcessBins = GEX_PROCESSPART_FIRSTINSTANCE;
	else
	if(!qstricmp(szProcess,"last_instance"))
		iProcessBins = GEX_PROCESSPART_LASTINSTANCE;
	else
	if(!qstricmp(szProcess,"parts_inside"))
		iProcessBins = GEX_PROCESSPART_PARTSINSIDE;
	else
	if(!qstricmp(szProcess,"parts_outside"))
		iProcessBins = GEX_PROCESSPART_PARTSOUTSIDE;
	else
		ZTHROWEXC("invalid parameter :"+ZString(szProcess));

	switch(iProcessBins)
	{
		case GEX_PROCESSPART_EXPARTLIST:
		case GEX_PROCESSPART_PARTLIST:
		case GEX_PROCESSPART_SBINLIST:
		case GEX_PROCESSPART_EXSBINLIST:
		case GEX_PROCESSPART_HBINLIST:
		case GEX_PROCESSPART_EXHBINLIST:
		case GEX_PROCESSPART_PARTSINSIDE:
		case GEX_PROCESSPART_PARTSOUTSIDE:
			// A range of parts-bins must have been specified
			// Check if range specified
			if(csl->get("argCount").asInt() < 6)
				ZTHROWEXC("missing parameter : range");
			szRangeList = (char*) csl->get("range").constBuffer();

			if (iProcessBins == GEX_PROCESSPART_PARTSINSIDE || iProcessBins == GEX_PROCESSPART_PARTSOUTSIDE)
			{
				if (CGexRangeCoord(szRangeList).isValid() == false)
					ZTHROWEXC("invalid range value: " + ZString(szRangeList));
			}
			break;

		default: // Allow any test#
			szRangeList = "0 to 2147483647";	// Test is a 31bits number.
			break;
	}

	// if Test Mapping file defined...
	if(csl->get("argCount").asInt() >= 7)
		szMapTests = (char*) csl->get("maptests").constBuffer();
	// Wafer to extractID defined...
	if(csl->get("argCount").asInt() >= 8)
		szWaferToExtract = (char*) csl->get("extractwafer").constBuffer();
	// Testing temperature if known...
	if(csl->get("argCount").asInt() >= 9)
	{
		char *ptChar = (char*) csl->get("temperature").constBuffer();
		QString strString = ptChar;
		strString = strString.stripWhiteSpace();
		bool	bOkay;
		lfTemperature = strString.toDouble(&bOkay);
		if(!bOkay)
			lfTemperature = -1000;	// If invalid or unknown temperature.
	}
	// Datasetname defined...
	if(csl->get("argCount").asInt() >= 10)
		szDatasetName = (char*) csl->get("datasetname").constBuffer();
	
	// Adds file to given group.
	return gexReport->addFile(lGroupID,szFile,iProcessSite,iProcessBins,szRangeList,szMapTests,szWaferToExtract,lfTemperature,szDatasetName);
}

////////////////////////////////////////////////////////////////////////////////
// Function gexGroup: Reset/Create new group.
// Call : gexGroup(action,value);
////////////////////////////////////////////////////////////////////////////////
static ZString gexGroup(ZCsl* csl)
{
	char	*szAction;	// action type
	char	*szValue;	// Action parameter

	szAction = (char*) csl->get("action").constBuffer();
	szValue = (char*) csl->get("value").constBuffer();

	if(!qstricmp(szAction,"reset"))
	{
		// Delete all existing structures...
		if(gexReport != NULL)
			delete gexReport;	
		gexReport = new CGexReport;
		gexReport->pReportOptions = NULL;	// Will remain NULL unless a report is created...
		
		// Dataset source results from inserting files (not a Query).
		ReportOptions.bQueryDataset = false;

		// Reset query structure
		cQuery.clear();
		return 1;
	}
	else
	if(!qstricmp(szAction,"insert"))
	{
		// Create new group
		int iGroupID = gexReport->addGroup(szValue);
		ZString sGroupID = iGroupID;

		// Dataset source results from inserting files (not a Query).
		ReportOptions.bQueryDataset = false;
		return sGroupID;
	}
	else
	if(!qstricmp(szAction,"insert_query"))
	{
		// Query (list of files to process)
		QStringList sFilesMatchingQuery;
		QString		strFilePath;
		int			iGroupID=0;
		
		// Dataset source results from running a Query.
		ReportOptions.bQueryDataset = true;

		// Get database entry
		GexDatabaseEntry *pDatabaseEntry = pGexMainWindow->pDatabaseCenter->FindDatabaseEntry(cQuery.strDatabaseLogicalName);
		if(pDatabaseEntry == NULL)
		{
			ZTHROWEXC("couldn't find database :"+ZString(cQuery.strDatabaseLogicalName.latin1()));
			return 0;
		}
		
		// Check if the dataset has to be split
		QStringList strlSplitValues;
		if(cQuery.strSplitField.isEmpty())
			strlSplitValues.append("*");
		else
		{
			// Check if RDB database, or file-based database
			if(pDatabaseEntry->bExternal && !cQuery.bOfflineQuery)
			{
				// RDB database
				strlSplitValues = pGexMainWindow->pDatabaseCenter->QuerySelectFilter(&cQuery, cQuery.strSplitField);
				if(strlSplitValues.count() == 0)
					strlSplitValues.append("*");
			}
			else
			{
				// File-based database: retrieve individual values for the split field
				strlSplitValues = pGexMainWindow->pDatabaseCenter->QuerySelectFilter(&cQuery, cQuery.strSplitField);
				if(strlSplitValues.count() == 0)
					strlSplitValues.append("*");
			}
		}

		QStringList::ConstIterator	itSplits, itFiles;
		QString						strFilter, strDatasetName = szValue;
		QStringList					strlSqlFilters = cQuery.strlSqlFilters;	// Save SQL filters
		for(itSplits = strlSplitValues.begin(); itSplits != strlSplitValues.end(); itSplits++)
		{
			// Check if this is a dataset split
			if((*itSplits) != "*")
			{
				// Add filter for dataset split
				if(pDatabaseEntry->bExternal && !cQuery.bOfflineQuery)
				{
					// RDB database
					strFilter = cQuery.strSplitField;
					strFilter += "=";
					strFilter += (*itSplits);
					cQuery.strlSqlFilters = strlSqlFilters;		// Restore SQL filters
					cQuery.strlSqlFilters.append(strFilter);	// Add current split filter
					strDatasetName = szValue;
					strDatasetName += "[";
					strDatasetName += cQuery.strSplitField;
					strDatasetName += "=";
					strDatasetName += (*itSplits);
					strDatasetName += "]";
				}
				else
				{
					// File-based database: add filter for this dataset split
					setQueryFilter((char *)cQuery.strSplitField.latin1(),(char *)(*itSplits).latin1());
					strDatasetName = szValue;
					strDatasetName += "[";
					strDatasetName += cQuery.strSplitField;
					strDatasetName += "=";
					strDatasetName += (*itSplits);
					strDatasetName += "]";
				}
			}

			// Find all files matching query into the group.
			sFilesMatchingQuery = pGexMainWindow->pDatabaseCenter->QuerySelectFiles(&cQuery);
					
			// 1: Compute total size of files in query (so to see if use Summary instead of data files!).
			QFile	cFile;
			cQuery.lfQueryFileSize = 0;
			for (itFiles = sFilesMatchingQuery.begin(); itFiles != sFilesMatchingQuery.end(); ++itFiles ) 
			{
				// Adds file to given group.
				strFilePath = (*itFiles);	// Absolute path to the file...

				// Check if file exists...if not, probably it is compressed (then add .gz extension)
				if(QFile::exists(strFilePath) == false)
					strFilePath += ".gz";

				cFile.setName(strFilePath);
				cQuery.lfQueryFileSize += (double) cFile.size();
			}

			// Create group to hold ALL files matching the query criteria
			cQuery.strTitle = strDatasetName;	// Query title.
			iGroupID= gexReport->addGroup(strDatasetName,&cQuery);

			// 2: Add all files of the query to the group.
			for (itFiles = sFilesMatchingQuery.begin(); itFiles != sFilesMatchingQuery.end(); ++itFiles ) 
			{
				// Adds file to given group.
				strFilePath = (*itFiles);	// Absolute path to the file...

				// Check if file exists...if not, probably it is compressed (then add .gz extension)...unless 'Summary' database
				if((QFile::exists(strFilePath) == false) && (cQuery.bSummaryOnly == false))
					strFilePath += ".gz";

				// Add file to group (if it exists!)
				if(QFile::exists(strFilePath) || cQuery.bSummaryOnly)
				{
					gexReport->addFile(iGroupID,
						strFilePath,
						cQuery.iSite,
						cQuery.iProcessData,
						(char*)cQuery.strProcessData.latin1(),
						cQuery.strMapFile,
						cQuery.strWaferID);
				}
			}
		}

		// Reset query structure
		cQuery.clear();

		// Return groupID		
		ZString sGroupID = iGroupID;
		return sGroupID;
	}
	else
		ZTHROWEXC("invalid parameter :"+ZString(szAction));
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
//------- Extract FILTER name + value  --------
////////////////////////////////////////////////////////////////////////////////
static int setQueryFilter(char *szSection, char *szField)
{
	// Burning time filter
	if(!qstricmp(szSection,"dbf_burnin_time"))
	{
		sscanf(szField,"%d",&cQuery.iBurninTime);
		cQuery.uFilterFlag |= GEX_QUERY_FLAG_BURNIN;
		return 1;
	}
	// Data origin filter
	if(!qstricmp(szSection,"dbf_data_origin"))
	{
		cQuery.strDataOrigin = szField;
		cQuery.uFilterFlag |= GEX_QUERY_FLAG_ORIGIN;
		return 1;
	}
	// Facility location filter
	if(!qstricmp(szSection,"dbf_facility_id"))
	{
		cQuery.strFacilityID = szField;
		cQuery.uFilterFlag |= GEX_QUERY_FLAG_FACILITY;
		return 1;
	}
	// FamilyID filter
	if(!qstricmp(szSection,"dbf_family_id"))
	{
		cQuery.strFamilyID = szField;
		cQuery.uFilterFlag |= GEX_QUERY_FLAG_FAMILY;
		return 1;
	}
	// Floor Location filter
	if(!qstricmp(szSection,"dbf_floor_id"))
	{
		cQuery.strFloorID = szField;
		cQuery.uFilterFlag |= GEX_QUERY_FLAG_FLOOR;
		return 1;
	}
	// Testing/step frequency filter
	if(!qstricmp(szSection,"dbf_freq_id"))
	{
		cQuery.strFrequencyStep = szField;
		cQuery.uFilterFlag |= GEX_QUERY_FLAG_FREQUENCYSTEP;
		return 1;
	}
	// LoadBoard name filter
	if(!qstricmp(szSection,"dbf_loadboard_id"))
	{
		cQuery.strLoadBoardName = szField;
		cQuery.uFilterFlag |= GEX_QUERY_FLAG_LOADBOARDNAME;
		return 1;
	}
	// LoadBoard type (brand/family) filter
	if(!qstricmp(szSection,"dbf_loadboard_type"))
	{
		cQuery.strLoadBoardType = szField;
		cQuery.uFilterFlag |= GEX_QUERY_FLAG_LOADBOARDTYPE;
		return 1;
	}
	// DIB name filter
	if(!qstricmp(szSection,"dbf_dib_id"))
	{
		cQuery.strDibName = szField;
		cQuery.uFilterFlag |= GEX_QUERY_FLAG_DIBNAME;
		return 1;
	}
	// DIB type (brand/family) filter
	if(!qstricmp(szSection,"dbf_dib_type"))
	{
		cQuery.strDibType = szField;
		cQuery.uFilterFlag |= GEX_QUERY_FLAG_DIBTYPE;
		return 1;
	}
	// Lot ID filter
	if(!qstricmp(szSection,"dbf_lot_id"))
	{
		cQuery.strLotID = szField;
		cQuery.uFilterFlag |= GEX_QUERY_FLAG_LOT;
		return 1;
	}
	// Operator name filter
	if(!qstricmp(szSection,"dbf_operator_name"))
	{
		cQuery.strOperator = szField;
		cQuery.uFilterFlag |= GEX_QUERY_FLAG_OPERATOR;
		return 1;
	}
	// Package type filter
	if(!qstricmp(szSection,"dbf_package_type"))
	{
		cQuery.strPackageType = szField;
		cQuery.uFilterFlag |= GEX_QUERY_FLAG_PACKAGE;
		return 1;
	}
	// Prober name filter
	if(!qstricmp(szSection,"dbf_prober_id"))
	{
		cQuery.strProberName = szField;
		cQuery.uFilterFlag |= GEX_QUERY_FLAG_PROBERNAME;
		return 1;
	}
	// Prober type (brand) filter
	if(!qstricmp(szSection,"dbf_prober_type"))
	{
		cQuery.strProberType = szField;
		cQuery.uFilterFlag |= GEX_QUERY_FLAG_PROBERTYPE;
		return 1;
	}
	// Product name filter
	if(!qstricmp(szSection,"dbf_product_id"))
	{
		cQuery.strProductID = szField;
		cQuery.uFilterFlag |= GEX_QUERY_FLAG_PRODUCT;
		return 1;
	}
	// Program name filter
	if(!qstricmp(szSection,"dbf_program_name"))
	{
		cQuery.strJobName = szField;
		cQuery.uFilterFlag |= GEX_QUERY_FLAG_PROGRAMNAME;
		return 1;
	}
	// Program revision filter
	if(!qstricmp(szSection,"dbf_program_rev"))
	{
		cQuery.strJobRev = szField;
		cQuery.uFilterFlag |= GEX_QUERY_FLAG_PROGRAMREVISION;
		return 1;
	}
	// Sublot ID filter
	if(!qstricmp(szSection,"dbf_retest_nbr"))
	{
		cQuery.strRetestNbr = szField;
		cQuery.uFilterFlag |= GEX_QUERY_FLAG_RETESTNBR;
		return 1;
	}
	// Sublot ID filter
	if(!qstricmp(szSection,"dbf_sublot_id"))
	{
		cQuery.strSubLotID = szField;
		cQuery.uFilterFlag |= GEX_QUERY_FLAG_SUBLOT;
		return 1;
	}
	// Testing temperature filter
	if(!qstricmp(szSection,"dbf_temperature"))
	{
		cQuery.strTemperature = szField;
		cQuery.uFilterFlag |= GEX_QUERY_FLAG_TEMPERATURE;
		return 1;
	}
	// Tester name filter
	if(!qstricmp(szSection,"dbf_tester_name"))
	{
		cQuery.strNodeName = szField;
		cQuery.uFilterFlag |= GEX_QUERY_FLAG_TESTERNAME;
		return 1;
	}
	// Tester type (brand/family) filter
	if(!qstricmp(szSection,"dbf_tester_type"))
	{
		cQuery.strTesterType = szField;
		cQuery.uFilterFlag |= GEX_QUERY_FLAG_TESTERTYPE;
		return 1;
	}
	// Testing code filter
	if(!qstricmp(szSection,"dbf_testing_code"))
	{
		cQuery.strTestingCode = szField;
		cQuery.uFilterFlag |= GEX_QUERY_FLAG_TESTCODE;
		return 1;
	}
	// Process name filter
	if(!qstricmp(szSection,"dbf_process_id"))
	{
		cQuery.strProcessID = szField;
		cQuery.uFilterFlag |= GEX_QUERY_FLAG_PROCESS;
		return 1;
	}
	// Tester Hardware testing site# filter
	if(!qstricmp(szSection,"dbf_site_nbr"))
	{
		cQuery.iSite = -1;
		sscanf(szField,"%d",&cQuery.iSite);
		cQuery.uFilterFlag |= GEX_QUERY_FLAG_SITENBR;
		return 1;
	}

	// WaferID name filter
	if(!qstricmp(szSection,"dbf_wafer_id"))
	{
		cQuery.strWaferID = szField;
		cQuery.uFilterFlag |= GEX_QUERY_FLAG_WAFERID;
		return 1;
	}

	// User1 name filter
	if(!qstricmp(szSection,"dbf_user_1"))
	{
		cQuery.strUser1 = szField;
		cQuery.uFilterFlag |= GEX_QUERY_FLAG_USER1;
		return 1;
	}

	// User2 name filter
	if(!qstricmp(szSection,"dbf_user_2"))
	{
		cQuery.strUser2 = szField;
		cQuery.uFilterFlag |= GEX_QUERY_FLAG_USER2;
		return 1;
	}

	// User3 name filter
	if(!qstricmp(szSection,"dbf_user_3"))
	{
		cQuery.strUser3 = szField;
		cQuery.uFilterFlag |= GEX_QUERY_FLAG_USER3;
		return 1;
	}

	// User4 name filter
	if(!qstricmp(szSection,"dbf_user_4"))
	{
		cQuery.strUser4 = szField;
		cQuery.uFilterFlag |= GEX_QUERY_FLAG_USER4;
		return 1;
	}

	// User5 name filter
	if(!qstricmp(szSection,"dbf_user_5"))
	{
		cQuery.strUser5 = szField;
		cQuery.uFilterFlag |= GEX_QUERY_FLAG_USER5;
		return 1;
	}

	// Invalid filter name
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Function gexQuery: Let's user define the Query content
// Call : gexQuery(section, field, value);
////////////////////////////////////////////////////////////////////////////////
static ZString gexQuery(ZCsl* csl)
{
	char	*szSection;	// Section name to setup
	char	*szField;	// Field name to setup
	char	*szValue;	// Field value
	int		argc;		// Number of arguments.

	szSection = (char*) csl->get("section").constBuffer();
	szField = (char*) csl->get("field").constBuffer();
	
	argc = csl->get("argCount").asInt();

	// Report folder name (or CSV file name)
	if(!qstricmp(szSection,"db_report"))
	{
		// Save Title string (may include spaces or any other characters).
		QString strTitle = szField;

		// Check if title not empty
		if(strTitle.stripWhiteSpace().isEmpty())
				ZTHROWEXC("empty report name specified in db_report() command");
		
		// Normalized: Filter string to be file-name/folder-name compatible.
		ReportOptions.strReportTitle = strTitle;
		strTitle.replace(QRegExp("[^A-Za-z0-9]"), "_" );
		ReportOptions.strReportNormalizedTitle = strTitle;
		return 1;
	}

	// Database Location
	if(!qstricmp(szSection,"db_location"))
	{
		if(!qstricmp(szField,"[Local]"))
			cQuery.bLocalDatabase = true;
		else
			cQuery.bLocalDatabase = false;
		return 1;
	}

	// Database Logical Name
	if(!qstricmp(szSection,"db_name"))
	{
		cQuery.strDatabaseLogicalName = szField;
		return 1;
	}

	// Data type to focus on in query (wafer sort, final test, e-test...:only applies to remote SQL databases)
	if(!qstricmp(szSection,"db_data_type"))
	{
		cQuery.strDataTypeQuery = szField;
		return 1;
	}

	// Query type: database or offline
	if(!qstricmp(szSection,"db_offline_query"))
	{
		if(!qstricmp(szField,"true"))
			cQuery.bOfflineQuery = true;
		else 
			cQuery.bOfflineQuery = false;
		return 1;
	}

	// Time period to consider
	if(!qstricmp(szSection,"db_period"))
	{
		int iIndex=0;
		do
		{
			if(gexTimePeriodChoices[iIndex] == 0)
				ZTHROWEXC("invalid parameter :"+ZString(szField));
			// Check if Time period keyword found...
			if(!qstricmp(szField,gexTimePeriodChoices[iIndex]))
				break;
			// Not found yet...move to next keyword entry
			iIndex++;
		}
		while(1);
		
		// According to date window type, compute From-To dates
		cQuery.calendarFrom = cQuery.calendarTo = QDate::currentDate();
		cQuery.iTimePeriod = iIndex;
		switch(iIndex)
		{
			case GEX_QUERY_TIMEPERIOD_ALLDATES:
			case GEX_QUERY_TIMEPERIOD_TODAY:
			default:
				break;
			case GEX_QUERY_TIMEPERIOD_LAST2DAYS:
				cQuery.calendarFrom = cQuery.calendarFrom.addDays(-1);
				break;
			case GEX_QUERY_TIMEPERIOD_LAST3DAYS:
				cQuery.calendarFrom = cQuery.calendarFrom.addDays(-2);
				break;
			case GEX_QUERY_TIMEPERIOD_LAST7DAYS:
				cQuery.calendarFrom = cQuery.calendarFrom.addDays(-6);
				break;
			case GEX_QUERY_TIMEPERIOD_LAST14DAYS:
				cQuery.calendarFrom = cQuery.calendarFrom.addDays(-13);
				break;
			case GEX_QUERY_TIMEPERIOD_LAST31DAYS:
				cQuery.calendarFrom = cQuery.calendarFrom.addDays(-30);
				break;
			case GEX_QUERY_TIMEPERIOD_THISWEEK:
				// 'From' date must be last 'Monday' before current date.
				while(cQuery.calendarFrom.dayOfWeek() != 1)
					cQuery.calendarFrom = cQuery.calendarFrom.addDays(-1);
				break;
			case GEX_QUERY_TIMEPERIOD_THISMONTH:
				// 'From' date must be 1st day of the month.
				cQuery.calendarFrom.setYMD(cQuery.calendarFrom.year(),cQuery.calendarFrom.month(),1);
				break;
			case GEX_QUERY_TIMEPERIOD_CALENDAR:
				// Extract <From> <To> dates: "YYYY MM DD YYYY MM DD"
				int iFromYear,iFromMonth,iFromDay;
				int iToYear,iToMonth,iToDay;
				// Check if range specified
				if(argc < 3)
					ZTHROWEXC("missing parameter : date range");
				szValue = (char*) csl->get("value").constBuffer();
				sscanf(szValue,"%d %d %d %d %d %d",
					&iFromYear,&iFromMonth,&iFromDay,
					&iToYear,&iToMonth,&iToDay);
				cQuery.calendarFrom.setYMD(iFromYear,iFromMonth,iFromDay);
				cQuery.calendarTo.setYMD(iToYear,iToMonth,iToDay);
				QStringList strlTokens = QStringList::split(' ', QString(szValue));
				if(strlTokens.count() >= 8)
				{
					int nHour, nMin, nSec;
					nHour = strlTokens[6].section(':',0,0).toUInt();
					nMin = strlTokens[6].section(':',1,1).toUInt();
					nSec = strlTokens[6].section(':',2,2).toUInt();
					cQuery.calendarFrom_Time = QTime(nHour, nMin, nSec);
					nHour = strlTokens[7].section(':',0,0).toUInt();
					nMin = strlTokens[7].section(':',1,1).toUInt();
					nSec = strlTokens[7].section(':',2,2).toUInt();
					cQuery.calendarTo_Time = QTime(nHour, nMin, nSec);
				}
				break;
		}
		return 1;
	}

	// Data type filter
	if(!qstricmp(szSection,"db_data"))
	{
		// Check for type of parts to process
		if(!qstricmp(szField,"all"))
			cQuery.iProcessData = GEX_PROCESSPART_ALL;
		else
		if(!qstricmp(szField,"allparts_except"))
			cQuery.iProcessData = GEX_PROCESSPART_EXPARTLIST;
		else
		if(!qstricmp(szField,"good"))
			cQuery.iProcessData = GEX_PROCESSPART_GOOD;
		else
		if(!qstricmp(szField,"fails"))
			cQuery.iProcessData = GEX_PROCESSPART_FAIL;
		else
		if(!qstricmp(szField,"parts"))
			cQuery.iProcessData = GEX_PROCESSPART_PARTLIST;
		else
		if(!qstricmp(szField,"bins"))
			cQuery.iProcessData = GEX_PROCESSPART_SBINLIST;
		else
		if(!qstricmp(szField,"allbins_except"))
			cQuery.iProcessData = GEX_PROCESSPART_EXSBINLIST;
		else
		if(!qstricmp(szField,"hbins"))
			cQuery.iProcessData = GEX_PROCESSPART_HBINLIST;
		else
		if(!qstricmp(szField,"allhbins_except"))
			cQuery.iProcessData = GEX_PROCESSPART_EXHBINLIST;
		else
		if(!qstricmp(szField,"odd_parts"))
			cQuery.iProcessData = GEX_PROCESSPART_ODD;
		else
		if(!qstricmp(szField,"even_parts"))
			cQuery.iProcessData = GEX_PROCESSPART_EVEN;
		else
		if(!qstricmp(szField,"first_instance"))
			cQuery.iProcessData = GEX_PROCESSPART_FIRSTINSTANCE;
		else
		if(!qstricmp(szField,"last_instance"))
			cQuery.iProcessData = GEX_PROCESSPART_LASTINSTANCE;
		else
		if(!qstricmp(szField,"parts_inside"))
			cQuery.iProcessData = GEX_PROCESSPART_PARTSINSIDE;
		else
		if(!qstricmp(szField,"parts_outside"))
			cQuery.iProcessData = GEX_PROCESSPART_PARTSOUTSIDE;
		else
			ZTHROWEXC("invalid parameter :"+ZString(szField));
		
		switch(cQuery.iProcessData)
		{
			case GEX_PROCESSPART_EXPARTLIST:
			case GEX_PROCESSPART_PARTLIST:
			case GEX_PROCESSPART_SBINLIST:
			case GEX_PROCESSPART_EXSBINLIST:
			case GEX_PROCESSPART_HBINLIST:
			case GEX_PROCESSPART_EXHBINLIST:
			case GEX_PROCESSPART_PARTSINSIDE:
			case GEX_PROCESSPART_PARTSOUTSIDE:
				// A range of parts-bins must have been specified
				// Check if range specified
				if(argc < 3)
					ZTHROWEXC("missing parameter : range");
				szValue = (char*) csl->get("value").constBuffer();
				
				if (cQuery.iProcessData == GEX_PROCESSPART_PARTSINSIDE || cQuery.iProcessData == GEX_PROCESSPART_PARTSOUTSIDE)
				{
					if (CGexRangeCoord(szValue).isValid() == false)
						ZTHROWEXC("invalid range value: " + ZString(szValue));
				}
				break;
				
			default: // Allow any test#
				szValue = "0 to 2147483647";	// Test is a 31bits number.
				break;
		}
		cQuery.strProcessData = szValue;
		return 1;
	}

	// Minimum number of parts required in file for considering it.
	if(!qstricmp(szSection,"db_minimum_samples"))
	{
		if(sscanf(szField,"%ld",&cQuery.lMinimumPartsInFile) != 1)
			cQuery.lMinimumPartsInFile = 0;
		return 1;
	}

	// Mapping/Merge test numbers
	if(!qstricmp(szSection,"db_mapping"))
	{
		cQuery.strMapFile = szField;
		return 1;
	}

	// Test list to focus on (only used for Remote Database queries)
	if(!qstricmp(szSection,"db_testlist"))
	{
		cQuery.strTestList = szField;
		return 1;
	}

	// Plugin options (only used for Remote Database queries)
	if(!qstricmp(szSection,"db_plugin_options"))
	{
		cQuery.strOptionsString = szField;
		return 1;
	}

	// SQL filter
	if(!qstricmp(szSection,"dbf_sql"))
	{
		// Make sure we have a third parameter
		if(argc < 3)
			ZTHROWEXC("missing parameter : filter value");
		QString strFilter = szField;
		strFilter += "=";
		strFilter += (char*) csl->get("value").constBuffer();
		cQuery.strlSqlFilters.append(strFilter);
		return 1;
	}

	// Split field
	if(!qstricmp(szSection,"db_split"))
	{
		if(!qstricmp(szField,"dbf_sql"))
		{
			// Make sure we have a third parameter
			if(argc < 3)
				ZTHROWEXC("missing parameter : filter value");
			cQuery.strSplitField = (char*) csl->get("value").constBuffer();
			return 1;
		}
		cQuery.strSplitField = szField;
		return 1;
	}

	// NON-SQL filters
	int iStatus = setQueryFilter(szSection,szField);
	if(iStatus)
		return 1;

	ZTHROWEXC("invalid parameter :"+ZString(szSection));
	return 0;
} // gexQuery

////////////////////////////////////////////////////////////////////////////////
// Function gexInteractive: Let's user prepare layers then jump into interactive mode!
// Call : gexOption(section, field, value);
////////////////////////////////////////////////////////////////////////////////
static ZString gexInteractive(ZCsl* csl)
{
	int		argc = csl->get("argCount").asInt();	// Number of arguments.
	char	*szSection;	// Section name to setup
	char	*szField="";	// Field name to setup
	char	*szValue="";	// Field value

	szSection = (char*) csl->get("section").constBuffer();
	if(argc > 1)
		szField = (char*) csl->get("field").constBuffer();
	if(argc > 2)
		szValue = (char*) csl->get("value").constBuffer();

	// Only valid under Examinator Production, Professional edition.
 	if(checkProfessionalFeature(true) == false)
		return 0;	

	// Check if valid handle to main GUI window
	if(pGexMainWindow == NULL)
		return 0;

	// RESET section
	if(!qstricmp(szSection,"reset"))
	{
		// Switch to Interactive page (have it visible)
		m_lGroupX = m_lGroupY = 0;

		// Define page to load after script completed...
		ShowReportOnCompletion("interactive_chart");

		// And force it to execute now as well (because Chart class is dynamically created and must be done now before inserting layers)
		pGexMainWindow->Wizard_DrillChart("");

		// Reset layer count
		m_InteractiveLayer = 0;
		return 1;
	}

	// Check if valid Interactive handle...
	if(pGexMainWindow->pWizardChart == NULL)
		return 0;

	// Add layer section: 'setX','<test#>','iGroup'
	if(!qstricmp(szSection,"setX"))
	{
		// Get Testnumber & Pinamp in X
		if(sscanf(szField,"%d%*c%d",&m_lInteractiveTestX,&m_lInteractiveTestPmX) < 2)
			m_lInteractiveTestPmX= GEX_PTEST;

		// Get Group#
		if(sscanf(szValue,"%d",&m_lGroupX) != 1)
			m_lGroupX = 0;

		// Reset Y axis for now
		m_lInteractiveTestY = m_lInteractiveTestPmY = -1;
		m_lGroupY = 0;
		return 1;
	}

	// Add layer section: 'setY','<test#>','iGroup'
	if(!qstricmp(szSection,"setY"))
	{
		// Get Testnumber & Pinamp in Y
		if(sscanf(szField,"%d%*c%d",&m_lInteractiveTestY,&m_lInteractiveTestPmY) < 2)
			m_lInteractiveTestPmY= GEX_PTEST;

		// Get Group#
		if(sscanf(szValue,"%d",&m_lGroupY) != 1)
			m_lGroupY = 0;
		return 1;
	}

	// Trigger to Add layer
	if(!qstricmp(szSection,"addLayer"))
	{
		bool bClearLayers = (m_InteractiveLayer == 0) ? true: false;
		pGexMainWindow->pWizardChart->addChart(false,bClearLayers,
			m_lInteractiveTestX,m_lInteractiveTestPmX,"",m_lGroupX,
			m_lInteractiveTestY,m_lInteractiveTestPmY,"",m_lGroupY);
		
		// Keep track of layers inserted
		m_InteractiveLayer++;
		return 1;
	}

	// Show all layers inserted
	if(!qstricmp(szSection,"Chart"))
	{
		// Have GUI (statistics, style, etc) properly refreshed.
		pGexMainWindow->pWizardChart->exitInsertMultiLayers(m_lInteractiveTestX,m_lInteractiveTestPmX,"");

		if(!qstricmp(szField,"Histogram"))
			pGexMainWindow->pWizardChart->OnHistogram();
		else
		if(!qstricmp(szField,"Trend"))
			pGexMainWindow->pWizardChart->OnTrend();
		else
		if(!qstricmp(szField,"Scatter"))
			pGexMainWindow->pWizardChart->OnScatter();
		else
		{
			ZTHROWEXC("invalid parameter :"+ZString(szField));
		}

		return 1;
	}

	ZTHROWEXC("invalid parameter :"+ZString(szSection));
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Function gexOption: Let's user control all GEX report options
// Call : gexOption(section, field, value);
////////////////////////////////////////////////////////////////////////////////
static ZString gexOptions(ZCsl* csl)
{
	CGexSingleChart	cChartStyle;		// Holds Chart style (color, line style & width,...)
	char	*		szSection;			// Section name to setup
	char	*		szField;			// Field name to setup
	char	*		szValue;			// Field value
	QString			strSetValue="1";	// can be used preset 'value' field

	int argc = csl->get("argCount").asInt();
	szSection = (char*) csl->get("section").constBuffer();
	szField = (char*) csl->get("field").constBuffer();
	szValue = (char*) csl->get("value").constBuffer();
	if(argc > 3)
		strSetValue = (char*) csl->get("setvalue").constBuffer();

	// RESET section
	if(!qstricmp(szSection,"reset"))
	{
		// Reset options
		if(qstricmp(szField,"all"))
		{
	      ZTHROWEXC("invalid parameter :"+ZString(szField));
		}

		if(!qstricmp(szValue,"default"))
		{
			// Reset all options to default
			ReportOptions.Reset(TRUE);
			return 1;	// No error.
		}
		else
		if(!qstricmp(szValue,"clear"))
		{
			// Reset all options to 0!
			ReportOptions.Reset(FALSE);
			return 1;	// No error.
		}
		else
	      ZTHROWEXC("invalid parameter :"+ZString(szValue));
	}

	// DATABASES section
	if(!qstricmp(szSection,"databases"))
	{
		if(!qstricmp(szField,"local_path"))
		{
			// Set Database local path
			ReportOptions.strDatabasesLocalPath = szValue;
			return 1;	// No error.
		}

		if(!qstricmp(szField,"server_path"))
		{
			// Set Database local path
			ReportOptions.strDatabasesServerPath = szValue;
			return 1;	// No error.
		}
      
		if(!qstricmp(szField,"rdb_default_parameters"))
		{
			if(!qstricmp(szValue,"all"))
			{
				ReportOptions.iRdbQueryDefaultParameters = GEX_OPTION_RDB_EXTRACT_PARAMETERS_ALL;
				return 1;	// No error.
			}

			if(!qstricmp(szValue,"none"))
			{
				ReportOptions.iRdbQueryDefaultParameters = GEX_OPTION_RDB_EXTRACT_PARAMETERS_NONE;
				return 1;	// No error.
			}

			ZTHROWEXC("invalid parameter :"+ZString(szValue));
		}

		ZTHROWEXC("invalid parameter :"+ZString(szField));
	}
	
	// OUTPUT section
	if(!qstricmp(szSection,"output"))
	{
		if(!qstricmp(szField,"format"))
		{
			// Set report output format: HTML
			if(!qstricmp(szValue,"html"))
			{
				ReportOptions.iOutputFormat = GEX_OPTION_OUTPUT_HTML;
				return 1;	// No error.
			}
			else
			// Set report output format: Microsoft Word
			if(!qstricmp(szValue,"word"))
			{
#ifdef unix				// Set report output format= MS-Word
				ReportOptions.iOutputFormat = GEX_OPTION_OUTPUT_PDF;		// MS-Word not available under unix!
#else
				if(ReportOptions.lEditionID == GEX_EDITION_ADV)
					ReportOptions.iOutputFormat = GEX_OPTION_OUTPUT_WORD;
				else
					ReportOptions.iOutputFormat = GEX_OPTION_OUTPUT_HTML;	// WORD format is only for the professional Edition!
#endif
				return 1;	// No error.
			}
			else
			// Set report output format: Powerpoint
			if(!qstricmp(szValue,"ppt"))
			{
#ifdef unix				// Set report output format= PowerPoint
				ReportOptions.iOutputFormat = GEX_OPTION_OUTPUT_PDF;		// PowerPoint not available under unix!
#else
				if(ReportOptions.lEditionID == GEX_EDITION_ADV)
					ReportOptions.iOutputFormat = GEX_OPTION_OUTPUT_PPT;
				else
					ReportOptions.iOutputFormat = GEX_OPTION_OUTPUT_HTML;	// PPT format is only for the professional Edition!
#endif
				return 1;	// No error.
			}
			else
			if(!qstricmp(szValue,"csv") || !qstricmp(szValue,"excel"))
			{
				// Set report output format= HTML
				ReportOptions.iOutputFormat = GEX_OPTION_OUTPUT_CSV;
				return 1;	// No error.
			}
			else
			// Set report output format: Adobe PDF
			if(!qstricmp(szValue,"pdf"))
			{
				// Set report output format= Adobe PDF
				if((ReportOptions.lEditionID == GEX_EDITION_ADV)
					|| (ReportOptions.lProductID == GEX_DATATYPE_ALLOWED_CREDENCE_ASL)
					|| (ReportOptions.lProductID ==  GEX_DATATYPE_ALLOWED_CREDENCE_SAPPHIRE)
					|| (ReportOptions.lProductID == GEX_DATATYPE_ALLOWED_SZ)
					|| (ReportOptions.lProductID == GEX_DATATYPE_ALLOWED_LTX))
					ReportOptions.iOutputFormat = GEX_OPTION_OUTPUT_PDF;
				else
					ReportOptions.iOutputFormat = GEX_OPTION_OUTPUT_HTML;	// PDF format is only for the professional Edition or ASL-OEM or SZ-OEM version!
				return 1;	// No error.
			}
			else
			// Set report output format: Adobe PDF
			if(!qstricmp(szValue,"interactive"))
			{
				// No report to create, only interactive mode!
				ReportOptions.iOutputFormat = GEX_OPTION_OUTPUT_INTERACTIVEONLY;

				return 1;	// No error.
			}
			
			ZTHROWEXC("invalid parameter :"+ZString(szValue));
		}

		// Home Page text (for Word, PPT and PDF reports)
		if(!qstricmp(szField,"front_page_text"))
		{
			ReportOptions.strFrontPageText = szValue;
			// Remove empty paragraphs
			ReportOptions.strFrontPageText.replace("<p> </p>","");

			return 1;	// No error.
		}

		// Home Page Image (for Word, PPT and PDF reports)
		if(!qstricmp(szField,"front_page_image"))
		{
			ReportOptions.strFrontPageImage = szValue;
			return 1;	// No error.
		}

		// Test names truncation (in Word, PDF, PPT reports)
		if(!qstricmp(szField,"truncate_names"))
		{
			if(!qstricmp(szValue,"yes"))
			{
				ReportOptions.iTruncateLabels = 32;
				return 1;	// No error.
			}
			else
			if(!qstricmp(szValue,"no"))
			{
				ReportOptions.iTruncateLabels = -1;	// Do not truncate labels!
				return 1;	// No error.
			}
			else
			{
				// Custom length (eg: 20 characters)
				if(sscanf(szValue,"%d",&ReportOptions.iTruncateLabels) != 1)
					ReportOptions.iTruncateLabels = 32;
				return 1;	// No error.
			}
			ZTHROWEXC("invalid parameter :"+ZString(szValue));
		}

		// Paper size: Letter or A4
		if(!qstricmp(szField,"paper_size"))
		{
			if(!qstricmp(szValue,"Letter"))
			{
				ReportOptions.iPaperSize = GEX_OPTION_PAPER_SIZE_LETTER;
				return 1;	// No error.
			}
			else
			if(!qstricmp(szValue,"A4"))
			{
				ReportOptions.iPaperSize = GEX_OPTION_PAPER_SIZE_A4;
				return 1;	// No error.
			}
			ZTHROWEXC("invalid parameter :"+ZString(szValue));
		}

		// Paper format: portrait or landscape
		if(!qstricmp(szField,"paper_format"))
		{
			if(!qstricmp(szValue,"portrait"))
			{
				ReportOptions.bPortraitFormat = true;
				return 1;	// No error.
			}
			else
			if(!qstricmp(szValue,"landscape"))
			{
				ReportOptions.bPortraitFormat = false;
				return 1;	// No error.
			}
			ZTHROWEXC("invalid parameter :"+ZString(szValue));
		}

		if(!qstricmp(szField,"location"))
		{
			// Set report output location
			ReportOptions.strOutputPath = szValue;
			return 1;	// No error.
		}
      
		ZTHROWEXC("invalid parameter :"+ZString(szField));
	}
	// Monitoring section
	if(!qstricmp(szSection,"monitoring"))
	{
		// History size section
		if(!qstricmp(szField,"history"))
		{
			if(!qstricmp(szValue,"none"))
			{
				ReportOptions.iMonitorHistory = GEXMO_OPTION_HISTORY_NONE;
				return 1;
			}
			if(!qstricmp(szValue,"1week"))
			{
				ReportOptions.iMonitorHistory = GEXMO_OPTION_HISTORY_1WEEK;
				return 1;
			}
			if(!qstricmp(szValue,"2week"))
			{
				ReportOptions.iMonitorHistory = GEXMO_OPTION_HISTORY_2WEEK;
				return 1;
			}
			if(!qstricmp(szValue,"3week"))
			{
				ReportOptions.iMonitorHistory = GEXMO_OPTION_HISTORY_3WEEK;
				return 1;
			}
			if(!qstricmp(szValue,"1month"))
			{
				ReportOptions.iMonitorHistory = GEXMO_OPTION_HISTORY_1MONTH;
				return 1;
			}
			if(!qstricmp(szValue,"2month"))
			{
				ReportOptions.iMonitorHistory = GEXMO_OPTION_HISTORY_2MONTH;
				return 1;
			}
			if(!qstricmp(szValue,"3month"))
			{
				ReportOptions.iMonitorHistory = GEXMO_OPTION_HISTORY_3MONTH;
				return 1;
			}
			if(!qstricmp(szValue,"4month"))
			{
				ReportOptions.iMonitorHistory = GEXMO_OPTION_HISTORY_4MONTH;
				return 1;
			}
			if(!qstricmp(szValue,"5month"))
			{
				ReportOptions.iMonitorHistory = GEXMO_OPTION_HISTORY_5MONTH;
				return 1;
			}
			if(!qstricmp(szValue,"6month"))
			{
				ReportOptions.iMonitorHistory = GEXMO_OPTION_HISTORY_6MONTH;
				return 1;
			}
			if(!qstricmp(szValue,"1year"))
			{
				ReportOptions.iMonitorHistory = GEXMO_OPTION_HISTORY_1YEAR;
				return 1;
			}
			if(!qstricmp(szValue,"all"))
			{
				ReportOptions.iMonitorHistory = GEXMO_OPTION_HISTORY_ALL;
				return 1;
			}
			ZTHROWEXC("invalid parameter :"+ZString(szValue));
		}
		// Home page fields
		if(!qstricmp(szField,"home_page"))
		{
			// Flags for fields to list in Home page
			sscanf(szValue,"%ld",&ReportOptions.iMonitorHome);
			return 1;	// No error.
		}
		// Products page fields
		if(!qstricmp(szField,"product_page"))
		{
			// Flags for fields to list in Products page
			sscanf(szValue,"%ld",&ReportOptions.iMonitorProduct);
			return 1;	// No error.
		}
		// Testers page fields
		if(!qstricmp(szField,"tester_page"))
		{
			// Flags for fields to list in Tester page
			sscanf(szValue,"%ld",&ReportOptions.iMonitorTester);
			return 1;	// No error.
		}
      
		ZTHROWEXC("invalid parameter :"+ZString(szField));
	}

	// Duplicate test section
	if(!qstricmp(szSection,"duplicate_test"))
	{
		ReportOptions.bMergeDuplicateTestNumber = false;
		ReportOptions.bMergeDuplicateTestName = false;
		if(!qstricmp(szField,"merge"))
		{
			// Merge test with identical test# (ignore test name)
			ReportOptions.bMergeDuplicateTestNumber = true;
			return 1;	// No error.
		}
		if(!qstricmp(szField,"merge_name"))
		{
			// Merge test with identical test name (ignore test#)
			ReportOptions.bMergeDuplicateTestName = true;
			return 1;	// No error.
		}
		if(!qstricmp(szField,"no_merge"))
		{
			// No merge same test numbers if test name is different
			return 1;	// No error.
		}
		ZTHROWEXC("invalid parameter :"+ZString(szValue));
	}

	// PARTID section
	if(!qstricmp(szSection,"part_id"))
	{
		if(!qstricmp(szField,"show"))
		{
			// Sets to SHOW PartID in reports
			ReportOptions.bShowPartID = true;
			return 1;	// No error.
		}
		if(!qstricmp(szField,"hide"))
		{
			// Sets to SHOW PartID in reports
			ReportOptions.bShowPartID = false;
			return 1;	// No error.
		}
		ZTHROWEXC("invalid parameter :"+ZString(szValue));
	}

	// Functional Test processing.
	if(!qstricmp(szSection,"functional_tests"))
	{
		if(!qstricmp(szField,"disabled"))
		{
			// Sets to Ignore STDF.FTR records
			ReportOptions.bIgnoreFunctionalTests = true;
			return 1;	// No error.
		}
		if(!qstricmp(szField,"enabled"))
		{
			// Sets to Process STDF.FTR records
			ReportOptions.bIgnoreFunctionalTests = false;
			return 1;	// No error.
		}
		ZTHROWEXC("invalid parameter :"+ZString(szValue));
	}

	// SORTING section
	if(!qstricmp(szSection,"sorting"))
	{
		if(!qstricmp(szField,"none"))
		{
			// Sets data sorting field
			ReportOptions.iSorting = GEX_SORTING_NONE;
			return 1;	// No error.
		}
		if(!qstricmp(szField,"date"))
		{
			ReportOptions.iSorting = GEX_SORTING_BYDATE;
			return 1;	// No error.
		}
		ZTHROWEXC("invalid parameter :"+ZString(szValue));
	}

	// FAILURES count section
	if(!qstricmp(szSection,"fail_count"))
	{
		if(!qstricmp(szField,"all"))
		{
			// Failure count mode = ALL failures are counted
			ReportOptions.iFailures = GEX_FAILCOUNT_ALL;
			return 1;	// No error.
		}
		if(!qstricmp(szField,"first"))
		{
			// Failure count mode = only 1st failure in flow counted
			ReportOptions.iFailures = GEX_FAILCOUNT_FIRST;
			return 1;	// No error.
		}
		ZTHROWEXC("invalid parameter :"+ZString(szValue));
	}

	// Multi-Parametric tests: merge pins under same name?
	if(!qstricmp(szSection,"multi_parametric"))
	{
		if(!qstricmp(szField,"merge"))
		{
			// Merge: Merge all multi-parametric pins under same test name
			ReportOptions.bMultiParametricMerge = TRUE;
			return 1;	// No error.
		}
		if(!qstricmp(szField,"no_merge"))
		{
			// No merge: keep one test per pin tested.
			ReportOptions.bMultiParametricMerge = FALSE;
			return 1;	// No error.
		}
		ZTHROWEXC("invalid parameter :"+ZString(szValue));
	}

	// STDF Compliancy handling
	if(!qstricmp(szSection,"stdf_compliancy"))
	{
		if(!qstricmp(szField,"stringent"))
		{
			// Expect STDF files to be fully compliant
			ReportOptions.bStringentStdfCompliancy = true;
			return 1;
		}
		if(!qstricmp(szField,"flexible"))
		{
			// Do best to handle any STDF file!
			ReportOptions.bStringentStdfCompliancy = false;
			return 1;
		}
		ZTHROWEXC("invalid parameter :"+ZString(szField));
	}

	// Folder where intermediate files are created (STDF & zip)
	if(!qstricmp(szSection,"stdf_intermediate"))
	{
		ReportOptions.strIntermediateStdfFolder = szField;
		if(QFile::exists(szField) || ReportOptions.strIntermediateStdfFolder.isEmpty() ||
			ReportOptions.strIntermediateStdfFolder.startsWith("(default)",false))
			return 1;	// Valid folder defined.

		// Throw exception in case folder doesn't exist.
		ZTHROWEXC("Invalid 'stdf_intermediate' folder path:"+ZString(szField));
	}

	// Speed Optimization
	if(!qstricmp(szSection,"speed"))
	{
		if(!qstricmp(szField,"adv_stats"))
		{
			// When to compute Advanced Statistics
			if(!qstricmp(szValue,"always"))
			{
				// Always
				ReportOptions.iSpeedComputeAdvancedStatistics = GEX_OPTION_SPEED_ADVSTATS_ALWAYS;
				return 1;
			}
			if(!qstricmp(szValue,"50mb"))
			{
				// If less than 50Mb of data
				ReportOptions.iSpeedComputeAdvancedStatistics = GEX_OPTION_SPEED_ADVSTATS_50MB;
				return 1;
			}
			if(!qstricmp(szValue,"100mb"))
			{
				// If less than 100Mb of data
				ReportOptions.iSpeedComputeAdvancedStatistics = GEX_OPTION_SPEED_ADVSTATS_100MB;
				return 1;
			}
			if(!qstricmp(szValue,"200mb"))
			{
				// If less than 200Mb of data
				ReportOptions.iSpeedComputeAdvancedStatistics = GEX_OPTION_SPEED_ADVSTATS_200MB;
				return 1;
			}
			if(!qstricmp(szValue,"300mb"))
			{
				// If less than 300Mb of data
				ReportOptions.iSpeedComputeAdvancedStatistics = GEX_OPTION_SPEED_ADVSTATS_300MB;
				return 1;
			}
			if(!qstricmp(szValue,"400mb"))
			{
				// If less than 400Mb of data
				ReportOptions.iSpeedComputeAdvancedStatistics = GEX_OPTION_SPEED_ADVSTATS_400MB;
				return 1;
			}
			if(!qstricmp(szValue,"500mb"))
			{
				// If less than 500Mb of data
				ReportOptions.iSpeedComputeAdvancedStatistics = GEX_OPTION_SPEED_ADVSTATS_500MB;
				return 1;
			}
			if(!qstricmp(szValue,"never"))
			{
				// Never
				ReportOptions.iSpeedComputeAdvancedStatistics = GEX_OPTION_SPEED_ADVSTATS_NEVER;
				return 1;
			}
			ZTHROWEXC("invalid parameter :"+ZString(szValue));
		}
		if(!qstricmp(szField,"collect_samples"))
		{
			// Option outdated!
			return 1;
		}
		if(!qstricmp(szField,"db_summary"))
		{
			// When to use the database SUMMARY records instead of the data samples
			if(!qstricmp(szValue,"always"))
			{
				// Always
				ReportOptions.iSpeedUseSummaryDB = GEX_OPTION_SPEED_SUMMARYDB_ALWAYS;
				return 1;
			}
			if(!qstricmp(szValue,"50mb"))
			{
				// If less than 50Mb of data
				ReportOptions.iSpeedUseSummaryDB = GEX_OPTION_SPEED_SUMMARYDB_50MB;
				return 1;
			}
			if(!qstricmp(szValue,"100mb"))
			{
				// If less than 100Mb of data
				ReportOptions.iSpeedUseSummaryDB = GEX_OPTION_SPEED_SUMMARYDB_100MB;
				return 1;
			}
			if(!qstricmp(szValue,"200mb"))
			{
				// If less than 200Mb of data
				ReportOptions.iSpeedUseSummaryDB = GEX_OPTION_SPEED_SUMMARYDB_200MB;
				return 1;
			}
			if(!qstricmp(szValue,"300mb"))
			{
				// If less than 300Mb of data
				ReportOptions.iSpeedUseSummaryDB = GEX_OPTION_SPEED_SUMMARYDB_300MB;
				return 1;
			}
			if(!qstricmp(szValue,"400mb"))
			{
				// If less than 400Mb of data
				ReportOptions.iSpeedUseSummaryDB = GEX_OPTION_SPEED_SUMMARYDB_400MB;
				return 1;
			}
			if(!qstricmp(szValue,"500mb"))
			{
				// If less than 500Mb of data
				ReportOptions.iSpeedUseSummaryDB = GEX_OPTION_SPEED_SUMMARYDB_500MB;
				return 1;
			}
			if(!qstricmp(szValue,"never"))
			{
				// Never
				ReportOptions.iSpeedUseSummaryDB = GEX_OPTION_SPEED_SUMMARYDB_NEVER;
				return 1;
			}
		}
		ZTHROWEXC("invalid parameter :"+ZString(szValue));
	}

	// OUTLIER section
	if(!qstricmp(szSection,"outlier"))
	{
		if(!qstricmp(szField,"removal"))
		{
			if(!qstricmp(szValue,"none"))
			{
				ReportOptions.iOutlierRemoveMode = GEX_OPTION_OUTLIER_NONE;
				return 1;
			}
			if(!qstricmp(szValue,"1sigma"))
			{
				ReportOptions.iOutlierRemoveMode = GEX_OPTION_OUTLIER_SIGMA;
				ReportOptions.fOutlierRemoveN_Factor = 0.5;
				return 1;
			}
			if(!qstricmp(szValue,"2sigma"))
			{
				ReportOptions.iOutlierRemoveMode = GEX_OPTION_OUTLIER_SIGMA;
				ReportOptions.fOutlierRemoveN_Factor = 1.0;
				return 1;
			}
			if(!qstricmp(szValue,"3sigma"))
			{
				ReportOptions.iOutlierRemoveMode = GEX_OPTION_OUTLIER_SIGMA;
				ReportOptions.fOutlierRemoveN_Factor = 1.5;
				return 1;
			}
			if(!qstricmp(szValue,"4sigma"))
			{
				ReportOptions.iOutlierRemoveMode = GEX_OPTION_OUTLIER_SIGMA;
				ReportOptions.fOutlierRemoveN_Factor = 2.0;
				return 1;
			}
			if(!qstricmp(szValue,"5sigma"))
			{
				ReportOptions.iOutlierRemoveMode = GEX_OPTION_OUTLIER_SIGMA;
				ReportOptions.fOutlierRemoveN_Factor = 2.5;
				return 1;
			}
			if(!qstricmp(szValue,"6sigma"))
			{
				ReportOptions.iOutlierRemoveMode = GEX_OPTION_OUTLIER_SIGMA;
				ReportOptions.fOutlierRemoveN_Factor = 3.0;
				return 1;
			}
			if(!qstricmp(szValue,"7sigma"))
			{
				ReportOptions.iOutlierRemoveMode = GEX_OPTION_OUTLIER_SIGMA;
				ReportOptions.fOutlierRemoveN_Factor = 3.5;
				return 1;
			}
			if(!qstricmp(szValue,"8sigma"))
			{
				ReportOptions.iOutlierRemoveMode = GEX_OPTION_OUTLIER_SIGMA;
				ReportOptions.fOutlierRemoveN_Factor = 4.0;
				return 1;
			}
			if(!qstricmp(szValue,"9sigma"))
			{
				ReportOptions.iOutlierRemoveMode = GEX_OPTION_OUTLIER_SIGMA;
				ReportOptions.fOutlierRemoveN_Factor = 4.5;
				return 1;
			}
			if(!qstricmp(szValue,"10sigma"))
			{
				ReportOptions.iOutlierRemoveMode = GEX_OPTION_OUTLIER_SIGMA;
				ReportOptions.fOutlierRemoveN_Factor = 5.0;
				return 1;
			}
			if(!qstricmp(szValue,"12sigma"))
			{
				ReportOptions.iOutlierRemoveMode = GEX_OPTION_OUTLIER_SIGMA;
				ReportOptions.fOutlierRemoveN_Factor = 6.0;
				return 1;
			}
			if(!qstricmp(szValue,"n_sigma"))
			{
				// N*Sigma.
				ReportOptions.iOutlierRemoveMode = GEX_OPTION_OUTLIER_SIGMA;
				ReportOptions.fOutlierRemoveN_Factor = strSetValue.toFloat();
				return 1;
			}
			if(!qstricmp(szValue,"exclude_n_sigma"))
			{
				// Exclude N*Sigma.: Inliner filtering.
				ReportOptions.iOutlierRemoveMode = GEX_OPTION_INLINER_SIGMA;
				ReportOptions.fOutlierRemoveN_Factor = strSetValue.toFloat();
				return 1;
			}
			if(!qstricmp(szValue,"100%"))
			{
				ReportOptions.iOutlierRemoveMode = GEX_OPTION_OUTLIER_100LIM;
				return 1;
			}
			if(!qstricmp(szValue,"150%"))
			{
				ReportOptions.iOutlierRemoveMode = GEX_OPTION_OUTLIER_150LIM;
				return 1;
			}
			if(!qstricmp(szValue,"200%"))
			{
				ReportOptions.iOutlierRemoveMode = GEX_OPTION_OUTLIER_200LIM;
				return 1;
			}
			if(!qstricmp(szValue,"250%"))
			{
				ReportOptions.iOutlierRemoveMode = GEX_OPTION_OUTLIER_250LIM;
				return 1;
			}
			if(!qstricmp(szValue,"300%"))
			{
				ReportOptions.iOutlierRemoveMode = GEX_OPTION_OUTLIER_300LIM;
				return 1;
			}
			if(!qstricmp(szValue,"iqr"))
			{
				ReportOptions.iOutlierRemoveMode = GEX_OPTION_OUTLIER_IQR;
				ReportOptions.fOutlierRemoveIQR_Factor = 1.5;
				return 1;
			}
			if(!qstricmp(szValue,"n_iqr"))
			{
				// Q1-N*IQR , Q3+N*IQR
				ReportOptions.iOutlierRemoveMode = GEX_OPTION_OUTLIER_IQR;
				ReportOptions.fOutlierRemoveIQR_Factor = strSetValue.toFloat();
				return 1;
			}

			ZTHROWEXC("invalid parameter :"+ZString(szValue));
		}

		ZTHROWEXC("invalid parameter :"+ZString(szField));
	}

	// STATISTICS section
	if(!qstricmp(szSection,"statistics"))
	{
		if(!qstricmp(szField,"section_name"))
		{
	      ReportOptions.strStatsSectionTitle = szValue;
		  return 1;
		}

		if(!qstricmp(szField,"computation"))
		{
			if(!qstricmp(szValue,"summary_only"))
			{
				ReportOptions.iStatsSource = GEX_STATISTICS_FROM_SUMMARY_ONLY;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"samples_only"))
			{
				ReportOptions.iStatsSource= GEX_STATISTICS_FROM_SAMPLES_ONLY;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"samples_then_summary") || !qstricmp(szValue,"samples"))
			{
				ReportOptions.iStatsSource= GEX_STATISTICS_FROM_SAMPLES_THEN_SUMMARY;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"summary_then_samples") || !qstricmp(szValue,"summary"))
			{
				ReportOptions.iStatsSource= GEX_STATISTICS_FROM_SUMMARY_THEN_SAMPLES;
				return 1;	// No error.
			}

			ZTHROWEXC("invalid parameter :"+ZString(szValue));
		}

		if(!qstricmp(szField,"cp_cpk_computation"))
		{
			if(!qstricmp(szValue,"standard"))
			{
				ReportOptions.bStatsCpCpkPercentileFormula= FALSE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"percentile"))
			{
				ReportOptions.bStatsCpCpkPercentileFormula= TRUE;
				return 1;	// No error.
			}
			ZTHROWEXC("invalid parameter :"+ZString(szValue));
		}

		if(!qstricmp(szField,"smart_scaling"))
		{
			if(!qstricmp(szValue,"no"))
			{
				ReportOptions.iStatsSmartScaling= GEX_UNITS_RESCALE_NONE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"yes"))
			{
				ReportOptions.iStatsSmartScaling=GEX_UNITS_RESCALE_SMART;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"normalized"))
			{
				ReportOptions.iStatsSmartScaling= GEX_UNITS_RESCALE_NORMALIZED;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"to_limits"))
			{
				ReportOptions.iStatsSmartScaling= GEX_UNITS_RESCALE_TO_LIMITS;
				return 1;	// No error.
			}
			ZTHROWEXC("invalid parameter :"+ZString(szValue));
		}

		if(!qstricmp(szField,"field"))
		{
			// Sets fields to list in Statistics table.
			if(!qstricmp(szValue,"test_name"))
			{
				ReportOptions.bStatsTableTestName = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"test_type"))
			{
				ReportOptions.bStatsTableTestType = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"limits"))
			{
				ReportOptions.bStatsTableTestLimits = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"spec_limits"))
			{
				ReportOptions.bStatsTableSpecLimits = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"shape"))
			{
				if(ReportOptions.lEditionID == GEX_EDITION_ADV)
					ReportOptions.bStatsTableShape = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"stats_source"))
			{
				ReportOptions.bStatsSource  = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"test_flow_id"))
			{
				ReportOptions.bStatsTableTestFlowID = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"exec_count"))
			{
				ReportOptions.bStatsTableExec = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"fail_count"))
			{
				ReportOptions.bStatsTableFail = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"fail_percent"))
			{
				ReportOptions.bStatsTableFailPercent = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"fail_bin"))
			{
				ReportOptions.bStatsTableFailBin = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"outlier_count"))
			{
				ReportOptions.bStatsTableOutlier = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"mean"))
			{
				ReportOptions.bStatsTableMean = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"mean_shift"))
			{
				ReportOptions.bStatsTableMeanShift = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"t_test"))
			{
				ReportOptions.bStatsTableT_Test = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"sigma"))
			{
				ReportOptions.bStatsTableSigma = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"sigma_shift"))
			{
				ReportOptions.bStatsTableSigmaShift = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"2sigma"))
			{
				ReportOptions.bStatsTable2Sigma = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"3sigma"))
			{
				ReportOptions.bStatsTable3Sigma = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"6sigma"))
			{
				ReportOptions.bStatsTable6Sigma = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"min"))
			{
				ReportOptions.bStatsTableMinVal = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"max"))
			{
				ReportOptions.bStatsTableMaxVal = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"range"))
			{
				ReportOptions.bStatsTableRange = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"max_range"))
			{
				ReportOptions.bStatsTableMaxRange = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"cp"))
			{
				ReportOptions.bStatsTableCp = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"cp_shift"))
			{
				ReportOptions.bStatsTableCpShift = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"cpk"))
			{
				ReportOptions.bStatsTableCpk = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"cpkL"))
			{
				if(ReportOptions.lEditionID == GEX_EDITION_ADV)
					ReportOptions.bStatsTableCpkLow = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"cpkH"))
			{
				if(ReportOptions.lEditionID == GEX_EDITION_ADV)
					ReportOptions.bStatsTableCpkHigh = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"cpk_shift"))
			{
				ReportOptions.bStatsTableCpkShift = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"yield"))
			{
				ReportOptions.bStatsTableYield = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"correlation"))
				return 1;	// Silent ignored: no longer suported!.
			if(!qstricmp(szValue,"perf_limits"))
				return 1;	// Silent ignored: no longer suported!.
			if(!qstricmp(szValue,"ev"))
			{
				ReportOptions.bStatsTableGageRepeatabilityEV = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"av"))
			{
				ReportOptions.bStatsTableGageReproducibilityAV = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"r&r"))
			{
				ReportOptions.bStatsTableGageR_and_R = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"pv"))
			{
				ReportOptions.bStatsTableGagePV = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"tv"))
			{
				ReportOptions.bStatsTableGageTV = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"p_t"))
			{
				ReportOptions.bStatsTableGageP_T = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"skew"))
			{
				if(ReportOptions.lEditionID == GEX_EDITION_ADV)
					ReportOptions.bStatsTableSkew = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"kurtosis"))
			{
				if(ReportOptions.lEditionID == GEX_EDITION_ADV)
					ReportOptions.bStatsTableKurtosis = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"P0.5%") || !qstricmp(szValue,"P0.5"))
			{
				if(ReportOptions.lEditionID == GEX_EDITION_ADV)
					ReportOptions.bStatsTableP0_5 = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"P2.5%") || !qstricmp(szValue,"P2.5"))
			{
				if(ReportOptions.lEditionID == GEX_EDITION_ADV)
					ReportOptions.bStatsTableP2_5 = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"P10%") || !qstricmp(szValue,"P10"))
			{
				if(ReportOptions.lEditionID == GEX_EDITION_ADV)
					ReportOptions.bStatsTableP10 = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"quartile1") || !qstricmp(szValue,"P25%") || !qstricmp(szValue,"P25"))
			{
				if(ReportOptions.lEditionID == GEX_EDITION_ADV)
					ReportOptions.bStatsTableQuartile1 = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"quartile2") || !qstricmp(szValue,"P50%") || !qstricmp(szValue,"P50"))
			{
				if(ReportOptions.lEditionID == GEX_EDITION_ADV)
					ReportOptions.bStatsTableQuartile2 = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"quartile3") || !qstricmp(szValue,"P75%") || !qstricmp(szValue,"P75"))
			{
				if(ReportOptions.lEditionID == GEX_EDITION_ADV)
					ReportOptions.bStatsTableQuartile3 = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"P90%") || !qstricmp(szValue,"P90"))
			{
				if(ReportOptions.lEditionID == GEX_EDITION_ADV)
					ReportOptions.bStatsTableP90 = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"P97.5%") || !qstricmp(szValue,"P97.5"))
			{
				if(ReportOptions.lEditionID == GEX_EDITION_ADV)
					ReportOptions.bStatsTableP97_5 = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"P99.5%") || !qstricmp(szValue,"P99.5"))
			{
				if(ReportOptions.lEditionID == GEX_EDITION_ADV)
					ReportOptions.bStatsTableP99_5 = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"interquartiles"))
			{
				if(ReportOptions.lEditionID == GEX_EDITION_ADV)
					ReportOptions.bStatsTableInterquartiles = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"SigmaInterQuartiles"))
			{
				if(ReportOptions.lEditionID == GEX_EDITION_ADV)
					ReportOptions.bStatsTableSigmaInterQuartiles = TRUE;
				return 1;	// No error.
			}

			ZTHROWEXC("invalid parameter :"+ZString(szValue));
		}

		if(!qstricmp(szField,"sorting"))
		{
			// Sets Statistics table sorting field
			if(!qstricmp(szValue,"test_number"))
			{
				ReportOptions.iStatsTableSorting = GEX_SORTING_TESTNUMBER;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"test_name"))
			{
				ReportOptions.iStatsTableSorting = GEX_SORTING_TESTNAME;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"test_flow_id"))
			{
				ReportOptions.iStatsTableSorting = GEX_SORTING_TESTFLOWID;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"mean"))
			{
				ReportOptions.iStatsTableSorting = GEX_SORTING_MEAN;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"mean_shift"))
			{
				ReportOptions.iStatsTableSorting = GEX_SORTING_MEAN_SHIFT;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"sigma"))
			{
				ReportOptions.iStatsTableSorting = GEX_SORTING_SIGMA;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"cp"))
			{
				ReportOptions.iStatsTableSorting = GEX_SORTING_CP;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"cpk"))
			{
				ReportOptions.iStatsTableSorting = GEX_SORTING_CPK;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"r&r"))
			{
				ReportOptions.iStatsTableSorting = GEX_SORTING_R_R;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"correlation"))
			{
				// ignored: no longer supported!
				return 1;	// No error.
			}
			ZTHROWEXC("invalid parameter :"+ZString(szValue));
		}
		if(!qstricmp(szField,"alarm_test_cpk"))
		{
			// Sets Cpk RED alarm level!
			if(sscanf(szValue,"%lf",&ReportOptions.fStatsTableRedAlarmCpk) != 1)
				ReportOptions.fStatsTableRedAlarmCpk = -1;	// Ignore alarm!
			return 1;
		}
		if(!qstricmp(szField,"warning_test_cpk"))
		{
			// Sets Cpk YELLOW alarm level!
			if(sscanf(szValue,"%lf",&ReportOptions.fStatsTableYellowAlarmCpk) != 1)
				ReportOptions.fStatsTableYellowAlarmCpk = -1;	// Ignore alarm!
			return 1;
		}		
		if(!qstricmp(szField,"alarm_test_yield"))
		{
			// Sets Test yield RED alarm level!
			if(sscanf(szValue,"%lf",&ReportOptions.fStatsTableRedAlarmTestYield) != 1)
				ReportOptions.fStatsTableRedAlarmTestYield = -1;	// Ignore alarm!
			return 1;
		}
		if(!qstricmp(szField,"warning_test_yield"))
		{
			// Sets Test yield YELLOW alarm level!
			if(sscanf(szValue,"%lf",&ReportOptions.fStatsTableYellowAlarmTestYield) != 1)
				ReportOptions.fStatsTableYellowAlarmTestYield = -1;	// Ignore alarm!
			return 1;
		}
		if(!qstricmp(szField,"alarm_mean"))
		{
			// Sets Mean shift alarm level!
			if(sscanf(szValue,"%lf",&ReportOptions.fStatsTableAlarmMeanDrift) != 1)
				ReportOptions.fStatsTableAlarmMeanDrift = -1;	// Ignore alarm!
			return 1;

		}
		if(!qstricmp(szField,"alarm_sigma"))
		{
			// Sets Sigma shift alarm level!
			if(sscanf(szValue,"%lf",&ReportOptions.fStatsTableAlarmSigmaDrift) != 1)
				ReportOptions.fStatsTableAlarmSigmaDrift = -1;	// Ignore alarm!
			return 1;

		}
		if(!qstricmp(szField,"alarm_cpk"))
		{
			// Sets Cpk shift alarm level!
			if(sscanf(szValue,"%lf",&ReportOptions.fStatsTableAlarmCpkDrift) != 1)
				ReportOptions.fStatsTableAlarmCpkDrift = -1;	// Ignore alarm!
			return 1;

		}
		if(!qstricmp(szField,"mean_drift_formula")) 
		{
			// Sets the formula to use to compute the MEAN drift.
			if(!qstricmp(szValue,"value"))
			{
				ReportOptions.iStatsMeanDriftFormula = GEX_DRIFT_ALARM_VALUE;	// Drift of value
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"limits"))
			{
				ReportOptions.iStatsMeanDriftFormula = GEX_DRIFT_ALARM_LIMITS;	// Drift of limits space
				return 1;	// No error.
			}
			ZTHROWEXC("invalid parameter :"+ZString(szValue));
			return 1;

		}
		ZTHROWEXC("invalid parameter :"+ZString(szField));
	}


	// WAFER MAP section
	if(!qstricmp(szSection,"wafer"))
	{
		if(!qstricmp(szField,"section_name"))
		{
	      ReportOptions.strWafmapSectionTitle = szValue;
		  return 1;
		}

		if(!qstricmp(szField,"chart_show"))
		{
			if(sscanf(szValue,"%d",&ReportOptions.iWafmapShow) != 1)
				ReportOptions.iWafmapShow = GEX_OPTION_WAFMAP_STACKED;

			return 1;	// No error.
		}
		if(!qstricmp(szField,"parametric_stacked"))
		{
			if(sscanf(szValue,"%d",&ReportOptions.iWafmapParametricStacked) != 1)
				ReportOptions.iWafmapParametricStacked = GEX_OPTION_WAFMAP_PARAM_STACKED_MEAN;

			return 1;	// No error.
		}
		if(!qstricmp(szField,"gross_die"))
		{
			if(sscanf(szValue,"%d",&ReportOptions.iGrossDieCount) != 1)
				ReportOptions.iGrossDieCount = -1;
			return 1;	// No error.
		}
		if(!qstricmp(szField,"chart_view"))
		{
			// Outdated functions. No longer supported. Quietly exit
			if(!qstricmp(szValue,"mirror_x"))
			{
				ReportOptions.bWafmapMirrorX = true;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"no_mirror_x"))
			{
				ReportOptions.bWafmapMirrorX = false;
				return 1;	// No error.
			}
			
			if(!qstricmp(szValue,"mirror_y"))
			{
				ReportOptions.bWafmapMirrorY = true;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"no_mirror_y"))
			{
				ReportOptions.bWafmapMirrorY = false;
				return 1;	// No error.
			}

			if(!qstricmp(szValue,"all_parts"))
			{
				ReportOptions.bWafmapFullWafer = true;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"filtered_parts"))
			{
				ReportOptions.bWafmapFullWafer = false;
				return 1;	// No error.
			}

			if(!qstricmp(szValue,"shape_round"))
			{
				ReportOptions.bWafmapAlwaysRound = true;	// Resize dies to always make wafermap look round.
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"shape_default"))
			{
				ReportOptions.bWafmapAlwaysRound = false;
				return 1;	// No error.
			}

#if 1 // BG: WaferMap orientation options
			if(!qstricmp(szValue,"positive_x_right"))
			{
				ReportOptions.iWafmapPositiveX = GEX_OPTION_WAFMAP_POSITIVE_RIGHT;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"positive_x_left"))
			{
				ReportOptions.iWafmapPositiveX = GEX_OPTION_WAFMAP_POSITIVE_LEFT;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"positive_x_auto"))
			{
				ReportOptions.iWafmapPositiveX = GEX_OPTION_WAFMAP_POSITIVE_AUTO;
				return 1;	// No error.
			}			
			if(!qstricmp(szValue,"positive_y_up"))
			{
				ReportOptions.iWafmapPositiveY = GEX_OPTION_WAFMAP_POSITIVE_UP;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"positive_y_down"))
			{
				ReportOptions.iWafmapPositiveY = GEX_OPTION_WAFMAP_POSITIVE_DOWN;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"positive_y_auto"))
			{
				ReportOptions.iWafmapPositiveY = GEX_OPTION_WAFMAP_POSITIVE_AUTO;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"notch_ignore"))
			{
				ReportOptions.iWafmapNotchOrientation = GEX_OPTION_WAFMAP_NOTCH_IGNORE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"notch_down"))
			{
				ReportOptions.iWafmapNotchOrientation = GEX_OPTION_WAFMAP_NOTCH_DOWN;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"notch_up"))
			{
				ReportOptions.iWafmapNotchOrientation = GEX_OPTION_WAFMAP_NOTCH_UP;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"notch_left"))
			{
				ReportOptions.iWafmapNotchOrientation = GEX_OPTION_WAFMAP_NOTCH_LEFT;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"notch_right"))
			{
				ReportOptions.iWafmapNotchOrientation = GEX_OPTION_WAFMAP_NOTCH_RIGHT;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"notch_auto"))
			{
				ReportOptions.iWafmapNotchOrientation = GEX_OPTION_WAFMAP_NOTCH_AUTO;
				return 1;	// No error.
			}
#endif
			ZTHROWEXC("invalid parameter :"+ZString(szValue));
		}
		if(!qstricmp(szField,"low_yield_pattern"))
		{
			// Extract Low-Yield pattern threshold value (used to detect failing patterns in stacked wafers)
			if(sscanf(szValue,"%f",&ReportOptions.fWafmapStackedLowYieldAlarm) != 1)
				ReportOptions.fWafmapStackedLowYieldAlarm = 33;
			if(ReportOptions.fWafmapStackedLowYieldAlarm < 0)
				ReportOptions.fWafmapStackedLowYieldAlarm = 0;
			else
			if(ReportOptions.fWafmapStackedLowYieldAlarm > 100)
				ReportOptions.fWafmapStackedLowYieldAlarm = 100;

				return 1;	// No error.
		}

		// Allow comparing wafer with different size?
		if(!qstricmp(szField,"compare"))
		{
			if(!qstricmp(szValue,"any_size"))
			{
				ReportOptions.bWafmapCompareAnySize = true;
				return 1;
			}
			if(!qstricmp(szValue,"diemismatch_table_off"))
			{
				ReportOptions.bWafmapIncludeDieMismatchTable = false;
				return 1;
			}
			if(!qstricmp(szValue,"deltayield_section_off"))
			{
				ReportOptions.bWafmapIncludeDeltaYieldSection = false;
				return 1;
			}
			ZTHROWEXC("invalid parameter :"+ZString(szValue));
		}

		if(!qstricmp(szField,"chart_size"))
		{
			// Sets fields wafer map chart size
			if(!qstricmp(szValue,"small"))
			{
				ReportOptions.iWafmapChartSize = GEX_CHARTSIZE_SMALL;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"medium"))
			{
				ReportOptions.iWafmapChartSize = GEX_CHARTSIZE_MEDIUM;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"large"))
			{
				ReportOptions.iWafmapChartSize = GEX_CHARTSIZE_LARGE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"auto"))
			{
				ReportOptions.iWafmapChartSize = GEX_CHARTSIZE_AUTO;
				return 1;	// No error.
			}
			ZTHROWEXC("invalid parameter :"+ZString(szValue));
		}

		if(!qstricmp(szField,"retest_policy"))
		{
			// Highest bin: promote the highest bin value for each die
			if(!qstricmp(szValue,"highest_bin"))
			{
				ReportOptions.bWafMapRetest_HighestBin = true;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"last_bin"))
			{
				// Last bin: Use Last bin value found for each die (default)
				ReportOptions.bWafMapRetest_HighestBin = false;
				return 1;	// No error.
			}
			ZTHROWEXC("invalid parameter :"+ZString(szValue));

		}
		if(!qstricmp(szField,"marker"))
		{
			// Sets wafer map markers
			if(!qstricmp(szValue,"bin"))
			{
				ReportOptions.bWafmapMarkerBinInDie = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"retest"))
			{
				ReportOptions.bWafmapMarkerDieRetested = TRUE;
				return 1;	// No error.
			}
			
			ZTHROWEXC("invalid parameter :"+ZString(szValue));
		}
		ZTHROWEXC("invalid parameter :"+ZString(szField));
	}


	// HISTOGRAM section
	if(!qstricmp(szSection,"histogram"))
	{
		if(!qstricmp(szField,"section_name"))
		{
	      ReportOptions.strHistoSectionTitle = szValue;
		  return 1;
		}
		ZTHROWEXC("invalid parameter :"+ZString(szField));
	}

	// Advanced CHART section
	if(!qstricmp(szSection,"adv_chart"))
	{
		if(!qstricmp(szField,"field"))
		{
			// Sets fields to list in Statistics table.
			if(!qstricmp(szValue,"limits"))
			{
				ReportOptions.bHistoStatsLimits = (bool) strSetValue.toInt();
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"exec_count"))
			{
				ReportOptions.bHistoStatsExec = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"mean"))
			{
				ReportOptions.bHistoStatsMean = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"quartile2"))
			{
				ReportOptions.bHistoStatsQuartile2 = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"sigma"))
			{
				ReportOptions.bHistoStatsSigma = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"outlier_count"))
			{
				ReportOptions.bHistoStatsOutlier = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"fail_count"))
			{
				ReportOptions.bHistoStatsFails = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"cp"))
			{
				ReportOptions.bHistoStatsCp = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"cpk"))
			{
				ReportOptions.bHistoStatsCpk = TRUE;
				return 1;	// No error.
			}
			ZTHROWEXC("invalid parameter :"+ZString(szValue));
		}

		ZTHROWEXC("invalid parameter :"+ZString(szField));
	}

	// Advanced HISTOGRAM section
	if(!qstricmp(szSection,"adv_histogram"))
	{
		if(!qstricmp(szField,"field"))
		{
			// Sets fields to list in Statistics table.
			if(!qstricmp(szValue,"limits"))
			{
				ReportOptions.bHistoStatsLimits = (bool) strSetValue.toInt();
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"exec_count"))
			{
				ReportOptions.bHistoStatsExec = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"mean"))
			{
				ReportOptions.bHistoStatsMean = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"quartile2"))
			{
				ReportOptions.bHistoStatsQuartile2 = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"sigma"))
			{
				ReportOptions.bHistoStatsSigma = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"outlier_count"))
			{
				ReportOptions.bHistoStatsOutlier = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"fail_count"))
			{
				ReportOptions.bHistoStatsFails = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"cp"))
			{
				ReportOptions.bHistoStatsCp = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"cpk"))
			{
				ReportOptions.bHistoStatsCpk = TRUE;
				return 1;	// No error.
			}
			ZTHROWEXC("invalid parameter :"+ZString(szValue));
		}

		if(!qstricmp(szField,"chart_size"))
		{
			// Sets fields histogram chart size
			if(!qstricmp(szValue,"banner"))
			{
				ReportOptions.iHistoChartSize = GEX_CHARTSIZE_BANNER;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"small"))
			{
				ReportOptions.iHistoChartSize = GEX_CHARTSIZE_SMALL;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"medium"))
			{
				ReportOptions.iHistoChartSize = GEX_CHARTSIZE_MEDIUM;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"large"))
			{
				ReportOptions.iHistoChartSize = GEX_CHARTSIZE_LARGE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"auto"))
			{
				ReportOptions.iHistoChartSize = GEX_CHARTSIZE_AUTO;
				return 1;	// No error.
			}
			ZTHROWEXC("invalid parameter :"+ZString(szValue));
		}
		if(!qstricmp(szField,"chart_type"))
		{
			// Sets fields histogram chart type
			if(!qstricmp(szValue,"bars"))
			{
				ReportOptions.iHistoChartType = GEX_CHARTTYPE_BARS;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"bars_outline"))
			{
				ReportOptions.iHistoChartType = GEX_CHARTTYPE_BARSOUTLINE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"curve"))
			{
				ReportOptions.iHistoChartType = GEX_CHARTTYPE_SPLINE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"bars_curve"))
			{
				ReportOptions.iHistoChartType = GEX_CHARTTYPE_BARSSPLINE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"gaussian"))
			{
				ReportOptions.iHistoChartType = GEX_CHARTTYPE_GAUSSIAN;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"bars_gaussian"))
			{
				ReportOptions.iHistoChartType = GEX_CHARTTYPE_BARSGAUSSIAN;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"bars_outline_gaussian"))
			{
				ReportOptions.iHistoChartType = GEX_CHARTTYPE_BARSOUTLINEGAUSSIAN;
				return 1;	// No error.
			}
			ZTHROWEXC("invalid parameter :"+ZString(szValue));
		}
		if(!qstricmp(szField,"y_axis"))
		{
			// histogram Y-axis: Display percentage scale
			if(!qstricmp(szValue,"percentage"))
			{
				ReportOptions.bHistoFrequencyCountScale = false;
				return 1;	// No error.
			}
			// histogram Y-axis: Display count / hits scale
			if(!qstricmp(szValue,"hits"))
			{
				ReportOptions.bHistoFrequencyCountScale = true;
				return 1;	// No error.
			}
			ZTHROWEXC("invalid parameter :"+ZString(szValue));
		}

		if(!qstricmp(szField,"total_bars"))
		{
			if(sscanf(szValue,"%d",&ReportOptions.iHistoClasses) != 1)
				ReportOptions.iHistoClasses = TEST_ADVHISTOSIZE;	// No value, force to default (40)
			if(ReportOptions.iHistoClasses < 2)
				ReportOptions.iHistoClasses = 2;
			if(ReportOptions.iHistoClasses > 10000)
				ReportOptions.iHistoClasses = 10000;
			return 1;	// No error.
		}
		if(!qstricmp(szField,"marker"))
		{
			// Sets Advanced histogram markers
			if(!qstricmp(szValue,"test_name") || !qstricmp(szValue,"title"))
			{
				ReportOptions.bHistoMarkerTitle = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"mean"))
			{
				ReportOptions.bHistoMarkerMean = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"median"))
			{
				// Only supported by Examinator Professional edition!
				if(ReportOptions.lEditionID == GEX_EDITION_ADV)
					ReportOptions.bHistoMarkerMedian = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"limits"))
			{
				ReportOptions.bHistoMarkerLimits = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"2sigma"))
			{
				ReportOptions.bHistoMarker2Sigma = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"3sigma"))
			{
				ReportOptions.bHistoMarker3Sigma = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"6sigma"))
			{
				ReportOptions.bHistoMarker6Sigma = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"12sigma"))
			{
				ReportOptions.bHistoMarker12Sigma = TRUE;
				return 1;	// No error.
			}
			ZTHROWEXC("invalid parameter :"+ZString(szValue));
		}
		ZTHROWEXC("invalid parameter :"+ZString(szField));
	}

	// Advanced TREND section
	if(!qstricmp(szSection,"adv_trend"))
	{
		if(!qstricmp(szField,"chart_size"))
		{
			// Sets fields trend chart size
			if(!qstricmp(szValue,"banner"))
			{
				ReportOptions.iTrendChartSize = GEX_CHARTSIZE_BANNER;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"small"))
			{
				ReportOptions.iTrendChartSize = GEX_CHARTSIZE_SMALL;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"medium"))
			{
				ReportOptions.iTrendChartSize = GEX_CHARTSIZE_MEDIUM;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"large"))
			{
				ReportOptions.iTrendChartSize = GEX_CHARTSIZE_LARGE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"auto"))
			{
				ReportOptions.iTrendChartSize = GEX_CHARTSIZE_AUTO;
				return 1;	// No error.
			}
			ZTHROWEXC("invalid parameter :"+ZString(szValue));
		}
		if(!qstricmp(szField,"chart_type"))
		{
			// Sets fields Trend chart type
			if(!qstricmp(szValue,"lines"))
			{
				ReportOptions.iTrendChartType = GEX_CHARTTYPE_LINES;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"spots"))
			{
				ReportOptions.iTrendChartType = GEX_CHARTTYPE_SPOTS;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"lines_spots"))
			{
				ReportOptions.iTrendChartType = GEX_CHARTTYPE_LINESSPOTS;
				return 1;	// No error.
			}
			ZTHROWEXC("invalid parameter :"+ZString(szValue));
		}		
		if(!qstricmp(szField,"marker"))
		{
			// Sets Advanced trend markers
			if(!qstricmp(szValue,"test_name") || !qstricmp(szValue,"title"))
			{
				ReportOptions.bTrendMarkerTitle = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"mean"))
			{
				ReportOptions.bTrendMarkerMean = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"median"))
			{
				// Only supported by Examinator Professional edition!
				if(ReportOptions.lEditionID == GEX_EDITION_ADV)
					ReportOptions.bTrendMarkerMedian = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"limits"))
			{
				ReportOptions.bTrendMarkerLimits = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"lot"))
			{
				ReportOptions.bTrendMarkerLot= TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"sublot"))
			{
				ReportOptions.bTrendMarkerSubLot= TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"group_name"))
			{
				ReportOptions.bTrendMarkerGroupName= TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"2sigma"))
			{
				ReportOptions.bTrendMarker2Sigma = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"3sigma"))
			{
				ReportOptions.bTrendMarker3Sigma = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"6sigma"))
			{
				ReportOptions.bTrendMarker6Sigma = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"12sigma"))
			{
				ReportOptions.bTrendMarker12Sigma = TRUE;
				return 1;	// No error.
			}
			ZTHROWEXC("invalid parameter :"+ZString(szValue));
		}
		if(!qstricmp(szField,"rolling_yield"))
		{
			// Extract number of parts to use for a rolling yield (default is 200)
			if(sscanf(szValue,"%d",&ReportOptions.iTrendRollingYieldParts) != 1)
				ReportOptions.iTrendRollingYieldParts = 200;
			if(ReportOptions.iTrendRollingYieldParts < 1)
				ReportOptions.iTrendRollingYieldParts = 1;
				return 1;	// No error.
		}
		ZTHROWEXC("invalid parameter :"+ZString(szField));
	}

	// Advanced CORRELATION section
	if(!qstricmp(szSection,"adv_correlation"))
	{
		if(!qstricmp(szField,"chart_size"))
		{
			// Sets fields correlation chart size
			if(!qstricmp(szValue,"small"))
			{
				ReportOptions.iScatterChartSize = GEX_CHARTSIZE_SMALL;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"medium"))
			{
				ReportOptions.iScatterChartSize = GEX_CHARTSIZE_MEDIUM;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"large"))
			{
				ReportOptions.iScatterChartSize = GEX_CHARTSIZE_LARGE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"auto"))
			{
				ReportOptions.iScatterChartSize = GEX_CHARTSIZE_AUTO;
				return 1;	// No error.
			}
			ZTHROWEXC("invalid parameter :"+ZString(szValue));
		}
	
		if(!qstricmp(szField,"marker"))
		{
			// Sets Advanced correlation markers
			if(!qstricmp(szValue,"test_name")  || !qstricmp(szValue,"title"))
			{
				ReportOptions.bScatterMarkerTitle = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"mean"))
			{
				ReportOptions.bScatterMarkerMean = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"median"))
			{
				// Only supported by Examinator Professional edition!
				if(ReportOptions.lEditionID == GEX_EDITION_ADV)
					ReportOptions.bScatterMarkerMedian = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"limits"))
			{
				ReportOptions.bScatterMarkerLimits = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"2sigma"))
			{
				ReportOptions.bScatterMarker2Sigma = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"3sigma"))
			{
				ReportOptions.bScatterMarker3Sigma = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"6sigma"))
			{
				ReportOptions.bScatterMarker6Sigma = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"12sigma"))
			{
				ReportOptions.bScatterMarker12Sigma = TRUE;
				return 1;	// No error.
			}
			ZTHROWEXC("invalid parameter :"+ZString(szValue));
		}
		
		if(!qstricmp(szField,"chart_type"))
		{
			// Sets fields Scatter chart type
			if(!qstricmp(szValue,"lines"))
			{
				ReportOptions.iScatterChartType = GEX_CHARTTYPE_SPOTS;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"spots"))
			{
				ReportOptions.iScatterChartType = GEX_CHARTTYPE_SPOTS;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"lines_spots"))
			{
				ReportOptions.iScatterChartType = GEX_CHARTTYPE_SPOTS;
				return 1;	// No error.
			}
			ZTHROWEXC("invalid parameter :"+ZString(szValue));
		}
		
		ZTHROWEXC("invalid parameter :"+ZString(szField));
	}

	// Advanced Probability plot section
	if(!qstricmp(szSection,"adv_probabilityplot"))
	{
		if(!qstricmp(szField,"chart_size"))
		{
			// Sets fields correlation chart size
			if(!qstricmp(szValue,"small"))
			{
				ReportOptions.iProbPlotChartSize = GEX_CHARTSIZE_SMALL;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"medium"))
			{
				ReportOptions.iProbPlotChartSize = GEX_CHARTSIZE_MEDIUM;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"large"))
			{
				ReportOptions.iProbPlotChartSize = GEX_CHARTSIZE_LARGE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"auto"))
			{
				ReportOptions.iProbPlotChartSize = GEX_CHARTSIZE_AUTO;
				return 1;	// No error.
			}
			ZTHROWEXC("invalid parameter :"+ZString(szValue));
		}
	
		if(!qstricmp(szField,"marker"))
		{
			// Sets Advanced correlation markers
			if(!qstricmp(szValue,"test_name")  || !qstricmp(szValue,"title"))
			{
				ReportOptions.bProbPlotMarkerTitle = true;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"mean"))
			{
				ReportOptions.bProbPlotMarkerMean = true;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"median"))
			{
				// Only supported by Examinator Professional edition!
				if(ReportOptions.lEditionID == GEX_EDITION_ADV)
					ReportOptions.bProbPlotMarkerMedian = true;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"limits"))
			{
				ReportOptions.bProbPlotMarkerLimits = true;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"2sigma"))
			{
				ReportOptions.bProbPlotMarker2Sigma = true;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"3sigma"))
			{
				ReportOptions.bProbPlotMarker3Sigma = true;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"6sigma"))
			{
				ReportOptions.bProbPlotMarker6Sigma = true;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"12sigma"))
			{
				ReportOptions.bProbPlotMarker12Sigma = true;
				return 1;	// No error.
			}
			ZTHROWEXC("invalid parameter :"+ZString(szValue));
		}

		if(!qstricmp(szField,"y_axis"))
		{
			// Sets Advanced correlation markers
			if(!qstricmp(szValue,"sigma"))
			{
				ReportOptions.bProbPlotYAxisSigma = true;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"percentage"))
			{
				ReportOptions.bProbPlotYAxisSigma = false;
				return 1;	// No error.
			}
			ZTHROWEXC("invalid parameter :"+ZString(szValue));
		}
		ZTHROWEXC("invalid parameter :"+ZString(szField));
	}

		// Advanced BoxPlot section
	if(!qstricmp(szSection,"adv_boxplot_ex"))
	{
		if(!qstricmp(szField,"chart_size"))
		{
			// Sets fields correlation chart size
			if(!qstricmp(szValue,"small"))
			{
				ReportOptions.iBoxPlotExChartSize = GEX_CHARTSIZE_SMALL;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"medium"))
			{
				ReportOptions.iBoxPlotExChartSize = GEX_CHARTSIZE_MEDIUM;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"large"))
			{
				ReportOptions.iBoxPlotExChartSize = GEX_CHARTSIZE_LARGE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"auto"))
			{
				ReportOptions.iBoxPlotExChartSize = GEX_CHARTSIZE_AUTO;
				return 1;	// No error.
			}
			ZTHROWEXC("invalid parameter :"+ZString(szValue));
		}
	
		if(!qstricmp(szField,"marker"))
		{
			// Sets Advanced correlation markers
			if(!qstricmp(szValue,"test_name")  || !qstricmp(szValue,"title"))
			{
				ReportOptions.bBoxPlotExMarkerTitle = true;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"mean"))
			{
				ReportOptions.bBoxPlotExMarkerMean = true;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"median"))
			{
				// Only supported by Examinator Professional edition!
				if(ReportOptions.lEditionID == GEX_EDITION_ADV)
					ReportOptions.bBoxPlotExMarkerMedian = true;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"limits"))
			{
				ReportOptions.bBoxPlotExMarkerLimits = true;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"2sigma"))
			{
				ReportOptions.bBoxPlotExMarker2Sigma = true;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"3sigma"))
			{
				ReportOptions.bBoxPlotExMarker3Sigma = true;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"6sigma"))
			{
				ReportOptions.bBoxPlotExMarker6Sigma = true;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"12sigma"))
			{
				ReportOptions.bBoxPlotExMarker12Sigma = true;
				return 1;	// No error.
			}
			ZTHROWEXC("invalid parameter :"+ZString(szValue));
		}

		if(!qstricmp(szField,"orientation"))
		{
			// Sets Advanced correlation markers
			if(!qstricmp(szValue,"vertical"))
			{
				ReportOptions.bBoxPlotExVertical = true;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"horizontal"))
			{
				ReportOptions.bBoxPlotExVertical = false;
				return 1;	// No error.
			}
			ZTHROWEXC("invalid parameter :"+ZString(szValue));
		}
		ZTHROWEXC("invalid parameter :"+ZString(szField));
	}

	// Advanced PRODUCTION YIELD/VOLUME section
	if(!qstricmp(szSection,"adv_production_yield"))
	{
		if(!qstricmp(szField,"chart_type"))
		{
			// Plot Yield chart only
			if(!qstricmp(szValue,"yield"))
			{
				ReportOptions.bProdYieldChart= TRUE;
				ReportOptions.bProdVolumeChart= FALSE;
				return 1;	// No error.
			}
			// Plot Volume chart only
			if(!qstricmp(szValue,"volume"))
			{
				ReportOptions.bProdVolumeChart= TRUE;
				ReportOptions.bProdYieldChart= FALSE;
				return 1;	// No error.
			}
			// Plot Yield/Volume combined chart
			if(!qstricmp(szValue,"yield_volume"))
			{
				ReportOptions.bProdVolumeChart= TRUE;
				ReportOptions.bProdYieldChart= TRUE;
				return 1;	// No error.
			}
			ZTHROWEXC("invalid parameter :"+ZString(szValue));
		}
		if(!qstricmp(szField,"marker"))
		{ 
			// Display title
			if(!qstricmp(szValue,"title"))
			{
				ReportOptions.bProdMarkerTitle= TRUE;
				return 1;	// No error.
			}
			// Display Yield percentage for each data point
			if(!qstricmp(szValue,"yield"))
			{
				ReportOptions.bProdMarkerYield= TRUE;
				return 1;	// No error.
			}
			// Display volume couont for each bar
			if(!qstricmp(szValue,"volume"))
			{
				ReportOptions.bProdMarkerVolume= TRUE;
				return 1;	// No error.
			}
			ZTHROWEXC("invalid parameter :"+ZString(szValue));
		}
		ZTHROWEXC("invalid parameter :"+ZString(szField));
	}

	// Gage R&R section
	if(!qstricmp(szSection,"adv_boxplot"))
	{
		if(!qstricmp(szField,"field"))
		{
			// Sets fields to list boxplot 
			if(!qstricmp(szValue,"limits"))
			{
				ReportOptions.bBoxplotTestLimits = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"mean"))
			{
				ReportOptions.bBoxplotMean = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"sigma"))
			{
				ReportOptions.bBoxplotSigma = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"median"))
			{
				ReportOptions.bBoxplotMedian = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"cp"))
			{
				ReportOptions.bBoxplotCp = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"cpk"))
			{
				ReportOptions.bBoxplotCpk = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"repeatability"))
			{
				ReportOptions.bBoxplotRepeatability = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"ev"))
			{
				ReportOptions.bBoxplotRepeatabilityEV = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"av"))
			{
				ReportOptions.bBoxplotReproducibilityAV = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"r&r"))
			{
				ReportOptions.bBoxplotR_and_R = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"pv"))
			{
				ReportOptions.bBoxplotPV = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"tv"))
			{
				ReportOptions.bBoxplotTV = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"p_t"))
			{
				ReportOptions.bBoxplotP_T = TRUE;
				return 1;	// No error.
			}

			ZTHROWEXC("invalid parameter :"+ZString(szValue));
		}
		if(!qstricmp(szField,"chart_type"))
		{
			// Sets boxplot charting mode to: Over Limits
			if(!qstricmp(szValue,"limits"))
			{
				ReportOptions.iBoxplotCharting = GEX_BOXPLOTTYPE_LIMITS;
				return 1;	// No error.
			}
			// Sets boxplot charting mode to: Over Range
			if(!qstricmp(szValue,"range"))
			{
				ReportOptions.iBoxplotCharting = GEX_BOXPLOTTYPE_RANGE;
				return 1;	// No error.
			}
			// Sets boxplot charting mode to: Adaptive (over Limits if one dataset, over range otherwise)
			if(!qstricmp(szValue,"adaptive"))
			{
				ReportOptions.iBoxplotCharting = GEX_BOXPLOTTYPE_ADAPTIVE;
				return 1;	// No error.
			}
			ZTHROWEXC("invalid parameter :"+ZString(szValue));
		}
		if(!qstricmp(szField,"sorting"))
		{
			// Sets boxplot table sorting field
			if(!qstricmp(szValue,"test_number"))
			{
				ReportOptions.iBoxplotSorting = GEX_SORTING_TESTNUMBER;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"test_name"))
			{
				ReportOptions.iBoxplotSorting = GEX_SORTING_TESTNAME;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"test_flow_id"))
			{
				ReportOptions.iBoxplotSorting = GEX_SORTING_TESTFLOWID;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"mean"))
			{
				ReportOptions.iBoxplotSorting = GEX_SORTING_MEAN;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"sigma"))
			{
				ReportOptions.iBoxplotSorting = GEX_SORTING_SIGMA;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"cp"))
			{
				ReportOptions.iBoxplotSorting = GEX_SORTING_CP;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"cpk"))
			{
				ReportOptions.iBoxplotSorting = GEX_SORTING_CPK;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"r&r"))
			{
				ReportOptions.iBoxplotSorting = GEX_SORTING_R_R;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"correlation"))
			{
				// ignored: no longer supported!
				return 1;	// No error.
			}
			ZTHROWEXC("invalid parameter :"+ZString(szValue));
		}
		if(!qstricmp(szField,"r&r_sigma"))
		{
			// Get R&R N*Sigma rule (default is N=5.15)
			if(sscanf(szValue,"%lf",&ReportOptions.lfR_R_Nsigma) != 1)
				ReportOptions.lfR_R_Nsigma = 5.15;
			if(ReportOptions.lfR_R_Nsigma < 1)
				ReportOptions.lfR_R_Nsigma  = 1;
			if(ReportOptions.lfR_R_Nsigma > 12)
				ReportOptions.lfR_R_Nsigma = 12;
			return 1;	// No error.
		}
		
		if(!qstricmp(szField,"direction"))
		{
			if(!qstricmp(szValue,"vertical"))
			{
				ReportOptions.bBoxPlotExVertical = true;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"horizontal"))
			{
				ReportOptions.bBoxPlotExVertical = false;
				return 1;	// No error.
			}
			ZTHROWEXC("invalid parameter :"+ZString(szValue));
		}

		if(!qstricmp(szField,"%"))
		{
			if(!qstricmp(szValue,"over_tv"))
			{
				ReportOptions.bBoxplotShiftOverTV = true;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"over_limits"))
			{
				ReportOptions.bBoxplotShiftOverTV = false;
				return 1;	// No error.
			}
			ZTHROWEXC("invalid parameter :"+ZString(szValue));
		}

		ZTHROWEXC("invalid parameter :"+ZString(szField));
	}

	// Pareto section
	if(!qstricmp(szSection,"pareto"))
	{
		if(!qstricmp(szField,"section_name"))
		{
	      ReportOptions.strParetoSectionTitle = szValue;
		  return 1;
		}

		if(!qstricmp(szField,"excludebinnings"))
		{
			if(!qstricmp(szValue,"pass"))
			{
				ReportOptions.bParetoExcludePassBins = true;
				return 1;
			}
			if(!qstricmp(szValue,"fail"))
			{
				ReportOptions.bParetoExcludeFailBins = true;
				return 1;
			}
			ZTHROWEXC("invalid parameter :"+ZString(szValue));
		}

		if(!qstricmp(szField,"section"))
		{
			if(!qstricmp(szValue,"enabled"))
			{
				// Include section in report
				ReportOptions.iParetoSection = GEX_OPTION_PARETO_ALL;
				return 1;
			}
			if(!qstricmp(szValue,"disabled"))
			{
				// Do NOT nclude section in report
				ReportOptions.iParetoSection = 0;
				return 1;
			}
			// Check if integer value specified...
			if(sscanf(szValue,"%d",&ReportOptions.iParetoSection) == 1)
				return 1;

			ZTHROWEXC("invalid parameter :"+ZString(szValue));
		}
		if(!qstricmp(szField,"cutoff_cp"))
		{
			// Sets Cp cutoff limit
			if(sscanf(szValue,"%lf",&ReportOptions.lfParetoCpCutoff) != 1)
				ReportOptions.lfParetoCpCutoff = -1;	// No cutoff, report ALL cp results
			return 1;
		}
		if(!qstricmp(szField,"cutoff_cpk"))
		{
			// Sets Cpk cutoff limit
			if(sscanf(szValue,"%lf",&ReportOptions.lfParetoCpkCutoff) != 1)
				ReportOptions.lfParetoCpkCutoff = -1;	// No cutoff, report ALL cpk results
			return 1;
		}
		if(!qstricmp(szField,"cutoff_failure"))
		{
			// Set maximum failures to report
			if(sscanf(szValue,"%d",&ReportOptions.iParetoFailCutoff) != 1)
				ReportOptions.iParetoFailCutoff = -1;	// No cutoff, report ALL failures
			else
			if(ReportOptions.iParetoFailCutoff < 0)
				ReportOptions.iParetoFailCutoff = -1;	// Report ALL
			return 1;
		}
		if(!qstricmp(szField,"cutoff_signature_failure"))
		{
			// Sets % of signature failures to report
			if(sscanf(szValue,"%lf",&ReportOptions.lfParetoFailPatternCutoff) != 1)
				ReportOptions.lfParetoFailPatternCutoff = -1;	// No cutoff, report ALL failing signatures
			else
			if(ReportOptions.lfParetoFailPatternCutoff < 0)
				ReportOptions.lfParetoFailPatternCutoff = -1;	// Report ALL
			else
			if(ReportOptions.lfParetoFailPatternCutoff > 100)
				ReportOptions.lfParetoFailPatternCutoff = 100;
			return 1;
		}

		ZTHROWEXC("invalid parameter :"+ZString(szField));
	}

	// Binning section
	if(!qstricmp(szSection,"binning"))
	{
		if(!qstricmp(szField,"section_name"))
		{
	      ReportOptions.strBinningSectionTitle = szValue;
		  return 1;
		}

		if(!qstricmp(szField,"section"))
		{
			if(!qstricmp(szValue,"enabled"))
			{
				// Include section in report
				ReportOptions.bBinningSection = true;
				return 1;
			}
			if(!qstricmp(szValue,"disabled"))
			{
				// Do NOT nclude section in report
				ReportOptions.bBinningSection = false;
				return 1;
			}
			ZTHROWEXC("invalid parameter :"+ZString(szValue));
		}
		if(!qstricmp(szField,"computation"))
		{
			if(!qstricmp(szValue,"wafer_map")) 
			{
				ReportOptions.bBinningUseWafermapOnly= TRUE;
				ReportOptions.bBinningUseSamplesOnly= FALSE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"summary"))
			{
				ReportOptions.bBinningUseWafermapOnly= FALSE;
				ReportOptions.bBinningUseSamplesOnly= FALSE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"samples"))
			{
				ReportOptions.bBinningUseWafermapOnly= FALSE;
				ReportOptions.bBinningUseSamplesOnly= TRUE;
				return 1;	// No error.
			}
			ZTHROWEXC("invalid parameter :"+ZString(szValue));
		}

		ZTHROWEXC("invalid parameter :"+ZString(szField));
	}

	// Advanced DATALOG section
	if(!qstricmp(szSection,"adv_datalog"))
	{
		if(!qstricmp(szField,"field"))
		{
			// Sets fields to list in datalog 
			if(!qstricmp(szValue,"comment"))
			{
				ReportOptions.bDatalogTableComments = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"test_number"))
			{
				ReportOptions.bDatalogTableTestNumber= TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"test_name"))
			{
				ReportOptions.bDatalogTableTestName = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"limits"))
			{
				ReportOptions.bDatalogTableLimits = TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"die_loc"))
			{
				ReportOptions.bDatalogTableDieXY = TRUE;
				return 1;	// No error.
			}
			ZTHROWEXC("invalid parameter :"+ZString(szValue));
		}
		if(!qstricmp(szField,"format"))
		{
			if(!qstricmp(szValue,"1row"))
			{
				ReportOptions.bDatalogSingleRow= TRUE;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"2rows"))
			{
				ReportOptions.bDatalogSingleRow= FALSE;
				return 1;	// No error.
			}
			ZTHROWEXC("invalid parameter :"+ZString(szValue));
		}

		ZTHROWEXC("invalid parameter :"+ZString(szField));
	}

	// Wha-if section
	if(!qstricmp(szSection,"adv_what_if"))
	{
		if(!qstricmp(szField,"pass_bin"))
		{
			// Sets What-if Passing bin value
			if(sscanf(szValue,"%d",&ReportOptions.iWhatIf_PassBin) != 1)
				ReportOptions.iWhatIf_PassBin = 1;	// Default
			return 1;
		}
		if(!qstricmp(szField,"fail_bin"))
		{
			// Sets What-if Failing bin value
			if(sscanf(szValue,"%d",&ReportOptions.iWhatIf_FailBin) != 1)
				ReportOptions.iWhatIf_FailBin = 0;	// Default
			return 1;
		}
		if(!qstricmp(szField,"fail_bin_is_pass"))
		{
			// Sets What-if Failing bin as a PASS bin type.
			int iValue;
			if(sscanf(szValue,"%d",&iValue) != 1)
				iValue = 0;	// Default
			ReportOptions.bWhatIf_FailBinIsPass = (bool) iValue;
			return 1;
		}
		ZTHROWEXC("invalid parameter :"+ZString(szField));
	}

	// Pearson correlation section
	if(!qstricmp(szSection,"adv_pearson"))
	{
		if(!qstricmp(szField,"cutoff"))
		{
			// Sets Pearson's cutoff limit
			if(sscanf(szValue,"%lf",&ReportOptions.lfPearsonCutoff) != 1)
				ReportOptions.lfPearsonCutoff = 0.8;	// Default
			return 1;
		}
		if(!qstricmp(szField,"sorting"))
		{
			// Sets Pearson's sorting mode
			if(!qstricmp(szValue,"test_name"))
			{
				ReportOptions.iSortingPearson = GEX_PEARSON_SORTING_TESTNAME;
				return 1;
			}
			if(!qstricmp(szValue,"pearson"))
			{
				ReportOptions.iSortingPearson = GEX_PEARSON_SORTING_RATIO;
				return 1;
			}
			ZTHROWEXC("invalid parameter :"+ZString(szValue));
		}
		ZTHROWEXC("invalid parameter :"+ZString(szField));
	}
	
	// Pearson correlation section
	if(!qstricmp(szSection,"toolbox"))
	{
		if(!qstricmp(szField,"csv_sorting"))
		{
			// Sets CSV export parser sorting mode
			if(!qstricmp(szValue,"flow_id"))
			{
				ReportOptions.bToolboxCsvSortTestFlow = true;
				return 1;
			}
			if(!qstricmp(szValue,"test_id"))
			{
				ReportOptions.bToolboxCsvSortTestFlow = false;
				return 1;
			}
			ZTHROWEXC("invalid parameter :"+ZString(szValue));
		}
		if(!qstricmp(szField,"csv_split_export"))
		{
			// Enabled/Diusabled splitting CSV generation into 250-rows CSV files.
			if(!qstricmp(szValue,"on"))
			{
				ReportOptions.bToolboxCsvSplitExport = true;
				return 1;
			}
			if(!qstricmp(szValue,"off"))
			{
				ReportOptions.bToolboxCsvSplitExport = false;
				return 1;
			}
			ZTHROWEXC("invalid parameter :"+ZString(szValue));
		}
		ZTHROWEXC("invalid parameter :"+ZString(szField));
	}

	// Application Preferences section
	if(!qstricmp(szSection,"preferences"))
	{
		// Define custom text editor.
		if(!qstricmp(szField,"text_editor"))
		{
			if(!qstricmp(szValue,"(default)")) 
			{
				#if (defined __sun__ || __hpux__)
				  ReportOptions.strTextEditor = "textedit";	// Editor under Solaris & HP-UX
				#elif (defined __linux__)
				  ReportOptions.strTextEditor = "gedit";	// Editor under Linux
				#else
				  ReportOptions.strTextEditor = "wordpad";	// Editor under windows
				#endif
				return 1;	// No error.
			}
			else
			{
				ReportOptions.strTextEditor = szValue;
				return 1;	// No error.
			}

			ZTHROWEXC("invalid parameter :"+ZString(szValue));
		}
		// Define custom SpreadSheet editor.
		if(!qstricmp(szField,"ssheet_editor"))
		{
			if(!qstricmp(szValue,"(default)")) 
			{
				#if (defined __sun__ || __hpux__)
				  ReportOptions.strSpreadSheetEditor = "textedit";	// Editor under Solaris & HP-UX
				#elif (defined __linux__)
				  ReportOptions.strSpreadSheetEditor = "oocalc";	// Editor under Linux
				#else
				  ReportOptions.strSpreadSheetEditor = "excel";	// Editor under windows
				#endif
				return 1;	// No error.
			}
			else
			{
				ReportOptions.strSpreadSheetEditor = szValue;
				return 1;	// No error.
			}

			ZTHROWEXC("invalid parameter :"+ZString(szValue));
		}

		// Define Auto-close (release license) if software is Idle
		if(!qstricmp(szField,"auto_close"))
		{
			if(!qstricmp(szValue,"15min")) 
			{
				ReportOptions.lAutoClose = 15*60;	// save information in seconds!
				return 1;	// No error.
			}
			else
			if(!qstricmp(szValue,"30min")) 
			{
				ReportOptions.lAutoClose = 30*60;
				return 1;	// No error.
			}
			else
			if(!qstricmp(szValue,"1hour")) 
			{
				ReportOptions.lAutoClose = 60*60;
				return 1;	// No error.
			}
			else
			if(!qstricmp(szValue,"2hours")) 
			{
				ReportOptions.lAutoClose = 2*60*60;
				return 1;	// No error.
			}
			else
			if(!qstricmp(szValue,"4hours")) 
			{
				ReportOptions.lAutoClose = 4*60*60;
				return 1;	// No error.
			}
			if(!qstricmp(szValue,"never")) 
			{
				ReportOptions.lAutoClose = 0;
				return 1;	// No error.
			}

			ZTHROWEXC("invalid parameter :"+ZString(szValue));
		}

		ZTHROWEXC("invalid parameter :"+ZString(szField));
	}
	
	
	// Build/No build report section
	if(!qstricmp(szSection,"report"))
	{
		if(!qstricmp(szField,"build"))
		{
			// Flag to tell if Build report or not after data query.
			int iBuildReport=1;
			sscanf(szValue,"%d",&iBuildReport);
			if(iBuildReport) 
				ReportOptions.bBuildReport = true;
			else
				ReportOptions.bBuildReport = false;
			return 1;	// No error.
		}
      
		ZTHROWEXC("invalid parameter :"+ZString(szField));
	}

	// Plugin section
	if(!qstricmp(szSection,"plugin"))
	{
		if(pGexMainWindow->pPluginMgr == NULL)
			ZTHROWEXC("No plugin loaded, can't process plugin commands");

		if(!qstricmp(szField,"set_command"))
		{
			// Set Command to execute when script is completed.
			pGexMainWindow->pPluginMgr->eventSetCommand(szValue);
			return 1;	// No error.
		}

		if(!qstricmp(szField,"set_setting"))
		{
			// Set Setting to used when processing datasets.
			pGexMainWindow->pPluginMgr->eventSetSetting(szValue);
			return 1;	// No error.
		}
		
		ZTHROWEXC("invalid parameter :"+ZString(szField));
	}

	// Charting style section (Interactive Charts)
	if(!qstricmp(szSection,"chart_style"))
	{
		int	iR,iG,iB;

		// Chart background color
		if(!qstricmp(szField,"bkg_color"))
		{
			sscanf(szValue,"%d %d %d",&iR,&iG,&iB);
			ReportOptions.cBkgColor = QColor(iR,iG,iB);
			return 1;	// No error.
		}

		// Overlay curve's name on chart?
		if(!qstricmp(szField,"show_legend"))
		{
			if(!qstricmp(szValue,"1")) 
				ReportOptions.bPlotLegend = true;
			else
				ReportOptions.bPlotLegend = false;
			return 1;	// No error.
		}

		// Plot Bars (applies to histograms only)
		if(!qstricmp(szField,"box_bars"))
		{
			if(!qstricmp(szValue,"1")) 
				cChartStyle.bBoxBars = true;
			else
				cChartStyle.bBoxBars = false;
			return 1;	// No error.
		}

		// Plot fitting curve
		if(!qstricmp(szField,"fitting_curve"))
		{
			if(!qstricmp(szValue,"1")) 
				cChartStyle.bFittingCurve = true;
			else
				cChartStyle.bFittingCurve = false;
			return 1;	// No error.
		}

		// Plot gaussian / bell curve
		if(!qstricmp(szField,"bell_curve"))
		{
			if(!qstricmp(szValue,"1")) 
				cChartStyle.bBellCurve = true;
			else
				cChartStyle.bBellCurve = false;
			return 1;	// No error.
		}

		// Box-Plot whisker type: Range or Q2 +/- 1.5*IQR
		if(!qstricmp(szField,"box_whisker"))
		{
			if(!qstricmp(szValue,"range")) 
			{
				cChartStyle.iWhiskerMode= GEX_WHISKER_RANGE;	// Range
				return 1;	// No error.
			}
			else
			if(!qstricmp(szValue,"q1q3")) 
			{
				cChartStyle.iWhiskerMode= GEX_WHISKER_Q1Q3;	// Q1-1.5*IQR, Q3+1.5*IQR
				return 1;	// No error.
			}
			else
			if(!qstricmp(szValue,"iqr")) 
			{
				cChartStyle.iWhiskerMode= GEX_WHISKER_IQR;	// Median+/-1.5IQR
				return 1;	// No error.
			}
			ZTHROWEXC("invalid parameter value :"+ZString(szValue));
		}

		// Plot lines
		if(!qstricmp(szField,"lines"))
		{
			if(!qstricmp(szValue,"1")) 
				cChartStyle.bLines = true;
			else
				cChartStyle.bLines = false;
			return 1;	// No error.
		}

		// Plot Spots
		if(!qstricmp(szField,"spots"))
		{
			if(!qstricmp(szValue,"1")) 
				cChartStyle.bSpots = true;
			else
				cChartStyle.bSpots = false;
			return 1;	// No error.
		}

		// Get line width
		if(!qstricmp(szField,"line_width"))
		{
			sscanf(szValue,"%d",&cChartStyle.iLineWidth);
			return 1;	// No error.
		}

		// Get line style (solid, dashed,...)
		if(!qstricmp(szField,"line_style"))
		{
			sscanf(szValue,"%d",&cChartStyle.iLineStyle);
			return 1;	// No error.
		}

		// Get spot style (cross, triangle, round,...)
		if(!qstricmp(szField,"spot_style"))
		{
			sscanf(szValue,"%d",&cChartStyle.iSpotStyle);
			return 1;	// No error.
		}

		// Get charting RGB colors
		if(!qstricmp(szField,"rgb_color"))
		{
			sscanf(szValue,"%d %d %d",&iR,&iG,&iB);
			cChartStyle.cColor = QColor(iR,iG,iB);
			return 1;	// No error.
		}

		///////////////////////////////////////
		// Markers
		///////////////////////////////////////

		// Mean marker: line width & color
		if(!qstricmp(szField,"marker_mean"))
		{
			sscanf(szValue,"%d %d %d %d",&cChartStyle.iMeanLineWidth,&iR,&iG,&iB);
			cChartStyle.cMeanColor = QColor(iR,iG,iB);
			return 1;	// No error.
		}

		// Median marker: line width & color
		if(!qstricmp(szField,"marker_median"))
		{
			sscanf(szValue,"%d %d %d %d",&cChartStyle.iMedianLineWidth, &iR,&iG,&iB);
			cChartStyle.cMedianColor = QColor(iR,iG,iB);
			return 1;	// No error.
		}

		// MIN marker: line width & color
		if(!qstricmp(szField,"marker_min"))
		{
			sscanf(szValue,"%d %d %d %d",&cChartStyle.iMinLineWidth,&iR,&iG,&iB);
			cChartStyle.cMinColor = QColor(iR,iG,iB);
			return 1;	// No error.
		}

		// MAX marker: line width & color
		if(!qstricmp(szField,"marker_max"))
		{
			sscanf(szValue,"%d %d %d %d",&cChartStyle.iMaxLineWidth,&iR,&iG,&iB);
			cChartStyle.cMaxColor = QColor(iR,iG,iB);
			return 1;	// No error.
		}

		// Limits marker: line width & color
		if(!qstricmp(szField,"marker_limits"))
		{
			sscanf(szValue,"%d %d %d %d",&cChartStyle.iLimitsLineWidth,&iR,&iG,&iB);
			cChartStyle.cLimitsColor = QColor(iR,iG,iB);
			return 1;	// No error.
		}

		// 2sigma marker: line width & color
		if(!qstricmp(szField,"marker_2sigma"))
		{
			sscanf(szValue,"%d %d %d %d",&cChartStyle.i2SigmaLineWidth,&iR,&iG,&iB);
			cChartStyle.c2SigmaColor = QColor(iR,iG,iB);
			return 1;	// No error.
		}

		// 3sigma marker: line width & color
		if(!qstricmp(szField,"marker_3sigma"))
		{
			sscanf(szValue,"%d %d %d %d ",&cChartStyle.i3SigmaLineWidth,&iR,&iG,&iB);
			cChartStyle.c3SigmaColor = QColor(iR,iG,iB);
			return 1;	// No error.
		}

		// 6sigma marker: line width & color
		if(!qstricmp(szField,"marker_6sigma"))
		{
			sscanf(szValue,"%d %d %d %d",&cChartStyle.i6SigmaLineWidth,&iR,&iG,&iB);
			cChartStyle.c6SigmaColor = QColor(iR,iG,iB);
			return 1;	// No error.
		}

		// 12sigma marker: line width & color
		if(!qstricmp(szField,"marker_12sigma"))
		{
			sscanf(szValue,"%d %d %d %d",&cChartStyle.i12SigmaLineWidth,&iR,&iG,&iB);
			cChartStyle.c12SigmaColor = QColor(iR,iG,iB);
			return 1;	// No error.
		}

		if(!qstricmp(szField,"chart_layer"))
		{
			// Get Layer ID
			sscanf(szValue,"%d",&iR);

			// Check if reset layer list
			if(iR < 0)
			{
				while (!ReportOptions.pLayersStyleList.isEmpty())
					delete ReportOptions.pLayersStyleList.takeFirst();
				
				return 1;
			}
			
			// Allocate structure to copy style info
			CGexSingleChart	*pChartStyle = new CGexSingleChart;
			*pChartStyle = cChartStyle;
			
			// Add structure to list of styles.
			ReportOptions.pLayersStyleList.append(pChartStyle);
			return 1;	// No error.
		}
      
		ZTHROWEXC("invalid parameter :"+ZString(szField));
	}

	// Binning style section (custom bin colors to be used in Binning & Wafer map reports)
	if(!qstricmp(szSection,"bin_style"))
	{
		int	iR,iG,iB;

		// Empty the list of custom style.
		if(!qstricmp(szField,"clear"))
		{
			// Need to manually delete items (do not use 'autodelete')
			while(!ReportOptions.pHardBinColorList.isEmpty())
				delete ReportOptions.pHardBinColorList.takeFirst();
			
			while(!ReportOptions.pSoftBinColorList.isEmpty())
				delete ReportOptions.pSoftBinColorList.takeFirst();
			
			return 1;	// No error.
		}

		// Tells if custom colors to be used or not.
		if(!qstricmp(szField,"custom_colors"))
		{
			if(!qstricmp(szValue,"1")) 
				ReportOptions.bUseCustomBinColors = true;
			else
				ReportOptions.bUseCustomBinColors = false;
			return 1;	// No error.
		}

		// Read The Bin# list
		if(!qstricmp(szField,"bin_list"))
		{
			cBinColor.cBinRange = new CGexRange(szValue);
			return 1;	// No error.
		}

		// Read The Bin RGB color
		if(!qstricmp(szField,"bin_color"))
		{
			sscanf(szValue,"%d %d %d",&iR,&iG,&iB);
			cBinColor.cBinColor = QColor(iR,iG,iB);
			return 1;	// No error.
		}

		// Insert the BinColor definition to the Hard bin or Soft bin list
		if(!qstricmp(szField,"add_bin"))
		{
			CBinColor *pBinColor = new CBinColor;
			*pBinColor = cBinColor;

			if(!qstricmp(szValue,"soft_bin")) 
				ReportOptions.pSoftBinColorList.append(pBinColor);
			else
				ReportOptions.pHardBinColorList.append(pBinColor);
			return 1;	// No error.
		}

		ZTHROWEXC("invalid parameter :"+ZString(szField));
	}

	ZTHROWEXC("invalid parameter :"+ZString(szSection));
	return 0;
} // gexOptions

////////////////////////////////////////////////////////////////////////////////
// Builds a test range from a test list string (or path to import file)
////////////////////////////////////////////////////////////////////////////////
CGexTestRange *createTestsRange(QString strParameterList,bool bAcceptRange=true, bool bIsAdvancedReport=false)
{
	// Check if the list is a file path...
	if(QFile::exists(strParameterList))
	{
		// The list must be imported from a file...
		strParameterList = ImportTestListFromFile(strParameterList);
	}
	
	// If the list doesn't a llow specifying range (eg: scatter plot specieid tests pairs only), then to appropriate filtering!
	if(!bAcceptRange)
	{
		// We have a regular list...
		strParameterList = strParameterList.replace( " ", ","); // ensure only one type of delimiter!
		strParameterList = strParameterList.replace( ";", ","); // ensure only one type of delimiter!
		strParameterList = strParameterList.replace( "to", ",",FALSE); // remove 'to' strings (not case sensitive)
		strParameterList = strParameterList.replace( "-", ","); // remove '-' strings
		strParameterList = strParameterList.replace( "..", ","); // remove '..' strings
		strParameterList = strParameterList.replace( ",,",","); // simplify ',,' strings
	}
	// If this is an advanced report, we need to save the parameter list.
	if(bIsAdvancedReport)
		ReportOptions.strAdvancedTestList = strParameterList;

	// Create the range list structures from the string holdinng the test list.
	return new CGexTestRange((char *)strParameterList.latin1());
}

////////////////////////////////////////////////////////////////////////////////
// Function GetWaferMapSettings: Extract & save wafer map settings
////////////////////////////////////////////////////////////////////////////////
static int GetWaferMapSettings(char *szField,char *szValue,bool bDataMining)
{
	if(!qstricmp(szField,"disabled"))
	{
	  ReportOptions.iWafermapType = GEX_WAFMAP_DISABLED;
	  return 1;
	}
	if(!qstricmp(szField,"soft_bin"))
	{
	  ReportOptions.iWafermapType = GEX_WAFMAP_SOFTBIN;

	  ReportOptions.pGexWafermapRangeList = createTestsRange(QString("1"),true,false);
	  
	  if(bDataMining)
		ReportOptions.iAdvancedReportSettings = GEX_WAFMAP_SOFTBIN;
	  return 1;
	}
	if(!qstricmp(szField,"stack_soft_bin"))
	{
		if(checkProfessionalFeature(true) == false)
		  return 0;	// STACKING mode requires the Production module or the Professional edition!

		ReportOptions.iWafermapType = GEX_WAFMAP_STACK_SOFTBIN;

		if(strstr(szValue,"all") != NULL)
		{
			ReportOptions.iWafermapTests = GEX_WAFMAP_ALL;
			
			// Save list of tests
			ReportOptions.pGexWafermapRangeList = createTestsRange(QString(""),true,false);
		}
		else
		{
			ReportOptions.iWafermapTests= GEX_WAFMAP_LIST;

			// Save list of tests
			ReportOptions.pGexWafermapRangeList = createTestsRange(szValue,true,false);
		}

		if(bDataMining)
			ReportOptions.iAdvancedReportSettings = GEX_WAFMAP_STACK_SOFTBIN;
	
		return 1;
	}
	if(!qstricmp(szField,"hard_bin"))
	{
	  ReportOptions.iWafermapType = GEX_WAFMAP_HARDBIN;

	  ReportOptions.pGexWafermapRangeList = createTestsRange(QString("1"),true,false);

	  if(bDataMining)
		ReportOptions.iAdvancedReportSettings = GEX_WAFMAP_HARDBIN;
	  return 1;
	}
	if(!qstricmp(szField,"stack_hard_bin"))
	{
 	  if(checkProfessionalFeature(true) == false)
		  return 0;	// STACKING mode requires the Production module or the Professional edition!

	  ReportOptions.iWafermapType = GEX_WAFMAP_STACK_HARDBIN;
	  // Save list of tests
	  ReportOptions.pGexWafermapRangeList = createTestsRange(szValue,true,false);
	  if(bDataMining)
		ReportOptions.iAdvancedReportSettings = GEX_WAFMAP_STACK_HARDBIN;
	  return 1;
	}
	if(!qstricmp(szField,"param_over_limits"))
	{
		ReportOptions.iWafermapType = GEX_WAFMAP_TESTOVERLIMITS;
	  
	  	if(strstr(szValue,"all") != NULL)
		{
			ReportOptions.iWafermapTests = GEX_WAFMAP_ALL;
			
			// Save list of tests
			ReportOptions.pGexWafermapRangeList = createTestsRange(QString(""),true,false);
		}
		else
		{
			ReportOptions.iWafermapTests= GEX_WAFMAP_LIST;

			// Save list of tests
			ReportOptions.pGexWafermapRangeList = createTestsRange(szValue,true,false);
		}

		if(bDataMining)
			ReportOptions.iAdvancedReportSettings = GEX_WAFMAP_TESTOVERLIMITS;
		return 1;
	}
	if(!qstricmp(szField,"stack_param_over_limits"))
	{
		if(checkProfessionalFeature(true) == false)
		  return 0;	// STACKING mode requires the Production module or the Professional edition!

		ReportOptions.iWafermapType = GEX_WAFMAP_STACK_TESTOVERLIMITS;
		
		if(strstr(szValue,"all") != NULL)
		{
			ReportOptions.iWafermapTests = GEX_WAFMAP_ALL;
			
			// Save list of tests
			ReportOptions.pGexWafermapRangeList = createTestsRange(QString(""),true,false);
		}
		else
		{
			ReportOptions.iWafermapTests= GEX_WAFMAP_LIST;

			// Save list of tests
			ReportOptions.pGexWafermapRangeList = createTestsRange(szValue,true,false);
		}

		if(bDataMining)
			ReportOptions.iAdvancedReportSettings = GEX_WAFMAP_STACK_TESTOVERLIMITS;
		
		return 1;
	}
	if(!qstricmp(szField,"param_over_range"))
	{
		ReportOptions.iWafermapType = GEX_WAFMAP_TESTOVERDATA;
	 
		if(strstr(szValue,"all") != NULL)
		{
			ReportOptions.iWafermapTests = GEX_WAFMAP_ALL;
			
			// Save list of tests
			ReportOptions.pGexWafermapRangeList = createTestsRange(QString(""),true,false);
		}
		else
		{
			ReportOptions.iWafermapTests= GEX_WAFMAP_LIST;

			// Save list of tests
			ReportOptions.pGexWafermapRangeList = createTestsRange(szValue,true,false);
		}

		if(bDataMining)
			ReportOptions.iAdvancedReportSettings = GEX_WAFMAP_TESTOVERDATA;

		return 1;
	}
	if(!qstricmp(szField,"stack_param_over_range"))
	{
		if(checkProfessionalFeature(true) == false)
			return 0;	// STACKING mode requires the Production module or the Professional edition!

		ReportOptions.iWafermapType = GEX_WAFMAP_STACK_TESTOVERDATA;

		if(strstr(szValue,"all") != NULL)
		{
			ReportOptions.iWafermapTests = GEX_WAFMAP_ALL;
			
			// Save list of tests
			ReportOptions.pGexWafermapRangeList = createTestsRange(QString(""),true,false);
		}
		else
		{
			ReportOptions.iWafermapTests= GEX_WAFMAP_LIST;

			// Save list of tests
			ReportOptions.pGexWafermapRangeList = createTestsRange(szValue,true,false);
		}
	
		if(bDataMining)
			ReportOptions.iAdvancedReportSettings = GEX_WAFMAP_STACK_TESTOVERDATA;
		
		return 1;
	}
	if(!qstricmp(szField,"param_passfail"))
	{
		ReportOptions.iWafermapType = GEX_WAFMAP_TEST_PASSFAIL;
	 
		if(strstr(szValue,"all") != NULL)
		{
			ReportOptions.iWafermapTests = GEX_WAFMAP_ALL;
			
			// Save list of tests
			ReportOptions.pGexWafermapRangeList = createTestsRange(QString(""),true,false);
		}
		else
		{
			ReportOptions.iWafermapTests= GEX_WAFMAP_LIST;

			// Save list of tests
			ReportOptions.pGexWafermapRangeList = createTestsRange(szValue,true,false);
		}

		if(bDataMining)
			ReportOptions.iAdvancedReportSettings = GEX_WAFMAP_TEST_PASSFAIL;

		return 1;
	}
	if(!qstricmp(szField,"stack_param_passfail"))
	{
		if(checkProfessionalFeature(true) == false)
			return 0;	// STACKING mode requires the Production module or the Professional edition!

		ReportOptions.iWafermapType = GEX_WAFMAP_STACK_TEST_PASSFAIL;

		if(strstr(szValue,"all") != NULL)
		{
			ReportOptions.iWafermapTests = GEX_WAFMAP_ALL;
			
			// Save list of tests
			ReportOptions.pGexWafermapRangeList = createTestsRange(QString(""),true,false);
		}
		else
		{
			ReportOptions.iWafermapTests= GEX_WAFMAP_LIST;

			// Save list of tests
			ReportOptions.pGexWafermapRangeList = createTestsRange(szValue,true,false);
		}
	
		if(bDataMining)
			ReportOptions.iAdvancedReportSettings = GEX_WAFMAP_STACK_TEST_PASSFAIL;
		
		return 1;
	}
	if(!qstricmp(szField,"zonal_soft_bin"))
	{
 	  if(checkProfessionalFeature(true) == false)
		  return 0;	// ZONAL mode requires the Production module or the Professional edition!

	  ReportOptions.iWafermapType = GEX_WAFMAP_ZONAL_SOFTBIN;
	  // Save list of tests
	  ReportOptions.pGexWafermapRangeList = createTestsRange(szValue,true,false);
	  if(bDataMining)
		ReportOptions.iAdvancedReportSettings = GEX_WAFMAP_ZONAL_SOFTBIN;
	  return 1;
	}
	if(!qstricmp(szField,"zonal_hard_bin"))
	{
 	  if(checkProfessionalFeature(true) == false)
		  return 0;	// ZONAL mode requires the Production module or the Professional edition!

	  ReportOptions.iWafermapType = GEX_WAFMAP_ZONAL_HARDBIN;
	  // Save list of tests
	  ReportOptions.pGexWafermapRangeList = createTestsRange(szValue,true,false);
	  if(bDataMining)
		ReportOptions.iAdvancedReportSettings = GEX_WAFMAP_ZONAL_HARDBIN;
	  return 1;
	}

	// Invalid parameter string.
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Function gexReportType: Let's user define the report sections to create
// Call : gexReportType(section, type, value);
////////////////////////////////////////////////////////////////////////////////
static ZString gexReportType(ZCsl* csl)
{
	char	*szSection="";		// Section name to setup
	char	*szField="";		// Field name to setup
	char	*szValue="";		// Field value
	char	*szCustomValue="";	// Custom value used ie for Volume HL for volume axis viewport (adv Production Report)
	QString strParameterList;	// Holds the copy of the value field
	QString	strTempString;

	int argc = csl->get("argCount").asInt();

	if(argc >= 1) 
	{
		szSection = (char*) csl->get("section").constBuffer();
		ToLower(szSection);
	}
	if(argc >= 2) 
	{
		szField = (char*) csl->get("type").constBuffer();
		ToLower(szField);
	}
	if(argc >= 3) 
	{
		szValue = (char*) csl->get("value").constBuffer();
		// Get copy of the 'value' field in case it is a file path to a test list to import.
		strParameterList = szValue;
		strParameterList = strParameterList.simplifyWhiteSpace();	// Remove any duplicated spaces
		ToLower(szValue);
	}
	if(argc >= 4) 
	{
		szCustomValue = (char*) csl->get("customvalue").constBuffer();
		ToLower(szCustomValue);
	}

	bool	bOemRelease=false;
	switch(ReportOptions.lProductID)
	{
		case GEX_DATATYPE_ALLOWED_CREDENCE_ASL:	// OEM-Examinator for Credence ASL
		case GEX_DATATYPE_ALLOWED_CREDENCE_SAPPHIRE:	// OEM-Examinator for Credence Sapphire
		case GEX_DATATYPE_ALLOWED_LTX:			// OEM-Examinator for LTX
			bOemRelease = true;
			break;
	}

	// Test statistics section
	if(!qstricmp(szSection,"stats"))
	{
		if(!qstricmp(szField,"disabled"))
		{
	      ReportOptions.iStatsType = GEX_STATISTICS_DISABLED;
		  return 1;
		}
		if(!qstricmp(szField,"all"))
		{
	      ReportOptions.iStatsType = GEX_STATISTICS_ALL;
		  return 1;
		}
		if(!qstricmp(szField,"fails"))
		{
	      ReportOptions.iStatsType = GEX_STATISTICS_FAIL;
		  return 1;
		}
		if(!qstricmp(szField,"outliers"))
		{
	      ReportOptions.iStatsType = GEX_STATISTICS_OUTLIERS;
		  return 1;
		}
		if(!qstricmp(szField,"tests"))
		{
			// Dummy user check: in case they select 'Histogram of specific tests'...then specify 'All' in test list!
			if(strstr(szValue,"all") != NULL)
			{
			  ReportOptions.iStatsType = GEX_STATISTICS_ALL;
			  return 1;
			}
		  
	      ReportOptions.iStatsType = GEX_STATISTICS_LIST;
		  
		  // Save list of tests
		  ReportOptions.pGexStatsRangeList = createTestsRange(strParameterList);
		  return 1;
		}
		if(!qstricmp(szField,"cp"))
		{
	      ReportOptions.iStatsType = GEX_STATISTICS_BADCP;
		  sscanf(szValue,"%lf",&ReportOptions.lfStatsLimit);
		  return 1;
		}
		if(!qstricmp(szField,"cpk"))
		{
	      ReportOptions.iStatsType = GEX_STATISTICS_BADCPK;
		  sscanf(szValue,"%lf",&ReportOptions.lfStatsLimit);
		  return 1;
		}

		ZTHROWEXC("invalid parameter :"+ZString(szField));
	}
	
	// Wafer map section
	if(!qstricmp(szSection,"wafer"))
	{
		if(GetWaferMapSettings(szField,szValue,false))
			return 1;

		ZTHROWEXC("invalid parameter :"+ZString(szField));
	}

	// Histogram section
	if(!qstricmp(szSection,"histogram"))
	{
		if(!qstricmp(szField,"disabled"))
		{
	      ReportOptions.iHistogramType = GEX_HISTOGRAM_DISABLED;
		  return 1;
		}

		// Forces standard histogram section to use Advanced histograms instead.
		if(!qstricmp(szField,"advanced"))
		{
	      ReportOptions.bForceAdvancedHisto = true;
		  return 1;
		}
		
		if(!qstricmp(szField,"test_over_limits"))
		{
			ReportOptions.iHistogramType = GEX_HISTOGRAM_OVERLIMITS;
			if(strstr(szValue,"all")!=NULL)
			{
				ReportOptions.iHistogramTests = GEX_HISTOGRAM_ALL;
				return 1;
			}
			ReportOptions.iHistogramTests= GEX_HISTOGRAM_LIST;

			// Save list of tests
			ReportOptions.pGexHistoRangeList = createTestsRange(strParameterList);
			return 1;
		}
		if(!qstricmp(szField,"cumul_over_limits"))
		{
			ReportOptions.iHistogramType = GEX_HISTOGRAM_CUMULLIMITS;
			if(strstr(szValue,"all")!=NULL)
			{
				ReportOptions.iHistogramTests = GEX_HISTOGRAM_ALL;
				return 1;
			}
			ReportOptions.iHistogramTests= GEX_HISTOGRAM_LIST;
			// Save list of tests
			ReportOptions.pGexHistoRangeList = createTestsRange(strParameterList);
			return 1;
		}
		if(!qstricmp(szField,"test_over_range"))
		{
			ReportOptions.iHistogramType = GEX_HISTOGRAM_OVERDATA;
			if(strstr(szValue,"all")!=NULL)
			{
				ReportOptions.iHistogramTests = GEX_HISTOGRAM_ALL;
				return 1;
			}
			ReportOptions.iHistogramTests= GEX_HISTOGRAM_LIST;
			// Save list of tests
			ReportOptions.pGexHistoRangeList = createTestsRange(strParameterList);
			return 1;
		}
		if(!qstricmp(szField,"cumul_over_range"))
		{
			ReportOptions.iHistogramType = GEX_HISTOGRAM_CUMULDATA;
			if(strstr(szValue,"all")!=NULL)
			{
				ReportOptions.iHistogramTests = GEX_HISTOGRAM_ALL;
				return 1;
			}
			ReportOptions.iHistogramTests= GEX_HISTOGRAM_LIST;
			// Save list of tests
			ReportOptions.pGexHistoRangeList = createTestsRange(strParameterList);
			return 1;
		}
		if(!qstricmp(szField,"adaptive"))
		{
			ReportOptions.iHistogramType = GEX_HISTOGRAM_DATALIMITS;
			if(strstr(szValue,"all")!=NULL)
			{
				ReportOptions.iHistogramTests = GEX_HISTOGRAM_ALL;
				return 1;
			}
			ReportOptions.iHistogramTests= GEX_HISTOGRAM_LIST;
			// Save list of tests
			ReportOptions.pGexHistoRangeList = createTestsRange(strParameterList);
			return 1;
		}
		ZTHROWEXC("invalid parameter :"+ZString(szField));
	}

	// Advanced Histogram section
	if(!qstricmp(szSection,"adv_histogram"))
	{
		if(!qstricmp(szField,"disabled"))
		{
	      ReportOptions.iAdvancedReport = GEX_ADV_DISABLED;
		  return 1;
		}
		if(!qstricmp(szField,"test_over_limits"))
		{
			ReportOptions.iAdvancedReport = GEX_ADV_HISTOGRAM;
			ReportOptions.iAdvancedReportSettings = GEX_ADV_HISTOGRAM_OVERLIMITS;

			// Save list of tests
			ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
			return 1;
		}
		if(!qstricmp(szField,"cumul_over_limits"))
		{
			ReportOptions.iAdvancedReport = GEX_ADV_HISTOGRAM;
			ReportOptions.iAdvancedReportSettings = GEX_ADV_HISTOGRAM_CUMULLIMITS;
		
			// Save list of tests
			ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
			return 1;
		}
		if(!qstricmp(szField,"test_over_range"))
		{
			ReportOptions.iAdvancedReport = GEX_ADV_HISTOGRAM;
			ReportOptions.iAdvancedReportSettings = GEX_ADV_HISTOGRAM_OVERDATA;
			
			// Save list of tests
			ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
			return 1;
		}
		if(!qstricmp(szField,"cumul_over_range"))
		{
			ReportOptions.iAdvancedReport = GEX_ADV_HISTOGRAM;
			ReportOptions.iAdvancedReportSettings = GEX_ADV_HISTOGRAM_CUMULDATA;
			
			// Save list of tests
			ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
			return 1;
		}
		if(!qstricmp(szField,"adaptive"))
		{
			ReportOptions.iAdvancedReport = GEX_ADV_HISTOGRAM;
			ReportOptions.iAdvancedReportSettings = GEX_ADV_HISTOGRAM_DATALIMITS;
			
			// Save list of tests
			ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
			return 1;
		}
		ZTHROWEXC("invalid parameter :"+ZString(szField));
	}

	// Advanced Trend section
	if(!qstricmp(szSection,"adv_trend"))
	{
		if(!qstricmp(szField,"disabled"))
		{
	      ReportOptions.iAdvancedReport = GEX_ADV_DISABLED;
		  return 1;
		}
		if(!qstricmp(szField,"test_over_limits"))
		{
			ReportOptions.iAdvancedReport = GEX_ADV_TREND;
			ReportOptions.iAdvancedReportSettings = GEX_ADV_TREND_OVERLIMITS;
			
			// Save list of tests
			ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
			return 1;
		}
		if(!qstricmp(szField,"test_over_range"))
		{
			ReportOptions.iAdvancedReport = GEX_ADV_TREND;
			ReportOptions.iAdvancedReportSettings = GEX_ADV_TREND_OVERDATA;
			
			// Save list of tests
			ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
			return 1;
		}
		if(!qstricmp(szField,"adaptive"))
		{
			ReportOptions.iAdvancedReport = GEX_ADV_TREND;
			ReportOptions.iAdvancedReportSettings = GEX_ADV_TREND_DATALIMITS;
			
			// Save list of tests
			ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
			return 1;
		}
		if(!qstricmp(szField,"difference"))
		{
			ReportOptions.iAdvancedReport = GEX_ADV_TREND;
			ReportOptions.iAdvancedReportSettings = GEX_ADV_TREND_DIFFERENCE;
			
			// Save list of tests...should only be 2 tests.
			ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
			return 1;
		}
		if(!qstricmp(szField,"test_mean"))
		{
			ReportOptions.iAdvancedReport = GEX_ADV_TREND;
			ReportOptions.iAdvancedReportSettings = GEX_ADV_TREND_AGGREGATE_MEAN;
			
			// Save list of tests
			ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
			return 1;
		}
		if(!qstricmp(szField,"test_sigma"))
		{
			ReportOptions.iAdvancedReport = GEX_ADV_TREND;
			ReportOptions.iAdvancedReportSettings = GEX_ADV_TREND_AGGREGATE_SIGMA;
			
			// Save list of tests
			ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
			return 1;
		}
		if(!qstricmp(szField,"test_cp"))
		{
			ReportOptions.iAdvancedReport = GEX_ADV_TREND;
			ReportOptions.iAdvancedReportSettings = GEX_ADV_TREND_AGGREGATE_CP;
			
			// Save list of tests
			ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
			return 1;
		}
		if(!qstricmp(szField,"test_cpk"))
		{
			ReportOptions.iAdvancedReport = GEX_ADV_TREND;
			ReportOptions.iAdvancedReportSettings = GEX_ADV_TREND_AGGREGATE_CPK;
			
			// Save list of tests
			ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
			return 1;
		}
		if(!qstricmp(szField,"soft_bin_sublots"))
		{
			ReportOptions.iAdvancedReport = GEX_ADV_TREND;
			ReportOptions.iAdvancedReportSettings = GEX_ADV_TREND_SOFTBIN_SBLOTS;
			
			// Save SoftBin# list
			ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
			return 1;
		}
		if(!qstricmp(szField,"soft_bin_parts"))
		{
			ReportOptions.iAdvancedReport = GEX_ADV_TREND;
			ReportOptions.iAdvancedReportSettings = GEX_ADV_TREND_SOFTBIN_PARTS;
			
			// Save SoftBin# list
			ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
			return 1;
		}
		if(!qstricmp(szField,"hard_bin_sublots"))
		{
			ReportOptions.iAdvancedReport = GEX_ADV_TREND;
			ReportOptions.iAdvancedReportSettings = GEX_ADV_TREND_HARDBIN_SBLOTS;
			
			// Save HardBin# list
			ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
			return 1;
		}
		if(!qstricmp(szField,"hard_bin_parts"))
		{
			ReportOptions.iAdvancedReport = GEX_ADV_TREND;
			ReportOptions.iAdvancedReportSettings = GEX_ADV_TREND_HARDBIN_PARTS;
			
			// Save HardBin# list
			ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
			return 1;
		}
		if(!qstricmp(szField,"soft_bin_rolling"))
		{
			// Only accepted under GEX-Production, Professional edition!
			if(checkProfessionalFeature(true) == false)
				return 1;

			ReportOptions.iAdvancedReport = GEX_ADV_TREND;
			ReportOptions.iAdvancedReportSettings = GEX_ADV_TREND_SOFTBIN_ROLLING;
			
			// Save SoftBin# list
			ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
			return 1;
		}
		if(!qstricmp(szField,"hard_bin_rolling"))
		{
			// Only accepted under GEX-Production, Professional edition!
			if(checkProfessionalFeature(true) == false)
				return 1;

			ReportOptions.iAdvancedReport = GEX_ADV_TREND;
			ReportOptions.iAdvancedReportSettings = GEX_ADV_TREND_HARDBIN_ROLLING;
			
			// Save HardBin# list
			ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
			return 1;
		}
		ZTHROWEXC("invalid parameter :"+ZString(szField));
	}

	// Advanced Scatter/Correlation section
	if(!qstricmp(szSection,"adv_correlation"))
	{
		if(!qstricmp(szField,"disabled"))
		{
	      ReportOptions.iAdvancedReport = GEX_ADV_DISABLED;
		  return 1;
		}
		if(!qstricmp(szField,"test_over_limits"))
		{
			ReportOptions.iAdvancedReport = GEX_ADV_CORRELATION;
			ReportOptions.iAdvancedReportSettings = GEX_ADV_CORR_OVERLIMITS;

			// Save Test pairs list
			ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
			return 1;
		}
		if(!qstricmp(szField,"test_over_range"))
		{
			ReportOptions.iAdvancedReport = GEX_ADV_CORRELATION;
			ReportOptions.iAdvancedReportSettings = GEX_ADV_CORR_OVERDATA;

			// Save Test pairs list
			ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
			return 1;
		}
		ZTHROWEXC("invalid parameter :"+ZString(szField));
	}
	
	// Advanced Boxplot section
	if(!qstricmp(szSection,"adv_probabilityplot"))
	{
		if(!qstricmp(szField,"disabled"))
		{
	      ReportOptions.iAdvancedReport = GEX_ADV_DISABLED;
		  return 1;
		}
	
		if(!qstricmp(szField,"test_over_limits"))
		{
			ReportOptions.iAdvancedReport = GEX_ADV_PROBABILITY_PLOT;
			ReportOptions.iAdvancedReportSettings = GEX_ADV_PROBPLOT_OVERLIMITS;

			// Save list of tests
			ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
			return 1;
		}
		if(!qstricmp(szField,"test_over_range"))
		{
			ReportOptions.iAdvancedReport = GEX_ADV_PROBABILITY_PLOT;
			ReportOptions.iAdvancedReportSettings = GEX_ADV_PROBPLOT_OVERDATA;
			
			// Save list of tests
			ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
			return 1;
		}
		if(!qstricmp(szField,"adaptive"))
		{
			ReportOptions.iAdvancedReport = GEX_ADV_PROBABILITY_PLOT;
			ReportOptions.iAdvancedReportSettings = GEX_ADV_PROBPLOT_DATALIMITS;
			
			// Save list of tests
			ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
			return 1;
		}
		ZTHROWEXC("invalid parameter :"+ZString(szField));
	}

	// Advanced Boxplot section
	if(!qstricmp(szSection,"adv_boxplot_ex"))
	{
		if(!qstricmp(szField,"disabled"))
		{
	      ReportOptions.iAdvancedReport = GEX_ADV_DISABLED;
		  return 1;
		}
	
		if(!qstricmp(szField,"test_over_limits"))
		{
			ReportOptions.iAdvancedReport = GEX_ADV_BOXPLOT;
			ReportOptions.iAdvancedReportSettings = GEX_ADV_BOXPLOT_OVERLIMITS;

			// Save list of tests
			ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
			return 1;
		}
		if(!qstricmp(szField,"test_over_range"))
		{
			ReportOptions.iAdvancedReport = GEX_ADV_BOXPLOT;
			ReportOptions.iAdvancedReportSettings = GEX_ADV_BOXPLOT_OVERDATA;
			
			// Save list of tests
			ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
			return 1;
		}
		if(!qstricmp(szField,"adaptive"))
		{
			ReportOptions.iAdvancedReport = GEX_ADV_BOXPLOT;
			ReportOptions.iAdvancedReportSettings = GEX_ADV_BOXPLOT_DATALIMITS;
			
			// Save list of tests
			ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
			return 1;
		}
		ZTHROWEXC("invalid parameter :"+ZString(szField));
	}

	// Advanced Boxplot section
	if(!qstricmp(szSection,"adv_boxplot"))
	{
		if(!qstricmp(szField,"disabled"))
		{
	      ReportOptions.iAdvancedReport = GEX_ADV_DISABLED;
		  return 1;
		}
		if(!qstricmp(szField,"tests"))
		{
			ReportOptions.iAdvancedReport = GEX_ADV_CANDLE_MEANRANGE;
			if(strstr(szValue,"all")!=NULL)
			{
				ReportOptions.iAdvancedReportSettings = GEX_ADV_ALL;
				// Save list of tests: Take them ALL!
				QString strAllTests;
				strAllTests = QString::number(GEX_MINTEST) + " to " + QString::number(GEX_MAXTEST);
				
				// Save Test list
				ReportOptions.pGexAdvancedRangeList = createTestsRange(strAllTests,true,true);
				return 1;
			}
			
			ReportOptions.iAdvancedReportSettings= GEX_ADV_LIST;

			// Save Test list
			ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
			return 1;
		}
		ZTHROWEXC("invalid parameter :"+ZString(szField));
	}

	// Advanced Datalog section
	if(!qstricmp(szSection,"adv_datalog"))
	{
		if(!qstricmp(szField,"disabled"))
		{
	      ReportOptions.iAdvancedReport = GEX_ADV_DISABLED;
		  return 1;
		}
		if(strstr(szField,"all")!=NULL)
		{
			ReportOptions.iAdvancedReport = GEX_ADV_DATALOG;
			ReportOptions.iAdvancedReportSettings = GEX_ADV_DATALOG_ALL;
			return 1;
		}
		if(!qstricmp(szField,"outliers"))
		{
			ReportOptions.iAdvancedReport = GEX_ADV_DATALOG;
			ReportOptions.iAdvancedReportSettings = GEX_ADV_DATALOG_OUTLIER;
		
			// Save list of tests
			ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
			return 1;
		}
		if(!qstricmp(szField,"fails"))
		{
			ReportOptions.iAdvancedReport = GEX_ADV_DATALOG;
			ReportOptions.iAdvancedReportSettings = GEX_ADV_DATALOG_FAIL;
			return 1;
		}
		if(!qstricmp(szField,"tests"))
		{
			ReportOptions.iAdvancedReport = GEX_ADV_DATALOG;
			ReportOptions.iAdvancedReportSettings = GEX_ADV_DATALOG_LIST;
			
			// Save list of tests
			ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
			return 1;
		}
		if(!qstricmp(szField,"tests_only"))
		{
			ReportOptions.iAdvancedReport = GEX_ADV_DATALOG;
			ReportOptions.iAdvancedReportSettings = GEX_ADV_DATALOG_RAWDATA;
			
			// Save list of tests
			ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
			return 1; 
		}
		ZTHROWEXC("invalid parameter :"+ZString(szField));
	}

	// Advanced PRODUCTION Reports (YIELD) section
	if(!qstricmp(szSection,"adv_production_yield"))
	{
		// Only valid under Examinator Production, Professional edition.
 		if(checkProfessionalFeature(true) == false)
			return 1;	

		// Specify information to trend
		if(!qstricmp(szField,"sublot"))
		{
			ReportOptions.iAdvancedReport = GEX_ADV_PROD_YIELD;
			ReportOptions.iAdvancedReportSettings = GEX_ADV_PRODYIELD_SBLOT;
			// Save Bin list
			ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
			return 1;	// No error.
		}
		if(!qstricmp(szField,"lot"))
		{
			ReportOptions.iAdvancedReport = GEX_ADV_PROD_YIELD;
			ReportOptions.iAdvancedReportSettings = GEX_ADV_PRODYIELD_LOT;
			// Save Bin list
			ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
			return 1;	// No error.
		}
		if(!qstricmp(szField,"group"))
		{
			ReportOptions.iAdvancedReport = GEX_ADV_PROD_YIELD;
			ReportOptions.iAdvancedReportSettings = GEX_ADV_PRODYIELD_GROUP;
			// Save Bin list
			ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
			return 1;	// No error.
		}
		if(!qstricmp(szField,"daily"))
		{
			ReportOptions.iAdvancedReport = GEX_ADV_PROD_YIELD;
			ReportOptions.iAdvancedReportSettings = GEX_ADV_PRODYIELD_DAY;
			// Save Bin list
			ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
			return 1;	// No error.
		}
		if(!qstricmp(szField,"weekly"))
		{
			ReportOptions.iAdvancedReport = GEX_ADV_PROD_YIELD;
			ReportOptions.iAdvancedReportSettings = GEX_ADV_PRODYIELD_WEEK;
			// Save Bin list
			ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
			return 1;	// No error.
		}
		if(!qstricmp(szField,"monthly"))
		{
			ReportOptions.iAdvancedReport = GEX_ADV_PROD_YIELD;
			ReportOptions.iAdvancedReportSettings = GEX_ADV_PRODYIELD_MONTH;
			// Save Bin list
			ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
			return 1;	// No error.
		}
		// Specify chart title
		if(!qstricmp(szField,"title"))
		{
			ReportOptions.strProdReportTitle = strParameterList; // Use strString instead of szValue, because szValue has been lower-cased
			return 1;	// No error.
		}
		// Yield Axis options
		if(!qstricmp(szField,"yield_axis"))
		{
			if(!qstricmp(szValue,"0-100"))
				ReportOptions.uiYieldAxis = GEX_ADV_PROD_YIELDAXIS_0_100;
			if(!qstricmp(szValue,"0-max"))
				ReportOptions.uiYieldAxis = GEX_ADV_PROD_YIELDAXIS_0_MAX;
			if(!qstricmp(szValue,"min-100"))
				ReportOptions.uiYieldAxis = GEX_ADV_PROD_YIELDAXIS_MIN_100;
			if(!qstricmp(szValue,"min-max"))
				ReportOptions.uiYieldAxis = GEX_ADV_PROD_YIELDAXIS_MIN_MAX;
			return 1;	// No error.
		}
		// Volume Axis options
		if(!qstricmp(szField,"volume_axis"))
		{
			if(!qstricmp(szValue,"0-max"))
				ReportOptions.uiVolumeAxis = GEX_ADV_PROD_VOLUMEAXIS_0_MAX;
			if(!qstricmp(szValue,"0-custom"))
			{
				strTempString = szCustomValue;
				ReportOptions.uiVolumeAxis = GEX_ADV_PROD_VOLUMEAXIS_0_CUSTOM;
				ReportOptions.uiVolumeHL = strTempString.toUInt();
			}
			if(!qstricmp(szValue,"min-max"))
				ReportOptions.uiVolumeAxis = GEX_ADV_PROD_VOLUMEAXIS_MIN_MAX;
			if(!qstricmp(szValue,"min-custom"))
			{
				strTempString = szCustomValue;
				ReportOptions.uiVolumeAxis = GEX_ADV_PROD_VOLUMEAXIS_MIN_CUSTOM;
				ReportOptions.uiVolumeHL = strTempString.toUInt();
			}
			return 1;	// No error.
		}
		// Yield marker
		if(!qstricmp(szField,"yield_marker"))
		{
			strTempString = szValue;
			ReportOptions.uiYieldMarker = strTempString.toUInt();
			return 1;	// No error.
		}

		ZTHROWEXC("invalid parameter :"+ZString(szField));
	}

	// Test to test correlation (for Test time optimization): Pearson's correlation value
	if(!qstricmp(szSection,"adv_pearson"))
	{
		ReportOptions.iAdvancedReport = GEX_ADV_PEARSON;
		return 1;
	}

	// PAT Traceability report: extract text logged into STDF.DTR records during PAT off-line processing
	if(!qstricmp(szSection,"adv_pat"))
	{
		ReportOptions.iAdvancedReport = GEX_ADV_PAT_TRACEABILITY;
		return 1;
	}

	// PAT-Man: triggering a 'Outlier Removal' report generation (following the Outlier sweeping over a STDF file)
	if(!qstricmp(szSection,"adv_outlier_removal"))
	{
		if(!qstricmp(szField,"disabled"))
		{
	      ReportOptions.iAdvancedReport = GEX_ADV_DISABLED;
		  return 1;
		}
		if(!qstricmp(szField,"enabled"))
		{
	      ReportOptions.iAdvancedReport = GEX_ADV_OUTLIER_REMOVAL;
		  return 1;
		}
		ZTHROWEXC("invalid parameter :"+ZString(szField));
	}

	// Advanced Optimizer Diags section
	if(!qstricmp(szSection,"adv_optimizer"))
	{
		if(!qstricmp(szField,"disabled"))
		{
	      ReportOptions.iAdvancedReport = GEX_ADV_DISABLED;
		  return 1;
		}
		if(!qstricmp(szField,"all"))
		{
			// 
			ReportOptions.iAdvancedReport = GEX_ADV_GO_DIAGNOSTICS;
			ReportOptions.iAdvancedReportSettings = GEX_ADV_TREND_OVERLIMITS;
			// Save list of tests: Take them ALL!
			QString strAllTests;
			strAllTests = QString::number(GEX_MINTEST) + " to " + QString::number(GEX_MAXTEST);
			ReportOptions.pGexAdvancedRangeList = createTestsRange(strAllTests);
			return 1;
		}
		ZTHROWEXC("invalid parameter :"+ZString(szField));
	}

	// Interactive Drill section: DataMining-Yield Analysis
	if(!qstricmp(szSection,"drill_disabled"))
	{
	      ReportOptions.iDrillReport = -1;
		  return 1;
	}

	if(!qstricmp(szSection,"drill_what_if"))
	{
		ReportOptions.iDrillReport = GEX_DRILL_GUARDBANDING;
		ReportOptions.iAdvancedReport = GEX_ADV_GUARDBANDING;
		return 1;
	}

	if(!qstricmp(szSection,"drill_wafer"))
	{
		// Extract Drill type (structure shared with GEX-InstantReport)
		ReportOptions.iDrillReport = GEX_DRILL_WAFERPROBE;
		if(GetWaferMapSettings(szField,szValue,true))
			return 1;
	}
	if(!qstricmp(szSection,"drill_packaged"))
	{
		// Extract Drill type (structure shared with GEX-InstantReport)
		ReportOptions.iDrillReport = GEX_DRILL_PACKAGED;
		if(GetWaferMapSettings(szField,szValue,true))
			return 1;
	}
	if(!qstricmp(szSection,"drill_histo"))
	{
		// Extract Drill type (structure shared with GEX-InstantReport)
		ReportOptions.iDrillReport = GEX_DRILL_HISTOGRAM;
		return 1;
	}
	if(!qstricmp(szSection,"drill_trend"))
	{
		// Extract Drill type (structure shared with GEX-InstantReport)
		ReportOptions.iDrillReport = GEX_DRILL_TREND;
		return 1;
	}
	if(!qstricmp(szSection,"drill_correlation"))
	{
		// Extract Drill type (structure shared with GEX-InstantReport)
		ReportOptions.iDrillReport = GEX_DRILL_CORRELATION;
		return 1;
	}

	// MyReports section
	if(!qstricmp(szSection,"adv_my_report"))
	{
		// function only accepted if Examinator Production, Professional
		if(!bOemRelease && checkProfessionalFeature(false) == false)
			return 0;

		if(!qstricmp(szField,"template"))
		{
			// Get Template file to process
			ReportOptions.strTemplateFile = szValue;	
			return 1;
		}

		if(!qstricmp(szField,"disabled"))
		{
			// Get Template file to process
			ReportOptions.strTemplateFile = "";	
			return 1;
		}
		ZTHROWEXC("invalid parameter :"+ZString(szField));
	}

	// Global advanced options
	if(!qstricmp(szSection,"adv_global_options"))
	{
		if(!qstricmp(szField,"binning"))
		{
			// Get which binning to focus on (hard-bin or soft bin)
			if(!qstricmp(szValue,"hard_bin"))
				ReportOptions.bAdvancedUsingSoftBin = false;	
			else
				ReportOptions.bAdvancedUsingSoftBin = true;	
			return 1;
		}
		ZTHROWEXC("invalid parameter :"+ZString(szField));
	}

	ZTHROWEXC("invalid parameter :"+ZString(szSection));
	return 0;
} // gexReportType

////////////////////////////////////////////////////////////////////////////////
// Function gexFileInfo: Extracts info about a given data file
// Call : gexFileInfo(file_name, field);
////////////////////////////////////////////////////////////////////////////////
static ZString gexFileInfo(ZCsl* csl)
{
	char	*szField;
	int		group_id=0;
	int		file_id=0;

	szField = (char*) csl->get("group_id").constBuffer();
	sscanf(szField,"%d",&group_id);
	szField = (char*) csl->get("file_id").constBuffer();
	sscanf(szField,"%d",&file_id);
	szField = (char*) csl->get("field").constBuffer();

	// Only valid under Examinator Production, Professional edition.
 	if(checkProfessionalFeature(true) == false)
		return 0;	

	// Check if valid handle to main GUI window
	if(pGexMainWindow == NULL)
		return 0;

	// If processing error
	if(gexReport == NULL)
		return 0;

	CGexGroupOfFiles *pGroup;
	CGexFileInGroup *pFile;
	if((group_id < 0) || (group_id >= gexReport->pGroupsList.size()))
		return 0;
	pGroup = gexReport->pGroupsList.at(group_id);
	if(pGroup == NULL)
		return 0;
	if(file_id >= pGroup->pFilesList.size())
		return 0;
	pFile = pGroup->pFilesList.at(file_id);
	if(pFile == NULL)
		return 0;

	if(!qstricmp(szField,"file_name"))
	{
		// Return original file name.
		return pFile->strFileName.latin1();
	}
	if(!qstricmp(szField,"stdf_file_name"))
	{
		// Return original file name.
		return pFile->strFileNameSTDF.latin1();
	}
	if(!qstricmp(szField,"stdf_file_name"))
	{
		// Return original file name.
		return pFile->strFileNameSTDF.latin1();
	}
	if(!qstricmp(szField,"program_name"))
	{
		// Program name.
		return pFile->MirData.szJobName;
	}
	if(!qstricmp(szField,"program_version"))
	{
		// Program name.
		return pFile->MirData.szJobRev;
	}
	if(!qstricmp(szField,"lot_id"))
	{
		// Lot ID.
		return pFile->MirData.szLot;
	}
	if(!qstricmp(szField,"wafer_id"))
	{
		// Wafere ID.
		return pFile->WaferMapData.szWaferID;
	}
	if(!qstricmp(szField,"tester_name"))
	{
		// Tester name.
		return pFile->MirData.szNodeName;
	}
	if(!qstricmp(szField,"tester_type"))
	{
		// Tester type.
		return pFile->MirData.szTesterType;
	}
	if(!qstricmp(szField,"product"))
	{
		// Part Type type.
		return pFile->MirData.szPartType;
	}
	if(!qstricmp(szField,"operator"))
	{
		// Part Type type.
		return pFile->MirData.szOperator;
	}
	if(!qstricmp(szField,"exec_type"))
	{
		// Exec type.
		return pFile->MirData.szExecType;
	}
	if(!qstricmp(szField,"exec_version"))
	{
		// Exec version.
		return pFile->MirData.szExecVer;
	}
	if(!qstricmp(szField,"good_parts"))
	{
		// Total good parts.
		return pFile->MirData.lGoodCount;
	}
	if(!qstricmp(szField,"bad_parts"))
	{
		// Total Failing parts.
		return pFile->MirData.lPartCount - pFile->MirData.lGoodCount;
	}
	if(!qstricmp(szField,"total_parts"))
	{
		// Total parts tested
		return pFile->MirData.lPartCount;
	}
	if(!qstricmp(szField,"facility_id"))
	{
		// Facility ID.
		return pFile->MirData.szFacilityID;
	}
	if(!qstricmp(szField,"family_id"))
	{
		// Family ID.
		return pFile->MirData.szFamilyID;
	}
	if(!qstricmp(szField,"floor_id"))
	{
		// Floor ID.
		return pFile->MirData.szFloorID;
	}
	if(!qstricmp(szField,"flow_id"))
	{
		// Flow ID.
		return pFile->MirData.szFlowID;
	}
	if(!qstricmp(szField,"handler_id"))
	{
		// Hanlder/Prober ID.
		return pFile->MirData.szHandlerProberID;
	}
	if(!qstricmp(szField,"package_id"))
	{
		// Package ID.
		return pFile->MirData.szPkgType;
	}
	if(!qstricmp(szField,"probecard_id"))
	{
		// Probe card ID.
		return pFile->MirData.szProbeCardID;
	}
	if(!qstricmp(szField,"serial_id"))
	{
		// Serial ID.
		return pFile->MirData.szSerialNumber;
	}
	if(!qstricmp(szField,"setup_id"))
	{
		// setup ID.
		return pFile->MirData.szSetupID;
	}
	if(!qstricmp(szField,"spec_name"))
	{
		// spec name.
		return pFile->MirData.szSpecName;
	}
	if(!qstricmp(szField,"spec_version"))
	{
		// spec name.
		return pFile->MirData.szSpecVersion;
	}
	if(!qstricmp(szField,"sublot_id"))
	{
		// sublot ID.
		return pFile->MirData.szSubLot;
	}
	if(!qstricmp(szField,"temperature"))
	{
		// Temperature ID.
		return pFile->MirData.szTestTemperature;
	}
	if(!qstricmp(szField,"user_text"))
	{
		// Temperature ID.
		return pFile->MirData.szUserText;
	}
	if(!qstricmp(szField,"aux_file"))
	{
		// Temperature ID.
		return pFile->MirData.szAuxFile;
	}

	return 0;
}

void ZCslInitLib_Gex(ZCsl* csl)
{
   ZString iFile("GexLib");
   std::istringstream init("const myVersion = 3.00;\n");
   csl->loadScript(iFile, &init);
   (*csl)
      .addFunc(
          iFile,
         "gexBuildReport([const show_page,const sections_ignored])",
          gexBuildReport)
      .addFunc(
          iFile,
         "gexConvertSTDF(const input_file, [const output_file])",
          gexConvertSTDF)
      .addFunc(
          iFile,
         "gexConvertCSV(const input_file, [const output_file])",
          gexConvertCSV)
      .addFunc(
          iFile,
         "gexWeb(const action, [const field, const value])",
          gexWeb)
      .addFunc(
          iFile,
         "gexParameter(const parameter, const name, const field, const value,const label)",
          gexParameter)
      .addFunc(
          iFile,
         "gexParameterFile(const parameter)",
          gexParameterFile)
      .addFunc(
          iFile,
         "gexToolbox(const action, const field, [const value, const optional])",
          gexToolbox)
      .addFunc(
          iFile,
         "gexFavoriteScripts(const action, [const path, const title])",
          gexFavoriteScripts)	  
		  .addFunc(
          iFile,
         "gexFile(const group_id, const action, const file, const site, const process, [const range, const maptests, const extractwafer, const temperature, const datasetname])",
          gexFile)
      .addFunc(
          iFile,
         "gexFileInfo(const group_id, const file_id, const field)",
          gexFileInfo)
      .addFunc(
          iFile,
         "gexGroup(const action, const value)",
          gexGroup)
      .addFunc(
          iFile,
         "gexOptions(const section, const field, const value, [const setvalue])",
          gexOptions)
      .addFunc(
          iFile,
         "gexInteractive(const section, [const field, const value])",
          gexInteractive)
      .addFunc(
          iFile,
         "gexQuery(const section, const field, [const value])",
          gexQuery)
      .addFunc(
          iFile,
         "gexReportType(const section, [const type, const value, const customvalue])",
          gexReportType);

   // Erase previous report structure if any. Clean start!
	if(gexReport != NULL)
		delete gexReport;
	gexReport = new CGexReport;
	gexReport->pReportOptions = NULL;	// Will remain NULL unless a report is created...

} // ZCslInitLib

void ZCslCleanupLib_Gex(ZCsl* csl)
{
   // cleanup codes.
	if(ptStdfToAscii !=  NULL)
		delete ptStdfToAscii;
	ptStdfToAscii=NULL;

} // ZCslCleanupLib
