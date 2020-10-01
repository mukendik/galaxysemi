//////////////////////////////////////////////////////////////////////
// import_semi_e142_xml.cpp: Convert a SemiE142Xml Xml file to STDF V4.0
//////////////////////////////////////////////////////////////////////
#include <qfileinfo.h>
#include <qprogressbar.h>
#include <qapplication.h>
#include <qlabel.h>
#include <qregexp.h>

#include "import_semi_e142_xml.h"
#include "time.h"
#include "import_constants.h"
#include "patman_lib.h"
#include "engine.h"

//<?xml version="1.0" encoding="utf-8" ?>
//<MapData xmlns="urn:semiorg:xsd.E142-1.V0105.SubstrateMap">
//<SubstrateMaps>
//<SubstrateMap SubstrateType="Wafer" SubstrateId="EVAKRUN1SWJ1_VF0KH4F_j973t10_77611_030113_111457_8Z_stdf-VF0KH4F" Orientation="0" OriginLocation="toUpperLeft" AxisDirection="DownRight" LayoutSpecifier="WaferLayout/Devices">
//<Overlay MapName="SortGrade" MapVersion="1">
//<ReferenceDevices>
//<ReferenceDevice Name="FirstDevice">
//<Coordinates X="18" Y="1"/>
//</ReferenceDevice>
//</ReferenceDevices>
//<BinCodeMap BinType="HexaDecimal" NullBin="FF" MapType="2DArray">
//<BinDefinitions>
//<BinDefinition BinCode= "00" BinCount="165" BinQuality="Fail" BinDescription=""/>
//</BinDefinitions>
//<BinCode>FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00FFFFFF</BinCode>
//<BinCode>FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF</BinCode>
//<BinCode>FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF</BinCode>
//<BinCode>FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF</BinCode>
//<BinCode>FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF</BinCode>
//<BinCode>FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF</BinCode>
//<BinCode>FFFFFFFFFFFFFFFFFF000000000000000000FFFFFF</BinCode>
//<BinCode>FFFFFF000000FF00000000FF0000FFFF00FFFFFFFF</BinCode>
//<BinCode>FFFF000000000000FF00000000000000000000FF00</BinCode>
//<BinCode>000000000000000000000000000000FF0000000000</BinCode>
//<BinCode>000000000000000000000000FF0000FF000000FF00</BinCode>
//<BinCode>000000000000000000000000000000FF00FF00FFFF</BinCode>
//<BinCode>FF000000000000000000000000FFFF0000000000FF</BinCode>
//<BinCode>FFFF0000000000000000000000FF00FF0000FFFFFF</BinCode>
//<BinCode>FFFF00000000FFFF0000FF00FF00000000FFFFFFFF</BinCode>
//<BinCode>FFFFFF000000000000000000FFFF000000FFFFFFFF</BinCode>
//<BinCode>FFFFFFFFFF000000000000000000FFFFFFFFFFFFFF</BinCode>
//<BinCode>FFFFFFFFFFFFFF000000000000FFFFFFFFFFFFFFFF</BinCode>
//</BinCodeMap>
//</Overlay>
//</SubstrateMap>
//</SubstrateMaps>
//</MapData>

// main.cpp
extern QLabel			*GexScriptStatusLabel;	// Handle to script status text in status bar
extern QProgressBar	*	GexProgressBar;		// Handle to progress bar in status bar


CSemiE142XmlBinInfo::CSemiE142XmlBinInfo()
{
  nBinNb = 0;
  nNbCntSummary = -1;
  nNbCnt = 0;
  bPass  = false;
};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGSemiE142XmltoSTDF::CGSemiE142XmltoSTDF()
{
  m_lStartTime	= 0;
  m_nWaferRows	= 0;
  m_nWaferColumns = 0;
  m_fWaferSizeX	= 0;
  m_fWaferSizeY	= 0;
  m_nWaferUnit	= 3;
  m_nNbRefDies	= 0;
  m_nOriginPosX = -32768;
  m_nOriginPosY = -32768;
  m_nFirstRefPosX = -32768;
  m_nFirstRefPosY = -32768;
  m_cWfFlat		= ' ';
  m_cPosX			= 'R';
  m_cPosY			= 'D';
  m_cOrgPosX		= 'L';
  m_cOrgPosY		= 'U';
  m_nWaferBytePerDie = 0;
  m_strStatus		= "Product";
  m_bHaveGoodBins	= false;
  m_bSaveBinCntSummary = false;
  m_nBinType		= 10;


  m_bEndOfFile = false;
  m_XMLManager.AttachObserver(this);
}

//////////////////////////////////////////////////////////////////////
// Destruction
//////////////////////////////////////////////////////////////////////
CGSemiE142XmltoSTDF::~CGSemiE142XmltoSTDF()
{
  QMap<int,CSemiE142XmlBinInfo*>::Iterator itMapBin;
  for ( itMapBin = m_qMapBins.begin(); itMapBin != m_qMapBins.end(); ++itMapBin )
  {
    delete itMapBin.value();
  }
}

