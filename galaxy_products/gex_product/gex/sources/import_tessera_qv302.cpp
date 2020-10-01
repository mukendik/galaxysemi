//////////////////////////////////////////////////////////////////////
// import_tessera_QV302.cpp: Convert a TesseraQV302 .csv
// file to STDF V4.0
//////////////////////////////////////////////////////////////////////

#include "gqtl_global.h"
#include <qmath.h>

#ifdef _WIN32
#include "windows.h"		// For 'GetWindowsDirectoryA' declaration
#endif

#include <qfileinfo.h>
#include <qregexp.h>
#include <qprogressbar.h>
#include <qapplication.h>
#include <qlabel.h>

#include "engine.h"
#include "import_tessera_qv302.h"
#include "time.h"
#include "import_constants.h"

// QV302 format 1
//SNPASSY-CKingsley-WC146-U-STX-910-200712131925-FULL
//~
//~,Company: ,Tessera North America ,  , Machine ID: QV302  WC146
//~,Jupiter Sinope Assy Program, SINOPE_ASSY.QVB,, Revision 3
//~,Date Time: ,12/13/2007 7:25:58 PM
//~,Local Result filename: ,C:\DOC_QVPak\Results\SNPASSY-CKingsley-WC146-U-STX-910-200712131925-FULL-MPTS.txt
//~,Network Result filename: ,\\Yogi\tester\QV302\SNPASSY-CKingsley-WC146-U-STX-910-200712131925-FULL-MPTS.txt
//~,Operator: ,CKingsley
//~,Batch # ,STX-910, ,Comment: ,Jupiter Fixture #123
//~, *** All measurement values in microns or degrees. ***
//~,Prod ID, Redo#, Part ID
//Column,Row,Part ID,CTR_R(um),CTR_X(um),CTR_Y(um),DIE_XZ_Angle(deg),DIE_YZ_ANGLE(deg),DIE_ROT_ANGLE(deg),DIE_VCSEL_Z(um),LOAD_DIFF(um),FIXTURE,REV,NOTES
//-1,0,STX-SPC,7.302,-3.602,-6.352,0.011,-.197,0.164,87.585,-4.8,123,3,
//-1,1,STX-SPC,8.040,-4.549,-6.629,0.006,-.211,0.167,87.977,-8.7,123,3,
//~,
//~,Runtime = ,2.388, minutes,, Avg. Time/Part = ,0.796, minutes

// QV302 format 2
//RSE_LDR-EHANT-WC146-S-17954-22UV-200906120849-FULL-MPTS
//Column,Row,Part ID,Wafer ID,# of pits,Pitted area um2,Pitted area %,Dia um,Roundness um
//25,0,1,17954-22UV,1,0,0.005,220.8,12.2
//21,0,1,17954-22UV,0,0,0,224.4,3.9

// TalySurf Format 3
//TALYPOLY000-user-WC194-U-18720-13DT-200906041009-FULL
//Column,Row,Part ID,A,SD,SDnp,Apr,Chi^2,x^0(mm),x^1(mm),x^2(mm),x^4(mm),x^6(mm),x^8(mm),x^10(mm),x^12(mm),x^14(mm),x^16(mm),talysamp rev,angle,decenter,tilt,SDhi,SDlo,surface,product,timestamp,wc,side,spec rev,Sag(PV),SDnp(AW),PV,PVnp,Outlier,
//17,21,18720-13DT,-6.79731,184.843,77.9846,0.573,5.30729e-006,0.0492077,-1.13771e-005,-0.927038,3.73415,-148.588,1972.22,-9687.91,-2315.33,-346.782,-42.1529,1.78,0,0,-0.00286642,12.2744,59.9998,B,900,200906041009,WC194,U,62,0.141807,101.241,902.1,456.417,0,
//18,21,18720-13DT,-5.6668,155.411,67.6659,0.573,5.15957e-006,0.0431214,-3.65131e-005,-0.928463,4.02294,-160.462,2156.3,-10608.9,-2536.27,-380.011,-46.2093,1.78,0,-0.00061,-0.00185538,14.6568,50.7024,B,900,200906041009,WC194,U,62,0.141745,86.7118,788.472,408.731,0,

// Voyager Format 4
//RSRES_DIA-EH-WC25-S-17954-22UV-200906181050-SAMPLE-MPTS
//~, Io TXS Reflow lens diameter metrology.
//~, Wafer: 17954-22UV
//~, User: EH    ,Date: 06/18/09 ,  Time: 10:51:35
//~, Result File: \\Yogi\tester\voyager\RSRES_DIA-EH-WC25-S-17954-22UV-200906181050-SAMPLE-MPTS.txt
//~, Network File: \\Yogi\tester\voyager\RSRES_DIA-EH-WC25-S-17954-22UV-200906181050-SAMPLE-MPTS.txt
//~, Pattern File: C:\Prog\Jupiter\Refractives\Map_Files\Io_CMM_s.txt
//~, Program:  Io_TXS_Reflow_10X.voy
//~, Revision: 001
//~, Lens:  Nikon 10X/0.21NA
//~, Values in units of microns.
//Column,Row,Part ID,DIA(um),ROUND(um),REV
//11,00,0,201.02,1.32,001
//15,00,0,200.55,1.35,001

// Voyager Format 5
//APTR_DIA-EH-WC25-U-18918-18DT-200906111158-SAMPLE-MPTS
//~, 969_Aperture lens diameter metrology.
//~, Wafer: 18918-18DT
//~, User: EH    ,Date: 06/11/09 ,  Time: 11:59:08
//~, Result File: \\Yogi\tester\voyager\APTR_DIA-EH-WC25-U-18918-18DT-200906111158-SAMPLE-MPTS.txt
//~, Network File: \\Yogi\tester\voyager\APTR_DIA-EH-WC25-U-18918-18DT-200906111158-SAMPLE-MPTS.txt
//~, Pattern File: C:\Prog\969\Map_files\969_Aperture_S_pat3.txt
//~, Program:  969_B_Aperture_U_Cir_6X.voy
//~, Revision: 001
//~, Lens:  High Mag
//~, Values in units of microns.
//Column,Row,DIA(um),ROUND(um)
//15,09,1007.26,5.83
//22,09,1007.00,5.73

