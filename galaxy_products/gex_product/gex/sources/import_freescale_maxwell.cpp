//////////////////////////////////////////////////////////////////////
// import_freescale_maxwell.cpp: Convert a Freescale (Process control Monitor).csv
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

#include "browser_dialog.h"
#include "engine.h"
#include "import_freescale_maxwell.h"
#include "import_constants.h"
#include "product_info.h"

#include "converter_external_file.h"

//
//Lot,Start Date,Start Time,S/N,Site,Index,Outcome,Failed,Duration,Measure SCL Neg[Open Short Test],...
//Trimming Lower Limit,,,,,,,,,-1.9,0.3,-1.9,0.3,-1.9,-1.9,-1.9,0.3,-1.9,196,196,4,,,1950000,,,31500...
//Trimming Upper Limit,,,,,,,,,-0.3,1.9,-0.3,1.9,-0.3,-0.3,-0.3,1.9,-0.3,196,196,4,,,2050000,,,32500...
//XDBD285900_FT,3/28/2012,43:31.1,11,1,1,Failed,Pre-Reset Offset Y Std Deviation,2.092,-0.863980676,...
//XDBD285900_FT,3/28/2012,43:31.1,10,0,2,Passed,None,8.079,-0.81043972,0.91115417,-0.89342151,1.0228...

// main.cpp
extern QLabel             *GexScriptStatusLabel;  // Handle to script status text in status bar
extern QProgressBar	*     GexProgressBar;         // Handle to progress bar in status bar
extern GexMainwindow      *pGexMainWindow;



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGFreescaletoSTDF::CGFreescaletoSTDF()
{
  // Default: Freescale parameter list on disk includes all known Freescale parameters...
  m_bNewFreescaleParameterFound = false;
  m_lStartTime = 0;
  m_bHaveBinmapFile = false;

  m_pCGFreescaleParameter = NULL;
}

//////////////////////////////////////////////////////////////////////
// Destruction
//////////////////////////////////////////////////////////////////////
CGFreescaletoSTDF::~CGFreescaletoSTDF()
{
  // Destroy list of Parameters tables.
  if(m_pCGFreescaleParameter!=NULL)
    delete [] m_pCGFreescaleParameter;
}

//////////////////////////////////////////////////////////////////////
// Get Error
//////////////////////////////////////////////////////////////////////
QString CGFreescaletoSTDF::GetLastError()
{
  QString strErrorMessage = "Import Freescale Maxwell: ";

  switch(iLastError)
  {
  default:
  case errNoError:
    strErrorMessage = "";
    break;
  case errOpenFail:
    strErrorMessage += "Failed to open file";
    break;
  case errInvalidFormat:
    strErrorMessage += "Invalid file format";
    break;
  case errInvalidFormatLowInRows:
    strErrorMessage += "Invalid file format: 'Parameter' line too short, missing rows";
    break;
  case errNoLimitsFound:
    strErrorMessage += "Invalid file format: Specification Limits not found";
    break;
  case errMissingData:
    strErrorMessage += "Missing mandatory info";
    break;
  case errWriteSTDF:
    strErrorMessage += "Failed creating temporary file. Folder permission issue?";
    break;
  case errLicenceExpired:
    strErrorMessage += "License has expired or Data file out of date...";
    break;
  }
  if(!strLastError.isEmpty())
    strErrorMessage += "\n" + strLastError;

  // Return Error Message
  return strErrorMessage;
}

//////////////////////////////////////////////////////////////////////
// Load Freescale Parameter table from DISK
//////////////////////////////////////////////////////////////////////
void CGFreescaletoSTDF::LoadParameterIndexTable(void)
{
  QString	strFreescaleTableFile;
  QString	strString;

  strFreescaleTableFile  = GS::Gex::Engine::GetInstance().Get("UserFolder").toString();
  strFreescaleTableFile += GEX_FREESCALE_PARAMETERS;

  // Open Freescale Parameter table file
  QFile f( strFreescaleTableFile );
  if(!f.open( QIODevice::ReadOnly ))
    return;

  // Assign file I/O stream
  QTextStream hFreescaleTableFile(&f);

  // Skip comment lines
  do
  {
    strString = hFreescaleTableFile.readLine();
  }
  while((strString.indexOf("----------------------") < 0) && (hFreescaleTableFile.atEnd() == false));

  // Read lines
  m_pFullFreescaleParametersList.clear();
  strString = hFreescaleTableFile.readLine();
  while (strString.isNull() == false)
  {
    // Save Parameter name in list
    m_pFullFreescaleParametersList.append(strString);
    // Read next line
    strString = hFreescaleTableFile.readLine();
  };

  // Close file
  f.close();
}

