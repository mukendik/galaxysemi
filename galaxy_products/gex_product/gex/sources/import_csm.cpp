//////////////////////////////////////////////////////////////////////
// import_csm.cpp: Convert a .CSM (TSMC) file to STDF V4.0
//////////////////////////////////////////////////////////////////////

#include "gqtl_global.h"
#include <qmath.h>
#include <time.h>

#ifdef _WIN32
#include "windows.h"		// For 'GetWindowsDirectoryA' declaration
#endif

#include <qfileinfo.h>
#include <qprogressbar.h>
#include <qapplication.h>
#include <qlabel.h>
#include <QTextStream>
#include <QListIterator>

#include "engine.h"
#include "import_csm.h"
#include "import_constants.h"

// File format:
//**************************************** CHARTERED SEMICONDUCTOR MANUFACTURING ****************************************
//********************************** ELECTRICAL TEST BINNING SUMMARY REPORT ***************************************
//
//<LOT ID>  2WSB49241.1
//<DEVICE>  WM8750SLV
//<PROCESS> 2A323-697-0TA
//<TEST PROGRAM> YI-083-ET025A      DC/MJC#29/A62-11/csm4062j
//<DATE TESTED>  08 Jan 2006
//<TIME TESTED>  13:11:18
//<TOTAL # OF WAFER/S TESTED>  25
//<# OF TEST SITES / WAFER>    5
//<# OF BINNING PARAMETERS>   47
//
//<--------+---------+---------+---------+---------+---------+---------+------->
//
//Waf si TNE2 20/20  TNE2 20/.35 TNE2 20/.35 TNE2 20/.35 TNE2 20/.35 TESD 20/0.5 
//       Vth0 (V)    Vth0 (V)    Idsat (mA)  Isubst (uA) BVdss (V)   Vth0 (V)    
//...............................................................................
//
//  1  5  6.159e-01   6.188e-01   1.045e+01   1.166e-05   8.800e+00   6.549e-01  
//  1  3  6.118e-01   6.075e-01   1.061e+01   1.642e-05   8.800e+00   6.439e-01  
//  1  9  6.146e-01   6.195e-01   1.046e+01   7.960e-06   8.900e+00   6.461e-01  
//  1  7  6.094e-01   6.101e-01   1.072e+01   5.400e-06   8.800e+00   6.426e-01  
//  1  1  6.215e-01   6.269e-01   1.021e+01   5.340e-06   8.800e+00   6.523e-01  
//-------------------------------------------------------------------------------
//Wf avg  6.146e-01   6.166e-01   1.049e+01   9.356e-06   8.820e+00   6.480e-01  
//-------------------------------------------------------------------------------
//
//  2  5  6.136e-01   6.339e-01   1.044e+01   5.080e-06   8.800e+00   6.530e-01  
//...
//===============================================================================
//Lot Statistics
//# site      125         125         125         125         125         125         
//Sc Rej        0           0           0           0           0           0         
//Max     6.351e-01   6.407e-01   1.092e+01   3.748e-05   8.900e+00   6.725e-01  
//Avg     6.119e-01   6.184e-01   1.053e+01   1.219e-05   8.823e+00   6.470e-01  
//Min     5.957e-01   5.944e-01   9.970e+00   4.280e-06   8.700e+00   6.258e-01  
//St dev  6.192e-03   9.696e-03   1.655e-01   5.088e-06   4.775e-02   7.793e-03  
//===============================================================================
//
//...
//<>-----<>-----<>-----<>-----<>-----<>-----<>-----<>-----<>-----<>-----<>-----<>-----<>-----<>-----<>-----<>-----<>-----<>
//Bin Parameter\Waf #       1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19  20  21  22  23  24  25 
//<>-----<>-----<>-----<>-----<>-----<>-----<>-----<>-----<>-----<>-----<>-----<>-----<>-----<>-----<>-----<>-----<>-----<>
//TNE2 20/20 Vth0 (V)       0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0 
//...
//*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*
//Bin Parameter\Waf #       1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19  20  21  22  23  24  25 
//PASS / FAIL STATUS        P   P   P   P   P   P   P   P   P   P   P   P   P   P   P   P   P   P   P   P   P   P   P   P   P 
//*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*
//
//
//...
//-------------------------------------------------------------------------------------------------------------------------
//BIN        NAME                  LOW          HIGH         N   SR      MIN          MEAN         MAX          S.D.
//-------------------------------------------------------------------------------------------------------------------------
//
//  1  TNE2 20/20 Vth0 (V)       5.300e-01    7.300e-01    125    0    5.957e-01    6.119e-01    6.351e-01    6.192e-03
//


// main.cpp
extern QLabel			*GexScriptStatusLabel;	// Handle to script status text in status bar
extern QProgressBar	*	GexProgressBar;		// Handle to progress bar in status bar


//////////////////////////////////////////////////////////////////////
// Construction
//////////////////////////////////////////////////////////////////////
CGCSMtoSTDF::CGCSMtoSTDF()
{
	// Default: CSM parameter list on disk includes all known CSM parameters...
	m_bNewCsmParameterFound = false;
	m_lStartTime = 0;

}

//////////////////////////////////////////////////////////////////////
// Destruction
//////////////////////////////////////////////////////////////////////
CGCSMtoSTDF::~CGCSMtoSTDF()
{
	while (!m_pWaferList.isEmpty())
		delete m_pWaferList.takeFirst();
}

//////////////////////////////////////////////////////////////////////
// Get Error
//////////////////////////////////////////////////////////////////////
QString CGCSMtoSTDF::GetLastError()
{
	m_strLastError = "Import CSM: ";

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
		case errWriteSTDF:
			m_strLastError += "Failed creating temporary file. Folder permission issue?";
			break;	
		case errLicenceExpired:
			m_strLastError += "License has expired or Data file out of date...";
			break;	
	}
	// Return Error Message
	return m_strLastError;
}

