//////////////////////////////////////////////////////////////////////
// import_semi_g85_xml.cpp: Convert a SemiG85Xml Xml file to STDF V4.0
//////////////////////////////////////////////////////////////////////
#include <qfileinfo.h>
#include <qprogressbar.h>
#include <qapplication.h>
#include <qlabel.h>
#include <qregexp.h>


#include "xml_parser.h"

#include "import_semi_g85_xml.h"
#include "time.h"
#include "import_constants.h"
#include "engine.h"

//<?xml version="1.0"?>
//<Maps>
//  <Map xmlns:semi="http://www.semi.org" WaferId="1" FormatRevision="G85-1101">
//    <Device ProductId="g92_MLO_eng_pc" LotId="" CreateDate="2009022511532100" SupplierName="" Rows="15" Columns="16" Orientation="0" OriginLocation="3" BinType="Decimal" NullBin="255" >
//      <Bin BinCode="0" BinQuality="Fail" BinCount="116" BinDescription=""/>
//      <Bin BinCode="1" BinQuality="Pass" BinCount="119" BinDescription=""/>
//      <Data MapName="Map" MapVersion="6">
//        <Row><![CDATA[255 255 255 255 255 255 001 000 000 000 255 255 255 255 255 255 ]]></Row>
//        <Row><![CDATA[255 255 255 255 000 001 001 001 001 001 000 001 255 255 255 255 ]]></Row>
//        <Row><![CDATA[255 255 255 000 001 001 001 000 000 000 000 000 001 000 255 255 ]]></Row>
//        <Row><![CDATA[255 255 000 001 001 000 000 000 001 001 001 001 001 001 255 255 ]]></Row>
//        <Row><![CDATA[255 000 000 001 001 001 001 001 000 000 001 001 001 001 000 255 ]]></Row>
//        <Row><![CDATA[255 000 001 001 001 000 000 000 001 001 001 001 001 000 000 000 ]]></Row>
//        <Row><![CDATA[001 001 001 000 001 001 001 001 000 001 000 000 000 001 001 001 ]]></Row>
//        <Row><![CDATA[001 001 001 001 001 001 001 000 000 001 000 000 000 001 001 001 ]]></Row>
//        <Row><![CDATA[001 001 001 001 000 001 000 001 001 001 001 001 000 001 001 000 ]]></Row>
//        <Row><![CDATA[255 000 000 001 001 001 000 001 001 001 001 001 001 000 000 001 ]]></Row>
//        <Row><![CDATA[255 001 001 001 001 000 001 001 000 000 001 000 001 001 000 255 ]]></Row>
//        <Row><![CDATA[255 001 001 001 001 000 001 001 000 001 000 001 000 001 001 255 ]]></Row>
//        <Row><![CDATA[255 255 001 001 001 001 001 001 000 000 001 001 000 000 255 255 ]]></Row>
//        <Row><![CDATA[255 255 255 001 000 001 001 001 001 001 001 001 001 255 255 255 ]]></Row>
//        <Row><![CDATA[255 255 255 255 255 001 000 001 001 000 000 000 255 255 255 255 ]]></Row>
//      </Data>
//    </Device>
//  </Map>
//</Maps>

// main.cpp
extern QLabel			*GexScriptStatusLabel;	// Handle to script status text in status bar
extern QProgressBar	*	GexProgressBar;		// Handle to progress bar in status bar