//////////////////////////////////////////////////////////////////////
// Save Freescale Parameter table to DISK
//////////////////////////////////////////////////////////////////////
void CGFreescaletoSTDF::DumpParameterIndexTable(void)
{
  QString		strFreescaleTableFile;

  strFreescaleTableFile  = GS::Gex::Engine::GetInstance().Get("UserFolder").toString();
  strFreescaleTableFile += GEX_FREESCALE_PARAMETERS;

  // Open Freescale Parameter table file
  QFile f( strFreescaleTableFile );
  if(!f.open( QIODevice::WriteOnly ))
    return;

  // Assign file I/O stream
  QTextStream hFreescaleTableFile(&f);

  // First few lines are comments:
  hFreescaleTableFile << "############################################################" << endl;
  hFreescaleTableFile << "# DO NOT EDIT THIS FILE!" << endl;
  hFreescaleTableFile << "# Quantix Examinator: Freescale Parameters detected" << endl;
  hFreescaleTableFile << "# www.mentor.com" << endl;
  hFreescaleTableFile << "# Quantix Examinator reads and writes into this file..." << endl;
  hFreescaleTableFile << "-----------------------------------------------------------" << endl;

  // Write lines
  // m_pFullFreescaleParametersList.sort();
  for (QStringList::const_iterator
       iter  = m_pFullFreescaleParametersList.begin();
       iter != m_pFullFreescaleParametersList.end(); ++iter) {
    // Write line
    hFreescaleTableFile << *iter << endl;
  }

  // Close file
  f.close();
}

//////////////////////////////////////////////////////////////////////
// If Examinator doesn't have this Freescale parameter in his dictionnary, have it added.
//////////////////////////////////////////////////////////////////////
void CGFreescaletoSTDF::UpdateParameterIndexTable(QString strParamName)
{
  // Check if the table is empty...if so, load it from disk first!
  if(m_pFullFreescaleParametersList.isEmpty() == true)
  {
    // Load Freescale parameter table from disk...
    LoadParameterIndexTable();
  }

  // Check if Parameter name already in table...if not, add it to the list
  // the new full list will be dumped to the disk at the end.
  if(m_pFullFreescaleParametersList.indexOf(strParamName) < 0)
  {
    // Update list
    m_pFullFreescaleParametersList.append(strParamName);

    // Set flag to force the current Freescale table to be updated on disk
    m_bNewFreescaleParameterFound = true;
  }
}


//////////////////////////////////////////////////////////////////////
// Check if File is compatible with Freescale format
//////////////////////////////////////////////////////////////////////
bool CGFreescaletoSTDF::IsCompatible(const char *szFileName)
{
  QString strString;

  // Open hCsmFile file
  QFile f( szFileName );
  if(!f.open( QIODevice::ReadOnly ))
  {
    // Failed Opening Freescale file
    return false;
  }
  // Assign file I/O stream
  QTextStream hFreescaleFile(&f);
  QRegExp	qRegExp("[\\( \\)]+");

  // Check if first line is the correct Freescale header...
  //Lot,Start Date,Start Time,S/N,Site,Index,Outcome,Failed,Duration
  do
    strString = hFreescaleFile.readLine();
  while(!strString.isNull() && strString.isEmpty());

  f.close();

  strString = strString.simplified().remove(" ");	// remove leading spaces.

  if(strString.startsWith("Lot,StartDate,StartTime,S/N,Site,Index,Outcome,Failed,Duration", Qt::CaseInsensitive) == false)
  {
    // Incorrect header...this is not a Freescale file!
    return false;
  }

  return true;
}

