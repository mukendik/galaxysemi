//////////////////////////////////////////////////////////////////////
// import_sequoia.cpp: Convert a SEQUOIA .csv
// file to STDF V4.0
//////////////////////////////////////////////////////////////////////

#ifdef _WIN32
#include "windows.h"		// For 'GetWindowsDirectoryA' declaration
#endif

#include <qfileinfo.h>
#include <qregexp.h>
#include <qprogressbar.h>
#include <qapplication.h>
#include <qlabel.h>
#include <qdir.h>
#include <gqtl_sysutils.h>

#include "engine.h"
#include "import_sequoia.h"
#include "time.h"
#include "import_constants.h"
//
//# SEQUOIA DATA FILE HEADER #
//limit file:,limits as of 01302008,, [...] ,,,,,,,,,,,,,,,,, [...] ,,,,,,,,,,,,,,
//#C:\ate\Instrument_Drivers\RaptorC_ [...] ,,,,,,,,,,,,,,,,, [...] ,,,,,,,,,,,,,,
//#C:\ATE\Instrument_Drivers\RaptorC_ [...] h_fsk.csv,,,,,,,, [...] ,,,,,,,,,,,,,,
//#ATE108,,,,,,,,,,,,,,,,,,,,,,,,,,,, [...] ,,,,,,,,,,,,,,,,, [...] ,,,,,,,,,,,,,,
//TC,TC,TC,TC,TC,TC,TC,TC,TC,TC,TC,TC [...] ,TC,TC,TC,TC,TC,T [...] ,M,M,M,M,M,M,M,M, [...] ,M,H,,
//Comment,PS1 Output State,Ref Clock, [...] SetTemperature,Se [...] SetTXPower,SetRXP [...] ,DC Offset VGA2 Q,Time Stamp,Error Msg
//Bench Init,1,N/A,N/A,N/A,N/A,N/A,N/ [...] A,N/A,N/A,N/A,N/A [...] , , , , , , , , , [...] , ,Wed 23/Jan/2008 13:30:54,
//Standby,N/A,N/A,N/A,N/A,N/A,N/A,N/A [...] ,N/A,N/A,N/A,N/A, [...] ,-4.28E-06,7.13E- [...] , ,Wed 23/Jan/2008 13:30:56,
//TxG GSM 824.2,N/A,N/A,25,GSM,850,82 [...] 4.2,TX,20,-100,Co [...] ,8.193,-45.169,-5 [...] , ,Wed 23/Jan/2008 13:30:57,
//TxG GSM 836,N/A,N/A,25,GSM,850,836. [...] 6,TX,20,-100,Cont [...] ,-2.14E-06,0.0289 [...] , ,Wed 23/Jan/2008 13:30:58,
//TxG GSM 848.8,N/A,N/A,25,GSM,850,84 [...] 8.8,TX,20,-100,Co [...] ,0.3338293,1.1657 [...] , ,Wed 23/Jan/2008 13:30:58,

// main.cpp
extern QLabel			*GexScriptStatusLabel;	// Handle to script status text in status bar
extern QProgressBar	*	GexProgressBar;		// Handle to progress bar in status bar


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGSEQUOIAtoSTDF::CGSEQUOIAtoSTDF()
{
	// Default: SEQUOIA parameter list on disk includes all known SEQUOIA parameters...
	m_bNewSequoiaParameterFound = false;
	m_lStartTime = 0;

	m_tabTestParameter = NULL;
	m_tabTestCondition = NULL;
}

//////////////////////////////////////////////////////////////////////
// Destruction
//////////////////////////////////////////////////////////////////////
CGSEQUOIAtoSTDF::~CGSEQUOIAtoSTDF()
{
	// Destroy list of Parameters tables.
	if(m_tabTestParameter!=NULL)
		delete [] m_tabTestParameter;
	if(m_tabTestCondition!=NULL)
		delete [] m_tabTestCondition;
}

//////////////////////////////////////////////////////////////////////
// Get Error
//////////////////////////////////////////////////////////////////////
QString CGSEQUOIAtoSTDF::GetLastError()
{
	m_strLastError = "Import SEQUOIA: ";

	switch(m_iLastError)
	{
		default:
		case errNoError:
			m_strLastError += "No Error";
			break;
		case errOpenFail:
			m_strLastError += "Failed to open file";
			break;
		case errInvalidFormat:
			m_strLastError += "Invalid file format";
			break;
		case errNoLimitsFound:
			m_strLastError += "Invalid file format: Specification Limits not found";
			break;
		case errWriteSTDF:
			m_strLastError += "Failed creating temporary file. Folder permission issue?";
			break;	
		case errLicenceExpired:
			m_strLastError += "License has expired or Data file out of date...";
			break;	
		case errMultiCond:
			m_strLastError += "Invalid file format: This file contains more than one Global test condition";
			break;	
	}
	// Return Error Message
	return m_strLastError;
}

//////////////////////////////////////////////////////////////////////
// Load SEQUOIA Parameter table from DISK
//////////////////////////////////////////////////////////////////////
void CGSEQUOIAtoSTDF::LoadParameterIndexTable(void)
{
	QString	strSequoiaTableFile;
	QString	strString;

    strSequoiaTableFile  = GS::Gex::Engine::GetInstance().Get("UserFolder").toString();
	strSequoiaTableFile += GEX_SEQUOIA_PARAMETERS;

	// Open SEQUOIA Parameter table file
    QFile f( strSequoiaTableFile );
    if(!f.open( QIODevice::ReadOnly ))
		return;

	// Assign file I/O stream
	QTextStream hSequoiaTableFile(&f);

	// Skip comment lines
	do
	{
	  strString = hSequoiaTableFile.readLine();
	}
    while((strString.indexOf("----------------------") < 0) && (hSequoiaTableFile.atEnd() == false));

	// Read lines
	m_pFullSequoiaParametersList.clear();
	strString = hSequoiaTableFile.readLine();
	while (strString.isNull() == false)
	{
		// Save Parameter name in list
		m_pFullSequoiaParametersList.append(strString);
		// Read next line
		strString = hSequoiaTableFile.readLine();
	};

	// Close file
	f.close();
}

