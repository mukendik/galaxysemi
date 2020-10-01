//////////////////////////////////////////////////////////////////////
// import_kla_wafer_xml.cpp: Convert a Kla Xml file to STDF V4.0
//////////////////////////////////////////////////////////////////////
#include <qfileinfo.h>
#include <qprogressbar.h>
#include <qapplication.h>
#include <qlabel.h>
#include <qregexp.h>


#include "import_kla_wafer_xml.h"
#include "time.h"
#include "import_constants.h"
#include "engine.h"

//<ADB_DOCUMENT
//DataSource="SORT_RAW"
//FormatVersion="1.1"
//>
//<SORT_WAFER_RUN
//Lot="KNK40084.1"
//SourceLot="KNK40084.1"
//WaferNumber="5"
//TestType="CP1"
//MeasureTime="2008/01/19_19:34:51"
//Fab="SIL01"
//Product="KN02B"
//NumOfVisualInspDies=""
//>
//<WAFER_MAP
//FlatOrientation="DOWN"
//UpperLeftXY="16,27"
//LowerRightXY="-10,-1"
//Columns="27"
//Rows="29"
//BytePerDie="2"
//NoValueDie="M"
//NoDie="-"
//>
//<ROW>--------------------MMMMMMMMMMMMMM--------------------</ROW>
//<ROW>----------------MM050601010101010105MMMM--------------</ROW>
//<ROW>------------MM01010101010101010101010105MM------------</ROW>

// main.cpp
extern QLabel			*GexScriptStatusLabel;	// Handle to script status text in status bar
extern QProgressBar	*	GexProgressBar;		// Handle to progress bar in status bar


CKlaWaferXmlBinInfo::CKlaWaferXmlBinInfo()
{
    nBinNb = 0;
    nNbCnt = 0;
    bPass  = false;
};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGKlaWaferXmltoSTDF::CGKlaWaferXmltoSTDF()
{
    m_lStartTime = 0;
    m_nWaferRows = 0;
    m_nWaferColumns = 0;
    m_strGoodBin = "01";
    m_nWaferRows = 0;
    m_nWaferColumns = 0;
    m_cWfFlat = 'D';
    m_nWaferBytePerDie = 1;

    m_bEndOfFile = false;
    m_bBinDefInside = false;
}

//////////////////////////////////////////////////////////////////////
// Destruction
//////////////////////////////////////////////////////////////////////
CGKlaWaferXmltoSTDF::~CGKlaWaferXmltoSTDF()
{
    QMap<int,CKlaWaferXmlBinInfo*>::Iterator itMapBin;
    for ( itMapBin = m_qMapBins.begin(); itMapBin != m_qMapBins.end(); ++itMapBin )
    {
        delete itMapBin.value();
    }
}

//////////////////////////////////////////////////////////////////////
// Get Error
//////////////////////////////////////////////////////////////////////
QString CGKlaWaferXmltoSTDF::GetLastError()
{
    m_strLastError = "Import Kla Wafer Xml: ";

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
// Check if File is compatible with KlaWaferXml format
//////////////////////////////////////////////////////////////////////
bool CGKlaWaferXmltoSTDF::IsCompatible(const char *szFileName)
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
    QTextStream hKlaWaferXmlFile(&f);

    do
        strSection = hKlaWaferXmlFile.readLine().simplified();
    while(!strSection.isNull() && strSection.isEmpty());

    strValue = strSection.section("<",1).trimmed();
    if(strValue.startsWith("ADB_DOCUMENT", Qt::CaseInsensitive))
    {
        strSection = hKlaWaferXmlFile.readLine();
        strValue = strSection.section("=",1).remove('"').simplified();
        if(strValue.startsWith("SORT_RAW", Qt::CaseInsensitive))
        {
            f.close();
            return true;
        }
    }

    f.close();
    return false;
}

