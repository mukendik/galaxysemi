//////////////////////////////////////////////////////////////////////
// import_kla_inf_layers.cpp: Convert a Kla Inf Layers file to STDF V4.0
//////////////////////////////////////////////////////////////////////
#include <qfileinfo.h>
#include <qprogressbar.h>
#include <qapplication.h>
#include <qlabel.h>
#include <qregexp.h>


#include "import_kla_inf_layers.h"
#include "time.h"
#include "import_constants.h"
#include "engine.h"

//SmWaferFlow
//{
//    ID:QS0533-18F3
//    CASSETTE:1
//    SLOT:18
//    KEYID:QS0533
//    SESSION_RESULTS:PROCESSED PROCESSED
//    SmWafer
//    {
//	WPT_ID:TMT652-T105
//	LOT:QS0533
//	ID:QS0533-18F3
//    }


// main.cpp
extern QLabel			*GexScriptStatusLabel;	// Handle to script status text in status bar
extern QProgressBar	*	GexProgressBar;		// Handle to progress bar in status bar


CKlaInfLayersBinInfo::CKlaInfLayersBinInfo()
{
    nBinNb = 0;
    nNbCnt = 0;
    bPass  = false;
};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGKlaInfLayerstoSTDF::CGKlaInfLayerstoSTDF()
{
    m_lStartTime = 0;
    m_nWaferRows = 0;
    m_nWaferColumns = 0;
    m_strGoodBin = "01";
    m_nWaferRows = 0;
    m_cWfFlat = 'D';
    m_nFieldSize = 1;
    m_nFieldPacked = 0;
}

//////////////////////////////////////////////////////////////////////
// Destruction
//////////////////////////////////////////////////////////////////////
CGKlaInfLayerstoSTDF::~CGKlaInfLayerstoSTDF()
{
    m_qMapBins.clear();
}

//////////////////////////////////////////////////////////////////////
// Get Error
//////////////////////////////////////////////////////////////////////
QString CGKlaInfLayerstoSTDF::GetLastError()
{
    m_strLastError = "Import Kla Inf Layers: ";

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
// Check if File is compatible with KlaInfLayers format
//////////////////////////////////////////////////////////////////////
bool CGKlaInfLayerstoSTDF::IsCompatible(const char *szFileName)
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
    QTextStream hKlaInfLayersFile(&f);

    do
        strSection = hKlaInfLayersFile.readLine().simplified();
    while(!strSection.isNull() && strSection.isEmpty());

    f.close();
    strValue = strSection.trimmed();
    if(strValue.startsWith("SmWaferFlow", Qt::CaseInsensitive))
        return true;

    return false;
}

