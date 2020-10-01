//////////////////////////////////////////////////////////////////////
// import_verigy_edl.cpp: Convert a VERIGY_EDL (TMT) file to STDF V4.0
//////////////////////////////////////////////////////////////////////
#include <math.h>
#include <time.h>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#ifdef _WIN32
#include "windows.h"		// For 'GetWindowsDirectoryA' declaration
#endif
#include "import_verigy_edl.h"
#include "dl4_tools.h"
#include "import_constants.h"


#define BIT0			0x01
#define BIT1			0x02
#define BIT2			0x04
#define BIT3			0x08
#define BIT4			0x10
#define BIT5			0x20
#define BIT6			0x40
#define BIT7			0x80

#define	STRING_MAX		1024

#define GS_POW(x,y) pow(x,y)

//////////////////////////////////////////////////////////////////////
class CGVERIGY_EDL_StoredBuffer : public StoredObject
{
public:
  // Inverse CPUType (not PC file)
  CGVERIGY_EDL_StoredBuffer(){	bSameCPUType = !bSameCPUType;};

  long GetStorageSizeBytes (void)		{return 0;};
  long LoadFromBuffer (const void */*v*/) {return 0;};

  int	ReadByte(const void *v, BYTE *bData)
  {
    int nResult = StoredObject::ReadByte(v,bData);
    return nResult;
  };
  int	ReadWord(const void *v, short *ptWord)
  {
    int nResult = StoredObject::ReadWord(v,ptWord);
    return nResult;
  };
  int	ReadDword(const void *v, long *ptDword)
  {
    int nResult = StoredObject::ReadDword(v,ptDword);
    return nResult;
  };
  int	ReadFloat(const void *v,  float *ptFloat)
  {
    int nResult = StoredObject::ReadFloat(v,ptFloat);
    return nResult;
  };
  int	ReadDouble(const void *v, double *ptDouble)
  {
    int nResult = StoredObject::ReadDouble(v,ptDouble);
    return nResult;
  };
  int	 ReadString(const void *v, char *szString)
  {
    BYTE	*b = (BYTE*) v;
    char c = '\0';
    int i = 0;
    int iIndex = 0;


    // Read one char from v
    while( (c = b[i]) != '\0' )
    {
      if( c == EOF )         /* return FALSE on unexpected EOF */
      {
        szString[iIndex] = '\0';
        return 0;
      }

      // The max lengh is STRING_MAX
      // Ignore all extra char
      // But read until the end
      if((iIndex+1) < STRING_MAX)
        szString[iIndex++] = c;

      i++;
    }
    szString[iIndex] = '\0';
    i++;

    return i;
  };
};

//////////////////////////////////////////////////////////////////////
CGVERIGY_EDLParameter::CGVERIGY_EDLParameter()
{
  m_nTestNum=m_nScale=0;
  m_fLowLimit=m_fHighLimit=0;
  m_bValidLowLimit=m_bValidHighLimit=false;
  m_bStrictLowLimit=m_bStrictHighLimit=false;
  m_bStaticHeaderWritten=false;
}