//////////////////////////////////////////////////////////////////////
// Read and Parse the Freescale file
//////////////////////////////////////////////////////////////////////
bool CGFreescaletoSTDF::ReadFreescaleFile(const char *FreescaleFileName,const char *strFileNameSTDF)
{
  QString strString;
  QString strSection;
  bool	bStatus;
  int		iIndex;				// Loop index

  // Open CSV file
  QFile f( FreescaleFileName );
  if(!f.open( QIODevice::ReadOnly ))
  {
    // Failed Opening Freescale file
    iLastError = errOpenFail;

    // Convertion failed.
    return false;
  }


  //////////////////////////////////////////////////////////////////////
  // For ProgressBar
  iNextFilePos = 0;
  iProgressStep = 0;
  iFileSize = f.size() + 1;

  QString strDataFilePath = QFileInfo(FreescaleFileName).path();

  // Assign file I/O stream
  QTextStream hFreescaleFile(&f);
  QRegExp	qRegExp("[\\( \\)]+");

  m_iIndexParametersOffset = 9;

  // Check if first line is the correct Freescale header...
  //Lot,Start Date,Start Time,S/N,Site,Index,Outcome,Failed,Duration
  strString = ReadLine(hFreescaleFile);
  strString = strString.trimmed();	// remove leading spaces.

  strSection = strString.simplified().remove(" ");
  if(strSection.startsWith("Lot,StartDate,StartTime,S/N,Site,Index,Outcome,Failed,Duration", Qt::CaseInsensitive) == false)
  {
    // Incorrect header...this is not a Freescale file!
    iLastError = errInvalidFormat;

    // Convertion failed.
    // Close file
    f.close();
    return false;
  }

  QStringList lstSections = strString.split(",",QString::KeepEmptyParts);
  // Count the number of parameters specified in the line
  // Do not count first 9 fields
  m_iTotalParameters=lstSections.count() - (m_iIndexParametersOffset+1);
  // If no parameter specified...ignore!
  if(m_iTotalParameters <= 0)
  {
    // Incorrect header...this is not a valid Freescale file!
    iLastError = errInvalidFormat;

    // Convertion failed.
    // Close file
    f.close();
    return false;
  }

  // Allocate the buffer to hold the N parameters & results.
  m_pCGFreescaleParameter = new CGFreescaleParameter[m_iTotalParameters];	// List of parameters
  QString           strAssociatedRegister;
  QMap<QString,int> mapRegisterParameters;

  // Extract the N column names
  for(iIndex=0;iIndex<m_iTotalParameters;iIndex++)
  {
    strSection = lstSections[iIndex+m_iIndexParametersOffset].trimmed();  // Remove spaces
    m_pCGFreescaleParameter[iIndex].strName = strSection;
    m_pCGFreescaleParameter[iIndex].nNumber = -1;
    UpdateParameterIndexTable(strSection);                                // Update Parameter master list if needed.
    m_pCGFreescaleParameter[iIndex].bStaticHeaderWritten = false;
    m_pCGFreescaleParameter[iIndex].nHardBin = -1;
    m_pCGFreescaleParameter[iIndex].nSoftBin = -1;
    m_pCGFreescaleParameter[iIndex].bRegisterParameter = false;
    m_pCGFreescaleParameter[iIndex].nAssociatedIndex = -1;

    // Special case for Register parameter
    // Hexa parameter
    if(strSection.contains("Register Contents",Qt::CaseInsensitive))
    {
      m_pCGFreescaleParameter[iIndex].bRegisterParameter = true;
      strAssociatedRegister = m_pCGFreescaleParameter[iIndex].strName;
      // Update the map list
      mapRegisterParameters[strAssociatedRegister.toLower()]=iIndex;
      // Found the associated Register
      if(strAssociatedRegister.contains("Fused"))
        strAssociatedRegister = strAssociatedRegister.replace("Fused","Unfused");
      else
        strAssociatedRegister = strAssociatedRegister.replace("Unfused","Fused");
      if(mapRegisterParameters.contains(strAssociatedRegister.toLower()))
        m_pCGFreescaleParameter[iIndex].nAssociatedIndex = mapRegisterParameters[strAssociatedRegister.toLower()];
    }
  }

  int	iLimits =0;
  strString = ReadLine(hFreescaleFile);
  if(!strString.startsWith("Trimming Lower Limit",Qt::CaseInsensitive))
  {
    // Incorrect header...this is not a valid Freescale file!
    iLastError = errInvalidFormat;

    // Convertion failed.
    // Close file
    f.close();
    return false;
  }
  // found the Low limits
  iLimits |= 2;
  lstSections = strString.split(",",QString::KeepEmptyParts);
  // Check if have the good count
  if(lstSections.count() < m_iTotalParameters+m_iIndexParametersOffset)
  {
    iLastError = errInvalidFormatLowInRows;

    // Convertion failed.
    // Close file
    f.close();
    return false;
  }

  // Extract the N column Lower Limits
  for(iIndex=0;iIndex<m_iTotalParameters;iIndex++)
  {
    strSection = lstSections[iIndex+m_iIndexParametersOffset].trimmed();
    if(m_pCGFreescaleParameter[iIndex].bRegisterParameter)
    {
      m_pCGFreescaleParameter[iIndex].fLowLimit = (float)strSection.toInt(&bStatus,16);
      m_pCGFreescaleParameter[iIndex].bValidLowLimit = bStatus;
    }
    else
    {
      m_pCGFreescaleParameter[iIndex].fLowLimit = strSection.toFloat(&bStatus);
      m_pCGFreescaleParameter[iIndex].bValidLowLimit = bStatus;
    }
  }

  strString = ReadLine(hFreescaleFile);
  if(!strString.startsWith("Trimming Upper Limit",Qt::CaseInsensitive))
  {
    // Incorrect header...this is not a valid Freescale file!
    iLastError = errInvalidFormat;

    // Convertion failed.
    // Close file
    f.close();
    return false;
  }
  // found the HIGH limits
  iLimits |= 1;
  lstSections = strString.split(",",QString::KeepEmptyParts);
  // Check if have the good count
  if(lstSections.count() < m_iTotalParameters+m_iIndexParametersOffset)
  {
    iLastError = errInvalidFormatLowInRows;

    // Convertion failed.
    // Close file
    f.close();
    return false;
  }

  // Extract the N column Upper Limits
  for(iIndex=0;iIndex<m_iTotalParameters;iIndex++)
  {
    strSection = lstSections[iIndex+m_iIndexParametersOffset].trimmed();
    if(m_pCGFreescaleParameter[iIndex].bRegisterParameter)
    {
      m_pCGFreescaleParameter[iIndex].fHighLimit = (float)strSection.toInt(&bStatus,16);
      m_pCGFreescaleParameter[iIndex].bValidHighLimit = bStatus;
    }
    else
    {
      m_pCGFreescaleParameter[iIndex].fHighLimit = strSection.toFloat(&bStatus);
      m_pCGFreescaleParameter[iIndex].bValidHighLimit = bStatus;
    }
  }

  if(iLimits != 3)
  {
    // Incorrect header...this is not a valid Freescale file!: we didn't find the limits!
    iLastError = errNoLimitsFound;

    // Convertion failed.
    bStatus = false;
  }
  else
  {
    // Read the next line for StartTime
    strString = ReadLine(hFreescaleFile);
    lstSections = strString.split(",",QString::KeepEmptyParts);
    //Lot,Start Date,Start Time,S/N,Site,Index,Outcome,Failed,Duration
    //XDBD285900_FT,3/28/2012,43:31.1,11,1,1,Failed,Pre-Reset Offset Y Std Deviation,2.092

    // Lot
    //XDBD285900_FT
    m_strLotID = lstSections[0];

    // Date
    //3/28/2012,43:31.1
    QString strDate = lstSections[1];
    QString strTime = lstSections[2];
    QDate clDate = QDate(strDate.section("/",2,2).toInt(),strDate.section("/",0,0).toInt(),strDate.section("/",1,1).toInt());
    QTime clTime = QTime::fromString(strTime);
    QDateTime clDateTime(clDate, clTime);
    clDateTime.setTimeSpec(Qt::UTC);
    m_lStartTime = clDateTime.toTime_t();

    if(!ReadFreescaleBinmapFile(strDataFilePath))
    {
      // Do not reject the file with Examinator
      if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
      {
        // Convertion failed.
        // Close file
        f.close();
        return false;
      }

      strLastError += ". \nFreescale Bin mapping cannot be retrieved";
      m_lstSoftBin.append("Passed");
      m_bHaveBinmapFile = false;
    }

    // Restart to the begining
    hFreescaleFile.seek(0);
    // Loop reading file until end is reached & generate STDF file dynamically.
    bStatus = WriteStdfFile(&hFreescaleFile,strFileNameSTDF);
    if(!bStatus)
      QFile::remove(strFileNameSTDF);
  }

  // Close file
  f.close();

  // All Freescale file read...check if need to update the Freescale Parameter list on disk?
  if(bStatus && (m_bNewFreescaleParameterFound == true))
    DumpParameterIndexTable();

  // Success parsing Freescale file
  return bStatus;
}

