//////////////////////////////////////////////////////////////////////
// import_prober_tsmc.cpp: Convert a PROBER_TSMC file to STDF V4.0
//////////////////////////////////////////////////////////////////////

#include <qfileinfo.h>
#include <qprogressbar.h>
#include <qapplication.h>
#include <qlabel.h>


#include "import_prober_tsmc.h"
#include "time.h"
#include "import_constants.h"
#include "engine.h"

//TSMC or JAZZ
//CU738BAR
//R114792-02
//R114792-R114792-02
//...............1111111111111...............
//............X111111X1111111111X............

// main.cpp
extern QLabel			*GexScriptStatusLabel;	// Handle to script status text in status bar
extern QProgressBar	*	GexProgressBar;		// Handle to progress bar in status bar

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGPROBER_TSMCtoSTDF::CGPROBER_TSMCtoSTDF()
{
    m_lStartTime = 0;
    m_nGoodBinCnt = 0;
    m_nBadBinCnt = 0;
}

//////////////////////////////////////////////////////////////////////
// Destruction
//////////////////////////////////////////////////////////////////////
CGPROBER_TSMCtoSTDF::~CGPROBER_TSMCtoSTDF()
{
}

//////////////////////////////////////////////////////////////////////
// Get Error
//////////////////////////////////////////////////////////////////////
QString CGPROBER_TSMCtoSTDF::GetLastError()
{
    m_strLastError = "Import PROBER_TSMC: ";

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
// Check if File is compatible with PROBER_TSMC format
//////////////////////////////////////////////////////////////////////
bool CGPROBER_TSMCtoSTDF::IsCompatible(const char *szFileName)
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
    QTextStream hProber_TsmcFile(&f);

    // Check if first line is the correct PROBER_TSMC header...
    do
        strString = hProber_TsmcFile.readLine().simplified();
    while(!strString.isNull() && strString.isEmpty());

    strString = strString.toUpper();	// remove leading spaces.
    if(strString.startsWith("TSMC")
    || strString.startsWith("JAZZ"))
        strValue = "JAZZ";
    else
    {
        // Incorrect header...this is not a PROBER_TSMC file!
        // Close file
        f.close();
        return false;
    }

    // Close file
    f.close();

    return true;
}