//////////////////////////////////////////////////////////////////////
// Load CSM Parameter table from DISK
//////////////////////////////////////////////////////////////////////
void CGCSMtoSTDF::LoadParameterIndexTable(void)
{
	QString	strCsmTableFile;
	QString	strString;

    strCsmTableFile  = GS::Gex::Engine::GetInstance().Get("UserFolder").toString();
	strCsmTableFile += GEX_CSM_PARAMETERS;

	// Open CSM Parameter table file
    QFile f( strCsmTableFile );
    if(!f.open( QIODevice::ReadOnly ))
		return;

	// Assign file I/O stream
	QTextStream hCsmTableFile(&f);

	// Skip comment lines
	do
	{
	  strString = hCsmTableFile.readLine();
	}
    while((strString.indexOf("----------------------") < 0) && (hCsmTableFile.atEnd() == false));

	// Read lines
	m_pFullCsmParametersList.clear();
	strString = hCsmTableFile.readLine();
	while (strString.isNull() == false)
	{
		// Save Parameter name in list
		m_pFullCsmParametersList.append(strString);
		// Read next line
		strString = hCsmTableFile.readLine();
	};

	// Close file
	f.close();
}

//////////////////////////////////////////////////////////////////////
// Save CSM Parameter table to DISK
//////////////////////////////////////////////////////////////////////
void CGCSMtoSTDF::DumpParameterIndexTable(void)
{
	QString		strCsmTableFile;
	QString		strString;

    strCsmTableFile  = GS::Gex::Engine::GetInstance().Get("UserFolder").toString();
	strCsmTableFile += GEX_CSM_PARAMETERS;

	// Open CSM Parameter table file
    QFile f( strCsmTableFile );
    if(!f.open( QIODevice::WriteOnly ))
		return;

	// Assign file I/O stream
	QTextStream hCsmTableFile(&f);

	// First few lines are comments:
	hCsmTableFile << "############################################################" << endl;
	hCsmTableFile << "# DO NOT EDIT THIS FILE!" << endl;
    hCsmTableFile << "# Quantix Examinator: CSM Parameters detected" << endl;
	hCsmTableFile << "# www.mentor.com" << endl;
    hCsmTableFile << "# Quantix Examinator reads and writes into this file..." << endl;
	hCsmTableFile << "-----------------------------------------------------------" << endl;

	// Write lines
	// m_pFullCsmParametersList.sort();
	for (QStringList::const_iterator
		 iter  = m_pFullCsmParametersList.begin();
		 iter != m_pFullCsmParametersList.end(); ++iter) {
		// Write line
		hCsmTableFile << *iter << endl;
	}

	// Close file
	f.close();
}

//////////////////////////////////////////////////////////////////////
// If Examinator doesn't have this CSM parameter in his dictionnary, have it added.
//////////////////////////////////////////////////////////////////////
void CGCSMtoSTDF::UpdateParameterIndexTable(QString strParamName)
{
	// Check if the table is empty...if so, load it from disk first!
	if(m_pFullCsmParametersList.isEmpty() == true)
	{
		// Load CSM parameter table from disk...
		LoadParameterIndexTable();
	}
	
	// Check if Parameter name already in table...if not, add it to the list
	// the new full list will be dumped to the disk at the end.
    if(m_pFullCsmParametersList.indexOf(strParamName) < 0)
	{
		// Update list
		m_pFullCsmParametersList.append(strParamName);

		// Set flag to force the current CSM table to be updated on disk
		m_bNewCsmParameterFound = true;
	}
}

//////////////////////////////////////////////////////////////////////
// Destructor
//////////////////////////////////////////////////////////////////////
CGCsmWafer::~CGCsmWafer()
{
	while(!m_pParameterList.isEmpty())
		delete m_pParameterList.takeFirst();
}

//////////////////////////////////////////////////////////////////////
// Find or create Parameter entry, return handle to it.
//////////////////////////////////////////////////////////////////////
CGCsmParameter *CGCSMtoSTDF::FindParameterEntry(int iWaferID,int iSiteID,QString strParamName,QString strUnits)
{
	CGCsmWafer	*ptWafers;		// List of Wafers in CSM file
	CGCsmParameter *ptParam;	// List of parameters

	// Save each valid sites for X-Y wafer positions
	if((iSiteID > 0)
	&& (!m_lstSites.contains(QString::number(iSiteID))))
		m_lstSites.append(QString::number(iSiteID));
	
	if(m_pWaferList.count() > 0)
	{
		ptWafers = m_pWaferList.first();
		QListIterator<CGCsmWafer*> lstIteratorWafer(m_pWaferList);	

		lstIteratorWafer.toFront();
		while(lstIteratorWafer.hasNext())
		{
			ptWafers = lstIteratorWafer.next();

			if(ptWafers->m_iWaferID == iWaferID)
			{
				// Update Min/Max SiteID
				if(iSiteID > 0)
				{
					ptWafers->m_iLowestSiteID=gex_min(ptWafers->m_iLowestSiteID,iSiteID);				// Lowest WaferID found in CSM file
					ptWafers->m_iHighestSiteID=gex_max(ptWafers->m_iHighestSiteID,iSiteID);				// Highest WaferID found in CSM file
				}

				// Found WaferID entry...now find Parameter entry
				QList<CGCsmParameter *>::iterator itBegin	= ptWafers->m_pParameterList.begin();
				QList<CGCsmParameter *>::iterator itEnd		= ptWafers->m_pParameterList.end();
				
				while (itBegin != itEnd)
				{
					if((*itBegin)->m_strName == strParamName)
						return (*itBegin);	// Found the Parameter!

					itBegin++;
				}
				
				// Parameter not in list...create entry!
				// Create & add new Parameter entry in Wafer list
				ptParam = new CGCsmParameter;
				ptParam->m_strName = strParamName;
				ptParam->m_strUnits = strUnits;
				NormalizeLimits(ptParam);
				ptParam->m_bValidLowLimit = false;		// Low limit defined
				ptParam->m_bValidHighLimit = false;	// High limit defined

				ptParam->m_bStaticHeaderWritten = false;
				ptWafers->m_pParameterList.append(ptParam);
				iTotalParameters++;
				// If Examinator doesn't have this CSM parameter in his dictionnary, have it added.
				UpdateParameterIndexTable(strParamName);
				
				// Return pointer to the Parameter 
				return ptParam;
			}
		};
	}


	// WaferID entry doesn't exist yet...so create it now!
	ptWafers = new CGCsmWafer;
	ptWafers->m_iLowestSiteID=ptWafers->m_iHighestSiteID=0;
	ptWafers->m_iWaferID = iWaferID;

	// Update Min/Max SiteID
	if(iSiteID > 0)
	{
		ptWafers->m_iLowestSiteID=iSiteID;				// Lowest WaferID found in CSM file
		ptWafers->m_iHighestSiteID=iSiteID;				// Highest WaferID found in CSM file
	}

	// Create & add new Parameter entry in Wafer list
	ptParam = new CGCsmParameter;
	ptParam->m_strName = strParamName;
	ptParam->m_strUnits = strUnits;
	ptParam->m_nScale = 0;
	ptParam->m_bValidLowLimit = false;		// Low limit defined
	ptParam->m_bValidHighLimit = false;	// High limit defined
	ptParam->m_bStaticHeaderWritten = false;
	ptWafers->m_pParameterList.append(ptParam);
	iTotalParameters++;
	
	// If Examinator doesn't have this CSM parameter in his dictionnary, have it added.
	UpdateParameterIndexTable(strParamName);
	
	// Add Wafer entry to existing list.
	m_pWaferList.append(ptWafers);

	// Return pointer to the Parameter 
	return ptParam;
}

