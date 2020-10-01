//////////////////////////////////////////////////////////////////////
// import_nd1nd2_datalog.cpp: Convert a nd1nd2_datalog file to STDF V4.0
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

#include "import_nd1nd2_datalog.h"
#include "import_constants.h"
#include "engine.h"
#include "gqtl_global.h"

// File format: Data log
//**** Viewpoint DataLog: Mar 26 20:32 2010 ****
//  Plan Name  : LG4261_PT1_R01
//  Device Name: LG4261_0326c            DUT I/F   : --------
//  Lot #      : lgm47500L5d0326c        Sub Lot # : Mar26
//  Wafer Name : --------                Sample  # : 3390
//  Operator   : --------                Comments  : --------
//  Station No : 1
//  DUT    X    Y
//    1   29    1
//
//******************** [Test 1000] FT PASS "CONTACT TEST : DPIN & LCDPIN : POSITIVE" ********************
//Lpat         : /test/DDI/2009/LG4261/PATTERN/lg4261_contact.lpa
//Start(STA)   : 0x00000000
//Stop(SPA)    : 0x00000002
//PASS
//
//******************** [Test 1010] FT PASS "CONTACT TEST : DPIN & LCDPIN : NEGATIVE" ********************
//Lpat         : /test/DDI/2009/LG4261/PATTERN/lg4261_contact.lpa
//Start(STA)   : 0x00000003
//Stop(SPA)    : 0x00000005
//PASS
//
//******************** [Test 1040] VM FAIL "CONTACT TEST : RVSPINS_N_SD_1A : NEGATIVE" ********************
//Resc :HVDC_RVS SRng :R8MA     SVal: 1.000mA MRng  :M8V      MSeq  :PARA
//CPVal: 6.000V  CMVal:-3.000V  Slew:         Filter:OFF      PCCapa:
//MIM  :     MIMPCCapa:
//TestID   RESULT       Value      UP_LIM      LO_LIM Dpin DUT
// 01040    HFAIL  364.0000mV  360.0000mV  260.0000mV SD1A   1




// main.cpp
extern QLabel			*GexScriptStatusLabel;	// Handle to script status text in status bar
extern QProgressBar	*	GexProgressBar;		// Handle to progress bar in status bar

#define BIT0			0x01
#define BIT1			0x02
#define BIT2			0x04
#define BIT3			0x08
#define BIT4			0x10
#define BIT5			0x20
#define BIT6			0x40
#define BIT7			0x80

#define	C_INFINITE	(float) 1e32

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGNd1Nd2DatalogtoSTDF::CGNd1Nd2DatalogtoSTDF()
{
    m_bFlagGenerateMpr = true;
    m_nPass			= 1;
    m_lStartTime = 0;
}