CSemiG85XmlBinInfo::CSemiG85XmlBinInfo()
{
    nBinNb = 0;
    nNbCntSummary = -1;
    nNbCnt = 0;
    bPass  = false;
};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGSemiG85XmltoSTDF::CGSemiG85XmltoSTDF()
{
    m_lStartTime	= 0;
    m_nWaferRows	= 0;
    m_nWaferColumns = 0;
    m_bWaferCenter	= false;
    m_fWaferSize	= 0;
    m_fWaferSizeX	= 0;
    m_fWaferSizeY	= 0;
    m_nNbRefDies	= 0;
    m_nFirstRefPosX = 0;
    m_nFirstRefPosY = 0;
    m_cWfFlat		= ' ';
    m_cPosX			= 'R';
    m_cPosY			= 'D';
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
CGSemiG85XmltoSTDF::~CGSemiG85XmltoSTDF()
{
    QMap<int,CSemiG85XmlBinInfo*>::Iterator itMapBin;
    for ( itMapBin = m_qMapBins.begin(); itMapBin != m_qMapBins.end(); ++itMapBin )
    {
        delete itMapBin.value();
    }
}

//////////////////////////////////////////////////////////////////////
// Get Error
//////////////////////////////////////////////////////////////////////
QString CGSemiG85XmltoSTDF::GetLastError()
{
    m_strLastError = "Import Semi G85 Xml: ";

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
// Check if File is compatible with SemiG85Xml format
//////////////////////////////////////////////////////////////////////
bool CGSemiG85XmltoSTDF::IsCompatible(const char *szFileName)
{
    XML::XMLParser lXMLManager;

    if(lXMLManager.LoadXMLFile(szFileName) == false)
    {
        return false;
    }

    QString lAttribut = lXMLManager.GetAttribute("Map", "FormatRevision");
    if(lAttribut.indexOf("G85-") >= 0)
        return true;

    return false;
}


//////////////////////////////////////////////////////////////////////
// Read and Parse the SemiG85Xml file
//////////////////////////////////////////////////////////////////////
bool CGSemiG85XmltoSTDF::ReadSemiG85XmlFile(const char* semiG85XmlFileName, const char* strFileNameSTDF)
{
    if(m_XMLManager.LoadXMLFile(semiG85XmlFileName) == false) {
        m_iLastError = errOpenFail;
        return false;
    }

    // -- Map
    m_strWaferId            = m_XMLManager.GetAttribute("Map", "WaferId");
    m_strNodeName           = m_XMLManager.GetAttribute("Map", "FormatRevision");

    // -- Device
    if(!m_XMLManager.HasNode("Device"))
        return false;

    m_strDeviceId           = m_XMLManager.GetAttribute("Device", "ProductId");
    m_strLotId              = m_XMLManager.GetAttribute("Device", "LotId");
    int lNotch              = m_XMLManager.GetAttribute("Device", "Orientation").toInt();

    switch(lNotch)
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

    m_fWaferSize              = m_XMLManager.GetAttribute("Device", "WaferSize").toFloat();
    m_fWaferSizeX             = m_XMLManager.GetAttribute("Device", "DeviceSizeX").toFloat();
    m_fWaferSizeY             = m_XMLManager.GetAttribute("Device", "DeviceSizeY").toFloat();
    m_nWaferRows              = m_XMLManager.GetAttribute("Device", "Rows").toInt();
    m_nWaferColumns           = m_XMLManager.GetAttribute("Device", "Columns").toInt();

    QString lStrValue         = m_XMLManager.GetAttribute("Device", "BinType");
    if(!lStrValue.isEmpty() && lStrValue.startsWith("Hexa",Qt::CaseInsensitive))
        m_nBinType = 16;
    m_strBinNull              = m_XMLManager.GetAttribute("Device", "NullBin");
    m_strOperatorName         = m_XMLManager.GetAttribute("Device", "SupplierName");

    int lOriginLocation       = m_XMLManager.GetAttribute("Device", "OriginLocation").toInt();

    m_bWaferCenter = false;
    if(lOriginLocation == 0) {
        // Center
        m_bWaferCenter = true;
        m_cPosY = 'D';
        m_cPosX = 'R';
    }
    else if(lOriginLocation == 1) {
        // DownLeft
        m_cPosY = 'D';
        m_cPosX = 'L';
    }
    else if(lOriginLocation == 2) {
        // DownRight
        m_cPosY = 'D';
        m_cPosX = 'R';
    }
    else if(lOriginLocation == 3) {
        // toUpperRight
        m_cPosY = 'U';
        m_cPosX = 'R';
    }
    else if(lOriginLocation == 4) {
        // toUpperLeft
        m_cPosY = 'U';
        m_cPosX = 'L';
    }

    lStrValue         = m_XMLManager.GetAttribute("Device", "CreateDate");
    if(!lStrValue.isEmpty()) {
        // yyyymmddhhmisszzz
        // Extract the date
        int iYear   = lStrValue.left(4).toInt();
        int iMonth  = lStrValue.mid(4,2).toInt();
        int iDay    = lStrValue.mid(6,2).toInt();
        // Extract the time
        int iHour   = lStrValue.mid(8,2).toInt();
        int iMin    = lStrValue.mid(10,2).toInt();
        int iSec    = lStrValue.mid(12,2).toInt();

        // Save this date
        QDate       clDate(iYear,iMonth,iDay);
        QTime       clTime(iHour,iMin,iSec);
        QDateTime   clDateTime(clDate,clTime);
        clDateTime.setTimeSpec(Qt::UTC);
        m_lStartTime = clDateTime.toTime_t();
    }

    m_strStatus = m_XMLManager.GetAttribute("Device", "Status");  ;

    // -- Reference Device
    if(m_XMLManager.HasNode("ReferenceDevice") )
    {
        ++m_nNbRefDies;
        m_nFirstRefPosX = m_XMLManager.GetAttribute("ReferenceDevice", "ReferenceDeviceX").toInt();
        m_nFirstRefPosY = m_XMLManager.GetAttribute("ReferenceDevice", "ReferenceDeviceY").toInt();
    }

    // -- Bin
    m_XMLManager.SetRepeatedNodes("Bin");

    while(m_XMLManager.NextRepeatedNode())
    {
        QString   lStrBinNum             = m_XMLManager.GetRepeatedNodeAttribut("BinCode");
        QString   lStrBinName            = m_XMLManager.GetRepeatedNodeAttribut("BinDescription");
        QString   lStrBinCat             = m_XMLManager.GetRepeatedNodeAttribut("BinQuality");
        QString   lStrBinCountSummary    = m_XMLManager.GetRepeatedNodeAttribut("BinCount");

        // Save bin info
        CSemiG85XmlBinInfo *pBin = 0;
        if(!m_qMapBins.contains(lStrBinNum.toInt())) {
            pBin = new CSemiG85XmlBinInfo();
            pBin->nBinNb = lStrBinNum.toInt();
            m_qMapBins[pBin->nBinNb] = pBin;
        }
        else
            pBin = m_qMapBins[lStrBinNum.toInt()];

        if(lStrBinName.isEmpty())
            lStrBinName = "G85Bin("+lStrBinNum+")";
        pBin->strName = lStrBinName;

        if(!lStrBinCountSummary.isEmpty()){
            pBin->nNbCntSummary = lStrBinCountSummary.toInt();
            m_bSaveBinCntSummary = true;
        }

        if(lStrBinCat.startsWith("Pass",Qt::CaseInsensitive) || lStrBinCat.startsWith("Good",Qt::CaseInsensitive))
        {
            pBin->bPass = true;
            m_bHaveGoodBins = true;
        }

    }

    if(!WriteStdfFile(strFileNameSTDF))
    {
        // -- Convertion failed.
        QFile::remove(strFileNameSTDF);
        return false;
    }

    return true;
}

//////////////////////////////////////////////////////////////////////
// Create STDF file from SemiG85Xml data parsed
//////////////////////////////////////////////////////////////////////
bool CGSemiG85XmltoSTDF::WriteStdfFile(const char *strFileNameSTDF)
{
    if(m_XMLManager.GetNbIdenticalNodes("Map") > 1) {
        m_iLastError = errInvalidFormat;
        m_strLastErrorSpecification = "Multi Wafer Map not allowed";

        return false;
    }

    //-- now generate the STDF file...
    GS::StdLib::Stdf StdfFile;
    GS::StdLib::StdfRecordReadInfo RecordReadInfo;
    if(StdfFile.Open((char*)strFileNameSTDF,STDF_WRITE) != GS::StdLib::Stdf::NoError)
    {
        m_iLastError = errWriteSTDF;
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
    StdfFile.WriteByte((BYTE) m_strStatus[0].toLatin1());				// Test Mode = PRODUCTION
    StdfFile.WriteByte((BYTE) ' ');				// rtst_cod
    StdfFile.WriteByte((BYTE) ' ');				// prot_cod
    StdfFile.WriteWord(65535);					// burn_tim
    StdfFile.WriteByte((BYTE) ' ');				// cmod_cod
    StdfFile.WriteString(m_strLotId.toLatin1().constData());		// Lot ID
    StdfFile.WriteString(m_strDeviceId.toLatin1().constData());	// Part Type / Product ID
    StdfFile.WriteString(m_strNodeName.toLatin1().constData());	// Node name
    StdfFile.WriteString("");					// Tester Type
    StdfFile.WriteString("");					// Job name
    StdfFile.WriteString("");					// Job rev
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
    strUserTxt += ":SemiG85/XML";
    StdfFile.WriteString((char *)strUserTxt.toLatin1().constData());		// user-txt
    StdfFile.WriteString("");					// aux-file
    StdfFile.WriteString("");					// package-type
    StdfFile.WriteString("");					// familyID
    StdfFile.WriteString("");					// Date-code
    StdfFile.WriteString("");					// Facility-ID
    StdfFile.WriteString("");					// FloorID
    StdfFile.WriteString("");					// ProcessID
    StdfFile.WriteRecord();

    // Write Test results for each line read.
    QString		strString, strBin, strValue;
    float		fValue = 0.;
    int			iIndex = 0;				// Loop index
    WORD		wHardBin;
    long		iTotalGoodBin = 0,iTotalFailBin =0;
    long		iPartNumber     = 0;
        //FIXME: not used ?
        //bool bPassStatus;
    int			iWaferColumns   = 0;
    int			iWaferRows      = 0;
    int			iOffsetX        = 0;
    int			iOffsetY        = 0;
    int			iStepX          = 0;
    int			iStepY          = 0;

    // Origin Location
    // We need to have the number of rows and the numbers of columns
    // to retrieve the wafer die coordinates.
    // Have to parse first the WaferMap
    // and then dump the STDF format

    m_nWaferRows = 0;
    iStepX = iStepY = 1;

    if(m_cPosX == 'L')
        iStepX = -1;

    if(m_cPosY == 'U')
        iStepY = -1;

    iWaferRows = -iStepY;

    //-- Reset counters
    iTotalGoodBin=iTotalFailBin=0;

    m_XMLManager.SetRepeatedNodes("Row");

    QList<CSemiG85XmlDevice*> lLstWaferMap;
    QRegExp space( "\\s" );     // match whitespace
    while(m_XMLManager.NextRepeatedNode())
    {
        QString lTempStr       = m_XMLManager.GetRepeatedNodeText();
        m_strTagValue           = lTempStr.remove(space);
        //255255255255255255001000000000255255255255255255
        if(m_nWaferBytePerDie == 0) {
            if(m_nWaferColumns > 0) {
                m_nWaferBytePerDie = (int)m_strTagValue.length()/m_nWaferColumns;

                // Check if it's ok
                fValue = (float) m_strTagValue.length() / (float) m_nWaferColumns;
                if(fValue != (float) m_nWaferBytePerDie) {
                    // have an error
                    m_iLastError = errInvalidFormat;
                    m_strLastErrorSpecification = "Inconsistency ![CDATA[ format";
                    return false;
                }
            }
            else if(space.indexIn(strString) > 0) {
                m_nWaferBytePerDie = space.indexIn(strString);
                m_nWaferColumns = m_strTagValue.length()/m_nWaferBytePerDie;
            }
            else if(!m_strBinNull.isEmpty()) {
                m_nWaferBytePerDie = m_strBinNull.length();
                m_nWaferColumns = m_strTagValue.length()/m_nWaferBytePerDie;
            }
            else {
                m_nWaferBytePerDie = 1;
                m_nWaferColumns = m_strTagValue.length()/m_nWaferBytePerDie;
            }
        }

        ++m_nWaferRows;

        iWaferRows+= iStepY;
        iWaferColumns = -iStepX;

        //-- Read Part results for this record
        for(iIndex=0;iIndex<m_nWaferColumns;++iIndex) {
            iWaferColumns+= iStepX;

            strBin = m_strTagValue.mid(iIndex*m_nWaferBytePerDie,m_nWaferBytePerDie);
            if((!m_strBinNull.isEmpty()) && (strBin.startsWith(m_strBinNull)))
            {
                // Part not tested
                continue;
            }

            //-- Have to convert in decimal
            bool ok;
            wHardBin = strBin.toInt(&ok,m_nBinType);

            if(!m_qMapBins.contains(wHardBin))
            {
                CSemiG85XmlBinInfo *pBin = new CSemiG85XmlBinInfo();
                pBin->nBinNb = wHardBin;
                pBin->strName = "G85Bin("+strBin+")";
                if(m_bHaveGoodBins)
                    pBin->bPass = false;
                else {
                    pBin->bPass = (wHardBin == 1);
                    m_bHaveGoodBins = pBin->bPass;
                }
                m_qMapBins[pBin->nBinNb] = pBin;
                m_bSaveBinCntSummary = false;
            }

            ++iPartNumber;

            if(!m_qMapBins[wHardBin]->bPass) {
                lLstWaferMap.append(new CSemiG85XmlDevice(iWaferColumns, iWaferRows, wHardBin, 8));
                ++iTotalFailBin;
            }
            else {
                lLstWaferMap.append(new CSemiG85XmlDevice(iWaferColumns, iWaferRows, wHardBin, 0));
                ++iTotalGoodBin;
            }

            m_qMapBins[wHardBin]->nNbCnt++;
        };

    }

    // -- Then extract information
    // -- and write the stdf file
    iOffsetX = iOffsetY = 0;

    if(m_bWaferCenter) {
        iOffsetX = -(m_nWaferColumns/2);
        iOffsetY = -(m_nWaferRows/2);
    }
    else {
        if(m_cPosX == 'L')
            iOffsetX = m_nWaferColumns - 1;

        if(m_cPosY == 'U')
            iOffsetY = m_nWaferRows - 1;
    }

    // -- Reset counters
    iPartNumber=0;

    // -- Write WIR of new Wafer.
    RecordReadInfo.iRecordType = 2;
    RecordReadInfo.iRecordSubType = 10;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteByte(1);							// Test head
    StdfFile.WriteByte(255);						// Tester site (all)
    StdfFile.WriteDword(m_lStartTime);							// Start time
    StdfFile.WriteString(m_strWaferId.toLatin1().constData());	// WaferID
    StdfFile.WriteRecord();

    // Write all Part read on this wafer.: WIR.PIR,PRR, PIR,PRR,   ... WRR
    CSemiG85XmlDevice* pDevice;
    while(!lLstWaferMap.isEmpty())
    {
        pDevice = lLstWaferMap.takeFirst();

        ++iPartNumber;

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
        StdfFile.WriteByte(pDevice->m_iFlagResult);				// PART_FLG : FAILED
        StdfFile.WriteWord((WORD)0);							// NUM_TEST
        StdfFile.WriteWord(pDevice->m_iBinResult);				// HARD_BIN
        StdfFile.WriteWord(pDevice->m_iBinResult);				// SOFT_BIN
        StdfFile.WriteWord(pDevice->m_iPosX + iOffsetX);		// X_COORD
        StdfFile.WriteWord(pDevice->m_iPosY + iOffsetY);			// Y_COORD
        StdfFile.WriteDword(0);					// No testing time known...
        StdfFile.WriteString(QString::number(iPartNumber).toLatin1().constData());		// PART_ID
        StdfFile.WriteString("");				// PART_TXT
        StdfFile.WriteString("");				// PART_FIX
        StdfFile.WriteRecord();

        delete pDevice;
    };			// -- Read all lines with valid data records in file

    // Write WRR for last wafer inserted
    RecordReadInfo.iRecordType = 2;
    RecordReadInfo.iRecordSubType = 20;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteByte(1);						// Test head
    StdfFile.WriteByte(255);					// Tester site (all)
    StdfFile.WriteDword(0);						// Time of last part tested
    StdfFile.WriteDword(iPartNumber);			// Parts tested: always 5
    StdfFile.WriteDword(0);						// Parts retested
    StdfFile.WriteDword(0);						// Parts Aborted
    StdfFile.WriteDword(iTotalGoodBin);			// Good Parts
    StdfFile.WriteDword(4294967295UL);			// Functionnal Parts
    StdfFile.WriteString(m_strWaferId.toLatin1().constData());	// WaferID
    StdfFile.WriteRecord();

    // -- Write WCR for last wafer inserted

    RecordReadInfo.iRecordType = 2;
    RecordReadInfo.iRecordSubType = 30;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteFloat(m_fWaferSize);				// Wafer size
    StdfFile.WriteFloat(m_fWaferSizeX);				// Height of die
    StdfFile.WriteFloat(m_fWaferSizeY);				// Width of die
    StdfFile.WriteByte(3);					// Units are in millimeters
    StdfFile.WriteByte((BYTE) m_cWfFlat);	// Orientation of wafer flat
    StdfFile.WriteWord(static_cast<WORD>(-32768));  // CENTER_X
    StdfFile.WriteWord(static_cast<WORD>(-32768));  // CENTER_Y
    StdfFile.WriteByte((BYTE) m_cPosX);		// POS_X
    StdfFile.WriteByte((BYTE) m_cPosY);		// POS_Y
    StdfFile.WriteRecord();

    // -- Write PCR for last wafer inserted
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
    QMap<int,CSemiG85XmlBinInfo*>::Iterator itMapBin;
    for ( itMapBin = m_qMapBins.begin(); itMapBin != m_qMapBins.end(); ++itMapBin )
    {
        // -- Write HBR
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

    for ( itMapBin = m_qMapBins.begin(); itMapBin != m_qMapBins.end(); ++itMapBin ) {
        // -- Write SBR
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

    // -- Write MRR
    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 20;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteDword(0);			// File finish-time.
    StdfFile.WriteRecord();

    // -- Close STDF file.
    StdfFile.Close();

    return true;
}

void CGSemiG85XmltoSTDF::Notify()
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

//////////////////////////////////////////////////////////////////////
// Convert 'FileName' SemiG85Xml file, to STDF 'strFileNameSTDF' file
//////////////////////////////////////////////////////////////////////
bool CGSemiG85XmltoSTDF::Convert(const char *SemiG85XmlFileName, const char *strFileNameSTDF)
{
    // No erro (default)
    m_iLastError = errNoError;

    // If STDF file already exists...do not rebuild it...unless dates not matching!
    QFileInfo fInput(SemiG85XmlFileName);
    QFileInfo fOutput(strFileNameSTDF);

    QFile f( strFileNameSTDF );
    if((f.exists() == true) && (fInput.lastModified() < fOutput.lastModified()))
        return true;

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
            GS::Gex::Engine::GetInstance().UpdateLabelStatus("Converting data from file "+QFileInfo(SemiG85XmlFileName).fileName()+"...");
            GexScriptStatusLabel->show();
        }
    }
    QCoreApplication::processEvents();

    if(ReadSemiG85XmlFile(SemiG85XmlFileName,strFileNameSTDF) != true)
    {
        //////////////////////////////////////////////////////////////////////
        // For ProgressBar
        if((GexProgressBar != NULL)
        && bHideProgressAfter)
            GexProgressBar->hide();

        if((GexScriptStatusLabel != NULL)
        && bHideLabelAfter)
            GexScriptStatusLabel->hide();
        return false;	// Error reading SemiG85Xml file
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
