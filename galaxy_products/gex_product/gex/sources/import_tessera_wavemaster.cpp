//////////////////////////////////////////////////////////////////////
// import_tessera_wavemaster.cpp: Convert a TesseraWavemaster file to STDF V4.0
//////////////////////////////////////////////////////////////////////

#include "gqtl_global.h"
#include <qmath.h>
#include <time.h>

#ifdef _WIN32
#include "windows.h"		// For 'GetWindowsDirectoryA' declaration
#endif

#include <qfileinfo.h>
#include <qregexp.h>
#include <qprogressbar.h>
#include <qapplication.h>
#include <qlabel.h>

#include "import_tessera_wavemaster.h"
#include "import_constants.h"
#include "engine.h"

extern QLabel			*GexScriptStatusLabel;	// Handle to script status text in status bar
extern QProgressBar		*GexProgressBar;		// Handle to progress bar in status bar

// File format:
//[Header]
//Tray - 3MP_AB_B side down.try
//Date and Starting Time : 8/21/2009 10:50:52 AM
//Operator : dp
//Lot : 970-11
//Batch : 082109
//WaferID : 970-11
//Wafer Alignment Rotation [°] : -0.509
//Wafer Alignment OffsetX [mm] : 1.437
//Wafer Alignment OffsetY [mm] : -0.013
//Measured MarkX1 pos [mm] : 52.154
//Measured MarkY1 pos [mm] : 99.877
//Measured MarkX2 pos [mm] : 149.365
//Measured MarkY2 pos [mm] : 99.013
//Pass/Fail Criterion : PV Zernike Fit    1.50000000
//Tolerance : 1.00000000
//
//[Linear Index 1]
//Row Index: 1
//Column Index: 7
//X Position [mm]:  95.02200000
//Y Position [mm]:  61.70380000
//Focus Position [mm]:  40.64600000
//Rms Comparison (Measured - Fit):  0.00000000
//Result: MISSING
//Result over all: Lens Missing
//Result over Sym: ...
//Result over Asym: ...
//
//A[0][0]=0.00000000
//A[1][-1]=0.00000000
//A[1][1]=0.00000000