//////////////////////////////////////////////////////////////////////
// Save CSM parameter result...
//////////////////////////////////////////////////////////////////////
void CGCSMtoSTDF::SaveParameterResult(int iWaferID,int iSiteID,QString strName,QString strUnits,QString strValue)
{
	if(strName.isNull() == true)
		return;	// Ignore empty entry!

	// Find Pointer to the Parameter cell
	CGCsmParameter *ptParam = FindParameterEntry(iWaferID,iSiteID,strName,strUnits);

	// Save the Test value in it
	bool bIsInt;
	strValue = strValue.trimmed();
	strValue.right(1).toInt(&bIsInt);
	if(!bIsInt)
	{
		strValue = strValue.left(strValue.length()-1);	// Remove any 'x' character if found (sometimes present to flag entry out of specs)
		strValue = strValue.trimmed();
	}
	ptParam->m_fValue[iSiteID] = strValue.toFloat() * GS_POW(10.0,ptParam->m_nScale);
}

//////////////////////////////////////////////////////////////////////
// Save CSM parameter High or Low Limit result...
//////////////////////////////////////////////////////////////////////
void CGCSMtoSTDF::SaveParameterLimit(QString strName,QString strValue,int iLimit)
{
	if(strName.isNull() == true)
		return;	// Ignore empty entry!

	CGCsmParameter *	ptParam;	// List of parameters
	
	QList<CGCsmWafer*>::iterator itBegin	= m_pWaferList.begin();
	QList<CGCsmWafer*>::iterator itEnd		= m_pWaferList.end();

	while(itBegin != itEnd)
	{
		// Get Parameter pointer...save limits in ALL WaferEntry.
		ptParam = FindParameterEntry((*itBegin)->m_iWaferID, 0, strName);
		switch(iLimit)
		{
			case eHighLimit:
				ptParam->m_fHighLimit = strValue.toFloat() * GS_POW(10.0,ptParam->m_nScale);
				ptParam->m_bValidHighLimit = true;		// High limit defined

				break;
			case eLowLimit:
				ptParam->m_fLowLimit = strValue.toFloat()  * GS_POW(10.0,ptParam->m_nScale);
				ptParam->m_bValidLowLimit = true;		// Low limit defined
				break;
		}

		itBegin++;
	};
}

//////////////////////////////////////////////////////////////////////
// Normalize test limits when writing into PTR.
//////////////////////////////////////////////////////////////////////
void CGCSMtoSTDF::NormalizeLimits(CGCsmParameter *pParameter)
{
	int	value_scale=0;
	if(pParameter->m_strUnits.length() <= 1)
	{
		// units too short to include a prefix, then keep it 'as-is'
		pParameter->m_nScale = 0;
		return;
	}

	QChar cPrefix = pParameter->m_strUnits[0];
	switch(cPrefix.toLatin1())
	{
		case 'm': // Milli
			value_scale = -3;
			break;
		case 'u': // Micro
			value_scale = -6;
			break;
		case 'n': // Nano
			value_scale = -9;
			break;
		case 'p': // Pico
			value_scale = -12;
			break;
		case 'f': // Fento
			value_scale = -15;
			break;
		case 'K': // Kilo
			value_scale = 3;
			break;
		case 'M': // Mega
			value_scale = 6;
			break;
		case 'G': // Giga
			value_scale = 9;
			break;
		case 'T': // Tera
			value_scale = 12;
			break;
	}
	pParameter->m_fLowLimit *= GS_POW(10.0,value_scale);
	pParameter->m_fHighLimit *= GS_POW(10.0,value_scale);
	pParameter->m_nScale = value_scale;
	if(value_scale)
		pParameter->m_strUnits = pParameter->m_strUnits.mid(1);	// Take all characters after the prefix.
}

//////////////////////////////////////////////////////////////////////
// Check if File is compatible with CSM format
//////////////////////////////////////////////////////////////////////
bool CGCSMtoSTDF::IsCompatible(const char *szFileName)
{
	QString strString;
	QString strSection;
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
	QTextStream hCsmFile(&f);

	// Check if first line is the correct CSM header...
	// **************************************** CHARTERED SEMICONDUCTOR MANUFACTURING ****************************************
	// **************************************** ELECTRICAL TEST BINNING SUMMARY REPORT ***************************************

	// Read first non-empty line
	do
		strString = hCsmFile.readLine();
	while(!strString.isNull() && strString.isEmpty());

	// BG Case 3235: the first line may change, so check only the second line
#if 0
	strString = strString.remove("*").trimmed();	// remove leading spaces.
	if(!strString.startsWith("CHARTERED SEMICONDUCTOR MANUFACTURING", Qt::CaseInsensitive))
	{
		// Incorrect header...this is not a CSM file!
		// Close file
		// Ignore the first line
		//f.close();
		//return false;
	}
#endif

	// Read second non-empty line
	do
		strString = hCsmFile.readLine();
	while(!strString.isNull() && strString.isEmpty());

	strString = strString.remove("*").trimmed();	// remove leading spaces.
	if(!strString.startsWith("ELECTRICAL TEST BINNING SUMMARY REPORT", Qt::CaseInsensitive))
	{
		// Incorrect header...this is not a CSM file!
		// Close file
		f.close();
		return false;
	}

	f.close();

	return true;
}