//////////////////////////////////////////////////////////////////////
CGVERIGY_EDLBinInfo::CGVERIGY_EDLBinInfo()
{
  mapSiteNbCnt[-1]=0;
  bPass=false;
};


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGVERIGY_EDLtoSTDF::CGVERIGY_EDLtoSTDF()
{
  m_lStartTime = 0;
  m_nBlockSize = 100000;
  m_szBlock = (char *) malloc((m_nBlockSize)*sizeof(char));
  m_nBlockData = 0;
  m_nBlockIndex = 0;

  m_bSplitMultiLot	= false;
  m_bHaveMultiLot		= false;
  m_bMirWritten		= false;
  m_bMrrWritten		= false;
  m_bTestResult		= false;


  // Init table of records to process
  for(int i=0; i<NumberOfEventTypes; i++)
    m_nEventRecordsCount[i] = 0;

  m_nXLocation = -32768;
  m_nYLocation = -32768;

  m_nPartNumber	= 0;
  m_iTotalTests	= 0;
  m_iTotalFailBin = 0;
  m_iTotalGoodBin = 0;
  m_iTestNumber	= -1;
  m_bIgnoreTestNumber = false;
  m_bWaferMap		= false;
  m_nPassStatus	= -1;
  m_nHardBin		= -1;
  m_nSoftBin		= -1;
  m_strWaferId	= "INVALID";

  m_mapAssignment["STAT_NUM"]	= "1";
  m_mapAssignment["HEAD_NUM"] = "1";
  m_mapAssignment["SITE_NUM"] = "1";
  m_mapAssignment["SITE_GRP"] = "255";
  m_mapAssignment["MODE_COD"]	= "P";
  m_mapAssignment["RTST_COD"]	= " ";
  m_mapAssignment["PROT_COD"]	= " ";
  m_mapAssignment["CMOD_COD"]	= " ";
  m_mapAssignment["DISP_COD"]	= " ";
  m_mapAssignment["TSTR_TYP"]	= "VerigyEdl";
  m_mapAssignment["BURN_TIM"] = "65535";
  m_mapAssignment["CENTER_X"] = "-32768";
  m_mapAssignment["CENTER_Y"] = "-32768";
  m_mapAssignment["WF_FLAT"]	= " ";
  m_mapAssignment["POS_X"]	= " ";
  m_mapAssignment["POS_Y"]	= " ";
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGVERIGY_EDLtoSTDF::~CGVERIGY_EDLtoSTDF()
{
  QMap<int,CGVERIGY_EDLPinInfo*>::Iterator itMapPin;
  for ( itMapPin = m_qMapPins.begin(); itMapPin != m_qMapPins.end(); ++itMapPin )
  {
    delete itMapPin.value();
  }
  QMap<int,CGVERIGY_EDLBinInfo*>::Iterator itMapBin;
  for ( itMapBin = m_qMapBins.begin(); itMapBin != m_qMapBins.end(); ++itMapBin )
  {
    delete itMapBin.value();
  }
  for ( itMapBin = m_qMapBins_Soft.begin(); itMapBin != m_qMapBins_Soft.end(); ++itMapBin )
  {
    delete itMapBin.value();
  }
  QMap<QString,CGVERIGY_EDLParameter*>::Iterator itMapParam;
  for ( itMapParam = m_qMapParameterList.begin(); itMapParam != m_qMapParameterList.end(); ++itMapParam )
  {
    delete itMapParam.value();
  }
  QMap<QString,CGVERIGY_EDLSiteInfo*>::Iterator itSiteInfo;
  for ( itSiteInfo = m_mapSiteInfo.begin(); itSiteInfo != m_mapSiteInfo.end(); ++itSiteInfo )
  {
    delete itSiteInfo.value();
  }
}


//////////////////////////////////////////////////////////////////////
// Get Error
//////////////////////////////////////////////////////////////////////
QString CGVERIGY_EDLtoSTDF::GetLastError()
{
  m_strLastError = "Import VERIGY_EDL: ";

  switch(m_iLastError)
  {
    default:
    case errNoError:
      m_strLastError += "No Error";
      break;
    case errWarning:
      m_strLastError += "*WARNING*";
      break;
    case errOpenFail:
      m_strLastError += "Failed to open file";
      break;
    case errConversionFailed:
      m_strLastError += "Invalid file format";
      break;
    case errWriteSTDF:
      m_strLastError += "Failed creating temporary file. Folder permission issue?";
      break;
    case errMultiLot:
      m_strLastError += "Invalid file format: This file contains more than one lot";
      break;
    case errLicenceExpired:
      m_strLastError += "License has expired or Data file out of date...";
      break;
  }
  if(!m_strErrorDetail.isEmpty())
    m_strLastError += ":\n "+m_strErrorDetail;

  // Return Error Message
  return m_strLastError;
}

//////////////////////////////////////////////////////////////////////
// Check if File date is older than license expiration date!
//////////////////////////////////////////////////////////////////////
bool CGVERIGY_EDLtoSTDF::CheckValidityDate(QDate *pExpirationDate)
{
  // If no expiration date, or m_lStartTime not set ...ignore !
  if((pExpirationDate == NULL) || (m_lStartTime <= 0))
    return true;

  // Check date found in data file to convert...and see if not to recent!
  // Check if STDF file is too recent
  QDateTime expirationDateTime(*pExpirationDate);
  QDateTime FileDateTime;
  FileDateTime.setTime_t(m_lStartTime);
  FileDateTime.setTimeSpec(Qt::UTC);
  expirationDateTime.setTimeSpec(Qt::UTC);
  if(FileDateTime > expirationDateTime)
    return false;	// Invalid file date! refuse to convert the file.
  return true;
}


//////////////////////////////////////////////////////////////////////
// Check if File is compatible with VERIGY_EDL format
//////////////////////////////////////////////////////////////////////
bool CGVERIGY_EDLtoSTDF::IsCompatible(const char *szFileName)
{
  QString strString;
  QString strSection;
  QString strValue;
  QString	strSite;
  char	szBlock[VERIGY_EDL_BLOCK_SIZE];
  bool	bSuccess = false;

  // Open hCsmFile file
    QFile f( szFileName );
    if(!f.open( QIODevice::ReadOnly ))
  {
    // Failed Opening file
    return false;
  }
  int	iIndex;
  int iBlockSize = f.read(szBlock, 6);
  if(iBlockSize != 6)
  {
    // Incorrect header...this is not a VERIGY_EDL file!
    return false;
  }

  long	lFilePos;
  long	lDword;
  short	sWord;
  int		nRecordLength;
  CGVERIGY_EDL_StoredBuffer	clBinaryObject;

  iIndex = 0;
  //Record Length : 4 Bytes
  iIndex += clBinaryObject.ReadDword(szBlock+iIndex, &lDword);
  nRecordLength = lDword;
  // Event Type : 2 Bytes
  iIndex += clBinaryObject.ReadWord(szBlock+iIndex, &sWord);
  // Event Type must be in the set of EventType
  if((lDword < 0)
  || ((UINT)lDword > f.size())
  || (sWord <= 0)
  || ((sWord+1) >= NumberOfEventTypes))
  {
    // Incorrect header...this is not a VERIGY_EDL file!
  }
  else
  {
    lFilePos = nRecordLength+6;
    if(f.seek(lFilePos))
    {
      // Check if not at the end of the file (only one EventRecord)
      if(f.atEnd())
        bSuccess = true;
      else
      {
        // Read next record
        iBlockSize = f.read(szBlock, 6);
        iIndex = 0;
        //Record Length : 4 Bytes
        iIndex += clBinaryObject.ReadDword(szBlock+iIndex, &lDword);
        nRecordLength = lDword;
        // Event Type : 2 Bytes
        iIndex += clBinaryObject.ReadWord(szBlock+iIndex, &sWord);
        // Event Type must be in the set of EventType
        if((lDword < 0)
        || ((UINT)lDword > f.size())
        || (sWord <= 0)
        || ((sWord+1) >= NumberOfEventTypes))
        {
          // Incorrect header...this is not a VERIGY_EDL file!

          // Ignore this record
          // Check the next
          lFilePos += nRecordLength+6;
          if(f.seek(lFilePos))
          {
            // Check if not at the end of the file (only one EventRecord)
            if(f.atEnd())
              bSuccess = true;
            else
            {

              // Read next record
              iBlockSize = f.read(szBlock, 6);
              iIndex = 0;
              //Record Length : 4 Bytes
              iIndex += clBinaryObject.ReadDword(szBlock+iIndex, &lDword);
              nRecordLength = lDword;
              // Event Type : 2 Bytes
              iIndex += clBinaryObject.ReadWord(szBlock+iIndex, &sWord);
              // Event Type must be in the set of EventType
              if((lDword < 0)
              || ((UINT)lDword > f.size())
              || (sWord <= 0)
              || ((sWord+1) >= NumberOfEventTypes))
              {
                // Incorrect header...this is not a VERIGY_EDL file!
              }
              else
                bSuccess = true;						}
          }
        }
        else
          bSuccess = true;
      }
    }
  }

  f.close();

  return bSuccess;
}


//////////////////////////////////////////////////////////////////////
// Read and Parse the VERIGY_EDL file
//////////////////////////////////////////////////////////////////////
int CGVERIGY_EDLtoSTDF::ReadVerigyEdlFile(QDate *pExpirationDate)
{
  QString			strString;
  QString			strSection;
  QString			strValue;
  QString			strSite;
  CGVERIGY_EDL_StoredBuffer	clBinaryObject;

  // Open VERIGY_EDL file
  m_lStartTime = 0;
  m_lEndTime = -1;
  m_lStartMicroSeconds = m_lEndMicroSeconds = 0;
    QFile f( m_strVerigyEdlFileName );
    if(!f.open( QIODevice::ReadOnly ))
  {
    // Failed Opening VERIGY_EDL file
    m_iLastError = errOpenFail;

    // Convertion failed.
    return eConvertError;
  }

  QFileInfo clFile(m_strVerigyEdlFileName);
  m_strDataFilePath = clFile.absolutePath();

  int iIndex = 0;
  int nStatus = eConvertSuccess;
  while((!f.atEnd()) && (nStatus == eConvertSuccess))
  {
    iIndex++;
    nStatus = ReadEventRecord(&f);
  }

  if((nStatus == eConvertSuccess) && (!m_bMrrWritten))
  {
    // have to correctly close the current STDF file
    nStatus &= WriteWrr();
    nStatus &= WriteMrr();
  }

  // Check if we could retrieve a date, else use .dat creation date
  QDateTime clDateTime = QDateTime::currentDateTime();
  clDateTime.setTimeSpec(Qt::UTC);
  if(m_lStartTime < 0)
    m_lStartTime = clDateTime.toTime_t();
  if(m_lEndTime < 0)
    m_lEndTime = clDateTime.toTime_t();

  // Check if file date is not more recent than license expiration date!
  if(CheckValidityDate(pExpirationDate) == false)
  {
    // Incorrect file date (greater than license expiration date)...refuse to convert file!
    m_iLastError = errLicenceExpired;

    // Convertion failed.
    nStatus = eConvertDelay;
  }

  f.close();

  if(nStatus != eConvertSuccess)
  {
    QFile::remove(m_strStdfFileName);
    for(int i=0; i<m_lstStdfFileName.count(); i++)
      QFile::remove(m_lstStdfFileName[i]);
    m_lstStdfFileName.clear();

    if((m_iLastError == errNoError) || (m_iLastError == errWarning))
      m_iLastError = errConversionFailed;
    return nStatus;
  }

  // Success parsing VERIGY_EDL file
  return eConvertSuccess;
}

//////////////////////////////////////////////////////////////////////
// Read EDL Event Record
//////////////////////////////////////////////////////////////////////
int CGVERIGY_EDLtoSTDF::ReadEventRecord(QFile* pFile)
{
  long						lDword;
  short						sWord;
  CGVERIGY_EDL_StoredBuffer	clBinaryObject;
  const char *				event_names[]	= EVENT_NAMES;

  // Read first 6 Bytes
  m_nBlockData = ReadBlock(pFile,m_szBlock, 6);
  if(m_nBlockData != 6)
    return eConvertError;

  //Record Length : 4 Bytes
  m_nBlockIndex += clBinaryObject.ReadDword(m_szBlock, &lDword);
  // Type : 2 Bytes
  m_nBlockIndex += clBinaryObject.ReadWord(m_szBlock+4, &sWord);

  if((sWord <= 0)
  )//|| ((sWord+1) > NumberOfEventTypes))
  {
    // Incorrect type
    return eConvertError;
  }
  if(m_szBlock)
  {
    // Check if have to resize
    if(m_nBlockSize < lDword)
    {
      free(m_szBlock);
      m_szBlock = NULL;
    }
  }
  if(m_szBlock == NULL)
  {
    m_nBlockSize = lDword;
    m_szBlock = (char *) malloc((m_nBlockSize)*sizeof(char));
  }
  if(m_szBlock == NULL)
  {
    m_iLastError = errConversionFailed;
    return eConvertError;
  }

  // Read Data
  m_nBlockData = ReadBlock(pFile,m_szBlock, lDword);
  if((sWord+1) > NumberOfEventTypes)
  {
    // Unknown type
    // Ignore it
    return eConvertSuccess;
  }


  m_nBlockIndex = 0;

  bool bIgnoreThisEvent = false;
  int  nStatus = eConvertSuccess;
  int	 iTestNumber=0;

  m_nEventRecordsCount[sWord]++;

  // BEFORE PARSE THIS EVENT
  switch(sWord)
  {
  case  TestProgramStartEventType:	/* 1 */
    break;
  case  TestProgramEndEventType:		/* 2 */
    break;
  case  TestFlowStartEventType:		/* 3 */
    if((!m_mapAssignment["LOT_ID"].isEmpty())
    && (!m_strLotId.isEmpty())
    && (m_strLotId != m_mapAssignment["LOT_ID"]))
    {
      if(m_bSplitMultiLot)
      {
        m_bHaveMultiLot = true;
        // Have to close current STDF file
        // and
        // Have to open new STDF file
        nStatus  = WriteWrr();
        nStatus &= WriteMrr();
      }
      else
      {
        nStatus  = WriteWrr();
        nStatus &= WriteMrr();
        m_iLastError = errMultiLot;
        nStatus = eConvertError;
        return nStatus;
      }

    }
    m_strLotId = m_mapAssignment["LOT_ID"];

    if((m_strWaferId != "INVALID")
    && (m_strWaferId != m_mapAssignment["WAFER_ID"]))
      nStatus = WriteWrr();

    m_lstSites.clear();
    break;
  case  TestFlowEndEventType:			/* 4 */
    break;
  case  TestSuiteStartEventType:		/* 5 */
    break;
  case  TestSuiteEndEventType:		/* 6 */
    break;
  case  TestFunctionStartEventType:	/* 7 */
    break;
  case  TestFunctionEndEventType:		/* 8 */
    break;
  case  UserProcedureEventType:		/* 9 */
    break;
  case  TestEventType:				/* 10 */
    iTestNumber = m_iTestNumber;
    break;
  case  AssignmentEventType:			/* 11 */
    break;
  case  BinReachedEventType:			/* 12 */
    break;
  case  ConfigurationChangedEventType:/* 13 */
    break;
  case  AppModelLevelStartEventType:	/* 14 */
    break;
  case  AppModelLevelEndEventType:	/* 15 */
    break;
  case  PauseEventType:				/* 16 */
    break;
  case  AlarmEventType:				/* 17 */
    break;
  case  WarningEventType:				/* 18 */
    break;
  case  PrintToDatalogEventType:      /* 19 */
    break;
  case  GenericDataEventType:         /* 20 */
    m_lstGenericData.clear();
    break;
  case  ManualExecutionEventType:     /* 21 */
    break;
  case  AppModelLevelLoopEventType:   /* 22 */
    break;
  case  UserProcedureStartEventType:	/* 23 */
    break;
  default:
    bIgnoreThisEvent = true;
  }

  if((bIgnoreThisEvent) || (nStatus != eConvertSuccess))
  {
    m_nBlockIndex += lDword;
  }
  else
  {
    // Then Read Attribute Record
    while((m_nBlockIndex < m_nBlockData) && (nStatus == eConvertSuccess))
      nStatus = ReadAttributeRecord();
  }

  if(nStatus != eConvertSuccess)
  {
    if(m_strErrorDetail.isEmpty())
    {
      m_strErrorDetail = "Unexpected Events [";
      m_strErrorDetail+= event_names[sWord];
      m_strErrorDetail+= "]";
    }
    return nStatus;
  }

  // AFTER PARSE THIS EVENT
  switch(sWord)
  {
  case  TestProgramStartEventType:	/* 1 */
    if((m_nEventRecordsCount[TestProgramStartEventType] == 2)
        && m_bMirWritten)
    {
      // Add a warning
      if(!m_strErrorDetail.isEmpty()) m_strErrorDetail+=".\n";
      m_strErrorDetail+= "Event[";
      m_strErrorDetail+= event_names[TestProgramStartEventType];
      m_strErrorDetail+= "] occurs more than one time. New attributes are ignored";
      m_iLastError = errWarning;
    }
    break;
  case  TestProgramEndEventType:		/* 2 */
    if(m_nEventRecordsCount[TestProgramStartEventType] == 0)
    {
      // Ignore this event
      // Add a warning
      if(!m_strErrorDetail.isEmpty()) m_strErrorDetail+=".\n";
      m_strErrorDetail+= "Event[";
      m_strErrorDetail+= event_names[TestProgramEndEventType];
      m_strErrorDetail+= "] occurs without event[";
      m_strErrorDetail+= event_names[TestProgramStartEventType];
      m_strErrorDetail+= "]";
      m_iLastError = errWarning;
    }
    else
    {
      // 2009 07 17
      // Can have multi "ProgramStart" and "ProgramEnd" like "FlowStart" and "FlowEnd"
      // Close the Stdf file only at the end of the parse
      // If other "ProgramStart" then Warning
      //nStatus  = WriteWrr();
      //nStatus &= WriteMrr();
    }
    break;
  case  TestFlowStartEventType:		/* 3 */
    // New Part
    nStatus  = WriteMir();
    nStatus &= WriteWir();
    nStatus &= WritePir();
    break;
  case  TestFlowEndEventType:			/* 4 */
    if(m_nEventRecordsCount[TestFlowStartEventType] == 0)
    {
      // Ignore this event
      // Add a warning
      if(!m_strErrorDetail.isEmpty()) m_strErrorDetail+=".\n";
      m_strErrorDetail+= "Event[";
      m_strErrorDetail+= event_names[sWord];
      m_strErrorDetail+= "] occurs without event[";
      m_strErrorDetail+= event_names[TestFlowStartEventType];
      m_strErrorDetail+= "]";
      m_iLastError = errWarning;
    }
    else
      nStatus  = WritePrr();
    break;
  case  TestSuiteStartEventType:		/* 5 */
    break;
  case  TestSuiteEndEventType:		/* 6 */
    break;
  case  TestFunctionStartEventType:	/* 7 */
    break;
  case  TestFunctionEndEventType:		/* 8 */
    break;
  case  UserProcedureEventType:		/* 9 */
    // Have result
    if(m_nEventRecordsCount[TestFlowStartEventType] == 0)
    {
      // Ignore this event
      // Add a warning
      if(!m_strErrorDetail.isEmpty()) m_strErrorDetail+=".\n";
      m_strErrorDetail+= "Event[";
      m_strErrorDetail+= event_names[sWord];
      m_strErrorDetail+= "] occurs without event[";
      m_strErrorDetail+= event_names[TestFlowStartEventType];
      m_strErrorDetail+= "]";
      m_iLastError = errWarning;
    }
    else
      nStatus  = WriteResult();
    break;
  case  TestEventType:				/* 10 */
    // Have result
    if(m_nEventRecordsCount[TestFlowStartEventType] == 0)
    {
      // Ignore this event
      // Add a warning
      if(!m_strErrorDetail.isEmpty()) m_strErrorDetail+=".\n";
      m_strErrorDetail+= "Event[";
      m_strErrorDetail+= event_names[sWord];
      m_strErrorDetail+= "] occurs without event[";
      m_strErrorDetail+= event_names[TestFlowStartEventType];
      m_strErrorDetail+= "]";
      m_iLastError = errWarning;
    }
    else
      nStatus  = WriteResult();
    m_iTestNumber = iTestNumber;
    break;
  case  AssignmentEventType:			/* 11 */
    break;
  case  BinReachedEventType:			/* 12 */
    break;
  case  ConfigurationChangedEventType:/* 13 */
    break;
  case  AppModelLevelStartEventType:	/* 14 */
    break;
  case  AppModelLevelEndEventType:	/* 15 */
    break;
  case  PauseEventType:				/* 16 */
    break;
  case  AlarmEventType:				/* 17 */
    break;
  case  WarningEventType:				/* 18 */
    break;
  case  PrintToDatalogEventType:      /* 19 */
    if(m_nEventRecordsCount[TestFlowStartEventType] == 0)
    {
      // Ignore this event
      // Add a warning
      if(!m_strErrorDetail.isEmpty()) m_strErrorDetail+=".\n";
      m_strErrorDetail+= "Event[";
      m_strErrorDetail+= event_names[sWord];
      m_strErrorDetail+= "] occurs without event[";
      m_strErrorDetail+= event_names[TestFlowStartEventType];
      m_strErrorDetail+= "]";
      m_iLastError = errWarning;
    }
    else
      nStatus  = WriteDtr();
    break;
  case  GenericDataEventType:         /* 20 */
    if(m_nEventRecordsCount[TestFlowStartEventType] == 0)
    {
      // Ignore this event
      // Add a warning
      if(!m_strErrorDetail.isEmpty()) m_strErrorDetail+=".\n";
      m_strErrorDetail+= "Event[";
      m_strErrorDetail+= event_names[sWord];
      m_strErrorDetail+= "] occurs without event[";
      m_strErrorDetail+= event_names[TestFlowStartEventType];
      m_strErrorDetail+= "]";
      m_iLastError = errWarning;
    }
    else
      nStatus  = WriteGdr();
    break;
  case  ManualExecutionEventType:     /* 21 */
    break;
  case  AppModelLevelLoopEventType:   /* 22 */
    break;
  case  UserProcedureStartEventType:	/* 23 */
    break;
  default:
    bIgnoreThisEvent = true;
  }

  if(nStatus != eConvertSuccess)
  {
    if(m_strErrorDetail.isEmpty())
    {
      m_strErrorDetail = "Unexpected Event[";
      m_strErrorDetail+= event_names[sWord];
      m_strErrorDetail+= "]";
    }

    return nStatus;
  }

  if(m_nBlockIndex != m_nBlockData)
    return eConvertSuccess; // eConvertError

  return eConvertSuccess;
}


//////////////////////////////////////////////////////////////////////
// Read EDL Attribute Record
//////////////////////////////////////////////////////////////////////
int CGVERIGY_EDLtoSTDF::ReadAttributeRecord()
{
  BYTE						bByte;
  char						szString[STRING_MAX];
  short						sWord;
  long						lDword;
  double						dDouble;
  CGVERIGY_EDL_StoredBuffer	clBinaryObject;
  int							nLength;
  int							nIndex;
  int							nLoop;
  int							i,j;
  bool                      bIsNum;
  char*						szStringBlock;
  QString strHBinName;
  QString strSBinName;
  QString strSBinNum;

  CGVERIGY_EDLSiteInfo * pSiteInfo;

  //Record Length : 4 Bytes
  m_nBlockIndex += clBinaryObject.ReadDword(m_szBlock+m_nBlockIndex, &lDword);
  // Type : 2 Bytes
  m_nBlockIndex += clBinaryObject.ReadWord(m_szBlock+m_nBlockIndex, &sWord);

  QString strString;

  if((sWord <= 0)
  || ((sWord+1) > NumberOfAttributeTypes))
  {
    // Unknown attribute
    // Ignore it
    // Go to the end
    m_nBlockIndex += lDword;
    return eConvertSuccess;
  }

  if(m_szBlock)
  {
    // Check if have to resize
    if(m_nBlockData < (m_nBlockIndex + lDword))
      return eConvertError;
  }


  nIndex = m_nBlockIndex;
  nLength = lDword;

  switch(sWord)
  {
  case  DeviceDirectoryAttrType:		  /*  1 */
    //device_directory string Name of the device data directory
    m_nBlockIndex += clBinaryObject.ReadString(m_szBlock+m_nBlockIndex,szString);
    break;
  case  TestProgramNameAttrType:        /*  2 */
    //test_program_name string File name of test program
    m_nBlockIndex += clBinaryObject.ReadString(m_szBlock+m_nBlockIndex,szString);
    m_mapAssignment["JOB_NAM"] = szString;
    break;
  case  WorkorderNameAttrType:          /*  3 */
    //workorder_name string Path and file name of the workorder
    m_nBlockIndex += clBinaryObject.ReadString(m_szBlock+m_nBlockIndex,szString);
    break;
  case  PinListAttrType:                /*  4 */
    //sites unsigned char Number of test sites, determines the number of the channel field
    m_nBlockIndex += clBinaryObject.ReadByte(m_szBlock+m_nBlockIndex,&bByte);
    while((m_nBlockIndex - nIndex) < nLength)
    {
      CGVERIGY_EDLPinInfo* pNewPin = new CGVERIGY_EDLPinInfo();
      //capabilities unsigned int
      m_nBlockIndex += clBinaryObject.ReadDword(m_szBlock+m_nBlockIndex,&lDword);
      for(i=0; i<bByte; i++)
      {
        //channel unsigned int channel number, occurs site times
        m_nBlockIndex += clBinaryObject.ReadDword(m_szBlock+m_nBlockIndex,&lDword);
        pNewPin->mapSiteChannel[i] = lDword;
        pNewPin->mapSiteIndex[i] = m_qMapPins.count()+1;
      }
      //name string
      m_nBlockIndex += clBinaryObject.ReadString(m_szBlock+m_nBlockIndex,szString);
      pNewPin->strName = szString;
      //number string
      m_nBlockIndex += clBinaryObject.ReadString(m_szBlock+m_nBlockIndex,szString);
      pNewPin->strNumber = szString;
      bool bIsInt;
      lDword = QString(szString).toInt(&bIsInt);
      //if(!bIsInt)
      lDword = m_qMapPins.count();

      m_qMapPins[lDword] = pNewPin;
    }
    if(!m_qMapPins.isEmpty())
    {
      QMap<int,CGVERIGY_EDLPinInfo*>::Iterator itMapPin;
      for(i=1; i<bByte; i++)
      {
        for ( itMapPin = m_qMapPins.begin(); itMapPin != m_qMapPins.end(); ++itMapPin )
        {
          itMapPin.value()->mapSiteIndex[i] = m_qMapPins.count() + itMapPin.value()->mapSiteIndex[i-1];
        }
      }
    }

    break;
  case  UIDAttrType:	                  /*  5 */
    //user_id string User Id
    m_nBlockIndex += clBinaryObject.ReadString(m_szBlock+m_nBlockIndex,szString);
    m_mapAssignment["OPER_NAM"] = szString;
    break;
  case  HostNameAttrType:               /*  6 */
    //host_name string Host name
    m_nBlockIndex += clBinaryObject.ReadString(m_szBlock+m_nBlockIndex,szString);
    m_mapAssignment["NODE_NAM"] = szString;
    break;
  case  NamedValueAttrType:             /*  7 */
    //name string Name
    m_nBlockIndex += clBinaryObject.ReadString(m_szBlock+m_nBlockIndex,szString);
    strString = QString(szString).trimmed().toUpper();
    if(strString.count("ALARM_ID") > 0)
      strString = "ALARM_ID";
    else if(strString.count("CENTER_Y") > 0)
      strString = "CENTER_Y";
    else if(strString.count("CENTER_X") > 0)
      strString = "CENTER_X";
    else if(strString.count("DIE_HT") > 0)
      strString = "DIE_HT";
    else if(strString.count("DIE_WID") > 0)
      strString = "DIE_WID";
    else if(strString.count("DISP_COD") > 0)
      strString = "DISP_COD";
    else if(strString.count("EXC_DESC") > 0)
      strString = "EXC_DESC";
    else if(strString.count("FRAME_ID") > 0)
      strString = "FRAME_ID";
    else if(strString.count("FABWF_ID") > 0)
      strString = "FABWF_ID";
    else if(strString.count("MASK_ID") > 0)
      strString = "MASK_ID";
    else if(strString.count("OP_CODE") > 0)
      strString = "OP_CODE";
    else if(strString.count("PART_FIX") > 0)
      strString = "PART_FIX";
    else if(strString.count("PART_TXT") > 0)
      strString = "PART_TXT";
    else if(strString.count("PROG_TXT") > 0)
      strString = "PROG_TXT";
    else if(strString.count("POS_Y") > 0)
      strString = "POS_Y";
    else if(strString.count("POS_X") > 0)
      strString = "POS_X";
    else if(strString.count("RSLT_TXT") > 0)
      strString = "RSLT_TXT";
    else if(strString.count("SITE_NUM") > 0)
      strString = "SITE_NUM";
    else if(strString.count("TIME_SET") > 0)
      strString = "TIME_SET";
    else if(strString.count("USR_DESC") > 0)
      strString = "USR_DESC";
    else if(strString.count("VECT_NAM") > 0)
      strString = "VECT_NAM";
    else if(strString.count("WAFER_ID") > 0)
      strString = "WAFER_ID";
    else if(strString.count("WAFR_SIZ") > 0)
      strString = "WAFR_SIZ";
    else if(strString.count("WF_FLAT") > 0)
      strString = "WF_FLAT";
    else if(strString.count("WF_UNITS") > 0)
      strString = "WF_UNITS";
    else if(strString.count("USR_DESC") > 0)
      strString = "USR_DESC";
    else if(strString.count("AUX_FILE") > 0)
      strString = "AUX_FILE";
    else if(strString.count("PKG_TYP") > 0)
      strString = "PKG_TYP";
    else if(strString.count("FAMLY_ID") > 0)
      strString = "FAMLY_ID";
    else if(strString.count("DATE_COD") > 0)
      strString = "DATE_COD";
    else if(strString.count("FACIL_ID") > 0)
      strString = "FACIL_ID";
    else if(strString.count("FLOOR_ID") > 0)
      strString = "FLOOR_ID";
    else if(strString.count("PROC_ID") > 0)
      strString = "PROC_ID";
    else if(strString.count("OPER_FRQ") > 0)
      strString = "OPER_FRQ";
    else if(strString.count("SPEC_NAM") > 0)
      strString = "SPEC_NAM";
    else if(strString.count("SPEC_VER") > 0)
      strString = "SPEC_VER";
    else if(strString.count("FLOW_ID") > 0)
      strString = "FLOW_ID";
    else if(strString.count("SETUP_ID") > 0)
      strString = "SETUP_ID";
    else if(strString.count("DSGN_REV") > 0)
      strString = "DSGN_REV";
    else if(strString.count("ENG_ID") > 0)
      strString = "ENG_ID";
    else if(strString.count("ROM_COD") > 0)
      strString = "ROM_COD";
    else if(strString.count("ROMCOD") > 0)
      strString = "ROM_COD";
    else if(strString.count("SERL_NUM") > 0)
      strString = "SERL_NUM";
    else if(strString.count("SUPR_NAM") > 0)
      strString = "SUPR_NAM";
    else if(strString.count("MODE_COD") > 0)
      strString = "MODE_COD";
    else if(strString.count("RTST_COD") > 0)
      strString = "RTST_COD";
    else if(strString.count("PROT_COD") > 0)
      strString = "PROT_COD";
    else if(strString.count("BURN_TIM") > 0)
      strString = "BURN_TIM";
    else if(strString.count("CMOD_COD") > 0)
      strString = "CMOD_COD";
    else if(strString.count("SBLOT_ID") > 0)
      strString = "SBLOT_ID";
    else if(strString.count("SUBLOTID") > 0)
      strString = "SBLOT_ID";
    else if(strString.count("LOTID") > 0)
      strString = "LOT_ID";
    else if(strString.count("LOT_ID") > 0)
      strString = "LOT_ID";
    else if(strString.count("PART_TYP") > 0)
      strString = "PART_TYP";
    else if(strString.count("NODE_NAM") > 0)
      strString = "NODE_NAM";
    else if(strString.count("TSTR_TYP") > 0)
      strString = "TSTR_TYP";
    else if(strString.count("TESTERTYPE") > 0)
      strString = "TSTR_TYP";
    else if(strString.count("JOB_NAM") > 0)
      strString = "JOB_NAM";
    else if(strString.count("JOB_REV") > 0)
      strString = "JOB_REV";
    else if(strString.count("OPER_NAM") > 0)
      strString = "OPER_NAM";
    else if(strString.count("OPERATOR") > 0)
      strString = "OPER_NAM";
    else if(strString.count("EXEC_TYP") > 0)
      strString = "EXEC_TYP";
    else if(strString.count("EXEC_VER") > 0)
      strString = "EXEC_VER";
    else if(strString.count("TEST_COD") > 0)
      strString = "TEST_COD";
    else if(strString.count("TST_TEMP") > 0)
      strString = "TST_TEMP";
    else if(strString.count("TEMPERATURE") > 0)
      strString = "TST_TEMP";
    else if(strString.count("HAND_TYP") > 0)
      strString = "HAND_TYP";
    else if(strString.count("HAND_ID") > 0)
      strString = "HAND_ID";
    else if(strString.count("PROBER") > 0)
      strString = "HAND_ID";
    else if(strString.count("HANDLER") > 0)
      strString = "HAND_ID";
    else if(strString.count("CARD_TYP") > 0)
      strString = "CARD_TYP";
    else if(strString.count("CARD_ID") > 0)
      strString = "CARD_ID";
    else if(strString.count("PROBECARD") > 0)
      strString = "CARD_ID";
    else if(strString.count("LOAD_TYP") > 0)
      strString = "LOAD_TYP";
    else if(strString.count("LOAD_ID") > 0)
      strString = "LOAD_ID";
    else if(strString.count("LOADBOARD") > 0)
      strString = "LOAD_ID";
    else if(strString.count("DIB_TYP") > 0)
      strString = "DIB_TYP";
    else if(strString.count("DIB_ID") > 0)
      strString = "DIB_ID";
    else if(strString.count("CABL_TYP") > 0)
      strString = "CABL_TYP";
    else if(strString.count("CABL_ID") > 0)
      strString = "CABL_ID";
    else if(strString.count("CONT_TYP") > 0)
      strString = "CONT_TYP";
    else if(strString.count("CONT_ID") > 0)
      strString = "CONT_ID";
    else if(strString.count("LASR_TYP") > 0)
      strString = "LASR_TYP";
    else if(strString.count("LASR_ID") > 0)
      strString = "LASR_ID";
    else if(strString.count("EXTR_TYP") > 0)
      strString = "EXTR_TYP";
    else if(strString.count("EXTR_ID") > 0)
      strString = "EXTR_ID";
    else if(strString.count("RTST_BIN") > 0)
      strString = "RTST_BIN";
    else if(strString.count("RETESTEDBINS") > 0)
      strString = "RTST_BIN";

    //type char Type of the s/i/d_value field, values:
    m_nBlockIndex += clBinaryObject.ReadByte(m_szBlock+m_nBlockIndex,&bByte);
    //• 1 = DRLStringValue
    //• 2 = DRLIntegerValuer
    //• 3 = DRLDoubleValue
    if(bByte == 1)
    {
      //s_value string if type = DRLStringValue
      m_nBlockIndex += clBinaryObject.ReadString(m_szBlock+m_nBlockIndex,szString);
      if(strString.toUpper() == "WAFER_ID")
        m_bWaferMap = true;
      if(strString.toUpper() == "START_TIME") //20051213 123813
      {
        strString = szString;
        strString = strString.insert(13,':').insert(11,':').insert(6,'/').insert(4,'/');
        long lTime = QDateTime::fromString(szString,Qt::ISODate).toTime_t();
        if(lTime > 0)
          m_lStartTime = lTime;
      }

      m_mapAssignment[strString.toUpper()] = szString;
    }
    if(bByte == 2)
    {
      //i_value int if type = DRLIntegerValue
      m_nBlockIndex += clBinaryObject.ReadDword(m_szBlock+m_nBlockIndex,&lDword);
      m_mapAssignment[strString.toUpper()] = QString::number(lDword);
    }
    if(bByte == 3)
    {
      //d_value double if type = DRLDoubleValue
      m_nBlockIndex += clBinaryObject.ReadDouble(m_szBlock+m_nBlockIndex,&dDouble);
      m_mapAssignment[strString.toUpper()] = QString::number(dDouble);
    }
    break;
  case  DeviceIdAttrType:               /*  8 */
    //site_no char Site number
    m_nBlockIndex += clBinaryObject.ReadByte(m_szBlock+m_nBlockIndex,&bByte);
    m_mapAssignment["SITE_NUM"] = QString::number(bByte);
    if(!m_lstSites.contains(m_mapAssignment["SITE_NUM"]))
      m_lstSites.append(m_mapAssignment["SITE_NUM"]);

    //device_id string Device id
    m_nBlockIndex += clBinaryObject.ReadString(m_szBlock+m_nBlockIndex,szString);
    strString = szString;

    if(m_mapSiteInfo.contains(m_mapAssignment["SITE_NUM"]))
      pSiteInfo = m_mapSiteInfo[m_mapAssignment["SITE_NUM"]];
    else
    {
      pSiteInfo = new CGVERIGY_EDLSiteInfo();
      m_mapSiteInfo[m_mapAssignment["SITE_NUM"]] = pSiteInfo;
    }

    // Find section delimiter
    if(strString.indexOf(",") > -1)
      szString[0] = ',';
    else
      szString[0] = '.';
    szString[1] = '\0';
    if(strString.startsWith("P", Qt::CaseInsensitive))
    {
      // P<PartNumber>.<SiteNumber>
      if(!m_mapPartNumber.contains(strString))
        m_mapPartNumber[strString] = m_mapPartNumber.count()+1;
      m_nPartNumber = m_mapPartNumber[strString];
      pSiteInfo->nPartNumber = m_nPartNumber;
      pSiteInfo->nXWafer = -32768;
      pSiteInfo->nYWafer = -32768;
      pSiteInfo->bExecuted = true;
    }
    if(strString.startsWith("W", Qt::CaseInsensitive))
    {
      // W<XLoc>.<YLoc>
      if(!m_mapPartNumber.contains(strString+"."+m_mapAssignment["SITE_NUM"]))
        m_mapPartNumber[strString+"."+m_mapAssignment["SITE_NUM"]] = m_mapPartNumber.count()+1;
      m_nPartNumber = m_mapPartNumber[strString+"."+m_mapAssignment["SITE_NUM"]];
      m_bWaferMap = true;

      m_nXLocation = strString.mid(1).section(szString,0,0).toInt();
      m_nYLocation = strString.mid(1).section(szString,1).toInt();
      if(m_nXLocation == -9999)
        m_nXLocation = -32768;
      if(m_nYLocation == -9999)
        m_nYLocation = -32768;
      pSiteInfo->nPartNumber = m_nPartNumber;
      pSiteInfo->nXWafer = m_nXLocation;
      pSiteInfo->nYWafer = m_nYLocation;
      pSiteInfo->bExecuted = true;
    }
    break;
  case  SiteSetAttrType:                /*  9 */
    while((m_nBlockIndex - nIndex) < nLength)
    {
      //site_no char Site number,the first site is site number 1
      m_nBlockIndex += clBinaryObject.ReadByte(m_szBlock+m_nBlockIndex,&bByte);
      //if(!m_lstSites.contains(QString::number(bByte)))
      //	m_lstSites.append(QString::number(bByte));
    }
    break;
  case  SiteIdAttrType:                 /* 10 */
    //site_no char Site number (Id), the first site is site number 1
    m_nBlockIndex += clBinaryObject.ReadByte(m_szBlock+m_nBlockIndex,&bByte);
    m_mapAssignment["SITE_NUM"] = QString::number(bByte);
    break;
  case  TestSuiteNameAttrType:          /* 11 */
    //name string Testsuite name
    m_nBlockIndex += clBinaryObject.ReadString(m_szBlock+m_nBlockIndex,szString);
    m_strTestSuiteName = szString;
    break;
  case  PassFailResultAttrType:         /* 12 */
    //failure_code char Failure code, values as defined in dcl_drl.h:
    m_nBlockIndex += clBinaryObject.ReadByte(m_szBlock+m_nBlockIndex,&bByte);
    m_nTestPFResult = bByte;
    //• -1= DRLFailureCode_NA: no pass/fail result determined
    //• 0 = DRLFailureCode_Pass: passed
    //• 1 = DRLFailureCode_Fail: failed
    //• 2 = DRLFailureCode_Error: error
    //• 3 = DRLFailureCode_Bypass: bypassed, only applicable for testsuites
    //description string Description
    m_nBlockIndex += clBinaryObject.ReadString(m_szBlock+m_nBlockIndex,szString);
    break;
  case  ValueResultAttrType:            /* 13 */
    //value double Value
    m_nBlockIndex += clBinaryObject.ReadDouble(m_szBlock+m_nBlockIndex,&dDouble);
    //unit string Unit
    m_nBlockIndex += clBinaryObject.ReadString(m_szBlock+m_nBlockIndex,szString);
    m_strUnit = PrefixUnitToUnit(szString);
    m_fTestResult = dDouble / GS_POW(10.0,PrefixUnitToScall(szString));
    m_bTestResult = true;
    //description string Description of the measurement, for example that this is the result of a pre-test.
    m_nBlockIndex += clBinaryObject.ReadString(m_szBlock+m_nBlockIndex,szString);
    break;
  case  PinValueResultAttrType:         /* 14 */
    m_lstPinResult.clear();
    m_lstPinIndex.clear();
    m_lstPinStat.clear();
    //unit string
    m_nBlockIndex += clBinaryObject.ReadString(m_szBlock+m_nBlockIndex,szString);
    m_strUnit = szString;
    //description string Number of test suites, determines the number of all following ??? fields
    m_nBlockIndex += clBinaryObject.ReadString(m_szBlock+m_nBlockIndex,szString);
    while((m_nBlockIndex - nIndex) < nLength)
    {
      //failure_code char Failure code, values as defined in dcl_drl.h:
      m_nBlockIndex += clBinaryObject.ReadByte(m_szBlock+m_nBlockIndex,&bByte);
      //• -1= DRLFailureCode_NA: no pass/fail result determined
      //• 0 = DRLFailureCode_Pass: passed
      //• 1 = DRLFailureCode_Fail: failed
      //• 2 = DRLFailureCode_Error: error
      //• 3 = DRLFailureCode_Bypass: bypassed, only applicable for testsuites
      //value double Value
      m_nBlockIndex += clBinaryObject.ReadDouble(m_szBlock+m_nBlockIndex,&dDouble);
      //pin_index short Pin index
      m_nBlockIndex += clBinaryObject.ReadWord(m_szBlock+m_nBlockIndex,&sWord);
      if(sWord == -1)
      {
        // Invalid pin
        m_bTestResult = true;
        m_nScale = PrefixUnitToScall(m_strUnit);
        m_fTestResult = dDouble / GS_POW(10.0,m_nScale);
        continue;
      }
      m_lstPinStat.append(QString::number(bByte));
      m_lstPinResult.append(QString::number(dDouble / GS_POW(10.0,PrefixUnitToScall(m_strUnit))));
      m_lstPinIndex.append(QString::number(sWord));
      // Check if in m_qMapPins
      if(!m_qMapPins.contains(sWord))
      {
        CGVERIGY_EDLPinInfo* pNewPin = new CGVERIGY_EDLPinInfo();
        pNewPin->mapSiteChannel[m_lstSites.indexOf(m_mapAssignment["SITE_NUM"])] = 0;
        pNewPin->mapSiteIndex[m_lstSites.indexOf(m_mapAssignment["SITE_NUM"])] = m_qMapPins.count()+m_lstSites.indexOf(m_mapAssignment["SITE_NUM"]);
        pNewPin->strNumber = QString::number(sWord);
        pNewPin->strName = "UNKNOWN_PIN";

        m_qMapPins[sWord] = pNewPin;
      }
    }
    m_nScale = PrefixUnitToScall(m_strUnit);
    m_strUnit = PrefixUnitToUnit(m_strUnit);

    break;
  case  PinPFResultAttrType:            /* 15 */
    m_lstPinResult.clear();
    m_lstPinIndex.clear();
    m_lstPinStat.clear();

    //description string Description
    m_nBlockIndex += clBinaryObject.ReadString(m_szBlock+m_nBlockIndex,szString);
    while((m_nBlockIndex - nIndex) < nLength)
    {
      //pin_index short Pin index
      m_nBlockIndex += clBinaryObject.ReadWord(m_szBlock+m_nBlockIndex,&sWord);
      m_lstPinIndex.append(QString::number(sWord));
      // Check if in m_qMapPins
      if(!m_qMapPins.contains(sWord))
      {
        CGVERIGY_EDLPinInfo* pNewPin = new CGVERIGY_EDLPinInfo();
        pNewPin->mapSiteChannel[m_lstSites.indexOf(m_mapAssignment["SITE_NUM"])] = 0;
        pNewPin->mapSiteIndex[m_lstSites.indexOf(m_mapAssignment["SITE_NUM"])] = m_qMapPins.count()+m_lstSites.indexOf(m_mapAssignment["SITE_NUM"]);
        pNewPin->strNumber = QString::number(sWord);
        pNewPin->strName = "UNKNOWN_PIN";

        m_qMapPins[sWord] = pNewPin;
      }

      //failure_code char
      m_nBlockIndex += clBinaryObject.ReadByte(m_szBlock+m_nBlockIndex,&bByte);
      m_lstPinStat.append(QString::number(bByte));
    }
    break;
  case  PinRangeResultAttrType:         /* 16 */
    m_lstPinResult.clear();
    m_lstPinIndex.clear();
    m_lstPinStat.clear();

    //unit string Unit
    m_nBlockIndex += clBinaryObject.ReadString(m_szBlock+m_nBlockIndex,szString);
    m_strUnit = PrefixUnitToUnit(szString);
    //description string Description
    m_nBlockIndex += clBinaryObject.ReadString(m_szBlock+m_nBlockIndex,szString);
    while((m_nBlockIndex - nIndex) < nLength)
    {
      //pin_index short Pin index
      m_nBlockIndex += clBinaryObject.ReadWord(m_szBlock+m_nBlockIndex,&sWord);
      m_lstPinIndex.append(QString::number(sWord));
      m_lstPinIndex.append(QString::number(sWord));
      // Check if in m_qMapPins
      if(!m_qMapPins.contains(sWord))
      {
        CGVERIGY_EDLPinInfo* pNewPin = new CGVERIGY_EDLPinInfo();
        pNewPin->mapSiteChannel[m_lstSites.indexOf(m_mapAssignment["SITE_NUM"])] = 0;
        pNewPin->mapSiteIndex[m_lstSites.indexOf(m_mapAssignment["SITE_NUM"])] = m_qMapPins.count()+m_lstSites.indexOf(m_mapAssignment["SITE_NUM"]);
        pNewPin->strNumber = QString::number(sWord);
        pNewPin->strName = "UNKNOWN_PIN";

        m_qMapPins[sWord] = pNewPin;
      }

      //failure_code char Failure code, values as defined in dcl_drl.h:
      m_nBlockIndex += clBinaryObject.ReadByte(m_szBlock+m_nBlockIndex,&bByte);
      m_lstPinStat.append(QString::number(bByte));
      m_lstPinStat.append(QString::number(bByte));
      //• -1= DRLFailureCode_NA: no pass/fail result determined
      //• 0 = DRLFailureCode_Pass: passed
      //• 1 = DRLFailureCode_Fail: failed
      //• 2 = DRLFailureCode_Error: error
      //• 3 = DRLFailureCode_Bypass: bypassed, only applicable for testsuites
      //min_value double Minimum value
      m_nBlockIndex += clBinaryObject.ReadDouble(m_szBlock+m_nBlockIndex,&dDouble);
      m_lstPinResult.append(QString::number(dDouble / GS_POW(10.0,PrefixUnitToScall(m_strUnit))));
      //max_value double Maximum value
      m_nBlockIndex += clBinaryObject.ReadDouble(m_szBlock+m_nBlockIndex,&dDouble);
      m_lstPinResult.append(QString::number(dDouble / GS_POW(10.0,PrefixUnitToScall(m_strUnit))));
    }
    break;
  case  FailingCycleAttrType:	          /* 17 */
    //cycle_number double Cycle number where the pin(s) failed
    m_nBlockIndex += clBinaryObject.ReadDouble(m_szBlock+m_nBlockIndex,&dDouble);
    while((m_nBlockIndex - nIndex) < nLength)
      //pin_index short Pin index
      m_nBlockIndex += clBinaryObject.ReadWord(m_szBlock+m_nBlockIndex,&sWord);
    break;
  case  VectorResultAttrType:	          /* 18 */
    //cycle_number double Cycle number
    m_nBlockIndex += clBinaryObject.ReadDouble(m_szBlock+m_nBlockIndex,&dDouble);
    //vector_number long Vector number
    m_nBlockIndex += clBinaryObject.ReadDword(m_szBlock+m_nBlockIndex,&lDword);
    //sequencer_cmd char Sequencer command
    m_nBlockIndex += clBinaryObject.ReadByte(m_szBlock+m_nBlockIndex,&bByte);
    //label_type char Label type
    m_nBlockIndex += clBinaryObject.ReadByte(m_szBlock+m_nBlockIndex,&bByte);
    //vector_comment string Vector comment
    m_nBlockIndex += clBinaryObject.ReadString(m_szBlock+m_nBlockIndex,szString);
    //label_name string Label name
    m_nBlockIndex += clBinaryObject.ReadString(m_szBlock+m_nBlockIndex,szString);
    while((m_nBlockIndex - nIndex) < nLength)
    {
      //pin_index short Pin index
      m_nBlockIndex += clBinaryObject.ReadWord(m_szBlock+m_nBlockIndex,&sWord);
      for(i=0; i<7; i++)
        //device_cycle_name char Device cycle name , the number of characters is depending on the attribute type:
        m_nBlockIndex += clBinaryObject.ReadByte(m_szBlock+m_nBlockIndex,&bByte);
      //• VectorResultAttrType: 7 characters
      //• VectorResultLongNameAttrType: 17 characters
      //device_cycle_offset char Device cycle offset
      m_nBlockIndex += clBinaryObject.ReadByte(m_szBlock+m_nBlockIndex,&bByte);
      //d1 char Drive parameter 1 of the device cycle
      m_nBlockIndex += clBinaryObject.ReadByte(m_szBlock+m_nBlockIndex,&bByte);
      //d2 char Drive parameter 2 of the device cycle
      m_nBlockIndex += clBinaryObject.ReadByte(m_szBlock+m_nBlockIndex,&bByte);
      //e1 char Receive parameter 1 of the device cycle
      m_nBlockIndex += clBinaryObject.ReadByte(m_szBlock+m_nBlockIndex,&bByte);
      //e2 char Receive parameter 2 of the device cycle
      m_nBlockIndex += clBinaryObject.ReadByte(m_szBlock+m_nBlockIndex,&bByte);
      //received char Received data , the bits have the follwing meaning: ppeeerrr, p = parameter, e = edge, r = received
      m_nBlockIndex += clBinaryObject.ReadByte(m_szBlock+m_nBlockIndex,&bByte);
    }
    break;
  case  TestFunctionTypeAttrType:       /* 19 */
    //test_function_type string Indicates the executed test function, like leakage, functional, or continuity
    m_nBlockIndex += clBinaryObject.ReadString(m_szBlock+m_nBlockIndex,szString);
    break;
  case  TestFunctionParametersAttrType: /* 20 */
    //parameter_list string Semicolon or new line separated list of test function parameters
    m_nBlockIndex += clBinaryObject.ReadString(m_szBlock+m_nBlockIndex,szString);
    break;
  case  BinAttrType:	                  /* 21 */
    //is_good_bin char Good or bad bin specification: 1 if good, 0 else
    m_nBlockIndex += clBinaryObject.ReadByte(m_szBlock+m_nBlockIndex,&bByte);
    m_nPassStatus = bByte;
    //bin_number int Bin number
    m_nBlockIndex += clBinaryObject.ReadDword(m_szBlock+m_nBlockIndex,&lDword);
    m_nHardBin = lDword;
    //bin_name string Bin name
    m_nBlockIndex += clBinaryObject.ReadString(m_szBlock+m_nBlockIndex,szString);
    strSBinNum = szString;
    m_nSoftBin = strSBinNum.toInt(&bIsNum);
    if(!bIsNum)
    {
        if(!m_lstSoftBinIssue.contains(strSBinNum))
            m_lstSoftBinIssue.append(strSBinNum);
        m_nSoftBin = m_lstSoftBinIssue.indexOf(strSBinNum);
    }
    //bin_description string Software bin description
    m_nBlockIndex += clBinaryObject.ReadString(m_szBlock+m_nBlockIndex,szString);
    strSBinName = szString;

    //hw_bin_description string Hardware bin description optional
    if((m_nBlockIndex - nIndex) < nLength)
    {
      m_nBlockIndex += clBinaryObject.ReadString(m_szBlock+m_nBlockIndex,szString);
      strHBinName = szString;
    }
    if(strHBinName.isEmpty())
      strHBinName = strSBinName;

    // Check if already saved
    if(m_mapSiteInfo.contains(m_mapAssignment["SITE_NUM"]))
      pSiteInfo = m_mapSiteInfo[m_mapAssignment["SITE_NUM"]];
    else
    {
      pSiteInfo = new CGVERIGY_EDLSiteInfo();
      m_mapSiteInfo[m_mapAssignment["SITE_NUM"]] = pSiteInfo;
    }

    if(pSiteInfo->nPassStatus == -1)
    {
      pSiteInfo->bExecuted = true;
      pSiteInfo->nPassStatus = m_nPassStatus;
      pSiteInfo->nHardBin = m_nHardBin;
      pSiteInfo->nSoftBin = m_nSoftBin;


      CGVERIGY_EDLBinInfo *pBin;
      if(!m_qMapBins.contains(m_nHardBin))
      {
        pBin = new CGVERIGY_EDLBinInfo();
        pBin->bPass = (bByte == 1);
        pBin->nHardBin = m_nHardBin;
        pBin->strBinName = strHBinName;
        m_qMapBins[m_nHardBin] = pBin;
      }
      else
        pBin = m_qMapBins[m_nHardBin];
      pBin->UpdateCount(m_mapAssignment["SITE_NUM"].toInt());

      if(!m_qMapBins_Soft.contains(m_nSoftBin))
      {
        pBin = new CGVERIGY_EDLBinInfo();
        pBin->bPass = (bByte == 1);
        pBin->nHardBin = m_nHardBin;
        pBin->strBinName = strSBinName;
        m_qMapBins_Soft[m_nSoftBin] = pBin;
      }
      else
        pBin = m_qMapBins_Soft[m_nSoftBin];
      pBin->UpdateCount(m_mapAssignment["SITE_NUM"].toInt());
    }

    break;
  case  TestIdAttrType:		          /* 22 */
    //test_number int Test number
    m_nBlockIndex += clBinaryObject.ReadDword(m_szBlock+m_nBlockIndex,&lDword);
    m_iTestNumber = lDword;
    break;
  case  TimeStampAttrType:	          /* 23 */
    //secs int seconds
    m_nBlockIndex += clBinaryObject.ReadDword(m_szBlock+m_nBlockIndex,&lDword);
    if(m_bPirWritten.isEmpty())
    {
      m_lStartTime = m_lEndTime;
      m_lStartMicroSeconds = m_lEndMicroSeconds;
    }
    m_lEndTime = lDword;
    //usecs int microseconds
    m_nBlockIndex += clBinaryObject.ReadDword(m_szBlock+m_nBlockIndex,&lDword);
    m_lEndMicroSeconds = lDword;

    if(m_lStartTime == -1)
    {
      m_lStartTime = m_lEndTime;
      m_lStartMicroSeconds = m_lEndMicroSeconds;
    }
    break;
  case  TimingsAttrType:                /* 24 */
    m_mapAssignment["TIME_SET"] = "";
    while((m_nBlockIndex - nIndex) < nLength)
    {
      //short Data
      m_nBlockIndex += clBinaryObject.ReadWord(m_szBlock+m_nBlockIndex,&sWord);
      if(!m_mapAssignment["TIME_SET"].isEmpty())
        m_mapAssignment["TIME_SET"] += ",";
      m_mapAssignment["TIME_SET"] += QString::number(sWord);
    }
    break;
  case  LevelsAttrType:                 /* 25 */
    while((m_nBlockIndex - nIndex) < nLength)
      //short Data
      m_nBlockIndex += clBinaryObject.ReadWord(m_szBlock+m_nBlockIndex,&sWord);
    break;
  case  LabelAttrType:                  /* 26 */
    //label string Label
    m_nBlockIndex += clBinaryObject.ReadString(m_szBlock+m_nBlockIndex,szString);
    m_mapAssignment["VECT_NAM"] = szString;
    break;
  case  PlainDataAttrType:              /* 27 */
    /*
    //data_length int Data length, determines the length of the data field
    m_nBlockIndex += clBinaryObject.ReadDword(m_szBlock+m_nBlockIndex,&lDword);
    for(i=0; i<lDword; i++)
      //data char Data
      m_nBlockIndex += clBinaryObject.ReadByte(m_szBlock+m_nBlockIndex,&bByte);
    */
    i=0;
    szStringBlock = (char *) malloc((nLength+1)*sizeof(char));
    if (!szStringBlock)
    {
      return eConvertError;
    }
    while((m_nBlockIndex - nIndex) < nLength)
    {
      //data char Data
      m_nBlockIndex += clBinaryObject.ReadByte(m_szBlock+m_nBlockIndex,&bByte);
      szStringBlock[i++] = bByte;
    }
    szStringBlock[i++] = '\0';
    if(!m_mapAssignment["RSLT_TXT"].isEmpty())
      strString = m_mapAssignment["RSLT_TXT"] + " ";
    else
      strString = "";
    strString += szStringBlock;

    m_mapAssignment["RSLT_TXT"] = strString;
    break;
  case  UserProcedureNameAttrType:      /* 28 */
    //user_proc string User procedure name
    m_nBlockIndex += clBinaryObject.ReadString(m_szBlock+m_nBlockIndex,szString);
    m_strTestName = szString;
    break;
  case  AppModelLevelAttrType:          /* 29 */
    //app_mod_lev_type char Type of the application model level,
    m_nBlockIndex += clBinaryObject.ReadByte(m_szBlock+m_nBlockIndex,&bByte);
    //level_name string Name of the application model level
    m_nBlockIndex += clBinaryObject.ReadString(m_szBlock+m_nBlockIndex,szString);
    if(QString(szString).toUpper() == "WAFER")
      m_bWaferMap = (bByte == 1);
    break;
  case  TestNameAttrType:               /* 30 */
    //name string Test name
    m_nBlockIndex += clBinaryObject.ReadString(m_szBlock+m_nBlockIndex,szString);
    m_strTestName = szString;
    break;
  case  PassRangeAttrType:              /* 31 */
    CGVERIGY_EDLParameter *pTest;
    // Check if have TestNumber available
    if(!m_qMapParameterList.contains(m_strTestSuiteName+m_strTestName))
    {
      if((m_iTestNumber < 0) || m_bIgnoreTestNumber)
      {
        m_bIgnoreTestNumber = true;
        m_iTestNumber = m_qMapParameterList.count() + 1;
      }
      pTest = new CGVERIGY_EDLParameter();
      pTest->m_nTestNum = m_iTestNumber;
      pTest->m_strName = m_strTestName;
      pTest->m_strSuiteName = m_strTestSuiteName;
      m_qMapParameterList[m_strTestSuiteName+m_strTestName] = pTest;
    }

    pTest = m_qMapParameterList[m_strTestSuiteName+m_strTestName];
    //min_value double Minimum value
    m_nBlockIndex += clBinaryObject.ReadDouble(m_szBlock+m_nBlockIndex,&dDouble);
    pTest->m_fLowLimit = dDouble;
    //max_value double Maximum value
    m_nBlockIndex += clBinaryObject.ReadDouble(m_szBlock+m_nBlockIndex,&dDouble);
    pTest->m_fHighLimit= dDouble;
    //min_relation char
    m_nBlockIndex += clBinaryObject.ReadByte(m_szBlock+m_nBlockIndex,&bByte);
    pTest->m_bValidLowLimit = (bByte != (BYTE)-1);
    pTest->m_bStrictLowLimit = (bByte == 0);
    //max_relation char
    m_nBlockIndex += clBinaryObject.ReadByte(m_szBlock+m_nBlockIndex,&bByte);
    pTest->m_bValidHighLimit = (bByte != (BYTE) -1);
    pTest->m_bStrictHighLimit = (bByte == 0);
    //unit string Unit
    m_nBlockIndex += clBinaryObject.ReadString(m_szBlock+m_nBlockIndex,szString);
    m_strUnit = szString;
    pTest->m_strUnits = PrefixUnitToUnit(m_strUnit);
    pTest->m_fLowLimit /= GS_POW(10.0,PrefixUnitToScall(m_strUnit));
    pTest->m_fHighLimit /= GS_POW(10.0,PrefixUnitToScall(m_strUnit));
    pTest->m_nScale = PrefixUnitToScall(m_strUnit);
    break;
  case  MeasuredRangeAttrType:          /* 32 */
    //min_value double Minimum value
    m_nBlockIndex += clBinaryObject.ReadDouble(m_szBlock+m_nBlockIndex,&dDouble);
    //max_value double Maximum value
    m_nBlockIndex += clBinaryObject.ReadDouble(m_szBlock+m_nBlockIndex,&dDouble);
    //unit string Unit
    m_nBlockIndex += clBinaryObject.ReadString(m_szBlock+m_nBlockIndex,szString);
    m_strUnit = szString;
    break;
  case  ShmooLegendAttrType:            /* 33 */
    //title string Title
    m_nBlockIndex += clBinaryObject.ReadString(m_szBlock+m_nBlockIndex,szString);
    //kind int Shmoo kind
    m_nBlockIndex += clBinaryObject.ReadDword(m_szBlock+m_nBlockIndex,&lDword);
    //shmoo_mode int Shmoo mode
    m_nBlockIndex += clBinaryObject.ReadDword(m_szBlock+m_nBlockIndex,&lDword);
    //test_mode int Shmoo test mode
    m_nBlockIndex += clBinaryObject.ReadDword(m_szBlock+m_nBlockIndex,&lDword);
    for(i=0; i<2; i++)
    {
      //step_n short Number of shmoo cells on this axis
      m_nBlockIndex += clBinaryObject.ReadWord(m_szBlock+m_nBlockIndex,&sWord);
      //axis_n short
      m_nBlockIndex += clBinaryObject.ReadWord(m_szBlock+m_nBlockIndex,&sWord);
      nLoop = sWord;
      for(j=0; j<nLoop; j++)
        //parameter int Parameter
        m_nBlockIndex += clBinaryObject.ReadDword(m_szBlock+m_nBlockIndex,&lDword);
      //device_cycle string Device cycle
      m_nBlockIndex += clBinaryObject.ReadString(m_szBlock+m_nBlockIndex,szString);
      //number_of_pins short Number of pins
      m_nBlockIndex += clBinaryObject.ReadWord(m_szBlock+m_nBlockIndex,&sWord);
      //pin_index short Pin index, occurs as often as defined in number_of_pins
      m_nBlockIndex += clBinaryObject.ReadWord(m_szBlock+m_nBlockIndex,&sWord);
      //unit string Unit
      m_nBlockIndex += clBinaryObject.ReadString(m_szBlock+m_nBlockIndex,szString);
      //formula short Formula
      m_nBlockIndex += clBinaryObject.ReadWord(m_szBlock+m_nBlockIndex,&sWord);
      //slope_start double Slope start
      m_nBlockIndex += clBinaryObject.ReadDouble(m_szBlock+m_nBlockIndex,&dDouble);
      //intersection_stop double Intersection stop
      m_nBlockIndex += clBinaryObject.ReadDouble(m_szBlock+m_nBlockIndex,&dDouble);
    }
    break;
  case  ShmooCellAttrType:              /* 34 */
    //x_coordinate short x coordinate
    m_nBlockIndex += clBinaryObject.ReadWord(m_szBlock+m_nBlockIndex,&sWord);
    //y_coordinate short y coordinate
    m_nBlockIndex += clBinaryObject.ReadWord(m_szBlock+m_nBlockIndex,&sWord);
    //status int Shmoo status
    m_nBlockIndex += clBinaryObject.ReadDword(m_szBlock+m_nBlockIndex,&lDword);
    //test_mode int Shmoo test mode
    m_nBlockIndex += clBinaryObject.ReadDword(m_szBlock+m_nBlockIndex,&lDword);
    if(lDword == DRLShmooTestFFCI)
    {
      //cycle_number double Cycle number
      m_nBlockIndex += clBinaryObject.ReadDouble(m_szBlock+m_nBlockIndex,&dDouble);
      //vector_number int Vector number
      m_nBlockIndex += clBinaryObject.ReadDword(m_szBlock+m_nBlockIndex,&lDword);
      //label string Label
      m_nBlockIndex += clBinaryObject.ReadString(m_szBlock+m_nBlockIndex,szString);
      //number_of_pins short Number of pins
      m_nBlockIndex += clBinaryObject.ReadWord(m_szBlock+m_nBlockIndex,&sWord);
      nLoop = sWord;
      for(i=0; i<nLoop; i++)
      {
        //pin_index short Pin index, occurs as often as defined in number_of_pins
        m_nBlockIndex += clBinaryObject.ReadWord(m_szBlock+m_nBlockIndex,&sWord);
      }
    }
    if(lDword == DRLShmooTestFFCI)
    {
      //error_count short Error count
      m_nBlockIndex += clBinaryObject.ReadWord(m_szBlock+m_nBlockIndex,&sWord);
    }
    break;
  case  LinearWaveformAttrType:         /* 35 */
    //pin_count short Pin count
    m_nBlockIndex += clBinaryObject.ReadWord(m_szBlock+m_nBlockIndex,&sWord);
    nLoop = sWord;
    for(i=0; i<nLoop; i++)
      //pin_index short Pin index This field occurs as often as defined in the pin_count field.
      m_nBlockIndex += clBinaryObject.ReadWord(m_szBlock+m_nBlockIndex,&sWord);
    //vector_variable string Vector variable name, empty string if no vector variable is defined
    m_nBlockIndex += clBinaryObject.ReadString(m_szBlock+m_nBlockIndex,szString);
    //description string Description/Title
    m_nBlockIndex += clBinaryObject.ReadString(m_szBlock+m_nBlockIndex,szString);
    //x_parameter string X axis parameter
    m_nBlockIndex += clBinaryObject.ReadString(m_szBlock+m_nBlockIndex,szString);
    //x_unit string X axis unit
    m_nBlockIndex += clBinaryObject.ReadString(m_szBlock+m_nBlockIndex,szString);
    //y_parameter string Y axis parameter
    m_nBlockIndex += clBinaryObject.ReadString(m_szBlock+m_nBlockIndex,szString);
    //y_unit string Y axis unit
    m_nBlockIndex += clBinaryObject.ReadString(m_szBlock+m_nBlockIndex,szString);
    //start double Start value
    m_nBlockIndex += clBinaryObject.ReadDouble(m_szBlock+m_nBlockIndex,&dDouble);
    //end double End value
    m_nBlockIndex += clBinaryObject.ReadDouble(m_szBlock+m_nBlockIndex,&dDouble);
    //step_count int Step count
    m_nBlockIndex += clBinaryObject.ReadDword(m_szBlock+m_nBlockIndex,&lDword);
    //samples double Values
    m_nBlockIndex += clBinaryObject.ReadDouble(m_szBlock+m_nBlockIndex,&dDouble);
    break;
  case  CycleAttrType:                  /* 36 */
    //cycle_number double Cycle number
    m_nBlockIndex += clBinaryObject.ReadDouble(m_szBlock+m_nBlockIndex,&dDouble);
    break;
  case  VectorAttrType:		          /* 37 */
    //vector_number int Vector number
    m_nBlockIndex += clBinaryObject.ReadDword(m_szBlock+m_nBlockIndex,&lDword);
    break;
  case  ExpressionAttrType:			  /* 38 */
    //expression string Expression
    m_nBlockIndex += clBinaryObject.ReadString(m_szBlock+m_nBlockIndex,szString);
    m_mapAssignment["DATALOG"] = szString;
    //value double Value
    if((m_nBlockIndex - nIndex) < nLength)
    {
      m_nBlockIndex += clBinaryObject.ReadDouble(m_szBlock+m_nBlockIndex,&dDouble);
      m_mapAssignment["DATALOG"] = QString::number(dDouble);
    }
    break;
  case  PinGroupListAttrType:			  /* 39 */
    while((m_nBlockIndex - nIndex) < nLength)
    {
      //name string Name of the pin group
      m_nBlockIndex += clBinaryObject.ReadString(m_szBlock+m_nBlockIndex,szString);
      //pin_count int Number of pins contained in the pin group
      m_nBlockIndex += clBinaryObject.ReadDword(m_szBlock+m_nBlockIndex,&lDword);
      for(i=0; i< lDword; i++)
        //pin_index short Index of the pin, occurs pin_count times
        m_nBlockIndex += clBinaryObject.ReadWord(m_szBlock+m_nBlockIndex,&sWord);
    }
    break;
  case  PortDescAttrType:               /* 40 */
    //count unsigned int Number of ports
    m_nBlockIndex += clBinaryObject.ReadDword(m_szBlock+m_nBlockIndex,&lDword);
    nLoop = lDword;
    for(i=0; i<nLoop; i++)
    {
      //name string Port name
      m_nBlockIndex += clBinaryObject.ReadString(m_szBlock+m_nBlockIndex,szString);
      //pin_count unsigned int Number of pins related to the port
      m_nBlockIndex += clBinaryObject.ReadDword(m_szBlock+m_nBlockIndex,&lDword);
      for(j=0; j<lDword; j++)
        //pin_index short Index of the pin, occurs pin_count times
        m_nBlockIndex += clBinaryObject.ReadWord(m_szBlock+m_nBlockIndex,&sWord);
    }
    break;
  case  VectorFineResultAttrType:       /* 41 */
    //number_of_pins int Number of pins
    m_nBlockIndex += clBinaryObject.ReadDword(m_szBlock+m_nBlockIndex,&lDword);
    nLoop = lDword;
    //cycle_number double Cycle number
    m_nBlockIndex += clBinaryObject.ReadDouble(m_szBlock+m_nBlockIndex,&dDouble);
    //resolution_mode int Resolution mode of fine result granularity, values: "CYCL" or "EDGE"
    m_nBlockIndex += clBinaryObject.ReadDword(m_szBlock+m_nBlockIndex,&lDword);
    //num_of_waveforms int Number of waveforms, with the device cycle expander one vector memory entry (Device Cycle) expands to up to ten waveforms
    m_nBlockIndex += clBinaryObject.ReadDword(m_szBlock+m_nBlockIndex,&lDword);
    for(i=0; i<nLoop; i++)
    {
      //pin_index short Pin index This field and the following four fields form a block, which occurs as often as defined by the number_of_pins field.
      m_nBlockIndex += clBinaryObject.ReadWord(m_szBlock+m_nBlockIndex,&sWord);
      //recv_event_size char Receive event size
      m_nBlockIndex += clBinaryObject.ReadByte(m_szBlock+m_nBlockIndex,&bByte);
      for(j=0; j<sWord; j++)
      {
        //wave_index char Waveform number (0..9) within device cycle table
        m_nBlockIndex += clBinaryObject.ReadByte(m_szBlock+m_nBlockIndex,&bByte);
        //event_index char Receive event index (0..63)
        m_nBlockIndex += clBinaryObject.ReadByte(m_szBlock+m_nBlockIndex,&bByte);
        //expected_received char Compare method
        m_nBlockIndex += clBinaryObject.ReadByte(m_szBlock+m_nBlockIndex,&bByte);
      }
    }
    break;
  case  PortAttrType:                   /* 42 */
    //port_index short Port index
    m_nBlockIndex += clBinaryObject.ReadWord(m_szBlock+m_nBlockIndex,&sWord);
    break;
  case  PortTimingsAttrType:            /* 43 */
    //description string Port description
    m_nBlockIndex += clBinaryObject.ReadString(m_szBlock+m_nBlockIndex,szString);
    //port_string_present short Displays, whether or not port_string is available
    m_nBlockIndex += clBinaryObject.ReadWord(m_szBlock+m_nBlockIndex,&sWord);
    //port_string string Port string, is present only if port_string_present = 1. optional
    if(sWord == 1)
      m_nBlockIndex += clBinaryObject.ReadString(m_szBlock+m_nBlockIndex,szString);
    //timing_set int Timing set
    m_nBlockIndex += clBinaryObject.ReadDword(m_szBlock+m_nBlockIndex,&lDword);
    break;
  case  PortPFResultAttrType:           /* 44 */
    //port_index short Port index
    m_nBlockIndex += clBinaryObject.ReadWord(m_szBlock+m_nBlockIndex,&sWord);
    //failure_code char Failure code, values as defined in dcl_drl.h:
    m_nBlockIndex += clBinaryObject.ReadByte(m_szBlock+m_nBlockIndex,&bByte);
    //• -1= DRLFailureCode_NA: no pass/fail result determined
    //• 0 = DRLFailureCode_Pass: passed
    //• 1 = DRLFailureCode_Fail: failed
    //• 2 = DRLFailureCode_Error: error
    //• 3 = DRLFailureCode_Bypass: bypassed, only applicable for testsuites
    break;
  case  XYDataResultAttrType:           /* 45 */
    //description string Description of the measurement, for example jitter test
    m_nBlockIndex += clBinaryObject.ReadString(m_szBlock+m_nBlockIndex,szString);
    //x_description string Description
    m_nBlockIndex += clBinaryObject.ReadString(m_szBlock+m_nBlockIndex,szString);
    //x_unit string Unit
    m_nBlockIndex += clBinaryObject.ReadString(m_szBlock+m_nBlockIndex,szString);
    //y_description string Description
    m_nBlockIndex += clBinaryObject.ReadString(m_szBlock+m_nBlockIndex,szString);
    //y_unit string Unit
    m_nBlockIndex += clBinaryObject.ReadString(m_szBlock+m_nBlockIndex,szString);
    //num_of_points short Number of points
    m_nBlockIndex += clBinaryObject.ReadWord(m_szBlock+m_nBlockIndex,&sWord);
    nLoop = sWord;
    //num_of_pin_meas short Number of pins on which the measurement was done
    j = sWord;
    m_nBlockIndex += clBinaryObject.ReadWord(m_szBlock+m_nBlockIndex,&sWord);
    for(i=0; i<nLoop; i++)
    {
      //x_value double Value
      m_nBlockIndex += clBinaryObject.ReadDouble(m_szBlock+m_nBlockIndex,&dDouble);
      //y_value double Value
      m_nBlockIndex += clBinaryObject.ReadDouble(m_szBlock+m_nBlockIndex,&dDouble);
    }
    for(i=0; i<j; i++)
    {
      //point_index short Point index
      m_nBlockIndex += clBinaryObject.ReadWord(m_szBlock+m_nBlockIndex,&sWord);
      //pin_index short Pin index
      m_nBlockIndex += clBinaryObject.ReadWord(m_szBlock+m_nBlockIndex,&sWord);
      //failure_code char Failure Code
      m_nBlockIndex += clBinaryObject.ReadByte(m_szBlock+m_nBlockIndex,&bByte);
    }
    break;
  case  TagAttrType:                    /* 46 */
    //name string Tag name
    m_nBlockIndex += clBinaryObject.ReadString(m_szBlock+m_nBlockIndex,szString);
    //value string Tag value
    m_nBlockIndex += clBinaryObject.ReadString(m_szBlock+m_nBlockIndex,szString);
    break;
  case  TestsuiteListAttrType:          /* 47 */
    //count int Number of testsuites, determines the number of all following four fields
    m_nBlockIndex += clBinaryObject.ReadDword(m_szBlock+m_nBlockIndex,&lDword);
    nLoop = lDword;
    for(i=0 ; i<nLoop; i++)
    {
      //testsuite_name string Name of the test suite
      m_nBlockIndex += clBinaryObject.ReadString(m_szBlock+m_nBlockIndex,szString);
      //test_type string Testsuite type
      m_nBlockIndex += clBinaryObject.ReadString(m_szBlock+m_nBlockIndex,szString);
      //description string Testsuite description
      m_nBlockIndex += clBinaryObject.ReadString(m_szBlock+m_nBlockIndex,szString);
      //test_number int Number as entered in the test number field of the testsuite dialog
      m_nBlockIndex += clBinaryObject.ReadDword(m_szBlock+m_nBlockIndex,&lDword);
    }
    break;
  case  BinListAttrType:                /* 48 */
    //count int Number of bins, determines the number of all following 6 fields
    m_nBlockIndex += clBinaryObject.ReadDword(m_szBlock+m_nBlockIndex,&lDword);
    nLoop = lDword;
    for(i=0 ; i<nLoop; i++)
    {
      CGVERIGY_EDLBinInfo *pBin = new CGVERIGY_EDLBinInfo();
      //name string Bin name
      m_nBlockIndex += clBinaryObject.ReadString(m_szBlock+m_nBlockIndex,szString);
      strSBinNum = szString;
      nIndex = strSBinNum.toInt(&bIsNum);
      //description string Bin description
      m_nBlockIndex += clBinaryObject.ReadString(m_szBlock+m_nBlockIndex,szString);
      pBin->strBinName = szString;
      //ext_data string always empty string, reserved for future use
      m_nBlockIndex += clBinaryObject.ReadString(m_szBlock+m_nBlockIndex,szString);
      //hardbin int
      m_nBlockIndex += clBinaryObject.ReadDword(m_szBlock+m_nBlockIndex,&lDword);
      pBin->nHardBin = lDword;
      //is_good_bin char One out of the following values:
      m_nBlockIndex += clBinaryObject.ReadByte(m_szBlock+m_nBlockIndex,&bByte);
      pBin->bPass = (bByte == 1);
      // reprobe_bin char One out of the following values:
      m_nBlockIndex += clBinaryObject.ReadByte(m_szBlock+m_nBlockIndex,&bByte);

      if(pBin->nHardBin < 0)
      {
        // invalid result
        delete pBin;
      }
      else
      {

        if(!bIsNum)
        {
          if(!m_lstSoftBinIssue.contains(strSBinNum))
            m_lstSoftBinIssue.append(strSBinNum);
          nIndex = m_lstSoftBinIssue.indexOf(strSBinNum);
        }
        if(m_qMapBins_Soft.contains(nIndex))
        {
          // already into the list
          delete pBin;
        }
        else
          m_qMapBins_Soft[nIndex] = pBin;
      }

    }
    break;
  case  SpecValueAttrType:              /* 49 */
    //type char Specification type, level or timing specification, values as defined in dcl_drl.h:
    m_nBlockIndex += clBinaryObject.ReadByte(m_szBlock+m_nBlockIndex,&bByte);
    //• 1 = DRLTimingEquations
    //• 2 = DRLLevelEquations
    //eq_set short Equation set number
    m_nBlockIndex += clBinaryObject.ReadWord(m_szBlock+m_nBlockIndex,&sWord);
    //spec_set short Spec set number
    m_nBlockIndex += clBinaryObject.ReadWord(m_szBlock+m_nBlockIndex,&sWord);
    //specification string Multi-port specification name
    m_nBlockIndex += clBinaryObject.ReadString(m_szBlock+m_nBlockIndex,szString);
    //spec_name string Spec variable name
    m_nBlockIndex += clBinaryObject.ReadString(m_szBlock+m_nBlockIndex,szString);
    //value double Value to which the spec variable of eq_set/spec_set of specification is set.
    m_nBlockIndex += clBinaryObject.ReadDouble(m_szBlock+m_nBlockIndex,&dDouble);
    break;
  /*CR Number: CR15791*/
  case  GangAttrType:                   /* 50 */
    //result string Value is always "1".
    m_nBlockIndex += clBinaryObject.ReadString(m_szBlock+m_nBlockIndex,szString);
    break;
  case  VectorResultLongNameAttrType:	  /* 51 */
    //cycle_number double Cycle number
    m_nBlockIndex += clBinaryObject.ReadDouble(m_szBlock+m_nBlockIndex,&dDouble);
    //vector_number long Vector number
    m_nBlockIndex += clBinaryObject.ReadDword(m_szBlock+m_nBlockIndex,&lDword);
    //sequencer_cmd char Sequencer command
    m_nBlockIndex += clBinaryObject.ReadByte(m_szBlock+m_nBlockIndex,&bByte);
    //label_type char Label type
    m_nBlockIndex += clBinaryObject.ReadByte(m_szBlock+m_nBlockIndex,&bByte);
    //vector_comment string Vector comment
    m_nBlockIndex += clBinaryObject.ReadString(m_szBlock+m_nBlockIndex,szString);
    //label_name string Label name
    m_nBlockIndex += clBinaryObject.ReadString(m_szBlock+m_nBlockIndex,szString);
    while((m_nBlockIndex - nIndex) < nLength)
    {
      //pin_index short Pin index
      m_nBlockIndex += clBinaryObject.ReadWord(m_szBlock+m_nBlockIndex,&sWord);
      for(i=0; i<17; i++)
      {
        //device_cycle_name char Device cycle name , the number of characters is depending on the attribute type:
        m_nBlockIndex += clBinaryObject.ReadByte(m_szBlock+m_nBlockIndex,&bByte);
        szString[i] = bByte;
      }
      szString[i] = '\0';
      //• VectorResultAttrType: 7 characters
      //• VectorResultLongNameAttrType: 17 characters
      //device_cycle_offset char Device cycle offset
      m_nBlockIndex += clBinaryObject.ReadByte(m_szBlock+m_nBlockIndex,&bByte);
      //d1 char Drive parameter 1 of the device cycle
      m_nBlockIndex += clBinaryObject.ReadByte(m_szBlock+m_nBlockIndex,&bByte);
      //d2 char Drive parameter 2 of the device cycle
      m_nBlockIndex += clBinaryObject.ReadByte(m_szBlock+m_nBlockIndex,&bByte);
      //e1 char Receive parameter 1 of the device cycle
      m_nBlockIndex += clBinaryObject.ReadByte(m_szBlock+m_nBlockIndex,&bByte);
      //e2 char Receive parameter 2 of the device cycle
      m_nBlockIndex += clBinaryObject.ReadByte(m_szBlock+m_nBlockIndex,&bByte);
      //received char Received data , the bits have the follwing meaning: ppeeerrr, p = parameter, e = edge, r = received
      m_nBlockIndex += clBinaryObject.ReadByte(m_szBlock+m_nBlockIndex,&bByte);
    }
    break;
  case  GenericDataFieldAttrType:		  /* 52 */
    //data_type unsigned char Data type according to STDF specification
    m_nBlockIndex += clBinaryObject.ReadByte(m_szBlock+m_nBlockIndex,&bByte);
    //data_length unsigned short
    m_nBlockIndex += clBinaryObject.ReadWord(m_szBlock+m_nBlockIndex,&sWord);
    nLoop = sWord;
    strString = "";
    // number, have to switch BigIndian (Unix - PC)
    if(bByte<9)
    {
      //number
      m_lstGenericData.append(QString::number(bByte));
      m_lstGenericData.append(QString::number(nLoop));
      if(clBinaryObject.bSameCPUType)
      {
        for(i=0 ; i<nLoop; i++)
        {
          //data char Data
          m_nBlockIndex += clBinaryObject.ReadByte(m_szBlock+m_nBlockIndex,&bByte);
          strString[i] = bByte;
        }
      }
      else
      {
        for(i=nLoop ; i>0; i--)
        {
          //data char Data
          m_nBlockIndex += clBinaryObject.ReadByte(m_szBlock+m_nBlockIndex,&bByte);
          strString[i-1] = bByte;
        }
      }
      m_lstGenericData.append(strString);
    }
    else
    {
      int nType = bByte;
      while(nLoop > 0)
      {
        if(nLoop > 255)
          sWord = 255;
        else
          sWord = nLoop;
        strString = "";
        for(i=0 ; i<sWord; i++)
        {
          //data char Data
          m_nBlockIndex += clBinaryObject.ReadByte(m_szBlock+m_nBlockIndex,&bByte);
          strString[i] = bByte;
        }
        m_lstGenericData.append(QString::number(nType));
        m_lstGenericData.append(QString::number(sWord));
        m_lstGenericData.append(strString);
        nLoop -= sWord;
      }
    }
    break;
  case  LotTypeAttrType:                /* 53 */
    //lot_type string Lot type, values: "PACKAGE" or "WAFER"
    m_nBlockIndex += clBinaryObject.ReadString(m_szBlock+m_nBlockIndex,szString);
    m_bWaferMap = QString(szString).startsWith("WAFER", Qt::CaseInsensitive);
    break;
  case  SWVersionAttrType:              /* 54 */
    //version string Software build version and build date in the format: <revision>,<date> where <date> is in a <dd-mmm-yy> format
    m_nBlockIndex += clBinaryObject.ReadString(m_szBlock+m_nBlockIndex,szString);
    m_mapAssignment["EXEC_VER"] = szString;
    break;
  case  TesterProductAttrType:          /* 55 */
    //tester_id string Tester identifier, constant value is "93000".
    m_nBlockIndex += clBinaryObject.ReadString(m_szBlock+m_nBlockIndex,szString);
    m_mapAssignment["TSTR_TYP"] = szString;
    break;
  case  TestHeadNumberAttrType:         /* 56 */
    //test_head_number string Test head number
    m_nBlockIndex += clBinaryObject.ReadString(m_szBlock+m_nBlockIndex,szString);
    m_mapAssignment["HEAD_NUM"] = szString;
    break;
  case  BypassStateAttrType:            /* 57 */
    //bypass_state string
    m_nBlockIndex += clBinaryObject.ReadString(m_szBlock+m_nBlockIndex,szString);
    break;
  case  ManualExecutionStateAttrType:   /* 58 */
    //execution_state string Execution state, values: "Manual" or "Auto".
    m_nBlockIndex += clBinaryObject.ReadString(m_szBlock+m_nBlockIndex,szString);
    break;
  case  PerPinOutputOnPassAttrType:     /* 59 */
    //per_pin_on_pass string State for the "Per Pin Output On Pass" flag of the particular testsuite, values: "Enable" or "Disable"
    m_nBlockIndex += clBinaryObject.ReadString(m_szBlock+m_nBlockIndex,szString);
    break;
  case  PerPinOutputOnFailAttrType:     /* 60 */
    //per_pin_on_fail string State for the "Per Pin Output On Fail" flag of the particular testsuite, values: "Enable" or "Disable"
    m_nBlockIndex += clBinaryObject.ReadString(m_szBlock+m_nBlockIndex,szString);
    break;
  case  TestDescriptionAttrType:        /* 61 */
    //test_description string Name/description of the test
    m_nBlockIndex += clBinaryObject.ReadString(m_szBlock+m_nBlockIndex,szString);
    break;
  case  ParametricOrFunctionalTestAttrType:        /* 62 */
    //parametric_or_functional char Test type
    m_nBlockIndex += clBinaryObject.ReadByte(m_szBlock+m_nBlockIndex,&bByte);
    //0 TEST_TYPE_UNDEFINED
    //1 TEST_TYPE_FUNCTIONAL
    //2 TEST_TYPE_PARAMETRIC
    //3 TEST_TYPE_MULTIPLE_PARAMETRIC
    //4 TEST_TYPE_PARAMETRIC_FUNCTIONAL
    //5 TEST_TYPE_ANALOG
    //6 TEST_TYPE_FLEX_DC
    m_nTestType = bByte;
    break;
  case  OutputOnPassAttrType:           /* 63 */
    //output_on_pass string State for the "Output On Pass" flag of the particular testsuite, values: "Enable" or "Disable"
    m_nBlockIndex += clBinaryObject.ReadString(m_szBlock+m_nBlockIndex,szString);
    break;
  case  OutputOnFailAttrType:           /* 64 */
    //output_on_fail string State for the "Output On Fail" flag of the particular testsuite, values: "Enable" or "Disable"
    m_nBlockIndex += clBinaryObject.ReadString(m_szBlock+m_nBlockIndex,szString);
    break;
  default:
    m_nBlockIndex += nLength;
    break;
  }

  if((m_nBlockIndex - nIndex) < nLength)
    // ?????????????????????????
    m_nBlockIndex = nIndex + nLength;
  return eConvertSuccess;
}

//////////////////////////////////////////////////////////////////////
// Create STDF file from VERIGY_EDL data parsed
//////////////////////////////////////////////////////////////////////
int CGVERIGY_EDLtoSTDF::WriteMir()
{
  if(m_bMirWritten)
    return eConvertSuccess;

  QString strString;
  int	i;
  int	iIndex;

  // now generate the STDF file...
  GS::StdLib::StdfRecordReadInfo RecordReadInfo;
  if(m_StdfFile.Open(m_strStdfFileName.toLatin1().constData(),STDF_WRITE,STDF_READ_CACHE) != GS::StdLib::Stdf::NoError)
  {
    // Failed importing VERIGY_EDL file into STDF database
    m_iLastError = errWriteSTDF;

    // Delay convertion.
    return eConvertDelay;
  }

  // Write FAR
  RecordReadInfo.iRecordType = 0;
  RecordReadInfo.iRecordSubType = 10;
  m_StdfFile.WriteHeader(&RecordReadInfo);
  m_StdfFile.WriteByte(1);					// SUN CPU type
  m_StdfFile.WriteByte(4);					// STDF V4
  m_StdfFile.WriteRecord();

  // Write ATR
  strString = "gex";
  strString += " (VerigyEdl converter)";
  RecordReadInfo.iRecordType = 0;
  RecordReadInfo.iRecordSubType = 20;
  m_StdfFile.WriteHeader(&RecordReadInfo);
  m_StdfFile.WriteDword(QDateTime::currentDateTime().toTime_t());	// MOD_TIM
  m_StdfFile.WriteString(strString.toLatin1().constData());				// CMD_LINE
  m_StdfFile.WriteRecord();

  // Check if have good retest bin
  QStringList lstRetestBins;
  if(m_mapAssignment.contains("RTST_BIN"))
    lstRetestBins = m_mapAssignment["RTST_BIN"].split(",");
  if(!lstRetestBins.isEmpty())
  {
    bool bGoodBin = true;
    for(i=0; i<lstRetestBins.count(); i++)
    {
      lstRetestBins[i].toInt(&bGoodBin);
      if(!bGoodBin)
        break;
    }
    if(!bGoodBin)
      lstRetestBins.clear();

  }

  if((m_mapAssignment["RTST_COD"] == " ")
  && (!lstRetestBins.isEmpty()))
    m_mapAssignment["RTST_COD"] = "Y";
  if(m_mapAssignment["RTST_COD"] == " ")
    m_mapAssignment["RTST_COD"] = "N";

  if(m_lStartTime <= 0)
    m_lStartTime = QDateTime::currentDateTime().toTime_t();

  // Write MIR
  RecordReadInfo.iRecordType = 1;
  RecordReadInfo.iRecordSubType = 10;
  m_StdfFile.WriteHeader(&RecordReadInfo);
  m_StdfFile.WriteDword(m_lStartTime);			// Setup time
  m_StdfFile.WriteDword(m_lStartTime);			// Start time
  m_StdfFile.WriteByte(m_mapAssignment["STAT_NUM"].toInt());	// Station
  m_StdfFile.WriteByte((BYTE) m_mapAssignment["MODE_COD"][0].toLatin1());			// Test Mode = PRODUCTION
  m_StdfFile.WriteByte((BYTE) m_mapAssignment["RTST_COD"][0].toLatin1());			// rtst_cod
  m_StdfFile.WriteByte((BYTE) m_mapAssignment["PROT_COD"][0].toLatin1());			// prot_cod
  m_StdfFile.WriteWord((short)m_mapAssignment["BURN_TIM"].toInt());				// burn_tim
  m_StdfFile.WriteByte((BYTE) m_mapAssignment["CMOD_COD"][0].toLatin1());			// cmod_cod
  m_StdfFile.WriteString(m_mapAssignment["LOT_ID"].toLatin1().constData());		// Lot ID
  m_StdfFile.WriteString(m_mapAssignment["PART_TYP"].toLatin1().constData());	// Part Type / Product ID
  m_StdfFile.WriteString(m_mapAssignment["NODE_NAM"].toLatin1().constData());	// Node name
  m_StdfFile.WriteString(m_mapAssignment["TSTR_TYP"].toLatin1().constData());	// Tester Type
  m_StdfFile.WriteString(m_mapAssignment["JOB_NAM"].toLatin1().constData());	// Job name
  m_StdfFile.WriteString(m_mapAssignment["JOB_REV"].toLatin1().constData());	// Job rev
  m_StdfFile.WriteString(m_mapAssignment["SBLOT_ID"].toLatin1().constData());	// sublot-id
  m_StdfFile.WriteString(m_mapAssignment["OPER_NAM"].toLatin1().constData());	// operator
  m_StdfFile.WriteString(m_mapAssignment["EXEC_TYP"].toLatin1().constData());	// exec-type
  m_StdfFile.WriteString(m_mapAssignment["EXEC_VER"].toLatin1().constData());	// exe-ver
  m_StdfFile.WriteString(m_mapAssignment["TEST_COD"].toLatin1().constData());	// test-cod
  m_StdfFile.WriteString(m_mapAssignment["TST_TEMP"].toLatin1().constData());	// test-temperature
  // Construct custom Galaxy USER_TXT
  QString	strUserTxt;
  strUserTxt = GEX_IMPORT_DATAORIGIN_LABEL;
  strUserTxt += ":";
  strUserTxt += GEX_IMPORT_DATAORIGIN_ATETEST;
  strUserTxt += ":VERIGY_EDL";
  m_StdfFile.WriteString(strUserTxt.toLatin1().constData());					// user-txt
  m_StdfFile.WriteString(m_mapAssignment["AUX_FILE"].toLatin1().constData());	// aux-file
  m_StdfFile.WriteString(m_mapAssignment["PKG_TYP"].toLatin1().constData());	// package-type
  m_StdfFile.WriteString(m_mapAssignment["FAMLY_ID"].toLatin1().constData());	// familyID
  m_StdfFile.WriteString(m_mapAssignment["DATE_COD"].toLatin1().constData());	// Date-code
  m_StdfFile.WriteString(m_mapAssignment["FACIL_ID"].toLatin1().constData());	// Facility-ID
  m_StdfFile.WriteString(m_mapAssignment["FLOOR_ID"].toLatin1().constData());	// FloorID
  m_StdfFile.WriteString(m_mapAssignment["PROC_ID"].toLatin1().constData());	// ProcessID
  m_StdfFile.WriteString(m_mapAssignment["OPER_FRQ"].toLatin1().constData());	// Oper-Frq
  m_StdfFile.WriteString(m_mapAssignment["SPEC_NAM"].toLatin1().constData());	// Spec-Name
  m_StdfFile.WriteString(m_mapAssignment["SPEC_VER"].toLatin1().constData());	// Spec-Ver
  m_StdfFile.WriteString(m_mapAssignment["FLOW_ID"].toLatin1().constData());	// Flow-Id
  m_StdfFile.WriteString(m_mapAssignment["SETUP_ID"].toLatin1().constData());	// Setup-Id
  m_StdfFile.WriteString(m_mapAssignment["DSGN_REV"].toLatin1().constData());	// Design-Ver
  m_StdfFile.WriteString(m_mapAssignment["ENG_ID"].toLatin1().constData());		// Eng-Id
  m_StdfFile.WriteString(m_mapAssignment["ROM_COD"].toLatin1().constData());	// Rom-Code
  m_StdfFile.WriteString(m_mapAssignment["SERL_NUM"].toLatin1().constData());	// Serl-Num
  m_StdfFile.WriteString(m_mapAssignment["SUPR_NAM"].toLatin1().constData());	// Supr-Name
  m_StdfFile.WriteRecord();

  // In Retest mod
  if(!lstRetestBins.isEmpty())
  {
    // Write RDR
    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 70;
    m_StdfFile.WriteHeader(&RecordReadInfo);

    m_StdfFile.WriteWord(lstRetestBins.count());
    for(i=0; i<lstRetestBins.count(); i++)
      m_StdfFile.WriteWord(lstRetestBins[i].toInt());
    m_StdfFile.WriteRecord();
  }


  // Write PinMap
  if(!m_qMapPins.isEmpty())
  {
    // Write PMR
    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 60;
    iIndex = 0;

    QMap<int,CGVERIGY_EDLPinInfo*>::Iterator itMapPin;
    QMap<int,int>::Iterator itSiteChannel;
    for ( itMapPin = m_qMapPins.begin(); itMapPin != m_qMapPins.end(); ++itMapPin )
    {
      for(itSiteChannel = itMapPin.value()->mapSiteChannel.begin(); itSiteChannel != itMapPin.value()->mapSiteChannel.end(); ++itSiteChannel)
      {
        if(m_lstSites.count() <= itSiteChannel.key())
          continue;

        m_StdfFile.WriteHeader(&RecordReadInfo);
        i = itMapPin.value()->mapSiteIndex[itSiteChannel.key()];
        m_StdfFile.WriteWord(i);											// PMR_INDX
        m_StdfFile.WriteWord(itMapPin.key());								// CHAN_TYP
        m_StdfFile.WriteString(itMapPin.value()->strName.toLatin1().constData());	// CHAN_NAM
        m_StdfFile.WriteString(QString::number(itSiteChannel.value()).toLatin1().constData());	// PHY_NAM
        m_StdfFile.WriteString(itMapPin.value()->strNumber.toLatin1().constData());	// LOG_NAM
        m_StdfFile.WriteByte(m_mapAssignment["HEAD_NUM"].toInt());			// HEAD_NUM
        m_StdfFile.WriteByte(m_lstSites[itSiteChannel.key()].toInt());		// SITE_NUM
        m_StdfFile.WriteRecord();
      }
      iIndex++;
    }

  }

  if(!m_lstSites.isEmpty())
  {
    // Write SDR
    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 80;
    m_StdfFile.WriteHeader(&RecordReadInfo);
    m_StdfFile.WriteByte((BYTE)1);			// head#
    m_StdfFile.WriteByte((BYTE)1);			// Group#
    m_StdfFile.WriteByte((BYTE)m_lstSites.count());			// site_count
    for(int i=0; i<m_lstSites.count(); i++)
      m_StdfFile.WriteByte((BYTE)m_lstSites[i].toInt());	// array of test site#
    m_StdfFile.WriteString(m_mapAssignment["HAND_TYP"].toLatin1().constData());	// HAND_TYP: Handler/prober type
    m_StdfFile.WriteString(m_mapAssignment["HAND_ID"].toLatin1().constData());	// HAND_ID: Handler/prober name
    m_StdfFile.WriteString(m_mapAssignment["CARD_TYP"].toLatin1().constData());	// CARD_TYP: Probe card type
    m_StdfFile.WriteString(m_mapAssignment["CARD_ID"].toLatin1().constData());	// CARD_ID: Probe card name
    m_StdfFile.WriteString(m_mapAssignment["LOAD_TYP"].toLatin1().constData());	// LOAD_TYP: Load board type
    m_StdfFile.WriteString(m_mapAssignment["LOAD_ID"].toLatin1().constData());	// LOAD_ID: Load board name
    m_StdfFile.WriteString(m_mapAssignment["DIB_TYP"].toLatin1().constData());	// DIB_TYP: DIB board type
    m_StdfFile.WriteString(m_mapAssignment["DIB_ID"].toLatin1().constData());		// DIB_ID: DIB board name
    m_StdfFile.WriteString(m_mapAssignment["CABL_TYP"].toLatin1().constData());	// CABL_TYP: Interface cable type
    m_StdfFile.WriteString(m_mapAssignment["CABL_ID"].toLatin1().constData());	// CABL_ID: Interface cable name
    m_StdfFile.WriteString(m_mapAssignment["CONT_TYP"].toLatin1().constData());	// CONT_TYP: Handler contactor type
    m_StdfFile.WriteString(m_mapAssignment["CONT_ID"].toLatin1().constData());	// CONT_ID: Handler contactor name
    m_StdfFile.WriteString(m_mapAssignment["LASR_TYP"].toLatin1().constData());	// LASR_TYP: Laser type
    m_StdfFile.WriteString(m_mapAssignment["LASR_ID"].toLatin1().constData());	// LASR_ID: Laser name
    m_StdfFile.WriteString(m_mapAssignment["EXTR_TYP"].toLatin1().constData());	// EXTR_TYP: Extra equipment type
    m_StdfFile.WriteString(m_mapAssignment["EXTR_ID"].toLatin1().constData());	// EXTR_ID: Extra equipment name
    m_StdfFile.WriteRecord();
  }

  m_bMirWritten = true;

  // Success
  return eConvertSuccess;
}


//////////////////////////////////////////////////////////////////////
// Create STDF file from VERIGY_EDL data parsed
//////////////////////////////////////////////////////////////////////
int CGVERIGY_EDLtoSTDF::WriteMrr()
{
  if(!m_bMirWritten)
    return eConvertError;

  QString strString;
  int i;
  QMap<int,int> mapPcrGood;
  QMap<int,int> mapPcrBad;

  mapPcrGood[-1]	= 0;
  mapPcrBad[-1]	= 0;

  // now generate the STDF file...
  GS::StdLib::StdfRecordReadInfo RecordReadInfo;

  RecordReadInfo.iRecordType = 1;
  RecordReadInfo.iRecordSubType = 40;
  QMap<int,CGVERIGY_EDLBinInfo*>::Iterator itMapBin;
  for ( itMapBin = m_qMapBins.begin(); itMapBin != m_qMapBins.end(); ++itMapBin )
  {
    // Write HBR
    m_StdfFile.WriteHeader(&RecordReadInfo);
    m_StdfFile.WriteByte(255);								// Test Head = ALL
    m_StdfFile.WriteByte(255);								// Test sites = ALL
    m_StdfFile.WriteWord(itMapBin.key());					// HBIN = 0
    m_StdfFile.WriteDword(itMapBin.value()->GetCount(-1));	// Total Bins
    if(itMapBin.value()->bPass)
    {
      m_StdfFile.WriteByte('P');
      mapPcrGood[-1] += itMapBin.value()->GetCount(-1);
    }
    else
    {
      m_StdfFile.WriteByte('F');
      mapPcrBad[-1] += itMapBin.value()->GetCount(-1);
    }
    m_StdfFile.WriteString((itMapBin.value()->strBinName.toLatin1().constData()));
    m_StdfFile.WriteRecord();

    for(i=0; i<m_lstSites.count(); i++)
    {
      if(!mapPcrGood.contains(m_lstSites[i].toInt()))
      {
        mapPcrGood[m_lstSites[i].toInt()]	= 0;
        mapPcrBad[m_lstSites[i].toInt()]	= 0;
      }
      // Write HBR
      m_StdfFile.WriteHeader(&RecordReadInfo);
      m_StdfFile.WriteByte(m_mapAssignment["HEAD_NUM"].toInt());// Test Head
      m_StdfFile.WriteByte(m_lstSites[i].toInt());			// Test sites
      m_StdfFile.WriteWord(itMapBin.key());					// HBIN = 0
      m_StdfFile.WriteDword(itMapBin.value()->GetCount(m_lstSites[i].toInt()));// Total Bins
      if(itMapBin.value()->bPass)
      {
        m_StdfFile.WriteByte('P');
        mapPcrGood[m_lstSites[i].toInt()]	+= itMapBin.value()->GetCount(m_lstSites[i].toInt());
      }
      else
      {
        m_StdfFile.WriteByte('F');
        mapPcrBad[m_lstSites[i].toInt()]	+= itMapBin.value()->GetCount(m_lstSites[i].toInt());
      }
      m_StdfFile.WriteString((itMapBin.value()->strBinName.toLatin1().constData()));
      m_StdfFile.WriteRecord();
    }
    itMapBin.value()->ResetCount();
  }

  // Check if have some SoftBin issue
  if(m_lstSoftBinIssue.count() > 1)
  {
    // Add a warning
    if(!m_strErrorDetail.isEmpty()) m_strErrorDetail+=".\n";
    m_strErrorDetail+= "Attribute[BinAttrType] contains some bin_name (representing the SoftBin number)";
    m_strErrorDetail+= " that are not numeric strings [";
    m_strErrorDetail+= m_lstSoftBinIssue.join(", ")+"]";
    m_iLastError = errWarning;
  }
  RecordReadInfo.iRecordType = 1;
  RecordReadInfo.iRecordSubType = 50;

  for ( itMapBin = m_qMapBins_Soft.begin(); itMapBin != m_qMapBins_Soft.end(); ++itMapBin )
  {
    // Write SBR
    m_StdfFile.WriteHeader(&RecordReadInfo);
    m_StdfFile.WriteByte(255);						// Test Head = ALL
    m_StdfFile.WriteByte(255);						// Test sites = ALL
    m_StdfFile.WriteWord(itMapBin.key());				// HBIN = 0
    m_StdfFile.WriteDword(itMapBin.value()->GetCount(-1));	// Total Bins
    if(itMapBin.value()->bPass)
      m_StdfFile.WriteByte('P');
    else
      m_StdfFile.WriteByte('F');
    m_StdfFile.WriteString((itMapBin.value()->strBinName.toLatin1().constData()));
    m_StdfFile.WriteRecord();

    for(i=0; i<m_lstSites.count(); i++)
    {
      // Write SBR
      m_StdfFile.WriteHeader(&RecordReadInfo);
      m_StdfFile.WriteByte(m_mapAssignment["HEAD_NUM"].toInt());// Test Head
      m_StdfFile.WriteByte(m_lstSites[i].toInt());		// Test sites = ALL
      m_StdfFile.WriteWord(itMapBin.key());				// HBIN = 0
      m_StdfFile.WriteDword(itMapBin.value()->GetCount(m_lstSites[i].toInt()));// Total Bins
      if(itMapBin.value()->bPass)
        m_StdfFile.WriteByte('P');
      else
        m_StdfFile.WriteByte('F');
      m_StdfFile.WriteString((itMapBin.value()->strBinName.toLatin1().constData()));
      m_StdfFile.WriteRecord();
    }
    itMapBin.value()->ResetCount();
  }


  RecordReadInfo.iRecordType = 1;
  RecordReadInfo.iRecordSubType = 30;
  QMap<int,int>::Iterator itPcrCount;
  for ( itPcrCount = mapPcrGood.begin(); itPcrCount != mapPcrGood.end(); ++itPcrCount )
  {
    // Write PCR
    if(itPcrCount.key() == -1)
    {
      m_StdfFile.WriteHeader(&RecordReadInfo);
      m_StdfFile.WriteByte(255);                        // Test Head
      m_StdfFile.WriteByte(255);		// Test sites = ALL
      m_StdfFile.WriteDword(mapPcrGood[-1]+mapPcrBad[-1]);//PART_CNT
      m_StdfFile.WriteDword(0);							//RTST_CNT
      m_StdfFile.WriteDword(0);							//ABRT_CNT
      m_StdfFile.WriteDword(mapPcrGood[-1]);				//GOOD_CNT
      m_StdfFile.WriteDword(0);							//FUNC_CNT
      m_StdfFile.WriteRecord();
    }
    else
    {
      m_StdfFile.WriteHeader(&RecordReadInfo);
      m_StdfFile.WriteByte(m_mapAssignment["HEAD_NUM"].toInt());// Test Head
      m_StdfFile.WriteByte(itPcrCount.key());		// Test sites = ALL
      m_StdfFile.WriteDword(mapPcrGood[itPcrCount.key()]+mapPcrBad[itPcrCount.key()]);//PART_CNT
      m_StdfFile.WriteDword(0);							//RTST_CNT
      m_StdfFile.WriteDword(0);							//ABRT_CNT
      m_StdfFile.WriteDword(mapPcrGood[itPcrCount.key()]);//GOOD_CNT
      m_StdfFile.WriteDword(0);							//FUNC_CNT
      m_StdfFile.WriteRecord();
    }
  }

  // Write MRR
  RecordReadInfo.iRecordType = 1;
  RecordReadInfo.iRecordSubType = 20;
  m_StdfFile.WriteHeader(&RecordReadInfo);
  m_StdfFile.WriteDword(m_lEndTime);			// File finish-time.
  m_StdfFile.WriteByte((BYTE) m_mapAssignment["DISP_COD"][0].toLatin1());			// DISP_COD
  m_StdfFile.WriteString( m_mapAssignment["USR_DESC"].toLatin1().constData());	// USR_DESC
  m_StdfFile.WriteString( m_mapAssignment["EXC_DESC"].toLatin1().constData());	// EXC_DESC
  m_StdfFile.WriteRecord();

  // Close STDF file.
  m_StdfFile.Close();

  if(m_bHaveMultiLot)
  {
    // Rename current STDF file
    QDir clDir;
    QFileInfo clFile(m_strStdfFileName);
    m_lstStdfFileName += clFile.path()+"/"+clFile.baseName() +"_LotId"+m_strLotId+"."+clFile.suffix();
    clDir.rename(m_strStdfFileName,m_lstStdfFileName.last());
    m_strWaferId = "INVALID";
    m_strLotId = "";
    m_bMirWritten = false;

    QMap<QString,CGVERIGY_EDLParameter*>::Iterator itMapParam;
    for ( itMapParam = m_qMapParameterList.begin(); itMapParam != m_qMapParameterList.end(); ++itMapParam )
      m_qMapParameterList[itMapParam.key()]->m_bStaticHeaderWritten = false;

  }

  m_bMrrWritten = true;
  // Success
  return eConvertSuccess;
}

//////////////////////////////////////////////////////////////////////
// Create STDF file from VERIGY_EDL data parsed
//////////////////////////////////////////////////////////////////////
int CGVERIGY_EDLtoSTDF::WriteWir()
{
  if(!m_bWaferMap)
    return eConvertSuccess;

  if(!m_bMirWritten)
    return eConvertError;

  QString strString;
  GS::StdLib::StdfRecordReadInfo RecordReadInfo;

  if(m_strWaferId == "INVALID")
  {
    // Write WCR
    RecordReadInfo.iRecordType = 2;
    RecordReadInfo.iRecordSubType = 30;
    m_StdfFile.WriteHeader(&RecordReadInfo);
    m_StdfFile.WriteFloat( m_mapAssignment["WAFR_SIZ"].toFloat());		// WAFR_SIZ
    m_StdfFile.WriteFloat( m_mapAssignment["DIE_HT"].toFloat());		// DIE_HT
    m_StdfFile.WriteFloat( m_mapAssignment["DIE_WID"].toFloat());		// DIE_WID
    m_StdfFile.WriteByte((BYTE) m_mapAssignment["WF_UNITS"].toInt());	// WF_UNITS
    m_StdfFile.WriteByte((BYTE) m_mapAssignment["WF_FLAT"][0].toLatin1());  // WF_FLAT
    m_StdfFile.WriteWord((short) m_mapAssignment["CENTER_X"].toInt());	// CENTER_X
    m_StdfFile.WriteWord((short) m_mapAssignment["CENTER_Y"].toInt());	// CENTER_Y
    m_StdfFile.WriteByte((BYTE) m_mapAssignment["POS_X"][0].toLatin1());	// POS_X
    m_StdfFile.WriteByte((BYTE) m_mapAssignment["POS_Y"][0].toLatin1());	// POS_Y
    m_StdfFile.WriteRecord();

  }


  // Write WIR if change
  if(m_strWaferId != m_mapAssignment["WAFER_ID"])
  {
    RecordReadInfo.iRecordType = 2;
    RecordReadInfo.iRecordSubType = 10;
    m_StdfFile.WriteHeader(&RecordReadInfo);
    m_StdfFile.WriteByte((BYTE) m_mapAssignment["HEAD_NUM"].toInt());	// HEAD_NUM
    m_StdfFile.WriteByte((BYTE) m_mapAssignment["SITE_GRP"].toInt());	// SITE_GRP
    m_StdfFile.WriteDword(m_lStartTime);								// Start time
    m_StdfFile.WriteString( m_mapAssignment["WAFER_ID"].toLatin1().constData());// WAFER_ID
    m_StdfFile.WriteRecord();
    m_strWaferId = m_mapAssignment["WAFER_ID"];
  }


  // Success
  return eConvertSuccess;
}

//////////////////////////////////////////////////////////////////////
// Create STDF file from VERIGY_EDL data parsed
//////////////////////////////////////////////////////////////////////
int CGVERIGY_EDLtoSTDF::WriteWrr()
{
  if(!m_bMirWritten)
    return eConvertError;

  if(!m_bWaferMap)
    return eConvertSuccess;

  QString strString;
  GS::StdLib::StdfRecordReadInfo RecordReadInfo;
  // Write WRR

  RecordReadInfo.iRecordType = 2;
  RecordReadInfo.iRecordSubType = 20;
  m_StdfFile.WriteHeader(&RecordReadInfo);
  m_StdfFile.WriteByte((BYTE) m_mapAssignment["HEAD_NUM"].toInt());			// HEAD_NUM
  m_StdfFile.WriteByte((BYTE) m_mapAssignment["SITE_GRP"].toInt());			// SITE_GRP
  m_StdfFile.WriteDword(m_lEndTime);											// End time
  m_StdfFile.WriteDword(m_iTotalGoodBin+m_iTotalFailBin);						// PART_CNT
  m_StdfFile.WriteDword(0);													// RTST_CNT
  m_StdfFile.WriteDword(0);													// ABRT_CNT
  m_StdfFile.WriteDword(m_iTotalGoodBin);										// GOOD_CNT
  m_StdfFile.WriteDword(0);													// FUNC_CNT
  m_StdfFile.WriteString( m_strWaferId.toLatin1().constData());				// WAFER_ID
  m_StdfFile.WriteString( m_mapAssignment["FABWF_ID"].toLatin1().constData());// FABWF_ID
  m_StdfFile.WriteString( m_mapAssignment["FRAME_ID"].toLatin1().constData());// FRAME_ID
  m_StdfFile.WriteString( m_mapAssignment["MASK_ID"].toLatin1().constData()); // MASK_ID
  m_StdfFile.WriteString( m_mapAssignment["USR_DESC"].toLatin1().constData());// USR_DESC
  m_StdfFile.WriteString( m_mapAssignment["EXC_DESC"].toLatin1().constData());// EXC_DESC
  m_StdfFile.WriteRecord();

  m_iTotalTests	= 0;
  m_iTotalFailBin = 0;
  m_iTotalGoodBin = 0;
  //m_lstSites.clear();
  QMap<QString,CGVERIGY_EDLParameter*>::Iterator itMapParam;
  for ( itMapParam = m_qMapParameterList.begin(); itMapParam != m_qMapParameterList.end(); ++itMapParam )
  {
    itMapParam.value()->m_bStaticHeaderWritten = false;
  }
  QMap<QString,CGVERIGY_EDLSiteInfo*>::Iterator itSiteInfo;
  for ( itSiteInfo = m_mapSiteInfo.begin(); itSiteInfo != m_mapSiteInfo.end(); ++itSiteInfo )
  {
    delete itSiteInfo.value();
  }
  m_mapSiteInfo.clear();


  m_bPirWritten.clear();

  // Success
  return eConvertSuccess;
}


//////////////////////////////////////////////////////////////////////
// Create STDF file from VERIGY_EDL data parsed
//////////////////////////////////////////////////////////////////////
int CGVERIGY_EDLtoSTDF::WritePir()
{
  if(!m_bMirWritten)
    return eConvertError;

  QString strString;
  GS::StdLib::StdfRecordReadInfo RecordReadInfo;
  // Write PIR
  RecordReadInfo.iRecordType = 5;
  RecordReadInfo.iRecordSubType = 10;

  // For each SiteInfo have to write PIR if not already written
  for(int i=0; i!=m_lstSites.count(); i++)
  {
    if((m_bPirWritten.contains(m_lstSites[i]))
    && (m_bPirWritten[m_lstSites[i]]))
      continue;

    if(m_mapSiteInfo.contains(m_lstSites[i]))
    {
      CGVERIGY_EDLSiteInfo * pSiteInfo = m_mapSiteInfo[m_lstSites[i]];
      if(!pSiteInfo->bExecuted)
        continue;
    }

    m_StdfFile.WriteHeader(&RecordReadInfo);
    m_StdfFile.WriteByte((BYTE) m_mapAssignment["HEAD_NUM"].toInt());
    m_StdfFile.WriteByte((BYTE) m_lstSites[i].toInt());
    m_StdfFile.WriteRecord();

    m_bPirWritten[m_lstSites[i]] = true;
  }

  m_iTotalTests = 0;

  // Success
  return eConvertSuccess;
}

//////////////////////////////////////////////////////////////////////
// Create STDF file from VERIGY_EDL data parsed
//////////////////////////////////////////////////////////////////////
int CGVERIGY_EDLtoSTDF::WritePrr()
{
  if(!m_bMirWritten)
    return eConvertError;

  // Have to Write PRR for each site

  QString strString;
  GS::StdLib::StdfRecordReadInfo RecordReadInfo;
  CGVERIGY_EDLSiteInfo * pSiteInfo;
  for(int i=0; i!=m_lstSites.count(); i++)
  {
    if(m_mapSiteInfo.contains(m_lstSites[i]))
    {
      pSiteInfo = m_mapSiteInfo[m_lstSites[i]];
      m_nPartNumber = pSiteInfo->nPartNumber;
      m_nXLocation = pSiteInfo->nXWafer;
      m_nYLocation = pSiteInfo->nYWafer;
      m_nPassStatus = pSiteInfo->nPassStatus;
      m_nHardBin = pSiteInfo->nHardBin;
      m_nSoftBin = pSiteInfo->nSoftBin;
      m_iTotalTests = pSiteInfo->nTotalTests;
    }
    else
    {
      pSiteInfo = new CGVERIGY_EDLSiteInfo();
      m_mapSiteInfo[m_lstSites[i]] = pSiteInfo;
    }

    if(!pSiteInfo->bExecuted)
      continue;

    // Write PRR
    RecordReadInfo.iRecordType = 5;
    RecordReadInfo.iRecordSubType = 20;
    m_StdfFile.WriteHeader(&RecordReadInfo);
    m_StdfFile.WriteByte((BYTE) m_mapAssignment["HEAD_NUM"].toInt());
    m_StdfFile.WriteByte((BYTE) m_lstSites[i].toInt());
    if(m_nPassStatus == 1)
    {
      m_StdfFile.WriteByte(0);				// PART_FLG : PASSED
      m_iTotalGoodBin++;
      pSiteInfo->nTotalPass++;
    }
    else
    {
      m_StdfFile.WriteByte(8);				// PART_FLG : FAILED
      m_iTotalFailBin++;
      pSiteInfo->nTotalFail++;
    }

    if(!m_mapAssignment["SITE"+m_lstSites[i]+"_PART_TXT"].isEmpty())
      m_mapAssignment["PART_TXT"] = m_mapAssignment["SITE"+m_lstSites[i]+"_PART_TXT"];
    if(!m_mapAssignment["SITE"+m_lstSites[i]+"_PART_FIX"].isEmpty())
      m_mapAssignment["PART_FIX"] = m_mapAssignment["SITE"+m_lstSites[i]+"_PART_FIX"];

    m_StdfFile.WriteWord((WORD)m_iTotalTests);	// NUM_TEST
    m_StdfFile.WriteWord(m_nHardBin);			// HARD_BIN
    m_StdfFile.WriteWord(m_nSoftBin);			// SOFT_BIN
    m_StdfFile.WriteWord(m_nXLocation);			// X_COORD
    m_StdfFile.WriteWord(m_nYLocation);			// Y_COORD
    m_StdfFile.WriteDword((m_lEndTime - m_lStartTime)*1000 + ((m_lEndMicroSeconds-m_lStartMicroSeconds)/1000));	// No testing time known...
    m_StdfFile.WriteString(QString::number(m_nPartNumber).toLatin1().constData());	// PART_ID
    m_StdfFile.WriteString( m_mapAssignment["PART_TXT"].toLatin1().constData());		// PART_TXT
    m_StdfFile.WriteString( m_mapAssignment["PART_FIX"].toLatin1().constData());		// PART_FIX
    m_StdfFile.WriteRecord();

    pSiteInfo->nPassStatus	= -1;
    pSiteInfo->nHardBin		= -1;
    pSiteInfo->nSoftBin		= -1;
    pSiteInfo->nTotalTests	= 0;
    pSiteInfo->bExecuted	= false;
  }


  m_bPirWritten.clear();
  m_nPassStatus = -1;

  // Success
  return eConvertSuccess;
}

//////////////////////////////////////////////////////////////////////
// Create STDF file from VERIGY_EDL data parsed
//////////////////////////////////////////////////////////////////////
int CGVERIGY_EDLtoSTDF::WriteResult()
{
  if(!m_bMirWritten)
    return eConvertError;

  CGVERIGY_EDLSiteInfo *pSiteInfo;
  if(m_mapSiteInfo.contains(m_mapAssignment["SITE_NUM"]))
    pSiteInfo = m_mapSiteInfo[m_mapAssignment["SITE_NUM"]];
  else
  {
    pSiteInfo = new CGVERIGY_EDLSiteInfo();
    m_mapSiteInfo[m_mapAssignment["SITE_NUM"]] = pSiteInfo;
  }
  pSiteInfo->nTotalTests++;
  m_iTotalTests++;

  CGVERIGY_EDLParameter *pTest;
  // Check if have TestNumber available
  if(!m_qMapParameterList.contains(m_strTestSuiteName+m_strTestName))
  {
    if((m_iTestNumber < 0) || m_bIgnoreTestNumber)
    {
      m_bIgnoreTestNumber = true;
      m_iTestNumber = m_qMapParameterList.count() + 1;
    }
    pTest = new CGVERIGY_EDLParameter();
    pTest->m_nTestNum = m_iTestNumber;
    pTest->m_strName = m_strTestName;
    pTest->m_strSuiteName = m_strTestSuiteName;
    m_qMapParameterList[m_strTestSuiteName+m_strTestName] = pTest;
  }

  pTest = m_qMapParameterList[m_strTestSuiteName+m_strTestName];

  switch(m_nTestType)
  {
  case 0:	//0 TEST_TYPE_UNDEFINED
    if(m_bTestResult)
      WritePtr(pTest);
    else if(m_lstPinResult.count() > 0)
      WriteMpr(pTest);
    else
      WriteFtr(pTest);
    break;
  case 1:	//1 TEST_TYPE_FUNCTIONAL
    WriteFtr(pTest);
    break;
  case 2:	//2 TEST_TYPE_PARAMETRIC
  case 3:	//3 TEST_TYPE_MULTIPLE_PARAMETRIC
  case 4:	//4 TEST_TYPE_PARAMETRIC_FUNCTIONAL
  case 5:	//5 TEST_TYPE_ANALOG
  case 6:	//6 TEST_TYPE_FLEX_DC
  default:
    if(m_bTestResult)
      WritePtr(pTest);
    else if(m_lstPinResult.count() > 0)
      WriteMpr(pTest);
    else
      WriteFtr(pTest);
  }

  pTest->m_bStaticHeaderWritten = true;
  m_lstPinIndex.clear();
  m_lstPinResult.clear();
  m_lstPinStat.clear();

  m_mapAssignment["VECT_NAM"] = "";
  m_mapAssignment["TIME_SET"] = "";
  m_mapAssignment["OP_CODE"] = "";
  m_mapAssignment["ALARM_ID"] = "";
  m_mapAssignment["PROG_TXT"] = "";
  m_mapAssignment["RSLT_TXT"] = "";

  m_bTestResult = false;
  m_nTestPFResult = -1;
  // Success
  return eConvertSuccess;
}


//////////////////////////////////////////////////////////////////////
// Create STDF file from VERIGY_EDL data parsed
//////////////////////////////////////////////////////////////////////
int CGVERIGY_EDLtoSTDF::WritePtr(CGVERIGY_EDLParameter *pTest)
{
  BYTE		bData;
  GS::StdLib::StdfRecordReadInfo RecordReadInfo;

  if(!m_bTestResult && (m_lstPinResult.count() == 1))
    m_fTestResult = m_lstPinResult.first().toFloat();

  // Write PTR
  RecordReadInfo.iRecordType = 15;
  RecordReadInfo.iRecordSubType = 10;

  m_StdfFile.WriteHeader(&RecordReadInfo);
  m_StdfFile.WriteDword(pTest->m_nTestNum);				// Test Number
  m_StdfFile.WriteByte(m_mapAssignment["TEST_HEAD"].toInt());	// Test head
  m_StdfFile.WriteByte(m_mapAssignment["SITE_NUM"].toInt());	// Tester site:1,2,3,4 or 5, etc.
  if(m_nTestPFResult == 0)
    bData = 0;		// Test passed
  else
  if(m_nTestPFResult == 1)
    bData = BIT7;	// Test Failed
  else
    bData = BIT2|BIT6|BIT7;	// Unknown
  m_StdfFile.WriteByte(bData);					// TEST_FLG
  bData = 0;
  // Not Strict Low Limit
  if(pTest->m_bValidLowLimit)
    bData |= BIT6;
  //Not Strict High Limit
  if(pTest->m_bValidHighLimit)
    bData |= BIT7;
  m_StdfFile.WriteByte(bData);					// PARAM_FLG
  m_StdfFile.WriteFloat(m_fTestResult);		// Test result
  // save Parameter name without unit information
  m_StdfFile.WriteString(QString(pTest->m_strSuiteName + ":" + pTest->m_strName).toLatin1().constData());	// TEST_TXT
  if(!pTest->m_bStaticHeaderWritten)
  {
    m_StdfFile.WriteString("");							// ALARM_ID

    bData = BIT1|BIT2|BIT3;	// Valid data.
    if(!pTest->m_bValidLowLimit)
      bData |= BIT6;
    if(!pTest->m_bValidHighLimit)
      bData |= BIT7;
    m_StdfFile.WriteByte(bData);							// OPT_FLAG

    m_StdfFile.WriteByte(pTest->m_nScale);						// RES_SCALE
    m_StdfFile.WriteByte(pTest->m_nScale);						// LLM_SCALE
    m_StdfFile.WriteByte(pTest->m_nScale);						// HLM_SCALE
    m_StdfFile.WriteFloat(pTest->m_fLowLimit);	// LOW Limit
    m_StdfFile.WriteFloat(pTest->m_fHighLimit);	// HIGH Limit
    m_StdfFile.WriteString(pTest->m_strUnits.toLatin1().constData());		// Units
  }
  m_StdfFile.WriteRecord();

  // Success
  return eConvertSuccess;
}


//////////////////////////////////////////////////////////////////////
// Create STDF file from VERIGY_EDL data parsed
//////////////////////////////////////////////////////////////////////
int CGVERIGY_EDLtoSTDF::WriteFtr(CGVERIGY_EDLParameter *pTest)
{
  QString		strString;
  BYTE		bData;
  int			nIndex;
  int			i;
  GS::StdLib::StdfRecordReadInfo RecordReadInfo;


  // Write FTR
  RecordReadInfo.iRecordType = 15;
  RecordReadInfo.iRecordSubType = 20;
  m_StdfFile.WriteHeader(&RecordReadInfo);
  m_StdfFile.WriteDword(pTest->m_nTestNum);					// Test Number
  m_StdfFile.WriteByte(m_mapAssignment["HEAD_NUM"].toInt());	// Test head
  m_StdfFile.WriteByte(m_mapAssignment["SITE_NUM"].toInt());	// Tester site:1,2,3,4 or 5, etc.
  if(m_nTestPFResult == 0)
    m_StdfFile.WriteByte(0);		// passed			// TEST_FLG
  else
  if(m_nTestPFResult == 1)
    m_StdfFile.WriteByte(BIT7);		// failed
  else
    m_StdfFile.WriteByte(BIT6);		// no P/F info		// TEST_FLG

  // save empty field for report_readfile.cpp
  m_StdfFile.WriteByte(255);	// opt_flg

  // Always save all

  m_StdfFile.WriteDword(0);						// cycl_cnt
  m_StdfFile.WriteDword(0);						// rel_vadr
  m_StdfFile.WriteDword(0);						// rept_cnt
  m_StdfFile.WriteDword(0);						// num_fail
  m_StdfFile.WriteDword(0);						// xfail_ad
  m_StdfFile.WriteDword(0);						// yfail_ad
  m_StdfFile.WriteWord(0);						// vect_off
  m_StdfFile.WriteWord(m_lstPinStat.count());		// rtn_icnt
  m_StdfFile.WriteWord(0);						// pgm_icnt
  int nMaxPinIndex = 0;
  for(i=0; i<m_lstPinIndex.count(); i++)
  {
    nIndex = m_qMapPins[m_lstPinIndex[i].toInt()]->mapSiteIndex[m_lstSites.indexOf(m_mapAssignment["SITE_NUM"])];
    m_StdfFile.WriteWord(nIndex);				// rtn_indx
    nMaxPinIndex = qMax(nMaxPinIndex, nIndex);
  }

  // modulo 8
  // GCORE-3721 nIndex = m_qMapPins.count() + 1;
  nIndex = nMaxPinIndex + 1;
  nIndex = nIndex + (8 - (nIndex % 8));
  char* szString = new char[nIndex];
  nMaxPinIndex = 0;
  for(i=0; i<nIndex; i++)
    szString[i] = 0;

  QString lQsOne("1");
  nIndex = 0;
  for(i=0; i<(m_lstPinStat.count()+1)/2; i++)
  {
    if((nIndex < m_lstPinStat.count())&&(m_lstPinStat[nIndex] == lQsOne))
    {
      szString[m_qMapPins[m_lstPinIndex[nIndex].toInt()]->mapSiteIndex[m_lstSites.indexOf(m_mapAssignment["SITE_NUM"])]] = 1;
      nMaxPinIndex = qMax(nMaxPinIndex, m_qMapPins[m_lstPinIndex[nIndex].toInt()]->mapSiteIndex[m_lstSites.indexOf(m_mapAssignment["SITE_NUM"])]);
    }
    nIndex++;
    if((nIndex < m_lstPinStat.count())&&(m_lstPinStat[nIndex] == lQsOne))
    {
      szString[m_qMapPins[m_lstPinIndex[nIndex].toInt()]->mapSiteIndex[m_lstSites.indexOf(m_mapAssignment["SITE_NUM"])]] = 1;
      nMaxPinIndex = qMax(nMaxPinIndex, m_qMapPins[m_lstPinIndex[nIndex].toInt()]->mapSiteIndex[m_lstSites.indexOf(m_mapAssignment["SITE_NUM"])]);
    }
    nIndex++;
    m_StdfFile.WriteByte(0);						// RTN_STAT
  }
  if(nMaxPinIndex > 0)
  {
    // modulo 8
    nMaxPinIndex = nMaxPinIndex + (8 - (nMaxPinIndex % 8));
    // two bytes = unsigned count of bytes
    m_StdfFile.WriteWord(nMaxPinIndex);				// fail_pin
    while(nMaxPinIndex > 0)
    {
      bData = 0;
      for(i=8; i>0; i--)
        bData |= szString[--nMaxPinIndex] << (i-1);

      m_StdfFile.WriteByte(bData);
    }
  }
  else
    m_StdfFile.WriteWord(0);													// fail_pin
  m_StdfFile.WriteString(m_mapAssignment["VECT_NAM"].toLatin1().constData());	// vect_name
  m_StdfFile.WriteString(m_mapAssignment["TIME_SET"].toLatin1().constData());	// time_set
  m_StdfFile.WriteString(m_mapAssignment["OP_CODE"].toLatin1().constData());		// op_code
  m_StdfFile.WriteString(QString(pTest->m_strSuiteName + ":" + pTest->m_strName).toLatin1().constData());	// test_txt: test name
  m_StdfFile.WriteString(m_mapAssignment["ALARM_ID"].toLatin1().constData());	// alarm_id
  m_StdfFile.WriteString(m_mapAssignment["PROG_TXT"].toLatin1().constData());	// prog_txt
  m_StdfFile.WriteString(m_mapAssignment["RSLT_TXT"].toLatin1().constData());	// rslt_txt

  m_StdfFile.WriteRecord();

  // Success
  return eConvertSuccess;
}


//////////////////////////////////////////////////////////////////////
// Create STDF file from VERIGY_EDL data parsed
//////////////////////////////////////////////////////////////////////
int CGVERIGY_EDLtoSTDF::WriteMpr(CGVERIGY_EDLParameter *pTest)
{
  QString		strString;
  UINT		nIndex;
  int			i;
  GS::StdLib::StdfRecordReadInfo RecordReadInfo;

  // Write MPR
  RecordReadInfo.iRecordType = 15;
  RecordReadInfo.iRecordSubType = 15;

  float	fFloat;

  m_StdfFile.WriteHeader(&RecordReadInfo);
  m_StdfFile.WriteDword(pTest->m_nTestNum);		// Test Number
  m_StdfFile.WriteByte(m_mapAssignment["HEAD_NUM"].toInt());	// Test head
  m_StdfFile.WriteByte(m_mapAssignment["SITE_NUM"].toInt());	// Tester site

  if(m_nTestPFResult == 0)
    m_StdfFile.WriteByte(0);	// passed			// TEST_FLG
  else
  if(m_nTestPFResult == 1)
    m_StdfFile.WriteByte(BIT7);	// failed			// TEST_FLG
  else
    m_StdfFile.WriteByte(BIT6);	// NO P/F info		// TEST_FLG

  m_StdfFile.WriteByte(BIT6|BIT7);					// PARAM_FLG
  m_StdfFile.WriteWord(m_lstPinStat.count());			// RTN_ICNT
  m_StdfFile.WriteWord(m_lstPinResult.count());		// RSLT_CNT
  for(i=0; i!=(m_lstPinStat.count()+1)/2; i++)
    m_StdfFile.WriteByte(0);						// RTN_STAT

  for(i=0; i<m_lstPinResult.count(); i++)
  {
    fFloat = m_lstPinResult[i].toFloat();
    m_StdfFile.WriteFloat(fFloat);						// Test result
  }

  m_StdfFile.WriteString(QString(pTest->m_strSuiteName + ":" + pTest->m_strName).toLatin1().constData());		// TEST_TXT
  if(!pTest->m_bStaticHeaderWritten)
  {
    m_StdfFile.WriteString("");								// ALARM_ID
    BYTE bData = 0;
    bData |= BIT1|BIT2|BIT3;
    if(!pTest->m_bValidLowLimit)
      bData |= BIT4|BIT6;
    if(!pTest->m_bValidHighLimit)
      bData |= BIT5|BIT7;
    m_StdfFile.WriteByte(bData);				// OPT_FLAG
    m_StdfFile.WriteByte(pTest->m_nScale);		// SCAL
    m_StdfFile.WriteByte(pTest->m_nScale);		// SCAL
    m_StdfFile.WriteByte(pTest->m_nScale);		// SCAL
    m_StdfFile.WriteFloat(pTest->m_fLowLimit);	// LIMIT
    m_StdfFile.WriteFloat(pTest->m_fHighLimit);	// LIMIT


    m_StdfFile.WriteFloat(0);					// StartIn
    m_StdfFile.WriteFloat(0);					// IncrIn

    for(i=0; i<m_lstPinIndex.count(); i++)
    {
      nIndex = m_qMapPins[m_lstPinIndex[i].toInt()]->mapSiteIndex[ m_lstSites.indexOf(m_mapAssignment["SITE_NUM"])];
      m_StdfFile.WriteWord(nIndex);						// rtn_indx
    }
    m_StdfFile.WriteString(pTest->m_strUnits.toLatin1().constData());// TEST_UNIT
    m_StdfFile.WriteString("");					//
    m_StdfFile.WriteString("");					//
    m_StdfFile.WriteString("");					//
    m_StdfFile.WriteString("");					//
    m_StdfFile.WriteFloat(0);		// LIMIT
    m_StdfFile.WriteFloat(0);		// LIMIT
  }

  m_StdfFile.WriteRecord();
  // Success
  return eConvertSuccess;
}


//////////////////////////////////////////////////////////////////////
// Create STDF file from VERIGY_EDL data parsed
//////////////////////////////////////////////////////////////////////
int CGVERIGY_EDLtoSTDF::WriteDtr()
{

  if(m_mapAssignment["DATALOG"].isEmpty())
    return eConvertSuccess;

  if(!m_bMirWritten)
    WriteMir();

  GS::StdLib::StdfRecordReadInfo RecordReadInfo;
  // Write DTR
  RecordReadInfo.iRecordType = 50;
  RecordReadInfo.iRecordSubType = 30;

  QString strDatalog = m_mapAssignment["DATALOG"];
  int nLength = strDatalog.length();
  while(nLength > 0)
  {
    m_StdfFile.WriteHeader(&RecordReadInfo);
    m_StdfFile.WriteString(strDatalog.left(255).toLatin1().constData());
    m_StdfFile.WriteRecord();

    strDatalog = strDatalog.mid(255);
    nLength = strDatalog.length();
  }

  m_mapAssignment["DATALOG"] = "";

  // Success
  return eConvertSuccess;
}

//////////////////////////////////////////////////////////////////////
// Create STDF file from VERIGY_EDL data parsed
//////////////////////////////////////////////////////////////////////
int CGVERIGY_EDLtoSTDF::WriteGdr()
{
  if(m_lstGenericData.isEmpty())
    return eConvertSuccess;

  if(!m_bMirWritten)
    WriteMir();

  int		nType;
  int		nLengh;
  QString strString;
  GS::StdLib::StdfRecordReadInfo RecordReadInfo;
  // Write GDR
  RecordReadInfo.iRecordType = 50;
  RecordReadInfo.iRecordSubType = 10;
  while(!m_lstGenericData.empty())
  {
    m_StdfFile.WriteHeader(&RecordReadInfo);
    UINT nTotalData = m_lstGenericData.count()/3;
    if(nTotalData > 255)
      nTotalData = 255;

    m_StdfFile.WriteWord(nTotalData);
    for(UINT i=0; i<nTotalData; i++)
    {
      if(m_lstGenericData.count()<3)
        break;
      nType = m_lstGenericData.first().toInt();
      m_lstGenericData.pop_front();
      nLengh = m_lstGenericData.first().toInt();
      m_lstGenericData.pop_front();
      strString = m_lstGenericData.first();
      m_lstGenericData.pop_front();
      m_StdfFile.WriteByte(nType);
      if((nType==10) || (nType==11))
      {
        m_StdfFile.WriteByte(nLengh);
      }
      if(nType==12)
      {
        m_StdfFile.WriteWord(nLengh);
      }
      for(int j=0; j!=nLengh; j++)
        m_StdfFile.WriteByte((BYTE)strString[j].unicode());
    }

    m_StdfFile.WriteRecord();
  }

  m_lstGenericData.clear();
  // Success
  return eConvertSuccess;
}

//////////////////////////////////////////////////////////////////////
// Convert 'FileName' VERIGY_EDL file, to STDF 'strFileNameSTDF' file
//////////////////////////////////////////////////////////////////////
int CGVERIGY_EDLtoSTDF::Convert(const char *VerigyEdlFileName, QStringList &lstFileNameSTDF, bool bAllowOnlyOneFile, QDate *pExpirationDate)
{
  m_bSplitMultiLot = !bAllowOnlyOneFile;


  // No erro (default)
  m_iLastError = errNoError;
  m_strErrorDetail = "";

  // If STDF file already exists...do not rebuild it...unless dates not matching!
  QFileInfo fInput(VerigyEdlFileName);
  QFileInfo fOutput(lstFileNameSTDF.first());

    QFile f( lstFileNameSTDF.first() );
    if((f.exists() == true) && (fInput.lastModified() < fOutput.lastModified()))
    return eConvertSuccess;

  m_strVerigyEdlFileName = VerigyEdlFileName;
  m_strStdfFileName = lstFileNameSTDF.first();
  lstFileNameSTDF.clear();

  int nStatus = ReadVerigyEdlFile(pExpirationDate);

  if(lstFileNameSTDF.isEmpty())
    lstFileNameSTDF += m_lstStdfFileName;

  return nStatus;
}

//////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////
int	 CGVERIGY_EDLtoSTDF::ReadBlock(QFile* pFile, char *data, quint64 len)
{
  return pFile->read(data, len);
}


//////////////////////////////////////////////////////////////////////
int CGVERIGY_EDLtoSTDF::PrefixUnitToScall(QString strUnit)
{
  QString strPrefix;
  QString strNewUnit = strUnit.remove("_").trimmed();
  if(strNewUnit.length() < 2) return 0;

  strPrefix = strNewUnit.left(2);
  if(strPrefix == "EX") return -18;
  if(strPrefix == "PE") return -15;

  strPrefix = strNewUnit.left(1);
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
  if(strPrefix == "a") return 18;

  return 0;
}

//////////////////////////////////////////////////////////////////////
QString CGVERIGY_EDLtoSTDF::PrefixUnitToUnit(QString strUnit)
{
  QString strPrefix;
  QString strNewUnit = strUnit.remove("_").trimmed();
  if(strNewUnit.length() < 2)
    return strNewUnit;

  strPrefix = strNewUnit.left(2);
  if((strPrefix == "EX") || (strPrefix == "PE"))
    return strNewUnit.right(strNewUnit.length()-2);

  strPrefix = strNewUnit.left(1);

  if((strPrefix == "T") || (strPrefix == "G") || (strPrefix == "M") || (strPrefix == "K")
  || (strPrefix == "%") || (strPrefix == "m") || (strPrefix == "u") || (strPrefix == "n")
  || (strPrefix == "p") || (strPrefix == "f") || (strPrefix == "a"))
    return strNewUnit.right(strNewUnit.length()-1);;

  return strNewUnit;
}
