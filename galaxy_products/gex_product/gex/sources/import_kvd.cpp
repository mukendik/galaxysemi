//////////////////////////////////////////////////////////////////////
// import_kvd.cpp: Convert a kvd file to STDF V4.0
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


#include "import_kvd.h"
#include "import_constants.h"
#include "engine.h"

// File format: Data log
//----------------------------------------------------
//---------------BEGIN DATA LOG ----------------------
//---------------Start Log Time : 9/5/2005 6:23:20 PM
//----------------------------------------------------
//----------------------------------------------------
//Program ID     : C:\KVDTestPrograms\Dotpos test programs\D1445BB_V2_4_PRODUCTION_65LUX\Dotpos_V0.0.exe (09/01/05 09:09:30)
//Operator ID    :
//Tester         : DE-TEST-KVD01
//Fixture        :
//Lot ID         : NOLOTNUM
//Wafer Number   : 18
//Comment        :
//LimitsFile     : C:\KVDTestPrograms\Dotpos test programs\D1445BB_V2_4_PRODUCTION_65LUX\./limitSetups/Dotpos_wafer.lim (09/01/05 08:54:32)
//BinFile        : ./iniFiles/Dotpos.bin (04/11/05 17:54:22)
//ParametersFile : C:\KVDTestPrograms\Dotpos test programs\D1445BB_V2_4_PRODUCTION_65LUX\dotpos.inf (12/07/04 14:08:46)
//LibraryVersion : Version 05_02_Release_5
//----------------------------------------------------
//
//DEVICE(s) : 1
//Site 0 Wafer pos : X=23, Y=51
//S0  T5      SELF_MIDL_ILLUM                    9.825804  uW/cm2      8.000     12.000                        CTT :   0.091s   DTT :   0.091s
//S0  T6      CHUCK_TEMP                        55.400000  C        -1.000     80.000                        CTT :   0.091s   DTT :   0.000s
//S0  T7      CHUCK_TEMP_Goal                   55.000000  C        -1.000     80.000                        CTT :   0.091s   DTT :   0.000s
//S0  T10     CONT_OP_GPIO_0                    -0.386226  V        -0.750     -0.250                        CTT :   0.103s   DTT :   0.012s


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
CGKVDtoSTDF::CGKVDtoSTDF()
{
    m_lStartTime = 0;
}