//////////////////////////////////////////////////////////////////////
// Read and Parse the CSM file
//////////////////////////////////////////////////////////////////////
bool CGCSMtoSTDF::ReadCsmFile(const char *CsmFileName)
{
	QString strString;
	QString strSection;
	QString	strSite;
	QString	strParameters[6];	// Holds the 6 Parameters name (CSM file is organized by sets of 6 parameters columns)
	QString	strUnits[6];		// Holds the 6 Parameters Units 
	int		iWaferID;			// WaferID processed
	int		iSiteID;			// SiteID processed
	int		iIndex;				// Loop index
	int		iPos;				// String pos

	// Open CSM file
    QFile f( CsmFileName );
    if(!f.open( QIODevice::ReadOnly ))
	{
		// Failed Opening CSM file
		m_iLastError = errOpenFail;
		
		// Convertion failed.
		return false;
	}

	//////////////////////////////////////////////////////////////////////
	// For ProgressBar
	iFileSize = f.size() + 1;

	// Assign file I/O stream
	QTextStream hCsmFile(&f);

	// Check if first line is the correct CSM header...
	// **************************************** CHARTERED SEMICONDUCTOR MANUFACTURING ****************************************
	// **************************************** ELECTRICAL TEST BINNING SUMMARY REPORT ***************************************

	// Read first non-empty line
	strString = ReadLine(hCsmFile);
	if(strString.isNull())
	{
		// Close file
		f.close();
		return false;
	}
	
	// BG Case 3235: the first line may change, so check only the second line
#if 0
	strString = strString.remove("*").trimmed();	// remove leading spaces.
	if(!strString.startsWith("CHARTERED SEMICONDUCTOR MANUFACTURING", Qt::CaseInsensitive))
	{
		// Ignore the first line
		// Incorrect header...this is not a CSM file!
		//m_iLastError = errInvalidFormat;
		
		// Convertion failed.
		// Close file
		//f.close();
		//return false;
	}
#endif

	// Read second non-empty line
	strString = ReadLine(hCsmFile);
	if(strString.isNull())
	{
		// Close file
		f.close();
		return false;
	}

	strString = strString.remove("*").trimmed();	// remove leading spaces.
	if(!strString.startsWith("ELECTRICAL TEST BINNING SUMMARY REPORT", Qt::CaseInsensitive))
	{
		// Incorrect header...this is not a CSM file!
		m_iLastError = errInvalidFormat;
		
		// Convertion failed.
		// Close file
		f.close();
		return false;
	}

	strString = ReadLine(hCsmFile).trimmed();
	// Read CSM information
	QString strDate;
	QString	strTime;
	while(strString.isEmpty() || strString.left(1)=="<")
	{
		if(strString.isEmpty())
		{
			strString = ReadLine(hCsmFile).trimmed();
			continue;
		}
        iPos = strString.indexOf(">");
		if(iPos < 1)
		{
			strString = ReadLine(hCsmFile).trimmed();
			continue;
		}

		strSection = strString.mid(1,iPos-1).toUpper().trimmed();
		strString = strString.mid(iPos+1).trimmed();
		if(strSection.startsWith("LOT ID", Qt::CaseInsensitive))
			m_strLotID = strString;
		else
		if(strSection.startsWith("DEVICE", Qt::CaseInsensitive))
			m_strProductID = strString;
		else
		if(strSection.startsWith("PROCESS", Qt::CaseInsensitive))
			m_strProcessID = strString;
		else
		if(strSection.startsWith("TEST PROGRAM", Qt::CaseInsensitive))
			m_strJobName = strString;
		else
		if(strSection.startsWith("DATE TESTED", Qt::CaseInsensitive)) //08 Jan 2006 format to Wed Jan 08 2006
			strDate = "Wed "+strString.section(" ",1,1)+" "+strString.section(" ",0,0)+" "+strString.section(" ",2,2);
		else
		if(strSection.startsWith("TIME TESTED", Qt::CaseInsensitive))
			strTime = strString;
		strString = ReadLine(hCsmFile).trimmed();
	}
	
	if(!strDate.isEmpty())
	{
		QDateTime clDateTime;
		clDateTime.setTimeSpec(Qt::UTC);
		clDateTime.setDate(QDate::fromString(strDate));
		if(!strTime.isEmpty())
		{
			clDateTime.setTime(QTime::fromString(strTime));
		}
		m_lStartTime = clDateTime.toTime_t();
	}

	// Read line with list of Parameters names

	// Loop reading file until end is reached
	do
	{	
		if(!strString.startsWith("Waf si", Qt::CaseInsensitive))
		{
			strString = ReadLine(hCsmFile);
			continue;
		}
		// Waf si TNE2 20/20  TNE2 20/.35 TNE2 20/.35 TNE2 20/.35 TNE2 20/.35 TESD 20/0.5 
		//        Vth0 (V)    Vth0 (V)    Idsat (mA)  Isubst (uA) BVdss (V)   Vth0 (V)    
		// Extract the 6 column names (or less if not all are filled)
		for(iIndex=0;iIndex<6;iIndex++)
		{
			strSection = strString.mid(7+iIndex*12,12).trimmed();	// Remove spaces
			strParameters[iIndex] = strSection;
		}
		
		// Extract the 10 column Parameters second half of the name (usually contains units)
		strString = ReadLine(hCsmFile);
		for(iIndex=0;iIndex<6;iIndex++)
		{
			strSection = strString.mid(7+iIndex*12,12).trimmed();	// Remove spaces
			if(!strSection.isEmpty())
				strParameters[iIndex] += " " + strSection;
			/* Ignore all units
			if(strParameters[iIndex].endsWith(")") && (strParameters[iIndex].lastIndexOf("(")>0))
			{
				// have unit to save
				iPos = strParameters[iIndex].lastIndexOf("(");
				strUnits[iIndex] = strParameters[iIndex].mid(iPos+1,strParameters[iIndex].length()-iPos-2).trimmed();
				if(strUnits[iIndex].find("/")>0)
				{
					// not a unit
					// undo
					strUnits[iIndex] = "";
				}
				else
				{
					// delete unit from name
					//strParameters[iIndex] = strParameters[iIndex].left(strParameters[iIndex].length()-iPos-1).trimmed();
				}
			}
			else
			*/
			strUnits[iIndex]="";
		}

		strString = ReadLine(hCsmFile);
		// skip the line just after
		//.............................................................
		strString = ReadLine(hCsmFile);
		while(!strString.startsWith("===========================================") && !hCsmFile.atEnd())
		{
			if(strString.isEmpty())
			{
				strString = ReadLine(hCsmFile);
				continue;
			}
			// skip the average section
			// -------------------------------------------------------------------------------
			// Wf avg  8.395e+00   9.500e+00   9.150e+00   9.050e+00   3.502e+00   6.653e+02  
			// -------------------------------------------------------------------------------
			if(strString.startsWith("-------------------------------------------"))
			{
				strString = ReadLine(hCsmFile);
				continue;
			}
			if(strString.startsWith("Wf avg"))
			{
				strString = ReadLine(hCsmFile);
				continue;
			}
			
			// Read line of Parameter data unless end of data reached (reaching Param. Mean, Sigma and limits)
			// eg: " 2  5  8.418e+00   9.500e+00   9.050e+00   9.050e+00   3.644e+00   6.681e+02"
			// or may be end of data, in which case the line is full of '------'
	
			// Extract WaferID, and save it to the parameter list as a parameter itself!
			strSection = strString.mid(0,3);
			iWaferID = strSection.toInt();

			// Extract SiteID
			strSite = strString.mid(3,3);
			iSiteID = strSite.toInt();

			// For each column, extract parameter value and save it
			for(iIndex=0;iIndex<6;iIndex++)
			{
				strSection = strString.mid(7+iIndex*12,12);
					// Save parameter result in buffer
				if(!strSection.isEmpty())
					SaveParameterResult(iWaferID,iSiteID,strParameters[iIndex],strUnits[iIndex],strSection);
			}


			// Read next line in file...
			strString = ReadLine(hCsmFile);
		};

		// Check if last line processed (should normally never happen as statistcs should follow...)
		if(hCsmFile.atEnd())
			break;

		// Have reached the statistics section:
		// ===============================================================================
		// Lot Statistics
		// # site      125         125         125         125         125         125         
		// Sc Rej        0           0           0           0           0           0         
		// Max     8.687e+00   9.600e+00   9.300e+00   9.300e+00   4.133e+00   6.720e+02  
		// Avg     8.431e+00   9.502e+00   9.090e+00   9.054e+00   3.551e+00   6.658e+02  
		// Min     7.944e+00   9.500e+00   9.050e+00   9.050e+00   3.289e+00   6.596e+02  
		// St dev  1.336e-01   1.260e-02   9.202e-02   3.150e-02   1.533e-01   2.813e+00  
		// ===============================================================================

		// Skip all this section
		strString = ReadLine(hCsmFile);
		while(!strString.startsWith("===========================================") && !hCsmFile.atEnd())
			strString = ReadLine(hCsmFile);

		// Check if last line processed (should normally never happen as statistcs should follow...)
		if(hCsmFile.atEnd())
			break;

	}
	while(!strString.startsWith("*********************************") && !hCsmFile.atEnd());

	if(!hCsmFile.atEnd())
	{
		// section Parameters Spec 
		// **************************************** CHARTERED SEMICONDUCTOR MANUFACTURING ****************************************
		// **************************************** ELECTRICAL TEST BINNING SUMMARY REPORT ***************************************
		// ...
		// -------------------------------------------------------------------------------------------------------------------------
		// BIN        NAME                  LOW          HIGH         N   SR      MIN          MEAN         MAX          S.D.
		// -------------------------------------------------------------------------------------------------------------------------
		//
		//  1  TNE2 20/20 Vth0 (V)       5.300e-01    7.300e-01    125    0    5.957e-01    6.119e-01    6.351e-01    6.192e-03

		// goto to the first Parameters Spec 
		while(!strString.startsWith("-----------------------------------------------") && !hCsmFile.atEnd())
			strString = ReadLine(hCsmFile);
	}

	// Check if last line processed (should normally never happen as statistcs should follow...)
	if(!hCsmFile.atEnd())
	{
		// Skip header line
		ReadLine(hCsmFile);
		strString = ReadLine(hCsmFile);
		while(!strString.startsWith("*********************************") && !hCsmFile.atEnd())
		{
			strString = ReadLine(hCsmFile);
			if(strString.isEmpty())
				continue;
			strSection = strString.mid(5,24).trimmed();
			SaveParameterLimit(strSection,strString.mid(30,12).trimmed(),eLowLimit);
			SaveParameterLimit(strSection,strString.mid(43,12).trimmed(),eHighLimit);
		}
	}

	// All CSM file read...check if need to update the CSM Parameter list on disk?
	if(m_bNewCsmParameterFound == true)
		DumpParameterIndexTable();

	// Close file
	f.close();


	// Success parsing CSM file
	return true;
}