// main.cpp

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
CGTesseraWavemastertoSTDF::CGTesseraWavemastertoSTDF()
{
    m_lStartTime = 0;
    m_strTesterType = "WaveMaster";
    m_nLinearIndex = -1;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGTesseraWavemastertoSTDF::~CGTesseraWavemastertoSTDF()
{
}

//////////////////////////////////////////////////////////////////////
// Get Error
//////////////////////////////////////////////////////////////////////
QString CGTesseraWavemastertoSTDF::GetLastError()
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
// Check if File is compatible with TesseraWavemaster format
//////////////////////////////////////////////////////////////////////
bool CGTesseraWavemastertoSTDF::IsCompatible(const char *szFileName)
{
    QString strString;
    QString strSection;
    QString strValue;
    QString	strSite;

    // Open TesseraWavemaster file
    QFile f( szFileName );
    if(!f.open( QIODevice::ReadOnly ))
    {
        // Failed Opening TesseraWavemaster file
        return false;
    }
    // Assign file I/O stream
    QTextStream hTesseraWavemasterFile(&f);

    // Check if first line is the correct TesseraWavemaster header...
    //[Header]
    //Tray - 3MP_AB_B side down.try

    strString = hTesseraWavemasterFile.readLine();
    if(!strString.startsWith("[Header]", Qt::CaseInsensitive))
    {
        // Incorrect header...this is not a TesseraWavemaster file!
        f.close();
        return false;
    }

    strString = hTesseraWavemasterFile.readLine();
    if(!strString.startsWith("Tray", Qt::CaseInsensitive))
    {
        // Incorrect header...this is not a TesseraWavemaster file!
        f.close();
        return false;
    }


    f.close();

    return true;
}

//////////////////////////////////////////////////////////////////////
// Read and Parse the TesseraWavemaster file
//////////////////////////////////////////////////////////////////////
bool CGTesseraWavemastertoSTDF::ReadTesseraWavemasterFile(const char *TesseraWavemasterFileName,const char *strFileNameSTDF)
{
    QString strString;
    QString strSection;
    QString strValue;
    QString	strSite;

    // Open TesseraWavemaster file
    QFile f( TesseraWavemasterFileName );
    if(!f.open( QIODevice::ReadOnly ))
    {
        // Failed Opening TesseraWavemaster file
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
    QTextStream hTesseraWavemasterFile(&f);

    // Check if first line is the correct TesseraWavemaster header...
    //[Header]

    strString = ReadLine(hTesseraWavemasterFile);

    if(!strString.startsWith("[Header]", Qt::CaseInsensitive))
    {
        // Incorrect header...this is not a TesseraWavemaster file!
        m_iLastError = errInvalidFormat;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }

    QDate	clDate;
    QTime	clTime;

    //Tray - 3MP_AB_B side down.try
    strString = ReadLine(hTesseraWavemasterFile);
    m_strJobName = strString.section("-",1).simplified();
    m_strSetupId = "LensSettings=" + m_strJobName;

    // Read TesseraWavemaster information
    while(!hTesseraWavemasterFile.atEnd())
    {
        strString = ReadLine(hTesseraWavemasterFile);

        strSection = strString.section(":",0,0).simplified();
        strString = strString.section(":",1).simplified();

        //
        //[Linear Index 4]
        if(strSection.startsWith("[linear index", Qt::CaseInsensitive))
        {
            // Data
            m_nLinearIndex = strSection.section(" ",2).remove("]").toInt();
            break;
        }
        //Operator : dp
        else if(strSection.toLower() == "operator")
        {
            m_strOperatorId = strString.simplified();
        }
        //Date and Starting Time : 8/21/2009 10:50:52 AM
        else if(strSection.toLower() == "date and starting time")
        {
            //8/21/2009 10:50:52 AM
            QString	strDate = strString.section(" ",0,0);
            clDate = QDate(strDate.section("/",2,2).toInt(),strDate.section("/",1,1).toInt(),strDate.section("/",0,0).toInt());
            QString strTime = strString.section(" ",1,1);
            clTime = QTime(strTime.section(":",0,0).toInt(),strTime.section(":",1,1).toInt(),strTime.section(":",2,2).toInt());
            if(strString.section(" ",2).toLower().simplified() == "pm")
                clTime = clTime.addSecs(12*60*60);
        }
        //Batch : 082109
        else if(strSection.toLower() == "batch")
        {
            m_strProgramId = strString.simplified();
        }
        //Lot : 970-11
        else if(strSection.toLower() == "lot")
        {
            m_strLotId = strString.simplified();
        }
        //WaferID : 970-11
        else if(strSection.toLower() == "waferid")
        {
            m_strWaferId = strString;
        }
        //Wafer Alignment Rotation [°] : -0.509
        //Wafer Alignment OffsetX [mm] : 1.437
        //Wafer Alignment OffsetY [mm] : -0.013
        //Measured MarkX1 pos [mm] : 52.154
        //Measured MarkY1 pos [mm] : 99.877
        //Measured MarkX2 pos [mm] : 149.365
        //Measured MarkY2 pos [mm] : 99.013
        //Pass/Fail Criterion : PV Zernike Fit    1.50000000
        //Tolerance : 1.00000000
        else
        if(strSection.startsWith("Wafer ",Qt::CaseInsensitive)
        || strSection.startsWith("Measured ",Qt::CaseInsensitive)
        || strSection.startsWith("Pass/Fail  ",Qt::CaseInsensitive)
        || strSection.startsWith("Tolerance",Qt::CaseInsensitive))
        {
            // Setup Configuration
            if(!m_strSetupId.isEmpty())
                m_strSetupId += ";";
            m_strSetupId += strSection.remove(" ").section('[',0,0)+"="+strString.simplified();

            strString = "";
        }
    }

    // It's a TesseraWavemaster file
    if(!strSection.startsWith("[linear index", Qt::CaseInsensitive))
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

    if(!WriteStdfFile(hTesseraWavemasterFile,strFileNameSTDF))
    {
        // Close file
        f.close();
        return false;
    }

    // Close file
    f.close();

    // Success parsing TesseraWavemaster file
    return true;
}

//////////////////////////////////////////////////////////////////////
// Create STDF file from TesseraWavemaster data parsed
//////////////////////////////////////////////////////////////////////
bool CGTesseraWavemastertoSTDF::WriteStdfFile(QTextStream& hTesseraWavemasterFile,const char *strFileNameSTDF)
{
    // now generate the STDF file...
    GS::StdLib::Stdf StdfFile;
    GS::StdLib::StdfRecordReadInfo RecordReadInfo;
    if(StdfFile.Open((char*)strFileNameSTDF,STDF_WRITE) != GS::StdLib::Stdf::NoError)
    {
        // Failed importing TesseraWavemaster file into STDF database
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
    StdfFile.WriteString(m_strProgramId.toLatin1().constData());	// Part Type / Product ID
    StdfFile.WriteString(m_strComputerId.toLatin1().constData());	// Node name
    StdfFile.WriteString(m_strTesterType.toLatin1().constData());	// Tester Type
    StdfFile.WriteString(m_strJobName.toLatin1().constData());	// Job name
    StdfFile.WriteString(m_strJobRev.toLatin1().constData());		// Job rev
    StdfFile.WriteString("");					// sublot-id
    StdfFile.WriteString(m_strOperatorId.toLatin1().constData());	// operator
    StdfFile.WriteString("");					// exec-type
    StdfFile.WriteString("");					// exe-ver
    StdfFile.WriteString("");					// test-cod
    StdfFile.WriteString("");					// test-temperature
    // Construct custom Galaxy USER_TXT
    QString	strUserTxt;
    strUserTxt = GEX_IMPORT_DATAORIGIN_LABEL;
    strUserTxt += ":";
    strUserTxt += GEX_IMPORT_DATAORIGIN_ATETEST;
    strUserTxt += ":TESSERA_" + m_strTesterType.toUpper();
    StdfFile.WriteString((char *)strUserTxt.toLatin1().constData());		// user-txt
    StdfFile.WriteString("");					// aux-file
    StdfFile.WriteString("");					// package-type
    StdfFile.WriteString(m_strProgramId.toLatin1().constData());	// familyID
    StdfFile.WriteString("");					// Date-code
    StdfFile.WriteString("");					// Facility-ID
    StdfFile.WriteString("");					// FloorID
    StdfFile.WriteString("");					// ProcessID
    StdfFile.WriteString("");					// OperationFreq
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
    long		iTotalTests;
    bool		bPassStatus;
    BYTE		bData;

    int			iBin;
    int			iTestNumber;
    int			nPosX;
    int			nPosY;
    float		fPosX;
    float		fPosY;
    QString		strTestName;
    QString		strUnit;
    int			nScale;
    float		fValue;
    bool		bTestAlreadySaved;

    QStringList		lstBinning;
    QMap<int,int>	mapBinCount;

    // Init the list of standard binning
    lstBinning.append("DUMMY");
    lstBinning.append("PASS");
    lstBinning.append("THRESHOLD");
    lstBinning.append("MISSING");
    lstBinning.append("FAIL");


    // Reset counters
    iTotalGoodBin=iTotalFailBin=0;

    //Wafer name

    // Write WIR of new Wafer.
    RecordReadInfo.iRecordType = 2;
    RecordReadInfo.iRecordSubType = 10;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteByte(1);								// Test head
    StdfFile.WriteByte(255);							// Tester site (all)
    StdfFile.WriteDword(m_lStartTime);					// Start time
    StdfFile.WriteString(m_strWaferId.toLatin1().constData());	// WaferID
    StdfFile.WriteRecord();

    // Write all Parameters read on this file : PIR,PTR...,PRR, PIR,PTR..., PRR

    //[Linear Index 1]
    //Row Index: 1
    //Column Index: 7
    //X Position [mm]:  95.02200000
    //Y Position [mm]:  61.70380000
    //Focus Position [mm]:  40.64600000
    //Rms Comparison (Measured - Fit):  0.00000000
    //Result: MISSING
    //Result over all: Lens Missing
    //Result over Sym: ...
    //Result over Asym: ...
    //
    //A[0][0]=0.00000000
    //A[1][-1]=0.00000000
    //A[1][1]=0.00000000
    // Reset Pass/Fail flag.
    bPassStatus = true;
    iBin = 0;
    nPosX = -32768;
    nPosY = -32768;
    fPosX = -32768.0;
    fPosY = -32768.0;

    // Reset counters
    iTotalTests = 0;

    // Write PIR
    // Write PIR for parts in this Wafer site
    RecordReadInfo.iRecordType = 5;
    RecordReadInfo.iRecordSubType = 10;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteByte(1);					// Test head
    StdfFile.WriteByte(1);					// Tester site
    StdfFile.WriteRecord();


    // Read TesseraWavemaster information
    while(!hTesseraWavemasterFile.atEnd())
    {
        // read line and remove space
        strString = ReadLine(hTesseraWavemasterFile);

        strSection = strString.section(":",0,0).simplified();
        strString = strString.section(":",1).simplified();

        //Result over Sym: ...
        //Result over Asym: ...
        if(strString == "...")
            continue;

        if(strString.toLower() == "lens missing")
            continue;

        if(strString.toLower() == "dist sens failed")
        {
            strSection = "Result";
            strString = "Dist Sens Failed";
        }

        // Save X Y position
        if(strSection.startsWith("X Position ",Qt::CaseInsensitive))
        {
            //X Position [mm]:  80.44910000
            fPosX = strString.toFloat();
        }
        else
        if(strSection.startsWith("Y Position ",Qt::CaseInsensitive))
        {
            //Y Position [mm]:  76.31240000
            fPosY = strString.toFloat();
        }

        if((strSection.left(14).toLower() == "[linear index ")
        || (hTesseraWavemasterFile.atEnd()))
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
            StdfFile.WriteDword(0);					// testing time

            QString strPartId = QString::number(m_nLinearIndex);

            if((fPosX != -32768.0) && (fPosY != -32768.0))
                strPartId = QString::number(fPosX,'f',4) + " " + QString::number(fPosY,'f',4);

            StdfFile.WriteString(strPartId.toLatin1().constData());// PART_ID
            StdfFile.WriteString("");				// PART_TXT
            StdfFile.WriteString("");				// PART_FIX
            StdfFile.WriteRecord();

            if(hTesseraWavemasterFile.atEnd())
                break;

            m_nLinearIndex = strSection.section(" ",2).remove("]").toInt();

            //////////////////////////////////////////////////////////////////////
            // For ProgressBar
            if(GexProgressBar != NULL)
            {
                int iPos = (int) hTesseraWavemasterFile.device()->pos();

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
            fPosX = -32768.0;
            fPosY = -32768.0;

            // Reset counters
            iTotalTests = 0;

        }
        else
        if(strSection.toLower() == "row index")
        {
            nPosY = strString.toInt();
        }
        else
        if(strSection.toLower() == "column index")
        {
            nPosX = strString.toInt();
        }
        else
        if(strSection.toLower() == "result")
        {
            // MISS
            // PASS
            // FAIL
            // THRESHOLD
            strValue = strString.toUpper();

            if(lstBinning.indexOf(strValue) == -1)
                lstBinning.append(strValue);

            iBin = lstBinning.indexOf(strValue);

            if(!mapBinCount.contains(iBin))
                mapBinCount[iBin] = 0;
            mapBinCount[iBin]++;

            bPassStatus = (iBin == 1);

        }
        else
        if(strSection.startsWith("Result over ",Qt::CaseInsensitive))
        {
            //Result over all: PV Wavefront = 1.925 waves    RMS Wavefront = 0.335 waves
            while(!strString.isEmpty())
            {
                strSection = strString.section("=",0,0).simplified();
                strString = strString.section("=",1).simplified();

                // Line results
                // For each test, write a PTR

                strTestName = strSection;
                fValue = strString.section(" ",0,0).toFloat();
                strUnit = strString.section(" ",1,1);
                NormalizeLimits(strUnit, nScale);

                //Next result
                strString = strString.section(" ",2),

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
                StdfFile.WriteFloat(fValue * GS_POW(10.0,nScale));		// Test result

                if(!bTestAlreadySaved)
                {
                    // save Parameter name without unit information
                    StdfFile.WriteString(strTestName.toLatin1().constData());	// TEST_TXT
                    StdfFile.WriteString("");							// ALARM_ID

                    bData = BIT1|BIT2|BIT3|BIT4|BIT5;	// Valid data.
                    StdfFile.WriteByte(bData);					// OPT_FLAG

                    StdfFile.WriteByte(-nScale);				// RES_SCALE
                    StdfFile.WriteByte(-nScale);				// LLM_SCALE
                    StdfFile.WriteByte(-nScale);				// HLM_SCALE
                    StdfFile.WriteFloat(0);						// LOW Limit
                    StdfFile.WriteFloat(0);						// HIGH Limit
                    StdfFile.WriteString( strUnit.toLatin1().constData());// Units
                }
                StdfFile.WriteRecord();


            }

        }
        else
        if((strSection.startsWith("A[",Qt::CaseInsensitive)) || !strString.isEmpty())
        {
            //Focus Position [mm]:  41.50340000
            //Rms Comparison (Measured - Fit):  0.05852510

            //A[0][0]=0.00000000
            //A[1][-1]=0.00000000

            // Line result
            // For each test, write a PTR

            strUnit = "";
            nScale = 0;

            if(strString.isEmpty())
            {
                strTestName = strSection.section("=",0 ,0);
                fValue = strSection.section("=",1,1).toFloat();
            }
            else
            {
                if(strSection.indexOf("[") > 0)
                {
                    // unit
                    strUnit = strSection.section("[",1).remove("]");
                    NormalizeLimits(strUnit, nScale);
                    strSection = strSection.section("[",0,0).simplified();
                }

                strTestName = strSection;
                fValue = strString.toFloat();

            }

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
            StdfFile.WriteFloat(fValue * GS_POW(10.0,nScale));	// Test result

            if(!bTestAlreadySaved)
            {
                // save Parameter name without unit information
                StdfFile.WriteString(strTestName.toLatin1().constData());	// TEST_TXT
                StdfFile.WriteString("");							// ALARM_ID

                bData = BIT1|BIT2|BIT3|BIT4|BIT5;	// Valid data.
                StdfFile.WriteByte(bData);					// OPT_FLAG

                StdfFile.WriteByte(-nScale);				// RES_SCALE
                StdfFile.WriteByte(-nScale);				// LLM_SCALE
                StdfFile.WriteByte(-nScale);				// HLM_SCALE
                StdfFile.WriteFloat(0);						// LOW Limit
                StdfFile.WriteFloat(0);						// HIGH Limit
                StdfFile.WriteString( strUnit.toLatin1().constData());// Units
            }
            StdfFile.WriteRecord();

        }
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
    StdfFile.WriteString(m_strWaferId.toLatin1().constData());	// WaferID
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
// Convert 'FileName' TesseraWavemaster file, to STDF 'strFileNameSTDF' file
//////////////////////////////////////////////////////////////////////
bool CGTesseraWavemastertoSTDF::Convert(const char *TesseraWavemasterFileName, const char *strFileNameSTDF)
{
    // No erro (default)
    m_iLastError = errNoError;

    // If STDF file already exists...do not rebuild it...unless dates not matching!
    QFileInfo fInput(TesseraWavemasterFileName);
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
            GS::Gex::Engine::GetInstance().UpdateLabelStatus("Converting data from file "+QFileInfo(TesseraWavemasterFileName).fileName()+"...");
            GexScriptStatusLabel->show();
        }
    }
    QCoreApplication::processEvents();

    if(ReadTesseraWavemasterFile(TesseraWavemasterFileName,strFileNameSTDF) != true)
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
// Normalize test limits when writing into PTR.
//////////////////////////////////////////////////////////////////////
void CGTesseraWavemastertoSTDF::NormalizeLimits(QString &strUnit, int &nScale)
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
// Read line : skip empty line
//////////////////////////////////////////////////////////////////////
QString CGTesseraWavemastertoSTDF::ReadLine(QTextStream& hFile)
{
    QString strString;

    do
        strString = hFile.readLine();
    while(!strString.isNull() && strString.isEmpty());

    return strString;

}