//////////////////////////////////////////////////////////////////////
// Read and Parse the PROBER_TSMC file
//////////////////////////////////////////////////////////////////////
bool CGPROBER_TSMCtoSTDF::ReadProber_TsmcFile(const char* Prober_TsmcFileName,
                                              const char* strFileNameSTDF)
{
    QString strString;
    QString strSection;

    // Open CSV file
    QFile f( Prober_TsmcFileName );
    if(!f.open( QIODevice::ReadOnly ))
    {
        // Failed Opening PROBER_TSMC file
        m_iLastError = errOpenFail;

        // Convertion failed.
        return false;
    }

    // Assign file I/O stream
    QTextStream hProber_TsmcFile(&f);

    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    iProgressStep = 0;
    iNextFilePos = 0;
    iFileSize = f.size() + 1;
    QCoreApplication::processEvents();


    // Check if first line is the correct PROBER_TSMC header...
    strString = ReadLine(hProber_TsmcFile).trimmed().toUpper();	// remove leading spaces.
    if(strString.startsWith("TSMC"))
        m_strUserTxt = "TSMC";
    else if(strString.startsWith("JAZZ"))
        m_strUserTxt = "JAZZ";
    else
    {
        // Incorrect header...this is not a PROBER_TSMC file!
        m_iLastError = errInvalidFormat;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }

    m_strDeviceId = ReadLine(hProber_TsmcFile).simplified();
    m_strWaferId = ReadLine(hProber_TsmcFile).simplified();
    m_strLotId = ReadLine(hProber_TsmcFile).section("-",0,0).simplified();

    if(!WriteStdfFile(&hProber_TsmcFile,strFileNameSTDF))
    {
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
// Create STDF file from PROBER_TSMC data parsed
//////////////////////////////////////////////////////////////////////
bool CGPROBER_TSMCtoSTDF::WriteStdfFile(QTextStream *hProber_TsmcFile, const char *strFileNameSTDF)
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

    long lStartTime = QDateTime::currentDateTime().toTime_t();

    // Write MIR
    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 10;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteDword(lStartTime);			// Setup time
    StdfFile.WriteDword(lStartTime);			// Start time
    StdfFile.WriteByte(1);						// Station
    StdfFile.WriteByte((BYTE) 'P');				// Test Mode = PRODUCTION
    StdfFile.WriteByte((BYTE) ' ');				// rtst_cod
    StdfFile.WriteByte((BYTE) ' ');				// prot_cod
    StdfFile.WriteWord(65535);					// burn_tim
    StdfFile.WriteByte((BYTE) ' ');				// cmod_cod
    StdfFile.WriteString(m_strLotId.toLatin1().constData());		// Lot ID
    StdfFile.WriteString(m_strDeviceId.toLatin1().constData());	// Part Type / Product ID
    StdfFile.WriteString("");					// Node name
    StdfFile.WriteString("");					// Tester Type
    StdfFile.WriteString("");					// Job name
    StdfFile.WriteString("");					// Job rev
    StdfFile.WriteString("");					// sublot-id
    StdfFile.WriteString("");					// operator
    StdfFile.WriteString("");					// exec-type
    StdfFile.WriteString("");					// exe-ver
    StdfFile.WriteString("WAFER");				// test-cod
    StdfFile.WriteString("");					// test-temperature
    // Construct custom Galaxy USER_TXT
    QString	strUserTxt;
    strUserTxt = GEX_IMPORT_DATAORIGIN_LABEL;
    strUserTxt += ":";
    strUserTxt += GEX_IMPORT_DATAORIGIN_ATETEST;
    strUserTxt += ":PROBER " + m_strUserTxt;
    StdfFile.WriteString((char *)strUserTxt.toLatin1().constData());	// user-txt
    StdfFile.WriteString("");					// aux-file
    StdfFile.WriteString("");					// package-type
    StdfFile.WriteString("");					// familyID
    StdfFile.WriteString("");					// Date-code
    StdfFile.WriteString("");					// Facility-ID
    StdfFile.WriteString("");					// FloorID
    StdfFile.WriteString("");					// ProcessID
    StdfFile.WriteRecord();

    // Write Test results for each line read.
    QString		strString;
    int			iIndex;				// Loop index
    WORD		wSoftBin,wHardBin;
    long		iTotalGoodBin,iTotalFailBin;
  long iPartNumber;
  //FIXME: not used ?
  //long iTotalTests
  //bool bPassStatus;

    int			iWaferRows;
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

    iWaferRows = 0;
    // Write all Parameters read on this wafer.: WIR.PIR,PRR, PIR,PRR,   ... WRR
    while(hProber_TsmcFile->atEnd() == false)
    {

        iWaferRows++;

        // Read line
        strString = ReadLine(*hProber_TsmcFile).simplified();
        if(strString.isEmpty())
            break;	// Reach the end of valid data records...

        // Reset Pass/Fail flag.
    //FIXME: not used ?
    //bPassStatus = true;

        // Reset counters
    //FIXME: not used ?
    //iTotalTests = 0;


        // Read Parameter results for this record
        for(iIndex=0;iIndex<(int)strString.length();iIndex++)
        {
            if(strString.mid(iIndex,1) == ".")
                continue;
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
            StdfFile.WriteByte(1);			// Test head
            StdfFile.WriteByte(1);			// Tester site#:1
            if(strString.at(iIndex) == '1')
            {
                StdfFile.WriteByte(0);				// PART_FLG : PASSED
                wSoftBin = wHardBin = 1;
                iTotalGoodBin++;
            }
            else
            {
                StdfFile.WriteByte(8);		// PART_FLG : FAILED
                wSoftBin = wHardBin = 0;
                iTotalFailBin++;
            }

            StdfFile.WriteWord((WORD)0);			// NUM_TEST
            StdfFile.WriteWord(wHardBin);           // HARD_BIN
            StdfFile.WriteWord(wSoftBin);           // SOFT_BIN
            StdfFile.WriteWord(iIndex);				// X_COORD
            StdfFile.WriteWord(iWaferRows);			// Y_COORD
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

    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 40;
    // Write HBR
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteByte(255);						// Test Head = ALL
    StdfFile.WriteByte(1);							// Test sites = ALL
    StdfFile.WriteWord(1);							// HBIN = 0
    StdfFile.WriteDword(iTotalGoodBin);				// Total Bins
    StdfFile.WriteByte('P');
    StdfFile.WriteString("");
    StdfFile.WriteRecord();

    // Write HBR
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteByte(255);						// Test Head = ALL
    StdfFile.WriteByte(1);							// Test sites = ALL
    StdfFile.WriteWord(0);							// HBIN = 0
    StdfFile.WriteDword(iTotalFailBin);				// Total Bins
    StdfFile.WriteByte('F');
    StdfFile.WriteString("");
    StdfFile.WriteRecord();

    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 50;

    // Write SBR
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteByte(255);						// Test Head = ALL
    StdfFile.WriteByte(1);							// Test sites = ALL
    StdfFile.WriteWord(1);							// HBIN = 0
    StdfFile.WriteDword(iTotalGoodBin);				// Total Bins
    StdfFile.WriteByte('P');
    StdfFile.WriteString("");
    StdfFile.WriteRecord();

    // Write SBR
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteByte(255);						// Test Head = ALL
    StdfFile.WriteByte(1);							// Test sites = ALL
    StdfFile.WriteWord(0);							// HBIN = 0
    StdfFile.WriteDword(iTotalFailBin);				// Total Bins
    StdfFile.WriteByte('F');
    StdfFile.WriteString("");
    StdfFile.WriteRecord();

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
// Convert 'FileName' PROBER_TSMC file, to STDF 'strFileNameSTDF' file
//////////////////////////////////////////////////////////////////////
bool CGPROBER_TSMCtoSTDF::Convert(const char *Prober_TsmcFileName, const char *strFileNameSTDF)
{
    // No erro (default)
    m_iLastError = errNoError;

    // If STDF file already exists...do not rebuild it...unless dates not matching!
    QFileInfo fInput(Prober_TsmcFileName);
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
            GS::Gex::Engine::GetInstance().UpdateLabelStatus("Converting data from file "+QFileInfo(Prober_TsmcFileName).fileName()+"...");
            GexScriptStatusLabel->show();
        }
    }
    QCoreApplication::processEvents();

    if(ReadProber_TsmcFile(Prober_TsmcFileName,strFileNameSTDF) != true)
    {
        //////////////////////////////////////////////////////////////////////
        // For ProgressBar
        if((GexProgressBar != NULL)
        && bHideProgressAfter)
            GexProgressBar->hide();

        if((GexScriptStatusLabel != NULL)
        && bHideLabelAfter)
            GexScriptStatusLabel->hide();
        return false;	// Error reading PROBER_TSMC file
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
QString CGPROBER_TSMCtoSTDF::ReadLine(QTextStream& hFile)
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