//////////////////////////////////////////////////////////////////////
// Get Error
//////////////////////////////////////////////////////////////////////
QString CGSemiE142XmltoSTDF::GetLastError()
{
  m_strLastError = "Import Semi E142 Xml: ";

  if(!m_strLastErrorSpecification.isEmpty())
    m_strLastError += "\n\n" + m_strLastErrorSpecification + "\n\n";

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
    case errFormatNotSupported:
      m_strLastError += "Invalid file format: not a 2DWafer Map";
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
// Check if File is compatible with SemiE142Xml format
//////////////////////////////////////////////////////////////////////
bool CGSemiE142XmltoSTDF::IsCompatible(const char *szFileName)
{
    XML::XMLParser lXMLManager;

    if(lXMLManager.LoadXMLFile(szFileName) == false) {
        return false;
    }

    QString lAttribut = lXMLManager.GetNameSpaceURI("MapData");
    return (lAttribut.indexOf("E142-") >= 0);
}

void CGSemiE142XmltoSTDF::Notify()
{
    if(GexProgressBar)
    {
        int lStep        = m_XMLManager.GetProgressStep();
        int lTotalNodes  = m_XMLManager.GetTotalNodes();

        if(lTotalNodes > 0)
            GexProgressBar->setValue((lStep*100)/lTotalNodes);
    }
    QCoreApplication::processEvents();
}


uint CGSemiE142XmltoSTDF::BuildDate(QString strString)
{
    if(strString.isEmpty())
        return 0;

    // -- yyyymmddhhmisszzz
    int iYear, iMonth, iDay, iHour, iMin, iSec;
    // -- Extract the date
    iYear   = strString.left(4).toInt();
    iMonth  = strString.mid(4,2).toInt();
    iDay    = strString.mid(6,2).toInt();
    // -- Extract the time
    iHour   = strString.mid(8,2).toInt();
    iMin    = strString.mid(10,2).toInt();
    iSec    = strString.mid(12,2).toInt();

    // -- Save this date
    QDate       clDate(iYear,iMonth,iDay);
    QTime       clTime(iHour,iMin,iSec);
    QDateTime   clDateTime(clDate,clTime);
    clDateTime.setTimeSpec(Qt::UTC);
    return clDateTime.toTime_t();
}

//////////////////////////////////////////////////////////////////////
// Read and Parse the SemiE142Xml file
//////////////////////////////////////////////////////////////////////
bool CGSemiE142XmltoSTDF::ReadSemiE142XmlFile(const char *semiE142XmlFileName,const char *strFileNameSTDF)
{
    if(m_XMLManager.LoadXMLFile(semiE142XmlFileName) == false) {
        m_iLastError = errOpenFail;
        return false;
    }

    if(m_XMLManager.GetNbIdenticalNodes("BinCodeMap") > 1) {
        m_iLastError                = errInvalidFormat;
        m_strLastErrorSpecification = "Multi Wafer Map not allowed";
        return false;
    }


    QString lStrValue;
    // -- Clear list of Invalid bins (eg: Ugly dies, Null Bin)
    m_strInvalidBinList.clear();

    // -- Layout
    m_XMLManager.SetRepeatedNodes("Layout");
    while(m_XMLManager.NextRepeatedNode()) {

        QString lStrLayoutId     = m_XMLManager.GetRepeatedNodeAttribut("LayoutId");
        QString lStrUnit         = m_XMLManager.GetRepeatedNodeAttribut("DefaultUnits");
        QString lStrTopLevel     = m_XMLManager.GetRepeatedNodeAttribut("TopLevel");
        bool bTopLevel           = lStrTopLevel.contains("True", Qt::CaseInsensitive);

        m_strCurrentLayout = lStrLayoutId.toUpper();

        if(!bTopLevel) {
          m_mapChildLayouts[m_strCurrentLayout].strLayoutName = m_strCurrentLayout;
          m_mapChildLayouts[m_strCurrentLayout].strUnit = lStrUnit;
        }

        if(!m_strCurrentLayout.isEmpty()) {
            lStrValue               = m_XMLManager.GetAttribute("LowerLeft", "X") ;
            if(!lStrValue.isEmpty())
                 m_mapChildLayouts[m_strCurrentLayout].nLowerLeftX = lStrValue.toInt();

            lStrValue                = m_XMLManager.GetAttribute("LowerLeft", "Y") ;
            if(!lStrValue.isEmpty())
                 m_mapChildLayouts[m_strCurrentLayout].nLowerLeftY = lStrValue.toInt();

            lStrValue                = m_XMLManager.GetAttribute("DeviceSize", "X") ;
            if(!lStrValue.isEmpty())
              m_mapChildLayouts[m_strCurrentLayout].fDeviceSizeX = lStrValue.toFloat();

            lStrValue                = m_XMLManager.GetAttribute("DeviceSize", "Y") ;
            if(!lStrValue.isEmpty())
              m_mapChildLayouts[m_strCurrentLayout].fDeviceSizeY = lStrValue.toFloat();

            lStrValue                = m_XMLManager.GetAttribute("StepSize", "X") ;
            if(!lStrValue.isEmpty())
              m_mapChildLayouts[m_strCurrentLayout].fDeviceSizeX = lStrValue.toFloat();

            lStrValue                = m_XMLManager.GetAttribute("StepSize", "Y") ;
            if(!lStrValue.isEmpty())
              m_mapChildLayouts[m_strCurrentLayout].fDeviceSizeY = lStrValue.toFloat();

            lStrValue                = m_XMLManager.GetAttribute("StepSize", "Units") ;
            if(!lStrValue.isEmpty())
              m_mapChildLayouts[m_strCurrentLayout].strUnit = lStrValue;

            m_strProductId           = m_XMLManager.GetText("ProductId");
            if(!m_strProductId.isEmpty())
                m_mapChildLayouts[m_strCurrentLayout].strProductId = m_strProductId;
        }
    }

    // -- Substrate
    m_XMLManager.SetRepeatedNodes("Substrate");
    while(m_XMLManager.NextRepeatedNode()) {
        lStrValue = m_XMLManager.GetRepeatedNodeAttribut("SubstrateType");
        if(!lStrValue.isEmpty() && !lStrValue.startsWith("Wafer",Qt::CaseInsensitive)) {
            m_iLastError = errFormatNotSupported;
            m_strLastErrorSpecification = "SubstrateType value: " + lStrValue + " not supported";
            return false;
        }
        m_strNodeName       = m_XMLManager.GetAttribute("Tester", "Value");
        m_strProgramName    = m_XMLManager.GetAttribute("TestProgram", "Value");
        m_strProgramVer     = m_XMLManager.GetAttribute("TestProgVer", "Value") ;
        m_strHandlerId      = m_XMLManager.GetAttribute("Prober", "Value");
        m_strLoadBoardId    = m_XMLManager.GetAttribute("LoadBoard", "Value");
        m_strProbeId        = m_XMLManager.GetAttribute("ProbeCard", "Value");
        m_strOperatorName   = m_XMLManager.GetAttribute("Operator", "Value");
        m_strAuxFile        = m_XMLManager.GetAttribute("SetupFile", "Value");
        m_strNodeName       = m_XMLManager.GetAttribute("TestSystem", "Value");

        m_strLotId          = m_XMLManager.GetText("LotId");
        m_lStartTime        = BuildDate(m_XMLManager.GetText("CreateDate"));
        m_lEndTime          = BuildDate(m_XMLManager.GetText("LastModified"));
        m_strOperatorName   = m_XMLManager.GetText("SupplierName");

    }

    // -- Alias Id
    m_XMLManager.SetRepeatedNodes("AliasId");
    while(m_XMLManager.NextRepeatedNode()) {
       QString lType    = m_XMLManager.GetRepeatedNodeAttribut("Type");
       lStrValue        = m_XMLManager.GetRepeatedNodeAttribut("Value");

        if     (lType.startsWith("Tester"),         Qt::CaseInsensitive)    m_strNodeName       = lStrValue;
        else if(lType.startsWith("TestProgram"),    Qt::CaseInsensitive)    m_strProgramName    = lStrValue;
        else if(lType.startsWith("TestProgVer"),    Qt::CaseInsensitive)    m_strProgramVer     = lStrValue;
        else if(lType.startsWith("Prober"),         Qt::CaseInsensitive)    m_strHandlerId      = lStrValue;
        else if(lType.startsWith("LoadBoard"),      Qt::CaseInsensitive)    m_strLoadBoardId    = lStrValue;
        else if(lType.startsWith("ProbeCard"),      Qt::CaseInsensitive)    m_strProbeId        = lStrValue;
        else if(lType.startsWith("Operator"),       Qt::CaseInsensitive)    m_strOperatorName   = lStrValue;
        else if(lType.startsWith("SetupFile"),      Qt::CaseInsensitive)    m_strAuxFile        = lStrValue;
        else if(lType.startsWith("TestSystem"),     Qt::CaseInsensitive)    m_strNodeName       = lStrValue;
        else if(lType.startsWith("TestStartTime"),   Qt::CaseInsensitive)    m_lStartTime        = BuildDate(lStrValue);
        else if(lType.startsWith("TestEndTime"),    Qt::CaseInsensitive)    m_lEndTime          = BuildDate(lStrValue);
        else
            continue;
    }

    // -- SubstrateMap
    m_XMLManager.SetRepeatedNodes("SubstrateMap");
    while(m_XMLManager.NextRepeatedNode()) {
        lStrValue        = m_XMLManager.GetRepeatedNodeAttribut("SubstrateType");
        if(!lStrValue.isEmpty() && !lStrValue.startsWith("Wafer",Qt::CaseInsensitive)) {
          m_iLastError = errFormatNotSupported;
          m_strLastErrorSpecification = "SubstrateType value: " + lStrValue + " not supported";
          return false;
        }

        m_strWaferId    = m_XMLManager.GetRepeatedNodeAttribut("SubstrateId");
        if(m_strLotId.isEmpty())
            m_strLotId = m_strWaferId;

       int iNotch        = m_XMLManager.GetRepeatedNodeAttribut("Orientation").toInt();
       switch(iNotch)
       {
         case 180:	// Up
           m_cWfFlat = 'U';
           break;
         case 0:		// Down
           m_cWfFlat = 'D';
           break;
         case 270:	// Right
           m_cWfFlat = 'R';
           break;
         case 90:	// LEFT
           m_cWfFlat = 'L';
           break;
       }

       lStrValue        = m_XMLManager.GetRepeatedNodeAttribut("OriginLocation");

       if       (lStrValue.startsWith ("UPPER",    Qt::CaseInsensitive)) m_cOrgPosY = 'U';
       else if  (lStrValue.startsWith ("LOWER",    Qt::CaseInsensitive)) m_cOrgPosY = 'D';
       else if  (lStrValue.startsWith ("CENTER",   Qt::CaseInsensitive)) m_cOrgPosY = 'C';

       if       (lStrValue.endsWith   ("LEFT",     Qt::CaseInsensitive)) m_cOrgPosX = 'L';
       else if  (lStrValue.endsWith   ("RIGHT",    Qt::CaseInsensitive)) m_cOrgPosX = 'R';
       else if  (lStrValue.startsWith ("CENTER",   Qt::CaseInsensitive)) m_cOrgPosX = 'C';

       lStrValue        = m_XMLManager.GetRepeatedNodeAttribut("AxisDirection");
       if       (lStrValue.startsWith("DOWN",   Qt::CaseInsensitive))   m_cPosY = 'D';
       else if  (lStrValue.startsWith("UP",     Qt::CaseInsensitive))   m_cPosY = 'U';

       if       (lStrValue.endsWith("RIGHT",    Qt::CaseInsensitive))   m_cPosX = 'R';
       else if  (lStrValue.endsWith("LEFT",     Qt::CaseInsensitive))   m_cPosX = 'L';

       m_strCurrentLayout = m_XMLManager.GetRepeatedNodeAttribut("LayoutSpecifier").section("/",1).toUpper();
    }

    // -- Reference device
    m_XMLManager.SetRepeatedNodes("ReferenceDevice");
    while(m_XMLManager.NextRepeatedNode()) {
        ++m_nNbRefDies;
        lStrValue = m_XMLManager.GetRepeatedNodeAttribut("Name");
        int lX = m_XMLManager.GetAttribute("Coordinates", "X").toInt();
        int lY = m_XMLManager.GetAttribute("Coordinates", "Y").toInt();

        if(!lStrValue.compare("OriginLocation", Qt::CaseInsensitive)) {
          m_nOriginPosX = lX;
          m_nOriginPosY = lY;
        }

        if(!lStrValue.compare("FirstDevice", Qt::CaseInsensitive)) {
          m_nFirstRefPosX = lX;
          m_nFirstRefPosY = lY;
        }
    }

    // -- Bin Code
    m_nBinType = 10;
    lStrValue = m_XMLManager.GetAttribute("BinCodeMap", "BinType");
    if(lStrValue.startsWith("Hexa",Qt::CaseInsensitive)) {
      m_nBinType            = 16;
      m_nWaferBytePerDie    = 2;
    }
    else if(lStrValue.startsWith("ASCII",Qt::CaseInsensitive))
      m_nWaferBytePerDie = 1;
    else if(lStrValue.startsWith("Dec",Qt::CaseInsensitive))
      m_nWaferBytePerDie = 3;
    else if(lStrValue.startsWith("Int",Qt::CaseInsensitive)) {
      m_nBinType = 16;
      m_nWaferBytePerDie = 4;
    }

    m_strBinNull        = m_XMLManager.GetAttribute("BinCodeMap", "NullBin");
    m_nWaferBytePerDie  = m_strBinNull.length();

    lStrValue = m_XMLManager.GetAttribute("BinCodeMap", "MapType");

    if( !lStrValue.isEmpty() && !lStrValue.startsWith("2DArray",Qt::CaseInsensitive)) {
        m_iLastError = errFormatNotSupported;
        m_strLastErrorSpecification = " MapType value :" + lStrValue + "' is not supported";
        return false;
    }

    // -- Bin Definition
    m_XMLManager.SetRepeatedNodes("BinDefinition");
    while(m_XMLManager.NextRepeatedNode()) {
        QString strBinNum, strBinName, strBinCat, strBinCountSummary;
        strBinNum           = m_XMLManager.GetRepeatedNodeAttribut("BinCode");
        strBinName          = m_XMLManager.GetRepeatedNodeAttribut("BinDescription");
        strBinCat           = m_XMLManager.GetRepeatedNodeAttribut("BinQuality");
        strBinCountSummary  = m_XMLManager.GetRepeatedNodeAttribut("BinCount");

        // Save bin info
        CSemiE142XmlBinInfo *pBin;
        bool	bOk;
        if(!m_qMapBins.contains(strBinNum.toInt(&bOk,m_nBinType))) {
            pBin = new CSemiE142XmlBinInfo();
            pBin->nBinNb = strBinNum.toInt(&bOk,m_nBinType);
            m_qMapBins[pBin->nBinNb] = pBin;
        }
        else
            pBin = m_qMapBins[strBinNum.toInt(&bOk,m_nBinType)];

        if(strBinName.isEmpty())
            strBinName = "E142Bin("+strBinNum+")";
        pBin->strName = strBinName;
        if(!strBinCountSummary.isEmpty()) {
            pBin->nNbCntSummary = strBinCountSummary.toInt();
            m_bSaveBinCntSummary = true;
        }
        pBin->bPass = strBinCat.startsWith("Pass",Qt::CaseInsensitive);
        if(pBin->bPass)
            m_bHaveGoodBins = true;

        // -- Keep list of Bins that are to be ignored: Ugly & Null
        if(strBinCat.startsWith("Ugly",Qt::CaseInsensitive) || strBinCat.startsWith("Null",Qt::CaseInsensitive))
            m_strInvalidBinList << strBinNum;

        // -- Initialize m_nWaferBytePerDie if necessary
        // -- Try to find the number of char for DieBin
        if(m_nWaferBytePerDie < strBinNum.length()) {
            m_nWaferBytePerDie = strBinNum.length();
        }
    }

    if(!WriteStdfFile(strFileNameSTDF))
    {
      // Convertion failed.
      QFile::remove(strFileNameSTDF);
      return false;
    }

    return true;
}

//////////////////////////////////////////////////////////////////////
// Create STDF file from SemiE142Xml data parsed
//////////////////////////////////////////////////////////////////////
bool CGSemiE142XmltoSTDF::WriteStdfFile(const char *strFileNameSTDF)
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

  if(m_mapChildLayouts.contains(m_strCurrentLayout))
  {
    m_strProductId = m_mapChildLayouts[m_strCurrentLayout].strProductId;
    m_fWaferSizeX = m_mapChildLayouts[m_strCurrentLayout].fDeviceSizeX;
    m_fWaferSizeY = m_mapChildLayouts[m_strCurrentLayout].fDeviceSizeY;
    if(m_mapChildLayouts[m_strCurrentLayout].strUnit.startsWith("inch",Qt::CaseInsensitive))
      m_nWaferUnit = 1;
    else
    if((m_mapChildLayouts[m_strCurrentLayout].strUnit.startsWith("centi",Qt::CaseInsensitive))
    || (m_mapChildLayouts[m_strCurrentLayout].strUnit.startsWith("cm",Qt::CaseInsensitive)))
      m_nWaferUnit = 2;
    else
    if((m_mapChildLayouts[m_strCurrentLayout].strUnit.startsWith("milli",Qt::CaseInsensitive))
    || (m_mapChildLayouts[m_strCurrentLayout].strUnit.startsWith("mm",Qt::CaseInsensitive)))
      m_nWaferUnit = 3;
    else
    if((m_mapChildLayouts[m_strCurrentLayout].strUnit.startsWith("micro",Qt::CaseInsensitive))
    || (m_mapChildLayouts[m_strCurrentLayout].strUnit.startsWith("µm",Qt::CaseInsensitive)))
      m_nWaferUnit = 4;

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
  StdfFile.WriteByte((BYTE) m_strStatus[0].toLatin1());				// Test Mode = PRODUCTION
  StdfFile.WriteByte((BYTE) ' ');				// rtst_cod
  StdfFile.WriteByte((BYTE) ' ');				// prot_cod
  StdfFile.WriteWord(65535);					// burn_tim
  StdfFile.WriteByte((BYTE) ' ');				// cmod_cod
  StdfFile.WriteString(m_strLotId.toLatin1().constData());		// Lot ID
  StdfFile.WriteString(m_strProductId.toLatin1().constData());	// Part Type / Product ID
  StdfFile.WriteString(m_strNodeName.toLatin1().constData());	// Node name
  StdfFile.WriteString("");					// Tester Type
  StdfFile.WriteString(m_strProgramName.toLatin1().constData());// Job name
  StdfFile.WriteString(m_strProgramVer.toLatin1().constData()); // Job rev
  StdfFile.WriteString("");					// sublot-id
  StdfFile.WriteString(m_strOperatorName.toLatin1().constData());// operator
  StdfFile.WriteString("");					// exec-type
  StdfFile.WriteString("");					// exe-ver
  StdfFile.WriteString("WAFER");				// test-cod
  StdfFile.WriteString("");					// test-temperature
  // Construct custom Galaxy USER_TXT
  QString	strUserTxt;
  strUserTxt = GEX_IMPORT_DATAORIGIN_LABEL;
  strUserTxt += ":";
  strUserTxt += GEX_IMPORT_DATAORIGIN_ATETEST;
  strUserTxt += ":SemiE142/XML";
  StdfFile.WriteString((char *)strUserTxt.toLatin1().constData());		// user-txt
  StdfFile.WriteString(m_strAuxFile.toLatin1().constData());	// aux-file
  StdfFile.WriteString("");					// package-type
  StdfFile.WriteString("");					// familyID
  StdfFile.WriteString("");					// Date-code
  StdfFile.WriteString("");					// Facility-ID
  StdfFile.WriteString("");					// FloorID
  StdfFile.WriteString("");					// ProcessID
  StdfFile.WriteRecord();

  if(!m_strHandlerId.isEmpty() || !m_strProbeId.isEmpty() || !m_strLoadBoardId.isEmpty())
  {
    // Write SDR for last wafer inserted
    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 80;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteByte(255);						// Test Head = ALL
    StdfFile.WriteByte((BYTE)1);			// head#
    StdfFile.WriteByte((BYTE)1);			// Group#
    StdfFile.WriteByte((BYTE)1);			// site_count
    StdfFile.WriteByte((BYTE)1);			// array of test site#
    StdfFile.WriteString("");				// HAND_TYP: Handler/prober type
    StdfFile.WriteString(m_strHandlerId.toLatin1().constData());		// HAND_ID: Handler/prober name
    StdfFile.WriteString("");				// CARD_TYP: Probe card type
    StdfFile.WriteString(m_strProbeId.toLatin1().constData());		// CARD_ID: Probe card id
    StdfFile.WriteString("");				// LOAD_TYP: Probe card type
    StdfFile.WriteString(m_strLoadBoardId.toLatin1().constData());	// LOAD_ID: LoadBoard id
    StdfFile.WriteRecord();
  }

  // Write Test results for each line read.
  QString		strString, strBin, strValue;
  float		fValue;
  int			iIndex;				// Loop index
  WORD		wHardBin;
  long		iTotalGoodBin,iTotalFailBin;
  long		iPartNumber;
  //FIXME: not used ?
  //bool bPassStatus;
  int			iWaferColumns;
  int			iWaferRows;
  int			iStepX;
  int			iStepY;

  int			iFirstDiePosX;
  int			iFirstDiePosY;

  iFirstDiePosX = iFirstDiePosY = -32768;
  iPartNumber = iWaferColumns = iWaferRows = 0;
  iTotalGoodBin = iTotalFailBin = 0;

  iStepX = iStepY = 1;
  if(m_cPosX == 'L')
    iStepX = -1;

  if(m_cPosY == 'U')
    iStepY = -1;

  iWaferRows = -iStepY;
  m_nWaferRows = 0;

  // Checks if PAT running at ST-Microelectronics customer site
  // ST specs: Ignore bins where BinQuality is Not Pass or Fail (eg: Null, or Ugly)
  bool bST_MicroPAT = (getenv(PATMAN_USER_ST) != NULL);

  // Origin Location and AxisDirection
  // We need to have the number of rows and the numbers of columns
  // to retrieve the wafer die coordinates.
  // Have to parse first the WaferMap
  // and then dump the STDF format
  QList<CSemiE142XmlDevice*> m_lstWaferMap;
  //while(hSemiE142XmlFile->atEnd() == false)
  m_XMLManager.SetRepeatedNodes("BinCode");
  QRegExp space( "\\s" );     // match whitespace
  while(m_XMLManager.NextRepeatedNode())
  {
      m_strTagValue = m_XMLManager.GetRepeatedNodeText();
      m_strTagValue = m_strTagValue.remove(space);
      if((m_nWaferBytePerDie == 0) || (m_nWaferColumns == 0)) {
        if(m_nWaferColumns > 0) {
          m_nWaferBytePerDie = (int)m_strTagValue.length()/m_nWaferColumns;

          // -- Check if it's ok
          fValue = (float) m_strTagValue.length() / (float) m_nWaferColumns;
          if(fValue != (float) m_nWaferBytePerDie)  {
            // have an error
            m_iLastError = errInvalidFormat;
            m_strLastErrorSpecification = "Inconsistency <BinCode> format";
            return false;
          }
        }
        else{
          if(m_nWaferBytePerDie == 0)
            m_nWaferBytePerDie = 2;

          m_nWaferColumns = m_strTagValue.length()/m_nWaferBytePerDie;
        }
      }

      m_nWaferRows ++;

      iWaferRows+=iStepY;
      iWaferColumns = -iStepX;

      // Reset Pass/Fail flag.
      //FIXME: not used ?
      //bPassStatus = true;

      // Read Part results for this record
      for(iIndex=0;iIndex<m_nWaferColumns;iIndex++)
      {
        iWaferColumns+=iStepX;

        strBin = m_strTagValue.mid(iIndex*m_nWaferBytePerDie,m_nWaferBytePerDie);
        if((!m_strBinNull.isEmpty()) && (strBin.startsWith(m_strBinNull)))
        {
          // Part not tested
          continue;
        }
        // Chech if Bin# is an invalid Bin# (eg: NullBin, UglyBin)
        if(m_strInvalidBinList.contains(strBin))
        {
          // Ignore this die...only ST PAT-Man specific for now!
          if(bST_MicroPAT)
            continue;
        }

        // Have to convert in decimal
        bool ok;
        wHardBin = strBin.toInt(&ok,m_nBinType);
        CSemiE142XmlBinInfo *pBin;

        if(!m_qMapBins.contains(wHardBin))
        {
          pBin = new CSemiE142XmlBinInfo();
          pBin->nBinNb = wHardBin;
          pBin->strName = "E142Bin("+strBin+")";
          m_qMapBins[pBin->nBinNb] = pBin;
          m_bSaveBinCntSummary = false;

          if(m_bHaveGoodBins)
            pBin->bPass = false;
          else
          {
            pBin->bPass = (wHardBin == 1);
            m_bHaveGoodBins = pBin->bPass;
          }
        }
        else
        {
          pBin = m_qMapBins[wHardBin];
        }

        iPartNumber++;

        // Save the position of the FirstDie
        if((iStepX > 0) && (iStepY > 0))
        {
          // UpperLeft
          if(iFirstDiePosX == -32768)
            iFirstDiePosX = iWaferColumns;
          if(iFirstDiePosY == -32768)
            iFirstDiePosY = iWaferRows;
        }
        else
        if((iStepX > 0) && (iStepY < 0))
        {
          // LowerLeft
          // Have to find the last Row
          if(iFirstDiePosY != iWaferRows)
            iFirstDiePosX = iFirstDiePosY = -32768;

          if(iFirstDiePosX == -32768)
            iFirstDiePosX = iWaferColumns;
          if(iFirstDiePosY == -32768)
            iFirstDiePosY = iWaferRows;

        }
        else
        if((iStepX < 0) && (iStepY < 0))
        {
          // LowerRight
          // Have to find the last Row
          // Add the last die
          iFirstDiePosX = iWaferColumns;
          iFirstDiePosY = iWaferRows;
        }
        else
        {
          // UpperRight
          // Have to find the last Row
          // Add the last die in this row
          if(iFirstDiePosY == -32768)
            iFirstDiePosY = iWaferRows;

          if(iFirstDiePosY == iWaferRows)
            iFirstDiePosX = iWaferColumns;
        }

        // Save PartResult
        if(!m_qMapBins[wHardBin]->bPass)
        {
          m_lstWaferMap.append(new CSemiE142XmlDevice(iWaferColumns, iWaferRows, wHardBin, 8));
          iTotalFailBin++;
          m_qMapBins[wHardBin]->nNbCnt++;
        }
        else
        {
          m_lstWaferMap.append(new CSemiE142XmlDevice(iWaferColumns, iWaferRows, wHardBin, 0));
          m_qMapBins[wHardBin]->nNbCnt++;
          iTotalGoodBin++;
        }
      }

  }

  // Then extract information
  // and write the stdf file
  int			iOffsetX;
  int			iOffsetY;
  int			iOriginPosX;
  int			iOriginPosY;

  iOriginPosX = iOriginPosY = 0;
  iOffsetX = iOffsetY = 0;

  if(m_cOrgPosX == 'C')
  {
    if(m_cPosX == 'R')
      iOriginPosX =iOffsetX = (m_nWaferColumns/2) - m_nWaferColumns;
    else
      iOriginPosX =iOffsetX = (m_nWaferColumns/2);

    if(m_cPosY == 'D')
      iOriginPosY =iOffsetY = (m_nWaferRows/2) - m_nWaferRows;
    else
      iOriginPosY =iOffsetY = (m_nWaferRows/2);
  }
  else
  {

    if(m_cPosX == 'L')
    {
      // Axis to Left
      // iXStep < 0
      // If the OriginLocation is at the Right of the Wafer
      // start to Col else to 0
      if(m_cOrgPosX != m_cPosX)
        iOffsetX = m_nWaferColumns - 1;
    }
    else
    {
      // Axis to Right
      // iXStep > 0
      if(m_cOrgPosX == m_cPosX)
        iOffsetX = -m_nWaferColumns + 1;
    }

    if(m_cPosY == 'U')
    {
      if(m_cOrgPosY != m_cPosY)
        iOffsetY = m_nWaferRows - 1;
    }
    else
    {
      if(m_cOrgPosY == m_cPosY)
        iOffsetY = -m_nWaferRows + 1;
    }
    if(m_cOrgPosX == 'R')
      iOriginPosX = m_nWaferColumns - 1;
    if(m_cOrgPosY == 'D')
      iOriginPosY = m_nWaferRows - 1;
  }

  if(m_nOriginPosX != -32768)
  {
    iOffsetX = (iOriginPosX + m_nOriginPosX);
    iOffsetY = (iOriginPosY + m_nOriginPosY);
  }
  else
  if(m_nFirstRefPosX != -32768)
  {
    iOffsetX = (m_nFirstRefPosX-iFirstDiePosX);
    iOffsetY = (m_nFirstRefPosY-iFirstDiePosY);
  }

  iWaferRows = 0;
  iWaferColumns = 0;

  // Reset counters
  iPartNumber=0;
  // Write WIR of new Wafer.
  RecordReadInfo.iRecordType = 2;
  RecordReadInfo.iRecordSubType = 10;
  StdfFile.WriteHeader(&RecordReadInfo);
  StdfFile.WriteByte(1);							// Test head
  StdfFile.WriteByte(255);						// Tester site (all)
  StdfFile.WriteDword(m_lStartTime);				// Start time
  StdfFile.WriteString(m_strWaferId.toLatin1().constData());	// WaferID
  StdfFile.WriteRecord();


  // Write all Part read on this wafer.: WIR.PIR,PRR, PIR,PRR,   ... WRR
  CSemiE142XmlDevice* pDevice;
  while(!m_lstWaferMap.isEmpty())
  {
    pDevice = m_lstWaferMap.takeFirst();

    iPartNumber++;

    // Write PIR
    RecordReadInfo.iRecordType = 5;
    RecordReadInfo.iRecordSubType = 10;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteByte(1);			// Test head
    StdfFile.WriteByte(1);			// Tester site#:1
    StdfFile.WriteRecord();

    // Write PRR
    RecordReadInfo.iRecordType = 5;
    RecordReadInfo.iRecordSubType = 20;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteByte(1);					// Test head
    StdfFile.WriteByte(1);					// Tester site#:1
    StdfFile.WriteByte(pDevice->m_iFlagResult);		// PART_FLG : FAILED
    StdfFile.WriteWord((WORD)0);			// NUM_TEST
    StdfFile.WriteWord(pDevice->m_iBinResult);         // HARD_BIN
    StdfFile.WriteWord(pDevice->m_iBinResult);         // SOFT_BIN
    StdfFile.WriteWord(pDevice->m_iPosX + iOffsetX);	// X_COORD
    StdfFile.WriteWord(pDevice->m_iPosY + iOffsetY);	// Y_COORD
    StdfFile.WriteDword(0);					// No testing time known...
    StdfFile.WriteString(QString::number(iPartNumber).toLatin1().constData());		// PART_ID
    StdfFile.WriteString("");				// PART_TXT
    StdfFile.WriteString("");				// PART_FIX
    StdfFile.WriteRecord();

    delete pDevice;
  };

  m_lstWaferMap.clear();


  // Write WRR for last wafer inserted
  RecordReadInfo.iRecordType = 2;
  RecordReadInfo.iRecordSubType = 20;
  StdfFile.WriteHeader(&RecordReadInfo);
  StdfFile.WriteByte(1);						// Test head
  StdfFile.WriteByte(255);					// Tester site (all)
  StdfFile.WriteDword(m_lEndTime);			// Time of last part tested
  StdfFile.WriteDword(iPartNumber);			// Parts tested: always 5
  StdfFile.WriteDword(0);						// Parts retested
  StdfFile.WriteDword(0);						// Parts Aborted
  StdfFile.WriteDword(iTotalGoodBin);			// Good Parts
  StdfFile.WriteDword(4294967295UL);			// Functionnal Parts
  StdfFile.WriteString(m_strWaferId.toLatin1().constData());	// WaferID
  StdfFile.WriteRecord();

  // Write WCR for last wafer inserted

  RecordReadInfo.iRecordType = 2;
  RecordReadInfo.iRecordSubType = 30;
  StdfFile.WriteHeader(&RecordReadInfo);
  StdfFile.WriteFloat(0);						// Wafer size
  StdfFile.WriteFloat(m_fWaferSizeX);			// Height of die
  StdfFile.WriteFloat(m_fWaferSizeY);			// Width of die
  StdfFile.WriteByte(m_nWaferUnit);			// Units are in millimeters
  StdfFile.WriteByte((BYTE) m_cWfFlat);	// Orientation of wafer flat
  StdfFile.WriteWord(static_cast<WORD>(-32768));  // CENTER_X
  StdfFile.WriteWord(static_cast<WORD>(-32768));  // CENTER_Y
  StdfFile.WriteByte((BYTE) m_cPosX);		// POS_X
  StdfFile.WriteByte((BYTE) m_cPosY);		// POS_Y
  StdfFile.WriteRecord();

  // Write PCR for last wafer inserted
  RecordReadInfo.iRecordType = 1;
  RecordReadInfo.iRecordSubType = 30;
  StdfFile.WriteHeader(&RecordReadInfo);
  StdfFile.WriteByte(255);						// Test Head = ALL
  StdfFile.WriteByte(1);							// Test sites = ALL
  StdfFile.WriteDword(iPartNumber);			// Parts tested: always 5
  StdfFile.WriteDword(0);						// Parts retested
  StdfFile.WriteDword(0);						// Parts Aborted
  StdfFile.WriteDword(iTotalGoodBin);			// Good Parts
  StdfFile.WriteDword(4294967295UL);			// Functionnal Parts
  StdfFile.WriteRecord();

  RecordReadInfo.iRecordType = 1;
  RecordReadInfo.iRecordSubType = 40;
  QMap<int,CSemiE142XmlBinInfo*>::Iterator itMapBin;
  for ( itMapBin = m_qMapBins.begin(); itMapBin != m_qMapBins.end(); ++itMapBin )
  {
    // Write HBR
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteByte(255);						// Test Head = ALL
    StdfFile.WriteByte(1);							// Test sites = ALL
    StdfFile.WriteWord(itMapBin.value()->nBinNb);	// HBIN = 0
    if(m_bSaveBinCntSummary)
      StdfFile.WriteDword(itMapBin.value()->nNbCntSummary);	// Total Bins
    else
      StdfFile.WriteDword(itMapBin.value()->nNbCnt);	// Total Bins
    if(itMapBin.value()->bPass)
      StdfFile.WriteByte('P');
    else
      StdfFile.WriteByte('F');
    StdfFile.WriteString(itMapBin.value()->strName.toLatin1().constData());
    StdfFile.WriteRecord();
  }

  RecordReadInfo.iRecordType = 1;
  RecordReadInfo.iRecordSubType = 50;

  for ( itMapBin = m_qMapBins.begin(); itMapBin != m_qMapBins.end(); ++itMapBin )
  {
    // Write SBR
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteByte(255);						// Test Head = ALL
    StdfFile.WriteByte(1);							// Test sites = ALL
    StdfFile.WriteWord(itMapBin.value()->nBinNb);	// HBIN = 0
    if(m_bSaveBinCntSummary)
      StdfFile.WriteDword(itMapBin.value()->nNbCntSummary);	// Total Bins
    else
      StdfFile.WriteDword(itMapBin.value()->nNbCnt);	// Total Bins
    if(itMapBin.value()->bPass)
      StdfFile.WriteByte('P');
    else
      StdfFile.WriteByte('F');
    StdfFile.WriteString(itMapBin.value()->strName.toLatin1().constData());
    StdfFile.WriteRecord();
  }


  // Write MRR
  RecordReadInfo.iRecordType = 1;
  RecordReadInfo.iRecordSubType = 20;
  StdfFile.WriteHeader(&RecordReadInfo);
  StdfFile.WriteDword(m_lEndTime);			// File finish-time.
  StdfFile.WriteRecord();

  // Close STDF file.
  StdfFile.Close();

  // Success
  return true;
}

//////////////////////////////////////////////////////////////////////
// Convert 'FileName' SemiE142Xml file, to STDF 'strFileNameSTDF' file
//////////////////////////////////////////////////////////////////////
bool CGSemiE142XmltoSTDF::Convert(const char *SemiE142XmlFileName, const char *strFileNameSTDF)
{
  // No erro (default)
  m_iLastError = errNoError;

  // If STDF file already exists...do not rebuild it...unless dates not matching!
  QFileInfo fInput(SemiE142XmlFileName);
  QFileInfo fOutput(strFileNameSTDF);

    QFile f( strFileNameSTDF );
    if((f.exists() == true) && (fInput.lastModified() < fOutput.lastModified()))
    return true;

  //////////////////////////////////////////////////////////////////////
  // For ProgressBar
  iProgressStep = 0;
  iNextFilePos = 0;

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
      GS::Gex::Engine::GetInstance().UpdateLabelStatus("Converting data from file "+QFileInfo(SemiE142XmlFileName).fileName()+"...");
      GexScriptStatusLabel->show();
    }
  }
  QCoreApplication::processEvents();

  if(ReadSemiE142XmlFile(SemiE142XmlFileName,strFileNameSTDF) != true)
  {
    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    if((GexProgressBar != NULL)
    && bHideProgressAfter)
      GexProgressBar->hide();

    if((GexScriptStatusLabel != NULL)
    && bHideLabelAfter)
      GexScriptStatusLabel->hide();
    return false;	// Error reading SemiE142Xml file
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
// Parse the file and split line between each tags
//////////////////////////////////////////////////////////////////////
QString CGSemiE142XmltoSTDF::ReadNextTag()
{
  m_mapTagAttributes.clear();
  m_strTagValue = "";
  m_strTagName = "";

  if(m_bEndOfFile)
    return "";

  //////////////////////////////////////////////////////////////////////
  // For ProgressBar
  if(GexProgressBar != NULL)
  {

    while((int) m_phSemiE142XmlFile->device()->pos() > iNextFilePos)
    {
      iProgressStep += 100/iFileSize + 1;
      iNextFilePos  += iFileSize/100 + 1;
      GexProgressBar->setValue(iProgressStep);
    }
  }
  QCoreApplication::processEvents();


  int iPos;
  int iSpacePos;
  QString strString;


  while(!m_bEndOfFile)
  {
    if(m_phSemiE142XmlFile->atEnd())
      m_bEndOfFile = m_strCurrentLine.isEmpty();
    else
    {
      if(m_strCurrentLine.isEmpty() || m_strCurrentLine == " ")
        m_strCurrentLine = ReadLine();
    }

    if(m_strCurrentLine.left(1) == "<")
    {
      // save tag name
      m_strCurrentLine = m_strCurrentLine.remove(0,1).trimmed();

      QRegExp space( "\\s" );     // match whitespace

      iPos = space.indexIn(m_strCurrentLine);

      if((iPos > 0) && (m_strCurrentLine.indexOf("\"") > 0) && (m_strCurrentLine.indexOf("\"") < iPos) && (m_strCurrentLine.indexOf("\"",iPos)))
      {
        // have "data space"
        iPos = space.indexIn(m_strCurrentLine,m_strCurrentLine.indexOf("\"",m_strCurrentLine.indexOf("\"")+1));

      }

      // if no space (end of tag or end of line)
      if((m_strCurrentLine.indexOf(">") > -1) && (iPos > m_strCurrentLine.indexOf(">")))
        iPos = m_strCurrentLine.indexOf(">");
      if(iPos < 0)
        iPos = m_strCurrentLine.indexOf(">");
      if(iPos < 0)
        iPos = m_strCurrentLine.length();

      m_strTagName = m_strCurrentLine.left(iPos);
      m_strCurrentLine = m_strCurrentLine.remove(0,iPos).trimmed();

      iPos = m_strCurrentLine.indexOf(">");
      while(iPos < 0)
      {
        // save all attributes
        iSpacePos = space.indexIn(m_strCurrentLine);
        if((iSpacePos > 0) && (m_strCurrentLine.indexOf("\"") > 0) && (m_strCurrentLine.indexOf("\"") < iSpacePos) && (m_strCurrentLine.indexOf("\"",iSpacePos)))
        {
          // have "data space"
          iSpacePos = space.indexIn(m_strCurrentLine,m_strCurrentLine.indexOf("\"",m_strCurrentLine.indexOf("\"")+1));

        }

        // if no space (end of tag or end of line)
        if(iSpacePos < 0)
          iSpacePos = m_strCurrentLine.indexOf(">");
        if(iSpacePos < 0)
          iSpacePos = m_strCurrentLine.length();

        strString = m_strCurrentLine.left(iSpacePos);
        m_strCurrentLine = m_strCurrentLine.remove(0,iSpacePos).trimmed();

        if(!strString.isEmpty())
          m_mapTagAttributes[strString.section("=",0,0).toUpper()] = strString.section("=",1).remove("\"");

        if(m_strCurrentLine.isEmpty() || m_strCurrentLine == " ")
          m_strCurrentLine = ReadLine();

        iPos = m_strCurrentLine.indexOf(">");

        if(m_phSemiE142XmlFile->atEnd())
        {
          m_bEndOfFile = true;
          return "";
        }
      }
      strString = m_strCurrentLine.left(iPos).trimmed();
      m_strCurrentLine = m_strCurrentLine.remove(0,iPos+1).trimmed();
      while(!strString.isEmpty())
      {
        iPos = space.indexIn(strString);
        if((iPos > 0) && (strString.indexOf("\"") > 0) && (strString.indexOf("\"") < iPos) && (strString.indexOf("\"",iPos)))
        {
          // have "data space"
          iPos = space.indexIn(strString,strString.indexOf("\"",strString.indexOf("\"")+1));

        }
        if(iPos < 0)
        {
          m_mapTagAttributes[strString.section("=",0,0).toUpper()] = strString.section("=",1).remove("\"");
          strString = "";
        }
        else
        {
          m_mapTagAttributes[strString.left(iPos).section("=",0,0).toUpper()] = strString.left(iPos).section("=",1).remove("\"");
          strString = strString.mid(iPos).simplified();
        }
      }

      m_strTagValue = "";
      // Then go to the next (END-)tag and save the current value
      while(!m_bEndOfFile)
      {
        if(m_strCurrentLine.isEmpty() || m_strCurrentLine == " ")
          m_strCurrentLine = ReadLine();

        iPos = m_strCurrentLine.indexOf('<');
        if(iPos < 0)
          iPos = m_strCurrentLine.length();

        // save tag value
        strString = m_strCurrentLine.left(iPos).trimmed();
        m_strCurrentLine = m_strCurrentLine.remove(0,iPos).trimmed();
        if(!strString.isEmpty())
          m_strTagValue += strString + " ";

        if(iPos >= 0)
          break;

        if(m_phSemiE142XmlFile->atEnd())
        {
          m_bEndOfFile = true;
          return "";
        }
      }
      return m_strTagName;

    }
    else
    {
      // bad pos ??
      // Try to found next tag
      m_strCurrentLine = "";
    }

  }


  return m_strTagName;


}


//////////////////////////////////////////////////////////////////////
bool CGSemiE142XmltoSTDF::GotoMarker(const char *szEndMarker)
{
  QString strLine;
  QString strString;
  QString strSection;
  QString strEndMarker(szEndMarker);
  strEndMarker = strEndMarker.toUpper();


  while(!m_bEndOfFile)
  {

    strLine = ReadNextTag();
    if(strLine.isEmpty() || strLine == " ")
      continue;

    // if found a 'tag', probably a keyword
    strSection = strLine.toUpper();
    if(strSection == strEndMarker)
      return TRUE;
  }
  return FALSE;
}

//////////////////////////////////////////////////////////////////////
QString CGSemiE142XmltoSTDF::ReadLine()
{
  QString strLine;

  if(m_phSemiE142XmlFile->atEnd())
    return strLine;

  QRegExp spaceEqual( "\\s*=\\s*" );     // match whitespace around the sign '='
  strLine = m_phSemiE142XmlFile->readLine();
  strLine = strLine.trimmed();
  strLine = strLine.replace("/>",">");
  strLine = strLine.replace("?>",">");
  strLine = strLine.replace(spaceEqual,"=");

  return strLine;

}
