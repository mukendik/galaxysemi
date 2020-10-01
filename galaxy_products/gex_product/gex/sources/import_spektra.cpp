//////////////////////////////////////////////////////////////////////
// import_spektra.cpp: Converts a SPEKTRA file to ASCII/STDF
//////////////////////////////////////////////////////////////////////

#include <qfileinfo.h>
#include <qapplication.h>
#include <qlabel.h>
#include <QProgressBar>

#include "engine.h"
#include "import_spektra.h"
#include "export_ascii.h"
#include "cspektraparse_v3.h"
#include "import_constants.h"
#include "gex_shared.h"

// Output File format:
// <GEX Header: version, date, ...> 
// 
// ** <Record Name> **
// REC.<field>  = <value>
// ...

// main.cpp
extern QLabel *			GexScriptStatusLabel;	// Handle to script status text in status bar

// Error map
GBEGIN_ERROR_MAP(CSPEKTRAtoASCII)
	GMAP_ERROR(eOpenReadFail, "Failed to open file.")
	GMAP_ERROR(eOpenWriteFail,"Failed to create ASCII file:\n%s.")
	GMAP_ERROR(eLicenceExpired,QString("License has expired or Data file\n%s out of date...\nPlease contact  %1").arg(GEX_EMAIL_SALES).toLatin1().constData())
	GMAP_ERROR(eSpektraRead, "Invalid file format: Spektra read error.")
GEND_ERROR_MAP(CSPEKTRAtoASCII)

#define RECORD_ERROR "** ERROR: unexpected end of record!!\n"

#ifdef QT_DEBUG
int nFailsPerTest_Flag[2000];
int nFailsPerTest_Limit[2000];
int nPartsFailed;
#endif

//////////////////////////////////////////////////////////////////////
// Construction
//////////////////////////////////////////////////////////////////////
CSPEKTRAtoASCII::CSPEKTRAtoASCII(bool bEvaluationMode)
{
	// Init progress bar ptr
	m_pProgressBar = NULL;

	// Init table of records to process
	for(unsigned int i=0; i<CSpektra_Record_V3::Rec_COUNT; i++)
		m_bRecordsToProcess[i] = false;

	// Init other options
	m_nFieldFilter = CSpektra_Record_V3::FieldFlag_Present;
	m_nFileSizeLimit = 0;

	// Evaluation mode ??
	m_bEvaluationMode = bEvaluationMode;
}

//////////////////////////////////////////////////////////////////////
// Destruction
//////////////////////////////////////////////////////////////////////
CSPEKTRAtoASCII::~CSPEKTRAtoASCII()
{
}

//////////////////////////////////////////////////////////////////////
// Clear Data
//////////////////////////////////////////////////////////////////////
void CSPEKTRAtoASCII::Clear()
{
	// Reset record counter
	m_nSpektraRecords = 0;	
	// Reset progress
	ResetProgress();
}

void CSPEKTRAtoASCII::SetProcessRecord(const int nRecordType)
{
	if(nRecordType < CSpektra_Record_V3::Rec_COUNT)
		m_bRecordsToProcess[nRecordType] = true;
}

//////////////////////////////////////////////////////////////////////
// Write ASCII Header section (starting file)
//////////////////////////////////////////////////////////////////////
void CSPEKTRAtoASCII::WriteAsciiHeader(QTextStream & hStream)
{
	QDateTime	clDateTime;

	// Write ASCII header
	hStream  << GALAXYSEMI_ASCII_HEADER << endl;
    hStream  << "<!--Report created with: "
             << GS::Gex::Engine::GetInstance().Get("AppFullName").toString()
             << " - www.mentor.com-->" << endl;
	hStream  << endl;
	hStream  << "<!--Report Header!-->";
	hStream  << endl;
	hStream  << "** Date            : " << QDateTime::currentDateTime().toString("ddd dd MMM yyyy h:mm:ss");
	hStream  << endl;
	hStream  << "** Records         :";
	if(m_bRecordsToProcess[CSpektra_Record_V3::Rec_Label])
		hStream << " Label";
	if(m_bRecordsToProcess[CSpektra_Record_V3::Rec_TestItem])
		hStream << " TestItem";
	if(m_bRecordsToProcess[CSpektra_Record_V3::Rec_WaferID])
		hStream << " WaferID";
	if(m_bRecordsToProcess[CSpektra_Record_V3::Rec_DeviceID])
		hStream << " DeviceID";
	if(m_bRecordsToProcess[CSpektra_Record_V3::Rec_TestData])
		hStream << " TestData";
	hStream  << endl;
	hStream  << "** Field filter    : " << m_strFieldFilterText;
	hStream  << endl;
	hStream  << "** File size limit : " << m_strFileSizeLimitText;
	hStream  << endl;
	hStream  << "<!--Report Header!-->";
	hStream  << endl;
	hStream  << endl;
}

//////////////////////////////////////////////////////////////////////
// Write ASCII Footer section (end of file)
//////////////////////////////////////////////////////////////////////
void CSPEKTRAtoASCII::WriteAsciiFooter(QTextStream & hStream)
{
	// Write ASCII footer
	int nCurrentFileSize = m_hAsciiFile.size();

	hStream  << endl;

	if(m_bEvaluationMode)
	{
		hStream  << "<!--End of evaluation dump: contact " << GEX_EMAIL_SALES << " for a fully licensed version of the product-->" << endl;
		hStream  << endl;
	}

	if((m_nFileSizeLimit != 0) && (nCurrentFileSize >= m_nFileSizeLimit))
	{
		hStream  << "<!--Report truncated at " << QString::number(nCurrentFileSize/1024)  << " kB" << "-->" << endl;
		hStream  << endl;
	}
	
    hStream  << "<!--Report created with: "
             << GS::Gex::Engine::GetInstance().Get("AppFullName").toString()
             << " - www.mentor.com-->" << endl;
	hStream  << endl;
}

//////////////////////////////////////////////////////////////////////
// Get Error
//////////////////////////////////////////////////////////////////////
void CSPEKTRAtoASCII::GetLastError(QString & strError)
{
	strError = GGET_LASTERRORMSG(CSPEKTRAtoASCII,this);
    if(strError.isEmpty())
        strError = GGET_LASTERRORMSG(CSpektraParse_V3,&m_clSpektraParse);

}

