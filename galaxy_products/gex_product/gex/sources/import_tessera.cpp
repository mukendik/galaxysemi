//////////////////////////////////////////////////////////////////////
// import_Tessera.cpp: Convert a Tessera file to STDF V4.0
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

#include "import_tessera.h"
#include "import_constants.h"
#include "engine.h"

extern QLabel			*GexScriptStatusLabel;	// Handle to script status text in status bar
extern QProgressBar		*GexProgressBar;		// Handle to progress bar in status bar

// File format:
//Lens settings = Tessera_Adjusted_ROI_CG_AF_76LP.mtf
//Operator name = cpollock
//Login Date    = 07/17/08
//Login Time    = 17:05:08
//Batch name    = 760-100
//Lot name      = 07-17-08
//Optical Setup = Infinite - Finite
//Pitch         = 4
//Max Frequency = 116
//Optimise AF   = 76
//Optimise S1   = 76
//Optimise S2   = 76
//Optimise T2   = 76
//Optimise S3   = 76
//Optimise T3   = 76
//...
//Optimise T9   = 76
//Angle Cam 1   = 0.000
//Angle Cam 2   = 24.228
//...
//Angle Cam 6   = 18.650
//Angle Cam 7   = 18.650
//Angle Cam 8   = 18.650
//Angle Cam 9   = 18.650
//AOI		  = (100 79 150 81)
//AOI		  = (79 79 81 81)
//Serial Number = 908-06-071
//Software ver. = 2.37
//
//[measurement 1]
//Tray Name Tessera_39x39
//Number 104
//Position 3,18
//Time 17:05:47
//Result FAIL
//Driver Pos 36.10680
//FFL 24.754501
//Sensor Distance 25362.375000
//Calibration Offset 0.230000
//Defocus 0.019
//X1 -1.421
//Y1 -2.368
//X2 -2.842
//Y2 -7.578
//X3 -1.894
//Y3 -14.208
//...
//X9 4.736
//Y9 -9.472
//EFL 1.55915
//S1 57.9 100.0 99.8 99.1 98.0 96.4 94.6 92.4 90.0 87.3 84.5 81.6 78.8 76.0 73.3 70.7 68.1 65.6 63.0 60.5 57.9 55.4 53.1 50.9 48.8 46.8 45.0 43.1 41.3 39.6 37.9
//S2 33.0 100.0 99.3 97.4 94.4 90.6 86.3 81.9 77.4 73.1 68.9 64.8 60.7 56.6 52.7 48.9 45.4 42.0 38.8 35.8 33.0 30.4 28.0 25.7 23.7 21.8 20.2 18.6 17.2 15.9 14.7
//T2 39.1 100.0 99.4 97.9 95.5 92.4 88.8 85.0 81.1 77.1 73.3 69.5 65.8 62.1 58.4 54.8 51.3 48.0 44.8 41.8 39.1 36.6 34.4 32.3 30.5 28.9 27.6 26.4 25.4 24.6 23.9
//S3 53.6 100.0 99.5 98.1 95.9 93.2 90.3 87.2 84.3 81.6 79.0 76.5 74.0 71.4 68.7 66.1 63.4 60.8 58.3 55.9 53.6 51.4 49.1 46.9 44.8 42.7 40.7 38.8 37.0 35.3 33.7
//T3 44.0 100.0 99.4 97.5 94.8 91.2 87.3 83.2 79.2 75.2 71.4 67.7 64.1 60.7 57.6 54.7 52.2 49.8 47.7 45.8 44.0 42.5 41.2 40.0 39.0 38.1 37.3 36.5 35.8 35.1 34.4
//...
//
//[measurement 2]
//


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
CGTesseratoSTDF::CGTesseratoSTDF()
{
    m_lStartTime = 0;
    m_strTesterType = "ImageMaster";
    m_nTestPitch = 1;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGTesseratoSTDF::~CGTesseratoSTDF()
{
}

//////////////////////////////////////////////////////////////////////
// Get Error
//////////////////////////////////////////////////////////////////////
QString CGTesseratoSTDF::GetLastError()
{
    m_strLastError = "Import Tessera "+m_strTesterType+": ";

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
// Check if File is compatible with Tessera format
//////////////////////////////////////////////////////////////////////
bool CGTesseratoSTDF::IsCompatible(const char *szFileName)
{
    QString strString;
    QString strSection;
    QString strValue;
    QString	strSite;

    // Open Tessera file
    QFile f( szFileName );
    if(!f.open( QIODevice::ReadOnly ))
    {
        // Failed Opening Tessera file
        return false;
    }
    // Assign file I/O stream
    QTextStream hTesseraFile(&f);

    // Check if first line is the correct Tessera header...
    //TEXT REPORT

    do
        strString = hTesseraFile.readLine();
    while(!strString.isNull() && strString.isEmpty());

    f.close();
    if(!strString.startsWith("Lens settings =", Qt::CaseInsensitive))
    {
        // Incorrect header...this is not a Tessera file!
        return false;
    }


    return true;
}

//////////////////////////////////////////////////////////////////////
// Read and Parse the Tessera file
//////////////////////////////////////////////////////////////////////
bool CGTesseratoSTDF::ReadTesseraFile(const char *TesseraFileName,const char *strFileNameSTDF)
{
    QString strString;
    QString strSection;
    QString strValue;
    QString	strSite;

    // Open Tessera file
    QFile f( TesseraFileName );
    if(!f.open( QIODevice::ReadOnly ))
    {
        // Failed Opening Tessera file
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
    QTextStream hTesseraFile(&f);

    // Check if first line is the correct Tessera header...
    //Lens settings =

    strString = ReadLine(hTesseraFile);

    if(!strString.startsWith("Lens settings =", Qt::CaseInsensitive))
    {
        // Incorrect header...this is not a Tessera file!
        m_iLastError = errInvalidFormat;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }

    QDate	clDate;
    QTime	clTime;

    m_strJobName = strString.section("=",1);
    m_strSetupId = "LensSettings="+m_strJobName.simplified();

    strString = "";
    // Read Tessera information
    while(!hTesseraFile.atEnd())
    {
        if(strString.isEmpty())
            strString = ReadLine(hTesseraFile);

        strSection = strString.section("=",0,0).remove(' ');
        strString = strString.section("=",1);

        if(strSection.startsWith("[measurement", Qt::CaseInsensitive))
        {
            // Data
            break;
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

    // It's a Tessera file
    if(!strSection.startsWith("[measurement", Qt::CaseInsensitive))
    {
        // Incorrect format
        m_iLastError = errInvalidFormat;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }

    QDateTime clDateTime(clDate,clTime);
    clDateTime.setTimeSpec(Qt::UTC);
    m_lStartTime = clDateTime.toTime_t();

    // Open VERIGY_EDF file

    if(!WriteStdfFile(hTesseraFile,strFileNameSTDF))
    {
        // Close file
        f.close();
        return false;
    }

    // Close file
    f.close();

    // Success parsing Tessera file
    return true;
}

//////////////////////////////////////////////////////////////////////
// Create STDF file from Tessera data parsed
//////////////////////////////////////////////////////////////////////
bool CGTesseratoSTDF::WriteStdfFile(QTextStream& hTesseraFile,const char *strFileNameSTDF)
{
    // now generate the STDF file...
    GS::StdLib::Stdf StdfFile;
    GS::StdLib::StdfRecordReadInfo RecordReadInfo;
    if(StdfFile.Open((char*)strFileNameSTDF,STDF_WRITE) != GS::StdLib::Stdf::NoError)
    {
        // Failed importing Tessera file into STDF database
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
    strUserTxt += ":TESSERA_"+m_strTesterType.toUpper();
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
    long		iTotalTests=0;
    int			iPartNumber;
    bool		bPassStatus=false;
    BYTE		bData;

    int			iPos;
    int			iBin=0;
    int			iTestNumber;
    int			nPosX=-32768;
    int			nPosY=-32768;
    float		fPosX;
    float		fPosY;
    QString		strName;
    QString		strTestName;
    QStringList	lstValue;
    float		fValue;
    bool		bTestAlreadySaved;

    QTime		clTime;
    QTime		clNextTime;

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
    bool bWriteWir;

    // Reset counters
    bWriteWir = true;
    iTotalGoodBin=iTotalFailBin=0;
    iPartNumber=0;
    fPosX = -32768.0;
    fPosY = -32768.0;

    // Write all Parameters read on this file : PIR,PTR...,PRR, PIR,PTR..., PRR

    //[measurement 1]  ===> CURRENT POSITION
    //Tray Name Tessera_39x39
    //Number 104
    //Position 3,18
    //Time 17:05:47
    //Result FAIL

    // Read Tessera information
    while(!hTesseraFile.atEnd())
    {
        // read line and remove space
        strString = ReadLine(hTesseraFile);

        strSection = strString.toLower();

        // Save X Y position
        if(strSection.left(11) == "x position ")
        {
            //X Position 80.44910000
            fPosX = strString.section(" ",2,2).toFloat();
        }
        else
        if(strSection.left(11) == "y position ")
        {
            //Y Position 76.31240000
            fPosY = strString.section(" ",2,2).toFloat();
        }

        if((strSection.left(13) == "[measurement ")
        || (hTesseraFile.atEnd()))
        {
            // End of this part
            // close the PIR
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

            int nMSecond = clTime.msecsTo(clNextTime);
            if(nMSecond < 0)
            {
                nMSecond =  86400000 - QTime::fromString("00:00:00").msecsTo(clTime) ;
                nMSecond += QTime::fromString("00:00:00").msecsTo(clNextTime);
            }

            StdfFile.WriteDword(nMSecond);			// testing time

            QString strPartId = QString::number(iPartNumber);

            if((fPosX != -32768.0) && (fPosY != -32768.0))
                strPartId = QString::number(fPosX,'f',4) + " " + QString::number(fPosY,'f',4);


            StdfFile.WriteString(strPartId.toLatin1().constData());// PART_ID
            StdfFile.WriteString("");				// PART_TXT
            StdfFile.WriteString("");				// PART_FIX
            StdfFile.WriteRecord();

            clTime = clNextTime;
            fPosX = -32768.0;
            fPosY = -32768.0;
        }
        else
        if(strSection.left(5) == "tray ")
        {
            //Wafer name
            if(bWriteWir)
            {
                strWaferId = strString.mid(10).simplified();

                // Write WIR of new Wafer.
                RecordReadInfo.iRecordType = 2;
                RecordReadInfo.iRecordSubType = 10;
                StdfFile.WriteHeader(&RecordReadInfo);
                StdfFile.WriteByte(1);								// Test head
                StdfFile.WriteByte(255);							// Tester site (all)
                StdfFile.WriteDword(m_lStartTime);					// Start time
                StdfFile.WriteString(strWaferId.toLatin1().constData());	// WaferID
                StdfFile.WriteRecord();

                bWriteWir = false;
            }
        }
        else
        if(strSection.left(7) == "number ")
        {
            iPartNumber = strString.section(" ",1,1).toInt();

            //////////////////////////////////////////////////////////////////////
            // For ProgressBar
            if(GexProgressBar != NULL)
            {
                int iPos = (int) hTesseraFile.device()->pos();

                while(iPos  > iNextFilePos)
                {
                    iProgressStep += 100/iFileSize + 1;
                    iNextFilePos  += iFileSize/100 + 1;
                }
                GexProgressBar->setValue(iProgressStep);

            }
            QCoreApplication::processEvents();

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

            // Reset counters
            iTotalTests = 0;

        }
        else
        if(strSection.left(9) == "position ")
        {
            strValue = strString.section(" ",1,1);
            nPosY = strValue.section(",",0,0).toInt();
            nPosX = strValue.section(",",1).toInt();
        }
        else
        if(strSection.left(5) == "time ")
        {
            strValue = strString.section(" ",1,1);
            clNextTime = QTime::fromString(strValue);
        }
        else
        if(strSection.left(7) == "result ")
        {
            // MISS
            // PASS
            // FAIL
            // THRESHOLD
            strValue = strString.section(" ",1,1).toUpper();
            if(lstBinning.indexOf(strValue) == -1)
                lstBinning.append(strValue);

            iBin = lstBinning.indexOf(strValue);

            if(!mapBinCount.contains(iBin))
                mapBinCount[iBin] = 0;
            mapBinCount[iBin]++;

            bPassStatus = (iBin == 1);

        }
        else
        if((iPos = strString.count(" ")) >= 1)
        {
            // Line result
            // For each test, write a PTR
            strName = strTestName = "";
            lstValue.clear();

            if(iPos > 5)
            {
                strName = strString.section(" ",0 ,0);
                //CASE 2590
                //Modifications to Image Master File Parser
                //Please modify the Imagemaster parser to exclude all but the first measurement for each test.
                //lstValue = strString.section(" ",1).split(" ");
                lstValue.append(strString.section(" ",1,1));
            }
            else
            {
                strName = strString.section(" ",0 ,iPos-1);
                lstValue.append(strString.section(" ",iPos));
            }

            // For each value
            // Save a PTR
            for(iPos = 0; iPos < lstValue.count(); iPos++)
            {
                strTestName = strName;
                if(iPos > 0)
                    strTestName += "_" + QString::number((iPos-1)*m_nTestPitch);

                strValue = lstValue[iPos];
                fValue = strValue.toFloat();

                // Save a new PTR
                bTestAlreadySaved = true;

                iTotalTests++;

                iTestNumber = m_lstTest.indexOf(strTestName)+1;

                if(iTestNumber == 0)
                {
                    m_lstTest.append(strTestName);
                    iTestNumber = m_lstTest.indexOf(strTestName)+1;

                    bTestAlreadySaved = false;
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
        }
        else
            strString="";
    }

    clDateTime.fromTime_t(m_lStartTime);
    if(clDateTime.time() > clNextTime)
        ( void )clDateTime.addDays(1);
    clDateTime.setTime(clNextTime);
    clDateTime.setTimeSpec(Qt::UTC);
    m_lStartTime = clDateTime.toTime_t();



    if(!bWriteWir)
    {
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
    }

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
// Convert 'FileName' Tessera file, to STDF 'strFileNameSTDF' file
//////////////////////////////////////////////////////////////////////
bool CGTesseratoSTDF::Convert(const char *TesseraFileName, const char *strFileNameSTDF)
{
    // No erro (default)
    m_iLastError = errNoError;

    // If STDF file already exists...do not rebuild it...unless dates not matching!
    QFileInfo fInput(TesseraFileName);
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
            GS::Gex::Engine::GetInstance().UpdateLabelStatus("Converting data from file "+QFileInfo(TesseraFileName).fileName()+"...");
            GexScriptStatusLabel->show();
        }
    }
    QCoreApplication::processEvents();

    if(ReadTesseraFile(TesseraFileName,strFileNameSTDF) != true)
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
QString CGTesseratoSTDF::ReadLine(QTextStream& hFile)
{
    QString strString;

    do
        strString = hFile.readLine();
    while(!strString.isNull() && strString.isEmpty());

    return strString;

}