//////////////////////////////////////////////////////////////////////
// Get Error
//////////////////////////////////////////////////////////////////////
QString CGNd1Nd2DatalogtoSTDF::GetLastError()
{
    m_strLastError = "Import Nd1/Nd2 Datalog: ";

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
// Normalize test limits when writing into PTR.
//////////////////////////////////////////////////////////////////////
void CGNd1Nd2DatalogtoSTDF::NormalizeValues(QString &strUnits,float &fValue, int &nScale, bool &bIsNumber)
{
    int i;
    QString strValue = strUnits;

    // In strValue, the current value with unit
    bIsNumber = false;
    i = strValue.length();
    while(i > 0)
    {
        strValue.mid(i-1,1).toInt(&bIsNumber);
        if(bIsNumber)
            break;
        i--;
    }

    if(strValue.length() <= i)
    {
        // no unit
        strUnits = "";
    }
    else
    {
        strUnits = strValue.right(strValue.length()-i);
        strValue = strValue.left(i);
        if(strUnits == "NONE")
            strUnits = "";
    }

    fValue = strValue.toFloat(&bIsNumber);
    nScale=0;

    if(strUnits.length() <= 1)
    {
        // units too short to include a prefix, then keep it 'as-is'
        return;
    }

    QChar cPrefix = strUnits[0];
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
    if(nScale != 0)
    {
        fValue *= GS_POW(10.0,nScale);
        strUnits = strUnits.mid(1);	// Take all characters after the prefix.
    }
}

//////////////////////////////////////////////////////////////////////
// Check if File is compatible with Nd1Nd2Datalog format
//////////////////////////////////////////////////////////////////////
bool CGNd1Nd2DatalogtoSTDF::IsCompatible(const char *szFileName)
{
    QString strString;
    QString strSection;
    QString strValue;

    // Open hNd1Nd2DatalogFile file
    QFile f( szFileName );
    if(!f.open( QIODevice::ReadOnly ))
    {
        // Failed Opening Nd1Nd2Datalog file
        return false;
    }
    // Assign file I/O stream
    QTextStream hNd1Nd2DatalogFile(&f);

    // Check if first line is the correct Nd1Nd2Datalog header...
    //**** Viewpoint DataLog: Mar 26 20:32 2010 ****
    //  Plan Name  : LG4261_PT1_R01
    //  Device Name: LG4261_0326c            DUT I/F   : --------
    //  Lot #      : lgm47500L5d0326c        Sub Lot # : Mar26
    //  Wafer Name : --------                Sample  # : 3390
    //  Operator   : --------                Comments  : --------
    //  Station No : 1
    //  DUT    X    Y
    //    1   29    1
    //
    //******************** [Test 1000] FT PASS "CONTACT TEST : DPIN & LCDPIN : POSITIVE" ********************

    do
        strString = hNd1Nd2DatalogFile.readLine();
    while(!strString.isNull() && strString.isEmpty());

    // Close file
    f.close();

    if(strString.startsWith("**** Viewpoint DataLog:", Qt::CaseInsensitive))
        return true;
    else
        return false;
}

//////////////////////////////////////////////////////////////////////
// Read and Parse the Nd1Nd2Datalog file
//////////////////////////////////////////////////////////////////////
bool CGNd1Nd2DatalogtoSTDF::ReadNd1Nd2DatalogFile(const char *Nd1Nd2DatalogFileName,const char *strFileNameSTDF)
{
    QString strString;
    QString strSection;


    // Open Nd1Nd2Datalog file
    QFile f( Nd1Nd2DatalogFileName );
    if(!f.open( QIODevice::ReadOnly ))
    {
        // Failed Opening Nd1Nd2Datalog file
        m_iLastError = errOpenFail;

        // Convertion failed.
        return false;
    }

    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    iFileSize = f.size() + 1;
    m_nPass = 1;


    // Assign file I/O stream
    QTextStream hNd1Nd2DatalogFile(&f);

    // Check if first line is the correct Nd1Nd2Datalog header...
    //**** Viewpoint DataLog: Mar 26 20:32 2010 ****

    m_iLastError = errInvalidFormat;
    // Read first non-empty line
    strString = ReadLine(hNd1Nd2DatalogFile);
    if(strString.isNull())
    {
        // Close file
        f.close();
        return false;
    }

    // Incorrect header...this is not a Nd1Nd2Datalog file!
    // Convertion failed.
    if(!strString.startsWith("**** Viewpoint DataLog:", Qt::CaseInsensitive))
    {
        // Close file
        f.close();
        return false;
    }

    strString = strString.section(": ",1).remove('*').simplified();
    m_lStartTime = GetDateTimeFromString(strString);


    // Read Nd1Nd2Datalog information
    //**** Viewpoint DataLog: Mar 26 20:32 2010 ****
    //  Plan Name  : LG4261_PT1_R01
    //  Device Name: LG4261_0326c            DUT I/F   : --------
    //  Lot #      : lgm47500L5d0326c        Sub Lot # : Mar26
    //  Wafer Name : --------                Sample  # : 3390
    //  Operator   : --------                Comments  : --------
    //  Station No : 1
    //  DUT    X    Y
    //    1   29    1
    //
    //******************** [Test 1000] FT PASS "CONTACT TEST : DPIN & LCDPIN : POSITIVE" ********************

    strString = "";
    while(!hNd1Nd2DatalogFile.atEnd())
    {
        if(strString.isEmpty())
            strString = ReadLine(hNd1Nd2DatalogFile);

        // Read header
        strSection = strString.section(":",0,0).simplified().toUpper();
        strString = strString.section(":",1);

        if(strSection == "DUT X Y")
        {
            break;
        }
        else if(strSection.startsWith("******************** [TEST "))
        {
            break;
        }
        else if(strSection == "PLAN NAME")
        {
            m_strProgramId = strString.left(24).remove("-").simplified();
            strString = strString.mid(24);
        }
        else if(strSection == "DEVICE NAME")
        {
            m_strDeviceId = strString.left(24).remove("-").simplified();
            strString = strString.mid(24);
        }
        else if(strSection == "LOT #")
        {
            m_strLotId = strString.left(24).remove("-").simplified();
            strString = strString.mid(24);
        }
        else if(strSection == "WAFER NAME")
        {
            m_strWaferId = strString.left(24).remove("-").simplified();
            strString = strString.mid(24);
        }
        else if(strSection == "OPERATOR")
        {
            m_strOperator = strString.left(24).simplified();
            strString = strString.mid(24);
        }
        else if(strSection == "STATION NO")
        {
            m_strStationId = strString.left(24).remove("-").simplified();
            strString = strString.mid(24);
        }
        else if(strSection == "DUT I/F")
        {// ignore
            strString = "";
        }
        else if(strSection == "SUB LOT #")
        {
            m_strSubLotId = strString.left(24).remove("-").simplified();
            strString = strString.mid(24);
        }
        else if(strSection == "SAMPLE #")
        {// ignore
            strString = "";
        }
        else if(strSection == "COMMENTS")
        {// ignore
            strString = "";
        }
        else
        {
            // ignore
            strString = "";
        }
    }

    if(m_bFlagGenerateMpr)
    {
        // Read the first run to save all pin name for MPR parameter
        // only if m_bFlagGenerateMpr
    //FIXME: not used ?
    //int	nPosTestId, nPosResult, nPosValue,nPosHLimit, nPosLLimit;
    int nPosPin, nPosDUT;
        QString strPinName;

        while(!hNd1Nd2DatalogFile.atEnd())
        {
            strString = ReadLine(hNd1Nd2DatalogFile);

            if(strString.left(6) == "TestID")
            {
                //TestID   RESULT       Value      UP_LIM      LO_LIM Dpin DUT
                // 01040    HFAIL  364.0000mV  360.0000mV  260.0000mV SD1A   1

                // Check if have DPin
        //FIXME: not used ?
                /*nPosTestId = 0;
                nPosResult = strString.indexOf("TestID") + 6;
                nPosValue = strString.indexOf("RESULT") + 6;
                nPosHLimit = strString.indexOf("Value") + 5;
                nPosLLimit = strString.indexOf("UP_LIM") + 6;*/
                nPosPin = strString.indexOf("LO_LIM") + 6;
                nPosDUT = strString.indexOf("pin") + 3;
                if(nPosDUT < 3)
                {
                    nPosDUT = nPosPin;
                    nPosPin = -1;
                }

                if(nPosPin < 0)
                    continue;

                while(!hNd1Nd2DatalogFile.atEnd())
                {
                    strString = ReadLine(hNd1Nd2DatalogFile);

                    if(strString.left(4) == "****")
                        break;

                    strPinName = strString.mid(nPosPin, nPosDUT-nPosPin).trimmed();

                    if(!m_qMapPinIndex.contains(strPinName))
                        m_qMapPinIndex[strPinName] = m_qMapPinIndex.count()+1;
                }
            }
        }
    }

    //Restart at the beggining of the file
    hNd1Nd2DatalogFile.seek(0);
    iNextFilePos = 0;
    m_nPass			= 2;


    if(WriteStdfFile(hNd1Nd2DatalogFile,strFileNameSTDF) != true)
    {
        QFile::remove(strFileNameSTDF);
        // Close file
        f.close();
        return false;
    }

    // Success parsing Nd1Nd2Datalog file
    m_iLastError = errNoError;
    return true;
}

//////////////////////////////////////////////////////////////////////
// Create STDF file from Nd1Nd2Datalog data parsed
//////////////////////////////////////////////////////////////////////
bool CGNd1Nd2DatalogtoSTDF::WriteStdfFile(QTextStream &hNd1Nd2DatalogFile,const char *strFileNameSTDF)
{
    // now generate the STDF file...
    GS::StdLib::Stdf StdfFile;
    GS::StdLib::StdfRecordReadInfo RecordReadInfo;
    if(StdfFile.Open((char*)strFileNameSTDF,STDF_WRITE) != GS::StdLib::Stdf::NoError)
    {
        // Failed importing Nd1Nd2Datalog file into STDF database
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
    StdfFile.WriteByte(m_strStationId.toInt());	// Station
    StdfFile.WriteByte((BYTE) 'P');				// Test Mode = PRODUCTION
    StdfFile.WriteByte((BYTE) ' ');				// rtst_cod
    StdfFile.WriteByte((BYTE) ' ');				// prot_cod
    StdfFile.WriteWord(65535);					// burn_tim
    StdfFile.WriteByte((BYTE) ' ');				// cmod_cod
    StdfFile.WriteString(m_strLotId.toLatin1().constData());	// Lot ID
    StdfFile.WriteString(m_strDeviceId.toLatin1().constData());	// Part Type / Product ID
    StdfFile.WriteString("");					// Node name
    StdfFile.WriteString("");					// Tester Type
    StdfFile.WriteString(m_strProgramId.toLatin1().constData());// Job name
    StdfFile.WriteString("");					// Job rev
    StdfFile.WriteString(m_strSubLotId.toLatin1().constData());	// sublot-id
    StdfFile.WriteString("");					// operator
    StdfFile.WriteString("");					// exec-type
    StdfFile.WriteString("");					// exe-ver
    StdfFile.WriteString("");					// test-cod
    StdfFile.WriteString("");					// test-temperature
    // Construct custom Galaxy USER_TXT
    QString	strUserTxt;
    strUserTxt = GEX_IMPORT_DATAORIGIN_LABEL;
    strUserTxt += ":";
    strUserTxt += GEX_IMPORT_DATAORIGIN_ATETEST;
    strUserTxt += ":ND1/ND2_Datalog";
    StdfFile.WriteString(strUserTxt.toLatin1().constData());		// user-txt
    StdfFile.WriteString("");					// aux-file
    StdfFile.WriteString("");					// package-type
    StdfFile.WriteString(m_strDeviceId.toLatin1().constData());	// familyID
    StdfFile.WriteString("");					// Date-code
    StdfFile.WriteString("");					// Facility-ID
    StdfFile.WriteString("");					// FloorID
    StdfFile.WriteString("");					// ProcessID

    StdfFile.WriteRecord();

    QMap<QString,int>::Iterator itStringInt;
    QMap<QString, QString>::Iterator itStringFloat;
    QString			strString, strValue;
    QString			strTestName;
    QString			strTestType;
    QString			strPaternName;
    int				nTestNum;
    float			fResult;
    bool			bHaveResult;
    int				nScale;
    QString			strUnit;
    QString			strBin;
    QString			strPinName;
    int				iPinIndex;
    int				nSiteId;
    BYTE			bData;

    bool			bPassStatus;
    int				iHBin, iSBin, iTotalTests, iPartNumber;
    int				iTotalGoodBin, iTotalFailBin, iTotalPart;
    int				i;

    CGNd1Nd2DatalogParameter* ptTest;
    QStringList		lstSites;
  //FIXME: not used ?
  //int	nPosTestId, nPosResult;
  int nPosValue, nPosHLimit, nPosLLimit, nPosPin, nPosDUT;

    bData = 0;
    bPassStatus = true;
    iPinIndex = iHBin = iSBin = nSiteId = nTestNum = iPartNumber = 1;
    iTotalTests = iTotalPart = iTotalGoodBin = iTotalFailBin = 0;

    m_mapSoftBinsCount["1"].m_nBinNum = 1;
    m_mapSoftBinsCount["1"].m_strBinName = "PASS";

    m_mapHardBinsCount["1"].m_nBinNum = 1;
    m_mapHardBinsCount["1"].m_strBinName = "PASS";
    m_mapHardBinsCount["0"].m_nBinNum = 0;
    m_mapHardBinsCount["0"].m_strBinName = "FAIL";

    // Write PMR record if any
    if(m_qMapPinIndex.count() > 0)
    {

        for(itStringInt=m_qMapPinIndex.begin(); itStringInt!=m_qMapPinIndex.end(); itStringInt++)
        {
            strPinName = itStringInt.key();
            iPinIndex = itStringInt.value();

            // Write PMR pin info.
            RecordReadInfo.iRecordType = 1;
            RecordReadInfo.iRecordSubType = 60;
            StdfFile.WriteHeader(&RecordReadInfo);
            StdfFile.WriteWord(iPinIndex);						// Pin Index
            StdfFile.WriteWord(0);								// Channel type
            StdfFile.WriteString(strPinName.toLatin1().constData());	// Channel name
            StdfFile.WriteString("");							// Physical name
            StdfFile.WriteString("");							// Logical name
            StdfFile.WriteByte(1);								// Test head
            StdfFile.WriteByte(255);							// Tester site (all)
            StdfFile.WriteRecord();
        }

    }

    if(m_strWaferId.isEmpty())
        m_strWaferId = "1";

    // Write WIR of new Wafer.
    RecordReadInfo.iRecordType = 2;
    RecordReadInfo.iRecordSubType = 10;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteByte(1);									// Test head
    StdfFile.WriteByte(255);								// Tester site (all)
    StdfFile.WriteDword(m_lStartTime);						// Start time
    StdfFile.WriteString(m_strWaferId.toLatin1().constData());	// WaferID
    StdfFile.WriteRecord();


    // Write Test results for each PartID
    strString = "";
    while(!hNd1Nd2DatalogFile.atEnd())
    {
        if(strString.isEmpty())
            strString = ReadLine(hNd1Nd2DatalogFile);


        // Read header
        if(strString.left(23) == "**** Viewpoint DataLog:")
        {

            //**** Viewpoint DataLog: Mar 26 20:32 2010 ****
            //  Plan Name  : LG4261_PT1_R01
            //  Device Name: LG4261_0326c            DUT I/F   : --------
            //  Lot #      : lgm47500L5d0326c        Sub Lot # : Mar26
            //  Wafer Name : --------                Sample  # : 3390
            //  Operator   : --------                Comments  : --------
            //  Station No : 1
            //  DUT    X    Y
            //    1   29    1

            strString = strString.section(":",1).remove("*");
            m_lStartTime = GetDateTimeFromString(strString.simplified());

            m_clPartsInfo.m_strBin.clear();
            m_clPartsInfo.m_nXLoc.clear();
            m_clPartsInfo.m_nYLoc.clear();

            // then ignore all other line until next DUT X Y
            while(!hNd1Nd2DatalogFile.atEnd())
            {
                strString = ReadLine(hNd1Nd2DatalogFile).simplified();
                if(strString.left(4) == "****")
                    break;
                if(strString.left(7) == "DUT X Y")
                    break;
            }

            if(strString.left(7) == "DUT X Y")
            {

                while(!hNd1Nd2DatalogFile.atEnd())
                {
                    strString = ReadLine(hNd1Nd2DatalogFile);
                    if(strString.left(4) == "****")
                        break;
                    strString = strString.simplified();
                    nSiteId = strString.section(" ",0,0).toInt();
                    if(!lstSites.contains(QString::number(nSiteId)))
                        lstSites.append(QString::number(nSiteId));
                    m_clPartsInfo.m_nXLoc[nSiteId] = strString.section(" ",1,1).toInt();
                    m_clPartsInfo.m_nYLoc[nSiteId] = strString.section(" ",2,2).toInt();
                }
            }
        }
        else if(strString.left(5) == "DUT :")
        {

            //DUT : 1
            //		Category :  13, 12, 4
            //		Sort     :
            strString = strString.simplified();

            nSiteId = strString.section(":",1,1).trimmed().toInt();
            if(!lstSites.contains(QString::number(nSiteId)))
                lstSites.append(QString::number(nSiteId));

            strString = ReadLine(hNd1Nd2DatalogFile).simplified();
            while(!hNd1Nd2DatalogFile.atEnd())
            {
                if(strString.left(4) == "****")
                    break;
                if(strString.left(8) == "Category")
                    m_clPartsInfo.m_strBin[nSiteId] = strString.section(":",1,1).replace(","," ").simplified().section(" ",0,0);
                strString = ReadLine(hNd1Nd2DatalogFile).simplified();
            }

            // end of PartId tested
            // write PIR PTR PTR ... PRR for each Part/Site

            while(!lstSites.isEmpty())
            {
                nSiteId = lstSites.takeFirst().toInt();

                // Write PIR for parts in this Wafer site
                RecordReadInfo.iRecordType = 5;
                RecordReadInfo.iRecordSubType = 10;
                StdfFile.WriteHeader(&RecordReadInfo);
                StdfFile.WriteByte(1);								// Test head
                StdfFile.WriteByte(nSiteId);					// Tester site
                StdfFile.WriteRecord();


                // Reset counters
                iTotalTests = 0;
                strBin = "PASS";

                QListIterator<CGNd1Nd2DatalogParameter*> lstIteratorParameterFlow(m_lstpParameterFlow);
                lstIteratorParameterFlow.toFront();

                while(lstIteratorParameterFlow.hasNext())
                {
                    ptTest = lstIteratorParameterFlow.next();

                    // Check if have result for this site
                    if(!ptTest->m_mapBin.contains(nSiteId))
                        continue;

                    iTotalTests ++;

                    if(strBin.endsWith("PASS"))
                        strBin = ptTest->m_mapBin[nSiteId];

                    // Check if it is FTR (0 result), PTR (1 result without pin name), MPR (results with pin name)
                    if( ptTest->m_mapValue[nSiteId].contains("VECT_NAME"))
                    {
                        strTestType = "FTR";

                        // Write FTR
                        RecordReadInfo.iRecordType = 15;
                        RecordReadInfo.iRecordSubType = 20;

                        StdfFile.WriteHeader(&RecordReadInfo);
                        StdfFile.WriteDword(ptTest->m_nNumber);			// Test Number
                        StdfFile.WriteByte(1);							// Test head
                        StdfFile.WriteByte(nSiteId);					// Tester site:1,2,3,4 or 5, etc.
                        if(ptTest->m_mapBin[nSiteId].endsWith("PASS"))
                            bData = 0;		// Test passed
                        else
                            bData = BIT7;	// Test Failed

                        StdfFile.WriteByte(bData);						// TEST_FLG
                        bData = 0;
                        StdfFile.WriteByte(bData);						// OPT_FLG
                        StdfFile.WriteDword(0);		// cycl_cnt
                        StdfFile.WriteDword(0);		// rel_vadr
                        StdfFile.WriteDword(0);		// rept_cnt
                        StdfFile.WriteDword(0);		// num_fail
                        StdfFile.WriteDword(0);		// xfail_ad
                        StdfFile.WriteDword(0);		// yfail_ad
                        StdfFile.WriteWord(ptTest->m_mapValue[nSiteId]["VECT_OFF"].toInt());		// vect_off
                        StdfFile.WriteWord(0);		// rtn_icnt
                        StdfFile.WriteWord(0);
                        StdfFile.WriteWord(0);
                        StdfFile.WriteString(ptTest->m_mapValue[nSiteId]["VECT_NAME"].toLatin1().constData());	// vect_name
                        StdfFile.WriteString("");	// time_set
                        StdfFile.WriteString("");	// op_code
                        StdfFile.WriteString(ptTest->m_strName.toLatin1().constData());	// test_txt: test name
                        StdfFile.WriteString("");	// alarm_id
                        StdfFile.WriteString("");	// prog_txt
                        StdfFile.WriteString("");	// rslt_txt
                        StdfFile.WriteByte(0);		// patg_num
                        StdfFile.WriteString("");	// spin_map
                        StdfFile.WriteRecord();
                    }
                    else
                    if( ptTest->m_mapValue[nSiteId].count() == 1)
                    {
                        strTestType = "PTR";

                        strPinName = ptTest->m_mapValue[nSiteId].begin().key();

                        // Write PTR
                        RecordReadInfo.iRecordType = 15;
                        RecordReadInfo.iRecordSubType = 10;

                        StdfFile.WriteHeader(&RecordReadInfo);
                        StdfFile.WriteDword(ptTest->m_nNumber);			// Test Number
                        StdfFile.WriteByte(1);							// Test head
                        StdfFile.WriteByte(nSiteId);					// Tester site:1,2,3,4 or 5, etc.
                        if(ptTest->m_mapBin[nSiteId].endsWith("PASS"))
                            bData = 0;		// Test passed
                        else
                            bData = BIT7;	// Test Failed

                        StdfFile.WriteByte(bData);						// TEST_FLG
                        bData = BIT6|BIT7;
                        StdfFile.WriteByte(bData);						// PARAM_FLG
                        StdfFile.WriteFloat(ptTest->m_mapValue[nSiteId][strPinName].toFloat());							// Test result

                        // MPR with only one pin, save as PTR
                        strTestName = ptTest->m_strName;
                        if(strPinName != "NO-PIN-DATA")
                        {
                            strTestName += " - " + strPinName;
                            strTestType = "MPR";
                        }

                        StdfFile.WriteString(strTestName.toLatin1().constData());	// TEST_TXT
                        if(!ptTest->m_bStaticHeaderWritten)
                        {
                            // save Parameter name without unit information
                            StdfFile.WriteString("");					// ALARM_ID

                            bData = BIT1;	// Valid data.
                            if(!ptTest->m_bValidLowLimit)
                                bData |= BIT6;
                            if(!ptTest->m_bValidHighLimit)
                                bData |= BIT7;
                            StdfFile.WriteByte(bData);					// OPT_FLAG

                            StdfFile.WriteByte(-ptTest->m_nScale);		// RES_SCALE
                            StdfFile.WriteByte(-ptTest->m_nScale);		// LLM_SCALE
                            StdfFile.WriteByte(-ptTest->m_nScale);		// HLM_SCALE
                            StdfFile.WriteFloat(ptTest->m_fLowLimit);	// LOW Limit
                            StdfFile.WriteFloat(ptTest->m_fHighLimit);	// HIGH Limit
                            StdfFile.WriteString(ptTest->m_strUnits.toLatin1().constData());		// Units
                        }
                        StdfFile.WriteRecord();
                    }
                    else
                    {
                        strTestType = "MPR";

                        // only if m_bFlagGenerateMpr
                        // else error
                        if(!m_bFlagGenerateMpr)
                            return false;

                        // Write MPR
                        RecordReadInfo.iRecordType = 15;
                        RecordReadInfo.iRecordSubType = 15;

                        StdfFile.WriteHeader(&RecordReadInfo);
                        StdfFile.WriteDword(ptTest->m_nNumber);		// Test Number
                        StdfFile.WriteByte(1);						// Test head
                        StdfFile.WriteByte(nSiteId);				// Tester site:1,2,3,4 or 5, etc.
                        bData = 0;
                        if(!ptTest->m_mapBin[nSiteId].endsWith("PASS"))
                            bData |= BIT7;	// Test Failed

                        StdfFile.WriteByte(bData);					// TEST_FLG
                        bData = BIT6|BIT7;
                        StdfFile.WriteByte(bData);					// PARAM_FLG
                        StdfFile.WriteWord(ptTest->m_mapValue[nSiteId].count());	// RTN_ICNT
                        StdfFile.WriteWord(ptTest->m_mapValue[nSiteId].count());	// RSLT_CNT
                        bData = 0;
                        for(i=0; i!=(ptTest->m_mapValue[nSiteId].count()+1)/2; i++)
                        {
                            StdfFile.WriteByte(0);					// RTN_STAT
                        }
                        for(itStringFloat=ptTest->m_mapValue[nSiteId].begin(); itStringFloat!=ptTest->m_mapValue[nSiteId].end(); itStringFloat++)
                        {
                            StdfFile.WriteFloat(itStringFloat.value().toFloat());			// Test result
                        }
                        StdfFile.WriteString(ptTest->m_strName.toLatin1().constData());	// TEST_TXT
                        StdfFile.WriteString("");					// ALARM_ID

                        if(!ptTest->m_bStaticHeaderWritten)
                        {
                            // No Scale result
                            bData = 0;
                            if(ptTest->m_nScale == 0)
                                bData |= BIT0;
                            // No LowLimit
                            if(!ptTest->m_bValidLowLimit)
                            {
                                bData |= BIT2;
                                bData |= BIT6;
                            }
                            // No HighLimit
                            if(!ptTest->m_bValidHighLimit)
                            {
                                bData |= BIT3;
                                bData |= BIT7;
                            }
                            StdfFile.WriteByte(bData);					// OPT_FLAG
                            StdfFile.WriteByte(-ptTest->m_nScale);		// RES_SCALE
                            StdfFile.WriteByte(-ptTest->m_nScale);		// LLM_SCALE
                            StdfFile.WriteByte(-ptTest->m_nScale);		// HLM_SCALE
                            StdfFile.WriteFloat(ptTest->m_fLowLimit);	// LOW Limit
                            StdfFile.WriteFloat(ptTest->m_fHighLimit);	// HIGH Limit
                            StdfFile.WriteFloat(0);						// StartIn
                            StdfFile.WriteFloat(0);						// IncrIn
                            for(itStringFloat=ptTest->m_mapValue[nSiteId].begin(); itStringFloat!=ptTest->m_mapValue[nSiteId].end(); itStringFloat++)
                            {
                                if(!m_qMapPinIndex.contains(itStringFloat.key()))
                                    m_qMapPinIndex[itStringFloat.key()] = m_qMapPinIndex.count()+1;
                                StdfFile.WriteWord(m_qMapPinIndex[itStringFloat.key()]);		// RTN_INDX
                            }
                            StdfFile.WriteString(ptTest->m_strUnits.toLatin1().constData());	// Units
                            StdfFile.WriteString("");				// units_in
                            StdfFile.WriteString("");				// c_resfmt
                            StdfFile.WriteString("");				// c_llmfmt
                            StdfFile.WriteString("");				// c_hlmfmt
                            StdfFile.WriteFloat(0);					// lo_spec
                            StdfFile.WriteFloat(0);					// hi_spec
                        }
                        StdfFile.WriteRecord();
                    }

                    if(!m_lstParametersStaticHeaderWritten.contains(strTestType + ptTest->m_strName))
                        m_lstParametersStaticHeaderWritten.append(strTestType + ptTest->m_strName);
                }

                iSBin = iHBin = m_clPartsInfo.m_strBin[nSiteId].toInt();

                bPassStatus = (iSBin == 1);
                if(!bPassStatus)
                    iHBin = 0;

                if(!m_mapHardBinsCount[QString::number(iHBin)].m_nCount.contains(nSiteId))
                    m_mapHardBinsCount[QString::number(iHBin)].m_nCount[nSiteId] = 0;
                m_mapHardBinsCount[QString::number(iHBin)].m_nCount[nSiteId]++;

                if(!m_mapSoftBinsCount.contains(QString::number(iSBin)))
                {
                    m_mapSoftBinsCount[QString::number(iSBin)].m_strBinName = "";
                    m_mapSoftBinsCount[QString::number(iSBin)].m_nBinNum = iSBin;
                    m_mapSoftBinsCount[QString::number(iSBin)].m_nCount[nSiteId] = 0;
                }
                if(!m_mapSoftBinsCount[QString::number(iSBin)].m_nCount.contains(nSiteId))
                    m_mapSoftBinsCount[QString::number(iSBin)].m_nCount[nSiteId] = 0;

                m_mapSoftBinsCount[QString::number(iSBin)].m_nCount[nSiteId]++;

                iTotalPart++;

                // Write PRR
                RecordReadInfo.iRecordType = 5;
                RecordReadInfo.iRecordSubType = 20;
                StdfFile.WriteHeader(&RecordReadInfo);
                StdfFile.WriteByte(1);						// Test head
                StdfFile.WriteByte(nSiteId);			// Tester site:1,2,3,4 or 5
                if(bPassStatus)
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
                StdfFile.WriteWord(iHBin);				// HARD_BIN
                StdfFile.WriteWord(iSBin);				// SOFT_BIN
                StdfFile.WriteWord(m_clPartsInfo.m_nXLoc[nSiteId]);	// X_COORD
                StdfFile.WriteWord(m_clPartsInfo.m_nYLoc[nSiteId]);	// Y_COORD
                StdfFile.WriteDword(0);					// No testing time known...
                StdfFile.WriteString(QString::number(iPartNumber ++).toLatin1().constData());// PART_ID
                StdfFile.WriteString("");				// PART_TXT
                StdfFile.WriteString("");				// PART_FIX
                StdfFile.WriteRecord();
            }

            // Clean all the flow results
            while(!m_lstpParameterFlow.isEmpty())
            {
                ptTest = m_lstpParameterFlow.takeFirst();

                ptTest->m_mapBin.clear();
                ptTest->m_mapValue.clear();
                delete ptTest;
            }

            lstSites.clear();
            ptTest=NULL;
        }
        else if(strString.left(27) == "******************** [Test ")
        {
            //******************** [Test 1040] VM PASS "CONTACT TEST : RVSPINS_N_SD_1A : NEGATIVE" ********************
            //Resc :HVDC_RVS SRng :R8MA     SVal: 1.000mA MRng  :M8V      MSeq  :PARA
            //CPVal: 6.000V  CMVal:-3.000V  Slew:         Filter:OFF      PCCapa:
            //MIM  :     MIMPCCapa:
            //TestID   RESULT       Value      UP_LIM      LO_LIM Dpin DUT
            // 01040           354.0000mV  360.0000mV  260.0000mV SD1A   1

            // OR
            //******************** [Test 1010] FT PASS "CONTACT TEST : DPIN & LCDPIN : NEGATIVE" ********************
            //Lpat         : /test/DDI/2009/LG4261/PATTERN/lg4261_contact.lpa
            //Start(STA)   : 0x00000003
            //Stop(SPA)    : 0x00000005
            //PASS

            strPaternName = "";
            nTestNum = strString.section("[Test ",1).section("]",0,0).toInt();
            strTestName = strString.section("\"",1,1);
            strBin = strString.section("] ",1).section(" ",0,1);
            bPassStatus = strBin.endsWith("PASS");

            ptTest = NULL;

            if(strString.section("] ",1).left(3) == "FT ")
            {
                // Function test
                strTestType = "FTR";

                strString = ReadLine(hNd1Nd2DatalogFile);
                if(strString.left(5) == "Lpat ")
                    strPaternName = strString.section(":",1).simplified();
                // goto Result info
                strString = ReadLine(hNd1Nd2DatalogFile);
                strString = ReadLine(hNd1Nd2DatalogFile);
                while(!hNd1Nd2DatalogFile.atEnd())
                {
                    strString = ReadLine(hNd1Nd2DatalogFile);
                    if(strString.left(4) == "****")
                        break;
                    if(strString.left(5) == "DUT :")
                        break;

                    // Ignore "BLUE BOX"
                    if(strString.length() > 25)
                        continue;

                    if(strString.left(4) == "DUT ")
                    {
                        nSiteId = strString.section(":",1).simplified().toInt();
                        if(!lstSites.contains(QString::number(nSiteId)))
                            lstSites.append(QString::number(nSiteId));

                        strString = ReadLine(hNd1Nd2DatalogFile);
                    }

                    /*
                    if(strString.left(11) == "Fail Count ")
                        strBin = "FAIL";
                    else
                    if(!strString.section(" ",0,0).simplified().isEmpty())
                        strBin = strString.section(" ",0,0);
                    */
                    if(!(ptTest && (ptTest->m_nNumber == nTestNum) && (ptTest->m_strName == strTestName)))
                    {
                        ptTest = new CGNd1Nd2DatalogParameter();
                        ptTest->m_strName = strTestName;
                        ptTest->m_nNumber = nTestNum;
                        ptTest->m_bValidHighLimit = ptTest->m_bValidLowLimit = false;
                        ptTest->m_fHighLimit = ptTest->m_fLowLimit = 0;
                        ptTest->m_nScale = 0;

                        if(!m_bFlagGenerateMpr)
                            ptTest->m_bStaticHeaderWritten = false; // always write test name in FTR record
                        else
                            ptTest->m_bStaticHeaderWritten = m_lstParametersStaticHeaderWritten.contains(strTestType + ptTest->m_strName);

                        m_lstpParameterFlow.append(ptTest);
                    }

                    // Result
                    ptTest->m_mapBin[nSiteId] = strBin;
                    ptTest->m_mapValue[nSiteId]["VECT_NAME"] = strPaternName;
                }
            }
            else
            {
                // Parametric test
                strBin = "PASS";

                // goto Result
                while(!hNd1Nd2DatalogFile.atEnd())
                {
                    strString = ReadLine(hNd1Nd2DatalogFile);
                    if(strString.left(4) == "****")
                        break;
                    if(strString.left(5) == "DUT :")
                        break;
                    if(strString.left(7) == "TestID ")
                        break;

                }

                if(strString.left(4) == "****")
                    break;
                if(strString.left(5) == "DUT :")
                    break;

        //FIXME: not used ?
        /*nPosTestId = 0;
        nPosResult = strString.indexOf("TestID") + 6;*/

        int lLenghtPin = 4;
        nPosValue = strString.indexOf("RESULT") + 6;
                nPosHLimit = strString.indexOf("Value") + 5;
                nPosLLimit = strString.indexOf("UP_LIM") + 6;
                nPosPin = strString.indexOf("LO_LIM") + 6;
                nPosDUT = strString.indexOf("pin") + 3;
                if(nPosDUT < 3)
                {
                    nPosDUT = nPosPin;
                    lLenghtPin = 5;
                    nPosPin = -1;
                    strTestType = "PTR";
                }
                else
                    strTestType = "MPR";

                while(!hNd1Nd2DatalogFile.atEnd())
                {
                    // PTR AND MPR
                    strString = ReadLine(hNd1Nd2DatalogFile);
                    if(strString.left(4) == "****")
                        break;
                    if(strString.left(5) == "DUT :")
                        break;
                    /*
                    if((strBin == "PASS")
                    && !strString.mid(nPosResult,nPosValue-nPosResult).trimmed().isEmpty())
                        strBin = strString.mid(nPosResult,nPosValue-nPosResult).trimmed();
                    */


                    if(!strString.mid(nPosDUT,lLenghtPin).trimmed().isEmpty())
                        nSiteId = strString.mid(nPosDUT,lLenghtPin).trimmed().toInt();

                    if(!lstSites.contains(QString::number(nSiteId)))
                        lstSites.append(QString::number(nSiteId));



                    strUnit = strString.mid(nPosValue,nPosHLimit-nPosValue).trimmed();
                    NormalizeValues(strUnit,fResult,nScale,bHaveResult);

                    if(nPosPin < 0)
                        strPinName = "NO-PIN-DATA";
                    else
                        strPinName = strString.mid(nPosPin, nPosDUT-nPosPin).trimmed();

                    if(!(ptTest && (ptTest->m_nNumber == nTestNum) && (ptTest->m_strName == strTestName)))
                    {
                        ptTest = new CGNd1Nd2DatalogParameter();
                        ptTest->m_strName = strTestName;
                        ptTest->m_nNumber = nTestNum;
                        ptTest->m_bValidHighLimit = ptTest->m_bValidLowLimit = false;
                        ptTest->m_fHighLimit = ptTest->m_fLowLimit = 0;
                        ptTest->m_nScale = nScale;
                        ptTest->m_strUnits = strUnit;

                        if(!m_bFlagGenerateMpr)
                            ptTest->m_bStaticHeaderWritten = false; // always write test name in PTR record
                        else
                            ptTest->m_bStaticHeaderWritten = m_lstParametersStaticHeaderWritten.contains(strTestType + ptTest->m_strName);

                        // Check if already saved
                        if(!m_lstParametersStaticHeaderWritten.contains(strTestType + strTestName))
                        {
                            strValue = strString.mid(nPosHLimit,nPosLLimit-nPosHLimit).trimmed();
                            NormalizeValues(strValue,ptTest->m_fHighLimit,i,ptTest->m_bValidHighLimit);
                            if(nPosPin < 0)
                                strValue = strString.mid(nPosLLimit,nPosDUT-nPosLLimit).trimmed();
                            else
                                strValue = strString.mid(nPosLLimit,nPosPin-nPosLLimit).trimmed();
                            NormalizeValues(strValue,ptTest->m_fLowLimit,i,ptTest->m_bValidLowLimit);
                        }

                        m_lstpParameterFlow.append(ptTest);
                    }

                    // Result
                    ptTest->m_mapBin[nSiteId] = strBin;
                    ptTest->m_mapValue[nSiteId][strPinName] = QString::number(fResult);


                }
            }
        }
        else
            strString = "";

    }

    // Write WRR for last wafer inserted
    RecordReadInfo.iRecordType = 2;
    RecordReadInfo.iRecordSubType = 20;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteByte(1);						// Test head
    StdfFile.WriteByte(255);					// Tester site (all)
    StdfFile.WriteDword(0);						// Time of last part tested
    StdfFile.WriteDword(iTotalPart);			// Parts tested: always 5
    StdfFile.WriteDword(0);						// Parts retested
    StdfFile.WriteDword(0);						// Parts Aborted
    StdfFile.WriteDword(iTotalGoodBin);			// Good Parts
    StdfFile.WriteDword(4294967295UL);			// Functionnal Parts
    StdfFile.WriteString(m_strWaferId.toLatin1().constData());	// WaferID
    StdfFile.WriteRecord();

    QMap<QString,CNd1Nd2DatalogBinInfo>::Iterator itMapBin;
    QMap<int, int>::Iterator itBinCount;

    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 40;
    for ( itMapBin = m_mapHardBinsCount.begin(); itMapBin != m_mapHardBinsCount.end(); ++itMapBin )
    {
        // Write HBR for each site
        for ( itBinCount = itMapBin.value().m_nCount.begin(); itBinCount != itMapBin.value().m_nCount.end(); itBinCount++)
        {
            StdfFile.WriteHeader(&RecordReadInfo);
            StdfFile.WriteByte(1);							// Test Head = ALL
            StdfFile.WriteByte(itBinCount.key());			// Test sites = ALL
            StdfFile.WriteWord(itMapBin.value().m_nBinNum);	// HBIN
            StdfFile.WriteDword(itBinCount.value());			// Total Bins
            if(itMapBin.value().m_strBinName.endsWith("PASS"))
                StdfFile.WriteByte('P');
            else
                StdfFile.WriteByte('F');
            StdfFile.WriteString(itMapBin.value().m_strBinName.toLatin1().constData());
            StdfFile.WriteRecord();
        }
    }

    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 50;

    for ( itMapBin = m_mapSoftBinsCount.begin(); itMapBin != m_mapSoftBinsCount.end(); ++itMapBin )
    {
        // Write SBRfor each site
        for ( itBinCount = itMapBin.value().m_nCount.begin(); itBinCount != itMapBin.value().m_nCount.end(); itBinCount++)
        {

            StdfFile.WriteHeader(&RecordReadInfo);
            StdfFile.WriteByte(1);						// Test Head
            StdfFile.WriteByte(itBinCount.key());		// Test sites
            StdfFile.WriteWord(itMapBin.value().m_nBinNum);			// SBIN = 0
            StdfFile.WriteDword(itBinCount.value());	// Total Bins
            if(itMapBin.value().m_strBinName.endsWith("PASS"))
                StdfFile.WriteByte('P');
            else
                StdfFile.WriteByte('F');
            StdfFile.WriteString(itMapBin.value().m_strBinName.toLatin1().constData());
            StdfFile.WriteRecord();
        }
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
// Convert 'FileName' Nd1Nd2Datalog file, to STDF 'strFileNameSTDF' file
//////////////////////////////////////////////////////////////////////
bool CGNd1Nd2DatalogtoSTDF::Convert(const char *Nd1Nd2DatalogFileName, const char *strFileNameSTDF)
{
    // No erro (default)
    m_iLastError = errNoError;

    // If STDF file already exists...do not rebuild it...unless dates not matching!
    QFileInfo fInput(Nd1Nd2DatalogFileName);
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

    if(GexScriptStatusLabel != NULL)
    {
        if(GexScriptStatusLabel->isHidden())
        {
            bHideLabelAfter = true;
            GS::Gex::Engine::GetInstance().UpdateLabelStatus("Converting data from file "+QFileInfo(Nd1Nd2DatalogFileName).fileName()+"...");
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

    if(ReadNd1Nd2DatalogFile(Nd1Nd2DatalogFileName,strFileNameSTDF) != true)
    {
        //////////////////////////////////////////////////////////////////////
        // For ProgressBar
        if((GexProgressBar != NULL)
        && bHideProgressAfter)
            GexProgressBar->hide();

        if((GexScriptStatusLabel != NULL)
        && bHideLabelAfter)
            GexScriptStatusLabel->hide();

        return false;	// Error reading Nd1Nd2Datalog file
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
//return lDateTime from string strDateTime "Mar 26 20:32 2010" = 2010 May 26, 20:32:00.
//////////////////////////////////////////////////////////////////////
long CGNd1Nd2DatalogtoSTDF::GetDateTimeFromString(QString strDateTime)
{
    int		nYear, nMonth, nDay;
    int		nHour, nMin, nSec;
    long	lDateTime;
    QString strMonth;
    QString strDT = strDateTime;

    if(strDT.length()<17)
        return 0;

    nYear = strDT.section(" ",3,3).toInt();
    strMonth = strDT.section(" ",0,0);
    nDay = strDT.section(" ",1,1).toInt();
    nHour = strDT.section(" ",2,2).section(":",0,0).toInt();
    nMin= strDT.section(" ",2,2).section(":",1,1).toInt();
    nSec = 0;

  if      (strMonth == "Jan") nMonth = 1;
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

//////////////////////////////////////////////////////////////////////
// Read line : skip empty line
//////////////////////////////////////////////////////////////////////
QString CGNd1Nd2DatalogtoSTDF::ReadLine(QTextStream& hFile)
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