//////////////////////////////////////////////////////////////////////
// Convert 'szFileNameSPEKTRA' SPEKTRA file, to ASCII 'szAsciiFileName' file
//////////////////////////////////////////////////////////////////////
bool CSPEKTRAtoASCII::Convert(const char *szFileNameSPEKTRA, const char *szAsciiFileName,QProgressBar* pProgressBar /*= NULL*/)
{
	CSpektra_Label_V3		clSpektraLabel;
	CSpektra_TestItem_V3	clSpektraTestItem;
	CSpektra_WaferID_V3		clSpektraWaferID;
	CSpektra_DeviceID_V3	clSpektraDeviceID;
	CSpektra_TestData_V3	clSpektraTestData;
	bool					bStopDump;
	int						i, j, nDevices, nDevicesInWafer, nTests;

	// Set progress bar ptr
	m_pProgressBar = pProgressBar;
	
	// Open ASCII file
	if(OpenAsciiFile(szAsciiFileName) == false)
	{
		// Error. Can't create ASCII file!
		return false;
	}

	// Set SPEKTRA file name
	m_strSpektraFile = szFileNameSPEKTRA;

	// Read all file...and build ASCII file as we go...
	int	iStatus;

	// Pass 1: count nb of records (for eventual progress bar), and in debug dump SPEKTRA structure
	// Pass 2: create ASCII file
	for(lPass = 1;lPass <=2; lPass++)
	{
		// Clear data between passes
		Clear();

		// Open SPEKTRA file to read...
		iStatus = m_clSpektraParse.Open((char *)szFileNameSPEKTRA);
		if(iStatus == false)
		{
			// Error. Can't open SPEKTRA file in read mode!
			GSET_ERROR0(CSPEKTRAtoASCII, eOpenReadFail, GGET_LASTERROR(CSpektraParse_V3,&m_clSpektraParse));
			// Close ASCII files
			CloseAsciiFile();
			return false;
		}

		// Read first record from SPEKTRA file: Label record.
		iStatus = m_clSpektraParse.LoadNextRecord(CSpektra_Record_V3::Rec_Label);
		if(iStatus != CSpektraParse_V3::NoError)
		{
            GSET_ERROR0(CSPEKTRAtoASCII, eSpektraRead, GGET_LASTERROR(CSpektraParse_V3,&m_clSpektraParse));
			// Close SPEKTRA file
			m_clSpektraParse.Close();
			// Close ASCII files
			CloseAsciiFile();
			return false;
		}

		// Process Label record
        if(ProcessLabel(&clSpektraLabel, &bStopDump) != true)
		{
			// File timestamp is invalid, and higher than expiration date!
			// Close SPEKTRA file
			m_clSpektraParse.Close();
			// Close ASCII files
			CloseAsciiFile();
			return false;
		}

		// Check if dump should be stopped
		if(bStopDump == true)
		{
			// Force progress bar to completed
			ResetProgress(true);
			// Close SPEKTRA file
			m_clSpektraParse.Close();
			// Close ASCII files
			CloseAsciiFile();
			return true;
		}

		// Get nb of devices and nb of tests
		nTests = clSpektraLabel.m_u2TESTMAX;
		nDevices = clSpektraLabel.m_u2INDEXMAX;

		// Read and process TestItem records
		for(i=0 ; i<nTests ; i++)
		{
			iStatus = m_clSpektraParse.LoadNextRecord(CSpektra_Record_V3::Rec_TestItem);			
			if(iStatus != CSpektraParse_V3::NoError)
			{
				// Close SPEKTRA file
                GSET_ERROR0(CSPEKTRAtoASCII, eSpektraRead, GGET_LASTERROR(CSpektraParse_V3,&m_clSpektraParse));
				m_clSpektraParse.Close();
				// Close ASCII files
				CloseAsciiFile();
				return false;
			}

			// Process TestItem records
			if(m_bRecordsToProcess[CSpektra_Record_V3::Rec_TestItem] == true)
				ProcessRecord((CSpektra_Record_V3*)&clSpektraTestItem, &bStopDump);

			// Check if dump should be stopped
			if(bStopDump == true)
			{
				// Force progress bar to completed
				ResetProgress(true);
				// Close SPEKTRA file
				m_clSpektraParse.Close();
				// Close ASCII files
				CloseAsciiFile();
				return true;
			}
		}

		// Wafer loop
		iStatus = m_clSpektraParse.LoadNextRecord(CSpektra_Record_V3::Rec_WaferID);			
		while(iStatus == CSpektraParse_V3::NoError)
		{
			// Process WaferID record
			if(m_bRecordsToProcess[CSpektra_Record_V3::Rec_WaferID] == true)
				ProcessRecord((CSpektra_Record_V3*)&clSpektraWaferID, &bStopDump);

			// Device loop
			nDevicesInWafer = clSpektraWaferID.m_u2NUMBEROFDEVICES;
			if(nDevicesInWafer == 0)
				nDevicesInWafer = nDevices;

			for(i=0 ; i<nDevicesInWafer ; i++)
			{
				iStatus = m_clSpektraParse.LoadNextRecord(CSpektra_Record_V3::Rec_DeviceID);			
				if(iStatus != CSpektraParse_V3::NoError)
				{
					// Close SPEKTRA file
                    GSET_ERROR0(CSPEKTRAtoASCII, eSpektraRead, GGET_LASTERROR(CSpektraParse_V3,&m_clSpektraParse));
					m_clSpektraParse.Close();
					// Close ASCII files
					CloseAsciiFile();
					return false;
				}

				// Process DeviceID record
				if(m_bRecordsToProcess[CSpektra_Record_V3::Rec_DeviceID] == true)
					ProcessRecord((CSpektra_Record_V3*)&clSpektraDeviceID, &bStopDump);

				// Check if dump should be stopped
				if(bStopDump == true)
				{
					// Force progress bar to completed
					ResetProgress(true);
					// Close SPEKTRA file
					m_clSpektraParse.Close();
					// Close ASCII files
					CloseAsciiFile();
					return true;
				}

				// Test Data loop
				for(j=0 ; j<nTests ; j++)
				{
					iStatus = m_clSpektraParse.LoadNextRecord(CSpektra_Record_V3::Rec_TestData);			
					if(iStatus != CSpektraParse_V3::NoError)
					{
						// Close SPEKTRA file
                        GSET_ERROR0(CSPEKTRAtoASCII, eSpektraRead, GGET_LASTERROR(CSpektraParse_V3,&m_clSpektraParse));
						m_clSpektraParse.Close();
						// Close ASCII files
						CloseAsciiFile();
						return false;
					}

					// Process TestItem record (not in evaluation mode)
					if(!m_bEvaluationMode && m_bRecordsToProcess[CSpektra_Record_V3::Rec_TestData] == true)
						ProcessRecord((CSpektra_Record_V3*)&clSpektraTestData, &bStopDump);

					// Check if dump should be stopped
					if(bStopDump == true)
					{
						// Force progress bar to completed
						ResetProgress(true);
						// Close SPEKTRA file
						m_clSpektraParse.Close();
						// Close ASCII files
						CloseAsciiFile();
						return true;
					}
				}
			}

			iStatus = m_clSpektraParse.LoadNextRecord(CSpektra_Record_V3::Rec_WaferID);			
		}

		// Save nb of records
		m_nSpektraRecordsToProcess = m_nSpektraRecords;

		// Close input SPEKTRA file between passes.
		m_clSpektraParse.Close();
	}	// 2 passes.

	// Force progress bar to completed
	ResetProgress(true);

	// Convertion successful, close ASCII files.
	CloseAsciiFile();

	return true;
}

// Extract record data and dump it
void CSPEKTRAtoASCII::ProcessRecord(CSpektra_Record_V3* pclRecord, bool* pbStopDump)
{
	QString	strAsciiString;

	*pbStopDump = false;

	// Update record counter
	m_nSpektraRecords++;

	// Pass#1: update counters, dump if in debug
	if(lPass == 1)
	{
	}

	// Pass#2: write record to ASCII
	if(lPass == 2)
	{
		if(((m_nFieldFilter & CSpektra_Record_V3::FieldFlag_None) == 0) && (m_clSpektraParse.ReadRecord(pclRecord) == false))
		{
			// Error reading SPEKTRA file. Dump data read and add an error message.
			// Make sure to get only data that has been read (FieldFlag_Present).
			pclRecord->GetAsciiRecord(strAsciiString, m_nFieldFilter | CSpektra_Record_V3::FieldFlag_Present);
			m_hAsciiStream  << strAsciiString;
			m_hAsciiStream  << RECORD_ERROR;
			m_hAsciiStream  << endl;
		}
		else
		{
			pclRecord->GetAsciiRecord(strAsciiString, m_nFieldFilter);
			m_hAsciiStream  << strAsciiString;
			m_hAsciiStream  << endl;
		}

		// Update progress bar
		if(UpdateProgress() == false)
			*pbStopDump = true;
	}
}