//////////////////////////////////////////////////////////////////////
// Read and Parse the KlaWaferXml file
//////////////////////////////////////////////////////////////////////
bool CGKlaWaferXmltoSTDF::ReadKlaWaferXmlFile(const char* KlaWaferXmlFileName,
                                              const char* strFileNameSTDF)
{
    QString strString;
    QString strSection;
    QString strValue;
    bool	bHaveBinPass;

    m_bHaveBinSum = false;
    bHaveBinPass = false;

    // Open file
    QFile f( KlaWaferXmlFileName );
    m_pfKlaFile = &f;
    if(!f.open( QIODevice::ReadOnly ))
    {
        // Failed Opening KlaWaferXml file
        m_iLastError = errOpenFail;

        // Convertion failed.
        return false;
    }

    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    iProgressStep = 0;
    iNextFilePos = 0;
    iFileSize = f.size() + 1;
    QCoreApplication::processEvents();

    // Assign file I/O stream
    QTextStream hKlaWaferXmlFile(&f);
    m_phKlaWaferXmlFile = &hKlaWaferXmlFile;

    while(!m_bEndOfFile)
    {
        strSection = ReadNextTag().toUpper();

        if(strSection.startsWith("/"))
        {
            // Ignore this tag
            continue;
        }
        else
        if(strSection == "ADB_DOCUMENT")
        {
            continue;
        }
        else if(strSection == "SORT_WAFER_RUN")
        {
            // Save all Attributes
            QMap<QString,QString>::Iterator itAttribute;
            QString strAttribute, strValue;
            for(itAttribute = m_mapTagAttributes.begin(); itAttribute != m_mapTagAttributes.end(); itAttribute++)
            {
                strAttribute = itAttribute.key().toUpper();
                strValue = itAttribute.value();

                if(strAttribute == "LOT")
                    m_strLotId = strValue;
                else
                if(strAttribute == "WAFERNUMBER")
                    m_strWaferId = strValue;
                else
                if(strAttribute == "MEASURETIME")
                {
                    // yyyy/mm/dd_hh24:mi:ss
                    QString strBuffer;
                    int iYear, iMonth, iDay, iHour, iMin, iSec;
                    // Extract the date
                    strBuffer = strValue.section("_",0,0);
                    iYear = strBuffer.section("/",0,0).toInt();
                    iMonth = strBuffer.section("/",1,1).toInt();
                    iDay = strBuffer.section("/",2).toInt();
                    // Extract the time
                    strBuffer = strValue.section("_",1);
                    iHour = strBuffer.section(":",0,0).toInt();
                    iMin = strBuffer.section(":",1,1).toInt();
                    iSec = strBuffer.section(":",2).toInt();

                    // Save this date
                    QDate clDate(iYear,iMonth,iDay);
                    QTime clTime(iHour,iMin,iSec);
                    QDateTime clDateTime(clDate,clTime);
                    clDateTime.setTimeSpec(Qt::UTC);
                    m_lStartTime = clDateTime.toTime_t();
                }
                else
                if(strAttribute == "PRODUCT")
                    m_strDeviceId = strValue;
                else
                if(strAttribute == "OPERATOR")
                    m_strOperatorName = strValue;
                else
                if(strAttribute == "TESTPROGRAM")
                    m_strJobName = strValue;
                else
                if(strAttribute == "PROBECARD")
                    m_strHandId = strValue;
                else
                if(strAttribute == "LOADBOARD")
                    m_strLoadId = strValue;
                else
                if(strAttribute == "TESTTYPE")
                    m_strTstrType = strValue;
                else
                if(strAttribute == "EQUIPMENT")
                    m_strEquipment = strValue;
                else
                if(strAttribute == "PROBECARD")
                    m_strProbId = strValue;
                else
                if(strAttribute == "FAB")
                    m_strProcessId = strValue;
                else
                if(strAttribute == "STATION")
                    m_strStatNum = strValue;
                else
                if(strAttribute == "SLOT")
                    m_strSBLot = strValue;
                else
                if(strAttribute == "WAFERSIZE")
                    m_strWaferSize = strValue;
                else
                if(strAttribute == "DIESIZE")
                    m_strDieSize = strValue;
                else
                if(strAttribute == "GROSSDIES")
                    m_strGrossDies = strValue;
                else
                if(strAttribute == "BINDEFINSIDE")
                    m_bBinDefInside = strValue.startsWith("Y", Qt::CaseInsensitive);

            }
        }
        else if(strSection == "WAFER_MAP")
        {
            // Save all Attributes
            QMap<QString,QString>::Iterator itAttribute;
            QString strAttribute, strValue;
            for(itAttribute = m_mapTagAttributes.begin(); itAttribute != m_mapTagAttributes.end(); itAttribute++)
            {
                strAttribute = itAttribute.key().toUpper();
                strValue = itAttribute.value();

                if(strAttribute == "FLATORIENTATION")
                {
                    m_cWfFlat = (char) strValue.toUpper()[0].toLatin1();
                }
                else
                if(strAttribute == "COLUMNS")
                    m_nWaferColumns = strValue.toInt();
                else
                if(strAttribute == "ROWS")
                    m_nWaferRows = strValue.toInt();
                else
                if(strAttribute == "BYTEPERDIE")
                    m_nWaferBytePerDie = strValue.toInt();
                else
                if(strAttribute == "NOVALUEDIE")
                    m_strWaferNoValueDie = strValue;
                else
                if(strAttribute == "NODIE")
                    m_strWaferNoDie = strValue;
                else
                if(strAttribute == "UPPERLEFTXY")
                    m_strUpperLeftXY = strValue;
                else
                if(strAttribute == "LOWERRIGHTXY")
                    m_strLowerRightXY = strValue;
            }
            if(m_bBinDefInside)
            {
                if(!GotoMarker("/WAFER_MAP"))
                {
                    m_iLastError = errInvalidFormat;
                    // Convertion failed.
                    // Close file
                    f.close();
                    return false;
                }
            }
            else
                break;

        }
        else if(strSection == "BIN_SUMMARY")
        {
            m_bHaveBinSum = true;

        }
        else if(strSection == "QTY")
        {
            // Save bin info
            CKlaWaferXmlBinInfo *pBin;
            strValue = m_strTagValue.remove('"');
            if(!m_qMapBins.contains(strValue.section(",",0,0).simplified().toInt()))
            {
                pBin = new CKlaWaferXmlBinInfo();
                pBin->nBinNb = strValue.section(",",0,0).simplified().toInt();
                pBin->bPass = false;
                m_qMapBins[pBin->nBinNb] = pBin;
            }
            else
                pBin = m_qMapBins[strValue.section(",",0,0).simplified().toInt()];

            pBin->nNbCnt = strValue.section(",",1,1).simplified().toInt();

            if(!GotoMarker("/QTY"))
            {
                m_iLastError = errInvalidFormat;
                // Close file
                f.close();
                return false;
            }
        }
        else if(strSection == "BIN_DEFINITION")
        {
            if(!m_bHaveBinSum)
                m_qMapBins.clear();
        }
        else if(strSection == "DEF")
        {
            // Save bin info
            CKlaWaferXmlBinInfo *pBin;
            strValue = m_strTagValue.remove('"');
            if(!m_qMapBins.contains(strValue.section(",",0,0).simplified().toInt()))
            {
                pBin = new CKlaWaferXmlBinInfo();
                pBin->nBinNb = strValue.section(",",0,0).simplified().toInt();
                pBin->nNbCnt = 0;
                m_qMapBins[pBin->nBinNb] = pBin;
            }
            else
                pBin = m_qMapBins[strValue.section(",",0,0).simplified().toInt()];

            pBin->strName = strValue.section(",",2,2).simplified();
            pBin->bPass = strValue.section(",",3).startsWith("B", Qt::CaseInsensitive);
            if(pBin->bPass)
                bHaveBinPass = true;

            if(!GotoMarker("/DEF"))
            {
                m_iLastError = errInvalidFormat;
                // Close file
                f.close();
                return false;
            }
        }
        else
        if(!m_strTagName.isEmpty())
        {
            // Ignore this tag
            if(!GotoMarker(QString("/"+m_strTagName).toLatin1().constData()))
            {
                m_iLastError = errInvalidFormat;
                // Convertion failed.
                // Close file
                f.close();
                return false;
            }
        }
    }


    if(m_bBinDefInside)
    {
        if((!bHaveBinPass)
        && (m_qMapBins.contains(1)))
            m_qMapBins[1]->bPass = true;

        // have a map to save
        // write STDF file
        // Loop reading file until end is reached & generate STDF file dynamically.
        //Restart at the beggining of the file
        hKlaWaferXmlFile.seek(0);

        m_bEndOfFile = false;
        // write STDF file
        if(!GotoMarker("WAFER_MAP"))
        {
            m_iLastError = errInvalidFormat;
            // Convertion failed.
            // Close file
            f.close();
            return false;
        }
    }


    if(!WriteStdfFile(&hKlaWaferXmlFile,strFileNameSTDF))
    {
        // Convertion failed.
        QFile::remove(strFileNameSTDF);
        // Close file
        f.close();
        return false;
    }

    // Close file
    f.close();

    return true;
}