//////////////////////////////////////////////////////////////////////
// Create STDF file from Freescale data parsed
//////////////////////////////////////////////////////////////////////
bool CGFreescaletoSTDF::WriteStdfFile(QTextStream *hFreescaleFile, const char *strFileNameSTDF)
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
  StdfFile.WriteDword(m_lStartTime);        // Setup time
  StdfFile.WriteDword(m_lStartTime);        // Start time
  StdfFile.WriteByte(1);                    // Station
  StdfFile.WriteByte((BYTE) 'P');           // Test Mode = PRODUCTION
  StdfFile.WriteByte((BYTE) ' ');           // rtst_cod
  StdfFile.WriteByte((BYTE) ' ');           // prot_cod
  StdfFile.WriteWord(65535);                // burn_tim
  StdfFile.WriteByte((BYTE) ' ');           // cmod_cod
  StdfFile.WriteString(m_strLotID.toLatin1().constData());		// Lot ID
  StdfFile.WriteString(m_strProductID.toLatin1().constData());	// Part Type / Product ID
  StdfFile.WriteString("");                 // Node name
  StdfFile.WriteString("");                 // Tester Type
  StdfFile.WriteString(m_strProgramID.toLatin1().constData());	// Job name
  StdfFile.WriteString("");                 // Job rev
  StdfFile.WriteString("");                 // sublot-id
  StdfFile.WriteString("");                 // operator
  StdfFile.WriteString("");                 // exec-type
  StdfFile.WriteString("");                 // exe-ver
  StdfFile.WriteString("");                 // test-cod
  StdfFile.WriteString("");                 // test-temperature
  // Construct custom Galaxy USER_TXT
  QString	strUserTxt;
  strUserTxt = GEX_IMPORT_DATAORIGIN_LABEL;
  strUserTxt += ":";
  strUserTxt += GEX_IMPORT_DATAORIGIN_ETEST;
  strUserTxt += ":FreescaleMaxwell";
  StdfFile.WriteString(strUserTxt.toLatin1().constData());	// user-txt
  StdfFile.WriteString("");                 // aux-file
  StdfFile.WriteString("");                 // package-type
  StdfFile.WriteString(m_strProductID.toLatin1().constData());	// familyID
  StdfFile.WriteString("");                 // Date-code
  StdfFile.WriteString("");                 // Facility-ID
  StdfFile.WriteString("");                 // FloorID
  StdfFile.WriteString("");                 // ProcessID
  StdfFile.WriteRecord();

  // Write Test results for each line read.
  QString     strString;
  char        szString[257];
  QString     strSection;
  float       fValue;               // Used for readng floating point numbers.
  int         iIndex;               // Loop index
  int         iSiteNumber;
  BYTE        bData;
  int         nSoftBin,nHardBin;
  QString     strSoftBinName,strHardBinName;
  long        iTotalGoodBin,iTotalFailBin;
  long        iTestNumber,iTotalTests;
  long        iLastTestExecuted;
  long        iPartCount,iPartNumber;
  long        lTime;
  bool        bStatus,bPassStatus,bIsNumber,bTestFailed;
  QStringList	lstSections;
  // Binning count
  QMap<int,CGFreescaleBinning>	mapSoftBinning;		// List of Bin tables.
  QMap<int,CGFreescaleBinning>	mapHardBinning;		// List of Bin tables.
  QMap<int,CGFreescaleBinning>::Iterator it;

  // Reset counters
  lTime = 0;
  iLastTestExecuted=-1;
  iTotalGoodBin=iTotalFailBin=0;
  iPartCount=iPartNumber=0;

  // Write all Parameters read on this wafer.: WIR.PIR,PTR....PTR, PRR, PIR, PTR,PTR...PRR,   ... WRR
  while(hFreescaleFile->atEnd() == false)
  {

    // Read line
    strString = ReadLine(*hFreescaleFile);

    if(strString.startsWith("Lot,Start",Qt::CaseInsensitive))
      continue;
    if(strString.startsWith("Trimming Lower Limit",Qt::CaseInsensitive))
      continue;
    if(strString.startsWith("Trimming Upper Limit",Qt::CaseInsensitive))
      continue;

    lstSections = strString.split(",",QString::KeepEmptyParts);
    // Check if have the good count
    if(lstSections.count() < m_iTotalParameters+m_iIndexParametersOffset)
    {
      iLastError = errInvalidFormatLowInRows;

      StdfFile.Close();
      // Convertion failed.
      return false;
    }

    //Lot,Start Date,Start Time,S/N,Site,Index,Outcome,Failed,Duration
    // Lot
    //XDBD285900_FT
    if(m_strLotID.toLower() != lstSections[0].toLower())
    {
      iLastError = errInvalidFormat;
      strLastError = "Multi lots found";
      StdfFile.Close();
      // Convertion failed.
      return false;
    }

    // Date
    //3/28/2012,43:31.1
    QString strDate = lstSections[1];
    QString strTime = lstSections[2];
    QDate clDate = QDate(strDate.section("/",2,2).toInt(),strDate.section("/",0,0).toInt(),strDate.section("/",1,1).toInt());
    QTime clTime = QTime::fromString(strTime);
    QDateTime clDateTime(clDate, clTime);
    clDateTime.setTimeSpec(Qt::UTC);
    m_lStartTime = clDateTime.toTime_t();

    // Part number
    iPartCount++;
    iPartNumber=lstSections[3].toInt(&bIsNumber);
    if(!bIsNumber)
      iPartNumber = iPartCount;

    // Site
    iSiteNumber = lstSections[4].toInt(&bIsNumber);

    //« Failed » is similar to soft bin description. « Outcome » similar to a hard bin description.
    // HardBin description
    strHardBinName = lstSections[6];

    // SoftBin description
    strSoftBinName = lstSections[7];
    if(strSoftBinName == "None")
      strSoftBinName = strHardBinName;
    if(strSoftBinName.contains("Expected Value ="))
    {
      // Clean the Register error
      strSoftBinName = strSoftBinName.section("0x",0,0).trimmed();
    }

    // Duration
    lTime = (int)(lstSections[8].toFloat()*1000.0);

    // Pass/Fail flag.
    bPassStatus = true;

    // Reset counters
    iTotalTests = 0;
    iLastTestExecuted=-1;
    nSoftBin=nHardBin=-1;

    // Write PIR for parts in this Wafer site
    RecordReadInfo.iRecordType = 5;
    RecordReadInfo.iRecordSubType = 10;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteByte(1);								// Test head
    StdfFile.WriteByte(iSiteNumber);					// Tester site
    StdfFile.WriteRecord();

    // Read Parameter results for this record
    for(iIndex=0;iIndex<m_iTotalParameters;iIndex++)
    {
      strSection = lstSections[iIndex+m_iIndexParametersOffset].trimmed();
      if(m_pCGFreescaleParameter[iIndex].bRegisterParameter)
        fValue = (float) strSection.toInt(&bStatus,16);
      else
        fValue = strSection.toFloat(&bStatus);

      if(bStatus == true)
      {
        bTestFailed = false;
        // Ignore parameters duration
        if(!m_pCGFreescaleParameter[iIndex].strName.endsWith("(duration)"))
          iLastTestExecuted = iIndex;
        // Valid test result...write the PTR
        iTotalTests++;
        m_pCGFreescaleParameter[iIndex].fValue = fValue;

        RecordReadInfo.iRecordType = 15;
        RecordReadInfo.iRecordSubType = 10;
        StdfFile.WriteHeader(&RecordReadInfo);
        iTestNumber = m_pCGFreescaleParameter[iIndex].nNumber;
        if((m_bHaveBinmapFile) && (iTestNumber < 0))
        {
          iLastError = errInvalidFormat;
          strLastError = "Parameter '"+m_pCGFreescaleParameter[iIndex].strName;
          strLastError+= "' does not referenced into the Binmap file";
          StdfFile.Close();
          // Convertion failed.
          return false;
        }
        if(iTestNumber < 0)
        {
          // Compute Test# (add user-defined offset)
          iTestNumber = (long) m_pFullFreescaleParametersList.indexOf(m_pCGFreescaleParameter[iIndex].strName);
          iTestNumber += GEX_TESTNBR_OFFSET_FREESCALE;  // Test# offset
        }
        StdfFile.WriteDword(iTestNumber);             // Test Number
        StdfFile.WriteByte(1);                        // Test head
        StdfFile.WriteByte(iSiteNumber);              // Tester site#
        // Check if the test failed
        if(((m_pCGFreescaleParameter[iIndex].bValidLowLimit==true) && (fValue < m_pCGFreescaleParameter[iIndex].fLowLimit)) ||
            ((m_pCGFreescaleParameter[iIndex].bValidHighLimit==true) && (fValue > m_pCGFreescaleParameter[iIndex].fHighLimit)))
          bTestFailed = true;
        // Case for Register parameters
        // Must have the same value as the associated Register
        if(m_pCGFreescaleParameter[iIndex].nAssociatedIndex >= 0)
        {
          // Have an associated Register
          // Check if have the same value
          if(!(m_pCGFreescaleParameter[iIndex].fValue == m_pCGFreescaleParameter[m_pCGFreescaleParameter[iIndex].nAssociatedIndex].fValue))
            bTestFailed = true;
        }

        if(bTestFailed)
        {
          bData = 0200;	// Test Failed
          if(!strHardBinName.startsWith("P",Qt::CaseInsensitive))
          {
            // When Part is flagged as passed, ignore this failure
            if(bPassStatus)
            {
              // First test failed
              nSoftBin = m_pCGFreescaleParameter[iIndex].nSoftBin;
              nHardBin = m_pCGFreescaleParameter[iIndex].nHardBin;

              // Check and update bin count
              // with binmap file
              if(m_bHaveBinmapFile
              &&((nSoftBin < 0) || (nHardBin < 0)))
              {
                iLastError = errMissingData;
                strLastError = "Parameter "+m_pCGFreescaleParameter[iIndex].strName;
                strLastError+= " not associated with Soft/Hard bin into the Binmap file";
                StdfFile.Close();
                // Convertion failed.
                return false;
              }
            }
            bPassStatus = false;
          }
        }
        else
        {
          bData = 0;		// Test passed
          if(bPassStatus)
          {
            nSoftBin = 0;
            nHardBin = 1;
          }
        }

        StdfFile.WriteByte(bData);                            // TEST_FLG
        StdfFile.WriteByte(0x40|0x80);                        // PARAM_FLG
        StdfFile.WriteFloat(fValue);                          // Test result
        if(m_pCGFreescaleParameter[iIndex].bStaticHeaderWritten == false)
        {
          StdfFile.WriteString(m_pCGFreescaleParameter[iIndex].strName.toLatin1().constData());	// TEST_TXT
          StdfFile.WriteString("");                         // ALARM_ID
          bData = 2;	// Valid data.
          if(m_pCGFreescaleParameter[iIndex].bValidLowLimit==false)
            bData |=0x40;
          if(m_pCGFreescaleParameter[iIndex].bValidHighLimit==false)
            bData |=0x80;
          StdfFile.WriteByte(bData);                        // OPT_FLAG
          StdfFile.WriteByte(0);                            // RES_SCALE
          StdfFile.WriteByte(0);                            // LLM_SCALE
          StdfFile.WriteByte(0);                            // HLM_SCALE
          StdfFile.WriteFloat(m_pCGFreescaleParameter[iIndex].fLowLimit);			// LOW Limit
          StdfFile.WriteFloat(m_pCGFreescaleParameter[iIndex].fHighLimit);		// HIGH Limit
          StdfFile.WriteString("");                         // No Units
          m_pCGFreescaleParameter[iIndex].bStaticHeaderWritten = true;
        }
        StdfFile.WriteRecord();
      }	// Valid test result
    }		// Read all results on line


    // Pass/Fail flag.
    if(!strHardBinName.startsWith("P",Qt::CaseInsensitive))
    {
      if(bPassStatus)
      {
        // No test failed found
        // No Soft/Hard bin can be associated
        nSoftBin = nHardBin = -1;

        // Check if the LastTestExecuted can be used
        if(iLastTestExecuted > -1)
        {
          nSoftBin = m_pCGFreescaleParameter[iLastTestExecuted].nSoftBin;
          nHardBin = m_pCGFreescaleParameter[iLastTestExecuted].nHardBin;
        }

        // Check and update bin count
        // with binmap
        if(m_bHaveBinmapFile
        &&((nSoftBin < 0) || (nHardBin < 0)))
        {
          iLastError = errMissingData;
          strLastError = "Part["+QString::number(iPartNumber)+"] flagged as failed - No parameter failed - ";
          strLastError+= "Last parameter executed '"+m_pCGFreescaleParameter[iLastTestExecuted].strName;
          strLastError+= "' is not associated with Soft/Hard bin into the Binmap file";
          StdfFile.Close();
          // Convertion failed.
          return false;
        }
      }

      bPassStatus = false;

      if(!m_bHaveBinmapFile)
      {
        // no binmap
        nHardBin = 2;
        if(!m_lstSoftBin.contains(strSoftBinName))
          m_lstSoftBin.append(strSoftBinName);
        nSoftBin = m_lstSoftBin.indexOf(strSoftBinName);
      }

    }
    else
      bPassStatus = true;

    // Write PRR
    RecordReadInfo.iRecordType = 5;
    RecordReadInfo.iRecordSubType = 20;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteByte(1);			// Test head
    StdfFile.WriteByte(iSiteNumber);// Tester site#:1
    if(bPassStatus == true)
    {
      StdfFile.WriteByte(0);            // PART_FLG : PASSED
      nSoftBin = 0;
      nHardBin = 1;
      iTotalGoodBin++;
    }
    else
    {
      StdfFile.WriteByte(8);            // PART_FLG : FAILED
      iTotalFailBin++;
    }
    StdfFile.WriteWord((WORD)iTotalTests);  // NUM_TEST
    StdfFile.WriteWord(nHardBin);           // HARD_BIN
    StdfFile.WriteWord(nSoftBin);           // SOFT_BIN
    StdfFile.WriteWord(static_cast<WORD>(-32768));             // X_COORD
    StdfFile.WriteWord(static_cast<WORD>(-32768));             // Y_COORD
    StdfFile.WriteDword(lTime);             // testing time known...
    sprintf(szString,"%ld",iPartNumber);
    StdfFile.WriteString(szString);         // PART_ID
    StdfFile.WriteString("");               // PART_TXT
    StdfFile.WriteString("");               // PART_FIX
    StdfFile.WriteRecord();

    // Update Binning info
    // for Hard bin
    if(!mapHardBinning.contains(nHardBin))
    {
      mapHardBinning[nHardBin].strName = strHardBinName;
      mapHardBinning[nHardBin].nNumber = nHardBin;
      mapHardBinning[nHardBin].bPass = bPassStatus;
      mapHardBinning[nHardBin].nCount = 0;
    }
    mapHardBinning[nHardBin].nCount++;
    // for Soft bin
    if(!mapSoftBinning.contains(nSoftBin))
    {
      mapSoftBinning[nSoftBin].strName = strSoftBinName;
      mapSoftBinning[nSoftBin].nNumber = nSoftBin;
      mapSoftBinning[nSoftBin].bPass = bPassStatus;
      mapSoftBinning[nSoftBin].nCount = 0;
    }
    mapSoftBinning[nSoftBin].nCount++;

  };			// Read all lines with valid data records in file

  // Write Binning summary
  RecordReadInfo.iRecordType = 1;
  RecordReadInfo.iRecordSubType = 40;
  for(it = mapHardBinning.begin(); it != mapHardBinning.end(); it++)
  {
    // Write HBR
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteByte(255);						// Test Head = ALL
    StdfFile.WriteByte(255);						// Test sites = ALL
    StdfFile.WriteWord(mapHardBinning[it.key()].nNumber);			// HBIN
    StdfFile.WriteDword(mapHardBinning[it.key()].nCount);			// Total Bins
    if(mapHardBinning[it.key()].bPass)
      StdfFile.WriteByte('P');
    else
      StdfFile.WriteByte('F');
    StdfFile.WriteString(mapHardBinning[it.key()].strName.toLatin1().constData());
    StdfFile.WriteRecord();
  }

  RecordReadInfo.iRecordType = 1;
  RecordReadInfo.iRecordSubType = 50;
  for(it = mapSoftBinning.begin(); it != mapSoftBinning.end(); it++)
  {
    // Write SBR
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteByte(255);						// Test Head = ALL
    StdfFile.WriteByte(255);						// Test sites = ALL
    StdfFile.WriteWord(mapSoftBinning[it.key()].nNumber);			// HBIN
    StdfFile.WriteDword(mapSoftBinning[it.key()].nCount);			// Total Bins
    if(mapSoftBinning[it.key()].bPass)
      StdfFile.WriteByte('P');
    else
      StdfFile.WriteByte('F');
    StdfFile.WriteString(mapSoftBinning[it.key()].strName.toLatin1().constData());
    StdfFile.WriteRecord();
  }

  // Write MRR
  RecordReadInfo.iRecordType = 1;
  RecordReadInfo.iRecordSubType = 20;
  StdfFile.WriteHeader(&RecordReadInfo);
  StdfFile.WriteDword(m_lStartTime+lTime);			// File finish-time.
  StdfFile.WriteRecord();

  // Close STDF file.
  StdfFile.Close();

  // Success
  return true;
}

