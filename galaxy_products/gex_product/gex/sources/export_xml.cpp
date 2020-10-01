//! \file export_xml.cpp: Converts a STDF file to XML

#include <QProgressBar>

#include "gex_shared.h"
#include "export_xml.h"
#include "engine.h"
#include "stdfparse.h"

// Output File format:
// <STDF>
//		<FAR>
//			<cpu_type>0</cpu_type> 
//			<stdf_ver>0</stdf_ver> 
//		</FAR>
// ...
// </STDF>

// Error map
GBEGIN_ERROR_MAP(CSTDFtoXML)
	GMAP_ERROR(eOpenReadFail,"Failed to open STDF file:\n%s.")
	GMAP_ERROR(eOpenWriteFail,"Failed to create XML file:\n%s.")
	GMAP_ERROR(eLicenceExpired,QString("License has expired or Data file\n%s out of date...\nPlease contact  %1").arg(GEX_EMAIL_SALES).toLatin1().constData())
	GMAP_ERROR(eStdfRead,"Failed to read corrupted STDF file:\n%s.")
GEND_ERROR_MAP(CSTDFtoXML)

CSTDFtoXML::CSTDFtoXML()
{
	ptTestList = NULL;
	m_nStdfRecords = 0;	
	Clear();
}

//////////////////////////////////////////////////////////////////////
// Destruction
//////////////////////////////////////////////////////////////////////
CSTDFtoXML::~CSTDFtoXML()
{
	Clear();
}

void CSTDFtoXML::Clear()
{
	CXmlShortTest* ptTest;

	while(ptTestList != NULL)
	{
		ptTest = ptTestList;
		ptTestList = ptTestList->ptNextTest;
		delete ptTest;
	}

	m_nPTRCount = 0;
	m_nMPRCount = 0;
	m_nFTRCount = 0;
	m_nMainSequenceIndentationLevel = 0;
	m_nInitialSequenceIndentationLevel = 0;
	m_nPinmapSequenceIndentationLevel = 0;
	m_nLimitsSequenceIndentationLevel = 0;
	m_nResultsSequenceIndentationLevel = 0;
	m_nSummarySequenceIndentationLevel = 0;
	m_nFinalSequenceIndentationLevel = 0;
	m_bAddToLimits = true;
	m_bTestProcessed = false;
	m_bPartStarted = false;
	m_nProgress = 0;
}

//////////////////////////////////////////////////////////////////////
// Write XML Header section (starting file)
//////////////////////////////////////////////////////////////////////
void CSTDFtoXML::WriteXmlHeader(QTextStream & hStream)
{
	// Write XML header
	hStream  << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;
    hStream  << "<!--Semiconductor Yield Analysis is easy with Quantix!-->" << endl;
    hStream  << "<!--Report created with: "
      << GS::Gex::Engine::GetInstance().Get("AppFullName").toString()
      << " - www.mentor.com-->" << endl;
	hStream  << "<stdf" << endl;
	hStream  << "xmlns=\"http://www.galaxysemi.com/STDFSchema\"" << endl;
	hStream  << "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"" << endl;
	hStream  << "xsi:schemaLocation=\"http://www.galaxysemi.com/STDFSchema stdf.xsd\">" << endl;

	m_nMainSequenceIndentationLevel++;
	m_nInitialSequenceIndentationLevel++;
	m_nPinmapSequenceIndentationLevel++;
	m_nLimitsSequenceIndentationLevel++;
	m_nResultsSequenceIndentationLevel++;
	m_nSummarySequenceIndentationLevel++;
	m_nFinalSequenceIndentationLevel++;
}

//////////////////////////////////////////////////////////////////////
// Write XML Footer section (end of file)
//////////////////////////////////////////////////////////////////////
void CSTDFtoXML::WriteXmlFooter(QTextStream & hStream)
{
	m_nMainSequenceIndentationLevel--;
	m_nInitialSequenceIndentationLevel--;
	m_nPinmapSequenceIndentationLevel--;
	m_nLimitsSequenceIndentationLevel--;
	m_nResultsSequenceIndentationLevel--;
	m_nSummarySequenceIndentationLevel--;
	m_nFinalSequenceIndentationLevel--;

	// Close XML STDF bloc.
	hStream  << "</stdf>" << endl;
}

CXmlShortTest::CXmlShortTest()
{
	ptNextTest  = NULL;
	bTestExecuted=false;
	lTestNumber=0;			// TestNumber
	res_scal = 0;			// No scaling
	llm_scal = 0;
	hlm_scal = 0;
	bTestType = ' ';		// TestType: Parametric,Functional,...
	lfLowLimit=0;			// Test Low Limit
	lfHighLimit=0;			// Test High Limit
	bLimitFlag=CTEST_LIMITFLG_NOLTL | CTEST_LIMITFLG_NOHTL | CTEST_LIMITFLG_NOLSL | CTEST_LIMITFLG_NOHSL;	// No limits defined
	strTestName="";			// Test Name.
	*szTestLabel=0;			// TestNumber.Pinmap# ...string used in HTML, XML reports
	*szTestUnits= 0;		// Test Units.
	*szLowL= 0;				// Test low limit.
	*szHighL= 0;			// Test low limit.
}

//////////////////////////////////////////////////////////////////////
// Get Error
//////////////////////////////////////////////////////////////////////
QString CSTDFtoXML::GetLastError()
{
	QString strLastError = "Export XML:\n";
	strLastError += GGET_LASTERRORMSG(CSTDFtoXML,this);

	// Return Error Message
	return strLastError;
}

//////////////////////////////////////////////////////////////////////
// Convert 'szFileNameSTDF' STDF file, to XML 'szXmlFileName' file
//////////////////////////////////////////////////////////////////////
bool CSTDFtoXML::Convert(const char *szFileNameSTDF, const char *szXmlFileName, QProgressBar* pProgressBar /*= NULL*/)
{
	// Reset record counter
	m_nStdfRecords = 0;	

	// Reset variables
	PartInfo.lExecutionTime = 0;
	PartInfo.lPartNumber = 0;

	// Open XML files
	if(OpenFiles(szFileNameSTDF, szXmlFileName) == false)
	{
		// Error. Can't create XML file!
		return false;
	}

	// Set STDF file name
	m_strStdfFile = szFileNameSTDF;

	// Read all file...and build XML file as we go...
	int	iStatus, nRecordType;

	// Pass 1: count nb of records (for eventual progress bar), and in debug dump STDF structure
	// Pass 2: create XML file
	for(lPass = 1;lPass <=2; lPass++)
	{
		// Clear data between passes
		Clear();

		// Open STDF file to read...
		iStatus = m_clStdfParse.Open((char *)szFileNameSTDF);
		if(iStatus == false)
		{
			// Error. Can't open STDF file in read mode!
			GSET_ERROR1(CSTDFtoXML, eOpenReadFail, NULL, szFileNameSTDF);
			// Close XML files
			CloseFiles();
			return false;
		}

		// Read one record from STDF file.
		iStatus = m_clStdfParse.LoadNextRecord(&nRecordType);			
        while(iStatus == GQTL_STDF::StdfParse::NoError)
		{
			// Do some common PASS 1 stuff
			if(lPass == 1)
			{
				// Update counters
				m_nStdfRecords++;
			}

			// Do some common PASS 2 stuff
			if(lPass == 2)
			{
				// Update progress bar
				UpdateProgressBar(pProgressBar);
			}
			
			// Process STDF record read.
			switch(nRecordType)
			{
                case GQTL_STDF::Stdf_Record::Rec_FAR:
					ProcessFAR();
					break;
                case GQTL_STDF::Stdf_Record::Rec_ATR:
					ProcessATR();
					break;
                case GQTL_STDF::Stdf_Record::Rec_MIR:
                    if(ProcessMIR() != true)
					{
						m_clStdfParse.Close();	// Close STDF file
						// Close XML files
						CloseFiles();
						return false;		// File timestamp is invalid, and higher than expiration date!
					}
					break;
                case GQTL_STDF::Stdf_Record::Rec_MRR:
					ProcessMRR();
					break;
                case GQTL_STDF::Stdf_Record::Rec_PCR:
					ProcessPCR();
					break;
                case GQTL_STDF::Stdf_Record::Rec_HBR:
					ProcessHBR();
					break;
                case GQTL_STDF::Stdf_Record::Rec_SBR:
					ProcessSBR();
					break;
                case GQTL_STDF::Stdf_Record::Rec_PMR:
					ProcessPMR();
					break;
                case GQTL_STDF::Stdf_Record::Rec_PGR:
					ProcessPGR();
					break;
                case GQTL_STDF::Stdf_Record::Rec_PLR:
					ProcessPLR();
					break;
                case GQTL_STDF::Stdf_Record::Rec_RDR:
					ProcessRDR();
					break;
                case GQTL_STDF::Stdf_Record::Rec_SDR:
					ProcessSDR();
					break;
                case GQTL_STDF::Stdf_Record::Rec_WIR:
					ProcessWIR();
					break;
                case GQTL_STDF::Stdf_Record::Rec_WRR:
					ProcessWRR();
					break;
                case GQTL_STDF::Stdf_Record::Rec_WCR:
					ProcessWCR();
					break;
                case GQTL_STDF::Stdf_Record::Rec_PIR:
					ProcessPIR();
					break;
                case GQTL_STDF::Stdf_Record::Rec_PRR:
					ProcessPRR();
					break;
                case GQTL_STDF::Stdf_Record::Rec_TSR:
					ProcessTSR();
					break;
                case GQTL_STDF::Stdf_Record::Rec_PTR:
					ProcessPTR();
					break;
                case GQTL_STDF::Stdf_Record::Rec_MPR:
					ProcessMPR();
					break;
                case GQTL_STDF::Stdf_Record::Rec_FTR:
					ProcessFTR();
					break;
                case GQTL_STDF::Stdf_Record::Rec_BPS:
					ProcessBPS();
					break;
                case GQTL_STDF::Stdf_Record::Rec_EPS:
					ProcessEPS();
					break;
                case GQTL_STDF::Stdf_Record::Rec_GDR:
					ProcessGDR();
					break;
                case GQTL_STDF::Stdf_Record::Rec_DTR:
					ProcessDTR();
					break;
                case GQTL_STDF::Stdf_Record::Rec_RESERVED_IMAGE:
					ProcessRESERVED_IMAGE();
					break;
                case GQTL_STDF::Stdf_Record::Rec_RESERVED_IG900:
					ProcessRESERVED_IG900();
					break;
                case GQTL_STDF::Stdf_Record::Rec_UNKNOWN:
					ProcessUNKNOWN();
					break;
			}
			// Read one record from STDF file.
			iStatus = m_clStdfParse.LoadNextRecord(&nRecordType);			
		};

		// Close input STDF file between passes.
		m_clStdfParse.Close();
	}	// 2 passes.

	// Convertion successful, close XML files.
	CloseFiles();

	return true;
}