// IVS Format 6
//CADENCE_PR_E2E3_1_NZREG-PROGRAM-ACVR294-U-18075-58UV-200901301309-FULL-MPTS.txt,
//~,SYSTEM:ACVR294
//~,DEVICE:CADENCE
//~,LEVEL:PR_E2E3
//~,JOB_PLAN:1_NZREG
//~,LOT_#:18075-58UV
//~,DATE:Fri Jan 30 2009 13:9:52
//~,USER_NAME:PROGRAM
//~,HEADER_LENGTH:19
//~,WAFER_SIZE:150
//~,DIE_X_ARRAY:10
//~,DIE_Y_ARRAY:10
//~,SITE_X_ARRAY:1
//~,SITE_Y_ARRAY:1
//~,DIE_STEP_X:11469.96
//~,DIE_STEP_Y:11470.5
//~,SITE_STEP_X:1,,,,
//~,SITE_STEP_Y:1,,,,
//~,DIE_X_OFFSET:-576.56,,,,
//~,DIE_Y_OFFSET:1960.64,,,,
//~,SITE_X_OFFSET:0,,,,
//~,SITE_Y_OFFSET:0,,,,
//~,PIVOT_X:0,,,,
//~,PIVOT_Y:0,,,,
//~,EXPOSURE_AXIS_X:0,,,,
//~,EXPOSURE_AXIS_Y:0,,,,
//~,SITE_LOCATION:SITE1X:43.4 Y:-10.22,,,,
//~,SLOT:20,,,,
//Column,Row,Part ID,~NA,X_ovly,Y_ovly
//1,3,1_1,S1,0.0535,-0.0595
//4,3,2_1,S1,0.0533,-0.0385

// ET MOE Format 7
//MOE-ppenilla-excilab3-U-14878-303AF-200901280755-FULL
//"RowOffset:",0,"ColOffset:",0
//"RowMult:",1,"ColMult:",1
//~, 12 Part Basic Wafer Data Sheet,,,,,,,,,,,,,,,,,,
//~,,,,,,,,,,,,,,,,,,,
//~,Wafer #,RWAF00103-14878-303AF,,Product Name,MDOC00755_A_KIN045,,,,,,,,,,,,,,
//~,Date,1/28/09,,,,,,,,,,,,,,,,,
//~,Wavelength,193,nm,,,All energy measurements in uJ.,,,,,,,,,,,,,
//,,,,,,,,,,,,,,,,,,,
//Column,Row,Part ID,Zero_Order (%),Reference,0th,Diffuse,PASS/FAIL,,,,,,,,,,,,
//0,1,K,1.25,149.5,8.58,7.31,FAIL,,,,,,,,,,,,

// ICOS Fomat 8
//inspection2-jgardner-WCXXX-U-18195-2DT-200902050816-FULL-MPTS.csv,,
//Column,Row,Pass
//43,72,0
//42,72,0

// main.cpp
extern QLabel			*GexScriptStatusLabel;	// Handle to script status text in status bar
extern QProgressBar	*	GexProgressBar;		// Handle to progress bar in status bar


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGTesseraQV302toSTDF::CGTesseraQV302toSTDF()
{
	// Default: TesseraQV302 parameter list on disk includes all known TesseraQV302 parameters...
	m_bNewTesseraQV302ParameterFound = false;
	m_lStartTime = 0;
	m_strTesterType = "Qv302";

	m_pTesseraQV302Parameter = NULL;
}

//////////////////////////////////////////////////////////////////////
// Destruction
//////////////////////////////////////////////////////////////////////
CGTesseraQV302toSTDF::~CGTesseraQV302toSTDF()
{
	// Destroy list of Parameters tables.
	if(m_pTesseraQV302Parameter!=NULL)
		delete [] m_pTesseraQV302Parameter;
	// Destroy list of Bin tables.
	QMap<int,CGTesseraQV302Binning *>::Iterator it;
	for(it = m_mapTesseraQV302Binning.begin(); it!= m_mapTesseraQV302Binning.end(); it++)
		delete m_mapTesseraQV302Binning[it.key()];
	m_mapTesseraQV302Binning.clear();
}


//////////////////////////////////////////////////////////////////////
// Get Error
//////////////////////////////////////////////////////////////////////
QString CGTesseraQV302toSTDF::GetLastError()
{
	strLastError = "Import Tessera "+m_strTesterType+": ";

	switch(iLastError)
	{
		default:
		case errNoError:
			strLastError += "No Error";
			break;
		case errOpenFail:
			strLastError += "Failed to open file";
			break;
		case errInvalidFormat:
			strLastError += "Invalid file format";
			break;
		case errInvalidFormatLowInRows:
			strLastError += "Invalid file format: 'Parameter' line too short, missing rows";
			break;
		case errNoLimitsFound:
			strLastError += "Invalid file format: Specification Limits not found";
			break;
		case errWriteSTDF:
			strLastError += "Failed creating temporary file. Folder permission issue?";
            break;
		case errLicenceExpired:
			strLastError += "License has expired or Data file out of date...";
            break;
	}
	// Return Error Message
	return strLastError;
}

//////////////////////////////////////////////////////////////////////
// Load TesseraQV302 Parameter table from DISK
//////////////////////////////////////////////////////////////////////
void CGTesseraQV302toSTDF::LoadParameterIndexTable(void)
{
	QString	strTesseraQV302TableFile;
	QString	strString;

    strTesseraQV302TableFile  = GS::Gex::Engine::GetInstance().Get("UserFolder").toString();
	strTesseraQV302TableFile += GEX_TESSERA_PARAMETERS;

	// Open TesseraQV302 Parameter table file
    QFile f( strTesseraQV302TableFile );
    if(!f.open( QIODevice::ReadOnly ))
		return;

	// Assign file I/O stream
	QTextStream hTesseraQV302TableFile(&f);

	// Skip comment lines
	do
	{
	  strString = hTesseraQV302TableFile.readLine();
	}
    while((strString.indexOf("----------------------") < 0) && (hTesseraQV302TableFile.atEnd() == false));

	// Read lines
	m_pFullTesseraQV302ParametersList.clear();
	strString = hTesseraQV302TableFile.readLine();
	while (strString.isNull() == false)
	{
		// Save Parameter name in list
		m_pFullTesseraQV302ParametersList.append(strString);
		// Read next line
		strString = hTesseraQV302TableFile.readLine();
	};

	// Close file
	f.close();
}

