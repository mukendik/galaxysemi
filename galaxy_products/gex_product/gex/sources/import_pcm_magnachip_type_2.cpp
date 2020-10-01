//////////////////////////////////////////////////////////////////////
// import_pcm_magnachip_type_2.cpp: Convert a .csv PCM_MAGNACHIP (ETEST)
// file to STDF V4.0
//////////////////////////////////////////////////////////////////////

#include <time.h>
#include "gqtl_global.h"
#include <qmath.h>

#ifdef _WIN32
#include "windows.h"		// For 'GetWindowsDirectoryA' declaration
#endif

#include <qfileinfo.h>
#include <qprogressbar.h>
#include <qapplication.h>
#include <qlabel.h>

#include "engine.h"
#include "import_pcm_magnachip_type_2.h"
#include "import_constants.h"

// File format: (CSV delimited)
//
//LOT NO             ,FB0770                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  
//DEVICE NAME        ,HC353335E /WM8252E                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      
//FACILITY           ,GFAB3                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   
//TEST PATTERN NAME  ,.                  ,DATE               ,2006/07/30 05:08:00                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             
//PCM SPEC NAME      ,.                  ,WAFER/LOT QTY      ,25                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              
//TEST PROGRAM NAME  ,/opt/ki/pgm/ASIC/HL,SITE/WAFER QTY     ,05                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              
//PROCESS NAME       ,HL35S23M           ,PARAMETER QTY      ,39                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              
//                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            
//PARAMETER          ,BVGOXN             ,BVGOXP             ,BVGTRN             ,...
//DESCRIPTION        ,BVGOXN             ,BVGOXP             ,BVGTRN             ,...
//SIZE               ,.                  ,.                  ,.                  ,...
//SPEC LOW           ,  7.000            ,  8.000            ,0                  ,...
//SPEC HIGH          ,  12.00            ,  12.00            ,  15.00            ,...
//WAFER ID / UNIT    ,V                  ,V                  ,V                  ,...
//1 B                ,  8.125            ,  9.375            ,  12.08            ,...
//1 C                ,  8.125            ,  9.375            ,  12.08            ,...
//1 L                ,  8.125            ,  9.375            ,  11.67            ,...

// main.cpp
extern QLabel			*GexScriptStatusLabel;	// Handle to script status text in status bar
extern QProgressBar	*	GexProgressBar;		// Handle to progress bar in status bar

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGPCM_MAGNACHIP_TYPE_2toSTDF::CGPCM_MAGNACHIP_TYPE_2toSTDF()
{
	// Default: PCM_MAGNACHIP_TYPE_2 parameter list on disk includes all known PCM_MAGNACHIP_TYPE_2 parameters...
	m_bNewPcmMagnachipType2ParameterFound = false;

	m_pCGPcmMagnachipType2Parameter = NULL;
	m_lStartTime = 0;
}

//////////////////////////////////////////////////////////////////////
// Destruction
//////////////////////////////////////////////////////////////////////
CGPCM_MAGNACHIP_TYPE_2toSTDF::~CGPCM_MAGNACHIP_TYPE_2toSTDF()
{
	// Destroy list of Parameters tables.
	if(m_pCGPcmMagnachipType2Parameter!=NULL)
		delete [] m_pCGPcmMagnachipType2Parameter;
}