/////////////////////////////////////////////////////////////////////////////
// Find Test Cell structure in Test List.
/////////////////////////////////////////////////////////////////////////////
/*int	CSTDFtoXML::FindTestCell(unsigned int lTestNumber,int lPinmapIndex, CXmlShortTest **ptTestCellFound,BOOL bCreateIfNew,BOOL bResetList)
{
	static CXmlShortTest *ptPrevCell;
	static CXmlShortTest *ptCellFound;
	CXmlShortTest *ptTestCell;
	CXmlShortTest *ptNewCell;

	if(ptTestList == NULL)
	{
		// First test : list is currently empty.
		ptTestList = new CXmlShortTest;
		ptTestList->lTestNumber = lTestNumber;
		ptTestList->lPinmapIndex = lPinmapIndex;
		ptTestList->ptNextTest  = NULL;
		*ptTestCellFound = ptTestList;
		ptCellFound = ptTestList;
		ptPrevCell = NULL;
		return 1;	// Success
	}

    if(bResetList)
		goto rewind_list;

	// Loop until test cell found, or end of list reached.
	ptTestCell = ptCellFound;

	// Check if starting cell has a test already higher than the one we look for...
    if(ptTestCell->lTestNumber > lTestNumber)
		goto rewind_list;	// We need to start from the beginnnig of the list!

	while(ptTestCell != NULL)
	{
	   if(ptTestCell->lTestNumber > lTestNumber)
	   {
		   switch(lPinmapIndex)
		   {
			   case GEX_PTEST:	// This test is not a Multiple-resut parametric...so create test!
				   goto create_test;

			   case GEX_MPTEST: // Multiple-result parametric test...but no PinmapIndex
			   default:	// Multiple-result parametric test...we need to look for
				   goto rewind_list;
		   }
	   }
	   else
	   if(ptTestCell->lTestNumber == lTestNumber)
	   {
		   switch(lPinmapIndex)
		   {
		   case GEX_PTEST:	// This test is not a Multiple-resut parametric...
							// so no need to check its PinmapIndex#.
				*ptTestCellFound = ptTestCell;
				ptCellFound = ptTestCell;
				return 1; // Test found, pointer to it returned.
	
		   case GEX_MPTEST: // Multiple-result parametric test...but no PinmapIndex
		   default:	// Multiple-result parametric test...we need to look for
					// the test record ALSO matching the PinmapIndex# !
				while(ptTestCell != NULL)
				{
				   // If gone too far (passed test cell or PinmapIndex #) rewind list!
				   if((ptTestCell->lTestNumber > lTestNumber) || (ptTestCell->lPinmapIndex > lPinmapIndex))
					   goto rewind_list;
					else
					if(ptTestCell->lPinmapIndex == lPinmapIndex)
					{
						*ptTestCellFound = ptTestCell;
						ptCellFound = ptTestCell;
						return 1; // Test+PinmapIndex# found, pointer to it returned.
					}
					ptPrevCell = ptTestCell;
					ptTestCell = ptTestCell->ptNextTest;
				};
				// Reached the end of list without finding Test#+PinmapIndex#
			   goto rewind_list;
		   }
	   }
	   ptPrevCell = ptTestCell;
	   ptTestCell = ptTestCell->ptNextTest;
	};

	// We have scanned the list without finding the test...so we may have to create it! (unless it is a MP test)
	if(lPinmapIndex == GEX_PTEST)
		goto create_test;

rewind_list:
	// Start from first test in list
	ptTestCell = ptTestList;
	ptCellFound = ptTestList;
	ptPrevCell = NULL;
	// Check from the beginning of the list if can find the test.
	while(ptTestCell != NULL)
	{
	   if(ptTestCell->lTestNumber > lTestNumber)
		   goto create_test;
	   else
	   if(ptTestCell->lTestNumber == lTestNumber)
	   {
		   switch(lPinmapIndex)
		   {
		   case GEX_PTEST:	// This test is not a Multiple-resut parametric...
							// so no need to check its PinmapIndex#.
				*ptTestCellFound = ptTestCell;
				ptCellFound = ptTestCell;
				return 1; // Test found, pointer to it returned.
	
		   case GEX_MPTEST: // Multiple-result parametric test...but no PinmapIndex
		   default:	// Multiple-result parametric test...we need to look for
					// the test record ALSO matching the PinmapIndex# !
				while(ptTestCell != NULL)
				{
				   if((ptTestCell->lTestNumber > lTestNumber) || (ptTestCell->lPinmapIndex > lPinmapIndex))
					   goto create_test;
					else
					if(ptTestCell->lPinmapIndex == lPinmapIndex)
					{
						*ptTestCellFound = ptTestCell;
						ptCellFound = ptTestCell;
						return 1; // Test+PinmapIndex# found, pointer to it returned.
					}
					ptPrevCell = ptTestCell;
					ptTestCell = ptTestCell->ptNextTest;
				};
				// Reached the end of list AGAIN without finding Test#+PinmapIndex#...so append it to current list
				goto create_test;
		   }
	   }	   
	   ptPrevCell = ptTestCell;
	   ptTestCell = ptTestCell->ptNextTest;
	};

create_test:
	if(bCreateIfNew == false)
		return 0;	// Test not in current list...

	// Test not in list: insert in list.
	ptCellFound = ptNewCell = new CXmlShortTest;
	ptNewCell->lTestNumber = lTestNumber;
	ptNewCell->lPinmapIndex = lPinmapIndex;
	ptNewCell->ptNextTest = NULL;

	if(ptPrevCell == NULL)
	{
		// This cell becomes head of list
		ptNewCell->ptNextTest = ptTestList;
		ptTestList = ptNewCell;
	}
	else
	{
		// Insert cell in list
		ptPrevCell->ptNextTest = ptNewCell;
		ptNewCell->ptNextTest  = ptTestCell;
	}

	*ptTestCellFound = ptNewCell;
	return 1;	// Success
}
*/
/////////////////////////////////////////////////////////////////////////////
// Read STDF.MIR record, dump it to XML file...
/////////////////////////////////////////////////////////////////////////////
bool CSTDFtoXML::ProcessMIR()	// Extract MIR data
{
	QString			strXmlString;
    GQTL_STDF::Stdf_MIR_V4	clStdfMIR;

	// Pass#1: check license validity, update counters, dump if in debug
	if(lPass == 1)
	{
		// Check if file date is not more recent than license expiration date!
        if(m_clStdfParse.ReadRecord((GQTL_STDF::Stdf_Record *)&clStdfMIR) == false)
		{
			// Error reading STDF file
			// Convertion failed.
            GSET_ERROR1(CSTDFtoXML, eStdfRead, GGET_LASTERROR(StdfParse,&m_clStdfParse), m_strStdfFile.toLatin1().constData());
			return false;
		}

		// Dump 
        DebugDumpRecord(GQTL_STDF::Stdf_Record::Rec_MIR, m_nInitialSequenceIndentationLevel);
		
		return true;
	}

	// Pass#2: write record to XML
	if(lPass == 2)
	{
        if(m_clStdfParse.ReadRecord((GQTL_STDF::Stdf_Record *)&clStdfMIR) == false)
		{
			// Error reading STDF file
			// Convertion failed.
			GSET_ERROR1(CSTDFtoXML, eStdfRead, GGET_LASTERROR(StdfParse,&m_clStdfParse), m_strStdfFile.toLatin1().constData());
			return false;
		}
        clStdfMIR.GetXMLString(strXmlString, m_nInitialSequenceIndentationLevel, GQTL_STDF::Stdf_Record::FieldFlag_Valid);
		m_hXmlInitialSequenceStream  << strXmlString;
		return true;
	}

	return true;
}