//////////////////////////////////////////////////////////////////////
// Save TesseraQV302 Parameter table to DISK
//////////////////////////////////////////////////////////////////////
void CGTesseraQV302toSTDF::DumpParameterIndexTable(void)
{
	QString		strTesseraQV302TableFile;
	int			uIndex;

    strTesseraQV302TableFile  = GS::Gex::Engine::GetInstance().Get("UserFolder").toString();
	strTesseraQV302TableFile += GEX_TESSERA_PARAMETERS;

	// Open TesseraQV302 Parameter table file
    QFile f( strTesseraQV302TableFile );
    if(!f.open( QIODevice::WriteOnly ))
		return;

	// Assign file I/O stream
	QTextStream hTesseraQV302TableFile(&f);

	// First few lines are comments:
	hTesseraQV302TableFile << "############################################################" << endl;
	hTesseraQV302TableFile << "# DO NOT EDIT THIS FILE!" << endl;
    hTesseraQV302TableFile << "# Quantix Examinator: Tessera Parameters detected" << endl;
	hTesseraQV302TableFile << "# www.mentor.com" << endl;
    hTesseraQV302TableFile << "# Quantix Examinator reads and writes into this file..." << endl;
	hTesseraQV302TableFile << "-----------------------------------------------------------" << endl;

	// Write lines
	// m_pFullTesseraQV302ParametersList.sort();
	for(uIndex=0;uIndex<m_pFullTesseraQV302ParametersList.count();uIndex++)
	{
		// Write line
		hTesseraQV302TableFile << m_pFullTesseraQV302ParametersList[uIndex] << endl;
	};

	// Close file
	f.close();
}

//////////////////////////////////////////////////////////////////////
// If Examinator doesn't have this TesseraQV302 parameter in his dictionnary, have it added.
//////////////////////////////////////////////////////////////////////
void CGTesseraQV302toSTDF::UpdateParameterIndexTable(QString strParamName)
{
	// Check if the table is empty...if so, load it from disk first!
	if(m_pFullTesseraQV302ParametersList.isEmpty() == true)
	{
		// Load TesseraQV302 parameter table from disk...
		LoadParameterIndexTable();
	}

	// Check if Parameter name already in table...if not, add it to the list
	// the new full list will be dumped to the disk at the end.
    if(m_pFullTesseraQV302ParametersList.indexOf(strParamName) < 0)
	{
		// Update list
		m_pFullTesseraQV302ParametersList.append(strParamName);

		// Set flag to force the current TesseraQV302 table to be updated on disk
		m_bNewTesseraQV302ParameterFound = true;
	}
}


//////////////////////////////////////////////////////////////////////
// Check if File is compatible with TesseraQV302 format
//////////////////////////////////////////////////////////////////////
bool CGTesseraQV302toSTDF::IsCompatible(const char *szFileName)
{
	QString strString;
	QString strValue;
	QString	strSite;

	// Open hCsmFile file
    QFile f( szFileName );
    if(!f.open( QIODevice::ReadOnly ))
	{
		// Failed Opening file
		return false;
	}
	// Assign file I/O stream
	QTextStream hTesseraQV302File(&f);

	// Check if first line is the correct TesseraQV302 header...
	//RSE_LDR-EHANT-WC146-S-17954-22UV-200906120849-FULL-MPTS
	//Column,Row,Part ID,Wafer ID,# of pits,Pitted area um2,Pitted area %,Dia um,Roundness um

	// Skip the first line
	hTesseraQV302File.readLine();

	do
	{
		strString = hTesseraQV302File.readLine();

		// Skip all line started with ~
		if(strString.left(1) == "~")
			strString = "";

		// Skip RowOffset and RowMult def
		if((strString.indexOf("RowOffset",0,Qt::CaseInsensitive) >= 0) && (strString.indexOf("ColOffset",0,Qt::CaseInsensitive) >= 0))
			strString = "";
		if((strString.indexOf("RowMult",0,Qt::CaseInsensitive) >= 0) && (strString.indexOf("ColMult",0,Qt::CaseInsensitive) >= 0))
			strString = "";

		// Skip empty CVS line
		if(strString.left(3) == ",,,")
			strString = "";

	}
	while(!strString.isNull() && strString.isEmpty());

	// QV302
    if(strString.startsWith("Column,Row", Qt::CaseInsensitive) == false)
	{
		// Incorrect header...this is not a TesseraQV302 file!
		f.close();
		return false;
	}

	// Delete all "~" into the line
	strString = strString.remove("~");

    if(strString.startsWith("Column,Row,Pass,Bin", Qt::CaseInsensitive))
	{
		// It's a Tessera inspection
		// Incorrect header...this is not a TesseraQV302 file!
		f.close();
		return false;
	}

	f.close();

	return true;
}

