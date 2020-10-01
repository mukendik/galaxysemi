//////////////////////////////////////////////////////////////////////
// import_shibasoku_dlk.cpp: Convert a SHIBASOKU DLK file to STDF V4.0
//////////////////////////////////////////////////////////////////////

#include "gqtl_global.h"
#include <qmath.h>
#include <time.h>

#ifdef _WIN32
#include "windows.h"		// For 'GetWindowsDirectoryA' declaration
#endif

#include <qfileinfo.h>
#include <QDir>
#include <qprogressbar.h>
#include <qapplication.h>
#include <qlabel.h>

#include "import_shibasoku_dlk.h"
#include "import_constants.h"
#include "engine.h"

extern QLabel			*GexScriptStatusLabel;	// Handle to script status text in status bar
extern QProgressBar	*	GexProgressBar;		// Handle to progress bar in status bar

// File format:
//_WL25_DL0
//WSS45J,wss45j3
//H
//1,1,2,BAW,16/11/10,09:50
//W
//1,7,1
//D
//1,P,VF(1A),481.000, mV,200.000,580.000,0.00
//2,F,VR(10uA),160.4, V,165.0,210.0,0.00
//5,P,DV5-0.5u,-1.5, V,-3.0,3.0,0.00
//5,P,DV5-0.6u,-1.0, V,-3.0,3.0,0.00
//6,P,1uA,34.0, V,5.0,100.0,0.00
//6,F,10uA,162.3, V,165.0,210.0,0.00
//6,F,100uA,160.4, V,165.0,210.0,0.00
//6,F,1000uA,163.4, V,165.0,210.0,0.00
//3,P,IR(160V),149.80, nA,-10.00,200.00,0.00
//R
//F,0,0,0.00