/////////////////////////////////////////////////////////////////////////////
// Read SPEKTRA.Label record, dump it to ASCII file...
/////////////////////////////////////////////////////////////////////////////
bool CSPEKTRAtoASCII::ProcessLabel(CSpektra_Label_V3* pclSpektraLabel, bool* pbStopDump)	// Extract Label data
{
	QString	strAsciiString;

	*pbStopDump = false;

	// Update record counter
	if(m_bRecordsToProcess[CSpektra_Record_V3::Rec_Label] == true)
		m_nSpektraRecords++;

	// Pass#1: check license validity, update counters, dump if in debug
	if(lPass == 1)
	{
		// Check if file date is not more recent than license expiration date!
		if(m_clSpektraParse.ReadRecord((CSpektra_Record_V3 *)pclSpektraLabel) == false)
		{
			// Error reading SPEKTRA file
			// Convertion failed.
			GSET_ERROR0(CSPEKTRAtoASCII, eSpektraRead, GGET_LASTERROR(CSpektraParse_V3,&m_clSpektraParse));
			return false;
		}

		return true;
	}

	// Pass#2: write record to ASCII
	if((lPass == 2) && (m_bRecordsToProcess[CSpektra_Record_V3::Rec_Label] == true))
	{
		if(((m_nFieldFilter & CSpektra_Record_V3::FieldFlag_None) == 0) && (m_clSpektraParse.ReadRecord((CSpektra_Record_V3 *)pclSpektraLabel) == false))
		{
			// Error reading SPEKTRA file. Dump data read and add an error message.
			// Make sure to get only data that has been read (FieldFlag_Present).
			pclSpektraLabel->GetAsciiRecord(strAsciiString, m_nFieldFilter | CSpektra_Record_V3::FieldFlag_Present);
			m_hAsciiStream  << strAsciiString;
			m_hAsciiStream  << RECORD_ERROR;
			m_hAsciiStream  << endl;
		}
		else
		{
			pclSpektraLabel->GetAsciiRecord(strAsciiString, m_nFieldFilter);
			m_hAsciiStream  << strAsciiString;
			m_hAsciiStream  << endl;
		}

		// Update progress bar
		if(UpdateProgress() == false)
			*pbStopDump = true;

		return true;
	}

	return true;
}

bool CSPEKTRAtoASCII::OpenAsciiFile(const QString & strAsciiFileName)
{
	// Open ASCII files
    m_hAsciiFile.setFileName(strAsciiFileName);
    if(!m_hAsciiFile.open(QIODevice::WriteOnly))
	{
		GSET_ERROR1(CSPEKTRAtoASCII, eOpenWriteFail, NULL, strAsciiFileName.toLatin1().constData());
		return false;
	}

	// Assign file I/O stream
	m_hAsciiStream.setDevice(&m_hAsciiFile);

	// Write ASCII header file
	WriteAsciiHeader(m_hAsciiStream);

	return true;
}

void CSPEKTRAtoASCII::CloseAsciiFile(void)
{	
	// Write ASCII footer file
	WriteAsciiFooter(m_hAsciiStream);

	m_hAsciiFile.close();
}

void CSPEKTRAtoASCII::ResetProgress(bool bForceCompleted /*= false*/)
{
	if(bForceCompleted == true)
		m_nProgress = 100;
	else
		m_nProgress = 0;

	if(m_pProgressBar != NULL)
		m_pProgressBar->setValue(m_nProgress);
}

bool CSPEKTRAtoASCII::UpdateProgress(void)
{
	int nProgress;

	if(m_nFileSizeLimit == 0)
	{
		// No file size limit
		if((m_pProgressBar != NULL) && (m_nSpektraRecordsToProcess != 0))
		{
			nProgress = m_nSpektraRecords*100/m_nSpektraRecordsToProcess;
			if(nProgress != m_nProgress)
			{
				m_nProgress = nProgress;
				m_pProgressBar->setValue(m_nProgress);
			}
		}
	}
	else
	{
		// A file size limit has been set
		int nCurrentFileSize = m_hAsciiFile.size();
		int nFileSize = nCurrentFileSize < m_nFileSizeLimit ? nCurrentFileSize : m_nFileSizeLimit;
		if(m_pProgressBar != NULL)
		{
			nProgress = nFileSize*100/m_nFileSizeLimit;
			if(nProgress != m_nProgress)
			{
				m_nProgress = nProgress;
				m_pProgressBar->setValue(m_nProgress);
			}
		}
		if(nFileSize == m_nFileSizeLimit)
			return false;
	}

	return true;
}

// Error map
GBEGIN_ERROR_MAP(CSPEKTRAtoSTDF)
	GMAP_ERROR(eOpenReadFail, "Failed to read file.")
	GMAP_ERROR(eOpenWriteFail,"Failed to create STDF file:\n%s.")
	GMAP_ERROR(eLicenceExpired,QString("License has expired or Data file\n%s out of date...\nPlease contact %1").arg(GEX_EMAIL_SALES).toLatin1().constData())
	GMAP_ERROR(eSpektraRead, "Invalid file format: Spektra read error.")
GEND_ERROR_MAP(CSPEKTRAtoSTDF)

#define RECORD_ERROR "** ERROR: unexpected end of record!!\n"

CSPEKTRABinningInfo::CSPEKTRABinningInfo()
{
	Reset();
}

CSPEKTRABinningInfo::~CSPEKTRABinningInfo()
{
}

void CSPEKTRABinningInfo::Reset()
{
	m_nCount = 0;
	m_cPassFail = 'F';
}

//////////////////////////////////////////////////////////////////////
// Construction
//////////////////////////////////////////////////////////////////////
CSPEKTRAtoSTDF::CSPEKTRAtoSTDF()
{
	// Init progress bar ptr
	m_pProgressBar = NULL;

	// TestItem array
	m_pTestItems = NULL;
}

//////////////////////////////////////////////////////////////////////
// Destruction
//////////////////////////////////////////////////////////////////////
CSPEKTRAtoSTDF::~CSPEKTRAtoSTDF()
{
	if(m_pTestItems != NULL)
		delete [] m_pTestItems;
}

//////////////////////////////////////////////////////////////////////
// Clear Data
//////////////////////////////////////////////////////////////////////
void CSPEKTRAtoSTDF::Clear()
{
	int i;

	// Reset record counter
	m_nSpektraRecords = 0;	

	// Reset progress
	ResetProgress();

	// For pass 2, initialize some variables and counters
	if(lPass == 2)
	{
		// Init counters
		m_nWaferCount = 0;
		m_nParts = 0;
		m_nGoodParts = 0;
		m_nFailParts = 0;
		m_nPartsInWafer = 0;
		m_nGoodPartsInWafer = 0;
		m_nCurrentWaferID = -1;
		m_nCurrentDeviceID = -1;
		m_nCurrentBinning = 0;
		m_nTests = 0;
		m_nFailTestsInDevice = 0;
		m_bFirstDevice = true;
		for(i=0; i<251; i++)
			m_pclBinnings[i].Reset();
	}
}

//////////////////////////////////////////////////////////////////////
// Get Error
//////////////////////////////////////////////////////////////////////
void CSPEKTRAtoSTDF::GetLastError(QString & strError)
{
	strError = GGET_LASTERRORMSG(CSPEKTRAtoSTDF,this);
    if(strError.isEmpty())
        strError = GGET_LASTERRORMSG(CSpektraParse_V3,&m_clSpektraParse);
}

//////////////////////////////////////////////////////////////////////
// Check if File is compatible with SPEKTRA format
//////////////////////////////////////////////////////////////////////
bool CSPEKTRAtoSTDF::IsCompatible(const char *szFileName)
{
	QFileInfo cFileInfo(szFileName);
	// Only check if have the good extension !!!

    return (cFileInfo.suffix().toLower() == "dta");
}