//////////////////////////////////////////////////////////////////////
// Create STDF file from KlaWaferXml data parsed
//////////////////////////////////////////////////////////////////////
bool CGKlaWaferXmltoSTDF::WriteStdfFile(QTextStream *hKlaWaferXmlFile, const char *strFileNameSTDF)
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
    StdfFile.WriteByte(m_strStatNum.toInt());	// Station
    StdfFile.WriteByte((BYTE) 'P');				// Test Mode = PRODUCTION
    StdfFile.WriteByte((BYTE) ' ');				// rtst_cod
    StdfFile.WriteByte((BYTE) ' ');				// prot_cod
    StdfFile.WriteWord(65535);					// burn_tim
    StdfFile.WriteByte((BYTE) ' ');				// cmod_cod
    StdfFile.WriteString(m_strLotId.toLatin1().constData());		// Lot ID
    StdfFile.WriteString(m_strDeviceId.toLatin1().constData());	// Part Type / Product ID
    StdfFile.WriteString("");					// Node name
    StdfFile.WriteString(m_strTstrType.toLatin1().constData());	// Tester Type
    StdfFile.WriteString(m_strJobName.toLatin1().constData());		// Job name
    StdfFile.WriteString("");					// Job rev
    StdfFile.WriteString(m_strSBLot.toLatin1().constData());		// sublot-id
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
    strUserTxt += ":KlaWafer/XML";
    StdfFile.WriteString(strUserTxt.toLatin1().constData());		// user-txt
    StdfFile.WriteString("");					// aux-file
    StdfFile.WriteString("");					// package-type
    StdfFile.WriteString("");					// familyID
    StdfFile.WriteString("");					// Date-code
    StdfFile.WriteString("");					// Facility-ID
    StdfFile.WriteString("");					// FloorID
    StdfFile.WriteString(m_strProcessId.toLatin1().constData());	// ProcessID
    StdfFile.WriteRecord();

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
    StdfFile.WriteString(m_strHandId.toLatin1().constData());		// HAND_ID: Handler/prober name
    StdfFile.WriteString("");				// CARD_TYP: Probe card type
    StdfFile.WriteString(m_strProbId.toLatin1().constData());				// CARD_ID: Probe card name
    StdfFile.WriteString("");				// LOAD_TYP: Load board type
    StdfFile.WriteString(m_strLoadId.toLatin1().constData());		// LOAD_ID: Load board name
    StdfFile.WriteString("");				// DIB_TYP: DIB board type
    StdfFile.WriteString("");				// DIB_ID: DIB board name
    StdfFile.WriteString("");				// CABL_TYP: Interface cable type
    StdfFile.WriteString("");				// CABL_ID: Interface cable name
    StdfFile.WriteString("");				// CONT_TYP: Handler contactor type
    StdfFile.WriteString("");				// CONT_ID: Handler contactor name
    StdfFile.WriteString("");				// LASR_TYP: Laser type
    StdfFile.WriteString("");				// LASR_ID: Laser name
    StdfFile.WriteString("");				// EXTR_TYP: Extra equipment type
    StdfFile.WriteString(m_strEquipment.toLatin1().constData());				// EXTR_ID: Extra equipment name
    StdfFile.WriteRecord();

    // Write Test results for each line read.
    QString		strString, strBin, strValue;
    int			iIndex;				// Loop index
    WORD		wHardBin;
    long		iTotalGoodBin,iTotalFailBin;
    long		iPartNumber;
  //FIXME: not used ?
  //bool bPassStatus;
    int			iWaferRows;
    int			iWaferUpperLeftX;
    int			iWaferUpperLeftY;

    iWaferUpperLeftX = m_strUpperLeftXY.section(",",0,0).toInt();
    iWaferUpperLeftY = m_strUpperLeftXY.section(",",1).toInt();

    // Retrieve the correct coordonate
    int		iULX=0, iULY=0, iDRX=0, iDRY=0;
    char	cPosX='R', cPosY='D';
    if(!m_strUpperLeftXY.isEmpty() && !m_strLowerRightXY.isEmpty())
    {
        iULX = m_strUpperLeftXY.section(",",0,0).toInt();
        iULY = m_strUpperLeftXY.section(",",1).toInt();
        iDRX = m_strLowerRightXY.section(",",0,0).toInt();
        iDRY = m_strLowerRightXY.section(",",1).toInt();

        // Compute POS_X, POS_Y
        if(iULX > iDRX)
            cPosX = 'L';
        if(iULY > iDRY)
            cPosY = 'U';
    }

    int			iStepX, iStepY;
    iStepX = (iDRX - iULX) / (m_nWaferColumns - 1);
    iStepY = (iDRY - iULY) / (m_nWaferRows - 1);

    if(iStepX == 0) iStepX=1;
    if(iStepY == 0) iStepY=1;



    // Reset counters
    iTotalGoodBin=iTotalFailBin=0;
    iPartNumber=0;

    // Write WIR of new Wafer.
    RecordReadInfo.iRecordType = 2;
    RecordReadInfo.iRecordSubType = 10;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteByte(1);							// Test head
    StdfFile.WriteByte(255);						// Tester site (all)
    StdfFile.WriteDword(0);							// Start time
    StdfFile.WriteString(m_strWaferId.toLatin1().constData());	// WaferID
    StdfFile.WriteRecord();

    iWaferRows = -1;
    iULY = iWaferUpperLeftY - iStepY;
    // Write all Part read on this wafer.: WIR.PIR,PRR, PIR,PRR,   ... WRR
    while(hKlaWaferXmlFile->atEnd() == false)
    {


        // Read line
        strString = ReadNextTag().toUpper();
        if(strString == "/WAFER_MAP")
        {
            break;
        }
        else
        if(strString != "ROW")
        {
            break;
        }

        iWaferRows++;
        iULY = iULY + iStepY;
        iULX = iWaferUpperLeftX - iStepX;


        // Reset Pass/Fail flag.
    //FIXME: not used ?
    //bPassStatus = true;

        // Read Part results for this record
        for(iIndex=0;iIndex<m_nWaferColumns;iIndex++)
        {
            strBin = m_strTagValue.mid(iIndex*m_nWaferBytePerDie,m_nWaferBytePerDie);
            wHardBin = strBin.toInt();
            iULX = iULX + iStepX;

            if(strBin.startsWith(m_strWaferNoDie))
                continue;

            if(strBin.startsWith(m_strWaferNoValueDie))
            {
                // Part not tested
                continue;
            }

            if(!m_qMapBins.contains(wHardBin))
            {
                CKlaWaferXmlBinInfo *pBin = new CKlaWaferXmlBinInfo();
                pBin->nBinNb = wHardBin;
                pBin->nNbCnt = 0;
                pBin->strName = "";
                pBin->bPass = (wHardBin == 1);
                m_qMapBins[pBin->nBinNb] = pBin;
            }

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
            if(!m_qMapBins[wHardBin]->bPass)
            {
                StdfFile.WriteByte(8);				// PART_FLG : FAILED
                iTotalFailBin++;
                if(!m_bHaveBinSum)
                    m_qMapBins[wHardBin]->nNbCnt++;
            }
            else
            {
                StdfFile.WriteByte(0);				// PART_FLG : PASSED
                if(!m_bHaveBinSum)
                    m_qMapBins[wHardBin]->nNbCnt++;
                iTotalGoodBin++;
            }
            StdfFile.WriteWord((WORD)0);			// NUM_TEST
            StdfFile.WriteWord(wHardBin);           // HARD_BIN
            StdfFile.WriteWord(wHardBin);           // SOFT_BIN
            //StdfFile.WriteWord(iIndex + iWaferUpperLeftX);				// X_COORD
            //StdfFile.WriteWord(iWaferRows + iWaferUpperLeftY);			// Y_COORD
            StdfFile.WriteWord(iULX);				// X_COORD
            StdfFile.WriteWord(iULY);				// Y_COORD
            StdfFile.WriteDword(0);					// No testing time known...
            StdfFile.WriteString(QString::number(iPartNumber).toLatin1().constData());		// PART_ID
            StdfFile.WriteString("");				// PART_TXT
            StdfFile.WriteString("");				// PART_FIX
            StdfFile.WriteRecord();
        };

        if(!GotoMarker("/ROW"))
        {
            m_iLastError = errInvalidFormat;
            return false;
        }
    };			// Read all lines with valid data records in file

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

    // Write WCR for last wafer inserted
    float fValue;


    RecordReadInfo.iRecordType = 2;
    RecordReadInfo.iRecordSubType = 30;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteFloat(0);						// Wafer size
    fValue = m_strWaferSize.section(",",0,0).toFloat();
    StdfFile.WriteFloat(fValue);				// Height of die
    fValue = m_strWaferSize.section(",",1).toFloat();
    StdfFile.WriteFloat(fValue);				// Width of die
    StdfFile.WriteByte(3);						// Units are in millimeters
    StdfFile.WriteByte((BYTE) m_cWfFlat);		// Orientation of wafer flat
    StdfFile.WriteWord(0);						// CENTER_X
    StdfFile.WriteWord(0);						// CENTER_Y
    StdfFile.WriteByte((BYTE)cPosX);			// POS_X
    StdfFile.WriteByte((BYTE)cPosY);			// POS_Y
    StdfFile.WriteRecord();

    // Write PCR for last wafer inserted
    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 30;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteByte(255);					// Test Head = ALL
    StdfFile.WriteByte(1);						// Test sites = ALL
    StdfFile.WriteDword(iPartNumber);			// Parts tested: always 5
    StdfFile.WriteDword(0);						// Parts retested
    StdfFile.WriteDword(0);						// Parts Aborted
    StdfFile.WriteDword(iTotalGoodBin);			// Good Parts
    StdfFile.WriteDword(4294967295UL);			// Functionnal Parts
    StdfFile.WriteRecord();

    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 40;
    QMap<int,CKlaWaferXmlBinInfo*>::Iterator itMapBin;
    for ( itMapBin = m_qMapBins.begin(); itMapBin != m_qMapBins.end(); ++itMapBin )
    {
        // Write HBR
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte(255);						// Test Head = ALL
        StdfFile.WriteByte(1);							// Test sites = ALL
        StdfFile.WriteWord(itMapBin.value()->nBinNb);	// HBIN = 0
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
    StdfFile.WriteDword(0);			// File finish-time.
    StdfFile.WriteRecord();

    // Close STDF file.
    StdfFile.Close();

    // Success
    return true;
}