//////////////////////////////////////////////////////////////////////
// Read and Parse the TesseraQV302 file
//////////////////////////////////////////////////////////////////////
bool CGTesseraQV302toSTDF::ReadTesseraQV302File(const char *TesseraQV302FileName, const char *strFileNameSTDF)
{
	QString strString;
	QString strSection;
	bool	bStatus;
	int		iIndex;				// Loop index

	// Open CSV file
    QFile f( TesseraQV302FileName );
    if(!f.open( QIODevice::ReadOnly ))
	{
		// Failed Opening TesseraQV302 file
		iLastError = errOpenFail;

		// Convertion failed.
		return false;
	}

 	//////////////////////////////////////////////////////////////////////
	// For ProgressBar
	iNextFilePos = 0;
	iProgressStep = 0;
	iFileSize = f.size() + 1;


	// Assign file I/O stream
	QTextStream hTesseraQV302File(&f);

	m_strWaferID = "1";
	m_strTesterType = "QV302";
	m_lStartTime = m_lStopTime = 0;

	// Tessera Naming convention
	//
	//X1-ppenilla-diffrac2-S-18538-12UV-200906121554-Sample
	strString = ReadLine(hTesseraQV302File);
	m_strJobName = strString.section(',',0,0).simplified();
	m_strSetupId = "LensSettings="+m_strJobName;
	if(strString.count("-") >= 7)
	{
		QDate	clDate;
		QTime	clTime;
		// Good naming convention
		// extract info
		strString = strString.remove(',');
		m_strProductID = strString.section("-",0,0);
		m_strOperatorID = strString.section("-",1,1);
		m_strTesterID = strString.section("-",2,2);
		m_strWaferID = strString.section("-",4,5);
		m_strJobRev = strString.section("-",7,7);

		//Date Time:200906121554
		QString	strDate = strString.section("-",6,6);
		clDate = QDate(strDate.left(4).toInt(),strDate.mid(4,2).toInt(),strDate.mid(6,2).toInt());
		clTime.setHMS(strDate.mid(8,2).toInt(), strDate.right(2).toInt(), 0);


		QDateTime clDateTime(clDate,clTime);
		clDateTime.setTimeSpec(Qt::UTC);
		m_lStartTime = clDateTime.toTime_t();
	}

	if(strString.startsWith("TALY",Qt::CaseInsensitive))
		m_strTesterType = "TalySurf";

	//~,Company: ,Tessera North America ,  , Machine ID: QV302  WC146
	//~,Jupiter Sinope Assy Program, SINOPE_ASSY.QVB,, Revision 3
	//~,Date Time: ,12/13/2007 7:25:58 PM
	//~,Local Result filename: ,C:\DOC_QVPak\Results\SNPASSY-CKingsley-WC146-U-STX-910-200712131925-FULL-MPTS.txt
	//~,Network Result filename: ,\\Yogi\tester\QV302\SNPASSY-CKingsley-WC146-U-STX-910-200712131925-FULL-MPTS.txt
	//~,Operator: ,CKingsley
	//~,Batch # ,STX-910, ,Comment: ,Jupiter Fixture #123
	//~, *** All measurement values in microns or degrees. ***
	//~,Prod ID, Redo#, Part ID

	QDate	clDate;
	QTime	clTime;

	strString = ReadLine(hTesseraQV302File);

    if(strString.startsWith("Column,Row,Pass", Qt::CaseInsensitive))
		m_strTesterType = "ICOS";

	// Read Tessera information
	while(!hTesseraQV302File.atEnd())
	{
		if(strString.isEmpty())
			strString = ReadLine(hTesseraQV302File);

        if(strString.startsWith("Column,Row", Qt::CaseInsensitive))
		{
			// Data
			break;
		}

		// Delete "~," or "~"
		strString = strString.remove(QRegExp("~,?"));

		// Delete '"'
		strString = strString.remove('"');

		// Remove all "," just after a ":"
		strString = strString.replace(QRegExp(":\\s*,"),":");

		// Remove all "," just after a ","
		strString = strString.replace(QRegExp(",\\s*,"),",");

		while(strString.startsWith(","))
		{
			strString = strString.section(",",1);
		}

		// Check if ':' sep or ',' sep
		if(strString.count(":") > 0)
		{
			strSection = strString.section(":",0,0).simplified();
			strString = strString.section(":",1).simplified();
		}
		else
		{
			strSection = strString.section(",",0,0).simplified();
			strString = strString.section(",",1).simplified();
		}

		if((strSection.toLower() == "rowoffset")
			 || (strSection.toLower() == "coloffset")
			 || (strSection.toLower() == "rowmult")
			 || (strSection.toLower() == "colmult"))
		{
			//"RowOffset:",0,"ColOffset:",0
			//"RowMult:",1,"ColMult:",1

			m_strTesterType = "ET_Moe";
		}

		if(strSection.toLower() == "company")
		{
			//Revision: 001
        }
		else if(strSection.toLower() == "lot_#")
		{
			//LOT_#:14878-116AF
			m_strLotID = strString.section(",",0,0);
		}
		else if(strSection.startsWith("batch",Qt::CaseInsensitive))
		{
			//Batch #:14878-116AF
			m_strLotID = strString.section(",",0,0);
		}
		else if(strSection.toLower() == "device")
		{
			//DEVICE:CADENCE
			m_strProductID = strString.section(",",0,0);
		}
		else if(strSection.toLower() == "level")
		{
			//LEVEL:PE_E1E2
			if(!m_strProductID.isEmpty())
				m_strProductID += "_";
			m_strProductID += strString.section(",",0,0);
		}
		else if(strSection.startsWith("product",Qt::CaseInsensitive))
		{
			//Product Name,MDOC00755_A_KIN045
			m_strProductID = strString.section(",",0,0);
		}
		else if(strSection.toLower() == "machine id")
		{
			//Machine ID: QV302  WC146
			m_strTesterID = strString.section(",",0,0);
		}
		else if(strSection.toLower() == "system")
		{
			//SYSTEM: QV302  WC146
			m_strTesterID = strString.section(",",0,0);
		}
		else if(strSection.startsWith("wafer",Qt::CaseInsensitive))
		{
			//Wafer: 17954-22UV
			m_strWaferID = strString.section(",",0,0);
		}
		else if(strSection.toLower().startsWith("user",Qt::CaseInsensitive))
		{
			//User: EH
			m_strOperatorID = strString.section(",",0,0);
		}
		else if(strSection.toLower() == "operator")
		{
			//Operator: CKingsley
			m_strOperatorID = strString.section(",",0,0);
		}
		else if(strSection.toLower() == "date time")
		{
			//Date Time:12/13/2007 7:25:58 PM
			QString	strDate = strString.section(" ",0,0);
			clDate = QDate(strDate.section("/",2,2).toInt(),strDate.section("/",0,0).toInt(),strDate.section("/",1,1).toInt());

			QString strTime = strString.section(" ",1,1).section(" ",0,0);
			int nHour = strTime.section(":",0,0).toInt();
			if((strString.section(" ",2,2).toUpper() == "PM")
			&& (nHour < 12))
				nHour += 12;
			if(nHour == 24)
				nHour = 0;
			clTime.setHMS(nHour, strTime.section(":",1,1).toInt(), strTime.section(":",2,2).toInt());
		}
		else if(strSection.toLower() == "date")
		{
			//Date:12/13/2007
			if(strString.contains("/"))
			{
				QString	strDate = strString.section(" ",0,0);
				clDate = QDate(strDate.section("/",2,2).toInt(),strDate.section("/",0,0).toInt(),strDate.section("/",1,1).toInt());
			}
			else
			//DATE:Fri Jan 30 2009 13:9:52
			if(strString.count(" ") > 3)
			{
				QString	strDate = strString.section(" ",0,strString.count(" ")-1);
				clDate = QDate::fromString(strDate.section(" ",strDate.count(" ")-2), "mmm dd yyyy");

				QString strTime = strString.section(" ",strString.count(" "));
				clTime.setHMS(strTime.section(":",0,0).toInt(), strTime.section(":",1,1).toInt(), strTime.section(":",2,2).toInt());
			}
			else
			//DATE:WedAug8:9 17:51
			{
				QString	strDate = strString.section(" ",0,0);
				clDate = QDate::fromString(strDate.section(" ",strDate.count(" ")-2), "dddmmmd:y");

				QString strTime = strString.section(" ",1);
				clTime.setHMS(strTime.section(":",0,0).toInt(), strTime.section(":",1,1).toInt(), strTime.section(":",2,2).toInt());
			}
		}
		else if(strSection.toLower() == "time")
		{
			//Time:7:25:58
			QString strTime = strString.section(" ",0,0);
			int nHour = strTime.section(":",0,0).toInt();
			if((strString.section(" ",1,1).toUpper() == "PM")
			&& (nHour < 12))
				nHour += 12;
			if(nHour == 24)
				nHour = 0;
			clTime.setHMS(nHour, strTime.section(":",1,1).toInt(), strTime.section(":",2,2).toInt());
		}
		else if(strSection.toLower() == "revision")
		{
			//Revision: 001
			m_strSoftRev = strString.section(",",0,0);
		}
		else if(strSection.toLower() == "program")
		{
			//Program:  Io_TXS_Reflow_10X.voy
			m_strProgramID = strString.section(",",0,0);
		}
		else if(strSection.startsWith("job",Qt::CaseInsensitive))
		{
			//JOB_PLAN:1_NZREG
			m_strProgramID = strString.section(",",0,0);
		}
		else if(strSection.toLower().indexOf("result file") >= 0
			||  strSection.toLower().indexOf("network file") >= 0)
		{
			//ignore
		}
		else if(!strSection.isEmpty())
		{
			if((m_strSetupId.length()+strSection.length()+strString.section(",",0,0).length()) < 255)
			{
				if(!strString.section(",",0,0).simplified().isEmpty())
				{
					// Setup Configuration
					if(!m_strSetupId.isEmpty())
						m_strSetupId += ";";
					m_strSetupId += strSection.remove(' ')+"="+strString.section(",",0,0).simplified();
					if(strSection.startsWith("Wavelength"))
					{
						strString = strString.section(",",1);
						m_strSetupId += strString.section(",",0,0).simplified();
					}
			}
			}
		}
		else
			strString = strString.section(",",1);

		strString = strString.section(",",1);

	}

	// Check if have enought info to determine if it is a QV302, a TalySurf or a Voyager
	if(m_strProgramID.endsWith(".voy",Qt::CaseInsensitive))
		m_strTesterType = "Voyager";

	if(m_strTesterID.startsWith("ACVR",Qt::CaseInsensitive))
		m_strTesterType = "IVS";

	QDateTime clDateTime(clDate,clTime);
	clDateTime.setTimeSpec(Qt::UTC);
	m_lStartTime = clDateTime.toTime_t();

	//Column,Row,Part ID,Wafer ID,# of pits
	if(strString.startsWith("Column,Row,Part ID,Wafer ID",Qt::CaseInsensitive))
	{
		m_iIndexOffset = 4;
	}
	else
	//Column,Row,Part ID,CTR_R(um)
	if(strString.startsWith("Column,Row,Part ID",Qt::CaseInsensitive))
	{
		m_iIndexOffset = 3;
	}
	else
	//Column,Row,DIA(um)
	if(strString.startsWith("Column,Row",Qt::CaseInsensitive))
	{
		m_iIndexOffset = 2;
	}
	else
	{
		// Incorrect header...this is not a TesseraQV302 file!
		f.close();
		return false;
	}

    QStringList lstSections = strString.split(",",QString::KeepEmptyParts);

	// Count the number of parameters specified in the line
	m_iTotalParameters=lstSections.count();
	// If no parameter specified...ignore!
	if(m_iTotalParameters <= 0)
	{
		// Incorrect header...this is not a valid TesseraQV302 file!
		iLastError = errInvalidFormat;

		// Convertion failed.
		f.close();
		return false;
	}

	// Allocate the buffer to hold the N parameters & results.
	m_pTesseraQV302Parameter = new CGTesseraQV302Parameter[m_iTotalParameters];	// List of parameters

	// Extract the N column names
	// Do not count first m_iIndexOffset fields
	QString strUnit;
	int nScale;
	// Check if have Pass/Fail flag
	m_nPassFailIndex = -1;
	m_nSiteIndex = -1;

	for(iIndex=m_iIndexOffset;iIndex<m_iTotalParameters;iIndex++)
	{
		strSection = lstSections[iIndex];
		strUnit = "";
		nScale = 0;

		if(strSection.isEmpty())
			break;

		// Delete "~"
		strSection = strSection.remove("~");

		// Delete '"'
		strSection = strSection.remove('"');

		if((strSection.toLower() == "pass") || (strSection.toLower() == "pass/fail"))
			m_nPassFailIndex = iIndex;

		if(strSection.toLower() == "na")
			m_nSiteIndex = iIndex;

		// Check if have some unit
		if(strSection.indexOf("(") > 0)
		{
			// unit
			strUnit = strSection.section("(",1).remove(")");
			NormalizeLimits(strUnit, nScale);
			strSection = strSection.section("(",0,0).simplified();
		}
		else
		if((strSection.count(" ")>0)
		&& (strSection.section(" ",strSection.count(" ")).length() < 4))
		{
			// unit
			strUnit = strSection.section(" ",strSection.count(" "));
			NormalizeLimits(strUnit, nScale);
		}

		UpdateParameterIndexTable(strSection);		// Update Parameter master list if needed.

		m_pTesseraQV302Parameter[iIndex].nNumber = m_pFullTesseraQV302ParametersList.indexOf(strSection);
		m_pTesseraQV302Parameter[iIndex].strName = strSection;
		m_pTesseraQV302Parameter[iIndex].strUnits = strUnit;
		m_pTesseraQV302Parameter[iIndex].nScale = nScale;
		m_pTesseraQV302Parameter[iIndex].bValidHighLimit = false;
		m_pTesseraQV302Parameter[iIndex].bValidLowLimit = false;
		m_pTesseraQV302Parameter[iIndex].fHighLimit = 0;
		m_pTesseraQV302Parameter[iIndex].fLowLimit = 0;
		m_pTesseraQV302Parameter[iIndex].bStaticHeaderWritten = false;
	}

	m_iTotalParameters = iIndex;


	// Loop reading file until end is reached & generate STDF file dynamically.
	bStatus = WriteStdfFile(&hTesseraQV302File,strFileNameSTDF);
	if(!bStatus)
		QFile::remove(strFileNameSTDF);

	// All TesseraQV302 file read...check if need to update the TesseraQV302 Parameter list on disk?
	if(bStatus && (m_bNewTesseraQV302ParameterFound == true))
		DumpParameterIndexTable();

	// Success parsing TesseraQV302 file
	f.close();
	return bStatus;
}