//////////////////////////////////////////////////////////////////////
// Create STDF file from CSM data parsed
//////////////////////////////////////////////////////////////////////
bool CGCSMtoSTDF::WriteStdfFile(const char *strFileNameSTDF)
{
	// now generate the STDF file...
    GS::StdLib::Stdf StdfFile;
    GS::StdLib::StdfRecordReadInfo RecordReadInfo;
    if(StdfFile.Open((char*)strFileNameSTDF,STDF_WRITE) != GS::StdLib::Stdf::NoError)
	{
		// Failed importing CSM file into STDF database
		m_iLastError = errWriteSTDF;
		
		// Convertion failed.
		return false;
	}

	// Write FAR
	RecordReadInfo.iRecordType = 0;
	RecordReadInfo.iRecordSubType = 10;
	StdfFile.WriteHeader(&RecordReadInfo);
	StdfFile.WriteByte(1);					// SUN CPU type
	StdfFile.WriteByte(4);					// STDF V4
	StdfFile.WriteRecord();

	if(m_lStartTime <= 0)
		m_lStartTime = QDateTime::currentDateTime().toTime_t();

	// Write MIR
	RecordReadInfo.iRecordType = 1;
	RecordReadInfo.iRecordSubType = 10;
	StdfFile.WriteHeader(&RecordReadInfo);
	StdfFile.WriteDword(m_lStartTime);			// Setup time
	StdfFile.WriteDword(m_lStartTime);			// Start time
	StdfFile.WriteByte(1);						// Station
	StdfFile.WriteByte((BYTE) 'P');				// Test Mode = PRODUCTION
	StdfFile.WriteByte((BYTE) ' ');				// rtst_cod
	StdfFile.WriteByte((BYTE) ' ');				// prot_cod
	StdfFile.WriteWord(65535);					// burn_tim
	StdfFile.WriteByte((BYTE) ' ');				// cmod_cod
	StdfFile.WriteString(m_strLotID.toLatin1().constData());		// Lot ID
	StdfFile.WriteString(m_strProductID.toLatin1().constData());	// Part Type / Product ID
	StdfFile.WriteString("");					// Node name
	StdfFile.WriteString("");					// Tester Type
	StdfFile.WriteString(m_strJobName.toLatin1().constData());		// Job name
	StdfFile.WriteString("");					// Job rev
	StdfFile.WriteString("");					// sublot-id
	StdfFile.WriteString("");					// operator
	StdfFile.WriteString("");					// exec-type
	StdfFile.WriteString("");					// exe-ver
	StdfFile.WriteString("WAFER");				// test-cod
	StdfFile.WriteString("");					// test-temperature
	// Construct custom Galaxy USER_TXT 
	QString	strUserTxt;
	strUserTxt = GEX_IMPORT_DATAORIGIN_LABEL;	
	strUserTxt += ":";
	strUserTxt += GEX_IMPORT_DATAORIGIN_ETEST;
	strUserTxt += ":Chartered Semi";
	StdfFile.WriteString(strUserTxt.toLatin1().constData());		// user-txt
	StdfFile.WriteString("");					// aux-file
	StdfFile.WriteString("");					// package-type
	StdfFile.WriteString(m_strProductID.toLatin1().constData());	// familyID
	StdfFile.WriteString("");					// Date-code
	StdfFile.WriteString("");					// Facility-ID
	StdfFile.WriteString("");					// FloorID
	StdfFile.WriteString(m_strProcessID.toLatin1().constData());	// ProcessID

	StdfFile.WriteRecord();

	// Write Test results for each waferID
	CGCsmWafer	*ptWafers;		// List of Wafers in CSM file
	char		szString[257];
	BYTE		iSiteNumber,bData;
	WORD		wSoftBin,wHardBin;
	long		iTotalGoodBin,iTotalFailBin;
	long		iTestNumber,iPartNumber=0;
	bool		bPassStatus;
	CGCsmParameter *ptParam;

	QListIterator<CGCsmWafer*> lstIteratorWafer(m_pWaferList);

	lstIteratorWafer.toFront();
	while(lstIteratorWafer.hasNext())
	{
		ptWafers	= lstIteratorWafer.next();
		iPartNumber = 0;

		// Write WIR
		RecordReadInfo.iRecordType = 2;
		RecordReadInfo.iRecordSubType = 10;
		StdfFile.WriteHeader(&RecordReadInfo);
		StdfFile.WriteByte(1);						// Test head
		StdfFile.WriteByte(255);					// Tester site (all)
		StdfFile.WriteDword(m_lStartTime);			// Start time
		sprintf(szString,"%d",ptWafers->m_iWaferID);
		StdfFile.WriteString(szString);				// WaferID
		StdfFile.WriteRecord();

		// Write all Parameters read on this wafer.: PTR....PTR, PRR
		iTotalGoodBin=iTotalFailBin=0;

		// Write PTRs for EACH of the X sites
		for(iSiteNumber = ptWafers->m_iLowestSiteID; iSiteNumber <= ptWafers->m_iHighestSiteID; iSiteNumber++)
		{
			// before write PIR, PTRs, PRR
			// verify if we have some test executed from this site
			QListIterator<CGCsmParameter*> lstIteratorParameter(ptWafers->m_pParameterList);
				
			lstIteratorParameter.toFront();
			bPassStatus = false;
			
			while(lstIteratorParameter.hasNext())
			{
				ptParam = lstIteratorParameter.next();

				if(ptParam->m_fValue.contains (iSiteNumber) == true)
					bPassStatus = true;
			};

			if(!bPassStatus)
				continue;
			
			// Write PIR for parts in this Wafer site
			RecordReadInfo.iRecordType = 5;
			RecordReadInfo.iRecordSubType = 10;
			StdfFile.WriteHeader(&RecordReadInfo);
			StdfFile.WriteByte(1);								// Test head
			StdfFile.WriteByte(iSiteNumber);					// Tester site
			StdfFile.WriteRecord();

			// Part number
			iPartNumber++;

			bPassStatus = true;

			lstIteratorParameter.toFront();
			while(lstIteratorParameter.hasNext())
			{
				ptParam = lstIteratorParameter.next();

				//////////////////////////////////////////////////////////////////////
				// For ProgressBar
				if(GexProgressBar != NULL)
				{
					iParameterCount++;
					while(iParameterCount > iNextParameter)
					{
						iProgressStep += 100/iTotalParameters + 1;
						iNextParameter  += iTotalParameters/100 + 1;
						GexProgressBar->setValue(iProgressStep);

					}
				}
                QCoreApplication::processEvents();

				// Write the PTR if it exists for this site...
				if(ptParam->m_fValue.contains (iSiteNumber) == true)
				{
					RecordReadInfo.iRecordType = 15;
					RecordReadInfo.iRecordSubType = 10;
					StdfFile.WriteHeader(&RecordReadInfo);
					// Compute Test# (add user-defined offset)
                    iTestNumber = (long) m_pFullCsmParametersList.indexOf(ptParam->m_strName);
					iTestNumber += GEX_TESTNBR_OFFSET_CSM;		// Test# offset
					StdfFile.WriteDword(iTestNumber);			// Test Number
					StdfFile.WriteByte(1);						// Test head
					StdfFile.WriteByte(iSiteNumber);			// Tester site:1,2,3,4 or 5, etc.
					if((ptParam->m_fValue[iSiteNumber] < ptParam->m_fLowLimit) || (ptParam->m_fValue[iSiteNumber] > ptParam->m_fHighLimit))
					{
						bData = 0200;	// Test Failed
						bPassStatus = false;
					}
					else
					{
						bData = 0;		// Test passed
					}
					StdfFile.WriteByte(bData);							// TEST_FLG
					StdfFile.WriteByte(0x40|0x80);						// PARAM_FLG
					StdfFile.WriteFloat(ptParam->m_fValue[iSiteNumber]);	// Test result
					if(ptParam->m_bStaticHeaderWritten == false)
					{
						// save Parameter name without unit information
						if(ptParam->m_strUnits.isEmpty())
							StdfFile.WriteString(ptParam->m_strName.toLatin1().constData());	// TEST_TXT
						else
							StdfFile.WriteString((ptParam->m_strName.left(ptParam->m_strName.lastIndexOf("(")-1).trimmed().toLatin1().constData()));	// TEST_TXT
						StdfFile.WriteString("");							// ALARM_ID

						bData = 2;	// Valid data.
						if(ptParam->m_bValidLowLimit==false)
							bData |=0x40;
						if(ptParam->m_bValidHighLimit==false)
							bData |=0x80;
						StdfFile.WriteByte(bData);							// OPT_FLAG

						StdfFile.WriteByte(-ptParam->m_nScale);				// RES_SCALE
						StdfFile.WriteByte(-ptParam->m_nScale);				// LLM_SCALE
						StdfFile.WriteByte(-ptParam->m_nScale);				// HLM_SCALE
						StdfFile.WriteFloat(ptParam->m_bValidLowLimit ? ptParam->m_fLowLimit : 0);			// LOW Limit
						StdfFile.WriteFloat(ptParam->m_bValidHighLimit ? ptParam->m_fHighLimit : 0);			// HIGH Limit
						StdfFile.WriteString(ptParam->m_strUnits.toLatin1().constData());	// Units
						ptParam->m_bStaticHeaderWritten = true;
					}
					StdfFile.WriteRecord();
				}
			};

			// Write PRR
			RecordReadInfo.iRecordType = 5;
			RecordReadInfo.iRecordSubType = 20;
			StdfFile.WriteHeader(&RecordReadInfo);
			StdfFile.WriteByte(1);						// Test head
			StdfFile.WriteByte(iSiteNumber);			// Tester site:1,2,3,4 or 5
			if(bPassStatus == true)
			{
				StdfFile.WriteByte(0);				// PART_FLG : PASSED
				wSoftBin = wHardBin = 1;
				iTotalGoodBin++;
			}
			else
			{
				StdfFile.WriteByte(8);				// PART_FLG : FAILED
				wSoftBin = wHardBin = 0;
				iTotalFailBin++;
			}
			StdfFile.WriteWord((WORD)ptWafers->m_pParameterList.count());		// NUM_TEST
			StdfFile.WriteWord(wHardBin);            // HARD_BIN
			StdfFile.WriteWord(wSoftBin);            // SOFT_BIN
			// 200mm wafers, usually have 5 sites, 300mm wafers usually have 9 sites
			if(m_lstSites.count() > 5)
			{
				switch(iSiteNumber)
				{
					case 1:	// Center
						StdfFile.WriteWord(1);			// X_COORD
						StdfFile.WriteWord(1);			// Y_COORD
						break;
					case 2:	// Bottom
						StdfFile.WriteWord(1);			// X_COORD
						StdfFile.WriteWord(2);			// Y_COORD
						break;
					case 3:	// Right
						StdfFile.WriteWord(0);			// X_COORD
						StdfFile.WriteWord(1);			// Y_COORD
						break;
					case 4:	// Top
						StdfFile.WriteWord(1);			// X_COORD
						StdfFile.WriteWord(0);			// Y_COORD
						break;
					case 5:	// Left
						StdfFile.WriteWord(2);			// X_COORD
						StdfFile.WriteWord(1);			// Y_COORD
						break;
					case 6:	// Lower-Right corner
						StdfFile.WriteWord(2);			// X_COORD
						StdfFile.WriteWord(2);			// Y_COORD
						break;
					case 7:	// Lower left corner
						StdfFile.WriteWord(0);			// X_COORD
						StdfFile.WriteWord(2);			// Y_COORD
						break;
					case 8:	// toUpper-Left corner
						StdfFile.WriteWord(0);			// X_COORD
						StdfFile.WriteWord(0);			// Y_COORD
						break;
					case 9:	// toUpper-Right corner
						StdfFile.WriteWord(2);			// X_COORD
						StdfFile.WriteWord(0);			// Y_COORD
						break;
					default: // More than 9 sites?....give 0,0 coordonates
						StdfFile.WriteWord(0);			// X_COORD
						StdfFile.WriteWord(0);			// Y_COORD
						break;
				}
			}
			else
			{
				switch(iSiteNumber)
				{
					case 1:	// Center
						StdfFile.WriteWord(1);			// X_COORD
						StdfFile.WriteWord(1);			// Y_COORD
						break;
					case 2:	// Bottom
						StdfFile.WriteWord(1);			// X_COORD
						StdfFile.WriteWord(2);			// Y_COORD
						break;
					case 3:	// Left
						StdfFile.WriteWord(0);			// X_COORD
						StdfFile.WriteWord(1);			// Y_COORD
						break;
					case 4:	// Top
						StdfFile.WriteWord(1);			// X_COORD
						StdfFile.WriteWord(0);			// Y_COORD
						break;
					case 5:	// Right
						StdfFile.WriteWord(2);			// X_COORD
						StdfFile.WriteWord(1);			// Y_COORD
						break;
					case 7:	// Bottom
						StdfFile.WriteWord(1);			// X_COORD
						StdfFile.WriteWord(2);			// Y_COORD
						break;
					case 9:	// Top
						StdfFile.WriteWord(1);			// X_COORD
						StdfFile.WriteWord(0);			// Y_COORD
						break;
					default: // More than 9 sites?....give 0,0 coordonates
						StdfFile.WriteWord(0);			// X_COORD
						StdfFile.WriteWord(0);			// Y_COORD
						break;
				}
			}
			StdfFile.WriteDword(0);				// No testing time known...
			sprintf(szString,"%ld",iPartNumber);
			StdfFile.WriteString(szString);		// PART_ID
			StdfFile.WriteString("");			// PART_TXT
			StdfFile.WriteString("");			// PART_FIX
			StdfFile.WriteRecord();
		}

			
		// Write WRR
		RecordReadInfo.iRecordType = 2;
		RecordReadInfo.iRecordSubType = 20;
		StdfFile.WriteHeader(&RecordReadInfo);
		StdfFile.WriteByte(1);						// Test head
		StdfFile.WriteByte(255);					// Tester site (all)
		StdfFile.WriteDword(m_lStartTime);			// Time of last part tested
		StdfFile.WriteDword(iTotalGoodBin+iTotalFailBin);// Parts tested: always 5
		StdfFile.WriteDword(0);						// Parts retested
		StdfFile.WriteDword(0);						// Parts Aborted
		StdfFile.WriteDword(iTotalGoodBin);			// Good Parts
		StdfFile.WriteDword((DWORD)(-1));			// Functionnal Parts

		sprintf(szString,"%d",ptWafers->m_iWaferID);
		StdfFile.WriteString(szString);				// WaferID
		StdfFile.WriteRecord();

#if 0
// NO SUMMARY FOR ETEST
		// Write SBR Bin0 (FAIL)
		RecordReadInfo.iRecordType = 1;
		RecordReadInfo.iRecordSubType = 50;
		StdfFile.WriteHeader(&RecordReadInfo);
		StdfFile.WriteByte(255);					// Test Head = ALL
		StdfFile.WriteByte(255);					// Test sites = ALL		
		StdfFile.WriteWord(0);						// SBIN = 0
		StdfFile.WriteDword(iTotalFailBin);			// Total Bins
		StdfFile.WriteByte('F');
		StdfFile.WriteRecord();

		// Write SBR Bin1 (PASS)
		RecordReadInfo.iRecordType = 1;
		RecordReadInfo.iRecordSubType = 50;
		StdfFile.WriteHeader(&RecordReadInfo);
		StdfFile.WriteByte(255);					// Test Head = ALL
		StdfFile.WriteByte(255);					// Test sites = ALL		
		StdfFile.WriteWord(1);						// SBIN = 1
		StdfFile.WriteDword(iTotalGoodBin);			// Total Bins
		StdfFile.WriteByte('P');
		StdfFile.WriteRecord();

		// Write HBR Bin0 (FAIL)
		RecordReadInfo.iRecordType = 1;
		RecordReadInfo.iRecordSubType = 40;
		StdfFile.WriteHeader(&RecordReadInfo);
		StdfFile.WriteByte(255);					// Test Head = ALL
		StdfFile.WriteByte(255);					// Test sites = ALL		
		StdfFile.WriteWord(0);						// HBIN = 0
		StdfFile.WriteDword(iTotalFailBin);			// Total Bins
		StdfFile.WriteByte('F');
		StdfFile.WriteRecord();

		// Write HBR Bin1 (PASS)
		RecordReadInfo.iRecordType = 1;
		RecordReadInfo.iRecordSubType = 40;
		StdfFile.WriteHeader(&RecordReadInfo);
		StdfFile.WriteByte(255);					// Test Head = ALL
		StdfFile.WriteByte(255);					// Test sites = ALL		
		StdfFile.WriteWord(1);						// HBIN = 1
		StdfFile.WriteDword(iTotalGoodBin);			// Total Bins
		StdfFile.WriteByte('P');
		StdfFile.WriteRecord();
#endif
	};

	// Write MRR
	RecordReadInfo.iRecordType = 1; 
	RecordReadInfo.iRecordSubType = 20;
	StdfFile.WriteHeader(&RecordReadInfo);
	StdfFile.WriteDword(0);			// File finish-time.
	StdfFile.WriteRecord();

	// Close STDF file.
	StdfFile.Close();

	// Success
	return true;
}