//////////////////////////////////////////////////////////////////////
// Get Error
//////////////////////////////////////////////////////////////////////
QString CGPCM_MAGNACHIP_TYPE_2toSTDF::GetLastError()
{
    m_strLastError = "Import PCM (Magnachip): ";

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
		case errInvalidFormatLowInRows:
			m_strLastError += "Invalid file format: 'Parameter' line too short, missing rows";
			break;
		case errNoLimitsFound:
			m_strLastError += "Invalid file format, Specification Limits not found";
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
// Load PCM_MAGNACHIP_TYPE_2 Parameter table from DISK
//////////////////////////////////////////////////////////////////////
void CGPCM_MAGNACHIP_TYPE_2toSTDF::LoadParameterIndexTable(void)
{
	QString	strPcmMagnachipType2TableFile;
	QString	strString;

    strPcmMagnachipType2TableFile  = GS::Gex::Engine::GetInstance().Get("UserFolder").toString();
	strPcmMagnachipType2TableFile += GEX_PCM_PARAMETERS;

	// Open PCM_MAGNACHIP_TYPE_2 Parameter table file
    QFile f( strPcmMagnachipType2TableFile );
    if(!f.open( QIODevice::ReadOnly ))
		return;

	// Assign file I/O stream
	QTextStream hPcmMagnachipType2TableFile(&f);

	// Skip comment lines
	do
	{
	  strString = hPcmMagnachipType2TableFile.readLine();
	}
    while((strString.indexOf("----------------------") < 0) && (hPcmMagnachipType2TableFile.atEnd() == false));

	// Read lines
	m_pFullPcmMagnachipType2ParametersList.clear();
	strString = hPcmMagnachipType2TableFile.readLine();
	while (strString.isNull() == false)
	{
		// Save Parameter name in list
		m_pFullPcmMagnachipType2ParametersList.append(strString);
		// Read next line
		strString = hPcmMagnachipType2TableFile.readLine();
	};

	// Close file
	f.close();
}

//////////////////////////////////////////////////////////////////////
// Save PCM_MAGNACHIP_TYPE_2 Parameter table to DISK
//////////////////////////////////////////////////////////////////////
void CGPCM_MAGNACHIP_TYPE_2toSTDF::DumpParameterIndexTable(void)
{
	QString		strPcmMagnachipType2TableFile;

    strPcmMagnachipType2TableFile  = GS::Gex::Engine::GetInstance().Get("UserFolder").toString();
	strPcmMagnachipType2TableFile += GEX_PCM_PARAMETERS;

	// Open PCM_MAGNACHIP_TYPE_2 Parameter table file
    QFile f( strPcmMagnachipType2TableFile );
    if(!f.open( QIODevice::WriteOnly ))
		return;

	// Assign file I/O stream
	QTextStream hPcmMagnachipType2TableFile(&f);

	// First few lines are comments:
	hPcmMagnachipType2TableFile << "############################################################" << endl;
	hPcmMagnachipType2TableFile << "# DO NOT EDIT THIS FILE!" << endl;
    hPcmMagnachipType2TableFile << "# Quantix Examinator: PCM Parameters detected" << endl;
	hPcmMagnachipType2TableFile << "# www.mentor.com" << endl;
    hPcmMagnachipType2TableFile << "# Quantix Examinator reads and writes into this file..." << endl;
	hPcmMagnachipType2TableFile << "-----------------------------------------------------------" << endl;

	// Write lines
	// m_pFullPcmMagnachipType2ParametersList.sort();
	for(int nIndex = 0; nIndex < m_pFullPcmMagnachipType2ParametersList.count(); nIndex++)
	{
		// Write line
		hPcmMagnachipType2TableFile << m_pFullPcmMagnachipType2ParametersList[nIndex] << endl;
	};

	// Close file
	f.close();
}

//////////////////////////////////////////////////////////////////////
// If Examinator doesn't have this PCM_MAGNACHIP_TYPE_2 parameter in his dictionnary, have it added.
//////////////////////////////////////////////////////////////////////
void CGPCM_MAGNACHIP_TYPE_2toSTDF::UpdateParameterIndexTable(QString strParamName)
{
	// Check if the table is empty...if so, load it from disk first!
	if(m_pFullPcmMagnachipType2ParametersList.isEmpty() == true)
	{
		// Load PCM_MAGNACHIP_TYPE_2 parameter table from disk...
		LoadParameterIndexTable();
	}
	
	// Check if Parameter name already in table...if not, add it to the list
	// the new full list will be dumped to the disk at the end.
    if(m_pFullPcmMagnachipType2ParametersList.indexOf(strParamName) < 0)
	{
		// Update list
		m_pFullPcmMagnachipType2ParametersList.append(strParamName);

		// Set flag to force the current PCM_MAGNACHIP_TYPE_2 table to be updated on disk
		m_bNewPcmMagnachipType2ParameterFound = true;
	}
}

//////////////////////////////////////////////////////////////////////
// Normalize test limits when writing into PTR.
//////////////////////////////////////////////////////////////////////
void CGPCM_MAGNACHIP_TYPE_2toSTDF::NormalizeLimits(int iIndex)
{
	int	value_scale=0;
	QString strUnits = m_pCGPcmMagnachipType2Parameter[iIndex].strUnits;
	if(strUnits.length() <= 1)
	{
		// units too short to include a prefix, then keep it 'as-is'
		m_pCGPcmMagnachipType2Parameter[iIndex].scale = 0;
		return;
	}

	QChar cPrefix = strUnits[0];
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
	m_pCGPcmMagnachipType2Parameter[iIndex].fLowLimit *= GS_POW(10.0,value_scale);
	m_pCGPcmMagnachipType2Parameter[iIndex].fHighLimit *= GS_POW(10.0,value_scale);
	m_pCGPcmMagnachipType2Parameter[iIndex].scale = value_scale;
	if(value_scale)
		strUnits = strUnits.mid(1);	// Take all characters after the prefix.
	m_pCGPcmMagnachipType2Parameter[iIndex].strUnits = strUnits;	
}


//////////////////////////////////////////////////////////////////////
// Check if File is compatible with PCM_MAGNACHIP_TYPE_2 format
//////////////////////////////////////////////////////////////////////
bool CGPCM_MAGNACHIP_TYPE_2toSTDF::IsCompatible(const char *szFileName)
{
	QString strString;
	QString strSection;
	QString strValue;
	QString	strSite;
	bool	bIsCompatible = false;

	// Open hCsmFile file
    QFile f( szFileName );
    if(!f.open( QIODevice::ReadOnly ))
	{
		// Failed Opening ASL1000 file
		return false;
	}
	// Assign file I/O stream
	QTextStream hPcmMagnachipType2File(&f);

	// extract SMIC_WAT information
	//LOT NO             ,FB0770                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  
	//DEVICE NAME        ,HC353335E /WM8252E                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      
	//FACILITY           ,GFAB3                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   
	//TEST PATTERN NAME  ,.                  ,DATE               ,2006/07/30 05:08:00                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             
	//PCM SPEC NAME      ,.                  ,WAFER/LOT QTY      ,25                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              
	//TEST PROGRAM NAME  ,/opt/ki/pgm/ASIC/HL,SITE/WAFER QTY     ,05                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              
	//PROCESS NAME       ,HL35S23M           ,PARAMETER QTY      ,39                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              
	while(hPcmMagnachipType2File.atEnd() == false)
	{
		if(strString.isEmpty())
			strString = hPcmMagnachipType2File.readLine().simplified();
		if(strString.isEmpty())
			continue;

		strSection = strString.section(",",0,0).simplified().toUpper();
		strValue = strString.section(",",1,1).simplified();

		if(strValue == ".")
			strValue = "";


		//PARAMETER          ,BVGOXN             ,BVGOXP             ,BVGTRN             ,...
		//DESCRIPTION        ,BVGOXN             ,BVGOXP             ,BVGTRN             ,...
		//SIZE               ,.                  ,.                  ,.                  ,...
		if(strSection=="PARAMETER")
		{
			strString = hPcmMagnachipType2File.readLine().simplified();
			strSection = strString.section(",",0,0).simplified().toUpper();
			if(strSection=="DESCRIPTION")
			{
				strString = hPcmMagnachipType2File.readLine().simplified();
				strSection = strString.section(",",0,0).simplified().toUpper();
				if(strSection=="SIZE")
				{
					bIsCompatible = true;
					break;
				}
			}
		}

		if((strSection=="LOT NO")
		|| (strSection=="DEVICE NAME")
		|| (strSection=="TEST PATTERN NAME")
		|| (strSection=="PCM SPEC NAME")
		|| (strSection=="TEST PROGRAM NAME")
		|| (strSection=="PROCESS NAME")
		|| (strSection=="WAFER/LOT QTY")
		|| (strSection=="SITE/WAFER QTY")
		|| (strSection=="PARAMETER QTY")
		|| (strSection=="DATE"))
            strSection = "";
		else
		{
			// Incorrect header...this is not a PCM_MAGNACHIP_TYPE_2 file!
			// Close file
			bIsCompatible = false;
			break;
		}
		strString = strString.section(",",2);

	}

	// Close file
	f.close();

	return bIsCompatible;
}

//////////////////////////////////////////////////////////////////////
// Read and Parse the PCM_MAGNACHIP_TYPE_2 file
//////////////////////////////////////////////////////////////////////
bool CGPCM_MAGNACHIP_TYPE_2toSTDF::ReadPcmMagnachipType2File(const char *PcmMagnachipType2FileName, const char *strFileNameSTDF)
{
	QString strString;
	QString strSection;
	QString strValue;
	bool	bStatus;
	int		iIndex;				// Loop index

	// Open CSV file
    QFile f( PcmMagnachipType2FileName );
    if(!f.open( QIODevice::ReadOnly ))
	{
		// Failed Opening PCM_MAGNACHIP_TYPE_2 file
		m_iLastError = errOpenFail;
		
		// Convertion failed.
		return false;
	}

	// Assign file I/O stream
	QTextStream hPcmMagnachipType2File(&f);

	//////////////////////////////////////////////////////////////////////
	// For ProgressBar
	iProgressStep = 0;
	iNextFilePos = 0;
	iFileSize = f.size() + 1;
    QCoreApplication::processEvents();
	
	// extract SMIC_WAT information
	//LOT NO             ,FB0770                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  
	//DEVICE NAME        ,HC353335E /WM8252E                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      
	//FACILITY           ,GFAB3                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   
	//TEST PATTERN NAME  ,.                  ,DATE               ,2006/07/30 05:08:00                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             
	//PCM SPEC NAME      ,.                  ,WAFER/LOT QTY      ,25                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              
	//TEST PROGRAM NAME  ,/opt/ki/pgm/ASIC/HL,SITE/WAFER QTY     ,05                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              
	//PROCESS NAME       ,HL35S23M           ,PARAMETER QTY      ,39                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              
	while(hPcmMagnachipType2File.atEnd() == false)
	{
		if(strString.isEmpty())
			strString = ReadLine(hPcmMagnachipType2File).simplified();
		if(strString.isEmpty())
			continue;

		strSection = strString.section(",",0,0).simplified().toUpper();
		strValue = strString.section(",",1,1).simplified();

		if(strValue == ".")
			strValue = "";


		if(strSection=="LOT NO")
		{
			m_strLotId = strValue.simplified();
		}
		else
		if(strSection=="DEVICE NAME")
		{
			m_strProductId = strValue.simplified();
		}
		else
		if(strSection=="FACILITY")
		{
			m_strFacilityId = strValue.simplified();
		}
		else
		if(strSection=="TEST PATTERN NAME")
		{
		}
		else
		if(strSection=="PCM SPEC NAME")
		{
			m_strSpecificationName = strValue.simplified();
		}
		else
		if(strSection=="TEST PROGRAM NAME")
		{
			m_strProgramName = strValue.simplified();
		}
		else
		if(strSection=="PROCESS NAME")
		{
			m_strProcessId = strValue;
		}
		else
		if(strSection=="WAFER/LOT QTY")
		{
		}
		else
		if(strSection=="SITE/WAFER QTY")
		{
		}
		else
		if(strSection=="PARAMETER QTY")
		{
		}
		else
		if(strSection=="DATE")
		{
			QDate clDate(strValue.section(' ',0,0).section("/",2,2).simplified().toInt(),
						 strValue.section(' ',0,0).section("/",0,0).simplified().toInt(),
						 strValue.section(' ',0,0).section("/",1,1).simplified().toInt());
			QTime clTime = QTime::fromString(strValue.section(' ',1).simplified());
			QDateTime clDateTime(clDate,clTime);
			clDateTime.setTimeSpec(Qt::UTC);
			if(!clDateTime.isValid())
				clDateTime = QDateTime::fromString(strValue);
			if(!clDateTime.isValid())
				clDateTime = QDateTime::fromString(strValue,Qt::ISODate);
			if(clDateTime.isValid())
				m_lStartTime = clDateTime.toTime_t();
		}
		else
		if(strSection=="PARAMETER")
			break;
		else
		{
			// Incorrect header...this is not a PCM_MAGNACHIP_TYPE_2 file!
			m_iLastError = errInvalidFormat;
			
			// Convertion failed.
			// Close file
			f.close();
			return false;
		}
		strString = strString.section(",",2);

	}

	//PARAMETER          ,BVGOXN             ,BVGOXP             ,BVGTRN             ,...
	//DESCRIPTION        ,BVGOXN             ,BVGOXP             ,BVGTRN             ,...
	//SIZE               ,.                  ,.                  ,.                  ,...
	//SPEC LOW           ,  7.000            ,  8.000            ,0                  ,...
	//SPEC HIGH          ,  12.00            ,  12.00            ,  15.00            ,...
	//WAFER ID / UNIT    ,V                  ,V                  ,V                  ,...
	//1 B                ,  8.125            ,  9.375            ,  12.08            ,...

	// have found the parameter description
	// skip the 2 other column description to have only parameter name
	strString = strString.section(",",1);
	// Count the number of parameters specified in the line
	m_iTotalParameters = strString.count(",")+1;

	// If no parameter specified...ignore!
	if(m_iTotalParameters <= 0)
	{
		// Incorrect header...this is not a valid PCM_MAGNACHIP_TYPE_2 file!
		m_iLastError = errInvalidFormat;
		
		// Convertion failed.
		// Close file
		f.close();
		return false;
	}

	// Allocate the buffer to hold the N parameters & results.
	m_pCGPcmMagnachipType2Parameter = new CGPcmMagnachipType2Parameter[m_iTotalParameters];	// List of parameters

	// Extract the N column names
	//PARAMETER          ,BVGOXN             ,BVGOXP             ,BVGTRN             ,...
	//DESCRIPTION        ,BVGOXN             ,BVGOXP             ,BVGTRN             ,...
	QString strName,strDescription;
	QString strNames,strDescriptions;
	QStringList lstNames, lstDescritions;

	strNames = strString;
	strString = ReadLine(hPcmMagnachipType2File);
	strDescriptions = strString.section(',',1);

    lstNames = strNames.split(",", QString::KeepEmptyParts);
	// Check if have the good count
	if(lstNames.count() < m_iTotalParameters)
	{
		// Incorrect header...this is not a valid CSM file!
		m_iLastError = errInvalidFormatLowInRows;
		
		// Convertion failed.
		// Close file
		f.close();
		return false;
	}
	
    lstDescritions = strDescriptions.split(",", QString::KeepEmptyParts);

	for(iIndex=0;iIndex<m_iTotalParameters;iIndex++)
	{
		strName = lstNames[iIndex].trimmed();
		strDescription = "";
		if(lstDescritions.count() > iIndex)
			strDescription = lstDescritions[iIndex];
		strDescription = strDescription.trimmed();
		if((!strDescription.isEmpty()) && (strDescription != ".") && (strName != strDescription))
			strName += " - " + strDescription;

		m_pCGPcmMagnachipType2Parameter[iIndex].strName = strName;
		UpdateParameterIndexTable(strName);		// Update Parameter master list if needed.
		m_pCGPcmMagnachipType2Parameter[iIndex].bStaticHeaderWritten = false;
	}

	//SIZE               ,.                  ,.                  ,.                  ,...
	//SPEC LOW           ,  7.000            ,  8.000            ,0                  ,...
	//SPEC HIGH          ,  12.00            ,  12.00            ,  15.00            ,...
	//WAFER ID / UNIT    ,V                  ,V                  ,V                  ,...
	int	iLimits =0;
	QStringList lstSections;
	while(hPcmMagnachipType2File.atEnd() == false)
	{
		strString = ReadLine(hPcmMagnachipType2File);
        lstSections = strString.split(",", QString::KeepEmptyParts);
		if(strString.startsWith("WAFER ID / UNIT", Qt::CaseInsensitive) == true)
		{
			// found the UNIT limits
	
			// Extract the N column toUpper Limits
			for(iIndex=0;iIndex<m_iTotalParameters;iIndex++)
			{
				strSection = "";
				if(lstSections.count() > iIndex+1)
					strSection = lstSections[iIndex+1];
				strSection = strSection.simplified();
				m_pCGPcmMagnachipType2Parameter[iIndex].strUnits = strSection;
			}
			break;
		}
		else
		if(strString.startsWith("SPEC HIGH", Qt::CaseInsensitive) == true)
		{
			// found the HIGH limits
			iLimits |= 1;

			// Extract the N column toUpper Limits
			for(iIndex=0;iIndex<m_iTotalParameters;iIndex++)
			{
				strSection = "";
				if(lstSections.count() > iIndex+1)
					strSection = lstSections[iIndex+1];
				m_pCGPcmMagnachipType2Parameter[iIndex].fHighLimit = strSection.simplified().toFloat(&bStatus);
				m_pCGPcmMagnachipType2Parameter[iIndex].bValidHighLimit = bStatus;
			}
		}
		else
		if(strString.startsWith("SPEC LOW", Qt::CaseInsensitive) == true)
		{
			// found the Low limits
			iLimits |= 2;

			// Extract the N column Lower Limits
			for(iIndex=0;iIndex<m_iTotalParameters;iIndex++)
			{
				strSection = "";
				if(lstSections.count() > iIndex+1)
					strSection = lstSections[iIndex+1];
				m_pCGPcmMagnachipType2Parameter[iIndex].fLowLimit = strSection.simplified().toFloat(&bStatus);
				m_pCGPcmMagnachipType2Parameter[iIndex].bValidLowLimit = bStatus;
			}
		}
		else
		if(strString.startsWith("SIZE", Qt::CaseInsensitive) == true)
			continue;
	}

	if(iLimits != 3)
	{
		// Incorrect header...this is not a valid PCM_MAGNACHIP_TYPE_2 file!: we didn't find the limits!
		m_iLastError = errNoLimitsFound;
		
		// Convertion failed.
		bStatus = false;
	}

	// Normalize all Limits
	for(iIndex=0;iIndex<m_iTotalParameters;iIndex++)
		NormalizeLimits(iIndex);

	// Loop reading file until end is reached & generate STDF file dynamically.
	bStatus = WriteStdfFile(&hPcmMagnachipType2File,strFileNameSTDF);
	if(!bStatus)
		QFile::remove(strFileNameSTDF);

	// All PCM_MAGNACHIP_TYPE_2 file read...check if need to update the PCM_MAGNACHIP_TYPE_2 Parameter list on disk?
	if(bStatus && (m_bNewPcmMagnachipType2ParameterFound == true))
		DumpParameterIndexTable();

	// Close file
	f.close();

	// Success parsing PCM_MAGNACHIP_TYPE_2 file
	return bStatus;
}

//////////////////////////////////////////////////////////////////////
// Create STDF file from PCM_MAGNACHIP_TYPE_2 data parsed
//////////////////////////////////////////////////////////////////////
bool CGPCM_MAGNACHIP_TYPE_2toSTDF::WriteStdfFile(QTextStream *hPcmMagnachipType2File, const char *strFileNameSTDF)
{
	// now generate the STDF file...
    GS::StdLib::Stdf StdfFile;
    GS::StdLib::StdfRecordReadInfo RecordReadInfo;
    if(StdfFile.Open((char*)strFileNameSTDF,STDF_WRITE) != GS::StdLib::Stdf::NoError)
	{
		// Failed importing CSV file into STDF database
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
	StdfFile.WriteString(m_strLotId.toLatin1().constData());		// Lot ID
	StdfFile.WriteString(m_strProductId.toLatin1().constData());	// Part Type / Product ID
	StdfFile.WriteString("");					// Node name
	StdfFile.WriteString("");					// Tester Type
	StdfFile.WriteString(m_strProgramName.toLatin1().constData());	// Job name
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
	strUserTxt += ":HYNIX MAGNACHIP";
	StdfFile.WriteString((char *)strUserTxt.toLatin1().constData());		// user-txt
	StdfFile.WriteString("");								// aux-file
	StdfFile.WriteString("");								// package-type
	StdfFile.WriteString(m_strProductId.toLatin1().constData());	// familyID
	StdfFile.WriteString("");								// Date-code
	StdfFile.WriteString(m_strFacilityId.toLatin1().constData());	// Facility-ID
	StdfFile.WriteString("");								// FloorID
	StdfFile.WriteString(m_strProcessId.toLatin1().constData());	// ProcessID
	StdfFile.WriteString("");								// OperFrq
	StdfFile.WriteString(m_strSpecificationName.toLatin1().constData());	// SpecName
	StdfFile.WriteRecord();

	// Write Test results for each line read.
	QString strString;
	QString strSection;
	QString	strWaferID;
	float	fValue;				// Used for readng floating point numbers.
	int		value_scale;		// Scale factor for limots & results.
	int		iIndex;				// Loop index
	int		iSiteNumber;
	QString	strSiteNumber;
	BYTE	bData;
	WORD	wSoftBin,wHardBin;
	long	iTotalGoodBin,iTotalFailBin;
	long	iTestNumber,iTotalTests,iPartNumber;
	bool	bStatus,bPassStatus;

	QStringList lstStrings, lstSections;
	// Reset counters
	iTotalGoodBin=iTotalFailBin=0;
	iPartNumber=0;

	// Write all Parameters read on this wafer.: WIR.PIR,PTR....PTR, PRR, PIR, PTR,PTR...PRR,   ... WRR
	while(hPcmMagnachipType2File->atEnd() == false)
	{

		// Part number
		iPartNumber++;

		// Read line
		strString = ReadLine(*hPcmMagnachipType2File);
		// Extract WaferID
		strSection = strString.section(',',0,0).simplified();
		if(strSection.section(' ',0,0) != strWaferID)
		{
			// Write WRR in case we have finished to write wafer records.
			if(strWaferID.isEmpty() == false)
			{
				// WRR
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
				StdfFile.WriteDword((DWORD)-1)	;			// Functionnal Parts
				StdfFile.WriteString(strWaferID.toLatin1().constData());	// WaferID
				StdfFile.WriteRecord();
			}

			iTotalGoodBin=iTotalFailBin=0;
			
			// For each wafer, have to write limit in the first PTR
			for(iIndex=0;iIndex<m_iTotalParameters;iIndex++)
				m_pCGPcmMagnachipType2Parameter[iIndex].bStaticHeaderWritten = false;

			// Write WIR of new Wafer.
			strWaferID = strSection.section(' ',0,0);
			RecordReadInfo.iRecordType = 2;
			RecordReadInfo.iRecordSubType = 10;
			StdfFile.WriteHeader(&RecordReadInfo);
			StdfFile.WriteByte(1);								// Test head
			StdfFile.WriteByte(255);							// Tester site (all)
			StdfFile.WriteDword(m_lStartTime);					// Start time
			StdfFile.WriteString(strWaferID.toLatin1().constData());	// WaferID
			StdfFile.WriteRecord();
		}

		// Reset Pass/Fail flag.
		bPassStatus = true;

		// Reset counters
		iTotalTests = 0;

		// Extract Site
		strSiteNumber = strSection.section(' ',1,1).simplified();
		if(strSiteNumber.startsWith("C", Qt::CaseInsensitive))
			iSiteNumber = 1;		// Center
		else
		if(strSiteNumber.startsWith("B", Qt::CaseInsensitive))
			iSiteNumber = 2;		// Bottom
		else
		if(strSiteNumber.startsWith("L", Qt::CaseInsensitive))
			iSiteNumber = 3;		// Left
		else
		if(strSiteNumber.startsWith("T", Qt::CaseInsensitive))
			iSiteNumber = 4;		// Top
		else
			iSiteNumber = 5;		// Right, or other position.

		// Write PIR for parts in this Wafer site
		RecordReadInfo.iRecordType = 5;
		RecordReadInfo.iRecordSubType = 10;
		StdfFile.WriteHeader(&RecordReadInfo);
		StdfFile.WriteByte(1);								// Test head
		StdfFile.WriteByte(iSiteNumber);					// Tester site
		StdfFile.WriteRecord();

		//1 B                ,  8.125            ,  9.375            ,  12.08            ,...
		// Read Parameter results for this record
        lstSections = strString.split(",", QString::KeepEmptyParts);

		for(iIndex=0;iIndex<m_iTotalParameters;iIndex++)
		{
			strSection = "";
			if(lstSections.count() > iIndex+1)
				strSection = lstSections[iIndex+1];
			fValue = strSection.simplified().toFloat(&bStatus);

			// Normalize result
			value_scale = m_pCGPcmMagnachipType2Parameter[iIndex].scale;
			fValue*= GS_POW(10.0,value_scale);

			if(bStatus == true)
			{
				// Valid test result...write the PTR
				iTotalTests++;

				RecordReadInfo.iRecordType = 15;
				RecordReadInfo.iRecordSubType = 10;
				StdfFile.WriteHeader(&RecordReadInfo);
				// Compute Test# (add user-defined offset)
                iTestNumber = (long) m_pFullPcmMagnachipType2ParametersList.indexOf(m_pCGPcmMagnachipType2Parameter[iIndex].strName);
				iTestNumber += GEX_TESTNBR_OFFSET_PCM;		// Test# offset
				StdfFile.WriteDword(iTestNumber);			// Test Number
				StdfFile.WriteByte(1);						// Test head
				StdfFile.WriteByte(iSiteNumber);			// Tester site#
				if(((m_pCGPcmMagnachipType2Parameter[iIndex].bValidLowLimit==true) && (fValue < m_pCGPcmMagnachipType2Parameter[iIndex].fLowLimit)) ||
				   ((m_pCGPcmMagnachipType2Parameter[iIndex].bValidHighLimit==true) && (fValue > m_pCGPcmMagnachipType2Parameter[iIndex].fHighLimit)))
				{
					bData = 0200;	// Test Failed
					bPassStatus = false;
				}
				else
				{
					bData = 0;		// Test passed
				}
				StdfFile.WriteByte(bData);							// TEST_FLG
				bData = 0;
				if(m_pCGPcmMagnachipType2Parameter[iIndex].bValidLowLimit==true)
					bData = 0x40;		// not strict limit
				if(m_pCGPcmMagnachipType2Parameter[iIndex].bValidHighLimit==true)
					bData |= 0x80;		// not strict limit
				StdfFile.WriteByte(bData);							// PARAM_FLG
				StdfFile.WriteFloat(fValue);						// Test result
				if(m_pCGPcmMagnachipType2Parameter[iIndex].bStaticHeaderWritten == false)
				{
					StdfFile.WriteString(m_pCGPcmMagnachipType2Parameter[iIndex].strName.toLatin1().constData());	// TEST_TXT
					StdfFile.WriteString("");							// ALARM_ID
					bData = 2;	// Valid data.
					if(m_pCGPcmMagnachipType2Parameter[iIndex].bValidLowLimit==false)
						bData |=0x40;
					if(m_pCGPcmMagnachipType2Parameter[iIndex].bValidHighLimit==false)
						bData |=0x80;
					StdfFile.WriteByte(bData);							// OPT_FLAG
					StdfFile.WriteByte(0);								// RES_SCALE
					StdfFile.WriteByte(-value_scale);					// LLM_SCALE
					StdfFile.WriteByte(-value_scale);					// HLM_SCALE
					StdfFile.WriteFloat(m_pCGPcmMagnachipType2Parameter[iIndex].fLowLimit);		// LOW Limit
					StdfFile.WriteFloat(m_pCGPcmMagnachipType2Parameter[iIndex].fHighLimit);		// HIGH Limit
					StdfFile.WriteString(m_pCGPcmMagnachipType2Parameter[iIndex].strUnits.toLatin1().constData());							// No Units
					m_pCGPcmMagnachipType2Parameter[iIndex].bStaticHeaderWritten = true;
				}
				StdfFile.WriteRecord();
			}	// Valid test result
		}		// Read all results on line

		// Write PRR
		RecordReadInfo.iRecordType = 5;
		RecordReadInfo.iRecordSubType = 20;
		StdfFile.WriteHeader(&RecordReadInfo);
		StdfFile.WriteByte(1);			// Test head
		StdfFile.WriteByte(iSiteNumber);// Tester site#:1
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
		StdfFile.WriteWord((WORD)iTotalTests);	// NUM_TEST
		StdfFile.WriteWord(wHardBin);           // HARD_BIN
		StdfFile.WriteWord(wSoftBin);           // SOFT_BIN
		switch(iSiteNumber)
		{
			case 1:	// Center
				StdfFile.WriteWord(1);			// X_COORD
				StdfFile.WriteWord(1);			// Y_COORD
				break;
			case 2:	// Down
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
			default: // More than 5 sites?....give 0,0 coordonates
				StdfFile.WriteWord(0);			// X_COORD
				StdfFile.WriteWord(0);			// Y_COORD
				break;
		}
		StdfFile.WriteDword(0);				// No testing time known...
		StdfFile.WriteString(QString::number(iPartNumber).toLatin1().constData());		// PART_ID
		StdfFile.WriteString("");			// PART_TXT
		StdfFile.WriteString("");			// PART_FIX
		StdfFile.WriteRecord();
	};			// Read all lines with valid data records in file

	// Write WRR for last wafer inserted
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
	StdfFile.WriteDword((DWORD)-1);			// Functionnal Parts
	StdfFile.WriteString(strWaferID.toLatin1().constData());	// WaferID
	StdfFile.WriteRecord();

	// Write MRR
	RecordReadInfo.iRecordType = 1; 
	RecordReadInfo.iRecordSubType = 20;
	StdfFile.WriteHeader(&RecordReadInfo);
	StdfFile.WriteDword(m_lStartTime);			// File finish-time.
	StdfFile.WriteRecord();

	// Close STDF file.
	StdfFile.Close();

	// Success
	return true;
}

//////////////////////////////////////////////////////////////////////
// Convert 'FileName' PCM_MAGNACHIP_TYPE_2 file, to STDF 'strFileNameSTDF' file
//////////////////////////////////////////////////////////////////////
bool CGPCM_MAGNACHIP_TYPE_2toSTDF::Convert(const char *PcmMagnachipType2FileName, const char *strFileNameSTDF)
{
	// No erro (default)
	m_iLastError = errNoError;

	// If STDF file already exists...do not rebuild it...unless dates not matching!
	QFileInfo fInput(PcmMagnachipType2FileName);
	QFileInfo fOutput(strFileNameSTDF);

    QFile f( strFileNameSTDF );
    if((f.exists() == true) && (fInput.lastModified() < fOutput.lastModified()))
		return true;

	//////////////////////////////////////////////////////////////////////
	// For ProgressBar
	bool bHideProgressAfter=true;
	bool bHideLabelAfter=false;
	if(GexScriptStatusLabel != NULL)
	{
		if(GexScriptStatusLabel->isHidden())
		{
			bHideLabelAfter = true;
			GS::Gex::Engine::GetInstance().UpdateLabelStatus("Converting data from file "+QFileInfo(PcmMagnachipType2FileName).fileName()+"...");
			GexScriptStatusLabel->show();
		}
	}

	if(GexProgressBar != NULL)
	{
		bHideProgressAfter = GexProgressBar->isHidden();
		GexProgressBar->setMaximum(100);
		GexProgressBar->setTextVisible(true);
		GexProgressBar->setValue(0);
		GexProgressBar->show();
	}
    QCoreApplication::processEvents();
	
    if(ReadPcmMagnachipType2File(PcmMagnachipType2FileName,strFileNameSTDF) != true)
	{
		//////////////////////////////////////////////////////////////////////
		// For ProgressBar
		if((GexProgressBar != NULL)
		&& bHideProgressAfter)
			GexProgressBar->hide();
		
		if((GexScriptStatusLabel != NULL)
		&& bHideLabelAfter)
			GexScriptStatusLabel->hide();
		return false;	// Error reading PCM_MAGNACHIP_TYPE_2 file
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
QString CGPCM_MAGNACHIP_TYPE_2toSTDF::ReadLine(QTextStream& hFile)
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
	{
		if(hFile.atEnd())
			break;
		strString = hFile.readLine();
		if(QString(strString).remove('\t').isEmpty())
			strString = "";
	}
	while(!strString.isNull() && strString.isEmpty());

	return strString;

}
