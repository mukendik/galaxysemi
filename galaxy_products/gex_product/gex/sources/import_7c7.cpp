//////////////////////////////////////////////////////////////////////
// import_7c7.cpp: Convert a 7C7 (.xml) file to STDF V4.0
//////////////////////////////////////////////////////////////////////
#include "import_7c7.h"
#include "import_constants.h"
#include "engine.h"
#include "gqtl_global.h"

#include <qmath.h>
#include <qprogressbar.h>
#include <QApplication>
#include <qlabel.h>
#include <qfileinfo.h>

// main.cpp
extern QLabel			*GexScriptStatusLabel;	// Handle to script status text in status bar
extern QProgressBar		*GexProgressBar;		// Handle to progress bar in status bar

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
C7C7toSTDF::C7C7toSTDF()
{
	m_pf7C7		= NULL;
	m_ph7C7File = NULL;
	m_pStdfFile = NULL;
	m_strCurrentLine= "";
	m_bEndOfFile	= false;


	m_lSetupT		= 0;
	m_lStartT		= 0;
	m_lEndT			= 0;
	m_nStatNum		= 1;
	m_cModeCod		= 'P';
	m_cRtstCod		= ' ';
	m_cProtCod		= ' ';
	m_lBurnTim		= 65535;
	m_cCModCod		= ' ';
	m_strLotId		= "";
	m_strPartTyp	= "";
	m_strNodeNam	= "";
	m_strEquipmentID	= "";
	m_strTestLevel	= "";
	m_strJobNam		= "";
	m_strJobRev		= "";
	m_strSbLotId	= "";
	m_strOperNam	= "";
	m_strExecTyp	= "";
	m_strExecVer	= "";
	m_strTestCod	= "";
	m_strTstTemp	= "";
	m_strAuxFile	= "";
	m_strPckTyp		= "";
	m_strFamlyId	= "";
	m_strDateCod	= "";
	m_strFacilId	= "";
	m_strFloorId	= "";
	m_strProcId		= "";
	m_strOperFrq	= "";
	m_strSpecNam	= "";
	m_strSpecVer	= "";
	m_strFlowId		= "";
	m_strSetupId	= "";;
	m_strDsgnRev	= "";
	m_strEngId		= "";
	m_strRomCod		= "";
	m_strSerlNum	= "";
	m_strSuprNam	= "";
	m_strDataOrigin = GEX_IMPORT_DATAORIGIN_ATETEST;


	m_nTotalWafers  = 0;

	m_strWaferId	= "";
	m_nWaferNumDie		= 0;
	m_nWaferNumPassed	= 0;
	m_nWaferNumRetested = 0;

	m_nTotalParts	= -1;
	m_nTotalPassed	= -1;
	m_nTotalRetested= -1;

	m_strUserDesc	= " ";
	m_strExecDesc	= " ";
	m_nDispCode		= 0;

	m_lLastPartTime = 0;
	
	m_bMirWriten	= false;
}

//////////////////////////////////////////////////////////////////////
C7C7toSTDF::~C7C7toSTDF()
{
	m_cl7C7DutResult.reset();
	m_cl7C7Summary.reset();
	delete m_cl7C7Summary.pSite; m_cl7C7Summary.pSite=0;
	m_cl7C7PartCnt.reset();
	m_cl7C7TestResult.reset();
    QMap<int,C7C7PinChannel*>::Iterator itPinChannel;
	for ( itPinChannel = m_clPinChannel.begin(); itPinChannel != m_clPinChannel.end(); ++itPinChannel ) 
	{
        delete itPinChannel.value();
	}
	m_clPinChannel.clear();
    QMap<int,C7C7Test*>::Iterator itTest;
	for ( itTest = m_clTest.begin(); itTest != m_clTest.end(); ++itTest ) 
	{
        delete itTest.value();
	}
	m_clTest.clear();
    QMap<int,C7C7Sbr*>::Iterator itSbrList;
	for ( itSbrList = m_clSbrList.begin(); itSbrList != m_clSbrList.end(); ++itSbrList ) 
	{
        delete itSbrList.value();
	}
	m_clSbrList.clear();
	m_clVarValue.clear();


}