//////////////////////////////////////////////////////////////////////
// Convert 'szFileNameSPEKTRA' SPEKTRA file, to STDF 'szStdfFileName' file
//////////////////////////////////////////////////////////////////////
bool CSPEKTRAtoSTDF::Convert(const char *szFileNameSPEKTRA, const char *szStdfFileName, QProgressBar* pProgressBar /*= NULL*/)
{
	CSpektra_Label_V3		clSpektraLabel;
	CSpektra_TestItem_V3	clSpektraTestItem;
	CSpektra_WaferID_V3		clSpektraWaferID;
	CSpektra_DeviceID_V3	clSpektraDeviceID;
	CSpektra_TestData_V3	clSpektraTestData;
	int						i;

#ifdef QT_DEBUG
	memset(nFailsPerTest_Flag, 0, 1000*sizeof(int));
	memset(nFailsPerTest_Limit, 0, 1000*sizeof(int));
	nPartsFailed = 0;
#endif

	// Set progress bar ptr
	m_pProgressBar = pProgressBar;
	
	// Open STDF file
	if(OpenStdfFile(szStdfFileName) == false)
	{
		// Error. Can't create STDF file!
		return false;
	}

	// Set SPEKTRA file name
	m_strSpektraFile = szFileNameSPEKTRA;

	// Read all file...and build STDF file as we go...
	int	iStatus;

	// Pass 1: count nb of records (for eventual progress bar), and in debug dump SPEKTRA structure
	// Pass 2: create STDF file
	for(lPass = 1;lPass <=2; lPass++)
	{
		// Clear data between passes
		Clear();

		// Open SPEKTRA file to read...
		iStatus = m_clSpektraParse.Open((char *)szFileNameSPEKTRA);
		if(iStatus == false)
		{
			// Error. Can't open SPEKTRA file in read mode!
			GSET_ERROR0(CSPEKTRAtoSTDF, eOpenReadFail, GGET_LASTERROR(CSpektraParse_V3,&m_clSpektraParse));
			// Close ASCII files
			CloseStdfFile();
			return false;
		}

		// Read first record from SPEKTRA file: Label record.
		iStatus = m_clSpektraParse.LoadNextRecord(CSpektra_Record_V3::Rec_Label);
		if(iStatus != CSpektraParse_V3::NoError)
		{
			// Close SPEKTRA file
            GSET_ERROR0(CSPEKTRAtoSTDF, eSpektraRead, GGET_LASTERROR(CSpektraParse_V3,&m_clSpektraParse));
			m_clSpektraParse.Close();
			// Close STDF files
			CloseStdfFile();
			return false;
		}

		// Process Label record
        if(ProcessLabel(&clSpektraLabel) != true)
		{
			// File timestamp is invalid, and higher than expiration date!
			// Close SPEKTRA file
			m_clSpektraParse.Close();
			// Close ASCII files
			CloseStdfFile();
			return false;
		}

		// Get nb of devices and nb of tests
		m_nTests = clSpektraLabel.m_u2TESTMAX;
		m_nParts = clSpektraLabel.m_u2INDEXMAX;

		// Allocate memory for TestItem array
		if(m_pTestItems == NULL)
			m_pTestItems = new CSpektra_TestItem_V3[m_nTests];

		// Read and process TestItem records
		for(m_nTestItemIndex=0 ; m_nTestItemIndex<m_nTests ; m_nTestItemIndex++)
		{
			iStatus = m_clSpektraParse.LoadNextRecord(CSpektra_Record_V3::Rec_TestItem);			
			if(iStatus != CSpektraParse_V3::NoError)
			{
				// Close SPEKTRA file
                GSET_ERROR0(CSPEKTRAtoSTDF, eSpektraRead, GGET_LASTERROR(CSpektraParse_V3,&m_clSpektraParse));
				m_clSpektraParse.Close();
				// Close STDF files
				CloseStdfFile();
				return false;
			}

			// Process TestItem records
			if(ProcessTestItem(&clSpektraTestItem) != true)
			{
				// File timestamp is invalid, and higher than expiration date!
				// Close SPEKTRA file
				m_clSpektraParse.Close();
				// Close ASCII files
				CloseStdfFile();
				return false;
			}
		}

		// Wafer loop
		iStatus = m_clSpektraParse.LoadNextRecord(CSpektra_Record_V3::Rec_WaferID);			
		while(iStatus == CSpektraParse_V3::NoError)
		{
			// Process WaferID record
			ProcessWaferID(&clSpektraWaferID);

			// Device loop
			m_nPartsInWafer = clSpektraWaferID.m_u2NUMBEROFDEVICES;
			if(m_nPartsInWafer == 0)
				m_nPartsInWafer = m_nParts;

			for(i=0 ; i<m_nPartsInWafer ; i++)
			{
				iStatus = m_clSpektraParse.LoadNextRecord(CSpektra_Record_V3::Rec_DeviceID);			
				if(iStatus != CSpektraParse_V3::NoError)
				{
					// Close SPEKTRA file
                    GSET_ERROR0(CSPEKTRAtoSTDF, eSpektraRead, GGET_LASTERROR(CSpektraParse_V3,&m_clSpektraParse));
					m_clSpektraParse.Close();
					// Close STDF files
					CloseStdfFile();
					return false;
				}

				// Process DeviceID record
				ProcessDeviceID(&clSpektraDeviceID);

				// Test Data loop
				for(m_nTestItemIndex=0 ; m_nTestItemIndex<m_nTests ; m_nTestItemIndex++)
				{
					iStatus = m_clSpektraParse.LoadNextRecord(CSpektra_Record_V3::Rec_TestData);			
					if(iStatus != CSpektraParse_V3::NoError)
					{
						// Close SPEKTRA file
                        GSET_ERROR0(CSPEKTRAtoSTDF, eSpektraRead, GGET_LASTERROR(CSpektraParse_V3,&m_clSpektraParse));
						m_clSpektraParse.Close();
						// Close STDF files
						CloseStdfFile();
						return false;
					}

					// Process TestData record (not in evaluation mode)
					ProcessTestData(&clSpektraTestData);
				}
			} // Device loop

			if((lPass == 2) && (m_nCurrentDeviceID != -1))	WritePRR();

			iStatus = m_clSpektraParse.LoadNextRecord(CSpektra_Record_V3::Rec_WaferID);			
		} // Wafer loop

		// Pass 2: write WRR for previous wafer if any, and final sequence records
		if(lPass == 2)
		{
			if(m_nCurrentWaferID != -1)	WriteWRR();

			// Write HBR records
			WriteHBRs();

			// Write SBR
			WriteSBRs();

			// Write PCR
			WritePCR();

			// Write MRR
			WriteMRR();
		}

		// Save nb of records
		m_nSpektraRecordsToProcess = m_nSpektraRecords;

		// Close input SPEKTRA file between passes.
		m_clSpektraParse.Close();
	}	// 2 passes.

	// Force progress bar to completed
	ResetProgress(true);

	// Convertion successful, close STDF files.
	CloseStdfFile();

#ifdef QT_DEBUG
	// Create debug CSV file
	CreateDebugCsvFile();
#endif

	return true;
}

/////////////////////////////////////////////////////////////////////////////
// Create debug CSV file...
/////////////////////////////////////////////////////////////////////////////
void CSPEKTRAtoSTDF::CreateDebugCsvFile()
{
#ifdef QT_DEBUG
	QString strCsvFile;
	FILE	*hDebugFile;
	int		i;

	strCsvFile = m_strSpektraFile + ".csv";
	hDebugFile = fopen(strCsvFile.toLatin1().constData(), "w");
	if(hDebugFile)
	{
		fprintf(hDebugFile, "**** NB FAILS PER TEST:\n");
		fprintf(hDebugFile, "Test nb,Item Group Code,Item Code,BE Conditions & Flags,Result As Bias Flags,"
							"flag fails,limit fails,"
							"Limit name,Limit Value,Limit Min,Limit Max,Limit Unit,Bias1 name,Bias1 Value,Bias1 Unit,Bias2 name,Bias2 Value,Bias2 Unit,TimeCond name,Time Value,Time Unit\n");
		for(i=0; i<m_nTests; i++)
		{
			fprintf(hDebugFile, "%d,0x%02X,0x%04X,0x%02X,0x%02X,%d,%d,%s,%g,%g,%g,%s,%s,%g,%s,%s,%g,%s,%s,%g,%s,\n",
				(int)(m_pTestItems[i].m_u1TESTNUMBER),
				(int)(m_pTestItems[i].m_u1ITEMGROUPCODE),
				(int)(m_pTestItems[i].m_u2ITEMCODE),
				(int)(m_pTestItems[i].m_b1BECONDITIONANDFLAGS),
				(int)(m_pTestItems[i].m_b1RESULTASBIASFLAGS),
				nFailsPerTest_Flag[i],
				nFailsPerTest_Limit[i],
				m_pTestItems[i].m_cnLIMITITEMNAME.toLatin1().constData(),
				m_pTestItems[i].m_r4LIMITVALUE,
				m_pTestItems[i].m_r4LIMITMIN,
				m_pTestItems[i].m_r4LIMITMAX,
				m_pTestItems[i].m_cnLIMITUNIT.toLatin1().constData(),
				m_pTestItems[i].m_cnBIAS1NAME.toLatin1().constData(),
				m_pTestItems[i].m_r4BIAS1VALUE,
				m_pTestItems[i].m_cnBIAS1UNIT.toLatin1().constData(),
				m_pTestItems[i].m_cnBIAS2NAME.toLatin1().constData(),
				m_pTestItems[i].m_r4BIAS2VALUE,
				m_pTestItems[i].m_cnBIAS2UNIT.toLatin1().constData(),
				m_pTestItems[i].m_cnTIMECONDITIONNAME.toLatin1().constData(),
				m_pTestItems[i].m_r4TIMEVALUE,
				m_pTestItems[i].m_cnTIMEUNIT.toLatin1().constData());
		}
		fprintf(hDebugFile, "**** OTHER COUNTERS:\n");
		fprintf(hDebugFile, "m_nWaferCount = %d\n", m_nWaferCount);
		fprintf(hDebugFile, "m_nParts = %d\n", m_nParts);
		fprintf(hDebugFile, "m_nGoodParts = %d\n", m_nGoodParts);
		fprintf(hDebugFile, "m_nFailParts = %d\n", m_nFailParts);
		fprintf(hDebugFile, "m_nPartsInWafer = %d\n", m_nPartsInWafer);
		fprintf(hDebugFile, "m_nGoodPartsInWafer = %d\n", m_nGoodPartsInWafer);
		fprintf(hDebugFile, "**** BINNINGS:\n");
		for(i=0; i<251; i++)
		{
			if(m_pclBinnings[i].m_nCount != 0)
			{
				fprintf(hDebugFile, "Bin %d (%c): %d parts\n", i, m_pclBinnings[i].m_cPassFail, m_pclBinnings[i].m_nCount);
			}
		}
		fclose(hDebugFile);
	}
#endif
}