//////////////////////////////////////////////////////////////////////
// Convert 'FileName' KlaWaferXml file, to STDF 'strFileNameSTDF' file
//////////////////////////////////////////////////////////////////////
bool CGKlaWaferXmltoSTDF::Convert(const char *KlaWaferXmlFileName, const char *strFileNameSTDF)
{
    // No erro (default)
    m_iLastError = errNoError;

    // If STDF file already exists...do not rebuild it...unless dates not matching!
    QFileInfo fInput(KlaWaferXmlFileName);
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
            GS::Gex::Engine::GetInstance().UpdateLabelStatus("Converting data from file "+QFileInfo(KlaWaferXmlFileName).fileName()+"...");
            GexScriptStatusLabel->show();
        }
    }
    QCoreApplication::processEvents();

    if(ReadKlaWaferXmlFile(KlaWaferXmlFileName,strFileNameSTDF) != true)
    {
        //////////////////////////////////////////////////////////////////////
        // For ProgressBar
        if((GexProgressBar != NULL)
        && bHideProgressAfter)
            GexProgressBar->hide();

        if((GexScriptStatusLabel != NULL)
        && bHideLabelAfter)
            GexScriptStatusLabel->hide();
        return false;	// Error reading KlaWaferXml file
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
QString CGKlaWaferXmltoSTDF::ReadNextTag()
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
        while((int) m_pfKlaFile->pos() > iNextFilePos)
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
        if(m_phKlaWaferXmlFile->atEnd())
            m_bEndOfFile = m_strCurrentLine.isEmpty();
        else
        {
            if(m_strCurrentLine.isEmpty() || m_strCurrentLine == " ")
                m_strCurrentLine = m_phKlaWaferXmlFile->readLine().trimmed();
        }

        if(m_strCurrentLine.left(1) == "<")
        {
            // save tag name
            m_strCurrentLine = m_strCurrentLine.remove(0,1).trimmed();

            QRegExp space( "\\s" );     // match whitespace
            iPos = space.indexIn(m_strCurrentLine);

            // if no space (end of tag or end of line)
            if((iPos < 0)
            ||((m_strCurrentLine.indexOf('>') > 0) && (m_strCurrentLine.indexOf('>') < iPos)))
                iPos = m_strCurrentLine.indexOf('>');
            if(iPos < 0)
                iPos = m_strCurrentLine.length();

            m_strTagName = m_strCurrentLine.left(iPos);
            m_strCurrentLine = m_strCurrentLine.remove(0,iPos).trimmed();

            iPos = m_strCurrentLine.indexOf('>');
            while(iPos < 0)
            {
                // save all attributes
                iSpacePos = space.indexIn(m_strCurrentLine);
                // if no space (end of tag or end of line)
                if(iSpacePos < 0)
                    iSpacePos = m_strCurrentLine.indexOf('>');
                if(iSpacePos < 0)
                    iSpacePos = m_strCurrentLine.length();

                strString = m_strCurrentLine.left(iSpacePos);
                m_strCurrentLine = m_strCurrentLine.remove(0,iSpacePos).trimmed();

                if(!strString.isEmpty())
                    m_mapTagAttributes[strString.section("=",0,0)] = strString.section("=",1).remove("\"");

                if(m_strCurrentLine.isEmpty() || m_strCurrentLine == " ")
                    m_strCurrentLine = m_phKlaWaferXmlFile->readLine().trimmed();
                iPos = m_strCurrentLine.indexOf('>');

                if(m_phKlaWaferXmlFile->atEnd())
                {
                    m_bEndOfFile = true;
                    return "";
                }
            }
            strString = m_strCurrentLine.left(iPos).trimmed();
            m_strCurrentLine = m_strCurrentLine.remove(0,iPos+1).trimmed();
            if(!strString.isEmpty())
                m_mapTagAttributes[strString.section("=",0,0)] = strString.section("=",1).remove("\"");

            m_strTagValue = "";
            // Then go to the next (END-)tag and save the current value
            while(!m_bEndOfFile)
            {
                if(m_strCurrentLine.isEmpty() || m_strCurrentLine == " ")
                    m_strCurrentLine = m_phKlaWaferXmlFile->readLine().trimmed();

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

                if(m_phKlaWaferXmlFile->atEnd())
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
bool CGKlaWaferXmltoSTDF::GotoMarker(const char *szEndMarker)
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
            return true;
    }
    return false;
}