//////////////////////////////////////////////////////////////////////
// Save SEQUOIA Parameter table to DISK
//////////////////////////////////////////////////////////////////////
void CGSEQUOIAtoSTDF::DumpParameterIndexTable(void)
{
    QString		strSequoiaTableFile;

    strSequoiaTableFile  = GS::Gex::Engine::GetInstance().Get("UserFolder").toString();
	strSequoiaTableFile += GEX_SEQUOIA_PARAMETERS;

	// Open SEQUOIA Parameter table file
    QFile f( strSequoiaTableFile );
    if(!f.open( QIODevice::WriteOnly ))
		return;

	// Assign file I/O stream
	QTextStream hSequoiaTableFile(&f);

	// First few lines are comments:
	hSequoiaTableFile << "############################################################" << endl;
	hSequoiaTableFile << "# DO NOT EDIT THIS FILE!" << endl;
    hSequoiaTableFile << "# Quantix Examinator: SEQUOIA Parameters detected" << endl;
	hSequoiaTableFile << "# www.mentor.com" << endl;
    hSequoiaTableFile << "# Quantix Examinator reads and writes into this file..." << endl;
	hSequoiaTableFile << "-----------------------------------------------------------" << endl;

	// Write lines
	// m_pFullSequoiaParametersList.sort();
	for(int nIndex = 0; nIndex < m_pFullSequoiaParametersList.count(); nIndex++)
	{
		// Write line
		hSequoiaTableFile << m_pFullSequoiaParametersList[nIndex] << endl;
	};

	// Close file
	f.close();
}

//////////////////////////////////////////////////////////////////////
// If Examinator doesn't have this SEQUOIA parameter in his dictionnary, have it added.
//////////////////////////////////////////////////////////////////////
void CGSEQUOIAtoSTDF::UpdateParameterIndexTable(QString strParamName)
{
	// Check if the table is empty...if so, load it from disk first!
	if(m_pFullSequoiaParametersList.isEmpty() == true)
	{
		// Load SEQUOIA parameter table from disk...
		LoadParameterIndexTable();
	}
	
	// Check if Parameter name already in table...if not, add it to the list
	// the new full list will be dumped to the disk at the end.
    if(m_pFullSequoiaParametersList.indexOf(strParamName) < 0)
	{
		// Update list
		m_pFullSequoiaParametersList.append(strParamName);

		// Set flag to force the current SEQUOIA table to be updated on disk
		m_bNewSequoiaParameterFound = true;
	}
}