//////////////////////////////////////////////////////////////////////
// Convert 'FileName' CSM file, to STDF 'strFileNameSTDF' file
//////////////////////////////////////////////////////////////////////
bool CGCSMtoSTDF::Convert(const char *CsmFileName, const char *strFileNameSTDF)
{
	// No erro (default)
	m_iLastError = errNoError;

	// If STDF file already exists...do not rebuild it...unless dates not matching!
	QFileInfo fInput(CsmFileName);
	QFileInfo fOutput(strFileNameSTDF);

    QFile f( strFileNameSTDF );
    if((f.exists() == true) && (fInput.lastModified() < fOutput.lastModified()))
		return true;

	//////////////////////////////////////////////////////////////////////
	// For ProgressBar
	bool bHideProgressAfter=true;
	bool bHideLabelAfter=false;

	iProgressStep = 0;
	iNextFilePos = 0;
	iNextParameter = 0;
	iTotalParameters = 0;
	iParameterCount = 0;

	if(GexScriptStatusLabel != NULL)
	{
		if(GexScriptStatusLabel->isHidden())
		{
			bHideLabelAfter = true;
			GS::Gex::Engine::GetInstance().UpdateLabelStatus("Converting data from file "+QFileInfo(CsmFileName).fileName()+"...");
			GexScriptStatusLabel->show();
		}
	}

	if(GexProgressBar != NULL)
	{
		bHideProgressAfter = GexProgressBar->isHidden();
		GexProgressBar->setMaximum(200);
		GexProgressBar->setTextVisible(true);
		GexProgressBar->setValue(0);
		GexProgressBar->show();
	}
    QCoreApplication::processEvents();
	
    if(ReadCsmFile(CsmFileName) != true)
	{
		//////////////////////////////////////////////////////////////////////
		// For ProgressBar
		if((GexProgressBar != NULL)
		&& bHideProgressAfter)
			GexProgressBar->hide();
		
		if((GexScriptStatusLabel != NULL)
		&& bHideLabelAfter)
			GexScriptStatusLabel->hide();

		return false;	// Error reading CSM file
	}

	iNextFilePos = 0;

	if(WriteStdfFile(strFileNameSTDF) != true)
	{
		//////////////////////////////////////////////////////////////////////
		// For ProgressBar
		if((GexProgressBar != NULL)
		&& bHideProgressAfter)
			GexProgressBar->hide();
		
		if((GexScriptStatusLabel != NULL)
		&& bHideLabelAfter)
			GexScriptStatusLabel->hide();

		QFile::remove(strFileNameSTDF);
		return false;
	}
	
	//////////////////////////////////////////////////////////////////////
	// For ProgressBar
	if((GexProgressBar != NULL)
	&& bHideProgressAfter)
		GexProgressBar->hide();
	
	if((GexScriptStatusLabel != NULL)
	&& bHideLabelAfter)
		GexScriptStatusLabel->hide();
	
	// Convertion successful
	return true;
}

//////////////////////////////////////////////////////////////////////
// Read line : skip empty line
//////////////////////////////////////////////////////////////////////
QString CGCSMtoSTDF::ReadLine(QTextStream& hFile)
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