//////////////////////////////////////////////////////////////////////
// Read and Parse the FreescaleBinmap file
//////////////////////////////////////////////////////////////////////
bool CGFreescaletoSTDF::ReadFreescaleBinmapFile(QString &strDataFilePath)
{
  QString strString;

  iNextFilePos = 0;

  QString strExternalFileName;
  QString strExternalFileFormat;
  QString strExternalFileError;
  // Check if Test->Binning mapping file to overload softbin
  ConverterExternalFile::GetBinmapFile(strDataFilePath,"final","prod",strExternalFileName,strExternalFileFormat,strExternalFileError);
  if(strExternalFileName.isEmpty())
  {
    if(strExternalFileError.isEmpty())
      strExternalFileError = "No 'GEX_FREESCALE_BINMAP_FILE' file defined";
    iLastError = errMissingData;
    strLastError = strExternalFileError;
    return false;
  }

  // Open CSV file
  QFile f( strExternalFileName );
  if(!f.open( QIODevice::ReadOnly ))
  {
    // Failed Opening SpektraDatalog file
    iLastError = errOpenFail;
    strLastError = strExternalFileName+"\n"+f.errorString();

    // Convertion failed.
    return false;
  }

  iFileSize += f.size();

  // Assign file I/O stream
  QTextStream hFile(&f);

  //Soft Bin,Hard Bin,Test Numbers,Test Names,Trimming Lower Limit,Trimming Upper Limit
  //17,2,1,Measure SCL Neg[Open Short Test],-1,9,-0,3
  //17,2,2,Measure SCL Pos[Open Short Test],0,3,1,9

  // Ignore Limits when already defined into thee Data file

  bool    bStatus = false;
  QString strTestName;
  int     nTestNumber;
  bool    bIsValidNum;
  int     iBinCode;

  bool    bTestFound;
  int     iTestIndex;
  bTestFound = false;
  iTestIndex = 0;

  // Read SpektraDatalog information
  while(!hFile.atEnd())
  {
    strString = ReadLine(hFile).trimmed();

    // Skip first line
    if(strString.startsWith("Soft",Qt::CaseInsensitive))
      continue;

    // Skip comments
    if(strString.startsWith("#"))
      continue;

    // Skip empty line
    if(strString.isEmpty())
      continue;

    // Split line, and make sure we have enough fields
    if(strString.count(",") < 4)
    {
      iLastError = errInvalidFormat;
      strLastError = "Incorrect BinMap format.\n";
      strLastError+= "line: "+strString+"\n\n";
      strLastError+= "Supported format: Soft Bin,Hard Bin,Test Numbers,Test Names,Trimming Lower Limit,Trimming Upper Limit\n";
      strLastError+= "Ex: 17,2,1,Measure SCL Neg[Open Short Test],-1.9,-0.3";
      // Convertion failed.
      bStatus = false;
      break;
    }

    strTestName = strString.section(",",3,3).simplified();

    // Found the Test into the list
    // Start to the last test found
    for(int iLoop=0; iLoop<2; iLoop++)
    {
      bTestFound = false;
      for(int iIndex=iTestIndex; iIndex<m_iTotalParameters; iIndex++)
      {
        if(m_pCGFreescaleParameter[iIndex].strName.toLower().simplified()
            == strTestName.toLower().simplified())
        {
          iTestIndex = iIndex;
          bTestFound = true;
          break;
        }
      }
      if(!bTestFound)
        iTestIndex=0;
      else
        break;
    }
    // ignore this line if no test found
    if(!bTestFound)
      continue;

    nTestNumber = strString.section(",",2,2).toInt(&bIsValidNum);
    if((!bIsValidNum) || (nTestNumber < 0))
    {
      iLastError = errInvalidFormat;
      strLastError = "Incorrect BinMap format.\n";
      strLastError+= "Test Number must be a valid integer\n";
      strLastError+= "line: "+strString+"\n\n";
      strLastError+= "Supported format: Soft Bin,Hard Bin,Test Numbers,Test Names,Trimming Lower Limit,Trimming Upper Limit\n";
      strLastError+= "Ex: 17,2,1,Measure SCL Neg[Open Short Test],-1.9,-0.3";
      // Convertion failed.
      bStatus = false;
      break;
    }

    // Update Test Number
    m_pCGFreescaleParameter[iTestIndex].nNumber = nTestNumber;

    // Check if we have to update Limits
    if(!m_pCGFreescaleParameter[iTestIndex].bValidLowLimit)
    {
      QString strLimit = strString.section(",",4,4).simplified();
      if(m_pCGFreescaleParameter[iTestIndex].bRegisterParameter)
      {
        m_pCGFreescaleParameter[iTestIndex].fLowLimit = (float)strLimit.toInt(&bStatus,16);
        m_pCGFreescaleParameter[iTestIndex].bValidLowLimit = bStatus;
      }
      else
      {
        m_pCGFreescaleParameter[iTestIndex].fLowLimit = strLimit.toFloat(&bStatus);
        m_pCGFreescaleParameter[iTestIndex].bValidLowLimit = bStatus;
      }
    }
    if(!m_pCGFreescaleParameter[iTestIndex].bValidHighLimit)
    {
      QString strLimit = strString.section(",",5,5).simplified();
      if(m_pCGFreescaleParameter[iTestIndex].bRegisterParameter)
      {
        m_pCGFreescaleParameter[iTestIndex].fHighLimit = (float)strLimit.toInt(&bStatus,16);
        m_pCGFreescaleParameter[iTestIndex].bValidHighLimit = bStatus;
      }
      else
      {
        m_pCGFreescaleParameter[iTestIndex].fHighLimit = strLimit.toFloat(&bStatus);
        m_pCGFreescaleParameter[iTestIndex].bValidHighLimit = bStatus;
      }
    }

    bStatus = true;

    iBinCode = strString.section(",",0,0).toInt(&bIsValidNum);
    if(!bIsValidNum)
      continue;

    m_pCGFreescaleParameter[iTestIndex].nSoftBin = iBinCode;
    m_pCGFreescaleParameter[iTestIndex].nHardBin = strString.section(",",1,1).toInt();

  }

  if(!bStatus)
  {
    iLastError = errMissingData;
    strLastError = "No parameter matches with this Binmap file";
    // Convertion failed.
  }
  else
  {
    // Have Test Number, do not save parameters file
    m_bNewFreescaleParameterFound = false;
    m_bHaveBinmapFile = true;
  }

  // Success parsing SpektraDatalog file
  f.close();
  return bStatus;
}