//////////////////////////////////////////////////////////////////////
// Get Error
//////////////////////////////////////////////////////////////////////
QString C7C7toSTDF::GetLastError()
{
	m_strLastError = "Import 7C7: ";

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
// Load 7C7 Parameter table from DISK
//////////////////////////////////////////////////////////////////////
void C7C7toSTDF::LoadParameterIndexTable(void)
{
	QString	strFileName;
	QString	strString;

    strFileName  = GS::Gex::Engine::GetInstance().Get("UserFolder").toString();
	strFileName += GEX_7C7_PARAMETERS;

	// Open 7C7 Parameter table file
    QFile f( strFileName );
    if(!f.open( QIODevice::ReadOnly ))
		return;

	// Assign file I/O stream
	QTextStream h7C7File(&f);

	// Skip comment lines
	do
	{
	  strString = h7C7File.readLine();
	}
    while((strString.indexOf("----------------------") < 0) && (h7C7File.atEnd() == false));

	// Read lines
	m_lstTestName.clear();
	strString = h7C7File.readLine();
	while (strString.isNull() == false)
	{
		// Save Parameter name in list
		m_lstTestName.append(strString);
		// Read next line
		strString = h7C7File.readLine();
	};

	// Close file
	f.close();
}

//////////////////////////////////////////////////////////////////////
// Save 7C7 Parameter table to DISK
//////////////////////////////////////////////////////////////////////
void C7C7toSTDF::DumpParameterIndexTable(void)
{
	QString		strFileName;
	QString		strString;

    strFileName  = GS::Gex::Engine::GetInstance().Get("UserFolder").toString();
	strFileName += GEX_7C7_PARAMETERS;

	// Open WAT Parameter table file
    QFile f( strFileName );
    if(!f.open( QIODevice::WriteOnly ))
		return;

	// Assign file I/O stream
	QTextStream h7C7File(&f);

	// First few lines are comments:
	h7C7File << "############################################################" << endl;
	h7C7File << "# DO NOT EDIT THIS FILE!" << endl;
    h7C7File << "# Quantix Examinator: 7C7 Parameters detected" << endl;
	h7C7File << "# www.mentor.com" << endl;
    h7C7File << "# Quantix Examinator reads and writes into this file..." << endl;
	h7C7File << "-----------------------------------------------------------" << endl;

	// Write lines
	for (QStringList::const_iterator
		 iter  = m_lstTestName.begin();
		 iter != m_lstTestName.end(); ++iter) {
		// Write line
		h7C7File  << *iter << endl;
	}

	// Close file
	f.close();
}

//////////////////////////////////////////////////////////////////////
// If Examinator doesn't have this 7C7 parameter in his dictionnary, have to add it.
//////////////////////////////////////////////////////////////////////
void C7C7toSTDF::FindParameterIndex(QString strParamName)
{

	if(m_bModeDatabase)
	{
		// Check if Parameter name already in table...if not, add it to the list
		// the new full list will be dumped to the disk at the end.
        if(m_lstTestName.indexOf(strParamName) < 0)
		{
			// Update list
			m_lstTestName.append(strParamName);
		}
        m_cl7C7TestResult.test_num = m_lstTestName.indexOf(strParamName) + 1;
	}
}


//////////////////////////////////////////////////////////////////////
// Check if File is compatible with 7C7 format
//////////////////////////////////////////////////////////////////////
bool C7C7toSTDF::IsCompatible(const char *szFileName)
{

	bool		bIs7C7File;
	QFile		qFile(szFileName);
	QString		strLine;

	bIs7C7File= false;

	// Open 7C7 file
    if(!qFile.open( QIODevice::ReadOnly ))
	{
		// Failed Opening 7C7 file
		return false;
	}
	// Assign file I/O stream
	QTextStream	qTextFile(&qFile);

	// Go to the first tag and verify if it is the good 7C7 tag
	int nLineNb = 0;
	do
	{
		strLine = qTextFile.readLine();
		if(strLine.isEmpty() || strLine == " ")
			continue;
		
		nLineNb++;
		if(nLineNb > 3)
			break;

		// Verify if found the 7C7 open tag
		if (strLine.startsWith("<semiconductortestdatanotification", Qt::CaseInsensitive))
		{
			bIs7C7File = true;
			break;
		}
	}
	while(!qTextFile.atEnd());

	qFile.close();
	
	// Verify if found the 7C7 open tag
	if(!bIs7C7File)
	{
		// Failed importing 7C7 file into STDF database
		return false;
	}
	return true;
	// It's a 7C7 file
}

//////////////////////////////////////////////////////////////////////
// Convert 7C7 file, to STDF file
// bModeDatabase used to ignore all test_num and used the index table saved
//////////////////////////////////////////////////////////////////////
bool C7C7toSTDF::Convert(const char *szFileName7C7, const char *szFileNameSTDF, bool bModeDatabase)
{
	int nLstCount = 0;
	// No erro (default)
	m_iLastError = errNoError;
	m_bModeDatabase = bModeDatabase;

	if(m_bModeDatabase)
	{
		// Load 7C7 parameter table from disk...
		LoadParameterIndexTable();
		nLstCount = m_lstTestName.count();
	}

	// If STDF file already exists...do not rebuild it!
    QFile fSdtf(szFileNameSTDF);
	if(fSdtf.exists() == true)
		return true;

	// Open 7C7 file
    m_pf7C7 = new QFile(szFileName7C7);
    if(!m_pf7C7->open( QIODevice::ReadOnly ))
	{
		// Failed Opening 7C7 file
		m_iLastError = errOpenFail;
		
		// Convertion failed.
		return false;
	}
	// Assign file I/O stream
	m_ph7C7File = new QTextStream(m_pf7C7);

    m_pStdfFile = new GS::StdLib::Stdf(); // true for mode Debug
	
    if(m_pStdfFile->Open((char*)szFileNameSTDF,STDF_WRITE) != GS::StdLib::Stdf::NoError)
	{
		// Failed importing 7C7 file into STDF database
		m_iLastError = errWriteSTDF;
		
		// Convertion failed.
		// Close file
		m_pf7C7->close();
		return false;
	}
	

	//////////////////////////////////////////////////////////////////////
	// For ProgressBar
	bool bHideProgressAfter=true;
	bool bHideLabelAfter=false;

	iProgressStep = 0;
	iNextFilePos = 0;
	iFileSize = m_pf7C7->size() + 1;

	if(GexScriptStatusLabel != NULL)
	{
		if(GexScriptStatusLabel->isHidden())
		{
			bHideLabelAfter = true;
			GS::Gex::Engine::GetInstance().UpdateLabelStatus("Converting data from file "+QFileInfo(szFileName7C7).fileName() +"...");
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
	
    if(FirstParse7C7File() != true)
	{
		m_iLastError = errInvalidFormat;
		m_pf7C7->close();
		m_pStdfFile->Close();
		QFile::remove(szFileNameSTDF);
		//////////////////////////////////////////////////////////////////////
		// For ProgressBar
		if((GexProgressBar != NULL)
		&& bHideProgressAfter)
			GexProgressBar->hide();
		
		if((GexScriptStatusLabel != NULL)
		&& bHideLabelAfter)
			GexScriptStatusLabel->hide();

		return false;	// Error reading 7C7 file
	}

	iNextFilePos = 0;

	if(SecondParse7C7File() != true)
	{
		m_iLastError = errInvalidFormat;
		m_pf7C7->close();
		m_pStdfFile->Close();
		QFile::remove(szFileNameSTDF);
		//////////////////////////////////////////////////////////////////////
		// For ProgressBar
		if((GexProgressBar != NULL)
		&& bHideProgressAfter)
			GexProgressBar->hide();
		
		if((GexScriptStatusLabel != NULL)
		&& bHideLabelAfter)
			GexScriptStatusLabel->hide();

		return false;	// Error reading 7C7 file
	}

	m_pf7C7->close();
	m_pStdfFile->Close();
	// Convertion successful

	//////////////////////////////////////////////////////////////////////
	// For ProgressBar
	if((GexProgressBar != NULL)
	&& bHideProgressAfter)
		GexProgressBar->hide();
	
	if((GexScriptStatusLabel != NULL)
	&& bHideLabelAfter)
		GexScriptStatusLabel->hide();

	if(m_bModeDatabase && (nLstCount != m_lstTestName.count()))
		DumpParameterIndexTable();
	return true;
}

//////////////////////////////////////////////////////////////////////
// Restart at the begining of the 7C7 file
//////////////////////////////////////////////////////////////////////
QString C7C7toSTDF::readFirst()
{
	QFile * pFile;
	QString strName;
    strName = m_pf7C7->fileName();
    pFile = new QFile(strName);
    if(!pFile->open( QIODevice::ReadOnly ))
	{
		// Failed Opening 7C7 file
		m_iLastError = errOpenFail;
		
		// Convertion failed.
		return "";
	}

	// Close the file to reinitialize all values
	m_pf7C7->close();
	delete m_pf7C7; m_pf7C7=0;
	delete m_ph7C7File; m_ph7C7File=0;

	// Open 7C7 file
	m_pf7C7 = pFile;
	m_ph7C7File = new QTextStream(m_pf7C7);
	m_ph7C7File->reset();
	m_strCurrentLine = "";
	m_bEndOfFile = false;
	return readNext();
}

//////////////////////////////////////////////////////////////////////
// Parse the 7C7 file and split line between each tags
//////////////////////////////////////////////////////////////////////
QString C7C7toSTDF::readNext()
{
	int iPos;
	QString strString;

	if(m_bEndOfFile)
		return "";

	//////////////////////////////////////////////////////////////////////
	// For ProgressBar
	if(GexProgressBar != NULL)
	{
        while((int) m_pf7C7->pos() > iNextFilePos)
		{
			iProgressStep += 100/iFileSize + 1;
			iNextFilePos  += iFileSize/100 + 1;
			GexProgressBar->setValue(iProgressStep);
		}
	}
    QCoreApplication::processEvents();
	
	

	while(m_strCurrentLine.isEmpty() || m_strCurrentLine == " ")
		m_strCurrentLine = m_ph7C7File->readLine().simplified();

	if(m_strCurrentLine.left(1) == "<")
        iPos = m_strCurrentLine.indexOf('>');
	else
        iPos = m_strCurrentLine.indexOf('<')-1;

	if(iPos < 0)
	{
		strString = m_strCurrentLine;
		m_strCurrentLine = "";
	}
	else
	{
		strString = m_strCurrentLine.left(iPos+1);
		m_strCurrentLine = m_strCurrentLine.remove(0,iPos+1);
	}
	if(m_ph7C7File->atEnd())
		m_bEndOfFile = m_strCurrentLine.isEmpty();

	return strString;


}

//////////////////////////////////////////////////////////////////////
// remove the '<' at the begin and the '>' at the end
// delete de NameSpace String
//////////////////////////////////////////////////////////////////////
QString C7C7toSTDF::unEncapsuled(QString strString)
{
	int iPos, iPos2;
	QString strName = strString.simplified();
    iPos = strName.indexOf('<');
	strName = strName.remove(0,iPos+1);
    iPos = strName.indexOf('>');
	strName = strName.left(iPos);
    iPos = strName.indexOf(' ');
	strName = strName.left(iPos);

	// remove the name space
    iPos = strName.indexOf('/');
    iPos2 = strName.indexOf(':')-iPos;
	if (iPos2 >= 0)
		strName = strName.remove(iPos+1,iPos2);


	return strName;

}

//////////////////////////////////////////////////////////////////////
// First Parse the 7C7 file and store Test information
//////////////////////////////////////////////////////////////////////
bool C7C7toSTDF::FirstParse7C7File()
{
	QString strLine;
	QString strString;
	QString strValue;
	QString strSection;

	QString strTestId;
	QString strTestName;
	QString strTestPrimaryId;
	QString strMeasName;
	QString strLowUnit;
	QString strHighUnit;
	QString	strLowVal;
	QString strHighVal;
	
	int iPos;
	int iNumberOfLimit = 0;

	m_pcl7C7Test=NULL;
	strTestId	="";
	strTestName	="";
	strTestPrimaryId="";
	strMeasName	="";
	strLowUnit	="";
	strHighUnit	="";
	strLowVal	="";
	strHighVal	="";

	// Go to the first tag and verify if it is the good 7C7 tag
	do
	{
		strLine = readNext();
		if(strLine.isEmpty() || strLine == " ")
			continue;

        iPos = strLine.indexOf('<');
		if(iPos >= 0)
		{
			// if found a 'tag'
			strSection = unEncapsuled(strLine).toLower();

			// Verify if found the 7C7 open tag
			if (strSection == "semiconductortestdatanotification")
				break;
			else
				continue;
		}
	}
	while(!m_bEndOfFile);
	// Verify if found the 7C7 open tag
	if(m_bEndOfFile)
	{
		// Failed importing 7C7 file into STDF database
		m_iLastError = errInvalidFormat;
		
		// Convertion failed.
		return false;
	}
	// It's a 7C7 file

	do
	{
		strLine = readNext();
		if(strLine.isEmpty() || strLine == " ")
			continue;

        iPos = strLine.indexOf('<');
		if(iPos >= 0)
		{
			// if found a 'tag'
			strSection = unEncapsuled(strLine).toLower();

			// Verify if found an open tag
			if(strSection.left(1) != "/")
			{
				
				/* TAGS WITH value <tag>value</tag>*/
				if (strSection == "sortweight")
				{
					// find the encapsulled value
					strValue = readNext();
					m_cl7C7Summary.P_F = (strValue.toInt() == 100);

                    if(!GotoMarker(QString("/" + strSection).toLatin1().constData()))
					{
						// Failed importing 7C7 file into STDF database
						m_iLastError = errInvalidFormat;
						
						// Convertion failed.
						return false;
					}
				}
				else if((strSection == "sortname")
				     || (strSection == "hardsortname"))
				{
					// find the encapsulled value
					m_cl7C7Summary.Bin_desc = readNext();
                    if(!GotoMarker(QString("/" + strSection).toLatin1().constData()))
					{
						// Failed importing 7C7 file into STDF database
						m_iLastError = errInvalidFormat;
						
						// Convertion failed.
						return false;
					}
				}
				else if (strSection == "hardsort")
				{
					// find the encapsulled value
					m_cl7C7Summary.HW_Bin = readNext().toInt();
                    if(!GotoMarker(QString("/" + strSection).toLatin1().constData()))
					{
						// Failed importing 7C7 file into STDF database
						m_iLastError = errInvalidFormat;
						
						// Convertion failed.
						return false;
					}
				}
				else if (strSection == "sortid")
				{
					// find the encapsulled value
					m_cl7C7Summary.SW_Bin = readNext().toInt();
					if(m_cl7C7Summary.HW_Bin < 0)
						m_cl7C7Summary.HW_Bin = m_cl7C7Summary.SW_Bin;
                    if(!GotoMarker(QString("/" + strSection).toLatin1().constData()))
					{
						// Failed importing 7C7 file into STDF database
						m_iLastError = errInvalidFormat;
						
						// Convertion failed.
						return false;
					}
				}
				else if (strSection == "units")
				{
					// find the encapsulled value
					strLowUnit = strHighUnit = readNext();
                    if(!GotoMarker(QString("/" + strSection).toLatin1().constData()))
					{
						// Failed importing 7C7 file into STDF database
						m_iLastError = errInvalidFormat;
						
						// Convertion failed.
						return false;
					}
				}
				else if (strSection == "lowlimit")
				{
					// find the encapsulled value
					strLowVal = readNext();
                    if(!GotoMarker(QString("/" + strSection).toLatin1().constData()))
					{
						// Failed importing 7C7 file into STDF database
						m_iLastError = errInvalidFormat;
						
						// Convertion failed.
						return false;
					}
				}
				else if (strSection == "highlimit")
				{
					// find the encapsulled value
					strHighVal = readNext();
                    if(!GotoMarker(QString("/" + strSection).toLatin1().constData()))
					{
						// Failed importing 7C7 file into STDF database
						m_iLastError = errInvalidFormat;
						
						// Convertion failed.
						return false;
					}
				}
				else if (strSection == "testname")
				{
					// find the encapsulled value
					strTestName = readNext();
                    if(!GotoMarker(QString("/" + strSection).toLatin1().constData()))
					{
						// Failed importing 7C7 file into STDF database
						m_iLastError = errInvalidFormat;
						
						// Convertion failed.
						return false;
					}
				}
				else if (strSection == "testid")
				{
					// find the encapsulled value
					strTestId = readNext();
                    if(!GotoMarker(QString("/" + strSection).toLatin1().constData()))
					{
						// Failed importing 7C7 file into STDF database
						m_iLastError = errInvalidFormat;
						
						// Convertion failed.
						return false;
					}
				}
				else if (strSection == "primaryidentifier")
				{
					// find the encapsulled value
					strTestPrimaryId = readNext();
                    if(!GotoMarker(QString("/" + strSection).toLatin1().constData()))
					{
						// Failed importing 7C7 file into STDF database
						m_iLastError = errInvalidFormat;
						
						// Convertion failed.
						return false;
					}
				}
/* LISTE OF TAGS IGNORED FOR THE MOMENT */
/*				else if (strSection == "binassignment")
				{
					// A user defined integer designation for hard bins
					// strValue = readNext();
                    if(!GotoMarker(QString("/" + strSection).toLatin1().constData()))
					{
						// Failed importing 7C7 file into STDF database
						m_iLastError = errInvalidFormat;
						
						// Convertion failed.
						return false;
					}
				}
				else if (strSection == "passvisualinspection")
				{
					// Boolean
					// A user defined pass/fail quality control inspection 
					// flag of wafer after wafer sort
					// strValue = readNext();
                    if(!GotoMarker(QString("/" + strSection).toLatin1().constData()))
					{
						// Failed importing 7C7 file into STDF database
						m_iLastError = errInvalidFormat;
						
						// Convertion failed.
						return false;
					}
				}
				else if (strSection == "repairableflag")
				{
					// Boolean
					// A flag to indicate if product (die or module) are 
					// repairable based on user definition
					// strValue = readNext();
                    if(!GotoMarker(QString("/" + strSection).toLatin1().constData()))
					{
						// Failed importing 7C7 file into STDF database
						m_iLastError = errInvalidFormat;
						
						// Convertion failed.
						return false;
					}
				}
				else if((strSection == "sortsymbol")
					 || (strSection == "hardsortsymbol"))
				{
					// A user defined grouping of fails on a wafer map
					// strValue = readNext();
                    if(!GotoMarker(QString("/" + strSection).toLatin1().constData()))
					{
						// Failed importing 7C7 file into STDF database
						m_iLastError = errInvalidFormat;
						
						// Convertion failed.
						return false;
					}
				}
				else if (strSection == "category")
				{
					// find the encapsulled value
                    if(!GotoMarker(QString("/" + strSection).toLatin1().constData()))
					{
						// Failed importing 7C7 file into STDF database
						m_iLastError = errInvalidFormat;
						
						// Convertion failed.
						return false;
					}
				}
				else if (strSection == "passkeyitemdefect")
				{
					// find the encapsulled value
					// strValue = readNext();
                    if(!GotoMarker(QString("/" + strSection).toLatin1().constData()))
					{
						// Failed importing 7C7 file into STDF database
						m_iLastError = errInvalidFormat;
						
						// Convertion failed.
						return false;
					}
				}
				else if (strSection == "calculation")
				{
					// String describe the math exp to derive a result
					// find the encapsulled value
					// strValue = readNext();
                    if(!GotoMarker(QString("/" + strSection).toLatin1().constData()))
					{
						// Failed importing 7C7 file into STDF database
						m_iLastError = errInvalidFormat;
						
						// Convertion failed.
						return false;
					}
				}
				else if (strSection == "censorhlim")
				{
					// Float for the high boundary for valid test data
					// find the encapsulled value
					// strValue = readNext();
                    if(!GotoMarker(QString("/" + strSection).toLatin1().constData()))
					{
						// Failed importing 7C7 file into STDF database
						m_iLastError = errInvalidFormat;
						
						// Convertion failed.
						return false;
					}
				}
				else if (strSection == "censorllim")
				{
					// Float for the low boundary for valid test data
					// find the encapsulled value
					// strValue = readNext();
                    if(!GotoMarker(QString("/" + strSection).toLatin1().constData()))
					{
						// Failed importing 7C7 file into STDF database
						m_iLastError = errInvalidFormat;
						
						// Convertion failed.
						return false;
					}
				}
				else if (strSection == "condition")
				{
					// String spec the test parameter : TestTemperature
					// find the encapsulled value
					// strValue = readNext();
                    if(!GotoMarker(QString("/" + strSection).toLatin1().constData()))
					{
						// Failed importing 7C7 file into STDF database
						m_iLastError = errInvalidFormat;
						
						// Convertion failed.
						return false;
					}
				}
				else if (strSection == "target")
				{
					// Float describe the expected value for the test
					// find the encapsulled value
					// strValue = readNext();
                    if(!GotoMarker(QString("/" + strSection).toLatin1().constData()))
					{
						// Failed importing 7C7 file into STDF database
						m_iLastError = errInvalidFormat;
						
						// Convertion failed.
						return false;
					}
				}
*/				/* TAGS WITH TAGS <tagheader><tag>value</tag></tagheader>*/
				else if (strSection == "sort")
				{
				}
				else if (strSection == "proprietaryunits")
				{
				}
				else if (strSection == "measurementunitchoice")
				{
				}
				else if (strSection == "measurementunit")
				{
				}
				else if (strSection == "testparameter")
				{
				}
				else if (strSection == "testspecificationreport")
				{
				}
				else if (strSection == "testopidentification")
				{
					// One per Wafer or FinalTest
					m_nTotalWafers++;
				}
				else if (strSection == "finaltest")
				{
					m_strDataOrigin = GEX_IMPORT_DATAORIGIN_ATETEST;
                    if(!GotoMarker(QString("/" + strSection).toLatin1().constData()))
					{
						// Failed importing 7C7 file into STDF database
						m_iLastError = errInvalidFormat;
						return false;
					}
				}
				else if (strSection == "wafersort")
				{
					m_strDataOrigin = GEX_IMPORT_DATAORIGIN_ATETEST;
                    if(!GotoMarker(QString("/" + strSection).toLatin1().constData()))
					{
						// Failed importing 7C7 file into STDF database
						m_iLastError = errInvalidFormat;
						return false;
					}
				}
				else if (strSection == "pcm")
				{
					m_strDataOrigin = GEX_IMPORT_DATAORIGIN_ETEST;
                    if(!GotoMarker(QString("/" + strSection).toLatin1().constData()))
					{
						// Failed importing 7C7 file into STDF database
						m_iLastError = errInvalidFormat;
						return false;
					}
				}
				else if (strSection == "testoperationdescription")
				{
				}
				else if (strSection == "lotreport")
				{
				}
				else if (strSection == "semiconductortestdatanotification")
				{
					// the first tag of a 7C7 file
				}
				else if (strSection == "creation")
				{
					// find the encapsulled value
					strValue = readNext();
					// "2005-03-16T07:53:50Z"
					// "Z" to indicate Coordinated Universal Time (UTC)
					// or to indicate the time zone immediately followed by a sign, + or -,
					QDate clDate(strValue.left(4).toInt(),
								 strValue.mid(5,2).toInt(),
								 strValue.mid(8,2).toInt());
					QTime clTime(strValue.mid(11,2).toInt(),
								 strValue.mid(14,2).toInt(),
								 strValue.mid(17,2).toInt());
					QDateTime clDateTime(clDate, clTime);

					int iUTCTime = 0;
                    iPos = strValue.indexOf('Z');
					if((iPos > 0) && (iPos < (int) (strValue.length()-1)))
					{
						// "Z+01:00" or "Z-01:00"
						iUTCTime = (strValue.mid(iPos+2,2).toInt() * 3600) + (strValue.right(2).toInt() * 60);

						// find "-" after the DateTime spec
                        if(strValue.indexOf('+',iPos)>0)
							iUTCTime = 0 - iUTCTime;
					}

                    clDateTime.setTime_t(clDateTime.toTime_t() + iUTCTime);
                    clDateTime.toUTC();

                    if(!GotoMarker(QString("/" + strSection).toLatin1().constData()))
					{
						// Failed importing 7C7 file into STDF database
						m_iLastError = errInvalidFormat;
						
						// Convertion failed.
						return false;
					}
				}
				else if (strSection == "documentinformation")
				{
				}
				else if (strSection == "documentheader")
				{
				}
				else 
				{
					// unknown section
					// skip it
                    if(!GotoMarker(QString("/" + strSection).toLatin1().constData()))
					{
						// Failed importing 7C7 file into STDF database
						m_iLastError = errInvalidFormat;
						
						// Convertion failed.
						return false;
					}
				}

			}
			else if (strSection == "/testparameter")
			{
				// found the end of one test description
				// save record
				if (strTestPrimaryId == "" )
				{
					// Failed importing 7C7 file into STDF database
					m_iLastError = errInvalidFormat;
					
					// Convertion failed.
					return false;
				}
				m_pcl7C7Test = GetTest(strTestPrimaryId.toInt());
				m_pcl7C7Test->test_num		= strTestId.toInt();
				m_pcl7C7Test->test_name		= strTestName;

				if (SetLimit(m_pcl7C7Test,iNumberOfLimit,"",strLowUnit,strLowVal,strHighUnit,strHighVal))
					iNumberOfLimit++;
				strLowUnit	="";
				strHighUnit	="";
				strLowVal	="";
				strHighVal	="";

			}
			else if (strSection == "/testspecificationreport")
			{
				// found the end of one test description
				// save record
				if (strTestPrimaryId == "" )
				{
					// Failed importing 7C7 file into STDF database
					m_iLastError = errInvalidFormat;
					
					// Convertion failed.
					return false;
				}
				m_pcl7C7Test = GetTest(strTestPrimaryId.toInt());
				m_pcl7C7Test->test_num		= strTestId.toInt();
				m_pcl7C7Test->test_name		= strTestName;

				SetLimit(m_pcl7C7Test,iNumberOfLimit,"",strLowUnit,strLowVal,strHighUnit,strHighVal);

				m_pcl7C7Test=NULL;
				strTestId	="";
				strTestName	="";
				strTestPrimaryId="";
				strMeasName	="";
				strLowUnit	="";
				strHighUnit	="";
				strLowVal	="";
				strHighVal	="";
				iNumberOfLimit=0;

			}
			else if (strSection == "/sort")
			{
				SaveSbr();
				m_cl7C7Summary.reset();
			}

		}
		// else it's a close tag, skip the line
		// and continue

	}
	while(!m_bEndOfFile);	
	// Success parsing 7C7 file
	
	return true;
}

//////////////////////////////////////////////////////////////////////
// Second step : Parse the 7C7 file and write STDF file
//////////////////////////////////////////////////////////////////////
bool C7C7toSTDF::SecondParse7C7File()
{
	QString strLine;
	QString strString;
	QString strValue;
	QString strSection;
	
	int iPos;

	// Restart at the begining
	strLine = readFirst();
	// Skip the Header
	do
	{
		strLine = readNext();
		if(strLine.isEmpty() || strLine == " ")
			continue;

        iPos = strLine.indexOf('<');
		if(iPos >= 0)
		{
			// if found a 'tag'
			strSection = unEncapsuled(strLine).toLower();

			// Verify if found an open tag
			if(strSection.left(1) != "/")
			{
				
				/* TAGS WITH VALUE <tag>value</tag>*/
				if (strSection == "lottype")
				{
					// find the encapsulled value
					strValue =  readNext().toLower();
					if( (strValue == "dev")||(strValue == "mfg")||(strValue == "eng")||(strValue == "rnd"))
					{m_cModeCod = 'E';}
					else if ((strValue == "tst")||(strValue == "plt"))
					{m_cModeCod = 'D';}
					else if ((strValue == "prd")||(strValue == "ppd"))
					{m_cModeCod = 'P';}
					else if ((strValue == "svc"))
					{m_cModeCod = 'M';}
					else if ((strValue == "pqc"))
					{m_cModeCod = 'C';}
					else if ((strValue == "euh"))
					{m_cModeCod = 'A';}
					 
                    if(!GotoMarker(QString("/" + strSection).toLatin1().constData()))
						return false;
				}
				else if (strSection == "productname")
				{
					// find the encapsulled value
					m_strPartTyp = readNext();
                    if(!GotoMarker(QString("/" + strSection).toLatin1().constData()))
						return false;
				}
				else if (strSection == "technology")
				{
					// find the encapsulled value
					m_strProcId = readNext();
                    if(!GotoMarker(QString("/" + strSection).toLatin1().constData()))
						return false;
				}
				else if (strSection == "lotenddatetime")
				{
					// find the encapsulled value
					strValue = readNext();
					// "2005-03-16T07:53:50Z"
					// "Z" to indicate Coordinated Universal Time (UTC)
					// or to indicate the time zone immediately followed by a sign, + or -,
					QDate clDate(strValue.left(4).toInt(),
								 strValue.mid(5,2).toInt(),
								 strValue.mid(8,2).toInt());
					QTime clTime(strValue.mid(11,2).toInt(),
								 strValue.mid(14,2).toInt(),
								 strValue.mid(17,2).toInt());
					QDateTime clDateTime(clDate, clTime);

					int iUTCTime = 0;
                    iPos = strValue.indexOf('Z');
					if((iPos > 0) && (iPos < (int) (strValue.length()-1)))
					{
						// "Z+01:00" or "Z-01:00"
						iUTCTime = (strValue.mid(iPos+2,2).toInt() * 3600) + (strValue.right(2).toInt() * 60);

						// find "-" after the DateTime spec
                        if(strValue.indexOf('+',iPos)>0)
							iUTCTime = 0 - iUTCTime;
					}

                    clDateTime.setTime_t(clDateTime.toTime_t() + iUTCTime);
                    clDateTime.toUTC();
					m_lEndT = m_lSetupT = clDateTime.toTime_t();

                    if(!GotoMarker(QString("/" + strSection).toLatin1().constData()))
						return false;
				}
				else if (strSection == "lotstartdatetime")
				{
					// find the encapsulled value
					strValue = readNext();
					// "2005-03-16T07:53:50Z"
					QDate clDate(strValue.left(4).toInt(),
								 strValue.mid(5,2).toInt(),
								 strValue.mid(8,2).toInt());
					QTime clTime(strValue.mid(11,2).toInt(),
								 strValue.mid(14,2).toInt(),
								 strValue.mid(17,2).toInt());
					QDateTime clDateTime(clDate, clTime);

					int iUTCTime = 0;
                    iPos = strValue.indexOf('Z');
					if((iPos > 0) && (iPos < (int) (strValue.length()-1)))
					{
						// "Z+01:00" or "Z-01:00"
						iUTCTime = (strValue.mid(iPos+2,2).toInt() * 3600) + (strValue.right(2).toInt() * 60);

						// find "-" after the DateTime spec
                        if(strValue.indexOf('+',iPos)>0)
							iUTCTime = 0 - iUTCTime;
					}

                    clDateTime.setTime_t(clDateTime.toTime_t() + iUTCTime);
                    clDateTime.toUTC();
					m_lStartT = m_lLastPartTime = clDateTime.toTime_t();
					if(m_lEndT <= 0) m_lEndT = m_lStartT; 

                    if(!GotoMarker(QString("/" + strSection).toLatin1().constData()))
						return false;
				}
				else if (strSection == "equipmentid")
				{
					// find the encapsulled value
					m_strEquipmentID = readNext();
                    if(!GotoMarker(QString("/" + strSection).toLatin1().constData()))
						return false;
				}
				else if (strSection == "testprogec")
				{
					// find the encapsulled value
					m_strJobRev = readNext();
                    if(!GotoMarker(QString("/" + strSection).toLatin1().constData()))
						return false;
				}
				else if (strSection == "testprogramname")
				{
					// find the encapsulled value
					m_strJobNam = readNext();
                    if(!GotoMarker(QString("/" + strSection).toLatin1().constData()))
						return false;
				}
				else if (strSection == "testlevel")
				{
					// Wafer
					// find the encapsulled value
					m_strTestLevel = readNext().toLower();
					if (m_strTestLevel == "wafer" )
						m_cl7C7DutResult.nFlags = WAFER_ID;
					else
					if (m_strTestLevel == "last_metal" )
						m_cl7C7DutResult.nFlags = WAFER_ID;
					else
						m_cl7C7DutResult.nFlags = PACKAGE_ID;
                    if(!GotoMarker(QString("/" + strSection).toLatin1().constData()))
						return false;
				}
				else if (strSection == "testmode")
				{
					// WFT = Wafer Final Test
					// find the encapsulled value
					// strValue = readNext();
                    if(!GotoMarker(QString("/" + strSection).toLatin1().constData()))
						return false;
				}
				else if (strSection == "waferuniqueid")
				{
					// find the encapsulled value
					m_strWaferId = readNext();
                    if(!GotoMarker(QString("/" + strSection).toLatin1().constData()))
						return false;
				}
				else if (strSection == "retesttype")
				{
					// Describes how test data should be combined 
					// with prior data for the same LotID, test and level
					// strValue = readNext();
					m_cRtstCod = 'Y';
                    if(!GotoMarker(QString("/" + strSection).toLatin1().constData()))
						return false;
				}
				else if (strSection == "contact")
				{
					// Name of the contact
					m_strOperNam = readNext();
                    if(!GotoMarker(QString("/" + strSection).toLatin1().constData()))
						return false;
				}
				else if (strSection == "identifier")
				{
					// find the encapsulled value
					m_strLotId = readNext();
                    if(!GotoMarker(QString("/" + strSection).toLatin1().constData()))
						return false;
				}
				else if (strSection == "manufacturingid")
				{
					// Alphanumeric string which describes the lot
					m_strNodeNam = readNext();
                    if(!GotoMarker(QString("/" + strSection).toLatin1().constData()))
						return false;
				}
/* LISTE OF TAGS IGNORED FOR THE MOMENT */
/*				else if (strSection == "filedataversion")
				{
					// User defined changes i.e. rules changes to content of data in the data 
					// file transfer.  Examples: version update to loaders, parsers, or 
					// operating system, specification limits etc
					// strValue = readNext();
                    if(!GotoMarker(QString("/" + strSection).toLatin1().constData()))
						return false;
				}
				else if (strSection == "globallotstatuscode")
				{
					// Describes lot or sublot status
					// strValue = readNext();
                    if(!GotoMarker(QString("/" + strSection).toLatin1().constData()))
						return false;
				}
				else if (strSection == "qualitycode")
				{
					// Designate a quality condition of a lot. This can be a proprietary 
					// label defined by the Foundry of the Test Services trading partner
					// strValue = readNext();
                    if(!GotoMarker(QString("/" + strSection).toLatin1().constData()))
						return false;
				}
				else if (strSection == "proprietarylabel")
				{
					// User defined label to uniquely identify a 
					// facility that is represented in any of the scenarios
					// strValue = readNext();
                    if(!GotoMarker(QString("/" + strSection).toLatin1().constData()))
						return false;
				}
				else if (strSection == "authority")
				{
					// find the encapsulled value
					// strValue = readNext();
                    if(!GotoMarker(QString("/" + strSection).toLatin1().constData()))
						return false;
				}
				else if (strSection == "idsuffix")
				{
					// user defined to identify a lot
					// strValue = readNext();
                    if(!GotoMarker(QString("/" + strSection).toLatin1().constData()))
						return false;
				}
				else if (strSection == "nominalsize")
				{
					// find the encapsulled value
					// strValue = readNext();
                    if(!GotoMarker(QString("/" + strSection).toLatin1().constData()))
						return false;
				}
				else if (strSection == "headid")
				{
					// Identifies the test head used for wafer sort and final test
					// strValue = readNext();
                    if(!GotoMarker(QString("/" + strSection).toLatin1().constData()))
						return false;
				}
				else if (strSection == "notch")
				{
					// Describes the orientation  of the wafer notch in a tester:  
					// Valid options are: 12 o'clock; 3 o'clock, 6 o'clock, 9 o'clock, 
					// notch up, notch down, notch right or notch left, Up=U, Down=D, 
					// Left=L, or Right=R, 0, 90, 180 or 270 degrees
					// strValue = readNext();
                    if(!GotoMarker(QString("/" + strSection).toLatin1().constData()))
						return false;
				}
				else if (strSection == "paralleltest")
				{
					// Integer
					// The number of devices or parallel tests for wafer sort
					// strValue = readNext();
                    if(!GotoMarker(QString("/" + strSection).toLatin1().constData()))
						return false;
				}
				else if (strSection == "probename")
				{
					// Describes the probe recipe or pattern file
					// strValue = readNext();
                    if(!GotoMarker(QString("/" + strSection).toLatin1().constData()))
						return false;
				}
				else if (strSection == "quadrant")
				{
					// A user defined division of the wafer into 4 quadrants. 
					// Valid values:  I, II, III, IV, Q1,Q2, Q3, Q4, Upper Left, 
					// Upper Right, toLower Left, toLower Right and User defined labels
					// strValue = readNext();
                    if(!GotoMarker(QString("/" + strSection).toLatin1().constData()))
						return false;
				}
				else if (strSection == "chippn")
				{
					// A string that describes the part number for the device
					// strValue = readNext();
                    if(!GotoMarker(QString("/" + strSection).toLatin1().constData()))
						return false;
				}
				else if (strSection == "diereticlepn")
				{
					// A string which defines the unique part numbers for 
					// die associated with a multi-project wafer
					// strValue = readNext();
                    if(!GotoMarker(QString("/" + strSection).toLatin1().constData()))
						return false;
				}
				else if (strSection == "packagedescription")
				{
					// User definable element for package size (default=mm), 
					// package type (string), and pin count (Integer)
					// strValue = readNext();
                    if(!GotoMarker(QString("/" + strSection).toLatin1().constData()))
						return false;
				}
				else if (strSection == "parttext")
				{
					// String. A user defined comment field to attach
					// additional die information
					// strValue = readNext();
                    if(!GotoMarker(QString("/" + strSection).toLatin1().constData()))
						return false;
				}
				else if (strSection == "serialnumber")
				{
					// An alphanumeric string that uniquely defines the 
					// numbering of packaged die
					// strValue = readNext();
                    if(!GotoMarker(QString("/" + strSection).toLatin1().constData()))
						return false;
				}
				else if (strSection == "primaryidentifier")
				{
					// find the encapsulled value
					// strValue = readNext();
                    if(!GotoMarker(QString("/" + strSection).toLatin1().constData()))
						return false;
				}
				else if (strSection == "propretarylabel")
				{
					// find the encapsulled value
					// strValue = readNext();
                    if(!GotoMarker(QString("/" + strSection).toLatin1().constData()))
						return false;
				}
*/				
				/* TAGS WITH TAGS <tagheader><tag>value</tag></tagheader>*/
				else if (strSection == "testreport")
				{
					if(!AnalyzeTestReport())
						return false;
				}
				else if (strSection == "wafersortlocation")
				{
				}
				else if (strSection == "finaltest")
				{
				}
				else if (strSection == "wafersort")
				{
					WriteWir();
				}
				else if (strSection == "pcm")
				{
					WriteWir();
				}
				else if (strSection == "die")
				{
				}
				else if (strSection == "testsetup")
				{
				}
				else if (strSection == "dimension")
				{
				}
				else if (strSection == "tester")
				{
				}
				else if (strSection == "testopidentification")
				{
				}
				else if (strSection == "testopidentificationchoice")
				{
				}
				else if (strSection == "setupreport")
				{
				}
				else if (strSection == "productidentification")
				{
				}
				else if (strSection == "productidentificationchoice")
				{
				}
				else if (strSection == "alternativeidentifier")
				{
				}
				else if (strSection == "testoperationdescription")
				{
				}
				else if (strSection == "customerpn")
				{
				}
				else if (strSection == "alternativepn")
				{
				}
				else if (strSection == "contractorlotnumber")
				{
				}
				else if (strSection == "customerlotnumber")
				{
				}
				else if (strSection == "sublot")
				{
				}
				else if (strSection == "alternativepn")
				{
				}
				else if (strSection == "alternativeidentifier")
				{
				}
				else if (strSection == "fablocation")
				{
					// Describes where the wafer was fabricated
				}
				else if (strSection == "balocation")
				{
					// Describes where the wafer was sent for bond and assembly
				}
				else if (strSection == "finaltestlocation")
				{
					// Describes where the packaged die was sent for final test
				}
				else if (strSection == "wafersortlocation")
				{
				}
				else if (strSection == "otherlocation")
				{
				}
				else if (strSection == "lot")
				{
				}
				else if (strSection == "lotreport")
				{
				}
				else if (strSection == "contactinformation")
				{
				}
				else if (strSection == "receiver")
				{
				}
				//else if (strSection == "sender")
				//{
				//}
				else if (strSection == "documentinformation")
				{
				}
				else if (strSection == "documentheader")
				{
				}
				else if (strSection == "semiconductortestdatanotification")
				{
					// the first tag of a 7C7 file
				}
				else  
				{
					// unknown section
					// skip it
                    if(!GotoMarker(QString("/" + strSection).toLatin1().constData()))
						return false;
				}

			}
			else if (strSection == "/contractorlotnumber")
			{
				m_strLotId = m_strNodeNam;
			}
			else if (strSection == "/customerlotnumber")
			{
				m_strLotId = m_strNodeNam;
			}
			else if (strSection == "/sublot")
			{
				m_strSbLotId = m_strNodeNam;
			}
			else if (strSection == "/setupreport")
			{
				if (!m_bMirWriten)
				{
					WriteFar();
					WriteMir();
				}
			}
			else if (strSection == "/wafersort")
			{
				WriteWrr();
				m_nWaferNumDie		= 0;
				m_nWaferNumPassed	= 0;
				m_nWaferNumRetested = 0;

			}
			else if (strSection == "/pcm")
			{
				WriteWrr();
				m_nWaferNumDie		= 0;
				m_nWaferNumPassed	= 0;
				m_nWaferNumRetested = 0;

			}
		}
		// else it's a close tag, skip the line
		// and continue

	}
	while(!m_bEndOfFile);	
	// Success parsing 7C7 file

	WriteTsr();
	WriteHbr();
	WriteSbr();
	//WritePcr();
	WriteMrr();
	
	return true;
}

//////////////////////////////////////////////////////////////////////
bool C7C7toSTDF::AnalyzeDutResult()
{
	int		iPos;
	QString strLine;
	QString	strValue;
	QString	strTmp;
	QString strString;
	QString strSection;

	QDate		clDate;
	QTime		clTime;

	m_cl7C7DutResult.reset();
	do
	{
		strLine = readNext();
		if(strLine.isEmpty() || strLine == " ")
			continue; // skip empty line

        iPos = strLine.indexOf('<');
		if(iPos >= 0)
		{
			// if found a 'tag'
			strSection = unEncapsuled(strLine).toLower();

			// Verify if found an open tag
			if(strSection.left(1) != "/")
			{
				// Verify if found a key word
				if (strSection == "firstfailsort")
				{
					// Integer
					// Described the sort ID for the first fail on the device
					strValue = readNext();
					C7C7Sbr* pclSbr;
					pclSbr = GetSbr(strValue.toInt());
					if(pclSbr)
					{
						pclSbr->Prcnt++;
						m_cl7C7DutResult.SW_Bin = pclSbr->SW_Bin;
						m_cl7C7DutResult.HW_Bin = pclSbr->HW_Bin;
						m_cl7C7DutResult.P_F = pclSbr->P_F;
					}
                    if(!GotoMarker(QString("/" + strSection).toLatin1().constData()))
						return false;
				}
				else if (strSection == "compositefailsort")
				{
					// A user initiated failure override of first fail sort to allow for the 
					// collection of additional sort fail data. The additional sort fails can 
					// be combined in a comma separated list of sort numbers as a composite fail
					// strValue = readNext();
                    if(!GotoMarker(QString("/" + strSection).toLatin1().constData()))
						return false;
				}
				else if (strSection == "charflag")
				{
					// Boolean
					// Used as a indicator for the presence of characterization data for a die
					// strValue = readNext();
                    if(!GotoMarker(QString("/" + strSection).toLatin1().constData()))
						return false;
				}
				else if (strSection == "usersite")
				{
					// Integer
					// A user-defined die coordinate represented as an integer which is often 
					// associated with the serial number assigned by the tester or prober executive
					strValue = readNext();
					m_cl7C7DutResult.bSite = true;
					m_cl7C7DutResult.Site = strValue.toInt();
                    if(!GotoMarker(QString("/" + strSection).toLatin1().constData()))
						return false;
				}
				else if (strSection == "x")
				{
					// find the encapsulled value
					strValue = readNext();
					m_cl7C7DutResult.X = strValue.toInt();
                    if(!GotoMarker(QString("/" + strSection).toLatin1().constData()))
						return false;
				}
				else if (strSection == "y")
				{
					// find the encapsulled value
					strValue = readNext();
					m_cl7C7DutResult.Y = strValue.toInt();
                    if(!GotoMarker(QString("/" + strSection).toLatin1().constData()))
						return false;
				}
				else if (strSection == "gangtest")
				{
				}
				else if (strSection == "intcoordinate")
				{
				}
				else 
				{
					// unknown section
					// skip it
                    if(!GotoMarker(QString("/" + strSection).toLatin1().constData()))
						return false;
				}
			}
			else if(strSection == "/diereport")
			{
				if(m_cl7C7DutResult.P_F)
					m_nWaferNumPassed++;

				m_nWaferNumDie++;
				WritePir();
				return true;
			}
		}

	}
	while(!m_bEndOfFile);
		
	return false;
}

//////////////////////////////////////////////////////////////////////
bool C7C7toSTDF::AnalyzeTestReport()
{
	int		iPos;
	QString strLine;
	QString strString;
	QString strSection;


	m_cl7C7TestResult.reset();
	m_cl7C7TestResult.test_primary_id = 1;

	do
	{
		strString = readNext();
		if(strString.isEmpty() || strLine == " ")
			continue; // skip empty line

        iPos = strString.indexOf('<');
		if(iPos >= 0)
		{
			// if found a 'tag'
			strSection = unEncapsuled(strString).toLower();

			// Verify if found an open tag
			if(strSection.left(1) != "/")
			{
				// Verify if found a key word
				if (strSection == "diereport")
				{
					if(!AnalyzeDutResult())
						return false;
				}
				else if (strSection == "fpreport")
				{
					// A report that collects functional pin test results
					if(!AnalyzeTestResult())
						return false;
				}
				else if (strSection == "prreport")
				{
					// A report that data logs parametric test results
					if(!AnalyzeTestResult())
						return false;
				}
				else if (strSection == "ppreport")
				{
					// A report for datalogged per pin data
					// if(!AnalyzeTestResult())
                    if(!GotoMarker(QString("/" + strSection).toLatin1().constData()))
						return false;
				}
				else if (strSection == "functionalpinreport")
				{
					// A report for datalogged per pin data
					// if(!AnalyzeTestResult())
                    if(!GotoMarker(QString("/" + strSection).toLatin1().constData()))
						return false;
				}
				else if (strSection == "measurement")
				{
					// find the encapsulled value
					// strValue = readNext();
                    if(!GotoMarker(QString("/" + strSection).toLatin1().constData()))
						return false;
				}
				else 
				{
					// unknown section
					// skip it
                    if(!GotoMarker(QString("/" + strSection).toLatin1().constData()))
						return false;
				}
			}
			else if (strSection == "/testreport")
			{
				WritePrr();
				return true;
			}

			continue;
		}
	}
	while(!m_bEndOfFile);

	m_cl7C7TestResult.reset();
	return false;
}

//////////////////////////////////////////////////////////////////////
bool C7C7toSTDF::AnalyzeTestResult()
{
	int		iPos;
	bool	bOk;
	QString strLine;
	QString	strWord;
	QString	strValue;
	QString	strTmp;
	QString strString;
	QString strSection;

	QDate		clDate;
	QTime		clTime;

	m_cl7C7TestResult.reset();
	m_cl7C7TestResult.test_primary_id = 1;
	m_pcl7C7Test = NULL;

	// In m_cl7C7DutResult, current Dut tested


	do
	{
		strLine = readNext();
		if(strLine.isEmpty() || strLine == " ")
			continue; // skip empty line

        iPos = strLine.indexOf('<');
		if(iPos >= 0)
		{
			// if found a 'tag'
			strSection = unEncapsuled(strLine).toLower();

			// Verify if found an open tag
			if(strSection.left(1) != "/")
			{
				// Verify if found a key word
				if (strSection == "primaryidentifier")
				{
					// find the encapsulled value
					strValue = readNext();
					m_cl7C7TestResult.test_primary_id = strValue.toInt(&bOk);
					m_pcl7C7Test = GetTest(m_cl7C7TestResult.test_primary_id);
					m_cl7C7TestResult.test_name = m_pcl7C7Test->test_name;
					m_cl7C7TestResult.test_num = m_pcl7C7Test->test_num;
					m_cl7C7TestResult.strResultUnit = m_pcl7C7Test->strResultUnit;

					FindParameterIndex(m_cl7C7TestResult.test_name);

                    if(bOk && !GotoMarker(QString("/" + strSection).toLatin1().constData()))
						return false;
				}
				else if (strSection == "testresult")
				{
					// find the encapsulled value
					// PASSED have measure else FAILED
					strValue = readNext().toLower();
					m_cl7C7TestResult.Result = (strValue == "pas");

					m_cl7C7DutResult.TT++;
					if(!m_cl7C7TestResult.Result)
						m_cl7C7DutResult.TF++;



                    if(!GotoMarker(QString("/" + strSection).toLatin1().constData()))
						return false;
				}
				else if (strSection == "measurement")
				{
					// find the encapsulled value
					strValue = readNext().toLower();
					strValue.toInt(&bOk);
					if(!bOk)
					{
						float fNum1,fNum2;
						int iNum;
						// "123e-3"
                        iNum = strValue.indexOf('e');
						if(iNum > 0)
						{
							fNum1 = strValue.left(iNum).toFloat();
							strValue.remove(0,iNum+1);
							iNum = strValue.toInt();
							fNum2 = GS_POW(10.0,iNum);
							strValue = strValue.number(fNum1*fNum2);
							m_cl7C7TestResult.ResUnit.append(strValue);
						}
					}
					else
							m_cl7C7TestResult.ResUnit.append(strValue);
                    if(!GotoMarker(QString("/" + strSection).toLatin1().constData()))
						return false;
				}
				else if(strSection == "pin")
				{
					// A Pin IDs for each pin measured
					// strValue = readNext();
				}
				else if(strSection == "bits")
				{
					// The value of 0 or 1, which represent On/Off, Pass/Fail or True/False.
					// strValue = readNext();
				}
				else if(strSection == "ppresult")
				{
				}
				else
				{
                    if(!GotoMarker(QString("/" + strSection).toLatin1().constData()))
						return false;
				}
			}
			else if(strSection == "/fpreport")
			{
				WriteFtr();
				m_cl7C7TestResult.reset();
				return true;
			}
			else if(strSection == "/prreport")
			{
				WritePtr();
				m_cl7C7TestResult.reset();
				return true;
			}
			else if(strSection == "/ppreport")
			{
				m_cl7C7TestResult.reset();
				return true;
			}
			else if(strSection == "/functionalpinreport")
			{
				m_cl7C7TestResult.reset();
				return true;
			}
		}
	}
	while(!m_bEndOfFile);

	m_cl7C7TestResult.reset();
	return false;
}



//////////////////////////////////////////////////////////////////////
bool C7C7toSTDF::GotoMarker(const char *szEndMarker)
{
	int iPos;
	QString strLine;
	QString strString;
	QString strSection;
	QString strEndMarker(szEndMarker);


	while(!m_bEndOfFile)
	{

		strLine = readNext();
		if(strLine.isEmpty() || strLine == " ")
			continue;

        iPos = strLine.indexOf('<');
		if(iPos < 0)
			continue;

		// if found a 'tag', probably a keyword
		// remove all space
		strSection = unEncapsuled(strLine).toLower();
		if(strSection == strEndMarker)
			return true;
	}
	return false;
}



//////////////////////////////////////////////////////////////////////
bool C7C7toSTDF::SetLimit(C7C7Test*pclTest, int iIndex, QString strLimitName, 
			  QString strLowUnit, QString strLowValue, 
			  QString strHighUnit, QString strHighValue)
{
	bool	bFloat = false;
	float	fLowValue = 0, fHighValue = 0;
	QString	strValue, strLowParam, strHighParam;

	if(!strLowUnit.isEmpty())
		pclTest->strResultUnit = strLowUnit;
	if((strLowValue.isEmpty() && strHighValue.isEmpty()))// || (strLowUnit.isEmpty() && strHighUnit.isEmpty()))
		return false;

	fLowValue = ToFloat(strLowValue,&bFloat);
	if(!bFloat && !strLowValue.isEmpty()) return false;
	if(!strLowValue.isEmpty() && (strLowValue.left(1) == "@"))
		strLowParam = strLowValue.right(strLowValue.length()-1);

	fHighValue = ToFloat(strHighValue,&bFloat);
	if(!bFloat && !strHighValue.isEmpty()) return false;
	if(!strHighValue.isEmpty() && (strHighValue.left(1) == "@"))
		strHighParam = strHighValue.right(strHighValue.length()-1);
	
	//save
	C7C7Limit* pLimit = pclTest->GetLimit(iIndex,strLimitName);
	pclTest->bLimitSaved |= !pLimit->SetLimit(strLowUnit,fLowValue,strHighUnit,fHighValue,strLowParam,strHighParam);
	
	return true;
}


//////////////////////////////////////////////////////////////////////
C7C7Test* C7C7toSTDF::GetTest(int iTestPrimaryId)
{
	C7C7Test* pclTest;
	if(m_clTest.contains(iTestPrimaryId))
	{	// exist
		QMap<int,C7C7Test*>::Iterator itTest;
        itTest = m_clTest.find(iTestPrimaryId);
        pclTest = itTest.value();
	}
	else
	{	// create it
		pclTest = new C7C7Test();
		pclTest->test_primary_id = iTestPrimaryId;
		m_clTest.insert(iTestPrimaryId, pclTest);
	}
	return pclTest;
}


//////////////////////////////////////////////////////////////////////
C7C7Sbr* C7C7toSTDF::GetSbr(int iSbrId)
{
	C7C7Sbr* pclSbr;
	if(m_clSbrList.contains(iSbrId))
	{	// exist
		QMap<int,C7C7Sbr*>::Iterator itSbr;
        itSbr = m_clSbrList.find(iSbrId);
        pclSbr = itSbr.value();
	}
	else
	{	// create it
		pclSbr = new C7C7Sbr();
		pclSbr->Number = iSbrId;
		pclSbr->SW_Bin = iSbrId;
		m_clSbrList.insert(iSbrId, pclSbr);
	}
	return pclSbr;
}


//////////////////////////////////////////////////////////////////////
// first have to verify if this is a user variable and then try to convert it into float
//////////////////////////////////////////////////////////////////////
float C7C7toSTDF::ToFloat(QString strVar, bool* pbFloat)
{
	QString	strValue;

	strValue = ToValue(strVar);
	return strValue.toFloat(pbFloat);
}



//////////////////////////////////////////////////////////////////////
// try to find if this variable was declared and initialized by the user
// return the current value
//////////////////////////////////////////////////////////////////////
QString C7C7toSTDF::ToValue(QString strVar)
{
	QString	strValue;

	strValue = strVar;
	if(!strValue.isEmpty() && (strValue.left(1) == "@")	&& (m_clVarValue.contains(strValue.right(strValue.length()-1))))
	{	// VARIABLE VALUE
		QMap<QString,QString>::Iterator itVar;
        itVar = m_clVarValue.find(strValue.right(strValue.length()-1));
        strValue = itVar.value();
	}
	return strValue;
}


//////////////////////////////////////////////////////////////////////
// Write STDF file
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
void C7C7toSTDF::WriteFar()
{
    GS::StdLib::StdfRecordReadInfo RecordReadInfo;
	// Write FAR
	RecordReadInfo.iRecordType = 0;
	RecordReadInfo.iRecordSubType = 10;
	m_pStdfFile->WriteHeader(&RecordReadInfo);
	m_pStdfFile->WriteByte(1);					// SUN CPU type
	m_pStdfFile->WriteByte(4);					// STDF V4
	m_pStdfFile->WriteRecord();
}

//////////////////////////////////////////////////////////////////////
void C7C7toSTDF::WriteMir()
{
    GS::StdLib::StdfRecordReadInfo	RecordReadInfo;
	
	// Write MIR
	RecordReadInfo.iRecordType = 1;
	RecordReadInfo.iRecordSubType = 10;
	m_pStdfFile->WriteHeader(&RecordReadInfo);

	if(m_lStartT <= 0)
		m_lStartT = QDateTime::currentDateTime().toTime_t();

	m_pStdfFile->WriteDword(m_lStartT);							// Setup time
	m_pStdfFile->WriteDword(m_lStartT);							// Start time
	m_pStdfFile->WriteByte((BYTE) m_nStatNum);					// Station
	m_pStdfFile->WriteByte((BYTE) m_cModeCod);					// Test Mode
	m_pStdfFile->WriteByte((BYTE) m_cRtstCod);					// rtst_cod
	m_pStdfFile->WriteByte((BYTE) m_cProtCod);					// prot_cod
	m_pStdfFile->WriteWord(m_lBurnTim);							// burn_time
	m_pStdfFile->WriteByte((BYTE) m_cCModCod);					// cmod_cod
	m_pStdfFile->WriteString(m_strLotId.toLatin1().constData());		// Lot ID
	m_pStdfFile->WriteString(m_strPartTyp.toLatin1().constData());	// Part Type / Product ID
	m_pStdfFile->WriteString(m_strNodeNam.toLatin1().constData());	// Node name
	m_pStdfFile->WriteString(m_strEquipmentID.toLatin1().constData());	// Tester Type
	m_pStdfFile->WriteString(m_strJobNam.toLatin1().constData());	// Job name
	m_pStdfFile->WriteString(m_strJobRev.toLatin1().constData());	// job_rev
	m_pStdfFile->WriteString(m_strSbLotId.toLatin1().constData());	// sblot_id
	m_pStdfFile->WriteString(m_strOperNam.toLatin1().constData());	// oper_nam
	m_pStdfFile->WriteString(m_strExecTyp.toLatin1().constData());	// exec_typ
	m_pStdfFile->WriteString(m_strExecVer.toLatin1().constData());	// exec_ver
	m_pStdfFile->WriteString(m_strTestCod.toLatin1().constData());	// test_cod
	m_pStdfFile->WriteString(m_strTstTemp.toLatin1().constData());	// tst_temp
	// Construct custom Galaxy USER_TXT 
	QString	strUserTxt;
	strUserTxt = GEX_IMPORT_DATAORIGIN_LABEL;	
	strUserTxt += ":";
	strUserTxt += m_strDataOrigin;
	strUserTxt += ":7C7";
	m_pStdfFile->WriteString(strUserTxt.toLatin1().constData());		// user_txt
	m_pStdfFile->WriteString(m_strAuxFile.toLatin1().constData());	// aux_file
	m_pStdfFile->WriteString(m_strPckTyp.toLatin1().constData());	// pkg_typ
	m_pStdfFile->WriteString(m_strFamlyId.toLatin1().constData());	// famly_id
	m_pStdfFile->WriteString(m_strDateCod.toLatin1().constData());	// date_cod
	m_pStdfFile->WriteString(m_strFacilId.toLatin1().constData());	// facil_id
	m_pStdfFile->WriteString(m_strFloorId.toLatin1().constData());	// floor_id
	m_pStdfFile->WriteString(m_strProcId.toLatin1().constData());	// proc_id
	m_pStdfFile->WriteString(m_strOperFrq.toLatin1().constData());	// oper_frq
	m_pStdfFile->WriteString(m_strSpecNam.toLatin1().constData());	// spec_nam
	m_pStdfFile->WriteString(m_strSpecVer.toLatin1().constData());	// spec_ver
	m_pStdfFile->WriteString(m_strFlowId.toLatin1().constData());	// flow_id
	m_pStdfFile->WriteString(m_strSetupId.toLatin1().constData());	// setup_id
	m_pStdfFile->WriteString(m_strDsgnRev.toLatin1().constData());	// dsgn_rev
	m_pStdfFile->WriteString(m_strEngId.toLatin1().constData());		// eng_id
	m_pStdfFile->WriteString(m_strRomCod.toLatin1().constData());	// rom_cod
	m_pStdfFile->WriteString(m_strSerlNum.toLatin1().constData());	// serl_num
	m_pStdfFile->WriteString(m_strSuprNam.toLatin1().constData());	// supr_nam
	
	m_pStdfFile->WriteRecord();

	m_bMirWriten = true;
}



//////////////////////////////////////////////////////////////////////
void C7C7toSTDF::WriteMrr()
{
    GS::StdLib::StdfRecordReadInfo RecordReadInfo;
	// Write MRR
	RecordReadInfo.iRecordType = 1;
	RecordReadInfo.iRecordSubType = 20;
	m_pStdfFile->WriteHeader(&RecordReadInfo);
	m_pStdfFile->WriteDword(m_lLastPartTime);				// Date and time last part tested
	m_pStdfFile->WriteByte(m_nDispCode);			// Lot disposition code
	m_pStdfFile->WriteString(m_strUserDesc.toLatin1().constData());// Lot description supplied by user
	m_pStdfFile->WriteString(m_strExecDesc.toLatin1().constData());// Lot description supplied by exec
	m_pStdfFile->WriteRecord();
}


//////////////////////////////////////////////////////////////////////
void C7C7toSTDF::WritePcr()
{
	// NO PCR FOR ETEST
	if(m_strDataOrigin == GEX_IMPORT_DATAORIGIN_ETEST)
		return;

    GS::StdLib::StdfRecordReadInfo RecordReadInfo;
	// Write PCR
	RecordReadInfo.iRecordType = 1;
	RecordReadInfo.iRecordSubType = 30;
	m_pStdfFile->WriteHeader(&RecordReadInfo);
	m_pStdfFile->WriteByte(1);						// Test head number
	m_pStdfFile->WriteByte(1);						// Test site number
	if(m_nTotalParts > -1) m_nTotalParts += 2;		// the variable was initialize to -1
	m_pStdfFile->WriteDword(m_nTotalParts);			// Number of parts tested
	m_pStdfFile->WriteDword((DWORD)(-1));			// Number of parts retested
	m_pStdfFile->WriteDword((DWORD)(-1));			// Number of aborts during testing
	if(m_nTotalPassed > -1) m_nTotalPassed += 2;	// the variable was initialize to -1
	m_pStdfFile->WriteDword(m_nTotalPassed);		// Number of good (passed) parts tested
	m_pStdfFile->WriteDword((DWORD)(-1));			// Number of functional parts tested
	m_pStdfFile->WriteRecord();
}


//////////////////////////////////////////////////////////////////////
void C7C7toSTDF::SaveSbr()
{

	C7C7Sbr* pclSbr = GetSbr(m_cl7C7Summary.SW_Bin);
	pclSbr->HW_Bin = m_cl7C7Summary.HW_Bin;
	pclSbr->P_F = m_cl7C7Summary.P_F;
	pclSbr->Number = m_cl7C7Summary.Number;
	pclSbr->Prcnt = m_cl7C7Summary.Prcnt;
	pclSbr->Bin_desc = m_cl7C7Summary.Bin_desc;
}


//////////////////////////////////////////////////////////////////////
void C7C7toSTDF::WriteSbr()
{
	// NO SBR FOR ETEST
	if(m_strDataOrigin == GEX_IMPORT_DATAORIGIN_ETEST)
		return;

	C7C7Sbr*			pSbr;
    GS::StdLib::StdfRecordReadInfo	RecordReadInfo;
	RecordReadInfo.iRecordType = 1;
	RecordReadInfo.iRecordSubType = 50;

    QMap<int,C7C7Sbr*>::Iterator itSbrList;
	for ( itSbrList = m_clSbrList.begin(); itSbrList != m_clSbrList.end(); ++itSbrList ) 
	{
         pSbr = itSbrList.value();
		if(pSbr == NULL)
			return;

		// Write SBR
		m_pStdfFile->WriteHeader(&RecordReadInfo);
		m_pStdfFile->WriteByte(255);					// Test Head = ALL
		m_pStdfFile->WriteByte(1);						// Test sites = ALL		
		m_pStdfFile->WriteWord(pSbr->SW_Bin);			// SBIN = 0
		m_pStdfFile->WriteDword(pSbr->Prcnt);			// Total Bins
		if(pSbr->P_F)
			m_pStdfFile->WriteByte('P');
		else
			m_pStdfFile->WriteByte('F');
		m_pStdfFile->WriteString(pSbr->Bin_desc.toLatin1().constData());
		m_pStdfFile->WriteRecord();
	}
}


//////////////////////////////////////////////////////////////////////
void C7C7toSTDF::WriteHbr()
{

	// NO HBR FOR ETEST
	if(m_strDataOrigin == GEX_IMPORT_DATAORIGIN_ETEST)
		return;

	C7C7Sbr*			pSbr;
    GS::StdLib::StdfRecordReadInfo	RecordReadInfo;
	RecordReadInfo.iRecordType = 1;
	RecordReadInfo.iRecordSubType = 40;

    QMap<int,C7C7Sbr*>::Iterator itSbrList;
	for ( itSbrList = m_clSbrList.begin(); itSbrList != m_clSbrList.end(); ++itSbrList ) 
	{
         pSbr = itSbrList.value();
		if(pSbr == NULL)
			return;

		// Write SBR
		m_pStdfFile->WriteHeader(&RecordReadInfo);
		m_pStdfFile->WriteByte(255);					// Test Head = ALL
		m_pStdfFile->WriteByte(1);						// Test sites = ALL		
		m_pStdfFile->WriteWord(pSbr->HW_Bin);			// SBIN = 0
		m_pStdfFile->WriteDword(pSbr->Prcnt);			// Total Bins
		if(pSbr->P_F)
			m_pStdfFile->WriteByte('P');
		else
			m_pStdfFile->WriteByte('F');
		m_pStdfFile->WriteString(pSbr->Bin_desc.toLatin1().constData());
		m_pStdfFile->WriteRecord();
	}
}


//////////////////////////////////////////////////////////////////////
void C7C7toSTDF::WritePmr()
{
    GS::StdLib::StdfRecordReadInfo RecordReadInfo;
	// Write PMR
	RecordReadInfo.iRecordType = 1;
	RecordReadInfo.iRecordSubType = 60;

	C7C7PinChannel* pcl7C7PinChannel;
    QMap<int,C7C7PinChannel*>::Iterator it;
	for ( it = m_clPinChannel.begin(); it != m_clPinChannel.end(); ++it ) 
	{
        pcl7C7PinChannel = it.value();
		for (QStringList::const_iterator
			 iter  = pcl7C7PinChannel->lstChannel.begin();
			 iter != pcl7C7PinChannel->lstChannel.end(); ++iter) {
			m_pStdfFile->WriteHeader(&RecordReadInfo);
			m_pStdfFile->WriteWord(pcl7C7PinChannel->iBitPosition);
			m_pStdfFile->WriteWord(0);
			m_pStdfFile->WriteString((*iter).toLatin1().constData());
			m_pStdfFile->WriteString(pcl7C7PinChannel->strPinName.toLatin1().constData());
			m_pStdfFile->WriteString("");
			m_pStdfFile->WriteByte(1);						// Test head number
			m_pStdfFile->WriteByte(1);						// Test site number
			m_pStdfFile->WriteRecord();
		}
		delete pcl7C7PinChannel;
	}
	m_clPinChannel.clear();
}

//////////////////////////////////////////////////////////////////////
void C7C7toSTDF::WriteWir()
{
    GS::StdLib::StdfRecordReadInfo RecordReadInfo;
	// Write WIR
	RecordReadInfo.iRecordType = 2;
	RecordReadInfo.iRecordSubType = 10;
	m_pStdfFile->WriteHeader(&RecordReadInfo);
	m_pStdfFile->WriteByte(1);						// Test head number
	m_pStdfFile->WriteByte(255);					// Site group number
	m_pStdfFile->WriteDword(m_lLastPartTime);		// Time first part tested
	if(m_nTotalWafers > 0)
		m_lLastPartTime += (m_lEndT - m_lStartT)/(m_nTotalWafers);
	if(m_lLastPartTime > (m_lEndT-m_nTotalWafers))	
		m_lLastPartTime = m_lEndT;
	m_pStdfFile->WriteString(m_strWaferId.toLatin1().constData());	// WaferID
	m_pStdfFile->WriteRecord();
}


//////////////////////////////////////////////////////////////////////
void C7C7toSTDF::WriteWrr()
{
    GS::StdLib::StdfRecordReadInfo RecordReadInfo;
	// Write WRR
	RecordReadInfo.iRecordType = 2;
	RecordReadInfo.iRecordSubType = 20;
	m_pStdfFile->WriteHeader(&RecordReadInfo);
	m_pStdfFile->WriteByte(1);					// Test head number
	m_pStdfFile->WriteByte(255);				// Site group number
	m_pStdfFile->WriteDword(m_lLastPartTime);	// Date and time last part tested
	m_pStdfFile->WriteDword(m_nWaferNumDie);	// Number of parts tested
	m_pStdfFile->WriteDword(m_nWaferNumRetested);// Number of parts retested
	m_pStdfFile->WriteDword((DWORD)(-1));		// Number of aborted during testing
	m_pStdfFile->WriteDword(m_nWaferNumPassed);	// Number of good (passed) parts tested
	m_pStdfFile->WriteDword((DWORD)(-1));		// Number of functional parts tested
	m_pStdfFile->WriteString(m_strWaferId.toLatin1().constData());	// WaferID
	m_pStdfFile->WriteString("");				// Fab wafer Id
	m_pStdfFile->WriteString("");				// Wafer frame Id
	m_pStdfFile->WriteString("");				// Wafer mask Id
	m_pStdfFile->WriteString(m_strUserDesc.toLatin1().constData());// Wafer description supplied by user
	m_pStdfFile->WriteString("");				// Wafer description supplied by exec
	m_pStdfFile->WriteRecord();
}


//////////////////////////////////////////////////////////////////////
void C7C7toSTDF::WritePir()
{
    GS::StdLib::StdfRecordReadInfo RecordReadInfo;
	// Write PIR
	RecordReadInfo.iRecordType = 5;
	RecordReadInfo.iRecordSubType = 10;
	m_pStdfFile->WriteHeader(&RecordReadInfo);
	m_pStdfFile->WriteByte(1);						// Test head
	m_pStdfFile->WriteByte(1);						// Test site number
	m_pStdfFile->WriteRecord();
}

//////////////////////////////////////////////////////////////////////
void C7C7toSTDF::WritePrr()
{
	char	bit_field=0;
	QString	strString;
    GS::StdLib::StdfRecordReadInfo RecordReadInfo;


	if(m_cl7C7DutResult.nFlags == WAFER_ID)
	{
		strString.sprintf("%d", m_nWaferNumDie);
		if(m_cl7C7DutResult.Retest == 1)
			bit_field = BIT1;
	}
	else
	{
		strString = "";
		if(m_cl7C7DutResult.Pkg_id != -1)
			strString.sprintf("%d",m_cl7C7DutResult.Pkg_id);
		if(m_cl7C7DutResult.Retest == 1)
			bit_field = BIT0;
	}
	
	if(m_cl7C7TestResult.Result)
		bit_field |= BIT3;

	int		HW_Bin = m_cl7C7DutResult.HW_Bin;
	if(HW_Bin < 0)
		HW_Bin = 0;

	// Write PRR
	RecordReadInfo.iRecordType = 5;
	RecordReadInfo.iRecordSubType = 20;
	m_pStdfFile->WriteHeader(&RecordReadInfo);
	m_pStdfFile->WriteByte(1);							// Test head
	m_pStdfFile->WriteByte(1);							// Tester site:1,2,3,4 or 5
	m_pStdfFile->WriteByte(bit_field);					// PART_FLG : PASSED
	m_pStdfFile->WriteWord(m_cl7C7DutResult.TT);		// NUM_TEST
	m_pStdfFile->WriteWord(HW_Bin);						// HARD_BIN
	m_pStdfFile->WriteWord(m_cl7C7DutResult.SW_Bin);	// SOFT_BIN
	m_pStdfFile->WriteWord(m_cl7C7DutResult.X);			// X_COORD
	m_pStdfFile->WriteWord(m_cl7C7DutResult.Y);			// Y_COORD
	m_pStdfFile->WriteDword(0);							// No testing time known...
	m_pStdfFile->WriteString(strString.toLatin1().constData());	// PART_ID
	m_pStdfFile->WriteString(m_cl7C7DutResult.Bin_desc.toLatin1().constData());// PART_TXT
	m_pStdfFile->WriteString("");						// PART_FIX

	m_pStdfFile->WriteRecord();
}

//////////////////////////////////////////////////////////////////////
void C7C7toSTDF::WriteTsr()
{
	// NO TSR FOR ETEST
	if(m_strDataOrigin == GEX_IMPORT_DATAORIGIN_ETEST)
		return;

    GS::StdLib::StdfRecordReadInfo RecordReadInfo;
	// Write TSR
	RecordReadInfo.iRecordType = 10;
	RecordReadInfo.iRecordSubType = 30;

    QMap<int,C7C7Test*>::Iterator itTest;
	C7C7Test*	pclTest;
	QMap<QString,C7C7Pin*>::Iterator itPin;
	C7C7Pin*	pclPin;
	for ( itTest = m_clTest.begin(); itTest != m_clTest.end(); ++itTest ) 
	{
        pclTest = itTest.value();
		for ( itPin = pclTest->pin_list.begin(); itPin != pclTest->pin_list.end(); ++itPin ) 
		{
            pclPin = itPin.value();
			m_pStdfFile->WriteHeader(&RecordReadInfo);

 			m_pStdfFile->WriteByte(255);				// Test head
			m_pStdfFile->WriteByte(1);					// Tester site
			if(pclTest->bFunctional)
				m_pStdfFile->WriteByte('F');			// Functional
			else
				m_pStdfFile->WriteByte('P');			// Parametric
			m_pStdfFile->WriteDword(pclPin->test_num);	// Test Number
			m_pStdfFile->WriteDword(pclPin->results.exec_cnt);// Number of test executions
			m_pStdfFile->WriteDword(pclPin->results.fail_cnt);// Number of test failures
			m_pStdfFile->WriteDword((DWORD)(-1));		// Number of alarmed tests
			m_pStdfFile->WriteString(pclTest->test_name.toLatin1().constData());// TEST_NAM
			m_pStdfFile->WriteString(pclTest->seq_name.toLatin1().constData());// SEQ_NAME
			m_pStdfFile->WriteString("");				// TEST_LBL
			m_pStdfFile->WriteByte(BIT0|BIT1|BIT2|BIT3|BIT4|BIT5|BIT6|BIT7);// OPT_FLG
			m_pStdfFile->WriteFloat(0);					// Values
			m_pStdfFile->WriteFloat(0);					// Values
			m_pStdfFile->WriteFloat(0);					// Values
			m_pStdfFile->WriteFloat(0);					// Values
			m_pStdfFile->WriteFloat(0);					// Values

			m_pStdfFile->WriteRecord();
		}
	}

}


//////////////////////////////////////////////////////////////////////
void C7C7toSTDF::WritePtr(C7C7TestResult* pclTestResultParam)
{
	C7C7TestResult* pclTestResult;
	if(pclTestResultParam)
		pclTestResult = pclTestResultParam;
	else
		pclTestResult = &m_cl7C7TestResult;
    GS::StdLib::StdfRecordReadInfo RecordReadInfo;
	// Write PTR
	RecordReadInfo.iRecordType = 15;
	RecordReadInfo.iRecordSubType = 10;

	int			iExp;
	bool		bHaveLimit = false;
	float		fFloat;
	BYTE		bData;
	QString		strUnit;
	C7C7Pin*	pclPin = NULL;
	C7C7Test*	pclTest;
	C7C7Limit*	pclLimit=NULL;

	if(pclTestResult->Result > 1) // bad result (PFNU)
		return;
	
	pclTest = GetTest(pclTestResult->test_primary_id);
	if(pclTest->test_primary_id == 0)
	{
		// is a new test
		// save the current test_num
		pclTest->test_primary_id = pclTestResult->test_primary_id;
	}
	else
		m_cl7C7TestResult.test_primary_id = pclTest->test_primary_id;
	// save information
	pclTest->seq_name = pclTestResult->seq_name;
	pclTest->bFunctional = false;
	for (int i = 0; i < pclTestResult->ResUnit.count(); i++) {
		bHaveLimit = false;
		if(!pclTest->bLimitSaved)
			bHaveLimit = pclTest->HaveLimit(i/2,pclTestResult->PinName);
		if(bHaveLimit)
			pclLimit = pclTest->GetLimit(i/2,pclTestResult->PinName);
		pclPin = pclTest->GetPin(i/2,pclTestResult->PinName); 
		pclPin->test_num = pclTestResult->test_num;
		pclPin->results.exec_cnt++;
		if(!pclTestResult->Result)
			pclPin->results.fail_cnt++;

		m_pStdfFile->WriteHeader(&RecordReadInfo);
		
		m_pStdfFile->WriteDword(pclTestResult->test_num);		// Test Number
		m_pStdfFile->WriteByte(1);								// Test head
		m_pStdfFile->WriteByte(1);								// Tester site
		fFloat = ToFloat(pclTestResult->ResUnit[i++]);
		iExp = PrefixUnitToScall_7C7(pclTestResult->strResultUnit);
		fFloat /= GS_POW(10.0,iExp);

		strUnit = PrefixUnitToUnit_7C7(pclTestResult->strResultUnit);
		if(pclTestResult->Result)
			m_pStdfFile->WriteByte(0);		// passed			// TEST_FLG
		else
			m_pStdfFile->WriteByte(BIT7);	// failed			// TEST_FLG
		bData = 0;
		if(m_strDataOrigin == GEX_IMPORT_DATAORIGIN_ETEST)
			bData = BIT6|BIT7;									// PARAM_FLG
		m_pStdfFile->WriteByte(bData);							// PARAM_FLG
		m_pStdfFile->WriteFloat(fFloat);						// Test result
		m_pStdfFile->WriteString(pclTestResult->test_name.toLatin1().constData());		// TEST_TXT
		m_pStdfFile->WriteString("");							// ALARM_ID

		if(!pclTest->bLimitSaved)
		{
			pclTest->bLimitSaved = true;
			if(bHaveLimit)
			{
				if(iExp == 0)
					m_pStdfFile->WriteByte(BIT0|BIT1);			// OPT_FLAG
				else
					m_pStdfFile->WriteByte(BIT1);				// OPT_FLAG
				m_pStdfFile->WriteByte(iExp);					// SCAL
				m_pStdfFile->WriteByte(iExp);					//(pclLimit->llm_scal);		// SCAL
				m_pStdfFile->WriteByte(iExp);					//(pclLimit->hlm_scal);		// SCAL
				m_pStdfFile->WriteFloat(pclLimit->lo_limit);	// LIMIT
				m_pStdfFile->WriteFloat(pclLimit->hi_limit);	// LIMIT
				m_pStdfFile->WriteString(strUnit.toLatin1().constData());				// TEST_UNIT
				m_pStdfFile->WriteString("");					// 
				m_pStdfFile->WriteString("");					// 
				m_pStdfFile->WriteString("");					// 
				m_pStdfFile->WriteFloat(pclLimit->lo_spec);		// LIMIT
				m_pStdfFile->WriteFloat(pclLimit->hi_spec);		// LIMIT
			}
			else if(!pclTestResult->strResultUnit.isEmpty())
			{
				m_pStdfFile->WriteByte(BIT1|BIT2|BIT3|BIT4|BIT5|BIT6|BIT7);// OPT_FLAG
				m_pStdfFile->WriteByte(iExp);					// SCAL
				m_pStdfFile->WriteByte(0);						// SCAL
				m_pStdfFile->WriteByte(0);						// SCAL
				m_pStdfFile->WriteFloat(0);						// LIMIT
				m_pStdfFile->WriteFloat(0);						// LIMIT
				m_pStdfFile->WriteString(strUnit.toLatin1().constData());				// TEST_UNIT
				m_pStdfFile->WriteString("");					// 
				m_pStdfFile->WriteString("");					// 
				m_pStdfFile->WriteString("");					// 
				m_pStdfFile->WriteFloat(0);						// LIMIT
				m_pStdfFile->WriteFloat(0);						// LIMIT
			}
		}

		m_pStdfFile->WriteRecord();
	}

			

}


//////////////////////////////////////////////////////////////////////
void C7C7toSTDF::WriteFtr()
{
    GS::StdLib::StdfRecordReadInfo RecordReadInfo;
	// Write FTR
	RecordReadInfo.iRecordType = 15;
	RecordReadInfo.iRecordSubType = 20;

	if(m_cl7C7TestResult.Result > 1)					// bad result (PFNU)
		return;
	
	m_pStdfFile->WriteHeader(&RecordReadInfo);
	m_pStdfFile->WriteDword(m_cl7C7TestResult.test_num);// Test Number
	m_pStdfFile->WriteByte(1);							// Test head
	m_pStdfFile->WriteByte(1);							// Tester site:1,2,3,4 or 5, etc.
	if(m_cl7C7TestResult.Result)
		m_pStdfFile->WriteByte(0);		// passed			// TEST_FLG
	else
		m_pStdfFile->WriteByte(BIT7);	// failed

	// save empty fields for report_readfile.cpp
	m_pStdfFile->WriteByte(255);	// opt_flg
	m_pStdfFile->WriteDword(0);		// cycl_cnt
	m_pStdfFile->WriteDword(0);		// rel_vadr
	m_pStdfFile->WriteDword(0);		// rept_cnt
	m_pStdfFile->WriteDword(0);		// num_fail
	m_pStdfFile->WriteDword(0);		// xfail_ad
	m_pStdfFile->WriteDword(0);		// yfail_ad
	m_pStdfFile->WriteWord(0);		// vect_off
	m_pStdfFile->WriteWord(0);		// rtn_icnt
	m_pStdfFile->WriteWord(0);
	m_pStdfFile->WriteWord(0);
	m_pStdfFile->WriteString("");	// vect_name
	m_pStdfFile->WriteString("");	// time_set
	m_pStdfFile->WriteString("");	// op_code
	m_pStdfFile->WriteString(m_cl7C7TestResult.test_name.toLatin1().constData());	// test_txt: test name
	m_pStdfFile->WriteString("");	// alarm_id
	m_pStdfFile->WriteString("");	// prog_txt
	m_pStdfFile->WriteString("");	// rslt_txt
	m_pStdfFile->WriteByte(0);		// patg_num
	m_pStdfFile->WriteString("");	// spin_map

	m_pStdfFile->WriteRecord();
				
	// save information
	C7C7Test* pclTest = GetTest(m_cl7C7TestResult.test_primary_id);
	if(pclTest->test_num == 0)
		// is a new test
		// save the current test_num
		pclTest->test_num = m_cl7C7TestResult.test_num;
	else
		m_cl7C7TestResult.test_num = pclTest->test_num;
	pclTest->seq_name = m_cl7C7TestResult.seq_name;
	pclTest->bFunctional = (m_cl7C7TestResult.ResUnit.count() == 0);
	C7C7Pin* pPin = pclTest->GetPin(0,m_cl7C7TestResult.PinName); // take the good pin
	pPin->test_num = m_cl7C7TestResult.test_num;
	pPin->results.exec_cnt++;
	if(!m_cl7C7TestResult.Result)
		pPin->results.fail_cnt++;
			

}


//////////////////////////////////////////////////////////////////////
// Structures
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
// C7C7DutResult
//////////////////////////////////////////////////////////////////////
C7C7DutResult::C7C7DutResult()
{
	nFlags = WAFER_ID;
	bSite = false;
	reset();
}


void C7C7DutResult::reset()
{
 	X		= -32768;
	Y		= -32768;
	Pkg_id	= -1;
	SW_Bin	= -1;
	HW_Bin	= -1;
	P_F		= -1;
	Time	= -1;
	Temp	= -1;
	FF		= 0;
	TF		= 0;
	TT		= 0;
	Site	= -1;
	Bin_desc= " ";
	Retest	= 0;
}

//////////////////////////////////////////////////////////////////////
// C7C7TestResult
//////////////////////////////////////////////////////////////////////
C7C7TestResult::C7C7TestResult()
{
	test_num	= 0;
	test_primary_id	= 0;
	test_name	= "";
	seq_name	= "";
	reset();
}

void C7C7TestResult::reset()
{
	//test_name	= ""; // save the last result for pin data
	//seq_name	= "";
	Result		= -1;
	ResUnit.clear();
	strResultUnit="";
	PinName		= "";
	LimitName	= "";
};

//////////////////////////////////////////////////////////////////////
// C7C7Summary
//////////////////////////////////////////////////////////////////////
C7C7Summary::C7C7Summary()
{
	NumberOfSite = 0;
	pSite = NULL;
	reset();
}

void C7C7Summary::reset()
{
	int i;
	SW_Bin	= -1;
	HW_Bin	= -1;
	P_F		= -1;
	Number	= -1;
	Prcnt	= 0;
	Bin_desc= " ";

	for(i=0; i<NumberOfSite;i++)
		pSite[i]=0;
}

//////////////////////////////////////////////////////////////////////
// C7C7Sbr
//////////////////////////////////////////////////////////////////////
C7C7Sbr::C7C7Sbr(int iSW_Bin,int iHW_Bin,int iP_F,int iNumber,int nPrcnt,QString strBin_desc)
{
	SW_Bin		= iSW_Bin;
	HW_Bin		= iHW_Bin;
	P_F			= iP_F;
	Number		= iNumber;
	Prcnt		= nPrcnt;
	Bin_desc	= strBin_desc;
}

//////////////////////////////////////////////////////////////////////
// C7C7PartCnt
//////////////////////////////////////////////////////////////////////
C7C7PartCnt::C7C7PartCnt()
{
	bSite = false;
	reset();
}
void C7C7PartCnt::reset()
{
	Site	= -1;
	Number	= -1;
	Prcnt	= -1;
}

//////////////////////////////////////////////////////////////////////
// C7C7Test
//////////////////////////////////////////////////////////////////////
C7C7Test::C7C7Test()
{
	bLimitSaved	= false;
	test_primary_id	= 0;
	test_num	= 0;
    test_name	= "";
	strResultUnit="";
    seq_name	= "";
    spreadFlag	= 0;
    bFunctional	= true;
}


C7C7Test::~C7C7Test()
{
    QMap<QString,C7C7Pin*>::Iterator itPin;
	for ( itPin = pin_list.begin(); itPin != pin_list.end(); ++itPin ) 
	{
        delete itPin.value();
	}
	pin_list.clear();
    QMap<QString,C7C7Limit*>::Iterator itLimit;
	for ( itLimit = limit_list.begin(); itLimit != limit_list.end(); ++itLimit ) 
    {
        delete itLimit.value();
	}
	limit_list.clear();
}

C7C7Pin* C7C7Test::GetPin(int iIndex, QString strPinName)
{
	QString		strKey;
	C7C7Pin*	pPin;
	
	strKey.sprintf("%d",iIndex);
	strKey += strPinName;
	if(pin_list.contains(strKey))
	{	// exist
		QMap<QString,C7C7Pin*>::Iterator itPin;
        itPin = pin_list.find(strKey);
        pPin = itPin.value();
	}
	else
	{	// create it
		pPin = new C7C7Pin();
		pin_list.insert(strKey,pPin);
		pPin->testNameSuffix = strKey;
	}
	return pPin;
}

C7C7Limit* C7C7Test::GetLimit(int iIndex, QString strLimitName)
{
	QString		strKey;
	C7C7Limit* pLimit = NULL;

	strKey.sprintf("%d",iIndex);
	strKey += strLimitName;
	if(limit_list.contains(strKey))
	{	// exist
		QMap<QString,C7C7Limit*>::Iterator itLimit;
        itLimit = limit_list.find(strKey);
        pLimit = itLimit.value();
	}
	else
	{	// create it
		pLimit = new C7C7Limit();
		limit_list.insert(strKey,pLimit);
		pLimit->limitName = strLimitName;
	}
	return pLimit;
}

bool C7C7Test::HaveLimit(int iIndex, QString strLimitName)
{
	QString strKey;

	strKey.sprintf("%d",iIndex);
	strKey += strLimitName;

	return limit_list.contains(strKey);
}

//////////////////////////////////////////////////////////////////////
// C7C7Results
//////////////////////////////////////////////////////////////////////
C7C7Results::C7C7Results()
{
    exec_cnt	= 0;
    fail_cnt	= 0;
    alarm_tests	= 0;
    opt_flag	= 0;
    pad_byte	= 0;
    test_min	= 0;
    test_max	= 0;
    tst_mean	= 0;
    tst_sdev	= 0;
    tst_sums	= 0;
    tst_sqrs	= 0;
}

//////////////////////////////////////////////////////////////////////
// C7C7Pin
//////////////////////////////////////////////////////////////////////
C7C7Pin::C7C7Pin()
{
    testNameSuffix = "";
    test_num = 0;
}

//////////////////////////////////////////////////////////////////////
// C7C7Limit
//////////////////////////////////////////////////////////////////////
C7C7Limit::C7C7Limit()
{
    opt_flag	= ' ';
    units		= "";
    llm_scal	= 0;
    hlm_scal	= 0;
    lo_limit	= 0;
    low_limit_param="";
    hi_limit	= 0;
    high_limit_param="";
    lo_spec		= 0;
    hi_spec		= 0;
    limitName	= "";
}

bool C7C7Limit::SetLimit(QString strLowUnits, float fLowValue, 
						  QString strHighUnits, float fHighValue, 
						  QString strLowParam, QString strHighParam)
{
	bool	bChanged = false;
    float	low_scale_exp, high_scale_exp;
    float	low_limit, high_limit;
	QString	strUnit;

    low_scale_exp = PrefixUnitToScall_7C7(strLowUnits);
    high_scale_exp = PrefixUnitToScall_7C7(strHighUnits);

    low_limit = fLowValue;
    high_limit = fHighValue;
  
	low_limit /= GS_POW(10.0,low_scale_exp);
	high_limit /= GS_POW(10.0,high_scale_exp);
	
    if(!strLowUnits.isEmpty()) 
    {
		opt_flag &= (~BIT4&~BIT6);	/* low limit is valid */
		strUnit = PrefixUnitToUnit_7C7(strLowUnits);
    }
	else
		opt_flag |= (BIT4|BIT6);	/* low limit is invalid */
    if(!strHighUnits.isEmpty()) 
    {
		opt_flag &= (~BIT5&~BIT7);	/* high limit is valid */
		strUnit = PrefixUnitToUnit_7C7(strHighUnits);
    }
	else
		opt_flag |= (BIT5|BIT7);	/* high limit is invalid */

	bChanged = ((units != strUnit) || (low_limit_param != strLowParam) || (high_limit_param != strHighParam) 
			 || (lo_limit != low_limit) || (hi_limit != high_limit));
	units = strUnit;
	low_limit_param = strLowParam;
	high_limit_param = strHighParam;
    llm_scal = 0;
    hlm_scal = 0;
    lo_limit = low_limit;
    hi_limit = high_limit;
    lo_spec = 0;
    hi_spec = 0;

	return bChanged;
}

//////////////////////////////////////////////////////////////////////
// C7C7PinChannel
//////////////////////////////////////////////////////////////////////
C7C7PinChannel::C7C7PinChannel()
{
	reset();
}

C7C7PinChannel::~C7C7PinChannel()
{
	reset();
}

void C7C7PinChannel::reset()
{
	iBitPosition = 0;
	lstChannel.clear();
	strPinName = "";

}


//////////////////////////////////////////////////////////////////////
// Utils
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
int PrefixUnitToScall_7C7(QString strUnit)
{
	if(strUnit.length() < 2) return 0;
	
	QString strPrefix = strUnit.left(1);
	if(strPrefix == "T") return -12;
	if(strPrefix == "G") return -9;
	if(strPrefix == "M") return -6;
	if(strPrefix == "K") return -3;
	if(strPrefix == "%") return 2;
	if(strPrefix == "m") return 3;
	if(strPrefix == "u") return 6;
	if(strPrefix == "n") return 9;
	if(strPrefix == "p") return 12;
	if(strPrefix == "f") return 15;

	return 0;
}

//////////////////////////////////////////////////////////////////////
QString PrefixUnitToUnit_7C7(QString strUnit)
{
	QString strPrefix;
	if(strUnit.length() < 2)
		return strUnit;
	
	strPrefix = strUnit.left(2);
	if((strPrefix == "EX") || (strPrefix == "PE"))
		return strUnit.right(strUnit.length()-2);

	strPrefix = strUnit.left(1);

	if((strPrefix == "T") || (strPrefix == "G") || (strPrefix == "M") || (strPrefix == "K")
	|| (strPrefix == "%") || (strPrefix == "m") || (strPrefix == "u") || (strPrefix == "n") 
	|| (strPrefix == "p") || (strPrefix == "f") || (strPrefix == "a"))
		return strUnit.right(strUnit.length()-1);;

	return strUnit;
}

//////////////////////////////////////////////////////////////////////
int	StringToHex_7C7(QString strHexa)
{
	int	nInteger = 0;
	if(!strHexa.isEmpty())
		sscanf(strHexa.toLatin1().constData(),"%x",&nInteger);
	return nInteger;
}