//////////////////////////////////////////////////////////////////////
// Get Error
//////////////////////////////////////////////////////////////////////
QString CGKVDtoSTDF::GetLastError()
{
    m_strLastError = "Import KVD: ";

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
void CGKVDtoSTDF::NormalizeValues(QString &strUnits,float &fValue, int &nScale)
{
    QString strValue = strUnits;

    nScale = 0;

    // In strValue, the current value with unit
    if(strValue == "-")
    {
        strUnits = "";
        return;
    }

    if(strValue.toUpper() == "P/F")
    {
        return;
    }

    if(strValue.toUpper() == "_%")
    {
        strUnits = "%";
        return;
    }

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
    fValue *= GS_POW(10.0,nScale);
    if(nScale)
        strUnits = strUnits.mid(1);	// Take all characters after the prefix.
}

//////////////////////////////////////////////////////////////////////
// Check if File is compatible with KVD format
//////////////////////////////////////////////////////////////////////
bool CGKVDtoSTDF::IsCompatible(const char *szFileName)
{
    QString strString;

    // Open hKVDFile file
    QFile f( szFileName );
    if(!f.open( QIODevice::ReadOnly ))
    {
        // Failed Opening KVD file
        return false;
    }
    // Assign file I/O stream
    QTextStream hKVDFile(&f);
    bool bReturn = false;

    // Check if first line is the correct KVD header...
    //----------------------------------------------------
    //---------------BEGIN DATA LOG ----------------------
    //---------------Start Log Time : 9/5/2005 6:23:20 PM
    //----------------------------------------------------
    //----------------------------------------------------
    //Program ID     : C:\KVDTestPrograms\Dotpos test programs\D1445BB_V2_4_PRODUCTION_65LUX\Dotpos_V0.0.exe (09/01/05 09:09:30)

    do
        strString = hKVDFile.readLine().remove("-").simplified();
    while(!strString.isNull() && strString.isEmpty());

    if(strString.startsWith("BEGIN DATA LOG", Qt::CaseInsensitive))
    {
        // Read next line
        strString = hKVDFile.readLine().remove("-").simplified();
        if(strString.startsWith("Start Log Time", Qt::CaseInsensitive))
            bReturn = true;
    }

    // Close file
    f.close();

    return bReturn;
}

//////////////////////////////////////////////////////////////////////
// Read and Parse the KVD file
//////////////////////////////////////////////////////////////////////
bool CGKVDtoSTDF::ReadKVDFile(const char *KVDFileName,const char *strFileNameSTDF)
{
    QString strString;
    QString strSection;

    // Open KVD file

    QFile f( KVDFileName );
    if(!f.open( QIODevice::ReadOnly ))
    {
        // Failed Opening KVD file
        m_iLastError = errOpenFail;

        // Convertion failed.
        return false;
    }

    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    iFileSize = f.size() + 1;

    // Assign file I/O stream
    QTextStream hKVDFile(&f);

    // Check if first line is the correct KVD header...
    //---------------BEGIN DATA LOG ----------------------
    //---------------Start Log Time : 9/5/2005 6:23:20 PM

    m_iLastError = errInvalidFormat;
    // Read first non-empty line
    do
        strString = ReadLine(hKVDFile).remove("-").simplified();
    while(!strString.isNull() && strString.isEmpty());

    // Incorrect header...this is not a KVD file!
    // Convertion failed.
    if(!strString.startsWith("BEGIN DATA LOG", Qt::CaseInsensitive))
    {
        // Close file
        f.close();
        return false;
    }

    strString = ReadLine(hKVDFile).remove("-").simplified();
    if(!strString.startsWith("Start Log Time", Qt::CaseInsensitive))
    {
        // Close file
        f.close();
        return false;
    }

    strString = strString.section(":",1).simplified();
    m_lStartTime = GetDateTimeFromString(strString);

    do
        strString = ReadLine(hKVDFile).remove("-").simplified();
    while(!strString.isNull() && strString.isEmpty());
    // Read KVD information

    while(!hKVDFile.atEnd())
    {
        if(strString.isEmpty())
            strString = ReadLine(hKVDFile).trimmed();

        // Read header
        if(strString.startsWith("------------------"))
        {
            break;
        }

        strSection = strString.section(":",0,0).simplified().toUpper();
        strString = strString.section(":",1).simplified();

        if(strSection == "LOT ID")
        {
            m_strLotId = strString;
        }
        else if(strSection == "TESTER")
        {
            m_strTesterId = strString;
        }
        else if(strSection == "PROGRAM ID")
        {
            m_strProgramId = strString;
        }
        else if(strSection == "LIBRARYVERSION")
        {
            m_strExecType = strString;
        }
        else if(strSection == "OPERATOR ID")
        {
            m_strOperatorId = strString;
        }
        else if(strSection == "WAFER NUMBER")
        {
            m_strWaferId = strString;
        }

        strString = "";
    }

    // Convertion failed.
    if(hKVDFile.atEnd())
    {
        // Close file
        f.close();
        return false;
    }



    if(WriteStdfFile(hKVDFile,strFileNameSTDF) != true)
    {
        QFile::remove(strFileNameSTDF);
        // Close file
        f.close();
        return false;
    }

    // Success parsing KVD file
    m_iLastError = errNoError;
    return true;
}

//////////////////////////////////////////////////////////////////////
// Create STDF file from KVD data parsed
//////////////////////////////////////////////////////////////////////
bool CGKVDtoSTDF::WriteStdfFile(QTextStream &hKVDFile,const char *strFileNameSTDF)
{
    // now generate the STDF file...
    GS::StdLib::Stdf StdfFile;
    GS::StdLib::StdfRecordReadInfo RecordReadInfo;
    if(StdfFile.Open((char*)strFileNameSTDF,STDF_WRITE) != GS::StdLib::Stdf::NoError)
    {
        // Failed importing KVD file into STDF database
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
    StdfFile.WriteString("");					// Part Type / Product ID
    StdfFile.WriteString(m_strTesterId.toLatin1().constData());			// Node name
    StdfFile.WriteString("KVD");				// Tester Type
    StdfFile.WriteString(m_strProgramId.toLatin1().constData());		// Job name
    StdfFile.WriteString("");					// Job rev
    StdfFile.WriteString("");					// sublot-id
    StdfFile.WriteString("");					// operator
    StdfFile.WriteString(m_strExecType.toLatin1().constData());		// exec-type
    StdfFile.WriteString("");					// exe-ver
    StdfFile.WriteString("");					// test-cod
    StdfFile.WriteString("");					// test-temperature
    // Construct custom Galaxy USER_TXT
    QString	strUserTxt;
    strUserTxt = GEX_IMPORT_DATAORIGIN_LABEL;
    strUserTxt += ":";
    strUserTxt += GEX_IMPORT_DATAORIGIN_ATETEST;
    strUserTxt += ":KVD";
    StdfFile.WriteString(strUserTxt.toLatin1().constData());		// user-txt
    StdfFile.WriteString("");					// aux-file
    StdfFile.WriteString("");					// package-type
    StdfFile.WriteString("");					// familyID
    StdfFile.WriteString("");					// Date-code
    StdfFile.WriteString("");					// Facility-ID
    StdfFile.WriteString("");					// FloorID
    StdfFile.WriteString("");					// ProcessID

    StdfFile.WriteRecord();

    QString			strString;
    QString			strTestName;
    int				nTestNum;
    float			fValue;
    float			fLL;
    float			fHL;
    int				nScale;
    QString			strUnit;
    int				nSiteId;
    int				nXpos,nYpos;
    BYTE			bData;
    bool			bHaveLL, bHaveHL;
    bool			bIsNumber;
    bool			bPassStatus=false;
    bool			bTestPass;
    int				nBin, nTotalTests;
    QString			strPartNumber;
    int				nTotalGoodBin, nTotalFailBin, nTotalPart;
    unsigned int	iIndex;


    bData = 0;
    nBin = nSiteId = nTestNum = 0;
    nTotalTests = nTotalPart = nTotalFailBin = nTotalGoodBin = 0;
    nXpos = nYpos = -32768;

    if(!m_strWaferId.isEmpty())
    {
        // Write WIR of new Wafer.
        RecordReadInfo.iRecordType = 2;
        RecordReadInfo.iRecordSubType = 10;
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte(1);									// Test head
        StdfFile.WriteByte(255);								// Tester site (all)
        StdfFile.WriteDword(m_lStartTime);						// Start time
        StdfFile.WriteString(m_strWaferId.toLatin1().constData());	// WaferID
        StdfFile.WriteRecord();
    }

    // Write Test results for each PartID
    while(!hKVDFile.atEnd())
    {
        strString = ReadLine(hKVDFile).trimmed();

        // Read header
        if(strString.startsWith("------"))
        {
            //----------------------------------------------------
            break;
        }
        else
            if(strString.startsWith("DEVICE(s) ", Qt::CaseInsensitive))
            {
                //DEVICE(s) : 2
                strPartNumber = strString.section(":",1).simplified();
                nTotalPart++;
            }
            else
        if(strString.startsWith("Site ", Qt::CaseInsensitive))
        {
            //Site 0 Wafer pos : X=31, Y=48
            nSiteId = strString.section(" ",1,1).toInt();
            strString = strString.section(" ",2);
            if(strString.startsWith("Wafer pos",Qt::CaseInsensitive))
            {
                strString = strString.section(":",1);
                nXpos = strString.section(",",0,0).section("=",1).toInt(&bIsNumber);
                if(!bIsNumber)
                    nXpos = -32768;
                nYpos = strString.section(",",1).section("=",1).toInt(&bIsNumber);
                if(!bIsNumber)
                    nYpos = -32768;
            }

            // Write PIR for parts in this Wafer site
            RecordReadInfo.iRecordType = 5;
            RecordReadInfo.iRecordSubType = 10;
            StdfFile.WriteHeader(&RecordReadInfo);
            StdfFile.WriteByte(1);							// Test head
            StdfFile.WriteByte(nSiteId);					// Tester site
            StdfFile.WriteRecord();
        }
        else
        if(strString.startsWith("S"+QString::number(nSiteId)+" ", Qt::CaseInsensitive))
        {
            // S0  T5      SELF_MIDL_ILLUM                    9.824249  uW/cm2      8.000     12.000                        CTT :   0.091s   DTT :   0.091s

            while(!hKVDFile.atEnd())
            {
                if(strString.startsWith("Site ",Qt::CaseInsensitive))
                {
                    //Site 0  FAIL  Bin 9
                    strString = strString.simplified();
                    bPassStatus = (strString.section(" ",2,2).toUpper() == "PASS");
                    nBin = strString.toUpper().section("BIN ",1).simplified().toInt();

                    break;

                }
                nTestNum = strString.mid(3,8).toUpper().remove("T").simplified().toInt();
                strTestName = strString.mid(12,30).simplified();
                fValue = strString.mid(42,14).toFloat();
                strUnit = strString.mid(57,6).simplified();

                // adjust next index pos
                iIndex = 63;
                if(strUnit.length() > 4)
                    iIndex += strUnit.length() - 4;

                NormalizeValues(strUnit,fValue,nScale);

                bTestPass = (strString.mid(iIndex + 21,20).simplified().toUpper() != "FAIL");

                // Write PTR
                RecordReadInfo.iRecordType = 15;
                RecordReadInfo.iRecordSubType = 10;

                StdfFile.WriteHeader(&RecordReadInfo);
                StdfFile.WriteDword(nTestNum);			// Test Number
                StdfFile.WriteByte(1);					// Test head
                StdfFile.WriteByte(nSiteId);			// Tester site:1,2,3,4 or 5, etc.
                if(bTestPass)
                    bData = 0;		// Test passed
                else
                    bData = BIT7;	// Test Failed
                StdfFile.WriteByte(bData);							// TEST_FLG
                bData = BIT6|BIT7;
                StdfFile.WriteByte(bData);							// PARAM_FLG
                StdfFile.WriteFloat(fValue);		// Test result
                if(!m_lstParametersStaticHeaderWritten.contains(strTestName))
                {
                    m_lstParametersStaticHeaderWritten.append(strTestName);

                    // save Parameter name
                    StdfFile.WriteString(strTestName.toLatin1().constData());	// TEST_TXT
                    StdfFile.WriteString("");							// ALARM_ID

                    strUnit = strString.mid(57,6).simplified();
                    fLL = strString.mid(iIndex,12).toFloat(&bHaveLL);
                    NormalizeValues(strUnit,fLL,nScale);

                    strUnit = strString.mid(57,6).simplified();
                    fHL = strString.mid(iIndex+12,12).toFloat(&bHaveHL);
                    NormalizeValues(strUnit,fHL,nScale);

                    bData = 2;	// Valid data.
                    if(!bHaveLL)
                        bData |= BIT6;
                    if(!bHaveHL)
                        bData |= BIT7;
                    StdfFile.WriteByte(bData);							// OPT_FLAG

                    StdfFile.WriteByte(-nScale);						// RES_SCALE
                    StdfFile.WriteByte(-nScale);							// LLM_SCALE
                    StdfFile.WriteByte(-nScale);						// HLM_SCALE
                    StdfFile.WriteFloat(fLL);							// LOW Limit
                    StdfFile.WriteFloat(fHL);							// HIGH Limit
                    StdfFile.WriteString(strUnit.toLatin1().constData());		// Units
                }
                StdfFile.WriteRecord();

                strString = ReadLine(hKVDFile);

            }


        }
        else
        if(strString.startsWith("Test Time ", Qt::CaseInsensitive))
        {
            //Test Time    7.472s

            fValue = strString.simplified().section(" ",2).remove("s").toFloat();
            fValue *= 1000;
            m_lStartTime += (long) fValue;

            if(!m_mapBinsCount.contains(nBin))
            {
                m_mapBinsCount[nBin].m_nCount[nSiteId] = 0;
                m_mapBinsCount[nBin].bPass = bPassStatus;
            }
            if(!m_mapBinsCount[nBin].m_nCount.contains(nSiteId))
                m_mapBinsCount[nBin].m_nCount[nSiteId] = 0;

            m_mapBinsCount[nBin].m_nCount[nSiteId]++;

            // Write PRR
            RecordReadInfo.iRecordType = 5;
            RecordReadInfo.iRecordSubType = 20;
            StdfFile.WriteHeader(&RecordReadInfo);
            StdfFile.WriteByte(1);					// Test head
            StdfFile.WriteByte(nSiteId);		// Tester site:1,2,3,4 or 5
            if(bPassStatus == true)
            {
                StdfFile.WriteByte(0);				// PART_FLG : PASSED
                nTotalGoodBin++;
            }
            else
            {
                StdfFile.WriteByte(8);				// PART_FLG : FAILED
                nTotalFailBin++;
            }
            StdfFile.WriteWord((WORD)nTotalTests);	// NUM_TEST
            StdfFile.WriteWord(nBin);				// HARD_BIN
            StdfFile.WriteWord(nBin);				// SOFT_BIN
            StdfFile.WriteWord(nXpos);				// X_COORD
            StdfFile.WriteWord(nYpos);				// Y_COORD
      StdfFile.WriteDword(static_cast<DWORD>(fValue));  // testing time
            StdfFile.WriteString(strPartNumber.toLatin1().constData());// PART_ID
            StdfFile.WriteString("");				// PART_TXT
            StdfFile.WriteString("");				// PART_FIX
            StdfFile.WriteRecord();
        }
    }

    //---------------- END DATA LOG ----------------------
    //---------------- End Log Time : 9/5/2005 6:32:59 PM
    //----------------------------------------------------
    //----------------------------------------------------
    do
        strString = ReadLine(hKVDFile).remove("-").simplified();
    while(!strString.isNull() && strString.isEmpty());

    if(strString.startsWith("END DATA LOG", Qt::CaseInsensitive))
    {
        strString = ReadLine(hKVDFile).remove("-").simplified();
    }

    if(strString.startsWith("End Log Time", Qt::CaseInsensitive))
    {
        strString = strString.section(":",1).simplified();
        m_lStartTime = GetDateTimeFromString(strString);
    }

    if(!m_strWaferId.isEmpty())
    {
        // Write WRR for last wafer inserted
        RecordReadInfo.iRecordType = 2;
        RecordReadInfo.iRecordSubType = 20;
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte(1);						// Test head
        StdfFile.WriteByte(255);					// Tester site (all)
        StdfFile.WriteDword(m_lStartTime);			// Time of last part tested
        StdfFile.WriteDword(nTotalGoodBin + nTotalFailBin);			// Parts tested
        StdfFile.WriteDword(0);						// Parts retested
        StdfFile.WriteDword(0);						// Parts Aborted
        StdfFile.WriteDword(nTotalGoodBin);			// Good Parts
        StdfFile.WriteDword(4294967295UL);			// Functionnal Parts
        StdfFile.WriteString(m_strWaferId.toLatin1().constData());	// WaferID
        StdfFile.WriteRecord();
    }


    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 40;
    QMap<int,CKVDBinInfo>::Iterator itMapBin;
    QMap<int, int>::Iterator itBinCount;
    for ( itMapBin = m_mapBinsCount.begin(); itMapBin != m_mapBinsCount.end(); ++itMapBin )
    {
        // Write HBR for each site
        for ( itBinCount = itMapBin.value().m_nCount.begin(); itBinCount != itMapBin.value().m_nCount.end(); itBinCount++)
        {
            StdfFile.WriteHeader(&RecordReadInfo);
            StdfFile.WriteByte(1);							// Test Head = ALL
            StdfFile.WriteByte(itBinCount.key());			// Test sites = ALL
            StdfFile.WriteWord(itMapBin.key());				// HBIN
            StdfFile.WriteDword(itBinCount.value());			// Total Bins
            if(m_mapBinsCount[itMapBin.key()].bPass)
                StdfFile.WriteByte('P');
            else
                StdfFile.WriteByte('F');
            StdfFile.WriteString("");
            StdfFile.WriteRecord();
        }
    }

    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 50;

    for ( itMapBin = m_mapBinsCount.begin(); itMapBin != m_mapBinsCount.end(); ++itMapBin )
    {
        // Write SBRfor each site
        for ( itBinCount = itMapBin.value().m_nCount.begin(); itBinCount != itMapBin.value().m_nCount.end(); itBinCount++)
        {

            StdfFile.WriteHeader(&RecordReadInfo);
            StdfFile.WriteByte(1);						// Test Head
            StdfFile.WriteByte(itBinCount.key());		// Test sites
            StdfFile.WriteWord(itMapBin.key());			// SBIN = 0
            StdfFile.WriteDword(itBinCount.value());	// Total Bins
            if(m_mapBinsCount[itMapBin.key()].bPass)
                StdfFile.WriteByte('P');
            else
                StdfFile.WriteByte('F');
            StdfFile.WriteString("");
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
// Convert 'FileName' KVD file, to STDF 'strFileNameSTDF' file
//////////////////////////////////////////////////////////////////////
bool CGKVDtoSTDF::Convert(const char *KVDFileName, const char *strFileNameSTDF)
{
    // No erro (default)
    m_iLastError = errNoError;

    // If STDF file already exists...do not rebuild it...unless dates not matching!
    QFileInfo fInput(KVDFileName);
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
            GS::Gex::Engine::GetInstance().UpdateLabelStatus("Converting data from file "+QFileInfo(KVDFileName).fileName()+"...");
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

    if(ReadKVDFile(KVDFileName,strFileNameSTDF) != true)
    {
        //////////////////////////////////////////////////////////////////////
        // For ProgressBar
        if((GexProgressBar != NULL)
        && bHideProgressAfter)
            GexProgressBar->hide();

        if((GexScriptStatusLabel != NULL)
        && bHideLabelAfter)
            GexScriptStatusLabel->hide();

        return false;	// Error reading KVD file
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
QString CGKVDtoSTDF::ReadLine(QTextStream& hFile)
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

//////////////////////////////////////////////////////////////////////
//return lDateTime from string strDateTime "03/19/2010, 02:55:16 PM".
//////////////////////////////////////////////////////////////////////
long CGKVDtoSTDF::GetDateTimeFromString(QString strDateTime)
{
    int nYear, nMonth, nDay;
    int	nHour, nMin, nSec;
    long lDateTime;

    if(strDateTime.length()<18)
        return 0;

    QString strDate = strDateTime.section(" ",0,0);
    QString strTime = strDateTime.section(" ",1);

    nMonth = strDate.section("/",0,0).toInt();
    nDay = strDate.section("/",1,1).toInt();
    nYear = strDate.section("/",2,2).toInt();
    nHour = strTime.section(":",0,0).toInt();
    nMin= strTime.section(":",1,1).toInt();
    nSec = strTime.section(":",2,2).left(2).toInt();

    if(strTime.endsWith("PM",Qt::CaseInsensitive))
        nHour += 12;

    QDate clDate(nYear,nMonth,nDay);
    QTime clTime(nHour,nMin,nSec);
    QDateTime clDateTime(clDate,clTime);

    clDateTime.setTimeSpec(Qt::UTC);
    lDateTime = clDateTime.toTime_t();
    return lDateTime;
}