//////////////////////////////////////////////////////////////////////
// Convert 'FileName' Freescale file, to STDF 'strFileNameSTDF' file
//////////////////////////////////////////////////////////////////////
bool CGFreescaletoSTDF::Convert(const char *FreescaleFileName, const char *strFileNameSTDF)
{
  // No erro (default)
  iLastError = errNoError;

  // If STDF file already exists...do not rebuild it...unless dates not matching!
  QFileInfo fInput(FreescaleFileName);
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
      GS::Gex::Engine::GetInstance().UpdateLabelStatus("Converting data from file "+QFileInfo(FreescaleFileName).fileName()+"...");
      GexScriptStatusLabel->show();
    }
    GS::Gex::Engine::GetInstance().UpdateLabelStatus("Converting data from file "+QFileInfo(FreescaleFileName).fileName()+"...");
    GexScriptStatusLabel->show();
  }
  QCoreApplication::processEvents();

  if(ReadFreescaleFile(FreescaleFileName,strFileNameSTDF) != true)
  {
    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    if((GexProgressBar != NULL)
        && bHideProgressAfter)
      GexProgressBar->hide();

    if((GexScriptStatusLabel != NULL)
        && bHideLabelAfter)
      GexScriptStatusLabel->hide();
    return false;	// Error reading Freescale file
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
QString CGFreescaletoSTDF::ReadLine(QTextStream& hFile)
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