#define BIT0			0x01
#define BIT1			0x02
#define BIT2			0x04
#define BIT3			0x08
#define BIT4			0x10
#define BIT5			0x20
#define BIT6			0x40
#define BIT7			0x80


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGDLKtoSTDF::CGDLKtoSTDF()
{
    // Default: shibasoku_dlk parameter list on disk includes all known shibasoku_dlk parameters...
    m_bNewDLKParameterFound = false;
    m_lStartTime = 0;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGDLKtoSTDF::~CGDLKtoSTDF()
{
}

//////////////////////////////////////////////////////////////////////
// Get Error
//////////////////////////////////////////////////////////////////////
QString CGDLKtoSTDF::GetLastError()
{
    m_strLastError = "Import SHIBASOKU DLK: ";

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
void CGDLKtoSTDF::NormalizeLimits(QString &strUnit, int &nScale)
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
// Check if File is compatible with DLK format
//////////////////////////////////////////////////////////////////////
bool CGDLKtoSTDF::IsCompatible(const char *szFileName)
{
    QString strString;

    // Open DLK file
    QFile f( szFileName );
    if(!f.open( QIODevice::ReadOnly ))
    {
        // Failed Opening DLK file
        return false;
    }
    // Assign file I/O stream
    QTextStream hDLKFile(&f);

    // Check if first line is the correct DLK header...
    //_WL25_DL0
    //WSS45J,wss45j3
    //H

    do
        strString = hDLKFile.readLine();
    while(!strString.isNull() && strString.isEmpty());

    f.close();

    if(!strString.simplified().endsWith("_DL0",Qt::CaseInsensitive))
    {
        // Incorrect header...this is not a DLK file!
        return false;
    }


    return true;
}

//////////////////////////////////////////////////////////////////////
// Read and Parse the DLK file
//////////////////////////////////////////////////////////////////////
bool CGDLKtoSTDF::ReadDLKFile(const char* DLKFileName,
                              const char* /*strFileNameSTDF*/)
{
    QString strString;
    QString strSection;

    // Open DLK file
    QFile f( DLKFileName );
    if(!f.open( QIODevice::ReadOnly ))
    {
        // Failed Opening DLK file
        m_iLastError = errOpenFail;

        // Convertion failed.
        return false;
    }

    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    iNextFilePos = 0;
    iProgressStep = 0;
    iFileSize = f.size() + 1;


    // Assign file I/O stream
    QTextStream hDLKFile(&f);

    // Check if first line is the correct DLK header...
    //_WL25_DL0

    strString = ReadLine(hDLKFile);

    if(!strString.simplified().endsWith("_DL0",Qt::CaseInsensitive))
    {
        // Incorrect header...this is not a DLK file!
        m_iLastError = errInvalidFormat;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }

    // Read DLK information
    //_WL25_DL0
    m_strNodeId = strString.section("_",1,1);
    m_strTesterType = m_strExecType = strString;

    //WSS45J,wss45j3
    strString = ReadLine(hDLKFile);
    m_strProgramId = strString.section(",",0,0);
    m_strLotId = strString.section(",",1);

    //H
    //1,1,2,BAW,16/11/10,09:50
    strString = ReadLine(hDLKFile).simplified().toUpper();
    if(strString != "H")
    {
        // Incorrect header...this is not a DLK file!
        m_iLastError = errInvalidFormat;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }
    strString = ReadLine(hDLKFile).simplified().toUpper();
    //DUT Number : (Variable),
    //Serial Number : (Variable),
    //Station Number : (1 Character),
    //Output Mode : (Variable), *Remark 2
    //Date : (8 Characters), Year/Month/Day each 2 characters
    //Time : (5 Characters)\n Hour:Minute each 2 characters
    if(strString.count(",") != 5)
    {
        // Incorrect header...this is not a DLK file!
        m_iLastError = errInvalidFormat;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }
    m_strTestCod = strString.section(",",3,3).simplified();
    // Date must be in this format Year/Month/Day
    QString strDate = strString.section(",",4,4);
    QString strTime = strString.section(",",5,5);
    // But in the example file is in this format Day/Month/Year
    strDate = "20"+strDate.section("/",2,2)+"-"+strDate.section("/",1,1)+"-"+strDate.section("/",0,0);
    strTime = strTime + ":00";

    QDateTime clDateTime(QDate::fromString(strDate,Qt::ISODate),QTime::fromString(strTime,Qt::ISODate));
    clDateTime.setTimeSpec(Qt::UTC);
    m_lStartTime = clDateTime.toTime_t();

    //W
    //1,7,1
    strString = ReadLine(hDLKFile).simplified().toUpper();
    if(strString != "W")
    {
        // Incorrect header...this is not a DLK file!
        m_iLastError = errInvalidFormat;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }
    strString = ReadLine(hDLKFile).simplified().toUpper();
    //Wafer Number : (Variable),
    //Wafer X : (Variable),
    //Wafer Y : (Variable),
    if(strString.count(",") != 2)
    {
        // Incorrect header...this is not a DLK file!
        m_iLastError = errInvalidFormat;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }
    //D
    //1,P,VF(1A),481.000, mV,200.000,580.000,0.00
    strString = ReadLine(hDLKFile).simplified().toUpper();
    if(strString != "D")
    {
        // Incorrect header...this is not a DLK file!
        m_iLastError = errInvalidFormat;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }

    // It's a DLK file
    // Reset file position and start to write STDF file
    hDLKFile.seek(0);

    if(!WriteStdfFile(hDLKFile))
    {
        f.close();
        return false;
    }

    // Close file
    f.close();

    // Success parsing DLK file
    return true;
}

//////////////////////////////////////////////////////////////////////
// Create STDF file from DLK data parsed
//////////////////////////////////////////////////////////////////////
bool CGDLKtoSTDF::WriteStdfFile(QTextStream &hFile)
{
    // now generate the STDF file...
    GS::StdLib::Stdf StdfFile;
    GS::StdLib::StdfRecordReadInfo RecordReadInfo;
    if(StdfFile.Open((char*)m_strStdfFileName.toLatin1().constData(),STDF_WRITE) != GS::StdLib::Stdf::NoError)
    {
        // Failed importing DLK file into STDF database
        m_iLastError = errWriteSTDF;

        // Convertion failed.
        return false;
    }


    // Write Test results for each line read.
    QString		strString;
    QString		strSection;
    long		iTotalGoodBin,iTotalFailBin;
    long		iTotalTests;
    QString		strPartNumber;
    bool		bPassStatus;
    BYTE		bData;

    int			iHeadNum = 1;
    int			iSiteNum = 1;
    int			iTestNumber;
    bool		bTestPass=false;
    bool		bTestAlarm=false;
    QString		strTestName;
    QString		strUnit;
    int			nScale;
    float		fValue;
    float		fLowLimit, fHighLimit;
    bool		bTestAlreadySaved;
    bool		bHaveLowLimit=false, bHaveHighLimit=false;
    QMap<QString,int> mapTestWithScale;

    bool		bHaveMultiWafers;
    QString		strWaferId;
    int			nPosX = -32768;
    int			nPosY = -32768;

    // Reset counters
    iTotalGoodBin=iTotalFailBin=0;
    bHaveMultiWafers = false;

    int		iHardBin;
    int		iSoftBin;
    bool	bSaveSoftBin;
    QMap<int,int>	mapHardBinCount;
    QMap<int,int>	mapSoftBinCount;
    QMap<int,bool>	mapHardBinPass;
    QMap<int,bool>	mapSoftBinPass;
    // Save only SoftBin when have value diff than 0
    bSaveSoftBin = false;

    // Write all Parameters read on this file : PIR,PTR...,PRR, PIR,PTR..., PRR
    //H
    //1,1,2,BAW,16/11/10,09:50
    //W
    //1,7,1
    //D
    //1,P,VF(1A),481.000, mV,200.000,580.000,0.00
    //2,F,VR(10uA),160.4, V,165.0,210.0,0.00
    //5,P,DV5-0.5u,-1.5, V,-3.0,3.0,0.00
    //5,P,DV5-0.6u,-1.0, V,-3.0,3.0,0.00
    //6,P,1uA,34.0, V,5.0,100.0,0.00
    //6,F,10uA,162.3, V,165.0,210.0,0.00
    //6,F,100uA,160.4, V,165.0,210.0,0.00
    //6,F,1000uA,163.4, V,165.0,210.0,0.00
    //3,P,IR(160V),149.80, nA,-10.00,200.00,0.00
    //R
    //F,0,0,0.00
    // Read DLK information
    while(!hFile.atEnd())
    {
        // read line and remove space
        strString = ReadLine(hFile);

        if(strString.isEmpty())
            continue;

        strSection = strString.section(",",0,0);

        if(strSection == "H")
        {
            //H
            //1,1,2,BAW,16/11/10,09:50
            strString = ReadLine(hFile).simplified().toUpper();
            //DUT Number : (Variable),
            //Serial Number : (Variable),
            //Station Number : (1 Character),
            //Output Mode : (Variable), *Remark 2
            //Date : (8 Characters), Year/Month/Day each 2 characters
            //Time : (5 Characters)\n Hour:Minute each 2 characters
            if(strString.count(",") != 5)
            {
                // Incorrect header...this is not a DLK file!
                m_iLastError = errInvalidFormat;

                // Convertion failed.
                return false;
            }
            iSiteNum = strString.section(",",0,0).simplified().toInt();
            strPartNumber = strString.section(",",1,1).simplified();
            iHeadNum = strString.section(",",2,2).simplified().toInt();
            m_strExecType = strString.section(",",3,3).simplified();
            // Date must be in this format Year/Month/Day
            QString strDate = strString.section(",",4,4);
            QString strTime = strString.section(",",5,5);
            // But in the example file is in this format Day/Month/Year
            strDate = "20"+strDate.section("/",2,2)+"-"+strDate.section("/",1,1)+"-"+strDate.section("/",0,0);
            strTime = strTime + ":00";

            QDateTime clDateTime(QDate::fromString(strDate,Qt::ISODate),QTime::fromString(strTime,Qt::ISODate));
            clDateTime.setTimeSpec(Qt::UTC);
            m_lStartTime = clDateTime.toTime_t();
        }
        else if(strSection == "W")
        {
            //W
            //1,7,1
            strString = ReadLine(hFile).simplified().toUpper();
            //Wafer Number : (Variable),
            //Wafer X : (Variable),
            //Wafer Y : (Variable),
            if(strString.count(",") != 2)
            {
                // Incorrect header...this is not a DLK file!
                m_iLastError = errInvalidFormat;

                // Convertion failed.
                StdfFile.Close();
                return false;
            }
            QString strNextWaferId = strString.section(",",0,0).simplified();
            nPosX = strString.section(",",1,1).simplified().toInt();
            nPosY = strString.section(",",2,2).simplified().toInt();

            if((!strWaferId.isEmpty()) && (strNextWaferId != strWaferId))
            {
                bHaveMultiWafers = true;

                // Write WRR
                // Write WRR
                RecordReadInfo.iRecordType = 2;
                RecordReadInfo.iRecordSubType = 20;
                StdfFile.WriteHeader(&RecordReadInfo);
                StdfFile.WriteByte((BYTE)iHeadNum);											// HEAD_NUM
                StdfFile.WriteByte((BYTE)iSiteNum);											// SITE_GRP
                StdfFile.WriteDword((DWORD)m_lStartTime);									// FINISH_T
                StdfFile.WriteDword((DWORD)iTotalGoodBin+iTotalFailBin);					// PART_CNT
                StdfFile.WriteDword((DWORD)0);												// RTST_CNT
                StdfFile.WriteDword((DWORD)0);												// ABRT_CNT
                StdfFile.WriteDword((DWORD)iTotalGoodBin);									// GOOD_CNT
                StdfFile.WriteDword((DWORD)0);												// FUNC_CNT
                StdfFile.WriteString(strWaferId.toLatin1().constData());					// WAFER_ID
                StdfFile.WriteRecord();

                QMap<int,int>::Iterator it;

                iTotalGoodBin = iTotalFailBin = 0;

                RecordReadInfo.iRecordType = 1;
                RecordReadInfo.iRecordSubType = 40;
                for(it = mapHardBinCount.begin(); it != mapHardBinCount.end(); it++)
                {
                    // Write HBR/site
                    StdfFile.WriteHeader(&RecordReadInfo);
                    StdfFile.WriteByte(255);						// Test Head
                    StdfFile.WriteByte(255);						// Test sites
                    StdfFile.WriteWord(it.key());					// HBIN
                    StdfFile.WriteDword(it.value());					// Total Bins
                    if(mapHardBinPass[it.key()])
                    {
                        StdfFile.WriteByte('P');
                        iTotalGoodBin++;
                    }
                    else
                    {
                        StdfFile.WriteByte('F');
                        iTotalFailBin++;
                    }
                    StdfFile.WriteRecord();
                }

                RecordReadInfo.iRecordType = 1;
                RecordReadInfo.iRecordSubType = 50;
                for(it = mapSoftBinCount.begin(); it != mapSoftBinCount.end(); it++)
                {
                    // Write SBR/site
                    StdfFile.WriteHeader(&RecordReadInfo);
                    StdfFile.WriteByte(255);						// Test Head
                    StdfFile.WriteByte(255);						// Test sites
                    StdfFile.WriteWord(it.key());					// HBIN
                    StdfFile.WriteDword(it.value());					// Total Bins
                    if(mapSoftBinPass[it.key()])
                        StdfFile.WriteByte('P');
                    else
                        StdfFile.WriteByte('F');
                    StdfFile.WriteRecord();
                }

                // Write PCR
                RecordReadInfo.iRecordType = 1;
                RecordReadInfo.iRecordSubType = 30;
                StdfFile.WriteHeader(&RecordReadInfo);
                StdfFile.WriteByte(255);					// Test Head = ALL
                StdfFile.WriteByte(255);					// Test sites = ALL
                StdfFile.WriteDword(iTotalGoodBin+iTotalFailBin);			// Total Parts tested
                StdfFile.WriteDword(0);						// Total Parts re-tested
                StdfFile.WriteDword(0);						// Total Parts aborted
                StdfFile.WriteDword(iTotalGoodBin);			// Total GOOD Parts
                StdfFile.WriteRecord();

                // Write MRR
                RecordReadInfo.iRecordType = 1;
                RecordReadInfo.iRecordSubType = 20;
                StdfFile.WriteHeader(&RecordReadInfo);
                StdfFile.WriteDword(m_lStartTime);			// File finish-time.
                StdfFile.WriteRecord();

                // Close STDF file.
                StdfFile.Close();

                // Rename current STDF file
                QDir clDir;
                QFileInfo clFile(m_strStdfFileName);
                m_lstStdfFileName += clFile.path()+"/"+clFile.baseName() +"_WaferId"+strWaferId+"."+clFile.suffix();
                clDir.rename(m_strStdfFileName,m_lstStdfFileName.last());

                mapTestWithScale.clear();
                mapHardBinCount.clear();
                mapSoftBinCount.clear();
                iTotalGoodBin = iTotalFailBin = 0;

                if(StdfFile.Open((char*)m_strStdfFileName.toLatin1().constData(),STDF_WRITE) != GS::StdLib::Stdf::NoError)
                {
                    // Failed importing DLK file into STDF database
                    m_iLastError = errWriteSTDF;

                    // Convertion failed.
                    return false;
                }
            }
            if(strWaferId.isEmpty() || (strNextWaferId != strWaferId))
            {
                strWaferId = strNextWaferId;

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
                StdfFile.WriteString(m_strNodeId.toLatin1().constData());		// Node name
                StdfFile.WriteString(m_strTesterType.toLatin1().constData());	// Tester Type
                StdfFile.WriteString(m_strProgramId.toLatin1().constData());	// Job name
                StdfFile.WriteString("");					// Job rev
                StdfFile.WriteString("");					// sublot-id
                StdfFile.WriteString(m_strOperatorId.toLatin1().constData());	// operator
                StdfFile.WriteString("");					// exec-type
                StdfFile.WriteString("");					// exe-ver
                StdfFile.WriteString(m_strTestCod.toLatin1().constData());		// test-cod
                StdfFile.WriteString("");					// test-temperature
                // Construct custom Galaxy USER_TXT
                QString	strUserTxt;
                strUserTxt = GEX_IMPORT_DATAORIGIN_LABEL;
                strUserTxt += ":";
                strUserTxt += GEX_IMPORT_DATAORIGIN_ATETEST;
                strUserTxt += ":SHIBASOKU_DLK";
                StdfFile.WriteString(strUserTxt.toLatin1().constData());		// user-txt
                StdfFile.WriteString("");					// aux-file
                StdfFile.WriteString("");					// package-type
                StdfFile.WriteString("");					// familyID
                StdfFile.WriteString("");					// Date-code
                StdfFile.WriteString("");					// Facility-ID
                StdfFile.WriteString("");					// FloorID
                StdfFile.WriteString("");					// ProcessID

                StdfFile.WriteRecord();

                // Write WIR
                RecordReadInfo.iRecordType = 2;
                RecordReadInfo.iRecordSubType = 10;
                StdfFile.WriteHeader(&RecordReadInfo);
                StdfFile.WriteByte((BYTE)iHeadNum);							// HEAD_NUM
                StdfFile.WriteByte((BYTE)255);								// SITE_GRP
                StdfFile.WriteDword((DWORD)m_lStartTime);					// START_T
                StdfFile.WriteString(strWaferId.toLatin1().constData());	// WAFER_ID
                StdfFile.WriteRecord();
            }

        }
        if(strSection == "D")
        {
            //D
            //1,P,VF(1A),481.000, mV,200.000,580.000,0.00

            // Write PIR
            // Write PIR for parts in this Wafer site
            RecordReadInfo.iRecordType = 5;
            RecordReadInfo.iRecordSubType = 10;
            StdfFile.WriteHeader(&RecordReadInfo);
            StdfFile.WriteByte(iHeadNum);			// Test head
            StdfFile.WriteByte(iSiteNum);			// Tester site
            StdfFile.WriteRecord();

            // Reset Pass/Fail flag.
            bPassStatus = true;
            // Reset counters
            iTotalTests = 0;
            while(!hFile.atEnd())
            {
                strString = ReadLine(hFile);
                if(strString.isEmpty())
                    break;
                if(strString.simplified().toUpper() == "R")
                    break;

                // reset values
                strTestName = strUnit = "";
                fValue = fLowLimit = fHighLimit = 0;
                bTestAlreadySaved = false;

                //1,P,VF(1A),481.000, mV,200.000,580.000,0.00
                iTestNumber = strString.section(",",0,0).simplified().toInt();
                strTestName = strString.section(",",2,2).simplified();
                fValue = strString.section(",",3,3).simplified().toFloat();
                bTestPass = (strString.section(",",1,1).simplified().toUpper() == "P");
                bTestAlarm = (strString.section(",",1,1).simplified().toUpper() == "A");

                if(mapTestWithScale.contains(QString::number(iTestNumber)+"#"+strTestName))
                {
                    nScale = mapTestWithScale[QString::number(iTestNumber)+"#"+strTestName];
                    bTestAlreadySaved = true;
                }
                else
                {
                    bHaveLowLimit = bHaveHighLimit = false;
                    strUnit = strString.section(",",4,4).simplified();
                    fLowLimit = strString.section(",",5,5).simplified().toFloat(&bHaveLowLimit);
                    fHighLimit = strString.section(",",6,6).simplified().toFloat(&bHaveHighLimit);

                    if(!strUnit.isEmpty())
                        NormalizeLimits(strUnit, nScale);

                    mapTestWithScale[QString::number(iTestNumber)+"#"+strTestName] = nScale;
                }

                iTotalTests++;

                // Write PTR
                RecordReadInfo.iRecordType = 15;
                RecordReadInfo.iRecordSubType = 10;

                StdfFile.WriteHeader(&RecordReadInfo);
                StdfFile.WriteDword(iTestNumber);	// Test Number
                StdfFile.WriteByte(iHeadNum);	// Test head
                StdfFile.WriteByte(iSiteNum);	// Tester site:1,2,3,4 or 5, etc.
                if(bTestPass)
                    bData = 0;		// Test passed
                else
                    bData = BIT7;	// Test Failed
                if(bTestAlarm)
                    bData|= BIT0;

                StdfFile.WriteByte(bData);							// TEST_FLG
                if(bHaveLowLimit)
                    bData |= BIT6;
                if(bHaveHighLimit)
                    bData |= BIT7;
                StdfFile.WriteByte(bData);							// PARAM_FLG
                StdfFile.WriteFloat(fValue * GS_POW(10.0,nScale));		// Test result
                StdfFile.WriteString(strTestName.toLatin1().constData());	// TEST_TXT
                if(!bTestAlreadySaved)
                {
                    StdfFile.WriteString("");								// ALARM_ID

                    bData = 2;	// Valid data.
                    if(!bHaveLowLimit)
                        bData |= BIT6;
                    if(!bHaveHighLimit)
                        bData |= BIT7;
                    StdfFile.WriteByte(bData);							// OPT_FLAG

                    StdfFile.WriteByte(-nScale);						// RES_SCALE
                    StdfFile.WriteByte(-nScale);						// LLM_SCALE
                    StdfFile.WriteByte(-nScale);						// HLM_SCALE
                    StdfFile.WriteFloat(fLowLimit * GS_POW(10.0,nScale));	// LOW Limit
                    StdfFile.WriteFloat(fHighLimit * GS_POW(10.0,nScale));	// HIGH Limit
                    StdfFile.WriteString(strUnit.toLatin1().constData());	// Units
                }
                StdfFile.WriteRecord();
            }

            if(strString.simplified().toUpper() != "R")
            {
                // Incorrect header...this is not a DLK file!
                m_iLastError = errInvalidFormat;

                // Convertion failed.
                // Close file
                StdfFile.Close();
                return false;
            }

            //R
            //F,0,0,0.00
            strString = ReadLine(hFile);
            if(strString.isEmpty())
            {
                // Incorrect header...this is not a DLK file!
                m_iLastError = errInvalidFormat;

                // Convertion failed.
                // Close file
                StdfFile.Close();
                return false;
            }

            bPassStatus = (strString.section(",",0,0).simplified().toUpper() == "P");
            iHardBin = strString.section(",",1,1).simplified().toInt();
            iSoftBin = strString.section(",",2,2).simplified().toInt();

            if(iSoftBin != 0)
                bSaveSoftBin = true;

            if(!bSaveSoftBin)
                iSoftBin = iHardBin;

            if(!mapHardBinCount.contains(iHardBin))
                mapHardBinCount[iHardBin] = 0;

            if(!mapSoftBinCount.contains(iSoftBin))
                mapSoftBinCount[iSoftBin] = 0;

            if(!mapHardBinPass.contains(iHardBin))
                mapHardBinPass[iHardBin] = bPassStatus;

            if(!mapSoftBinPass.contains(iSoftBin))
                mapSoftBinPass[iSoftBin] = bPassStatus;


            mapHardBinCount[iHardBin]++;
            mapSoftBinCount[iSoftBin]++;

            // Write PRR
            RecordReadInfo.iRecordType = 5;
            RecordReadInfo.iRecordSubType = 20;
            StdfFile.WriteHeader(&RecordReadInfo);
            StdfFile.WriteByte(iHeadNum);		// Test head
            StdfFile.WriteByte(iSiteNum);		// Tester site:1,2,3,4 or 5
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
            StdfFile.WriteWord(iHardBin);			// HARD_BIN
            StdfFile.WriteWord(iSoftBin);			// SOFT_BIN
            StdfFile.WriteWord((WORD)nPosX);		// X_COORD
            StdfFile.WriteWord((WORD)nPosY);		// Y_COORD
            StdfFile.WriteDword(0);					// No testing time known...
            StdfFile.WriteString(strPartNumber.toLatin1().constData());// PART_ID
            StdfFile.WriteString(strPartNumber.toLatin1().constData());// PART_TXT
            StdfFile.WriteString("");				// PART_FIX
            StdfFile.WriteRecord();
        }
    }

    if(!strWaferId.isEmpty())
    {
        // Write WRR
        // Write WRR
        RecordReadInfo.iRecordType = 2;
        RecordReadInfo.iRecordSubType = 20;
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte((BYTE)iHeadNum);											// HEAD_NUM
        StdfFile.WriteByte((BYTE)255);												// SITE_GRP
        StdfFile.WriteDword((DWORD)m_lStartTime);									// FINISH_T
        StdfFile.WriteDword((DWORD)iTotalGoodBin+iTotalFailBin);					// PART_CNT
        StdfFile.WriteDword((DWORD)0);												// RTST_CNT
        StdfFile.WriteDword((DWORD)0);												// ABRT_CNT
        StdfFile.WriteDword((DWORD)iTotalGoodBin);									// GOOD_CNT
        StdfFile.WriteDword((DWORD)0);												// FUNC_CNT
        StdfFile.WriteString(strWaferId.toLatin1().constData());					// WAFER_ID
        StdfFile.WriteRecord();
    }

    QMap<int,int>::Iterator it;

    iTotalGoodBin = iTotalFailBin = 0;

    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 40;
    for(it = mapHardBinCount.begin(); it != mapHardBinCount.end(); it++)
    {
        // Write HBR/site
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte(255);						// Test Head
        StdfFile.WriteByte(255);						// Test sites
        StdfFile.WriteWord(it.key());					// HBIN
        StdfFile.WriteDword(it.value());					// Total Bins
        if(mapHardBinPass[it.key()])
        {
            StdfFile.WriteByte('P');
            iTotalGoodBin++;
        }
        else
        {
            StdfFile.WriteByte('F');
            iTotalFailBin++;
        }
        StdfFile.WriteRecord();
    }

    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 50;
    for(it = mapSoftBinCount.begin(); it != mapSoftBinCount.end(); it++)
    {
        // Write SBR/site
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte(255);						// Test Head
        StdfFile.WriteByte(255);						// Test sites
        StdfFile.WriteWord(it.key());					// HBIN
        StdfFile.WriteDword(it.value());					// Total Bins
        if(mapSoftBinPass[it.key()])
            StdfFile.WriteByte('P');
        else
            StdfFile.WriteByte('F');
        StdfFile.WriteRecord();
    }

    // Write PCR
    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 30;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteByte(255);					// Test Head = ALL
    StdfFile.WriteByte(255);					// Test sites = ALL
    StdfFile.WriteDword(iTotalGoodBin+iTotalFailBin);			// Total Parts tested
    StdfFile.WriteDword(0);						// Total Parts re-tested
    StdfFile.WriteDword(0);						// Total Parts aborted
    StdfFile.WriteDword(iTotalGoodBin);			// Total GOOD Parts
    StdfFile.WriteRecord();

    // Write MRR
    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 20;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteDword(m_lStartTime);			// File finish-time.
    StdfFile.WriteRecord();

    // Close STDF file.
    StdfFile.Close();

    if(bHaveMultiWafers)
    {
        // Rename current STDF file
        QDir clDir;
        QFileInfo clFile(m_strStdfFileName);
        m_lstStdfFileName += clFile.path()+"/"+clFile.baseName() +"_WaferId"+strWaferId+"."+clFile.suffix();
        clDir.rename(m_strStdfFileName,m_lstStdfFileName.last());

    }
    else
        m_lstStdfFileName += m_strStdfFileName;

    // Success
    return true;
}

//////////////////////////////////////////////////////////////////////
// Convert 'FileName' DLK file, to STDF 'strFileNameSTDF' file
//////////////////////////////////////////////////////////////////////
bool CGDLKtoSTDF::Convert(const char *DLKFileName, QStringList &lstFileNameSTDF)
{
    // No erro (default)
    m_iLastError = errNoError;

    // If STDF file already exists...do not rebuild it...unless dates not matching!
    QFileInfo fInput(DLKFileName);
    QFileInfo fOutput(lstFileNameSTDF.first());

    QFile f( lstFileNameSTDF.first() );
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
            GS::Gex::Engine::GetInstance().UpdateLabelStatus("Converting data from file "+QFileInfo(DLKFileName).fileName()+"...");
            GexScriptStatusLabel->show();
        }
    }
    QCoreApplication::processEvents();

    m_strStdfFileName = lstFileNameSTDF.first();
    lstFileNameSTDF.clear();
    if(ReadDLKFile(DLKFileName,m_strStdfFileName.toLatin1().constData()) != true)
    {
        while(!lstFileNameSTDF.isEmpty())
            QFile::remove(lstFileNameSTDF.takeFirst());

        //////////////////////////////////////////////////////////////////////
        // For ProgressBar
        if((GexProgressBar != NULL)
        && bHideProgressAfter)
            GexProgressBar->hide();

        if((GexScriptStatusLabel != NULL)
        && bHideLabelAfter)
            GexScriptStatusLabel->hide();

        return false;
    }

    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    if((GexProgressBar != NULL)
    && bHideProgressAfter)
        GexProgressBar->hide();

    if((GexScriptStatusLabel != NULL)
    && bHideLabelAfter)
        GexScriptStatusLabel->hide();

    lstFileNameSTDF = m_lstStdfFileName;
    // Convertion successful
    return true;
}

//////////////////////////////////////////////////////////////////////
// Read line : skip empty line
//////////////////////////////////////////////////////////////////////
QString CGDLKtoSTDF::ReadLine(QTextStream& hFile)
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