/////////////////////////////////////////////////////////////////////////////
// Read STDF.PTR record, dump it to XML file...
/////////////////////////////////////////////////////////////////////////////
bool CSTDFtoXML::ProcessPTR(void)	// Extract Test results
{
	QString			strXmlString;
    GQTL_STDF::Stdf_PTR_V4	clStdfPTR;

	// Pass#1: update counters, dump if in debug
	if(lPass == 1)
	{
		// Update counters
		m_nPTRCount++;

		// Dump 
        DebugDumpRecord(GQTL_STDF::Stdf_Record::Rec_PTR, m_nResultsSequenceIndentationLevel);
		
		return true;
	}

	// Pass#2: write record to XML
	if(lPass == 2)
	{
        if(m_clStdfParse.ReadRecord((GQTL_STDF::Stdf_Record *)&clStdfPTR) == false)
		{
			// Error reading STDF file
			// Convertion failed.
			GSET_ERROR1(CSTDFtoXML, eStdfRead, GGET_LASTERROR(StdfParse,&m_clStdfParse), m_strStdfFile.toLatin1().constData());
			return false;
		}
		m_bTestProcessed = true;
		if(m_bAddToLimits == true)
		{
            clStdfPTR.GetXMLString(strXmlString, m_nLimitsSequenceIndentationLevel, GQTL_STDF::Stdf_Record::FieldFlag_Valid);
			m_hXmlLimitsSequenceStream  << strXmlString;
		}
		if(m_bPartStarted)
		{
            clStdfPTR.GetXMLString(strXmlString, m_nResultsSequenceIndentationLevel, GQTL_STDF::Stdf_Record::FieldFlag_Valid);
			m_hXmlResultsSequenceStream  << strXmlString;
		}

		return true;
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////
// Read STDF PIR record
/////////////////////////////////////////////////////////////////////////////
bool CSTDFtoXML::ProcessPIR(void)
{
	QString			strXmlString;
    GQTL_STDF::Stdf_PIR_V4	clStdfPIR;

	// Update counters
	m_nPTRCount = 0;
	m_nMPRCount = 0;
	m_nFTRCount = 0;

	// Pass#1: update counters, dump if in debug
	if(lPass == 1)
	{
		// Dump 
        DebugDumpRecord(GQTL_STDF::Stdf_Record::Rec_PIR, m_nResultsSequenceIndentationLevel);
		
		m_nResultsSequenceIndentationLevel++;
	
		return true;
	}

	// Pass#2: write record to XML
	if(lPass == 2)
	{
		WriteTag(m_hXmlResultsSequenceStream, "<part>", m_nResultsSequenceIndentationLevel);
		m_nResultsSequenceIndentationLevel++;

        if(m_clStdfParse.ReadRecord((GQTL_STDF::Stdf_Record *)&clStdfPIR) == false)
		{
			// Error reading STDF file
			// Convertion failed.
            GSET_ERROR1(CSTDFtoXML, eStdfRead, GGET_LASTERROR(StdfParse,&m_clStdfParse), m_strStdfFile.toLatin1().constData());
			return false;
		}
        clStdfPIR.GetXMLString(strXmlString, m_nResultsSequenceIndentationLevel, GQTL_STDF::Stdf_Record::FieldFlag_Valid);

		m_bPartStarted = true;
		if(m_bTestProcessed == true)
			m_bAddToLimits = false;
		m_hXmlResultsSequenceStream  << strXmlString;
		return true;
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////
// Read STDF PRR record
/////////////////////////////////////////////////////////////////////////////
bool CSTDFtoXML::ProcessPRR(void)
{
	QString			strXmlString;
    GQTL_STDF::Stdf_PRR_V4	clStdfPRR;


	// Pass#1: update counters, dump if in debug
	if(lPass == 1)
	{
		m_nResultsSequenceIndentationLevel--;

		// Dump 
        DebugDumpRecord(GQTL_STDF::Stdf_Record::Rec_PRR, m_nResultsSequenceIndentationLevel);
		
		return true;
	}

	// Pass#2: write record to XML
	if(lPass == 2)
	{
        if(m_clStdfParse.ReadRecord((GQTL_STDF::Stdf_Record *)&clStdfPRR) == false)
		{
			// Error reading STDF file
			// Convertion failed.
            GSET_ERROR1(CSTDFtoXML, eStdfRead, GGET_LASTERROR(StdfParse,&m_clStdfParse), m_strStdfFile.toLatin1().constData());
			return false;
		}
        clStdfPRR.GetXMLString(strXmlString, m_nResultsSequenceIndentationLevel, GQTL_STDF::Stdf_Record::FieldFlag_Valid);
		m_bAddToLimits = false;
		m_hXmlResultsSequenceStream  << strXmlString;

		m_nResultsSequenceIndentationLevel--;
		WriteTag(m_hXmlResultsSequenceStream, "</part>", m_nResultsSequenceIndentationLevel);
		return true;
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////
// Read STDF FAR record
/////////////////////////////////////////////////////////////////////////////
bool CSTDFtoXML::ProcessFAR(void)	// Translate FAR into XML and write XML header
{
	QString			strXmlString;
    GQTL_STDF::Stdf_FAR_V4	clStdfFAR;

	// Pass#1: update counters, dump if in debug
	if(lPass == 1)
	{
		// Dump 
        DebugDumpRecord(GQTL_STDF::Stdf_Record::Rec_FAR, m_nInitialSequenceIndentationLevel);
		
		return true;
	}

	// Pass#2: write record to XML
	if(lPass == 2)
	{
		// Initialize XML sequence files
		InitXmlSequenceFiles();

        if(m_clStdfParse.ReadRecord((GQTL_STDF::Stdf_Record *)&clStdfFAR) == false)
		{
			// Error reading STDF file
			// Convertion failed.
            GSET_ERROR1(CSTDFtoXML, eStdfRead, GGET_LASTERROR(StdfParse,&m_clStdfParse), m_strStdfFile.toLatin1().constData());
			return false;
		}
        clStdfFAR.GetXMLString(strXmlString, m_nInitialSequenceIndentationLevel, GQTL_STDF::Stdf_Record::FieldFlag_Valid);
		m_hXmlInitialSequenceStream  << strXmlString;
		return true;
	}

	return true;
}

bool CSTDFtoXML::ProcessATR(void)	// Extract ATR
{
	QString			strXmlString;
    GQTL_STDF::Stdf_ATR_V4	clStdfATR;

	// Pass#1: update counters, dump if in debug
	if(lPass == 1)
	{
		// Dump 
        DebugDumpRecord(GQTL_STDF::Stdf_Record::Rec_ATR, m_nInitialSequenceIndentationLevel);
		
		return true;
	}

	// Pass#2: write record to XML
	if(lPass == 2)
	{
        if(m_clStdfParse.ReadRecord((GQTL_STDF::Stdf_Record *)&clStdfATR) == false)
		{
			// Error reading STDF file
			// Convertion failed.
            GSET_ERROR1(CSTDFtoXML, eStdfRead, GGET_LASTERROR(StdfParse,&m_clStdfParse), m_strStdfFile.toLatin1().constData());
			return false;
		}
        clStdfATR.GetXMLString(strXmlString, m_nInitialSequenceIndentationLevel, GQTL_STDF::Stdf_Record::FieldFlag_Valid);
		m_hXmlInitialSequenceStream  << strXmlString;
		return true;
	}

	return true;
}

bool CSTDFtoXML::ProcessTSR(void)	// Extract TSR
{
	QString			strXmlString;
    GQTL_STDF::Stdf_TSR_V4	clStdfTSR;

	// Pass#1: update counters, dump if in debug
	if(lPass == 1)
	{
		// Dump 
        DebugDumpRecord(GQTL_STDF::Stdf_Record::Rec_TSR, m_nSummarySequenceIndentationLevel);
		
		return true;
	}

	// Pass#2: write record to XML
	if(lPass == 2)
	{
        if(m_clStdfParse.ReadRecord((GQTL_STDF::Stdf_Record *)&clStdfTSR) == false)
		{
			// Error reading STDF file
			// Convertion failed.
            GSET_ERROR1(CSTDFtoXML, eStdfRead, GGET_LASTERROR(StdfParse,&m_clStdfParse), m_strStdfFile.toLatin1().constData());
			return false;
		}
        clStdfTSR.GetXMLString(strXmlString, m_nSummarySequenceIndentationLevel, GQTL_STDF::Stdf_Record::FieldFlag_Valid);
		m_hXmlSummaryTSRSequenceStream  << strXmlString;
		return true;
	}

	return true;
}

bool CSTDFtoXML::ProcessFTR(void)	// Extract FTR data
{
	QString			strXmlString;
    GQTL_STDF::Stdf_FTR_V4	clStdfFTR;

	// Pass#1: update counters, dump if in debug
	if(lPass == 1)
	{
		// Update counters
		m_nFTRCount++;

		// Dump 
        DebugDumpRecord(GQTL_STDF::Stdf_Record::Rec_FTR, m_nResultsSequenceIndentationLevel);
		
		return true;
	}

	// Pass#2: write record to XML
	if(lPass == 2)
	{
        if(m_clStdfParse.ReadRecord((GQTL_STDF::Stdf_Record *)&clStdfFTR) == false)
		{
			// Error reading STDF file
			// Convertion failed.
            GSET_ERROR1(CSTDFtoXML, eStdfRead, GGET_LASTERROR(StdfParse,&m_clStdfParse), m_strStdfFile.toLatin1().constData());
			return false;
		}
		m_bTestProcessed = true;
		if(m_bAddToLimits == true)
		{
            clStdfFTR.GetXMLString(strXmlString, m_nLimitsSequenceIndentationLevel, GQTL_STDF::Stdf_Record::FieldFlag_Valid);
			m_hXmlLimitsSequenceStream  << strXmlString;
		}
		if(m_bPartStarted)
		{
            clStdfFTR.GetXMLString(strXmlString, m_nResultsSequenceIndentationLevel, GQTL_STDF::Stdf_Record::FieldFlag_Valid);
			m_hXmlResultsSequenceStream  << strXmlString;
		}
		return true;
	}

	return true;
}

bool CSTDFtoXML::ProcessMPR(void)	// Extract Test results (Multiple Parametric)
{
	QString			strXmlString;
    GQTL_STDF::Stdf_MPR_V4	clStdfMPR;

	// Pass#1: update counters, dump if in debug
	if(lPass == 1)
	{
		// Update counters
		m_nMPRCount++;

		// Dump 
        DebugDumpRecord(GQTL_STDF::Stdf_Record::Rec_MPR, m_nResultsSequenceIndentationLevel);
		
		return true;
	}

	// Pass#2: write record to XML
	if(lPass == 2)
	{
        if(m_clStdfParse.ReadRecord((GQTL_STDF::Stdf_Record *)&clStdfMPR) == false)
		{
			// Error reading STDF file
			// Convertion failed.
            GSET_ERROR1(CSTDFtoXML, eStdfRead, GGET_LASTERROR(StdfParse,&m_clStdfParse), m_strStdfFile.toLatin1().constData());
			return false;
		}
		m_bTestProcessed = true;
		if(m_bAddToLimits == true)
		{
            clStdfMPR.GetXMLString(strXmlString, m_nLimitsSequenceIndentationLevel, GQTL_STDF::Stdf_Record::FieldFlag_Valid);
			m_hXmlLimitsSequenceStream  << strXmlString;
		}
		if(m_bPartStarted)
		{
            clStdfMPR.GetXMLString(strXmlString, m_nResultsSequenceIndentationLevel, GQTL_STDF::Stdf_Record::FieldFlag_Valid);
			m_hXmlResultsSequenceStream  << strXmlString;
		}
		return true;
	}

	return true;
}

bool CSTDFtoXML::ProcessMRR(void)
{
	QString			strXmlString;
    GQTL_STDF::Stdf_MRR_V4	clStdfMRR;

	// Pass#1: update counters, dump if in debug
	if(lPass == 1)
	{
		// Dump 
        DebugDumpRecord(GQTL_STDF::Stdf_Record::Rec_MRR, m_nFinalSequenceIndentationLevel);
		
		return true;
	}

	// Pass#2: write record to XML
	if(lPass == 2)
	{
        if(m_clStdfParse.ReadRecord((GQTL_STDF::Stdf_Record *)&clStdfMRR) == false)
		{
			// Error reading STDF file
			// Convertion failed.
            GSET_ERROR1(CSTDFtoXML, eStdfRead, GGET_LASTERROR(StdfParse,&m_clStdfParse), m_strStdfFile.toLatin1().constData());
			return false;
		}
        clStdfMRR.GetXMLString(strXmlString, m_nFinalSequenceIndentationLevel, GQTL_STDF::Stdf_Record::FieldFlag_Valid);
		m_hXmlFinalSequenceStream  << strXmlString;
		
		// Close XML sequence files
		CloseXmlSequenceFiles();

		return true;
	}

	return true;
}

bool CSTDFtoXML::ProcessPCR(void)
{
	QString			strXmlString;
    GQTL_STDF::Stdf_PCR_V4	clStdfPCR;

	// Pass#1: update counters, dump if in debug
	if(lPass == 1)
	{
		// Dump 
        DebugDumpRecord(GQTL_STDF::Stdf_Record::Rec_PCR, m_nSummarySequenceIndentationLevel);
		
		return true;
	}

	// Pass#2: write record to XML
	if(lPass == 2)
	{
        if(m_clStdfParse.ReadRecord((GQTL_STDF::Stdf_Record *)&clStdfPCR) == false)
		{
			// Error reading STDF file
			// Convertion failed.
            GSET_ERROR1(CSTDFtoXML, eStdfRead, GGET_LASTERROR(StdfParse,&m_clStdfParse), m_strStdfFile.toLatin1().constData());
			return false;
		}
        clStdfPCR.GetXMLString(strXmlString, m_nSummarySequenceIndentationLevel, GQTL_STDF::Stdf_Record::FieldFlag_Valid);
		m_hXmlSummaryPCRSequenceStream  << strXmlString;
		return true;
	}

	return true;
}

bool CSTDFtoXML::ProcessHBR(void)
{
	QString			strXmlString;
    GQTL_STDF::Stdf_HBR_V4	clStdfHBR;

	// Pass#1: update counters, dump if in debug
	if(lPass == 1)
	{
		// Dump 
        DebugDumpRecord(GQTL_STDF::Stdf_Record::Rec_HBR, m_nSummarySequenceIndentationLevel);
		
		return true;
	}

	// Pass#2: write record to XML
	if(lPass == 2)
	{
        if(m_clStdfParse.ReadRecord((GQTL_STDF::Stdf_Record *)&clStdfHBR) == false)
		{
			// Error reading STDF file
			// Convertion failed.
            GSET_ERROR1(CSTDFtoXML, eStdfRead, GGET_LASTERROR(StdfParse,&m_clStdfParse), m_strStdfFile.toLatin1().constData());
			return false;
		}
        clStdfHBR.GetXMLString(strXmlString, m_nSummarySequenceIndentationLevel, GQTL_STDF::Stdf_Record::FieldFlag_Valid);
		m_hXmlSummaryHBRSequenceStream  << strXmlString;
		return true;
	}

	return true;
}

bool CSTDFtoXML::ProcessSBR(void)
{
	QString			strXmlString;
    GQTL_STDF::Stdf_SBR_V4	clStdfSBR;

	// Pass#1: update counters, dump if in debug
	if(lPass == 1)
	{
		// Dump 
        DebugDumpRecord(GQTL_STDF::Stdf_Record::Rec_SBR, m_nSummarySequenceIndentationLevel);
		
		return true;
	}

	// Pass#2: write record to XML
	if(lPass == 2)
	{
        if(m_clStdfParse.ReadRecord((GQTL_STDF::Stdf_Record *)&clStdfSBR) == false)
		{
			// Error reading STDF file
			// Convertion failed.
            GSET_ERROR1(CSTDFtoXML, eStdfRead, GGET_LASTERROR(StdfParse,&m_clStdfParse), m_strStdfFile.toLatin1().constData());
			return false;
		}
        clStdfSBR.GetXMLString(strXmlString, m_nSummarySequenceIndentationLevel, GQTL_STDF::Stdf_Record::FieldFlag_Valid);
		m_hXmlSummarySBRSequenceStream  << strXmlString;
		return true;
	}

	return true;
}

bool CSTDFtoXML::ProcessPMR(void)
{
	QString			strXmlString;
    GQTL_STDF::Stdf_PMR_V4	clStdfPMR;

	// Pass#1: update counters, dump if in debug
	if(lPass == 1)
	{
		// Dump 
        DebugDumpRecord(GQTL_STDF::Stdf_Record::Rec_PMR, m_nPinmapSequenceIndentationLevel);
		
		return true;
	}

	// Pass#2: write record to XML
	if(lPass == 2)
	{
        if(m_clStdfParse.ReadRecord((GQTL_STDF::Stdf_Record *)&clStdfPMR) == false)
		{
			// Error reading STDF file
			// Convertion failed.
            GSET_ERROR1(CSTDFtoXML, eStdfRead, GGET_LASTERROR(StdfParse,&m_clStdfParse), m_strStdfFile.toLatin1().constData());
			return false;
		}
        clStdfPMR.GetXMLString(strXmlString, m_nPinmapSequenceIndentationLevel, GQTL_STDF::Stdf_Record::FieldFlag_Valid);
		m_hXmlPinmapSequenceStream  << strXmlString;
		return true;
	}

	return true;
}

bool CSTDFtoXML::ProcessPGR(void)
{
	QString			strXmlString;
    GQTL_STDF::Stdf_PGR_V4	clStdfPGR;

	// Pass#1: update counters, dump if in debug
	if(lPass == 1)
	{
		// Dump 
        DebugDumpRecord(GQTL_STDF::Stdf_Record::Rec_PGR, m_nPinmapSequenceIndentationLevel);
		
		return true;
	}

	// Pass#2: write record to XML
	if(lPass == 2)
	{
        if(m_clStdfParse.ReadRecord((GQTL_STDF::Stdf_Record *)&clStdfPGR) == false)
		{
			// Error reading STDF file
			// Convertion failed.
            GSET_ERROR1(CSTDFtoXML, eStdfRead, GGET_LASTERROR(StdfParse,&m_clStdfParse), m_strStdfFile.toLatin1().constData());
			return false;
		}
        clStdfPGR.GetXMLString(strXmlString, m_nPinmapSequenceIndentationLevel, GQTL_STDF::Stdf_Record::FieldFlag_Valid);
		m_hXmlPinmapSequenceStream  << strXmlString;
		return true;
	}

	return true;
}

bool CSTDFtoXML::ProcessPLR(void)
{
	QString			strXmlString;
    GQTL_STDF::Stdf_PLR_V4	clStdfPLR;

	// Pass#1: update counters, dump if in debug
	if(lPass == 1)
	{
		// Dump 
        DebugDumpRecord(GQTL_STDF::Stdf_Record::Rec_PLR, m_nPinmapSequenceIndentationLevel);
		
		return true;
	}

	// Pass#2: write record to XML
	if(lPass == 2)
	{
        if(m_clStdfParse.ReadRecord((GQTL_STDF::Stdf_Record *)&clStdfPLR) == false)
		{
			// Error reading STDF file
			// Convertion failed.
            GSET_ERROR1(CSTDFtoXML, eStdfRead, GGET_LASTERROR(StdfParse,&m_clStdfParse), m_strStdfFile.toLatin1().constData());
			return false;
		}
        clStdfPLR.GetXMLString(strXmlString, m_nPinmapSequenceIndentationLevel, GQTL_STDF::Stdf_Record::FieldFlag_Valid);
		m_hXmlPinmapSequenceStream  << strXmlString;
		return true;
	}

	return true;
}

bool CSTDFtoXML::ProcessRDR(void)
{
	QString			strXmlString;
    GQTL_STDF::Stdf_RDR_V4	clStdfRDR;

	// Pass#1: update counters, dump if in debug
	if(lPass == 1)
	{
		// Dump 
        DebugDumpRecord(GQTL_STDF::Stdf_Record::Rec_RDR, m_nInitialSequenceIndentationLevel);
		
		return true;
	}

	// Pass#2: write record to XML
	if(lPass == 2)
	{
        if(m_clStdfParse.ReadRecord((GQTL_STDF::Stdf_Record *)&clStdfRDR) == false)
		{
			// Error reading STDF file
			// Convertion failed.
            GSET_ERROR1(CSTDFtoXML, eStdfRead, GGET_LASTERROR(StdfParse,&m_clStdfParse), m_strStdfFile.toLatin1().constData());
			return false;
		}
        clStdfRDR.GetXMLString(strXmlString, m_nInitialSequenceIndentationLevel, GQTL_STDF::Stdf_Record::FieldFlag_Valid);
		m_hXmlInitialSequenceStream  << strXmlString;
		return true;
	}

	return true;
}

bool CSTDFtoXML::ProcessSDR(void)
{
	QString			strXmlString;
    GQTL_STDF::Stdf_SDR_V4	clStdfSDR;

	// Pass#1: update counters, dump if in debug
	if(lPass == 1)
	{
		// Dump 
        DebugDumpRecord(GQTL_STDF::Stdf_Record::Rec_SDR, m_nInitialSequenceIndentationLevel);
		
		return true;
	}

	// Pass#2: write record to XML
	if(lPass == 2)
	{
        if(m_clStdfParse.ReadRecord((GQTL_STDF::Stdf_Record *)&clStdfSDR) == false)
		{
			// Error reading STDF file
			// Convertion failed.
            GSET_ERROR1(CSTDFtoXML, eStdfRead, GGET_LASTERROR(StdfParse,&m_clStdfParse), m_strStdfFile.toLatin1().constData());
			return false;
		}
        clStdfSDR.GetXMLString(strXmlString, m_nInitialSequenceIndentationLevel, GQTL_STDF::Stdf_Record::FieldFlag_Valid);
		m_hXmlInitialSequenceStream  << strXmlString;
		return true;
	}

	return true;
}

bool CSTDFtoXML::ProcessWIR(void)
{
	QString			strXmlString;
    GQTL_STDF::Stdf_WIR_V4	clStdfWIR;

	// Pass#1: update counters, dump if in debug
	if(lPass == 1)
	{
		// Dump 
        DebugDumpRecord(GQTL_STDF::Stdf_Record::Rec_WIR, m_nResultsSequenceIndentationLevel);

		m_nResultsSequenceIndentationLevel++;
		return true;
	}

	// Pass#2: write record to XML
	if(lPass == 2)
	{
		WriteTag(m_hXmlResultsSequenceStream, "<wafer>", m_nResultsSequenceIndentationLevel);
		m_nResultsSequenceIndentationLevel++;

        if(m_clStdfParse.ReadRecord((GQTL_STDF::Stdf_Record *)&clStdfWIR) == false)
		{
			// Error reading STDF file
			// Convertion failed.
            GSET_ERROR1(CSTDFtoXML, eStdfRead, GGET_LASTERROR(StdfParse,&m_clStdfParse), m_strStdfFile.toLatin1().constData());
			return false;
		}
        clStdfWIR.GetXMLString(strXmlString, m_nResultsSequenceIndentationLevel, GQTL_STDF::Stdf_Record::FieldFlag_Valid);
		m_hXmlResultsSequenceStream  << strXmlString;
		return true;
	}

	return true;
}

bool CSTDFtoXML::ProcessWRR(void)
{
	QString			strXmlString;
    GQTL_STDF::Stdf_WRR_V4	clStdfWRR;

	// Pass#1: update counters, dump if in debug
	if(lPass == 1)
	{
		m_nResultsSequenceIndentationLevel--;

		// Dump 
        DebugDumpRecord(GQTL_STDF::Stdf_Record::Rec_WRR, m_nResultsSequenceIndentationLevel);
		
		return true;
	}

	// Pass#2: write record to XML
	if(lPass == 2)
	{
        if(m_clStdfParse.ReadRecord((GQTL_STDF::Stdf_Record *)&clStdfWRR) == false)
		{
			// Error reading STDF file
			// Convertion failed.
            GSET_ERROR1(CSTDFtoXML, eStdfRead, GGET_LASTERROR(StdfParse,&m_clStdfParse), m_strStdfFile.toLatin1().constData());
			return false;
		}
        clStdfWRR.GetXMLString(strXmlString, m_nResultsSequenceIndentationLevel, GQTL_STDF::Stdf_Record::FieldFlag_Valid);
		m_hXmlResultsSequenceStream  << strXmlString;

		m_nResultsSequenceIndentationLevel--;
		WriteTag(m_hXmlResultsSequenceStream, "</wafer>", m_nResultsSequenceIndentationLevel);
		return true;
	}

	return true;
}

bool CSTDFtoXML::ProcessWCR(void)
{
	QString			strXmlString;
    GQTL_STDF::Stdf_WCR_V4	clStdfWCR;

	// Pass#1: update counters, dump if in debug
	if(lPass == 1)
	{
		// Dump 
        DebugDumpRecord(GQTL_STDF::Stdf_Record::Rec_WCR,m_nInitialSequenceIndentationLevel);
		
		return true;
	}

	// Pass#2: write record to XML
	if(lPass == 2)
	{
        if(m_clStdfParse.ReadRecord((GQTL_STDF::Stdf_Record *)&clStdfWCR) == false)
		{
			// Error reading STDF file
			// Convertion failed.
            GSET_ERROR1(CSTDFtoXML, eStdfRead, GGET_LASTERROR(StdfParse,&m_clStdfParse), m_strStdfFile.toLatin1().constData());
			return false;
		}
        clStdfWCR.GetXMLString(strXmlString, m_nInitialSequenceIndentationLevel, GQTL_STDF::Stdf_Record::FieldFlag_Valid);
		m_hXmlInitialSequenceStream  << strXmlString;
		return true;
	}

	return true;
}

bool CSTDFtoXML::ProcessDTR(void)
{
	QString			strXmlString;
    GQTL_STDF::Stdf_DTR_V4	clStdfDTR;

	// Pass#1: update counters, dump if in debug
	if(lPass == 1)
	{
		// Dump 
        DebugDumpRecord(GQTL_STDF::Stdf_Record::Rec_DTR, m_nFinalSequenceIndentationLevel);
		
		return true;
	}

	// Pass#2: write record to XML
	if(lPass == 2)
	{
        if(m_clStdfParse.ReadRecord((GQTL_STDF::Stdf_Record *)&clStdfDTR) == false)
		{
			// Error reading STDF file
			// Convertion failed.
            GSET_ERROR1(CSTDFtoXML, eStdfRead, GGET_LASTERROR(StdfParse,&m_clStdfParse), m_strStdfFile.toLatin1().constData());
			return false;
		}
        clStdfDTR.GetXMLString(strXmlString, m_nFinalSequenceIndentationLevel, GQTL_STDF::Stdf_Record::FieldFlag_Valid);
		m_hXmlFinalSequenceStream  << strXmlString;
		return true;
	}

	return true;
}

bool CSTDFtoXML::ProcessGDR(void)
{
	QString			strXmlString;
    GQTL_STDF::Stdf_GDR_V4	clStdfGDR;

	// Pass#1: update counters, dump if in debug
	if(lPass == 1)
	{
		// Dump 
        DebugDumpRecord(GQTL_STDF::Stdf_Record::Rec_GDR, m_nFinalSequenceIndentationLevel);
		
		return true;
	}

	// Pass#2: write record to XML
	if(lPass == 2)
	{
        if(m_clStdfParse.ReadRecord((GQTL_STDF::Stdf_Record *)&clStdfGDR) == false)
		{
			// Error reading STDF file
			// Convertion failed.
            GSET_ERROR1(CSTDFtoXML, eStdfRead, GGET_LASTERROR(StdfParse,&m_clStdfParse), m_strStdfFile.toLatin1().constData());
			return false;
		}
        clStdfGDR.GetXMLString(strXmlString, m_nFinalSequenceIndentationLevel, GQTL_STDF::Stdf_Record::FieldFlag_Valid);
		m_hXmlFinalSequenceStream  << strXmlString;
		return true;
	}

	return true;
}

bool CSTDFtoXML::ProcessBPS(void)
{
	QString			strXmlString;
    GQTL_STDF::Stdf_BPS_V4	clStdfBPS;

	// Pass#1: update counters, dump if in debug
	if(lPass == 1)
	{
		// Dump 
        DebugDumpRecord(GQTL_STDF::Stdf_Record::Rec_BPS, m_nResultsSequenceIndentationLevel);
		
		return true;
	}

	// Pass#2: write record to XML
	if(lPass == 2)
	{
        if(m_clStdfParse.ReadRecord((GQTL_STDF::Stdf_Record *)&clStdfBPS) == false)
		{
			// Error reading STDF file
			// Convertion failed.
            GSET_ERROR1(CSTDFtoXML, eStdfRead, GGET_LASTERROR(StdfParse,&m_clStdfParse), m_strStdfFile.toLatin1().constData());
			return false;
		}
        clStdfBPS.GetXMLString(strXmlString, m_nResultsSequenceIndentationLevel, GQTL_STDF::Stdf_Record::FieldFlag_Valid);
		m_hXmlResultsSequenceStream  << strXmlString;
		return true;
	}

	return true;
}

bool CSTDFtoXML::ProcessEPS(void)
{
	QString			strXmlString;
    GQTL_STDF::Stdf_EPS_V4	clStdfEPS;

	// Pass#1: update counters, dump if in debug
	if(lPass == 1)
	{
		// Dump 
        DebugDumpRecord(GQTL_STDF::Stdf_Record::Rec_EPS, m_nResultsSequenceIndentationLevel);
		
		return true;
	}

	// Pass#2: write record to XML
	if(lPass == 2)
	{
        if(m_clStdfParse.ReadRecord((GQTL_STDF::Stdf_Record *)&clStdfEPS) == false)
		{
			// Error reading STDF file
			// Convertion failed.
            GSET_ERROR1(CSTDFtoXML, eStdfRead, GGET_LASTERROR(StdfParse,&m_clStdfParse), m_strStdfFile.toLatin1().constData());
			return false;
		}
        clStdfEPS.GetXMLString(strXmlString, m_nResultsSequenceIndentationLevel, GQTL_STDF::Stdf_Record::FieldFlag_Valid);
		m_hXmlResultsSequenceStream  << strXmlString;
		return true;
	}

	return true;
}

bool CSTDFtoXML::ProcessRESERVED_IMAGE(void)
{
	// Pass#1: update counters, dump if in debug
	if(lPass == 1)
	{
        DebugDumpRecord(GQTL_STDF::Stdf_Record::Rec_RESERVED_IMAGE, m_nResultsSequenceIndentationLevel);
	}

	return true;
}

bool CSTDFtoXML::ProcessRESERVED_IG900(void)
{
	// Pass#1: update counters, dump if in debug
	if(lPass == 1)
	{
        DebugDumpRecord(GQTL_STDF::Stdf_Record::Rec_RESERVED_IG900, m_nResultsSequenceIndentationLevel);
	}

	return true;
}

bool CSTDFtoXML::ProcessUNKNOWN(void)
{
	// Pass#1: update counters, dump if in debug
	if(lPass == 1)
	{
        DebugDumpRecord(GQTL_STDF::Stdf_Record::Rec_UNKNOWN, m_nResultsSequenceIndentationLevel);
	}

	return true;
}

void CSTDFtoXML::WriteTag(QTextStream & hStream, const char* szTag, int nIndentationLevel)
{
	QString strXmlString;

	// Init string
	for(int nIndex=nIndentationLevel; nIndex>0; nIndex--)
		strXmlString += "\t";

	strXmlString += szTag;
	hStream  << strXmlString << endl;
}

void CSTDFtoXML::DebugDumpRecord(int nRecordType, int nIndentationLevel)
{
#ifdef QT_DEBUG
	QString strString="", strTemp;
	bool	bDump = true;

	// Init string
	for(int nIndex=nIndentationLevel; nIndex>0; nIndex--)
		strString += "\t";

	// Process STDF record read.
	switch(nRecordType)
	{
        case GQTL_STDF::Stdf_Record::Rec_FAR:
			strString += "FAR";
			break;
        case GQTL_STDF::Stdf_Record::Rec_ATR:
			strString += "ATR";
			break;
        case GQTL_STDF::Stdf_Record::Rec_MIR:
			strString += "MIR";
			break;
        case GQTL_STDF::Stdf_Record::Rec_MRR:
			strString += strTemp.sprintf("MRR (%d records", m_nStdfRecords);
			break;
        case GQTL_STDF::Stdf_Record::Rec_PCR:
			strString += "PCR";
			break;
        case GQTL_STDF::Stdf_Record::Rec_HBR:
			strString += "HBR";
			break;
        case GQTL_STDF::Stdf_Record::Rec_SBR:
			strString += "SBR";
			break;
        case GQTL_STDF::Stdf_Record::Rec_PMR:
			strString += "PMR";
			break;
        case GQTL_STDF::Stdf_Record::Rec_PGR:
			strString += "PGR";
			break;
        case GQTL_STDF::Stdf_Record::Rec_PLR:
			strString += "PLR";
			break;
        case GQTL_STDF::Stdf_Record::Rec_RDR:
			strString += "RDR";
			break;
        case GQTL_STDF::Stdf_Record::Rec_SDR:
			strString += "SDR";
			break;
        case GQTL_STDF::Stdf_Record::Rec_WIR:
			strString += "WIR";
			break;
        case GQTL_STDF::Stdf_Record::Rec_WRR:
			strString += "WRR";
			break;
        case GQTL_STDF::Stdf_Record::Rec_WCR:
			strString += "WCR";
			break;
        case GQTL_STDF::Stdf_Record::Rec_PIR:
			strString += "PIR";
			break;
        case GQTL_STDF::Stdf_Record::Rec_PRR:
			strString += strTemp.sprintf("PRR (%d PTRs, %d MPRs, %d FTRs", m_nPTRCount, m_nMPRCount, m_nFTRCount);
			break;
        case GQTL_STDF::Stdf_Record::Rec_TSR:
			strString += "TSR";
			break;
        case GQTL_STDF::Stdf_Record::Rec_PTR:
			// Don't dump PTRs, there may be a lot
			bDump = false;
			strString += "PTR";
			break;
        case GQTL_STDF::Stdf_Record::Rec_MPR:
			// Don't dump MPRs, there may be a lot
			bDump = false;
			strString += "MPR";
			break;
        case GQTL_STDF::Stdf_Record::Rec_FTR:
			// Don't dump FTRs, there may be a lot
			bDump = false;
			strString += "FTR";
			break;
        case GQTL_STDF::Stdf_Record::Rec_BPS:
			strString += "BPS";
			break;
        case GQTL_STDF::Stdf_Record::Rec_EPS:
			strString += "EPS";
			break;
        case GQTL_STDF::Stdf_Record::Rec_GDR:
			strString += "GDR";
			break;
        case GQTL_STDF::Stdf_Record::Rec_DTR:
			strString += "DTR";
			break;
        case GQTL_STDF::Stdf_Record::Rec_RESERVED_IMAGE:
			strString += "Reserved for use by Image";
			break;
        case GQTL_STDF::Stdf_Record::Rec_RESERVED_IG900:
			strString += "Reserved for use by IG900";
			break;
        case GQTL_STDF::Stdf_Record::Rec_UNKNOWN:
			strString += "UNKNOWN!!!!";
			break;
	}

	if(bDump == true)
		m_hDebugDumpStream  << strString << endl;
#else
	Q_UNUSED(nRecordType);
	Q_UNUSED(nIndentationLevel);
#endif
}

bool CSTDFtoXML::OpenFiles(const QString & strStdfFileName, const QString & strXmlFileName)
{
	QString strFileName;

	// Open XML files
	strFileName = strXmlFileName + ".main";
    m_hXmlMainSequenceFile.setFileName(strFileName);
    if(!m_hXmlMainSequenceFile.open(QIODevice::WriteOnly))
	{
		GSET_ERROR1(CSTDFtoXML, eOpenWriteFail, NULL, strFileName.toLatin1().constData());
		return false;
	}
	strFileName = strXmlFileName + ".initial";
    m_hXmlInitialSequenceFile.setFileName(strFileName);
    if(!m_hXmlInitialSequenceFile.open(QIODevice::WriteOnly))
	{
		m_hXmlMainSequenceFile.close();
		GSET_ERROR1(CSTDFtoXML, eOpenWriteFail, NULL, strFileName.toLatin1().constData());
		return false;
	}
	strFileName = strXmlFileName + ".pinmap";
    m_hXmlPinmapSequenceFile.setFileName(strFileName);
    if(!m_hXmlPinmapSequenceFile.open(QIODevice::WriteOnly))
	{
		m_hXmlMainSequenceFile.close();
		m_hXmlInitialSequenceFile.close();
		GSET_ERROR1(CSTDFtoXML, eOpenWriteFail, NULL, strFileName.toLatin1().constData());
		return false;
	}
	strFileName = strXmlFileName + ".limits";
    m_hXmlLimitsSequenceFile.setFileName(strFileName);
    if(!m_hXmlLimitsSequenceFile.open(QIODevice::WriteOnly))
	{
		m_hXmlMainSequenceFile.close();
		m_hXmlInitialSequenceFile.close();
		m_hXmlPinmapSequenceFile.close();
		GSET_ERROR1(CSTDFtoXML, eOpenWriteFail, NULL, strFileName.toLatin1().constData());
		return false;
	}
	strFileName = strXmlFileName + ".results";
    m_hXmlResultsSequenceFile.setFileName(strFileName);
    if(!m_hXmlResultsSequenceFile.open(QIODevice::WriteOnly))
	{
		m_hXmlMainSequenceFile.close();
		m_hXmlInitialSequenceFile.close();
		m_hXmlPinmapSequenceFile.close();
		m_hXmlLimitsSequenceFile.close();
		GSET_ERROR1(CSTDFtoXML, eOpenWriteFail, NULL, strFileName.toLatin1().constData());
		return false;
	}
	strFileName = strXmlFileName + ".summary";
    m_hXmlSummarySequenceFile.setFileName(strFileName);
    if(!m_hXmlSummarySequenceFile.open(QIODevice::WriteOnly))
	{
		m_hXmlMainSequenceFile.close();
		m_hXmlInitialSequenceFile.close();
		m_hXmlPinmapSequenceFile.close();
		m_hXmlLimitsSequenceFile.close();
		m_hXmlResultsSequenceFile.close();
		GSET_ERROR1(CSTDFtoXML, eOpenWriteFail, NULL, strFileName.toLatin1().constData());
		return false;
	}
	strFileName = strXmlFileName + ".summary.tsr";
    m_hXmlSummaryTSRSequenceFile.setFileName(strFileName);
    if(!m_hXmlSummaryTSRSequenceFile.open(QIODevice::WriteOnly))
	{
		m_hXmlMainSequenceFile.close();
		m_hXmlInitialSequenceFile.close();
		m_hXmlPinmapSequenceFile.close();
		m_hXmlLimitsSequenceFile.close();
		m_hXmlResultsSequenceFile.close();
		m_hXmlSummarySequenceFile.close();
		GSET_ERROR1(CSTDFtoXML, eOpenWriteFail, NULL, strFileName.toLatin1().constData());
		return false;
	}
	strFileName = strXmlFileName + ".summary.hbr";
    m_hXmlSummaryHBRSequenceFile.setFileName(strFileName);
    if(!m_hXmlSummaryHBRSequenceFile.open(QIODevice::WriteOnly))
	{
		m_hXmlMainSequenceFile.close();
		m_hXmlInitialSequenceFile.close();
		m_hXmlPinmapSequenceFile.close();
		m_hXmlLimitsSequenceFile.close();
		m_hXmlResultsSequenceFile.close();
		m_hXmlSummarySequenceFile.close();
		m_hXmlSummaryTSRSequenceFile.close();
		GSET_ERROR1(CSTDFtoXML, eOpenWriteFail, NULL, strFileName.toLatin1().constData());
		return false;
	}
	strFileName = strXmlFileName + ".summary.sbr";
    m_hXmlSummarySBRSequenceFile.setFileName(strFileName);
    if(!m_hXmlSummarySBRSequenceFile.open(QIODevice::WriteOnly))
	{
		m_hXmlMainSequenceFile.close();
		m_hXmlInitialSequenceFile.close();
		m_hXmlPinmapSequenceFile.close();
		m_hXmlLimitsSequenceFile.close();
		m_hXmlResultsSequenceFile.close();
		m_hXmlSummarySequenceFile.close();
		m_hXmlSummaryTSRSequenceFile.close();
		m_hXmlSummaryHBRSequenceFile.close();
		GSET_ERROR1(CSTDFtoXML, eOpenWriteFail, NULL, strFileName.toLatin1().constData());
		return false;
	}
	strFileName = strXmlFileName + ".summary.pcr";
    m_hXmlSummaryPCRSequenceFile.setFileName(strFileName);
    if(!m_hXmlSummaryPCRSequenceFile.open(QIODevice::WriteOnly))
	{
		m_hXmlMainSequenceFile.close();
		m_hXmlInitialSequenceFile.close();
		m_hXmlPinmapSequenceFile.close();
		m_hXmlLimitsSequenceFile.close();
		m_hXmlResultsSequenceFile.close();
		m_hXmlSummarySequenceFile.close();
		m_hXmlSummaryTSRSequenceFile.close();
		m_hXmlSummaryHBRSequenceFile.close();
		m_hXmlSummarySBRSequenceFile.close();
		GSET_ERROR1(CSTDFtoXML, eOpenWriteFail, NULL, strFileName.toLatin1().constData());
		return false;
	}
	strFileName = strXmlFileName + ".final";
    m_hXmlFinalSequenceFile.setFileName(strFileName);
    if(!m_hXmlFinalSequenceFile.open(QIODevice::WriteOnly))
	{
		m_hXmlMainSequenceFile.close();
		m_hXmlInitialSequenceFile.close();
		m_hXmlPinmapSequenceFile.close();
		m_hXmlLimitsSequenceFile.close();
		m_hXmlResultsSequenceFile.close();
		m_hXmlSummarySequenceFile.close();
		m_hXmlSummaryTSRSequenceFile.close();
		m_hXmlSummaryHBRSequenceFile.close();
		m_hXmlSummarySBRSequenceFile.close();
		m_hXmlSummaryPCRSequenceFile.close();
		GSET_ERROR1(CSTDFtoXML, eOpenWriteFail, NULL, strFileName.toLatin1().constData());
		return false;
	}
#ifdef QT_DEBUG
	// Open Debug Dump file
	strFileName = strStdfFileName + ".dump";
    m_hDebugDumpFile.setFileName(strFileName);
    if(!m_hDebugDumpFile.open(QIODevice::WriteOnly))
	{
		m_hXmlMainSequenceFile.close();
		m_hXmlInitialSequenceFile.close();
		m_hXmlPinmapSequenceFile.close();
		m_hXmlLimitsSequenceFile.close();
		m_hXmlResultsSequenceFile.close();
		m_hXmlSummarySequenceFile.close();
		m_hXmlSummaryTSRSequenceFile.close();
		m_hXmlSummaryHBRSequenceFile.close();
		m_hXmlSummarySBRSequenceFile.close();
		m_hXmlSummaryPCRSequenceFile.close();
		m_hXmlFinalSequenceFile.close();
		GSET_ERROR1(CSTDFtoXML, eOpenWriteFail, NULL, strFileName.toLatin1().constData());
		return false;
	}
#else
	Q_UNUSED(strStdfFileName);
#endif

	// Assign file I/O stream
	m_hXmlMainSequenceStream.setDevice(&m_hXmlMainSequenceFile);
	m_hXmlInitialSequenceStream.setDevice(&m_hXmlInitialSequenceFile);
	m_hXmlPinmapSequenceStream.setDevice(&m_hXmlPinmapSequenceFile);
	m_hXmlLimitsSequenceStream.setDevice(&m_hXmlLimitsSequenceFile);
	m_hXmlResultsSequenceStream.setDevice(&m_hXmlResultsSequenceFile);
	m_hXmlSummarySequenceStream.setDevice(&m_hXmlSummarySequenceFile);
	m_hXmlSummaryTSRSequenceStream.setDevice(&m_hXmlSummaryTSRSequenceFile);
	m_hXmlSummaryHBRSequenceStream.setDevice(&m_hXmlSummaryHBRSequenceFile);
	m_hXmlSummarySBRSequenceStream.setDevice(&m_hXmlSummarySBRSequenceFile);
	m_hXmlSummaryPCRSequenceStream.setDevice(&m_hXmlSummaryPCRSequenceFile);
	m_hXmlFinalSequenceStream.setDevice(&m_hXmlFinalSequenceFile);
#ifdef QT_DEBUG
	m_hDebugDumpStream.setDevice(&m_hDebugDumpFile);
#endif

	return true;
}

void CSTDFtoXML::CloseFiles(void)
{	
	m_hXmlMainSequenceFile.close();
	m_hXmlInitialSequenceFile.close();
	m_hXmlPinmapSequenceFile.close();
	m_hXmlLimitsSequenceFile.close();
	m_hXmlResultsSequenceFile.close();
	m_hXmlSummarySequenceFile.close();
	m_hXmlSummaryTSRSequenceFile.close();
	m_hXmlSummaryHBRSequenceFile.close();
	m_hXmlSummarySBRSequenceFile.close();
	m_hXmlSummaryPCRSequenceFile.close();
	m_hXmlFinalSequenceFile.close();
#ifdef QT_DEBUG
	m_hDebugDumpFile.close();
#endif
}

void CSTDFtoXML::InitXmlSequenceFiles(void)
{
	// Write XML header file
	WriteXmlHeader(m_hXmlMainSequenceStream);

	WriteTag(m_hXmlInitialSequenceStream, "<initial>", m_nInitialSequenceIndentationLevel);
	WriteTag(m_hXmlPinmapSequenceStream, "<pinmap>", m_nPinmapSequenceIndentationLevel);
	WriteTag(m_hXmlLimitsSequenceStream, "<limits>", m_nLimitsSequenceIndentationLevel);
	WriteTag(m_hXmlResultsSequenceStream, "<results>", m_nResultsSequenceIndentationLevel);
	WriteTag(m_hXmlSummarySequenceStream, "<summary>", m_nSummarySequenceIndentationLevel);
	WriteTag(m_hXmlFinalSequenceStream, "<final>", m_nFinalSequenceIndentationLevel);

	m_nMainSequenceIndentationLevel++;
	m_nInitialSequenceIndentationLevel++;
	m_nPinmapSequenceIndentationLevel++;
	m_nLimitsSequenceIndentationLevel++;
	m_nResultsSequenceIndentationLevel++;
	m_nSummarySequenceIndentationLevel++;
	m_nFinalSequenceIndentationLevel++;
}

void CSTDFtoXML::CloseXmlSequenceFiles(void)
{
	m_nMainSequenceIndentationLevel--;
	m_nInitialSequenceIndentationLevel--;
	m_nPinmapSequenceIndentationLevel--;
	m_nLimitsSequenceIndentationLevel--;
	m_nResultsSequenceIndentationLevel--;
	m_nSummarySequenceIndentationLevel--;
	m_nFinalSequenceIndentationLevel--;

	WriteTag(m_hXmlInitialSequenceStream, "</initial>", m_nInitialSequenceIndentationLevel);
	WriteTag(m_hXmlPinmapSequenceStream, "</pinmap>", m_nPinmapSequenceIndentationLevel);
	WriteTag(m_hXmlLimitsSequenceStream, "</limits>", m_nLimitsSequenceIndentationLevel);
	WriteTag(m_hXmlResultsSequenceStream, "</results>", m_nResultsSequenceIndentationLevel);
	WriteTag(m_hXmlSummarySequenceStream, "</summary>", m_nSummarySequenceIndentationLevel);
	WriteTag(m_hXmlFinalSequenceStream, "</final>", m_nFinalSequenceIndentationLevel);

	// Write XML footer file
	WriteXmlFooter(m_hXmlMainSequenceStream);
}

void CSTDFtoXML::UpdateProgressBar(QProgressBar* pProgressBar)
{
	if(pProgressBar != NULL)
	{
		m_nProgress++;
		if(m_nProgress == 1)
			pProgressBar->setMaximum(m_nStdfRecords);
		pProgressBar->setValue(m_nProgress);
	}
}