//////////////////////////////////////////////////////////////////////
// Read and Parse the KlaInfLayers file
//////////////////////////////////////////////////////////////////////
bool
CGKlaInfLayerstoSTDF::ReadKlaInfLayersFile(const char* KlaInfLayersFileName,
                                           const char* strFileNameSTDF)
{
    QString strString;
    QString strSection;

    // Open file
    QFile f( KlaInfLayersFileName );
    if(!f.open( QIODevice::ReadOnly ))
    {
        // Failed Opening KlaInfLayers file
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

    // init some variable
    m_fDieWidth = 0.0;
    m_fDieHeight = 0.0;
    m_fWaferDiameter = 0.0;

    m_nWaferNotch = 0;
    m_nWaferStepX = 1;
    m_nWaferStepY = -1;
    m_nFieldPacked = 0;
    m_nFieldSize = 2;
    m_nFieldBase = 16;
    m_lStartTime = 0;
    m_lEndTime = 0;

    m_strFieldOff = "_";
    m_strFieldOn = "@";


    // Assign file I/O stream
    QTextStream hKlaInfLayersFile(&f);
    while(!hKlaInfLayersFile.atEnd())
    {
        strSection = ReadLine(hKlaInfLayersFile);

        if(strSection == "SmWaferFlow{")
        {
            ReadAttributes(hKlaInfLayersFile);
            while(!m_lstTagAttributes.isEmpty())
            {
                strString = m_lstTagAttributes.takeFirst();
                strSection = strString.section(":",0,0);
                strString = strString.section(":",1);

                if(strSection == "ID")
                {
                    //ID:QS0533-18F3
                    m_strWaferId = strString;
                }
                else
                if(strSection == "CASSETTE")
                {
                    //CASSETTE:1
                }
                else
                if(strSection == "SLOT")
                {
                    //SLOT:18
                    m_strSubLot = strString;
                }
                else
                if(strSection == "KEYID")
                {
                    //KEYID:QS0533
                    m_strLotId = strString;
                }
                else
                if(strSection == "SESSION_RESULTS")
                {
                    //SESSION_RESULTS:PROCESSED PROCESSED
                }
            }
        }
        else
        if(strSection == "SmWafer{")
        {
            ReadAttributes(hKlaInfLayersFile);
            while(!m_lstTagAttributes.isEmpty())
            {
                strString = m_lstTagAttributes.takeFirst();
                strSection = strString.section(":",0,0);
                strString = strString.section(":",1);

                if(strSection == "WPT_ID")
                {
                    //WPT_ID:TMT652-T105
                    m_strDeviceId = strString;
                }
                else
                if(strSection == "LOT")
                {
                    //LOT:QS0533
                    m_strLotId = strString;
                }
                else
                if(strSection == "ID")
                {
                    //ID:QS0533-18F3
                    m_strWaferId = strString;
                }
            }
        }
        else
        if(strSection == "SmWaferPass{")
        {
            ReadAttributes(hKlaInfLayersFile);
            while(!m_lstTagAttributes.isEmpty())
            {
                strString = m_lstTagAttributes.takeFirst();
                strSection = strString.section(":",0,0);
                strString = strString.section(":",1);

                if(strSection == "TFLOW_ID")
                {
                    //TFLOW_ID:TMT652-T105
                    m_strDeviceId = strString;
                }
                else
                if(strSection == "STTI")
                {
                    //STTI:Mon Jan 28 23:45:42 2008
                    m_lStartTime = GetDateTimeFromString(strString.section(" ",1));

                }
                else
                if(strSection == "ENTI")
                {
                    //ENTI:Tue Jan 29 00:00:10 2008
                    m_lEndTime = GetDateTimeFromString(strString.section(" ",1));

                }
                else
                if(strSection == "OPID")
                {
                    //OPID:9522
                    m_strOperatorName = strString;

                }
                else
                if(strSection == "HOST")
                {
                    //HOST:w2_s
                    m_strNodeName = strString;

                }
                else
                if(strSection == "TST_ID")
                {
                    //TST_ID:w2_s
                    m_strNodeName = strString;

                }
                else
                if(strSection == "TYPRB")
                {
                    //TYPRB:TEL P8
                    m_strProbId = strString;

                }
                else
                if(strSection == "TST_TYPE")
                {
                    //TST_TYPE:Catalyst
                    m_strTstrType = strString;

                }
                else
                if(strSection == "TST_PRG_SW_NM")
                {
                    //TST_PRG_SW_NM:IMAGE test program
                    m_strJobName = strString;

                }
                else
                if(strSection == "TST_PRG_SW_RV")
                {
                    //TST_PRG_SW_RV:Rev 1.0
                    m_strJobRev = strString;

                }
                else
                if(strSection == "TST_SW_NM")
                {
                    //TST_SW_NM:IMAGE
                    m_strExecName = strString;

                }
                else
                if(strSection == "TST_SW_RV")
                {
                    //TST_SW_RV:V7.0 D11 120103
                    m_strExecRev = strString;

                }
            }
        }
        else
        if(strSection == "MdMapResult{")
        {
            // save attributes
            ReadAttributes(hKlaInfLayersFile);
            while(!m_lstTagAttributes.isEmpty())
            {
                strString = m_lstTagAttributes.takeFirst();
                strSection = strString.section(":",0,0);
                strString = strString.section(":",1);

                if(strSection == "dDieWidth")
                {
                    //dDieWidth:7.04
                    m_fDieWidth = strString.toFloat();
                }
                else
                if(strSection == "dDieHeight")
                {
                    //dDieHeight:6.055
                    m_fDieHeight = strString.toFloat();
                }
            }
        }
        else
        if(strSection == "MdBlank{")
        {
            // save attributes
            ReadAttributes(hKlaInfLayersFile);
            while(!m_lstTagAttributes.isEmpty())
            {
                strString = m_lstTagAttributes.takeFirst();
                strSection = strString.section(":",0,0);
                strString = strString.section(":",1);

                if(strSection == "dDiameter")
                {
                    //dDiameter:200.0
                    m_fWaferDiameter = strString.toFloat();
                }
                else
                if(strSection == "dNotchAngle")
                {
                    //dNotchAngle:180.0
                    m_nWaferNotch = (int)strString.toFloat();
                }
            }
        }
        else
        if(strSection == "NlCoordSys{")
        {
            // save attributes
            ReadAttributes(hKlaInfLayersFile);
            while(!m_lstTagAttributes.isEmpty())
            {
                strString = m_lstTagAttributes.takeFirst();
                strSection = strString.section(":",0,0);
                strString = strString.section(":",1);

                if(strSection == "xPol")
                {
                    //xPol:1
                    m_nWaferStepX = strString.toInt();
                }
                else
                if(strSection == "yPol")
                {
                    //yPol:-1
                    m_nWaferStepY = strString.toInt();
                }
            }
        }
        else
        if(strSection == "NlLayer{")
        {
            // save attributes
            bool bReadNlFormat = false;
            ReadAttributes(hKlaInfLayersFile);
            while(!m_lstTagAttributes.isEmpty())
            {
                strString = m_lstTagAttributes.takeFirst();
                strSection = strString.section(":",0,0);
                strString = strString.section(":",1);

                if(strSection == "strTag")
                {
                    //strTag:iBinCodeLast
                    if(strString != "iBinCodeLast")
                        break;

                    m_lstRowsData.clear();
                    bReadNlFormat = true;
                }
                else
                if(strSection == "iRowMin")
                {
                    //iRowMin:6
                    m_strUpperLeftXY = strString;
                }
                else
                if(strSection == "iColMin")
                {
                    //iColMin:8
                    m_strUpperLeftXY += ","+strString;
                }
                else
                if(strSection == "RowData")
                    m_lstRowsData << strString;
            }

            if(!bReadNlFormat)
                continue;

            strSection = ReadLine(hKlaInfLayersFile);
            if(strSection != "NlFormat{")
                continue;

            ReadAttributes(hKlaInfLayersFile);
            while(!m_lstTagAttributes.isEmpty())
            {
                strString = m_lstTagAttributes.takeFirst();
                strSection = strString.section(":",0,0);
                strString = strString.section(":",1);

                if(strSection == "sField")
                {
                    //sField:2
                    m_nFieldSize = strString.toInt();
                }
                else
                if(strSection == "iBase")
                {
                    //iBase:16
                    m_nFieldBase = strString.toInt();
                }
                else
                if(strSection == "fPacked")
                {
                    //fPacked:0
                    m_nFieldPacked = strString.toInt();
                }
                else
                if(strSection == "cOffWaferPad")
                {
                    //cOffWaferPad:_
                    m_strFieldOff = strString;
                }
                else
                if(strSection == "cOnWaferPad")
                {
                    //cOnWaferPad:@
                    m_strFieldOn = strString;
                }
            }
        }
        else
        if(strSection == "StBinTable{")
        {
            // P0 case 6531: if a 'StBinTable' with PSBN tag is found, read
            // binning info
            // save attributes
            int BinNumber=0,Index;
            ReadAttributes(hKlaInfLayersFile);
            while(!m_lstTagAttributes.isEmpty())
            {
                strString = m_lstTagAttributes.takeFirst();
                strSection = strString.section(":",0,0);
                strString = strString.section(":",1);

                if(strSection == "strTag")
                {
                    //strTag:PSBN
                    if(strString != "PSBN")
                        break;

                    // Clear list of binnings
                    m_qMapBins.clear();
                }
                else
                if(strSection == "ListData")
                {
                    for(Index=0; Index<strString.length(); Index++)
                    {
                        m_qMapBins[BinNumber].nBinNb = BinNumber;
                        m_qMapBins[BinNumber].bPass = (strString.at(Index) == QChar('1'));
                        BinNumber++;
                    }
                }
            }
        }
        else
        if(strSection.endsWith("{"))
        {
            // ignore this section
            // goto the next tag
            // skip all { } section
            if(!GoToNextTag(hKlaInfLayersFile))
            {
                // error
                return false;
            }
        }

    }


    if(m_lstRowsData.isEmpty())
    {
        // Error no data
        return false;
    }

    if(!WriteStdfFile(strFileNameSTDF))
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
// Create STDF file from KlaInfLayers data parsed
//////////////////////////////////////////////////////////////////////
bool CGKlaInfLayerstoSTDF::WriteStdfFile(const char *szFileNameSTDF)
{
    // now generate the STDF file...
    GS::StdLib::Stdf StdfFile;
    GS::StdLib::StdfRecordReadInfo RecordReadInfo;
    if(StdfFile.Open((char*)szFileNameSTDF,STDF_WRITE) != GS::StdLib::Stdf::NoError)
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
    StdfFile.WriteString(m_strNodeName.toLatin1().constData());					// Node name
    StdfFile.WriteString(m_strTstrType.toLatin1().constData());	// Tester Type
    StdfFile.WriteString(m_strJobName.toLatin1().constData());		// Job name
    StdfFile.WriteString(m_strJobRev.toLatin1().constData());					// Job rev
    StdfFile.WriteString(m_strSubLot.toLatin1().constData());		// sublot-id
    StdfFile.WriteString(m_strOperatorName.toLatin1().constData());// operator
    StdfFile.WriteString(m_strExecName.toLatin1().constData());					// exec-type
    StdfFile.WriteString(m_strExecRev.toLatin1().constData());					// exe-ver
    StdfFile.WriteString("");					// test-cod
    StdfFile.WriteString("");					// test-temperature
    // Construct custom Galaxy USER_TXT
    QString	strUserTxt;
    strUserTxt = GEX_IMPORT_DATAORIGIN_LABEL;
    strUserTxt += ":";
    strUserTxt += GEX_IMPORT_DATAORIGIN_ATETEST;
    strUserTxt += ":KlaInfLayers";
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
    QString		strString, strBin;
    int			iIndex;				// Loop index
    WORD		wBinning;
    long		iTotalGoodBin,iTotalFailBin;
    long		iPartNumber;
  //FIXME: not used ?
  //bool bPassStatus;
    int			iWaferRows;
    int			iWaferUpperLeftX;
    int			iWaferUpperLeftY;

    iWaferUpperLeftX = m_strUpperLeftXY.section(",",1,1).toInt();
    iWaferUpperLeftY = m_strUpperLeftXY.section(",",0,0).toInt();

    switch(m_nWaferNotch)
    {
        case 0: m_cWfFlat = 'D';
            break;
        case 90: m_cWfFlat = 'L';
            break;
        case 180: m_cWfFlat = 'U';
            break;
        case 270: m_cWfFlat = 'R';
            break;
        default:  m_cWfFlat = ' ';
            break;
    }

    // Retrieve the correct coordonate
    int		iULX=0, iULY=0;
    char	cPosX=' ', cPosY=' ';



    // Reset counters
    iTotalGoodBin=iTotalFailBin=0;
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

    iWaferRows = -1;
    // Ignore this option
    m_nWaferStepY = m_nWaferStepX = 1;

    iULY = iWaferUpperLeftY - m_nWaferStepY;
    // Write all Part read on this wafer.: WIR.PIR,PRR, PIR,PRR,   ... WRR
    while(!m_lstRowsData.isEmpty())
    {


        // Read line
        strString = m_lstRowsData.takeFirst();
        if(m_nWaferColumns == 0)
        {
            if(m_nFieldPacked == 0)
                m_nWaferColumns = strString.count(" ")+1;
            else
                m_nWaferColumns = strString.length()/m_nFieldSize;
        }

        iWaferRows++;
        iULY = iULY + m_nWaferStepY;
        iULX = iWaferUpperLeftX - m_nWaferStepX;


        // Reset Pass/Fail flag.
    //FIXME: not used ?
    //bPassStatus = true;

        // Read Part results for this record
        for(iIndex=0;iIndex<m_nWaferColumns;iIndex++)
        {
            if(m_nFieldPacked == 0)
                strBin = strString.section(" ",iIndex,iIndex);
            else
                strBin = strString.mid((iIndex*m_nFieldSize),m_nFieldSize);

            iULX = iULX + m_nWaferStepX;

            if(strBin.startsWith(m_strFieldOff))
                continue;

            if(strBin.startsWith(m_strFieldOn))
            {
                // Part not tested
                continue;
            }

            wBinning = strBin.toInt(NULL,m_nFieldBase);
            if(!m_qMapBins.contains(wBinning))
            {
                CKlaInfLayersBinInfo cBin;
                cBin.nBinNb = wBinning;
                cBin.nNbCnt = 0;
                cBin.strName = "";
                cBin.bPass = (wBinning == 1);
                m_qMapBins[cBin.nBinNb] = cBin;
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
            if(!m_qMapBins[wBinning].bPass)
            {
                StdfFile.WriteByte(8);				// PART_FLG : FAILED
                iTotalFailBin++;
                m_qMapBins[wBinning].nNbCnt++;
            }
            else
            {
                StdfFile.WriteByte(0);				// PART_FLG : PASSED
                m_qMapBins[wBinning].nNbCnt++;
                iTotalGoodBin++;
            }
            StdfFile.WriteWord((WORD)0);			// NUM_TEST
            StdfFile.WriteWord(wBinning);           // HARD_BIN
            StdfFile.WriteWord(wBinning);           // SOFT_BIN
            StdfFile.WriteWord(iULX);				// X_COORD
            StdfFile.WriteWord(iULY);				// Y_COORD
            StdfFile.WriteDword(0);					// No testing time known...
            StdfFile.WriteString(QString::number(iPartNumber).toLatin1().constData());		// PART_ID
            StdfFile.WriteString("");				// PART_TXT
            StdfFile.WriteString("");				// PART_FIX
            StdfFile.WriteRecord();
        };
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
    RecordReadInfo.iRecordType = 2;
    RecordReadInfo.iRecordSubType = 30;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteFloat(m_fWaferDiameter);		// Wafer size
    StdfFile.WriteFloat(m_fDieHeight);			// Height of die
    StdfFile.WriteFloat(m_fDieWidth);			// Width of die
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
    CGKlaInfLayersBinmap::Iterator itMapBin;
    for ( itMapBin = m_qMapBins.begin(); itMapBin != m_qMapBins.end(); ++itMapBin )
    {
        // Write HBR
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte(255);						// Test Head = ALL
        StdfFile.WriteByte(1);							// Test sites = ALL
        StdfFile.WriteWord(itMapBin.value().nBinNb);	// HBIN = 0
        StdfFile.WriteDword(itMapBin.value().nNbCnt);	// Total Bins
        if(itMapBin.value().bPass)
            StdfFile.WriteByte('P');
        else
            StdfFile.WriteByte('F');
        StdfFile.WriteString(itMapBin.value().strName.toLatin1().constData());
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
        StdfFile.WriteWord(itMapBin.value().nBinNb);	// HBIN = 0
        StdfFile.WriteDword(itMapBin.value().nNbCnt);	// Total Bins
        if(itMapBin.value().bPass)
            StdfFile.WriteByte('P');
        else
            StdfFile.WriteByte('F');
        StdfFile.WriteString(itMapBin.value().strName.toLatin1().constData());
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
// Convert 'FileName' KlaInfLayers file, to STDF 'strFileNameSTDF' file
//////////////////////////////////////////////////////////////////////
bool CGKlaInfLayerstoSTDF::Convert(const char *KlaInfLayersFileName, const char *strFileNameSTDF)
{
    // No erro (default)
    m_iLastError = errNoError;

    // If STDF file already exists...do not rebuild it...unless dates not matching!
    QFileInfo fInput(KlaInfLayersFileName);
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
            GS::Gex::Engine::GetInstance().UpdateLabelStatus("Converting data from file "+QFileInfo(KlaInfLayersFileName).fileName()+"...");
            GexScriptStatusLabel->show();
        }
    }
    QCoreApplication::processEvents();

    if(ReadKlaInfLayersFile(KlaInfLayersFileName,strFileNameSTDF) != true)
    {
        //////////////////////////////////////////////////////////////////////
        // For ProgressBar
        if((GexProgressBar != NULL)
        && bHideProgressAfter)
            GexProgressBar->hide();

        if((GexScriptStatusLabel != NULL)
        && bHideLabelAfter)
            GexScriptStatusLabel->hide();
        return false;	// Error reading KlaInfLayers file
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
QString CGKlaInfLayerstoSTDF::ReadLine(QTextStream& hFile)
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
        strString = hFile.readLine().trimmed();
    while(!strString.isNull() && strString.isEmpty());

    if(strString == "{")
    {
        m_strCurrentLine += strString;
        m_strCurrentTag = m_strCurrentLine;
        m_lstTagAttributes.clear();
    }
    else
        m_strCurrentLine = strString;
    return m_strCurrentLine;

}

//////////////////////////////////////////////////////////////////////
// GotoNextTag : skip tags
//////////////////////////////////////////////////////////////////////
bool CGKlaInfLayerstoSTDF::GoToNextTag(QTextStream& hFile)
{
    QString strLine;
    while(!hFile.atEnd())
    {
        strLine = ReadLine(hFile);

        if(strLine.endsWith("{"))
            return GoToNextTag(hFile);

        if(strLine == "}")
        {
            return true;
        }
    }

    return hFile.atEnd();
}

//////////////////////////////////////////////////////////////////////
// ReadTagAttributes : read all tag attributes
//////////////////////////////////////////////////////////////////////
bool CGKlaInfLayerstoSTDF::ReadAttributes(QTextStream& hFile)
{
    QString strLine;
    m_lstTagAttributes.clear();
    while(!hFile.atEnd())
    {
        strLine = ReadLine(hFile);

        if(strLine.indexOf(":") < 0)
            break;

        if(strLine.indexOf("}") >= 0)
            break;

        if(strLine.indexOf("{") >= 0)
            break;

        m_lstTagAttributes << strLine;
    }

    return true;
}


//////////////////////////////////////////////////////////////////////
//return lDateTime from string strDateTime "Mar 26 20:32 2010" = 2010 May 26, 20:32:00.
//////////////////////////////////////////////////////////////////////
long CGKlaInfLayerstoSTDF::GetDateTimeFromString(QString strDateTime)
{
    int		nYear, nMonth, nDay;
    int		nHour, nMin, nSec;
    long	lDateTime;
    QString strMonth;

    if(strDateTime.length()<17)
        return 0;

    QString strDT = strDateTime.simplified();

    nYear = strDT.section(" ",3,3).toInt();
    strMonth = strDT.section(" ",0,0);
    nDay = strDT.section(" ",1,1).toInt();
    nHour = strDT.section(" ",2,2).section(":",0,0).toInt();
    nMin= strDT.section(" ",2,2).section(":",1,1).toInt();
    nSec = 0;

    if (strMonth == "Jan") nMonth = 1;
    else if (strMonth == "Feb") nMonth = 2;
    else if (strMonth == "Mar") nMonth = 3;
    else if (strMonth == "Apr") nMonth = 4;
    else if (strMonth == "May") nMonth = 5;
    else if (strMonth == "Jun") nMonth = 6;
    else if (strMonth == "Jul") nMonth = 7;
    else if (strMonth == "Aug") nMonth = 8;
    else if (strMonth == "Sep") nMonth = 9;
    else if (strMonth == "Oct") nMonth = 10;
    else if (strMonth == "Nov") nMonth = 11;
    else if (strMonth == "Dec") nMonth = 12;
    else
    {
    nMonth = 0;
    //TODO: warning or error ?
    }

    QDate clDate(nYear,nMonth,nDay);
    QTime clTime(nHour,nMin,nSec);
    QDateTime clDateTime(clDate,clTime);

    clDateTime.setTimeSpec(Qt::UTC);
    lDateTime = clDateTime.toTime_t();
    return lDateTime;
}