//////////////////////////////////////////////////////////////////////
// Create STDF file from TesseraQV302 data parsed
//////////////////////////////////////////////////////////////////////
bool CGTesseraQV302toSTDF::WriteStdfFile(QTextStream *hTesseraQV302File, const char *strFileNameSTDF)
{
	// now generate the STDF file...
    GS::StdLib::Stdf StdfFile;
    GS::StdLib::StdfRecordReadInfo RecordReadInfo;
    if(StdfFile.Open((char*)strFileNameSTDF,STDF_WRITE) != GS::StdLib::Stdf::NoError)
	{
		// Failed importing CSV file into STDF database
		iLastError = errWriteSTDF;

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
	StdfFile.WriteString(m_strTesterID.toLatin1().constData());	// Node name
	StdfFile.WriteString(m_strTesterType.toLatin1().constData());	// Tester Type
	StdfFile.WriteString(m_strJobName.toLatin1().constData());	// Job name
	StdfFile.WriteString(m_strJobRev.toLatin1().constData());		// Job rev
	StdfFile.WriteString("");					// sublot-id
	StdfFile.WriteString(m_strOperatorID.toLatin1().constData());	// operator
	StdfFile.WriteString("");					// exec-type
	StdfFile.WriteString(m_strSoftRev.toLatin1().constData());	// exe-ver
	StdfFile.WriteString("WAFER");				// test-cod
	StdfFile.WriteString("");					// test-temperature
    // Construct custom Galaxy USER_TXT
	QString	strUserTxt;
    strUserTxt = GEX_IMPORT_DATAORIGIN_LABEL;
	strUserTxt += ":";
	strUserTxt += GEX_IMPORT_DATAORIGIN_ATETEST;
	strUserTxt += ":TESSERA_"+m_strTesterType.toUpper();
	StdfFile.WriteString((char *)strUserTxt.toLatin1().constData());	// user-txt
	StdfFile.WriteString((char *)m_strProgramID.toLatin1().constData());	// aux-file
	StdfFile.WriteString("");							// package-type
	StdfFile.WriteString(m_strProductID.toLatin1().constData());	// familyID
	StdfFile.WriteString("");							// Date-code
	StdfFile.WriteString("");							// Facility-ID
	StdfFile.WriteString("");							// FloorID
	StdfFile.WriteString("");							// ProcessID
	StdfFile.WriteString("");							// OperationFreq
	StdfFile.WriteString("");							// Spec-nam
	StdfFile.WriteString("");							// Spec-ver
	StdfFile.WriteString("");							// Flow-id
	StdfFile.WriteString((char *)m_strSetupId.left(255).toLatin1().constData());	// setup_id
	StdfFile.WriteRecord();

	// Write Test results for each line read.
	QString strString;
	QString strSection;
	float	fValue;				// Used for readng floating point numbers.
	int		iIndex;				// Loop index
	int		iXpos, iYpos;
	int		iBin;
	int		iSite;
	bool	bPassStatus;
	long		iTotalGoodBin,iTotalFailBin;
	long		iTotalTests,iPartNumber;
	QString		strPartId;
	bool		bStatus;
	QStringList	lstSections;
	BYTE		bData;
	CGTesseraQV302Binning* pBinning;

	bool bWriteWir;

	// Reset counters
	bWriteWir = true;
	iTotalGoodBin=iTotalFailBin=0;
	iPartNumber=0;
	iSite = 1;

	// Write all Parameters read on this wafer.: WIR.PIR,PTR....PTR, PRR, PIR, PTR,PTR...PRR,   ... WRR

	//Column,Row,Part ID,CTR_R(um),CTR_X(um),CTR_Y(um),DIE_XZ_Angle(deg),DIE_YZ_ANGLE(deg),DIE_ROT_ANGLE(deg),DIE_VCSEL_Z(um),LOAD_DIFF(um),FIXTURE,REV,NOTES
	//-1,0,STX-SPC,7.302,-3.602,-6.352,0.011,-.197,0.164,87.585,-4.8,123,3,
	//-1,1,STX-SPC,8.040,-4.549,-6.629,0.006,-.211,0.167,87.977,-8.7,123,3,
	while(hTesseraQV302File->atEnd() == false)
	{

		// Part number
		iPartNumber++;

		// Read line
		strString = ReadLine(*hTesseraQV302File);

		if(strString.startsWith("~"))
			break;

        lstSections = strString.split(",",QString::KeepEmptyParts);

		// Check if it's not an empty line : only "," without value
		if(strString.remove(",").remove(" ").isEmpty())
			continue;

		// Check if have the good count
		if(lstSections.count() < m_iTotalParameters)
		{
			bStatus = false;
			if(lstSections.count() > 0)
				lstSections[0].toInt(&bStatus);
			if(!bStatus)
				break;

			iLastError = errInvalidFormatLowInRows;

			StdfFile.Close();
			// Convertion failed.
			return false;
		}

		// Extract Column,Row,Pass
		iXpos = lstSections[0].toInt();
		iYpos = lstSections[1].toInt();

		if(m_iIndexOffset > 2)
			strPartId = lstSections[2];
		else
			strPartId = QString::number(iPartNumber);

		if(m_iIndexOffset > 3)
			m_strWaferID = lstSections[3];

		// Check if have PassFail info
		if(m_nPassFailIndex > 0)
		{
			iBin = lstSections[m_nPassFailIndex].toInt(&bStatus);
			if(!bStatus)
				iBin = (lstSections[m_nPassFailIndex].toLower() == "pass");
		}
		else
			iBin = 1;

		if(!m_mapTesseraQV302Binning.contains(iBin))
		{
			pBinning = new CGTesseraQV302Binning();

			pBinning->nNumber = iBin;
			pBinning->strName = "";
			pBinning->bPass = (iBin == 1);
			pBinning->nCount = 0;

			m_mapTesseraQV302Binning[iBin] = pBinning;
		}

		pBinning = m_mapTesseraQV302Binning[iBin];

		bPassStatus = pBinning->bPass;
		pBinning->nCount++;

		// Check if have Site info
		iSite = 1;
		if(m_nSiteIndex > 0)
		{
			iSite = lstSections[m_nSiteIndex].toLower().remove("s").toInt();

			// For the first line, check if it's effectively Site info
			if(iPartNumber == 1)
			{
				if(!lstSections[m_nSiteIndex].startsWith("s",Qt::CaseInsensitive))
				{
					m_nSiteIndex = -1;
					iSite = 1;
				}
			}
		}


		if(bWriteWir)
		{
			// Write WIR of new Wafer.
			RecordReadInfo.iRecordType = 2;
			RecordReadInfo.iRecordSubType = 10;
			StdfFile.WriteHeader(&RecordReadInfo);
			StdfFile.WriteByte(1);								// Test head
			StdfFile.WriteByte(255);							// Tester site (all)
			StdfFile.WriteDword(m_lStartTime);					// Start time
			StdfFile.WriteString(m_strWaferID.toLatin1().constData());	// WaferID
			StdfFile.WriteRecord();

			bWriteWir = false;
		}

		// Reset counters
		iTotalTests = 0;

		// Write PIR for parts in this Wafer site
		RecordReadInfo.iRecordType = 5;
		RecordReadInfo.iRecordSubType = 10;
		StdfFile.WriteHeader(&RecordReadInfo);
		StdfFile.WriteByte(1);					// Test head
		StdfFile.WriteByte(iSite);				// Tester site
		StdfFile.WriteRecord();

		// Read Parameter results for this record
		for(iIndex=m_iIndexOffset;iIndex<m_iTotalParameters;iIndex++)
		{
			if(m_nPassFailIndex == iIndex)
				continue;

			if(m_nSiteIndex == iIndex)
				continue;

			strSection = lstSections[iIndex];
			fValue = strSection.toFloat(&bStatus);
			if(bStatus == true)
			{
				// Valid test result...write the PTR
				iTotalTests++;

				RecordReadInfo.iRecordType = 15;
				RecordReadInfo.iRecordSubType = 10;
				StdfFile.WriteHeader(&RecordReadInfo);
				// Compute Test# (add user-defined offset)
				StdfFile.WriteDword(m_pTesseraQV302Parameter[iIndex].nNumber + GEX_TESTNBR_OFFSET_TESSERA);			// Test Number
				StdfFile.WriteByte(1);				// Test head
				StdfFile.WriteByte(iSite);			// Tester site#

				if(!m_pTesseraQV302Parameter[iIndex].bValidLowLimit && !m_pTesseraQV302Parameter[iIndex].bValidHighLimit)
				{
					// No pass/fail information
					bData = 0x40;
				}
				else if(((m_pTesseraQV302Parameter[iIndex].bValidLowLimit==true) && (fValue < m_pTesseraQV302Parameter[iIndex].fLowLimit)) ||
				   ((m_pTesseraQV302Parameter[iIndex].bValidHighLimit==true) && (fValue > m_pTesseraQV302Parameter[iIndex].fHighLimit)))
				{
					bData = 0200;	// Test Failed
				}
				else
				{
					bData = 0;		// Test passed
				}
				StdfFile.WriteByte(bData);							// TEST_FLG
				StdfFile.WriteByte(0x40|0x80);						// PARAM_FLG
				StdfFile.WriteFloat(fValue * GS_POW(10.0,m_pTesseraQV302Parameter[iIndex].nScale));						// Test result
				if(m_pTesseraQV302Parameter[iIndex].bStaticHeaderWritten == false)
				{
					StdfFile.WriteString(m_pTesseraQV302Parameter[iIndex].strName.toLatin1().constData());	// TEST_TXT
					StdfFile.WriteString("");							// ALARM_ID
					bData = 2;	// Valid data.
					if(m_pTesseraQV302Parameter[iIndex].bValidLowLimit==false)
						bData |=0x40;
					if(m_pTesseraQV302Parameter[iIndex].bValidHighLimit==false)
						bData |=0x80;
					StdfFile.WriteByte(bData);							// OPT_FLAG
					StdfFile.WriteByte(-m_pTesseraQV302Parameter[iIndex].nScale);			// RES_SCALE
					StdfFile.WriteByte(-m_pTesseraQV302Parameter[iIndex].nScale);			// LLM_SCALE
					StdfFile.WriteByte(-m_pTesseraQV302Parameter[iIndex].nScale);			// HLM_SCALE
					StdfFile.WriteFloat(m_pTesseraQV302Parameter[iIndex].fLowLimit);		// LOW Limit
					StdfFile.WriteFloat(m_pTesseraQV302Parameter[iIndex].fHighLimit);		// HIGH Limit
					StdfFile.WriteString(m_pTesseraQV302Parameter[iIndex].strUnits.toLatin1().constData());	// Units
					m_pTesseraQV302Parameter[iIndex].bStaticHeaderWritten = true;
				}
				StdfFile.WriteRecord();
			}	// Valid test result
			else
			{
				if(m_pTesseraQV302Parameter[iIndex].bStaticHeaderWritten == false)
				{
					RecordReadInfo.iRecordType = 15;
					RecordReadInfo.iRecordSubType = 10;
					StdfFile.WriteHeader(&RecordReadInfo);
					// Compute Test# (add user-defined offset)
					StdfFile.WriteDword(m_pTesseraQV302Parameter[iIndex].nNumber + GEX_TESTNBR_OFFSET_TESSERA);			// Test Number
					StdfFile.WriteByte(1);						// Test head
					StdfFile.WriteByte(iSite);					// Tester site#

					// No pass/fail information
					bData = 0x52;
					StdfFile.WriteByte(bData);							// TEST_FLG
					StdfFile.WriteByte(0x40|0x80);						// PARAM_FLG
					StdfFile.WriteFloat(0);								// Test result
					StdfFile.WriteString(m_pTesseraQV302Parameter[iIndex].strName.toLatin1().constData());	// TEST_TXT
					StdfFile.WriteString("");							// ALARM_ID
					bData = 2;	// Valid data.
					if(m_pTesseraQV302Parameter[iIndex].bValidLowLimit==false)
						bData |=0x40;
					if(m_pTesseraQV302Parameter[iIndex].bValidHighLimit==false)
						bData |=0x80;
					StdfFile.WriteByte(bData);							// OPT_FLAG
					StdfFile.WriteByte(-m_pTesseraQV302Parameter[iIndex].nScale);			// RES_SCALE
					StdfFile.WriteByte(-m_pTesseraQV302Parameter[iIndex].nScale);			// LLM_SCALE
					StdfFile.WriteByte(-m_pTesseraQV302Parameter[iIndex].nScale);			// HLM_SCALE
					StdfFile.WriteFloat(m_pTesseraQV302Parameter[iIndex].fLowLimit);		// LOW Limit
					StdfFile.WriteFloat(m_pTesseraQV302Parameter[iIndex].fHighLimit);		// HIGH Limit
					StdfFile.WriteString(m_pTesseraQV302Parameter[iIndex].strUnits.toLatin1().constData());	// Units
					m_pTesseraQV302Parameter[iIndex].bStaticHeaderWritten = true;
					StdfFile.WriteRecord();

				}
			}
		}		// Read all results on line

		// Write PRR
		RecordReadInfo.iRecordType = 5;
		RecordReadInfo.iRecordSubType = 20;
		StdfFile.WriteHeader(&RecordReadInfo);
		StdfFile.WriteByte(1);			// Test head
		StdfFile.WriteByte(iSite);		// Tester site#:1
		if(bPassStatus == true)
		{
			StdfFile.WriteByte(0);				// PART_FLG : PASSED
			iTotalGoodBin++;
		}
		else
		{
			StdfFile.WriteByte(8);				// PART_FLG : FAILED
			iTotalFailBin++;
		}
		StdfFile.WriteWord((WORD)iTotalTests);	// NUM_TEST
		StdfFile.WriteWord(iBin);				// HARD_BIN
		StdfFile.WriteWord(iBin);				// SOFT_BIN
		StdfFile.WriteWord(iXpos);				// X_COORD
		StdfFile.WriteWord(iYpos);				// Y_COORD
		StdfFile.WriteDword(0);					// No testing time known...
		StdfFile.WriteString(strPartId.toLatin1().constData());		// PART_ID
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
	StdfFile.WriteDword(m_lStopTime);			// Time of last part tested
	StdfFile.WriteDword(iTotalGoodBin+iTotalFailBin);	// Parts tested
	StdfFile.WriteDword(0);						// Parts retested
	StdfFile.WriteDword(0);						// Parts Aborted
	StdfFile.WriteDword(iTotalGoodBin);			// Good Parts
	StdfFile.WriteDword((DWORD)-1);				// Functionnal Parts
	StdfFile.WriteString(m_strWaferID.toLatin1().constData());	// WaferID
	StdfFile.WriteString("");					// FabId
	StdfFile.WriteString("");					// FrameId
	StdfFile.WriteString("");					// MaskId
	StdfFile.WriteString(QString("SIDE["+m_strWaferID+"]").toLatin1().constData());	// UserDesc
	StdfFile.WriteRecord();


	QMap<int,CGTesseraQV302Binning *>::Iterator it;

	RecordReadInfo.iRecordType = 1;
	RecordReadInfo.iRecordSubType = 40;
	for(it = m_mapTesseraQV302Binning.begin(); it != m_mapTesseraQV302Binning.end(); it++)
	{
		pBinning = *it;

		// Write HBR
		StdfFile.WriteHeader(&RecordReadInfo);
		StdfFile.WriteByte(255);						// Test Head = ALL
        StdfFile.WriteByte(255);                        // Test sites = ALL
		StdfFile.WriteWord(pBinning->nNumber);			// HBIN
		StdfFile.WriteDword(pBinning->nCount);			// Total Bins
		if(pBinning->bPass)
			StdfFile.WriteByte('P');
		else
			StdfFile.WriteByte('F');
		StdfFile.WriteString( pBinning->strName.toLatin1().constData());
		StdfFile.WriteRecord();
	}

	RecordReadInfo.iRecordType = 1;
	RecordReadInfo.iRecordSubType = 50;
	for(it = m_mapTesseraQV302Binning.begin(); it != m_mapTesseraQV302Binning.end(); it++)
	{
		pBinning = *it;

		// Write SBR
		StdfFile.WriteHeader(&RecordReadInfo);
		StdfFile.WriteByte(255);						// Test Head = ALL
        StdfFile.WriteByte(255);                        // Test sites = ALL
		StdfFile.WriteWord(pBinning->nNumber);			// HBIN
		StdfFile.WriteDword(pBinning->nCount);			// Total Bins
		if(pBinning->bPass)
			StdfFile.WriteByte('P');
		else
			StdfFile.WriteByte('F');
		StdfFile.WriteString( pBinning->strName.toLatin1().constData());
		StdfFile.WriteRecord();
	}


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
// Convert 'FileName' TesseraQV302 file, to STDF 'strFileNameSTDF' file
//////////////////////////////////////////////////////////////////////
bool CGTesseraQV302toSTDF::Convert(const char *TesseraQV302FileName, const char *strFileNameSTDF)
{
	// No erro (default)
	iLastError = errNoError;

	// If STDF file already exists...do not rebuild it...unless dates not matching!
	QFileInfo fInput(TesseraQV302FileName);
	QFileInfo fOutput(strFileNameSTDF);

    QFile f( strFileNameSTDF );
    if((f.exists() == true) && (fInput.lastModified() < fOutput.lastModified()))
		return true;

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
			GS::Gex::Engine::GetInstance().UpdateLabelStatus("Converting data from file "+QFileInfo(TesseraQV302FileName).fileName()+"...");
			GexScriptStatusLabel->show();
		}
		GS::Gex::Engine::GetInstance().UpdateLabelStatus("Converting data from file "+QFileInfo(TesseraQV302FileName).fileName()+"...");
		GexScriptStatusLabel->show();
	}
    QCoreApplication::processEvents();

    if(ReadTesseraQV302File(TesseraQV302FileName,strFileNameSTDF) != true)
	{
		//////////////////////////////////////////////////////////////////////
		// For ProgressBar
		if((GexProgressBar != NULL)
		&& bHideProgressAfter)
			GexProgressBar->hide();

		if((GexScriptStatusLabel != NULL)
		&& bHideLabelAfter)
			GexScriptStatusLabel->hide();
		return false;	// Error reading TesseraQV302 file
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
// Normalize test limits when writing into PTR.
//////////////////////////////////////////////////////////////////////
void CGTesseraQV302toSTDF::NormalizeLimits(QString &strUnit, int &nScale)
{
	nScale = 0;
	if(strUnit.length() <= 1)
	{
		// units too short to include a prefix, then keep it 'as-is'
		return;
	}

	QChar cPrefix = strUnit[0];
	switch(cPrefix.toLatin1())
	{
		case 'm': // Milli
			nScale = -3;
			break;
		case 'u': // Micro
			nScale = -6;
			break;
		case 'n': // Nano
			nScale = -9;
			break;
		case 'p': // Pico
			nScale = -12;
			break;
		case 'f': // Fento
			nScale = -15;
			break;
		case 'K': // Kilo
			nScale = 3;
			break;
		case 'M': // Mega
			nScale = 6;
			break;
		case 'G': // Giga
			nScale = 9;
			break;
		case 'T': // Tera
			nScale = 12;
			break;
	}
	if(nScale)
		strUnit = strUnit.mid(1);	// Take all characters after the prefix.
}

//////////////////////////////////////////////////////////////////////
// Read line : skip empty line
//////////////////////////////////////////////////////////////////////
QString CGTesseraQV302toSTDF::ReadLine(QTextStream& hFile)
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