/////////////////////////////////////////////////////////////////////////////
// Read SPEKTRA.Label record, dump it to STDF file...
/////////////////////////////////////////////////////////////////////////////
bool CSPEKTRAtoSTDF::ProcessLabel(CSpektra_Label_V3* pclSpektraLabel)	// Extract Label data
{
	// Update record counter
	m_nSpektraRecords++;

	// Pass#1: check license validity, update counters, dump if in debug
	if(lPass == 1)
	{
		// Check if file date is not more recent than license expiration date!
		if(m_clSpektraParse.ReadRecord((CSpektra_Record_V3 *)pclSpektraLabel) == false)
		{
			// Error reading SPEKTRA file
			// Convertion failed.
			GSET_ERROR0(CSPEKTRAtoSTDF, eSpektraRead, GGET_LASTERROR(CSpektraParse_V3,&m_clSpektraParse));
			return false;
		}

		return true;
	}

	// Pass#2: write record to STDF
	if(lPass == 2)
	{
		// Write MIR
		WriteMIR(pclSpektraLabel);

		// Update progress bar
		UpdateProgress();

		return true;
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////
// Read SPEKTRA.TestItem record, dump it to STDF file...
/////////////////////////////////////////////////////////////////////////////
bool CSPEKTRAtoSTDF::ProcessTestItem(CSpektra_TestItem_V3* pclSpektraTestItem)	// Extract TestItem data
{
	// Update record counter
	m_nSpektraRecords++;

	// Pass#1: nothing to do
	if(lPass == 1)
	{
		if(m_clSpektraParse.ReadRecord((CSpektra_Record_V3 *)pclSpektraTestItem) == false)
		{
			// Error reading SPEKTRA file.
			// Convertion failed.
			GSET_ERROR0(CSPEKTRAtoSTDF, eSpektraRead, GGET_LASTERROR(CSpektraParse_V3,&m_clSpektraParse));
			return false;
		}
		else
		{
			// Save record for use when creating PTRs
			m_pTestItems[m_nTestItemIndex] = *pclSpektraTestItem;
		}

		return true;
	}

	// Pass#2: write record to STDF
	if(lPass == 2)
	{
		// Update progress bar
		UpdateProgress();

		return true;
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////
// Read SPEKTRA.WaferID record, dump it to STDF file...
/////////////////////////////////////////////////////////////////////////////
bool CSPEKTRAtoSTDF::ProcessWaferID(CSpektra_WaferID_V3* pclSpektraWaferID)	
{
	// Update record counter
	m_nSpektraRecords++;

	// Pass#1: nothing to do
	if(lPass == 1)
	{
		return true;
	}

	// Pass#2: write record to STDF
	if(lPass == 2)
	{
		if(m_clSpektraParse.ReadRecord((CSpektra_Record_V3 *)pclSpektraWaferID) == false)
		{
			// Error reading SPEKTRA file.
			// Convertion failed.
			GSET_ERROR0(CSPEKTRAtoSTDF, eSpektraRead, GGET_LASTERROR(CSpektraParse_V3,&m_clSpektraParse));
			return false;
		}
		else
		{
			// If valid wafer data, write WIR record to STDF file.
			// Also write WRR for previous wafer if any
			if(m_nCurrentWaferID != -1)
			{
				// Write WRR
				WriteWRR();
			}
			m_nCurrentWaferID = -1;
			if(pclSpektraWaferID->m_u1WAFERNUMBER != 0)
			{
				// Write WIR
				WriteWIR(pclSpektraWaferID);
			}
		}

		// Update progress bar
		UpdateProgress();

		return true;
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////
// Read SPEKTRA.DeviceID record, dump it to STDF file...
/////////////////////////////////////////////////////////////////////////////
bool CSPEKTRAtoSTDF::ProcessDeviceID(CSpektra_DeviceID_V3* pclSpektraDeviceID)	
{
	// Update record counter
	m_nSpektraRecords++;

	// Pass#1: nothing to do
	if(lPass == 1)
	{
		return true;
	}

	// Pass#2: write record to STDF
	if(lPass == 2)
	{
		if(m_clSpektraParse.ReadRecord((CSpektra_Record_V3 *)pclSpektraDeviceID) == false)
		{
			// Error reading SPEKTRA file.
			// Convertion failed.
			GSET_ERROR0(CSPEKTRAtoSTDF, eSpektraRead, GGET_LASTERROR(CSpektraParse_V3,&m_clSpektraParse));
			return false;
		}
		else
		{
			// Write PIR record to STDF file.
			// Also write PRR for previous device if any
			if(m_nCurrentDeviceID != -1)
			{
				// Write PRR
				WritePRR();
			}
			WritePIR(pclSpektraDeviceID);
		}

		// Update progress bar
		UpdateProgress();

		return true;
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////
// Read SPEKTRA.TestData record, dump it to STDF file...
/////////////////////////////////////////////////////////////////////////////
bool CSPEKTRAtoSTDF::ProcessTestData(CSpektra_TestData_V3* pclSpektraTestData)	
{
	// Update record counter
	m_nSpektraRecords++;

	// Pass#1: nothing to do
	if(lPass == 1)
	{
		if(m_clSpektraParse.ReadRecord((CSpektra_Record_V3 *)pclSpektraTestData) == false)
		{
			// Error reading SPEKTRA file.
			// Convertion failed.
			GSET_ERROR0(CSPEKTRAtoSTDF, eSpektraRead, GGET_LASTERROR(CSpektraParse_V3,&m_clSpektraParse));
			return false;
		}
		else
		{
			// Update Test Item
			UpdateTestItem(pclSpektraTestData);
		}

		return true;
	}

	// Pass#2: write record to STDF
	if(lPass == 2)
	{
		if(m_clSpektraParse.ReadRecord((CSpektra_Record_V3 *)pclSpektraTestData) == false)
		{
			// Error reading SPEKTRA file.
			// Convertion failed.
			GSET_ERROR0(CSPEKTRAtoSTDF, eSpektraRead, GGET_LASTERROR(CSpektraParse_V3,&m_clSpektraParse));
			return false;
		}
		else
		{
			// Write PTR
			WritePTR(pclSpektraTestData);
		}

		// Update progress bar
		UpdateProgress();

		return true;
	}

	return true;
}

bool CSPEKTRAtoSTDF::OpenStdfFile(const QString & strStdfFileName)
{
    GS::StdLib::StdfRecordReadInfo	RecordReadInfo;

	// Open STDF file
    if(m_clStdf.Open(strStdfFileName.toLatin1().constData(), STDF_WRITE) != GS::StdLib::Stdf::NoError)
	{
		GSET_ERROR1(CSPEKTRAtoSTDF, eOpenWriteFail, NULL, strStdfFileName.toLatin1().constData());
		return false;
	}

	// Write FAR record
	RecordReadInfo.iRecordType = 0;
	RecordReadInfo.iRecordSubType = 10;
	m_clStdf.WriteHeader(&RecordReadInfo);
	m_clStdf.WriteByte(1);					// SUN CPU type
	m_clStdf.WriteByte(4);					// STDF V4
	m_clStdf.WriteRecord();

	return true;
}

void CSPEKTRAtoSTDF::CloseStdfFile(void)
{	
	m_clStdf.Close();
}

void CSPEKTRAtoSTDF::ResetProgress(bool bForceCompleted /*= false*/)
{
	if(bForceCompleted == true)
		m_nProgress = 100;
	else
		m_nProgress = 0;

	if(m_pProgressBar != NULL)
		m_pProgressBar->setValue(m_nProgress);
}

void CSPEKTRAtoSTDF::UpdateProgress(void)
{
	int nProgress;

	if((m_pProgressBar != NULL) && (m_nSpektraRecordsToProcess != 0))
	{
		nProgress = m_nSpektraRecords*100/m_nSpektraRecordsToProcess;
		if(nProgress != m_nProgress)
		{
			m_nProgress = nProgress;
			m_pProgressBar->setValue(m_nProgress);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// Update test item record...
/////////////////////////////////////////////////////////////////////////////
void CSPEKTRAtoSTDF::UpdateTestItem(CSpektra_TestData_V3* pclSpektraTestData)	
{
	// Update limit attributes (only if test has a valid limit
	if(m_pTestItems[m_nTestItemIndex].m_bHaslimit)
	{
		if(pclSpektraTestData->m_b1FLAGS & 0x80)
		{
			// Test is fail
			if(pclSpektraTestData->m_r4TESTRESULTVALUE  == m_pTestItems[m_nTestItemIndex].m_r4LIMITVALUE)
				m_pTestItems[m_nTestItemIndex].m_bIsStrictLimit = true;
			else if(pclSpektraTestData->m_r4TESTRESULTVALUE  > m_pTestItems[m_nTestItemIndex].m_r4LIMITVALUE)
				m_pTestItems[m_nTestItemIndex].m_bIsHighLimit = true;
			else if(pclSpektraTestData->m_r4TESTRESULTVALUE  < m_pTestItems[m_nTestItemIndex].m_r4LIMITVALUE)
				m_pTestItems[m_nTestItemIndex].m_bIsHighLimit = false;
		}
		else
		{
			// Test is pass
			if(pclSpektraTestData->m_r4TESTRESULTVALUE  == m_pTestItems[m_nTestItemIndex].m_r4LIMITVALUE)
				m_pTestItems[m_nTestItemIndex].m_bIsStrictLimit = false;
			else if(pclSpektraTestData->m_r4TESTRESULTVALUE  > m_pTestItems[m_nTestItemIndex].m_r4LIMITVALUE)
				m_pTestItems[m_nTestItemIndex].m_bIsHighLimit = false;
			else if(pclSpektraTestData->m_r4TESTRESULTVALUE  < m_pTestItems[m_nTestItemIndex].m_r4LIMITVALUE)
				m_pTestItems[m_nTestItemIndex].m_bIsHighLimit = true;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// Write PTR record...
/////////////////////////////////////////////////////////////////////////////
void CSPEKTRAtoSTDF::WritePTR(CSpektra_TestData_V3* pclSpektraTestData)	
{
    GS::StdLib::StdfRecordReadInfo	RecordReadInfo;
	BYTE				nFlags = 0;
	QString				strTestName, strName, strUnit;

	RecordReadInfo.iRecordType = 15;
	RecordReadInfo.iRecordSubType = 10;
	m_clStdf.WriteHeader(&RecordReadInfo);
	m_clStdf.WriteDword((BYTE)pclSpektraTestData->m_u1TESTNUMBER);						// TEST_NUM		(from TESTNUMBER)
	m_clStdf.WriteByte((BYTE)1);														// HEAD_NUM
	m_clStdf.WriteByte((BYTE)1);														// SITE_NUM
	if(pclSpektraTestData->m_b1FLAGS & 0x80)
	{
#ifdef QT_DEBUG
		nFailsPerTest_Flag[m_nTestItemIndex]++;
#endif

		// Test is fail
		m_nFailTestsInDevice++;
		nFlags |= 0x80;
	}
	else
	{
		// Test is pass
	}
	if(pclSpektraTestData->m_b1FLAGS & 0x10)
	{
		// No Data
		nFlags |= 0x02;
	}
	m_clStdf.WriteByte((BYTE)nFlags);													// TEST_FLG
	if(m_pTestItems[m_nTestItemIndex].m_bIsStrictLimit == true)
		nFlags = 0;
	else
		nFlags = 0xc0;
	m_clStdf.WriteByte((BYTE)nFlags);													// PARM_FLG
	m_clStdf.WriteFloat((FLOAT)pclSpektraTestData->m_r4TESTRESULTVALUE);				// RESULT		(from RESULTVALUE)

	if(m_bFirstDevice)
	{
		// First device: write test name and limits
		// Test name = T<test nb>:<limit name>_<limit value>_<limit unit>:<bias1 name>_<bias1 value>_<bias1 unit>:<bias2 name>_<bias2 value>_<bias2 unit>:<timecondition name>_<time value>_<time unit>
		// If 1 field is empty (name="", value=0, unit=""), use just 1 '_'.
		strTestName = "T" + QString::number(pclSpektraTestData->m_u1TESTNUMBER);
		strName = m_pTestItems[m_nTestItemIndex].m_cnLIMITITEMNAME.trimmed();
		strUnit = m_pTestItems[m_nTestItemIndex].m_cnLIMITUNIT.trimmed();
		if(strName.isEmpty() && strUnit.isEmpty() && (m_pTestItems[m_nTestItemIndex].m_r4LIMITVALUE == 0.0F))
			strTestName += ":_";
		else
			strTestName += ":" + strName + "_" + QString::number(m_pTestItems[m_nTestItemIndex].m_r4LIMITVALUE) + strUnit;
		strName = m_pTestItems[m_nTestItemIndex].m_cnBIAS1NAME.trimmed();
		strUnit = m_pTestItems[m_nTestItemIndex].m_cnBIAS1UNIT.trimmed();
		if(strName.isEmpty() && strUnit.isEmpty() && (m_pTestItems[m_nTestItemIndex].m_r4BIAS1VALUE == 0.0F))
			strTestName += ":_";
		else
			strTestName += ":" + strName + "_" + QString::number(m_pTestItems[m_nTestItemIndex].m_r4BIAS1VALUE) + strUnit;
		strName = m_pTestItems[m_nTestItemIndex].m_cnBIAS2NAME.trimmed();
		strUnit = m_pTestItems[m_nTestItemIndex].m_cnBIAS2UNIT.trimmed();
		if(strName.isEmpty() && strUnit.isEmpty() && (m_pTestItems[m_nTestItemIndex].m_r4BIAS2VALUE == 0.0F))
			strTestName += ":_";
		else
			strTestName += ":" + strName + "_" + QString::number(m_pTestItems[m_nTestItemIndex].m_r4BIAS2VALUE) + strUnit;
		strName = m_pTestItems[m_nTestItemIndex].m_cnTIMECONDITIONNAME.trimmed();
		strUnit = m_pTestItems[m_nTestItemIndex].m_cnTIMEUNIT.trimmed();
		if(strName.isEmpty() && strUnit.isEmpty() && (m_pTestItems[m_nTestItemIndex].m_r4TIMEVALUE == 0.0F))
			strTestName += ":_";
		else
			strTestName += ":" + strName + "_" + QString::number(m_pTestItems[m_nTestItemIndex].m_r4TIMEVALUE) + strUnit;
		m_clStdf.WriteString(strTestName.toLatin1().constData());							// TEST_TXT		(from LIMITITEMNAME field of TestItem record)
		m_clStdf.WriteString("");														// ALARM_ID
		if(m_pTestItems[m_nTestItemIndex].m_bHaslimit == false)
		{
			m_clStdf.WriteByte((BYTE)0xce);												// OPT_FLAG
			m_clStdf.WriteByte((BYTE)0);												// RES_SCAL
			m_clStdf.WriteByte((BYTE)0);												// LLM_SCAL
			m_clStdf.WriteByte((BYTE)0);												// HLM_SCAL
			m_clStdf.WriteFloat(0.0F);													// LO_LIMIT
			m_clStdf.WriteFloat(0.0F);													// HI_LIMIT
		}
		else if(m_pTestItems[m_nTestItemIndex].m_bIsHighLimit)
		{
			m_clStdf.WriteByte((BYTE)0x4e);												// OPT_FLAG
			m_clStdf.WriteByte((BYTE)0);												// RES_SCAL
			m_clStdf.WriteByte((BYTE)0);												// LLM_SCAL
			m_clStdf.WriteByte((BYTE)0);												// HLM_SCAL
			m_clStdf.WriteFloat(0.0F);													// LO_LIMIT
			m_clStdf.WriteFloat((FLOAT)m_pTestItems[m_nTestItemIndex].m_r4LIMITVALUE);	// HI_LIMIT
		}
		else
		{
			m_clStdf.WriteByte((BYTE)0x8e);												// OPT_FLAG
			m_clStdf.WriteByte((BYTE)0);												// RES_SCAL
			m_clStdf.WriteByte((BYTE)0);												// LLM_SCAL
			m_clStdf.WriteByte((BYTE)0);												// HLM_SCAL
			m_clStdf.WriteFloat((FLOAT)m_pTestItems[m_nTestItemIndex].m_r4LIMITVALUE);	// LO_LIMIT
			m_clStdf.WriteFloat(0.0F);													// HI_LIMIT
		}
		m_clStdf.WriteString(m_pTestItems[m_nTestItemIndex].m_cnLIMITUNIT.toLatin1().constData());	// UNITS
	}

#ifdef QT_DEBUG
	// BG DEBUG
	if(m_pTestItems[m_nTestItemIndex].m_bHaslimit == true)
	{
		if(m_pTestItems[m_nTestItemIndex].m_bIsHighLimit)
		{
			if(m_pTestItems[m_nTestItemIndex].m_bIsStrictLimit == true)
			{
				if(pclSpektraTestData->m_r4TESTRESULTVALUE  >= m_pTestItems[m_nTestItemIndex].m_r4LIMITVALUE)
					nFailsPerTest_Limit[m_nTestItemIndex]++;
			}
			else
			{
				if(pclSpektraTestData->m_r4TESTRESULTVALUE  > m_pTestItems[m_nTestItemIndex].m_r4LIMITVALUE)
					nFailsPerTest_Limit[m_nTestItemIndex]++;
			}
		}
		else
		{
			if(m_pTestItems[m_nTestItemIndex].m_bIsStrictLimit == true)
			{
				if(pclSpektraTestData->m_r4TESTRESULTVALUE  <= m_pTestItems[m_nTestItemIndex].m_r4LIMITVALUE)
					nFailsPerTest_Limit[m_nTestItemIndex]++;
			}
			else
			{
				if(pclSpektraTestData->m_r4TESTRESULTVALUE  < m_pTestItems[m_nTestItemIndex].m_r4LIMITVALUE)
					nFailsPerTest_Limit[m_nTestItemIndex]++;
			}
		}
	}
#endif

	m_clStdf.WriteRecord();
}

/////////////////////////////////////////////////////////////////////////////
// Write PRR record...
/////////////////////////////////////////////////////////////////////////////
void CSPEKTRAtoSTDF::WritePRR(void)
{
    GS::StdLib::StdfRecordReadInfo	RecordReadInfo;

	m_bFirstDevice = false;

	// Write PRR
	RecordReadInfo.iRecordType = 5;
	RecordReadInfo.iRecordSubType = 20;
	m_clStdf.WriteHeader(&RecordReadInfo);
	m_clStdf.WriteByte((BYTE)1);												// HEAD_NUM
	m_clStdf.WriteByte((BYTE)1);												// SITE_NUM
	if(m_nFailTestsInDevice == 0)												// PART_FLG
	{
		m_clStdf.WriteByte((BYTE)0x00);
		m_nGoodParts++;
		m_nGoodPartsInWafer++;
		
		// Update binning pass/fail status
		m_pclBinnings[m_nCurrentBinning].m_cPassFail = 'P';
	}
	else
	{
		m_clStdf.WriteByte((BYTE)0x08);
		m_nFailParts++;

		// Update binning pass/fail status
		m_pclBinnings[m_nCurrentBinning].m_cPassFail = 'F';
	}
	m_clStdf.WriteWord((WORD)m_nTests);												// NUM_TEST
	m_clStdf.WriteWord((WORD)m_nCurrentBinning);									// HARD_BIN		(from BINNUMBER)
	m_clStdf.WriteWord((WORD)m_nCurrentBinning);									// SOFT_BIN		(from BINNUMBER)
	m_clStdf.WriteWord((WORD)-32768);												// X_COORD
	m_clStdf.WriteWord((WORD)-32768);												// Y_COORD
	m_clStdf.WriteDword((DWORD)0);													// TEST_T
	m_clStdf.WriteString(QString::number(m_nCurrentDeviceID).toLatin1().constData());	// PART_ID		(from SERIALNUMBER)
	m_clStdf.WriteRecord();
}

/////////////////////////////////////////////////////////////////////////////
// Write PIR record...
/////////////////////////////////////////////////////////////////////////////
void CSPEKTRAtoSTDF::WritePIR(CSpektra_DeviceID_V3* pclSpektraDeviceID)
{
    GS::StdLib::StdfRecordReadInfo	RecordReadInfo;

	// Update counters
	m_nCurrentDeviceID = pclSpektraDeviceID->m_u2SERIALNUMBER;
	m_nCurrentBinning = pclSpektraDeviceID->m_u2BINNUMBER;
	m_nFailTestsInDevice = 0;
	(m_pclBinnings[m_nCurrentBinning].m_nCount)++;

	// Write PIR
	RecordReadInfo.iRecordType = 5;
	RecordReadInfo.iRecordSubType = 10;
	m_clStdf.WriteHeader(&RecordReadInfo);
	m_clStdf.WriteByte((BYTE)1);												// HEAD_NUM
	m_clStdf.WriteByte((BYTE)1);												// SITE_NUM
	m_clStdf.WriteRecord();
}

/////////////////////////////////////////////////////////////////////////////
// Write WRR record...
/////////////////////////////////////////////////////////////////////////////
void CSPEKTRAtoSTDF::WriteWRR(void)
{
    GS::StdLib::StdfRecordReadInfo	RecordReadInfo;

	// Write WRR
	RecordReadInfo.iRecordType = 2;
	RecordReadInfo.iRecordSubType = 20;
	m_clStdf.WriteHeader(&RecordReadInfo);
	m_clStdf.WriteByte((BYTE)1);												// HEAD_NUM
	m_clStdf.WriteByte((BYTE)255);												// SITE_GRP
	m_clStdf.WriteDword((DWORD)m_uiDateTime);									// FINISH_T		(from DATETIME field of Label record)
	m_clStdf.WriteDword((DWORD)m_nPartsInWafer);								// PART_CNT
	m_clStdf.WriteDword((DWORD)0);												// RTST_CNT
	m_clStdf.WriteDword((DWORD)0);												// ABRT_CNT
	m_clStdf.WriteDword((DWORD)m_nGoodPartsInWafer);							// GOOD_CNT
	m_clStdf.WriteDword((DWORD)0);												// FUNC_CNT
	m_clStdf.WriteString(QString::number(m_nCurrentWaferID).toLatin1().constData());	// WAFER_ID		(from WAFERNUMBER)
	m_clStdf.WriteRecord();
}

/////////////////////////////////////////////////////////////////////////////
// Write WIR record...
/////////////////////////////////////////////////////////////////////////////
void CSPEKTRAtoSTDF::WriteWIR(CSpektra_WaferID_V3* pclSpektraWaferID)
{
    GS::StdLib::StdfRecordReadInfo	RecordReadInfo;

	m_nWaferCount++;
	m_nCurrentWaferID = pclSpektraWaferID->m_u1WAFERNUMBER;
	m_nPartsInWafer = 0;
	m_nGoodPartsInWafer = 0;

	// Write WIR
	RecordReadInfo.iRecordType = 2;
	RecordReadInfo.iRecordSubType = 10;
	m_clStdf.WriteHeader(&RecordReadInfo);
	m_clStdf.WriteByte((BYTE)1);												// HEAD_NUM
	m_clStdf.WriteByte((BYTE)255);												// SITE_GRP
	m_clStdf.WriteDword((DWORD)m_uiDateTime);									// START_T		(from DATETIME field of Label record)
	m_clStdf.WriteString(QString::number(m_nCurrentWaferID).toLatin1().constData());	// WAFER_ID		(from WAFERNUMBER)
	m_clStdf.WriteRecord();
}

/////////////////////////////////////////////////////////////////////////////
// Write MIR record...
/////////////////////////////////////////////////////////////////////////////
void CSPEKTRAtoSTDF::WriteMIR(CSpektra_Label_V3* pclSpektraLabel)
{
    GS::StdLib::StdfRecordReadInfo	RecordReadInfo;

	m_uiDateTime = pclSpektraLabel->m_dtDATETIME.toTime_t();

	if(m_uiDateTime <= 0)
		m_uiDateTime = QDateTime::currentDateTime().toTime_t();

	if(pclSpektraLabel->m_c1STATIONNAME < 'A')
		pclSpektraLabel->m_c1STATIONNAME = 'A';
	// Write MIR
	RecordReadInfo.iRecordType = 1;
	RecordReadInfo.iRecordSubType = 10;
	m_clStdf.WriteHeader(&RecordReadInfo);
	m_clStdf.WriteDword((DWORD)m_uiDateTime);									// SETUP_T		(from DATETIME field)
	m_clStdf.WriteDword((DWORD)m_uiDateTime);									// START_T		(from DATETIME field)
	m_clStdf.WriteByte((BYTE)(pclSpektraLabel->m_c1STATIONNAME - 'A' + 1));		// STAT_NUM		(from STATIONNAME: 'A'->1, 'B'->2...)
	m_clStdf.WriteByte((BYTE)pclSpektraLabel->m_b1TESTINGMODE);					// MODE_COD		(from TESTINGMODE)
	m_clStdf.WriteByte((BYTE) ' ');												// RTST_COD
	m_clStdf.WriteByte((BYTE) ' ');												// PROT_COD
	m_clStdf.WriteWord((WORD)65535);											// BURN_TIM
	m_clStdf.WriteByte((BYTE) ' ');												// CMOD_COD
	m_clStdf.WriteString(pclSpektraLabel->m_cnLOTNAME.toLatin1().constData());		// LOT_ID		(from LOTNAME)
	m_clStdf.WriteString(pclSpektraLabel->m_cnDEVICENAME.toLatin1().constData());	// PART_TYP		(from DEVICENAME)
	m_clStdf.WriteString("NODE");												// NODE_NAM
	m_clStdf.WriteString("TESEC");												// TSTR_TYP
	m_clStdf.WriteString(pclSpektraLabel->m_cnFILENAME.toLatin1().constData());		// JOB_NAM		(from FILENAME)
	m_clStdf.WriteString("");													// JOB_REV
	m_clStdf.WriteString("");													// SBLOT_ID
	m_clStdf.WriteString(pclSpektraLabel->m_cnOPERATOR.toLatin1().constData());		// OPER_NAM		(from OPERATOR)
	m_clStdf.WriteString("");													// EXEC_TYP
	m_clStdf.WriteString("");													// EXEC_VER
	m_clStdf.WriteString("");													// TEST_COD
	m_clStdf.WriteString("");													// TST_TEMP
	// Construct custom Galaxy USER_TXT 
	QString	strUserTxt;
	strUserTxt = GEX_IMPORT_DATAORIGIN_LABEL;	
	strUserTxt += ":";
	strUserTxt += GEX_IMPORT_DATAORIGIN_ATETEST;
	strUserTxt += ":SPEKTRA";
	m_clStdf.WriteString(strUserTxt.toLatin1().constData());	// user-txt
	m_clStdf.WriteRecord();
}

/////////////////////////////////////////////////////////////////////////////
// Write HBR records...
/////////////////////////////////////////////////////////////////////////////
void CSPEKTRAtoSTDF::WriteHBRs(void)
{
    GS::StdLib::StdfRecordReadInfo	RecordReadInfo;
	int					i;

	for(i=1 ; i<=250 ; i++)
	{
		if(m_pclBinnings[i].m_nCount != 0)
		{
			// Write HBR
			RecordReadInfo.iRecordType = 1;
			RecordReadInfo.iRecordSubType = 40;
			m_clStdf.WriteHeader(&RecordReadInfo);
			m_clStdf.WriteByte((BYTE)255);												// HEAD_NUM
			m_clStdf.WriteByte((BYTE)1);												// SITE_NUM
			m_clStdf.WriteWord((WORD)i);												// HBIN_NUM
			m_clStdf.WriteDword((DWORD)(m_pclBinnings[i].m_nCount));					// HBIN_CNT
			m_clStdf.WriteByte((BYTE)(m_pclBinnings[i].m_cPassFail));					// HBIN_PF
			m_clStdf.WriteRecord();
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// Write SBR records...
/////////////////////////////////////////////////////////////////////////////
void CSPEKTRAtoSTDF::WriteSBRs(void)
{
    GS::StdLib::StdfRecordReadInfo	RecordReadInfo;
	int					i;

	for(i=1 ; i<=250 ; i++)
	{
		if(m_pclBinnings[i].m_nCount != 0)
		{
			// Write SBR
			RecordReadInfo.iRecordType = 1;
			RecordReadInfo.iRecordSubType = 50;
			m_clStdf.WriteHeader(&RecordReadInfo);
			m_clStdf.WriteByte((BYTE)255);												// HEAD_NUM
			m_clStdf.WriteByte((BYTE)1);												// SITE_NUM
			m_clStdf.WriteWord((WORD)i);												// SBIN_NUM
			m_clStdf.WriteDword((DWORD)(m_pclBinnings[i].m_nCount));					// SBIN_CNT
			m_clStdf.WriteByte((BYTE)(m_pclBinnings[i].m_cPassFail));					// SBIN_PF
			m_clStdf.WriteRecord();
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// Write PCR record...
/////////////////////////////////////////////////////////////////////////////
void CSPEKTRAtoSTDF::WritePCR(void)
{
    GS::StdLib::StdfRecordReadInfo	RecordReadInfo;

	// Write PCR
	RecordReadInfo.iRecordType = 1; 
	RecordReadInfo.iRecordSubType = 30;
	m_clStdf.WriteHeader(&RecordReadInfo);
	m_clStdf.WriteByte((BYTE)255);					// HEAD_NUM (All)
	m_clStdf.WriteByte((BYTE)255);					// SITE_NUM (All)
	m_clStdf.WriteDword((DWORD)m_nParts);			// PART_CNT
	m_clStdf.WriteDword((DWORD)0);					// RTST_CNT
	m_clStdf.WriteDword((DWORD)0);					// ABRT_CNT
	m_clStdf.WriteDword((DWORD)m_nGoodParts);		// GOOD_CNT
	m_clStdf.WriteRecord();
}

/////////////////////////////////////////////////////////////////////////////
// Write MRR record...
/////////////////////////////////////////////////////////////////////////////
void CSPEKTRAtoSTDF::WriteMRR(void)
{
    GS::StdLib::StdfRecordReadInfo	RecordReadInfo;

	// Write MRR
	RecordReadInfo.iRecordType = 1; 
	RecordReadInfo.iRecordSubType = 20;
	m_clStdf.WriteHeader(&RecordReadInfo);
	m_clStdf.WriteDword((DWORD)m_uiDateTime);		// FINISH_T		(from DATETIME field of Label record)
	m_clStdf.WriteRecord();
}
