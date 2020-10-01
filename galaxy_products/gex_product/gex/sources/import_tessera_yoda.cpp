//////////////////////////////////////////////////////////////////////
// import_TesseraYoda.cpp: Convert a TesseraYoda Yoda file to STDF V4.0
//////////////////////////////////////////////////////////////////////

#include <math.h>
#include <time.h>

#ifdef _WIN32
#include "windows.h"		// For 'GetWindowsDirectoryA' declaration
#endif

#include <qfileinfo.h>
#include <qregexp.h>
#include <qprogressbar.h>
#include <qapplication.h>
#include <qlabel.h>

#include "import_tessera_yoda.h"
#include "import_constants.h"
#include "engine.h"

extern QLabel			*GexScriptStatusLabel;	// Handle to script status text in status bar
extern QProgressBar		*GexProgressBar;		// Handle to progress bar in status bar

// File format:
//Header,COL,ROW,Pass,FFL,DOF,Defocus,S1,S2,S3,S4,S5,S6,S7,S8,S9,T2,T3,T4,...
//YODA = YODAv5.0,9,12,0,1.3497,0,125,21.231,15.212,136.87,34.232,35.593,0...
//Lens settings = Tessera_973_Spacer_Module.mtf,12,12,1,0.95786,40,23,67.7...
//Operator name = lwl,15,12,1,0.95713,46,20,71.015,30.57,42.6,43.657,35.25...
//Login Date    = 03/03/10,18,12,1,0.95557,45,19,72.38,30.655,40.704,41.70...
//Login Time    = 15:28:35,21,12,1,0.95867,44,21,70.869,31.428,38.057,43.1...
//Batch name    = C1_R1_FixedFFL1.852,24,12,1,0.95877,45,19,71.61,30.685,4...
//Lot name      = 03-03-10,27,12,1,0.95605,47,18,70.767,30.853,38.573,39.9...
//Name 1 = Operator Name,30,12,1,0.95712,47,18,71.14,31.017,39.609,41.169,...
//Name 2 = Product ID,33,12,1,0.95877,47,17,71.155,30.324,38.264,38.773,31...
//Name 3 = Date,36,12,1,0.95593,49,17,73.014,31.92,38.726,39.972,31.34,0,0...
//Optical Setup = Infinite - Finite,39,12,1,0.95705,47,18,72.181,33.313,38...
//Pitch         = 3,42,12,1,0.96156,46,19,71.588,33.202,38.186,38.476,31.1...
//Max Frequency = 114,45,12,1,0.96157,43,20,70.93,33.02,38.05,39.52,30.374...
//Optimise AF   = 57,48,12,1,0.96237,45,20,71.04,35.31,38.132,39.935,30.85...
//...
//Software ver. = 2.62,63,18,1,0.97994,43,19,70.573,35.236,35.624,35.378,2...
//,66,18,0,0.99314,0,153,23.53,13.088,9.1937,9.7581,32.214,0,0,0,0,18.238,...


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
CGTesseraYodatoSTDF::CGTesseraYodatoSTDF()
{
    m_lStartTime = 0;
    m_strTesterType = "Yoda";
    m_nTestPitch = 1;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGTesseraYodatoSTDF::~CGTesseraYodatoSTDF()
{
}

//////////////////////////////////////////////////////////////////////
// Get Error
//////////////////////////////////////////////////////////////////////
QString CGTesseraYodatoSTDF::GetLastError()
{
    m_strLastError = "Import Tessera Yoda "+m_strTesterType+": ";

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
// Check if File is compatible with TesseraYoda format
//////////////////////////////////////////////////////////////////////
bool CGTesseraYodatoSTDF::IsCompatible(const char *szFileName)
{
    QString strString;
    QString strSection;
    QString strValue;
    QString	strSite;

    // Open Tessera Yoda file
    QFile f( szFileName );
    if(!f.open( QIODevice::ReadOnly ))
    {
        // Failed Opening Tessera Yoda file
        return false;
    }
    // Assign file I/O stream
    QTextStream hTesseraYodaFile(&f);

    // Check if first line is the correct Tessera Yoda header...

    do
        strString = hTesseraYodaFile.readLine();
    while(!strString.isNull() && strString.isEmpty());

    if(!strString.startsWith("Header,COL,ROW,Pass,FFL", Qt::CaseInsensitive))
    {
        // Incorrect header...this is not a Tessera Yoda file!
        f.close();
        return false;
    }

    do
        strString = hTesseraYodaFile.readLine();
    while(!strString.isNull() && strString.isEmpty());

    if(!strString.startsWith("YODA", Qt::CaseInsensitive))
    {
        // Incorrect header...this is not a Tessera Yoda file!
        f.close();
        return false;
    }

    f.close();

    return true;
}

//////////////////////////////////////////////////////////////////////
// Read and Parse the TesseraYoda file
//////////////////////////////////////////////////////////////////////
bool CGTesseraYodatoSTDF::ReadTesseraYodaFile(const char *TesseraYodaFileName,const char *strFileNameSTDF)
{
    QString strString;
    QString strSection;
    QString strValue;
    QString	strSite;

    // Open TesseraYoda file
    QFile f( TesseraYodaFileName );
    if(!f.open( QIODevice::ReadOnly ))
    {
        // Failed Opening TesseraYoda file
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
    QTextStream hTesseraYodaFile(&f);

    // Check if first line is the correct TesseraYoda header...
    //Lens settings =

    strString = ReadLine(hTesseraYodaFile);

    if(!strString.startsWith("Header,COL,ROW,Pass,FFL", Qt::CaseInsensitive))
    {
        // Incorrect header...this is not a TesseraYoda file!
        m_iLastError = errInvalidFormat;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }

    QDate	clDate;
    QTime	clTime;


    strString = "";
    // Read TesseraYoda information
    while(!hTesseraYodaFile.atEnd())
    {
        if(strString.isEmpty())
            strString = ReadLine(hTesseraYodaFile);

        strSection = strString.section("=",0,0).remove(' ');
        strString = strString.section("=",1);
        strString = strString.section(",",0,0);

        if(strSection.startsWith(",", Qt::CaseInsensitive))
        {
            // Data
            break;
        }
        else if(strSection.toLower() == "lenssettings")
        {
            m_strJobName = strString;
            m_strSetupId = "LensSettings="+m_strJobName.simplified();
            strString = "";
        }
        else if(strSection.toLower() == "operatorname")
        {
            m_strOperatorId = strString.simplified();
            strString = "";
        }
        else if(strSection.toLower() == "logindate")
        {
            //07/17/08
            QString	strDate = strString.simplified();
            clDate = QDate(2000+strDate.section("/",2,2).toInt(),strDate.section("/",0,0).toInt(),strDate.section("/",1,1).toInt());
            clTime = QTime::fromString(strString.section(" ",1,1));

            strString = "";
        }
        else if(strSection.toLower() == "logintime")
        {
            //17:05:08
            QString	strTime = strString.simplified();
            clTime = QTime::fromString(strTime);

            strString = "";
        }
        else if(strSection.toLower() == "batchname")
        {
            m_strProductId = strString.simplified();
            strString = "";
        }
        else if(strSection.toLower() == "lotname")
        {
            m_strLotId = strString.simplified();
            strString = "";
        }
        else if(strSection.toLower() == "pitch")
        {
            // Setup Configuration
            if(!m_strSetupId.isEmpty())
                m_strSetupId += ";";
            m_strSetupId += strSection+"="+strString.simplified();

            m_nTestPitch = strString.simplified().toInt();
            if(m_nTestPitch < 1)
                m_nTestPitch = 1;
            strString = "";
        }
        else if(strSection.toLower() == "passsag")
        {
            // Setup Configuration
            if(!m_strSetupId.isEmpty())
                m_strSetupId += ";";
            m_strSetupId += strSection+"="+strString.simplified();

            strString = "";
        }
        else if(strSection.toLower() == "passtan")
        {
            // Setup Configuration
            if(!m_strSetupId.isEmpty())
                m_strSetupId += ";";
            m_strSetupId += strSection+"="+strString.simplified();

            strString = "";
        }
        else if(strSection.toLower() == "maxfrequency")
        {
            // Setup Configuration
            if(!m_strSetupId.isEmpty())
                m_strSetupId += ";";
            m_strSetupId += strSection+"="+strString.simplified();

            m_strFrequency = strString.simplified();
            strString = "";
        }
        else if(strSection.toLower() == "eflmax")
        {
            // Setup Configuration
            if(!m_strSetupId.isEmpty())
                m_strSetupId += ";";
            m_strSetupId += strSection+"="+strString.simplified();

            strString = "";
        }
        else if(strSection.toLower() == "eflmin")
        {
            // Setup Configuration
            if(!m_strSetupId.isEmpty())
                m_strSetupId += ";";
            m_strSetupId += strSection+"="+strString.simplified();

            strString = "";
        }
        else if(strSection.toLower() == "computer")
        {
            m_strComputerId = strString.simplified();
            strString = "";
        }
        else if(strSection.toLower() == "serialnumber")
        {
            m_strComputerId = strString.simplified();
            strString = "";
        }
        else if(strSection.toLower() == "softwarever")
        {
            m_strJobRev = strString.simplified();
            strString = "";
        }
        else if(strSection.toLower() == "sensorconfiguration")
        {
            m_strAuxFile = strString.simplified();
            strString = "";
        }
        else
            strString = "";


    }

    QDateTime clDateTime(clDate,clTime);
    clDateTime.setTimeSpec(Qt::UTC);
    m_lStartTime = clDateTime.toTime_t();

    // Open VERIGY_EDF file
    // restart to the beginning
    hTesseraYodaFile.seek(0);

    if(!WriteStdfFile(hTesseraYodaFile,strFileNameSTDF))
    {
        // Close file
        f.close();
        return false;
    }

    // Close file
    f.close();

    // Success parsing TesseraYoda file
    return true;
}

//////////////////////////////////////////////////////////////////////
// Create STDF file from TesseraYoda data parsed
//////////////////////////////////////////////////////////////////////
bool CGTesseraYodatoSTDF::WriteStdfFile(QTextStream& hTesseraYodaFile,const char *strFileNameSTDF)
{
    // now generate the STDF file...
    GS::StdLib::Stdf StdfFile;
    GS::StdLib::StdfRecordReadInfo RecordReadInfo;
    if(StdfFile.Open((char*)strFileNameSTDF,STDF_WRITE) != GS::StdLib::Stdf::NoError)
    {
        // Failed importing TesseraYoda file into STDF database
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
    StdfFile.WriteString(m_strProductId.toLatin1().constData());	// Part Type / Product ID
    StdfFile.WriteString(m_strComputerId.toLatin1().constData());	// Node name
    StdfFile.WriteString(m_strTesterType.toLatin1().constData());	// Tester Type
    StdfFile.WriteString(m_strJobName.toLatin1().constData());	// Job name
    StdfFile.WriteString(m_strJobRev.toLatin1().constData());		// Job rev
    StdfFile.WriteString("");					// sublot-id
    StdfFile.WriteString(m_strOperatorId.toLatin1().constData());	// operator
    StdfFile.WriteString("");					// exec-type
    StdfFile.WriteString(m_strSoftRev.toLatin1().constData());	// exe-ver
    StdfFile.WriteString("");					// test-cod
    StdfFile.WriteString("");					// test-temperature
    // Construct custom Galaxy USER_TXT
    QString	strUserTxt;
    strUserTxt = GEX_IMPORT_DATAORIGIN_LABEL;
    strUserTxt += ":";
    strUserTxt += GEX_IMPORT_DATAORIGIN_ATETEST;
    strUserTxt += ":TesseraYoda_"+m_strTesterType.toUpper();
    StdfFile.WriteString((char *)strUserTxt.toLatin1().constData());		// user-txt
    StdfFile.WriteString(m_strAuxFile.toLatin1().constData());	// aux-file
    StdfFile.WriteString("");					// package-type
    StdfFile.WriteString(m_strProductId.toLatin1().constData());	// familyID
    StdfFile.WriteString("");					// Date-code
    StdfFile.WriteString("");					// Facility-ID
    StdfFile.WriteString("");					// FloorID
    StdfFile.WriteString("");					// ProcessID
    StdfFile.WriteString((char *)m_strFrequency.toLatin1().constData());	// OperationFreq
    StdfFile.WriteString("");					// Spec-nam
    StdfFile.WriteString("");					// Spec-ver
    StdfFile.WriteString("");					// Flow-id
    StdfFile.WriteString((char *)m_strSetupId.left(255).toLatin1().constData());	// setup_id

    StdfFile.WriteRecord();

    // Write Test results for each line read.
    QString		strString;
    QString		strSection;
    QString		strValue;
    long		iTotalGoodBin,iTotalFailBin;
    long		iTotalTests,iPartNumber;
    bool		bPassStatus;
    BYTE		bData;

    int			iPos;
    int			iBin;
    int			iTestNumber;
    int			nPosX;
    int			nPosY;
    QString		strTestName;
    float		fValue;
    bool		bValidValue;
    bool		bTestAlreadySaved;

    QTime		clTime;

    QDateTime clDateTime;
    clDateTime.setTimeSpec(Qt::UTC);
    clTime = clDateTime.fromTime_t(m_lStartTime).time();

    QStringList		lstBinning;
    QMap<int,int>	mapBinCount;

    // Init the list of standard binning
    lstBinning.append("DUMMY");
    lstBinning.append("PASS");
    lstBinning.append("THRESHOLD");
    lstBinning.append("MISS");
    lstBinning.append("FAIL");


    QString strWaferId;
    QStringList lstTest;


    // Reset counters
    strWaferId = "1";
    iTotalGoodBin=iTotalFailBin=0;
    iPartNumber=0;

    // Read the first line
    // Extract test name
    strString = ReadLine(hTesseraYodaFile);
    for(iPos=4; iPos!=strString.count(",");iPos++)
        lstTest << strString.section(",",iPos,iPos);

    // Write WIR of new Wafer.
    RecordReadInfo.iRecordType = 2;
    RecordReadInfo.iRecordSubType = 10;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteByte(1);								// Test head
    StdfFile.WriteByte(255);							// Tester site (all)
    StdfFile.WriteDword(m_lStartTime);					// Start time
    StdfFile.WriteString(strWaferId.toLatin1().constData());	// WaferID
    StdfFile.WriteRecord();
    // Write all Parameters read on this file : PIR,PTR...,PRR, PIR,PTR..., PRR

    // Read TesseraYoda information
    while(!hTesseraYodaFile.atEnd())
    {
        // read line and remove space
        strString = ReadLine(hTesseraYodaFile);

        // Write PIR
        // Write PIR for parts in this Wafer site
        RecordReadInfo.iRecordType = 5;
        RecordReadInfo.iRecordSubType = 10;
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte(1);					// Test head
        StdfFile.WriteByte(1);					// Tester site
        StdfFile.WriteRecord();

        // Reset Pass/Fail flag.
        bPassStatus = true;
        iBin = 0;
        nPosX = -32768;
        nPosY = -32768;
        iPartNumber++;

        // Reset counters
        iTotalTests = 0;

        strSection = strString.section(",",1,1);
        nPosX = strSection.toInt();

        strSection = strString.section(",",2,2);
        nPosY = strSection.toInt();

        strSection = strString.section(",",3,3);
        iBin = strSection.toInt();
        bPassStatus = (iBin == 1);

        for(iPos=4; iPos!=strString.count(",");iPos++)
        {
            strValue = strString.section(",",iPos,iPos);
            fValue = strValue.toFloat(&bValidValue);
            iTestNumber = iPos-2;
            iTotalTests++;

            bTestAlreadySaved = true;
            if(!lstTest.isEmpty())
            {
                // Save a new PTR
                bTestAlreadySaved = false;
                strTestName = lstTest.takeFirst();
            }

            // Write PTR
            RecordReadInfo.iRecordType = 15;
            RecordReadInfo.iRecordSubType = 10;

            StdfFile.WriteHeader(&RecordReadInfo);
            StdfFile.WriteDword(iTestNumber);	// Test Number
            StdfFile.WriteByte(1);			// Test head
            StdfFile.WriteByte(1);			// Tester site:1,2,3,4 or 5, etc.

            // LE FLAG DOIT ETRE A IGNORE
            if(bPassStatus)
                bData = 0;		// Test passed
            else
                bData = BIT7;	// Test Failed
            if(!bValidValue)
                bData |= BIT1|BIT4;
            StdfFile.WriteByte(bData);							// TEST_FLG

            // PAS DE LIMIT
            StdfFile.WriteByte(0);								// PARAM_FLG
            StdfFile.WriteFloat(fValue);						// Test result

            if(!bTestAlreadySaved)
            {
                // save Parameter name without unit information
                StdfFile.WriteString(strTestName.toLatin1().constData());	// TEST_TXT
                StdfFile.WriteString("");							// ALARM_ID

                bData = BIT1|BIT2|BIT3|BIT4|BIT5;	// Valid data.
                StdfFile.WriteByte(bData);							// OPT_FLAG

                StdfFile.WriteByte(0);						// RES_SCALE
                StdfFile.WriteByte(0);						// LLM_SCALE
                StdfFile.WriteByte(0);						// HLM_SCALE
                StdfFile.WriteFloat(0);						// LOW Limit
                StdfFile.WriteFloat(0);						// HIGH Limit
                StdfFile.WriteString("");					// Units
            }
            StdfFile.WriteRecord();

        }


        lstTest.clear();
        // Write PRR
        RecordReadInfo.iRecordType = 5;
        RecordReadInfo.iRecordSubType = 20;
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte(1);				// Test head
        StdfFile.WriteByte(1);				// Tester site:1,2,3,4 or 5
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
        StdfFile.WriteWord(iBin);				// HARD_BIN
        StdfFile.WriteWord(iBin);				// SOFT_BIN
        StdfFile.WriteWord((WORD)nPosX);		// X_COORD
        StdfFile.WriteWord((WORD)nPosY);		// Y_COORD
        StdfFile.WriteDword(0);			// testing time

        QString strPartId = QString::number(iPartNumber);
        StdfFile.WriteString(strPartId.toLatin1().constData());// PART_ID
        StdfFile.WriteString("");				// PART_TXT
        StdfFile.WriteString("");				// PART_FIX
        StdfFile.WriteRecord();

    }


    // Write WRR for last wafer inserted
    RecordReadInfo.iRecordType = 2;
    RecordReadInfo.iRecordSubType = 20;

    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteByte(1);						// Test head
    StdfFile.WriteByte(255);					// Tester site (all)
    StdfFile.WriteDword(m_lStartTime);			// Time of last part tested
    StdfFile.WriteDword(iTotalGoodBin+iTotalFailBin)	;		// Parts tested
    StdfFile.WriteDword(0);						// Parts retested
    StdfFile.WriteDword(0);						// Parts Aborted
    StdfFile.WriteDword(iTotalGoodBin);						// Good Parts
    StdfFile.WriteDword((DWORD)-1);				// Functionnal Parts
    StdfFile.WriteString(strWaferId.toLatin1().constData());	// WaferID
    StdfFile.WriteString("");					// FabId
    StdfFile.WriteString("");					// FrameId
    StdfFile.WriteString("");					// MaskId
    StdfFile.WriteString("");					// UserDesc
    StdfFile.WriteRecord();

    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 40;

    for(iBin = 0; iBin < lstBinning.count(); iBin++)
    {
        if(!mapBinCount.contains(iBin))
            continue;
        // Write HBR

        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte(255);						// Test Head = ALL
        StdfFile.WriteByte(255);						// Test sites = ALL
        StdfFile.WriteWord(iBin);						// HBIN = 0
        StdfFile.WriteDword(mapBinCount[iBin]);			// Total Bins
        if(iBin == 1)
            StdfFile.WriteByte('P');
        else
            StdfFile.WriteByte('F');
        StdfFile.WriteString(lstBinning.at(iBin).toLatin1().constData());
        StdfFile.WriteRecord();
    }


    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 50;

    for(iBin = 0; iBin < lstBinning.count(); iBin++)
    {
        if(!mapBinCount.contains(iBin))
            continue;
        // Write SBR

        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte(255);						// Test Head = ALL
        StdfFile.WriteByte(255);						// Test sites = ALL
        StdfFile.WriteWord(iBin);						// HBIN = 0
        StdfFile.WriteDword(mapBinCount[iBin]);			// Total Bins
        if(iBin == 1)
            StdfFile.WriteByte('P');
        else
            StdfFile.WriteByte('F');
        StdfFile.WriteString(lstBinning.at(iBin).toLatin1().constData());
        StdfFile.WriteRecord();
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
// Convert 'FileName' TesseraYoda file, to STDF 'strFileNameSTDF' file
//////////////////////////////////////////////////////////////////////
bool CGTesseraYodatoSTDF::Convert(const char *TesseraYodaFileName, const char *strFileNameSTDF)
{
    // No erro (default)
    m_iLastError = errNoError;

    // If STDF file already exists...do not rebuild it...unless dates not matching!
    QFileInfo fInput(TesseraYodaFileName);
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
            GS::Gex::Engine::GetInstance().UpdateLabelStatus("Converting data from file "+QFileInfo(TesseraYodaFileName).fileName()+"...");
            GexScriptStatusLabel->show();
        }
    }
    QCoreApplication::processEvents();

    if(ReadTesseraYodaFile(TesseraYodaFileName,strFileNameSTDF) != true)
    {
        QFile::remove(strFileNameSTDF);
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

    // Convertion successful
    return true;
}

//////////////////////////////////////////////////////////////////////
// Read line : skip empty line
//////////////////////////////////////////////////////////////////////
QString CGTesseraYodatoSTDF::ReadLine(QTextStream& hFile)
{
    QString strString;

    do
        strString = hFile.readLine();
    while(!strString.isNull() && strString.isEmpty());

    return strString;

}