//////////////////////////////////////////////////////////////////////
// Check if File is compatible with SEQUOIA format
//////////////////////////////////////////////////////////////////////
bool CGSEQUOIAtoSTDF::IsCompatible(const char *szFileName)
{
	QString strString;
	QString strValue;
	QString	strSite;

	// Open hCsmFile file
    QFile f( szFileName );
    if(!f.open( QIODevice::ReadOnly ))
	{
		// Failed Opening ASL1000 file
		return false;
	}
	// Assign file I/O stream
	QTextStream hSequoiaFile(&f);

	// Check if first line is the correct SEQUOIA header...
	//# SEQUOIA DATA FILE HEADER #
	int nNbLine = 0;

	do
	{
		strString = hSequoiaFile.readLine();
		if(strString.startsWith("TC,", Qt::CaseInsensitive))
			break;
		nNbLine++;
		if(nNbLine>20)
			break;
		strString = "";
	}
	while(!strString.isNull() && strString.isEmpty());

	f.close();

	strString = strString.trimmed();	// remove leading spaces.

	if(strString.startsWith("TC,", Qt::CaseInsensitive) == false)
	{
		// Incorrect header...this is not a SEQUOIA file!
		return false;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////
// Read and Parse the SEQUOIA file
//////////////////////////////////////////////////////////////////////
bool CGSEQUOIAtoSTDF::ReadSequoiaFile(const char *SequoiaFileName)
{
	QString strString;
	QString strSection;
	bool	bStatus;
	int		iIndex;				// Loop index

	// Open CSV file
    QFile f( SequoiaFileName );
    if(!f.open( QIODevice::ReadOnly ))
	{
		// Failed Opening SEQUOIA file
		m_iLastError = errOpenFail;
		
		// Convertion failed.
		return false;
	}

 	//////////////////////////////////////////////////////////////////////
	// For ProgressBar
	iNextFilePos = 0;
	iProgressStep = 0;
	iFileSize = f.size() + 1;
	

	// Assign file I/O stream
	QTextStream hSequoiaFile(&f);
	// Check if first line is the correct SEQUOIA header...
	//# SEQUOIA DATA FILE HEADER #
	strString = ReadLine(hSequoiaFile);
	strString = strString.trimmed();	// remove leading spaces.

	int nNbLine = 0;
	QString strDate;
	while(!strString.startsWith("TC,"))
	{
		// extract program information
		strString = ReadLine(hSequoiaFile);
		nNbLine++;
		if(nNbLine>20)
			break;

		// Limit File Name
		if(strString.startsWith("limit file:", Qt::CaseInsensitive))
		{
			m_strLimitFileName = strString.section(",",1,1).simplified();
            if(!m_strLimitFileName.endsWith(".csv", Qt::CaseInsensitive))
				m_strLimitFileName += ".csv";
			
			CGexSystemUtils	clSysUtils;
			clSysUtils.NormalizePath(m_strLimitFileName);
			if(QFileInfo(m_strLimitFileName).isRelative())
			{
                m_strLimitFileName = QFileInfo(SequoiaFileName).absolutePath() +"/" + m_strLimitFileName;
				clSysUtils.NormalizePath(m_strLimitFileName);
			}
		}
		else
		if(strString.startsWith("date:", Qt::CaseInsensitive))
		{
			strDate =  strString.section(",",1,1).simplified();
		}
		else
		if(strString.startsWith("#Wed", Qt::CaseInsensitive))
		{
			strDate =  strString.section(",",0,0).remove("#");
		}
	}
	
	if(strString.startsWith("TC,", Qt::CaseInsensitive) == false)
	{
		// Incorrect header...this is not a SEQUOIA file!
		m_iLastError = errInvalidFormat;
		
		// Convertion failed.
		// Close file
		f.close();
		return false;
	}
	
	// Date format: 2007/03/11

	QDate qDate;
	QTime qTime;
	strDate = strDate.replace('/',' ').replace('_',' ').replace('-',' ').replace('T',' ');
	//Web 08 Jan 2006 format to Wed Jan 08 2006
	if(strDate.section(" ",2,2).toInt() == 0)
	{
		qTime = QTime::fromString(strDate.section(" ",4));
		strDate = strDate.section(" ",0,0)+" "+strDate.section(" ",2,2)+" "+strDate.section(" ",1,1)+" "+strDate.section(" ",3,3);
		qDate = QDate::fromString(strDate,Qt::TextDate);
	}
	//Check if have some time
	else
	if(strDate.contains(':'))
	{
		qTime = QTime::fromString(strDate.mid(strDate.indexOf(':')-2));
		strDate = strDate.left(strDate.indexOf(':')-3);
		qDate = QDate::fromString(strDate,Qt::ISODate);
	}
	else
	{
		qDate = QDate::fromString(strDate,Qt::ISODate);
	}

	if(qDate.isValid())
	{
		QDateTime clDateTime(qDate,qTime);
		clDateTime.setTimeSpec(Qt::UTC);
		m_lStartTime = clDateTime.toTime_t();
	}
	
	//TC,TC,TC,TC,TC,TC,TC,TC,TC,TC,TC,TC [...] ,TC,TC,TC,TC,TC,T [...] ,M,M,M,M,M,M,M,M, [...] ,M,H,,
	// In m_strSectionHeader the first line with TC and M symbols
	m_strSectionHeader = strString.toUpper();
	//Comment,PS1 Output State,Ref Clock, [...] SetTemperature,Se [...] SetTXPower,SetRXP [...] ,DC Offset VGA2 Q,Time Stamp,Error Msg
	// In strString the second line with name for each test conditions and parameters
	strString = ReadLine(hSequoiaFile);

	
	// Construct the List of Tests Conditions and Tests Parameters
	// Count the number of TC and the number of M
	m_iTotalTestCondition = m_strSectionHeader.count("TC")+1;
	m_iTotalTestParameters = m_strSectionHeader.count("M")+1;
	m_tabTestCondition = new CGSequoiaTestCondition[m_iTotalTestCondition];
	m_tabTestParameter = new CGSequoiaTestParameter[m_iTotalTestParameters];

	// Collect information about TestCondition and TestParameter
	
	int iCurrentTestCondition = 0;
	int iCurrentTestParameter = 0;
    int iPosSection = m_strSectionHeader.indexOf(",")+1;
	int iNextPosSection = 0;
    int iPosString = strString.indexOf(",")+1;
	int iNextPosString = 0;
	while(iNextPosSection > -1)
	{
        iNextPosSection = m_strSectionHeader.indexOf(",",iPosSection);
        iNextPosString = strString.indexOf(",",iPosString);
		
		if(m_strSectionHeader.mid(iPosSection).startsWith("TC", Qt::CaseInsensitive))
		{
			// Add new test condition
			m_tabTestCondition[iCurrentTestCondition++].strName = strString.mid(iPosString,iNextPosString-iPosString);
		}
		if(m_strSectionHeader.mid(iPosSection).startsWith("M", Qt::CaseInsensitive))
		{
			// Add new test parameter
			UpdateParameterIndexTable(strString.mid(iPosString,iNextPosString-iPosString));
			m_tabTestParameter[iCurrentTestParameter++].strName = strString.mid(iPosString,iNextPosString-iPosString);
		}
		
		iPosSection = iNextPosSection+1;
		iPosString = iNextPosString+1;
	}

	// If have File for limit
	// Open CSV file
    QFile fLimit(m_strLimitFileName);
    if(fLimit.open( QIODevice::ReadOnly ))
	{
		// Assign file I/O stream
		QTextStream hFile(&fLimit);

		strString = "";
		
		while(!hFile.atEnd() && !strString.startsWith("TC"))
			strString = ReadLine(hFile);
		
		QString strNames;
		QString strValues;
		QString strValue, strGlobalCond;
		int iPosNames, iNextPosNames = 0;
		int iPosValues, iNextPosValues = 0;

		strSection = strString;
		strNames = ReadLine(hFile);

		while(!hFile.atEnd())
		{
			strValues = ReadLine(hFile);
			
			iCurrentTestCondition = 0;
			iCurrentTestParameter = 0;
            iPosSection = iNextPosSection = strSection.indexOf(",")+1;
            iPosNames = iNextPosNames = strNames.indexOf(",")+1;
            iPosValues = iNextPosValues = strValues.indexOf(",")+1;

			strGlobalCond = strValues.left(iPosValues-1);

			while(iPosSection > 0)
			{
				
                iNextPosSection = strSection.indexOf(",",iPosSection);
                iNextPosNames = strNames.indexOf(",",iPosNames);
                iNextPosValues = strValues.indexOf(",",iPosValues);
				
				if(strSection.mid(iPosSection).startsWith("TC", Qt::CaseInsensitive))
				{
				}
				if(strSection.mid(iPosSection).startsWith("M", Qt::CaseInsensitive))
				{
					// Find the Test parameter and update limits
					iIndex = iCurrentTestParameter;						
					if(iIndex >= m_iTotalTestParameters)
						break;

					while(m_tabTestParameter[iIndex].strName != strNames.mid(iPosNames,iNextPosNames-iPosNames))
					{
						iIndex++;
						if(iIndex >= m_iTotalTestParameters)
							break;
					}
					
					if(iIndex >= m_iTotalTestParameters)
					{
						// Parameter not found
					}
					else
					{
						// Update limits
						iCurrentTestParameter = iIndex;

						strValue = strValues.mid(iPosValues,iNextPosValues-iPosValues);
						QMap<QString,CGSequoiaTestLimit *>::Iterator it;
						CGSequoiaTestLimit *pLimits;

						if(strValue.isEmpty() || !(strValue.contains("|")) || (strValue.toUpper().remove("/") == "NA") || (strValue.toUpper().remove("/").remove(" ") == "NA|NA"))
						{

							// N/A
							if(m_tabTestParameter[iCurrentTestParameter].mapGTestCondTestLimits.contains(strGlobalCond))
							{
								// Have to remove this limits
                                it = m_tabTestParameter[iCurrentTestParameter].mapGTestCondTestLimits.find(strGlobalCond);
                                delete it.value();
                                m_tabTestParameter[iCurrentTestParameter].mapGTestCondTestLimits.erase(it);// .remove(it);
							}
						}
						else
						{
							// Update limits
							if(m_tabTestParameter[iCurrentTestParameter].mapGTestCondTestLimits.contains(strGlobalCond))
								pLimits = m_tabTestParameter[iCurrentTestParameter].mapGTestCondTestLimits[strGlobalCond];
							else
							{
								pLimits = new CGSequoiaTestLimit;
								m_tabTestParameter[iCurrentTestParameter].mapGTestCondTestLimits[strGlobalCond] = pLimits;
							}
							
							pLimits->fLowLimit = strValue.section("|",0,0).simplified().toFloat(&pLimits->bValidLowLimit);
							pLimits->fHighLimit = strValue.section("|",1,1).simplified().toFloat(&pLimits->bValidHighLimit);
						}
						iCurrentTestParameter++;
					}
				}
				
				iPosSection = iNextPosSection + 1;
				iPosNames = iNextPosNames + 1;
				iPosValues = iNextPosValues + 1;
			}
		}
			

		fLimit.close();
	}



	// Loop reading file until end is reached & generate STDF file dynamically.
	bStatus = WriteStdfFile(&hSequoiaFile);
	if(!bStatus)
		QFile::remove(m_strStdfFileName);

	// All SEQUOIA file read...check if need to update the SEQUOIA Parameter list on disk?
	if(bStatus && (m_bNewSequoiaParameterFound == true))
		DumpParameterIndexTable();

	// Close file
	f.close();
	
	// Success parsing SEQUOIA file
	return bStatus;
}

//////////////////////////////////////////////////////////////////////
// Create STDF file from SEQUOIA data parsed
//////////////////////////////////////////////////////////////////////
bool CGSEQUOIAtoSTDF::WriteStdfFile(QTextStream *hSequoiaFile)
{
	// now generate the STDF file...
    GS::StdLib::StdfRecordReadInfo RecordReadInfo;

	// Write Test results for each line read.
	QString		strString;
	int			iIndex;				// Loop index
	BYTE		bData;
	WORD		wSoftBin,wHardBin;
	long		iTestNumber,iTotalTests,iPartNumber;
	bool		bPassStatus;
	bool		bOnePart = false;

	bool		bHaveLowLimit, bHaveHighLimit = false;
	float		fLowLimit, fHighLimit = 0.0;

	QString		strValue, strDate;
	int			iCurrentTestCondition = 0;
	int			iCurrentTestParameter = 0;
	int			iPosSection, iNextPosSection = 0;
	int			iPosString, iNextPosString = 0;
	int			nPass = 1;				// Index of the current GlobalTestCondition
	QString		strGlobalCond;
	QString		strGlobalTestCondition;
	
	// Reset counters
	m_iTotalGoodBin=m_iTotalFailBin=0;
	iPartNumber=0;

	// Write all Parameters read on this wafer.: WIR.PIR,PTR....PTR, PRR, PIR, PTR,PTR...PRR,   ... WRR
	while(hSequoiaFile->atEnd() == false)
	{


		// Read line
		strString = ReadLine(*hSequoiaFile);

		// Restart at the begining if other test cond
		if(bOnePart || hSequoiaFile->atEnd())
		{
			m_lstGlobalTestCondition.pop_front();
			if(m_lstGlobalTestCondition.isEmpty())
				break;
			
			WriteEndGlobalTestCondition();

			if(nPass == 1)
			{
				// Check if have more than One part
				// Then have to parse all the file for each GlobalTestCondition
				// Else can parse and create WIR/WRR for each lines
				bOnePart = (iPartNumber == 1);
			}
			
			if(!bOnePart || (nPass == 1))
			{
				hSequoiaFile->device()->close();
				hSequoiaFile->device()->open(QIODevice::ReadOnly);
				
				iProgressStep = 100/iFileSize + 1;
				iNextFilePos  = iFileSize/100 + 1;

				while(m_lstGlobalTestCondition.first() != strString.section(",",0,m_iTotalTestCondition))
					strString = ReadLine(*hSequoiaFile);

			}
			
			nPass = 2;
			iPartNumber = 0;
			m_iTotalGoodBin = m_iTotalFailBin = 0;
			m_strTestCondId = "";
		}

		// Ignore Bench Line
		if(strString.startsWith("Bench", Qt::CaseInsensitive))
			continue;


        strGlobalCond = strString.left(strString.indexOf(","));
		strGlobalTestCondition = strString.section(",",0,m_iTotalTestCondition);
		
		if(nPass == 1)
		{
			// First run
			// have to save all GlobalTestCondition
			if(m_lstGlobalTestCondition.contains(strGlobalTestCondition))
				nPass = 2;
			else
				m_lstGlobalTestCondition.append(strGlobalTestCondition);
		}
		if((strGlobalTestCondition != m_lstGlobalTestCondition.first())
		&& !m_bSplitMultiCond)
		{
			m_iLastError = errMultiCond;
			return false;
		}

		if(strGlobalTestCondition != m_lstGlobalTestCondition.first())
			continue;


		// For each line, construct the TestConditionName for WaferId
		if(m_strTestCondId.isEmpty())
		{
			m_strTestCondId = strGlobalCond;
			iCurrentTestCondition = 0;
			iCurrentTestParameter = 0;
            iPosSection = iNextPosSection = m_strSectionHeader.indexOf(",")+1;
            iPosString = iNextPosString = strString.indexOf(",")+1;
		}
		else
		{
			// Goto directly to measurment
			iCurrentTestCondition = 0;
			iCurrentTestParameter = 0;
            iPosSection = iNextPosSection = m_strSectionHeader.indexOf(",",m_strSectionHeader.section(",",0,m_iTotalTestCondition).length())+1;
            iPosString = iNextPosString = strString.indexOf(",",strGlobalTestCondition.length())+1;
			
		}
		
		while(iPosSection > 0)
		{
            iNextPosSection = m_strSectionHeader.indexOf(",",iPosSection);
            iNextPosString = strString.indexOf(",",iPosString);
			strValue = m_strSectionHeader.mid(iPosSection);
			strValue = strString.mid(iPosString,iNextPosString-iPosString).simplified();
			
			if(m_strSectionHeader.mid(iPosSection).startsWith("TC", Qt::CaseInsensitive))
			{
				// Update test condition
				m_tabTestCondition[iCurrentTestCondition].bValidValue = !((strValue.isEmpty() || strValue.toUpper().remove("/") == "NA"));

				if(m_tabTestCondition[iCurrentTestCondition].bValidValue)
				{
					m_tabTestCondition[iCurrentTestCondition].strValue = strValue;

					// Global test condition
					m_strTestCondId += ",";
					m_strTestCondId += m_tabTestCondition[iCurrentTestCondition].strName;
					m_strTestCondId += "=";
					m_strTestCondId += m_tabTestCondition[iCurrentTestCondition].strValue;
				}

				iCurrentTestCondition++;

			}
			else if(m_strSectionHeader.mid(iPosSection).startsWith("M", Qt::CaseInsensitive))
			{
				// Add new test parameter
				m_tabTestParameter[iCurrentTestParameter].bValidValue = !((strValue.isEmpty() || strValue.toUpper().remove("/") == "NA"));

				if(m_tabTestParameter[iCurrentTestParameter].bValidValue)
				{
					m_tabTestParameter[iCurrentTestParameter].fValue = strValue.toFloat(&m_tabTestParameter[iCurrentTestParameter].bValidValue);
				}
				
				iCurrentTestParameter++;
			}
			else if(m_strSectionHeader.mid(iPosSection).startsWith("H", Qt::CaseInsensitive))
				strDate = strValue;
			
			iPosSection = iNextPosSection + 1;
			iPosString = iNextPosString + 1;
		}
		
		


		// Part number
		iPartNumber++;
		// Date format: 2007/03/11
		QDate qDate;
		QTime qTime;
		strDate = strDate.replace('/',' ').replace('_',' ').replace('-',' ').replace('T',' ');
		//Web 08 Jan 2006 format to Wed Jan 08 2006
		if(strDate.section(" ",2,2).toInt() == 0)
		{
			qTime = QTime::fromString(strDate.section(" ",4));
			strDate = strDate.section(" ",0,0)+" "+strDate.section(" ",2,2)+" "+strDate.section(" ",1,1)+" "+strDate.section(" ",3,3);
			qDate = QDate::fromString(strDate,Qt::TextDate);
		}
		//Check if have some time
		else
		if(strDate.contains(':'))
		{
			qTime = QTime::fromString(strDate.mid(strDate.indexOf(':')-2));
			strDate = strDate.left(strDate.indexOf(':')-3);
			qDate = QDate::fromString(strDate,Qt::ISODate);
		}
		else
		{
			qDate = QDate::fromString(strDate,Qt::ISODate);
		}

		if(qDate.isValid())
		{
			QDateTime clDateTime(qDate,qTime);
			clDateTime.setTimeSpec(Qt::UTC);
			m_lStartTime = clDateTime.toTime_t();
		}

		
		if(iPartNumber == 1)
		{
			// For this test condition

			WriteBeginGlobalTestCondition();


			m_iTotalGoodBin=m_iTotalFailBin=0;
			// For each wafer, have to write limit in the first PTR
			for(iIndex=0;iIndex<m_iTotalTestParameters;iIndex++)
				m_tabTestParameter[iIndex].bStaticHeaderWritten = false;

			// For each wafer, have to write all test condition
			for(iIndex=0;iIndex<m_iTotalTestCondition;iIndex++)
			{
				if(!m_tabTestCondition[iIndex].bValidValue)
					continue;

				// Write DTR
				// or Write FTR for debug
				// Write FTR
				RecordReadInfo.iRecordType = 15;
				RecordReadInfo.iRecordSubType = 20;

				m_StdfFile.WriteHeader(&RecordReadInfo);
				m_StdfFile.WriteDword(iIndex+m_iTotalTestParameters);// Test Number
				m_StdfFile.WriteByte(1);							// Test head
				m_StdfFile.WriteByte(1);							// Tester site:1,2,3,4 or 5, etc.
				m_StdfFile.WriteByte(0);		// passed			// TEST_FLG

				// save empty field for report_readfile.cpp
				m_StdfFile.WriteByte(255);	// opt_flg
				m_StdfFile.WriteDword(0);		// cycl_cnt
				m_StdfFile.WriteDword(0);		// rel_vadr
				m_StdfFile.WriteDword(0);		// rept_cnt
				m_StdfFile.WriteDword(0);		// num_fail
				m_StdfFile.WriteDword(0);		// xfail_ad
				m_StdfFile.WriteDword(0);		// yfail_ad
				m_StdfFile.WriteWord(0);		// vect_off
				m_StdfFile.WriteWord(0);		// rtn_icnt
				m_StdfFile.WriteWord(0);
				m_StdfFile.WriteWord(0);
				m_StdfFile.WriteString(m_tabTestCondition[iIndex].strValue.toLatin1().constData());	// vect_name
				m_StdfFile.WriteString("");	// time_set
				m_StdfFile.WriteString("");	// op_code
				m_StdfFile.WriteString(m_tabTestCondition[iIndex].strName.toLatin1().constData());	// test_txt: test name
				m_StdfFile.WriteString("");	// alarm_id
				m_StdfFile.WriteString("");	// prog_txt
				m_StdfFile.WriteString(m_tabTestCondition[iIndex].strValue.toLatin1().constData());	// rslt_txt

				m_StdfFile.WriteRecord();
			}
		}

		// Reset Pass/Fail flag.
		bPassStatus = true;

		// Reset counters
		iTotalTests = 0;

		// Write PIR for parts in this Wafer site
		RecordReadInfo.iRecordType = 5;
		RecordReadInfo.iRecordSubType = 10;
		m_StdfFile.WriteHeader(&RecordReadInfo);
		m_StdfFile.WriteByte(1);								// Test head
		m_StdfFile.WriteByte(1);					// Tester site
		m_StdfFile.WriteRecord();

		// Write Parameter results for this record
		for(iIndex=0;iIndex<m_iTotalTestParameters;iIndex++)
		{
			if(m_tabTestParameter[iIndex].bValidValue == true)
			{
				// Valid test result...write the PTR
				iTotalTests++;

				if(m_tabTestParameter[iIndex].mapGTestCondTestLimits.contains(strGlobalCond))
				{
					bHaveLowLimit = m_tabTestParameter[iIndex].mapGTestCondTestLimits[strGlobalCond]->bValidLowLimit;
					if(bHaveLowLimit)
						fLowLimit = m_tabTestParameter[iIndex].mapGTestCondTestLimits[strGlobalCond]->fLowLimit;
					else
						fLowLimit = 0.0;

					bHaveHighLimit = m_tabTestParameter[iIndex].mapGTestCondTestLimits[strGlobalCond]->bValidHighLimit;
					if(bHaveHighLimit)
						fHighLimit = m_tabTestParameter[iIndex].mapGTestCondTestLimits[strGlobalCond]->fHighLimit;
					else
						fHighLimit = 0.0;
				}
				else
				{
					bHaveLowLimit = bHaveHighLimit = false;
					fLowLimit = fHighLimit = 0.0;
				}

				RecordReadInfo.iRecordType = 15;
				RecordReadInfo.iRecordSubType = 10;
				m_StdfFile.WriteHeader(&RecordReadInfo);
				// Compute Test# (add user-defined offset)
                iTestNumber = (long) m_pFullSequoiaParametersList.indexOf(m_tabTestParameter[iIndex].strName);
				iTestNumber += GEX_TESTNBR_OFFSET_SEQUOIA;	// Test# offset
				m_StdfFile.WriteDword(iTestNumber);			// Test Number
				m_StdfFile.WriteByte(1);						// Test head
				m_StdfFile.WriteByte(1);			// Tester site#
				if((bHaveLowLimit  && (m_tabTestParameter[iIndex].fValue < fLowLimit)) 
				|| (bHaveHighLimit && (m_tabTestParameter[iIndex].fValue > fHighLimit)))
				{
					bData = 0200;	// Test Failed
					bPassStatus = false;
				}
				else
				{
					bData = 0;		// Test passed
				}
				m_StdfFile.WriteByte(bData);							// TEST_FLG
				m_StdfFile.WriteByte(0x40|0x80);						// PARAM_FLG
				m_StdfFile.WriteFloat(m_tabTestParameter[iIndex].fValue);						// Test result
				if(m_tabTestParameter[iIndex].bStaticHeaderWritten == false)
				{
					m_StdfFile.WriteString(m_tabTestParameter[iIndex].strName.toLatin1().constData());	// TEST_TXT
					m_StdfFile.WriteString("");							// ALARM_ID
					bData = 2;	// Valid data.
					if(bHaveLowLimit==false)
						bData |=0x40;
					if(bHaveHighLimit==false)
						bData |=0x80;
					m_StdfFile.WriteByte(bData);							// OPT_FLAG
					m_StdfFile.WriteByte(0);								// RES_SCALE
					m_StdfFile.WriteByte(0);								// LLM_SCALE
					m_StdfFile.WriteByte(0);								// HLM_SCALE
					m_StdfFile.WriteFloat(fLowLimit);						// LOW Limit
					m_StdfFile.WriteFloat(fHighLimit);					// HIGH Limit
					m_StdfFile.WriteString("");							// No Units
					m_tabTestParameter[iIndex].bStaticHeaderWritten = true;
				}
				m_StdfFile.WriteRecord();
			}	// Valid test result
		}		// Read all results on line

		// Write PRR
		RecordReadInfo.iRecordType = 5;
		RecordReadInfo.iRecordSubType = 20;
		m_StdfFile.WriteHeader(&RecordReadInfo);
		m_StdfFile.WriteByte(1);			// Test head
		m_StdfFile.WriteByte(1);// Tester site#:1
		if(bPassStatus == true)
		{
			m_StdfFile.WriteByte(0);				// PART_FLG : PASSED
			wSoftBin = wHardBin = 1;
			m_iTotalGoodBin++;
		}
		else
		{
			m_StdfFile.WriteByte(8);				// PART_FLG : FAILED
			wSoftBin = wHardBin = 0;
			m_iTotalFailBin++;
		}
		m_StdfFile.WriteWord((WORD)iTotalTests);	// NUM_TEST
		m_StdfFile.WriteWord(wHardBin);				// HARD_BIN
		m_StdfFile.WriteWord(wSoftBin);				// SOFT_BIN
		m_StdfFile.WriteWord((WORD)-32768);			// X_COORD
		m_StdfFile.WriteWord((WORD)-32768);			// Y_COORD
		m_StdfFile.WriteDword(0);					// No testing time known...
		m_StdfFile.WriteString(QString::number(iPartNumber).toLatin1().constData());		// PART_ID
		m_StdfFile.WriteString("");			// PART_TXT
		m_StdfFile.WriteString("");			// PART_FIX
		m_StdfFile.WriteRecord();
	};
	

	WriteEndGlobalTestCondition();

	// Success
	return true;
}

//////////////////////////////////////////////////////////////////////
// MODE DataBase
//		Write DTR_BEGIN_GLOBAL_TEST_CONDITION
// MODE Examinator
//		Start new STDF file for each GLOBAL_TEST_CONDITION
//////////////////////////////////////////////////////////////////////
bool CGSEQUOIAtoSTDF::WriteBeginGlobalTestCondition()
{

    GS::StdLib::StdfRecordReadInfo RecordReadInfo;
#if 0
	// Open the StdfFile only at the begining
	if(m_StdfFile.GetPos() < 1)
	{
		if(m_StdfFile.Open((char*)m_strStdfFileName,STDF_WRITE) != GS::StdLib::Stdf::NoError)
		{
			// Failed importing CSV file into STDF database
			m_iLastError = errWriteSTDF;
			
			// Convertion failed.
			return false;
		}

		// Write FAR
		RecordReadInfo.iRecordType = 0;
		RecordReadInfo.iRecordSubType = 10;
		m_StdfFile.WriteHeader(&RecordReadInfo);
		m_StdfFile.WriteByte(1);					// SUN CPU type
		m_StdfFile.WriteByte(4);					// STDF V4
		m_StdfFile.WriteRecord();

		if(m_lStartTime <= 0)
			m_lStartTime = QDateTime::currentDateTime().toTime_t();

		// Write MIR
		RecordReadInfo.iRecordType = 1;
		RecordReadInfo.iRecordSubType = 10;
		m_StdfFile.WriteHeader(&RecordReadInfo);
		m_StdfFile.WriteDword(m_lStartTime);			// Setup time
		m_StdfFile.WriteDword(m_lStartTime);			// Start time
		m_StdfFile.WriteByte(1);						// Station
		m_StdfFile.WriteByte((BYTE) 'P');				// Test Mode = PRODUCTION
		m_StdfFile.WriteByte((BYTE) ' ');				// rtst_cod
		m_StdfFile.WriteByte((BYTE) ' ');				// prot_cod
		m_StdfFile.WriteWord(65535);					// burn_tim
		m_StdfFile.WriteByte((BYTE) ' ');				// cmod_cod
		m_StdfFile.WriteString(m_strLotID.toLatin1().constData());		// Lot ID
		m_StdfFile.WriteString(m_strProductID.toLatin1().constData());	// Part Type / Product ID
		m_StdfFile.WriteString("");					// Node name
		m_StdfFile.WriteString("");					// Tester Type
		m_StdfFile.WriteString(m_strProgramID.toLatin1().constData());	// Job name
		m_StdfFile.WriteString("");					// Job rev
		m_StdfFile.WriteString("");					// sublot-id
		m_StdfFile.WriteString("");					// operator
		m_StdfFile.WriteString("");					// exec-type
		m_StdfFile.WriteString("");					// exe-ver
		m_StdfFile.WriteString("");					// test-cod
		m_StdfFile.WriteString("");					// test-temperature
		// Construct custom Galaxy USER_TXT 
		QString	strUserTxt;
		strUserTxt = GEX_IMPORT_DATAORIGIN_LABEL;	
		strUserTxt += ":";
		strUserTxt += GEX_IMPORT_DATAORIGIN_ATETEST;
		strUserTxt += ":SEQUOIA";
		m_StdfFile.WriteString((char *)strUserTxt.toLatin1().constData());	// user-txt
		m_StdfFile.WriteString("");							// aux-file
		m_StdfFile.WriteString("");							// package-type
		m_StdfFile.WriteString(m_strProductID.toLatin1().constData());	// familyID
		m_StdfFile.WriteString("");							// Date-code
		m_StdfFile.WriteString("");							// Facility-ID
		m_StdfFile.WriteString("");							// FloorID
		m_StdfFile.WriteString("");	// ProcessID	m_StdfFile.WriteRecord();
		m_StdfFile.WriteRecord();
	}
	
	// Write WIR of new Wafer.
			RecordReadInfo.iRecordType = 2;
			RecordReadInfo.iRecordSubType = 10;
			m_StdfFile.WriteHeader(&RecordReadInfo);
			m_StdfFile.WriteByte(1);								// Test head
			m_StdfFile.WriteByte(255);							// Tester site (all)
			m_StdfFile.WriteDword(m_lStartTime);					// Start time
			m_StdfFile.WriteString(m_strTestCondId.toLatin1().constData());	// WaferID
			m_StdfFile.WriteRecord();
#else
	// Format new Stdf File

	if(!m_bHaveMultiCond && m_lstGlobalTestCondition.count() > 1)
		m_bHaveMultiCond = true;
	if(m_bHaveMultiCond && !m_bSplitMultiCond)
	{
		m_iLastError = errMultiCond;
		return false;
	}

	m_StdfFile.Close();

	// Rename current STDF file
	QDir clDir;
	QFileInfo clFile(m_strStdfFileName);
    m_strLastStdfFileName = clFile.absolutePath()+"/GlobalCond_"+QString::number(m_lstGlobalTestCondition.count())+"_"+clFile.fileName();
	clDir.rename(m_strStdfFileName,m_strLastStdfFileName);

	// Open new Stdf file
    if(m_StdfFile.Open(m_strStdfFileName.toLatin1().constData(),STDF_WRITE) != GS::StdLib::Stdf::NoError)
	{
		// Failed importing CSV file into STDF database
		m_iLastError = errWriteSTDF;
		
		// Convertion failed.
		return false;
	}

	// Write FAR
	RecordReadInfo.iRecordType = 0;
	RecordReadInfo.iRecordSubType = 10;
	m_StdfFile.WriteHeader(&RecordReadInfo);
	m_StdfFile.WriteByte(1);					// SUN CPU type
	m_StdfFile.WriteByte(4);					// STDF V4
	m_StdfFile.WriteRecord();

	if(m_lStartTime <= 0)
		m_lStartTime = QDateTime::currentDateTime().toTime_t();

	// Write MIR
	RecordReadInfo.iRecordType = 1;
	RecordReadInfo.iRecordSubType = 10;
	m_StdfFile.WriteHeader(&RecordReadInfo);
	m_StdfFile.WriteDword(m_lStartTime);			// Setup time
	m_StdfFile.WriteDword(m_lStartTime);			// Start time
	m_StdfFile.WriteByte(1);						// Station
	m_StdfFile.WriteByte((BYTE) 'P');				// Test Mode = PRODUCTION
	m_StdfFile.WriteByte((BYTE) ' ');				// rtst_cod
	m_StdfFile.WriteByte((BYTE) ' ');				// prot_cod
	m_StdfFile.WriteWord(65535);					// burn_tim
	m_StdfFile.WriteByte((BYTE) ' ');				// cmod_cod
	m_StdfFile.WriteString(m_strLotID.toLatin1().constData());		// Lot ID
	m_StdfFile.WriteString(m_strProductID.toLatin1().constData());	// Part Type / Product ID
	m_StdfFile.WriteString("");					// Node name
	m_StdfFile.WriteString("");					// Tester Type
	m_StdfFile.WriteString(m_strProgramID.toLatin1().constData());	// Job name
	m_StdfFile.WriteString("");					// Job rev
	m_StdfFile.WriteString("");					// sublot-id
	m_StdfFile.WriteString("");					// operator
	m_StdfFile.WriteString("");					// exec-type
	m_StdfFile.WriteString("");					// exe-ver
	m_StdfFile.WriteString(m_strTestCondId.toLatin1().constData());	// test-cod
	m_StdfFile.WriteString("");					// test-temperature
	// Construct custom Galaxy USER_TXT 
	QString	strUserTxt;
	strUserTxt = GEX_IMPORT_DATAORIGIN_LABEL;	
	strUserTxt += ":";
	strUserTxt += GEX_IMPORT_DATAORIGIN_ATETEST;
	strUserTxt += ":SEQUOIA";
	m_StdfFile.WriteString((char *)strUserTxt.toLatin1().constData());	// user-txt
	m_StdfFile.WriteString("");							// aux-file
	m_StdfFile.WriteString("");							// package-type
	m_StdfFile.WriteString(m_strProductID.toLatin1().constData());	// familyID
	m_StdfFile.WriteString("");							// Date-code
	m_StdfFile.WriteString("");							// Facility-ID
	m_StdfFile.WriteString("");							// FloorID
	m_StdfFile.WriteString("");	// ProcessID	m_StdfFile.WriteRecord();
	m_StdfFile.WriteRecord();

#endif

	return true;
}

//////////////////////////////////////////////////////////////////////
// MODE DataBase
//		Write DTR_END_GLOBAL_TEST_CONDITION
// MODE Examinator
//		Stop the current STDF file for each GLOBAL_TEST_CONDITION
//////////////////////////////////////////////////////////////////////
bool CGSEQUOIAtoSTDF::WriteEndGlobalTestCondition()
{
    GS::StdLib::StdfRecordReadInfo RecordReadInfo;

#if 0
	// Write WRR in case we have finished to write wafer records.
	// WRR
	RecordReadInfo.iRecordType = 2;
	RecordReadInfo.iRecordSubType = 20;
	m_StdfFile.WriteHeader(&RecordReadInfo);
	m_StdfFile.WriteByte(1);						// Test head
	m_StdfFile.WriteByte(255);					// Tester site (all)
	m_StdfFile.WriteDword(m_lStartTime);			// Time of last part tested
	m_StdfFile.WriteDword(m_iTotalGoodBin+m_iTotalFailBin);	// Parts tested: always 5
	m_StdfFile.WriteDword(0);						// Parts retested
	m_StdfFile.WriteDword(0);						// Parts Aborted
	m_StdfFile.WriteDword(m_iTotalGoodBin);			// Good Parts
	m_StdfFile.WriteDword((DWORD)-1);				// Functionnal Parts
	m_StdfFile.WriteString(m_strTestCondId.toLatin1().constData());	// WaferID
	m_StdfFile.WriteRecord();

	
	// Write MRR only for the last test cond
	if(m_lstGlobalTestCondition.isEmpty())
	{
		RecordReadInfo.iRecordType = 1; 
		RecordReadInfo.iRecordSubType = 20;
		m_StdfFile.WriteHeader(&RecordReadInfo);
		m_StdfFile.WriteDword(m_lStartTime);			// File finish-time.
		m_StdfFile.WriteRecord();

		// Close STDF file.
		m_StdfFile.Close();
	}
			
#else

	// Write MRR
	RecordReadInfo.iRecordType = 1; 
	RecordReadInfo.iRecordSubType = 20;
	m_StdfFile.WriteHeader(&RecordReadInfo);
	m_StdfFile.WriteDword(m_lStartTime);			// File finish-time.
	m_StdfFile.WriteRecord();

	// Close STDF file.
	m_StdfFile.Close();

	if(m_bHaveMultiCond)
	{
		// Rename current STDF file
		QDir clDir;
		QFileInfo clFile(m_strStdfFileName);
        m_strLastStdfFileName = clFile.absolutePath()+"/GlobalCond_"+QString::number(m_lstGlobalTestCondition.count())+"_"+clFile.fileName();
		clDir.rename(m_strStdfFileName,m_strLastStdfFileName);
	}
#endif
			
	return true;
}

//////////////////////////////////////////////////////////////////////
// Convert 'FileName' SEQUOIA file, to STDF 'strFileNameSTDF' file
//////////////////////////////////////////////////////////////////////
bool CGSEQUOIAtoSTDF::Convert(const char *SequoiaFileName, QString &strFileNameSTDF)
{
	// No erro (default)
	m_iLastError = errNoError;

	// If STDF file already exists...do not rebuild it...unless dates not matching!
	QFileInfo fInput(SequoiaFileName);
	QFileInfo fOutput(strFileNameSTDF);

    QFile f( strFileNameSTDF );
    if((f.exists() == true) && (fInput.lastModified() < fOutput.lastModified()))
		return true;

	m_strStdfFileName = strFileNameSTDF;
	if(m_strStdfFileName.endsWith(GEX_TEMPORARY_STDF))
		m_bSplitMultiCond = false;
	else
		m_bSplitMultiCond = true;
	m_bHaveMultiCond = false;
	
	//////////////////////////////////////////////////////////////////////
	// For ProgressBar
	bool bHideProgressAfter=true;
	bool bHideLabelAfter=false;
	if(GexProgressBar != NULL)
	{
		bHideProgressAfter = GexProgressBar->isHidden();
		GexProgressBar->setMaximum(100);
		GexProgressBar->setTextVisible(true);
		GexProgressBar->setValue(0);
		GexProgressBar->show();
	}

	if(GexScriptStatusLabel != NULL)
	{
		if(GexScriptStatusLabel->isHidden())
		{
			bHideLabelAfter = true;
			GS::Gex::Engine::GetInstance().UpdateLabelStatus("Converting data from file "+QFileInfo(SequoiaFileName).fileName()+"...");
			GexScriptStatusLabel->show();
		}
		GS::Gex::Engine::GetInstance().UpdateLabelStatus("Converting data from file "+QFileInfo(SequoiaFileName).fileName()+"...");
		GexScriptStatusLabel->show();
	}
    QCoreApplication::processEvents();

    if(ReadSequoiaFile(SequoiaFileName) != true)
	{
		//////////////////////////////////////////////////////////////////////
		// For ProgressBar
		if((GexProgressBar != NULL)
		&& bHideProgressAfter)
			GexProgressBar->hide();
		
		if((GexScriptStatusLabel != NULL)
		&& bHideLabelAfter)
			GexScriptStatusLabel->hide();
		return false;	// Error reading SEQUOIA file
	}
	
	//////////////////////////////////////////////////////////////////////
	// For ProgressBar
	if((GexProgressBar != NULL)
	&& bHideProgressAfter)
		GexProgressBar->hide();
	
	if((GexScriptStatusLabel != NULL)
	&& bHideLabelAfter)
		GexScriptStatusLabel->hide();
	
	if(m_bHaveMultiCond)
		strFileNameSTDF = m_strLastStdfFileName;
	
	// Convertion successful
	return true;
}


//////////////////////////////////////////////////////////////////////
// Read line : skip empty line
//////////////////////////////////////////////////////////////////////
QString CGSEQUOIAtoSTDF::ReadLine(QTextStream& hFile)
{
	QString strString;
	
	//////////////////////////////////////////////////////////////////////
	// For ProgressBar
	if(GexProgressBar != NULL)
	{
        while((int) hFile.device()->pos() > iNextFilePos)
		{
			iProgressStep += 100/iFileSize + 1;
			iNextFilePos  += iFileSize/100 + 1;
			GexProgressBar->setValue(iProgressStep);
		}
	}
    QCoreApplication::processEvents();
			
	do
		strString = hFile.readLine();
	while(!strString.isNull() && strString.isEmpty());

	return strString;

}
