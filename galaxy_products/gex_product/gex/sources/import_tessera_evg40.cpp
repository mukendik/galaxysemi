//////////////////////////////////////////////////////////////////////
// import_tessera_evg40.cpp: Convert a TesseraEvg40 .csv
// file to STDF V4.0
//////////////////////////////////////////////////////////////////////

#include <math.h>

#ifdef _WIN32
#include "windows.h"		// For 'GetWindowsDirectoryA' declaration
#endif

#include <qfileinfo.h>
#include <qregexp.h>
#include <qprogressbar.h>
#include <qapplication.h>
#include <qlabel.h>
#include "import_tessera_evg40.h"
#include "time.h"
#include "import_constants.h"
#include "engine.h"

#define BIT0			0x01
#define BIT1			0x02
#define BIT2			0x04
#define BIT3			0x08
#define BIT4			0x10
#define BIT5			0x20
#define BIT6			0x40
#define BIT7			0x80

// Evg40 format
//SEP=;
//Measure Summary
//
//File created;2008-12-02 17:59:39
//Substrate ID;17870-30
//Substrate Size;8
//User;lwl
//Measurement Begin;2008-12-02 17:55:54
//Measurement End;2008-12-02 17:59:39
//Measurement Duration;00:03:45
//Machine;EVG40NT
//Template;900-AMetal2BMetal-BoxNBox
//
//Measure Data [µm]
//
//Top to Bottom
//#;X displacement;Y displacement
//1;-0.554;2.082
//2;-0.544;2.093
//3;-0.524;2.074
//4;-0.529;2.045

// main.cpp
extern QLabel			*GexScriptStatusLabel;	// Handle to script status text in status bar
extern QProgressBar	*	GexProgressBar;		// Handle to progress bar in status bar


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGTesseraEvg40toSTDF::CGTesseraEvg40toSTDF()
{
    m_lStartTime	= 0;
    m_lStopTime		= 0;
    m_strSeparator	= "\t";
    m_strTesterType = "Evg40";
    m_nWaferSize	= 0;
}

//////////////////////////////////////////////////////////////////////
// Destruction
//////////////////////////////////////////////////////////////////////
CGTesseraEvg40toSTDF::~CGTesseraEvg40toSTDF()
{
}


//////////////////////////////////////////////////////////////////////
// Get Error
//////////////////////////////////////////////////////////////////////
QString CGTesseraEvg40toSTDF::GetLastError()
{
    strLastError = "Import Tessera "+m_strTesterType+": ";

    switch(iLastError)
    {
        default:
        case errNoError:
            strLastError += "No Error";
            break;
        case errOpenFail:
            strLastError += "Failed to open file";
            break;
        case errInvalidFormat:
            strLastError += "Invalid file format";
            break;
        case errInvalidFormatLowInRows:
            strLastError += "Invalid file format: 'Parameter' line too short, missing rows";
            break;
        case errInvalidFormatPitch:
            strLastError += "Invalid file format: unable to compute Pitch information";
            break;
        case errWriteSTDF:
            strLastError += "Failed creating temporary file. Folder permission issue?";
            break;
        case errLicenceExpired:
            strLastError += "License has expired or Data file out of date...";
            break;
    }
    // Return Error Message
    return strLastError;
}

//////////////////////////////////////////////////////////////////////
// Check if File is compatible with TesseraEvg40 format
//////////////////////////////////////////////////////////////////////
bool CGTesseraEvg40toSTDF::IsCompatible(const char *szFileName)
{
    QString strString;
    QString strValue;
    QString	strSite;

    // Open hCsmFile file
    QFile f( szFileName );
    if(!f.open( QIODevice::ReadOnly ))
    {
        // Failed Opening file
        return false;
    }
    // Assign file I/O stream
    QTextStream hTesseraEvg40File(&f);

    // Check if first line is the correct TesseraEvg40 header...
    //SEP=;
    //Measure Summary

    // Evg40
    strString = hTesseraEvg40File.readLine();
    if(strString.indexOf("SEP=",0,Qt::CaseInsensitive) < 0)
    {
        // If no SEP
        // Default is tab
    }
    else
        strString = hTesseraEvg40File.readLine();
    if(strString.indexOf("Measure Summary",0,Qt::CaseInsensitive) < 0)
    {
        // It's not a Tessera Evg40
        f.close();
        return false;
    }


    f.close();

    return true;
}

//////////////////////////////////////////////////////////////////////
// Read and Parse the TesseraEvg40 file
//////////////////////////////////////////////////////////////////////
bool CGTesseraEvg40toSTDF::ReadTesseraEvg40File(const char *TesseraEvg40FileName, const char *strFileNameSTDF)
{
    QString strString;
    QString strSection;
    bool	bStatus;

    // Open CSV file
    QFile f( TesseraEvg40FileName );
    if(!f.open( QIODevice::ReadOnly ))
    {
        // Failed Opening TesseraEvg40 file
        iLastError = errOpenFail;

        // Convertion failed.
        return false;
    }

    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    iNextFilePos = 0;
    iProgressStep = 0;
    iFileSize = f.size() + 1;

    // Assign file I/O stream
    QTextStream hTesseraEvg40File(&f);

    //SEP=;
    //Measure Summary
    //
    //File created;2008-12-02 17:59:39
    //Substrate ID;17870-30
    //Substrate Size;8
    //User;lwl
    //Measurement Begin;2008-12-02 17:55:54
    //Measurement End;2008-12-02 17:59:39
    //Measurement Duration;00:03:45
    //Machine;EVG40NT
    //Template;900-AMetal2BMetal-BoxNBox
    //
    //Measure Data [µm]
    //
    //Top to Bottom

    strString = ReadLine(hTesseraEvg40File).simplified();
    if(strString.indexOf("SEP=",0,Qt::CaseInsensitive) >= 0)
    {
        m_strSeparator = strString.section("=",1);
        strString = ReadLine(hTesseraEvg40File).simplified();
    }

    if(strString.indexOf("Measure Summary",0,Qt::CaseInsensitive) < 0)
    {
        // It's not a Tessera Evg40
        f.close();
        return false;
    }

    while(!hTesseraEvg40File.atEnd())
    {
        strString = ReadLine(hTesseraEvg40File);
        strSection = strString.section(m_strSeparator,0,0).simplified();
        strString = strString.section(m_strSeparator,1).simplified();

        //Top to Bottom
        if(strSection.startsWith("Top to Bottom",Qt::CaseInsensitive))
        {
            // Data
            break;
        }

        if(strString.isEmpty())
            continue;

        //File created;2008-12-02 17:59:39
        if(strSection.startsWith("File created",Qt::CaseInsensitive))
        {
            QString strDate = strString.section(" ",0,0);
            QString strTime = strString.section(" ",1);
            QDate clDate(strDate.section("-",0,0).toInt(),strDate.section("-",1,1).toInt(),strDate.section("-",2,2).toInt());
            QTime clTime = QTime::fromString(strTime);
            QDateTime clDateTime(clDate,clTime);
            clDateTime.setTimeSpec(Qt::UTC);
            m_lStopTime = clDateTime.toTime_t();
        }
        else
        //Substrate ID;17870-30
        if(strSection.startsWith("Substrate ID",Qt::CaseInsensitive))
        {
            m_strWaferID = strString;
        }
        else
        //Substrate Size;8
        if(strSection.startsWith("Substrate Size",Qt::CaseInsensitive))
        {
            m_nWaferSize = strString.toInt();
        }
        else
        //User;lwl
        if(strSection.startsWith("User",Qt::CaseInsensitive))
        {
            m_strOperatorID = strString;
        }
        else
        //Measurement Begin;2008-12-02 17:55:54
        if(strSection.startsWith("Measurement Begin",Qt::CaseInsensitive))
        {
            QString strDate = strString.section(" ",0,0);
            QString strTime = strString.section(" ",1);
            QDate clDate(strDate.section("-",0,0).toInt(),strDate.section("-",1,1).toInt(),strDate.section("-",2,2).toInt());
            QTime clTime = QTime::fromString(strTime);
            QDateTime clDateTime(clDate,clTime);
            clDateTime.setTimeSpec(Qt::UTC);
            m_lStartTime = clDateTime.toTime_t();
        }
        else
        //Measurement End;2008-12-02 17:59:39
        if(strSection.startsWith("Measurement End",Qt::CaseInsensitive))
        {
            QString strDate = strString.section(" ",0,0);
            QString strTime = strString.section(" ",1);
            QDate clDate(strDate.section("-",0,0).toInt(),strDate.section("-",1,1).toInt(),strDate.section("-",2,2).toInt());
            QTime clTime = QTime::fromString(strTime);
            QDateTime clDateTime(clDate,clTime);
            clDateTime.setTimeSpec(Qt::UTC);
            m_lStopTime = clDateTime.toTime_t();
        }
        else
        //Measurement Duration;00:03:45
        if(strSection.startsWith("Measurement Duration",Qt::CaseInsensitive))
        {
            QString strTime = strString;
            QTime clTime = QTime::fromString(strTime);
            int ellapsedTime = clTime.hour()*60*60 + clTime.minute()*60 + clTime.second();
            if((m_lStopTime > 0) && (m_lStartTime == 0))
                m_lStartTime = m_lStopTime - ellapsedTime;
            if((m_lStartTime > 0) && (m_lStopTime == 0))
                m_lStopTime = m_lStartTime + ellapsedTime;

        }
        else
        //Machine;EVG40NT
        if(strSection.startsWith("Machine",Qt::CaseInsensitive))
        {
            m_strTesterID = strString;
        }
        else
        //Template;900-AMetal2BMetal-BoxNBox
        if(strSection.startsWith("Template",Qt::CaseInsensitive))
        {
            m_strProductID = strString;
        }
        else
        //Measure Data [µm]
        if(strSection.startsWith("Measure Data",Qt::CaseInsensitive))
        {
            m_strUnit = strSection.section("[",1).section("]",0,0);
        }
        else
        {
            // Setup Configuration
            if(!m_strSetupId.isEmpty())
                m_strSetupId += ";";
            m_strSetupId += strSection.remove(' ')+"="+strString.simplified();
        }

    }

    // Loop reading file until end is reached & generate STDF file dynamically.
    bStatus = WriteStdfFile(&hTesseraEvg40File,strFileNameSTDF);
    if(!bStatus)
        QFile::remove(strFileNameSTDF);

    // Success parsing TesseraEvg40 file
    f.close();
    return bStatus;
}

//////////////////////////////////////////////////////////////////////
// Create STDF file from TesseraEvg40 data parsed
//////////////////////////////////////////////////////////////////////
bool CGTesseraEvg40toSTDF::WriteStdfFile(QTextStream *hTesseraEvg40File, const char *strFileNameSTDF)
{
    QString strString;

    // now generate the STDF file...
    GS::StdLib::Stdf StdfFile;
    GS::StdLib::StdfRecordReadInfo RecordReadInfo;
    if(StdfFile.Open((char*)strFileNameSTDF,STDF_WRITE) != GS::StdLib::Stdf::NoError)
    {
        // Failed importing CSV file into STDF database
        iLastError = errWriteSTDF;

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
    StdfFile.WriteString(m_strLotID.toLatin1().constData());		// Lot ID
    StdfFile.WriteString(m_strProductID.toLatin1().constData());	// Part Type / Product ID
    StdfFile.WriteString(m_strTesterID.toLatin1().constData());	// Node name
    StdfFile.WriteString(m_strTesterType.toLatin1().constData());	// Tester Type
    StdfFile.WriteString("");					// Job name
    StdfFile.WriteString("");					// Job rev
    StdfFile.WriteString("");					// sublot-id
    StdfFile.WriteString(m_strOperatorID.toLatin1().constData());	// operator
    StdfFile.WriteString("");					// exec-type
    StdfFile.WriteString("");					// exe-ver
    StdfFile.WriteString("WAFER");				// test-cod
    StdfFile.WriteString("");					// test-temperature
    // Construct custom Galaxy USER_TXT
    QString	strUserTxt;
    strUserTxt = GEX_IMPORT_DATAORIGIN_LABEL;
    strUserTxt += ":";
    strUserTxt += GEX_IMPORT_DATAORIGIN_ATETEST;
    strUserTxt += ":TESSERA_"+m_strTesterType.toUpper();
    StdfFile.WriteString((char *)strUserTxt.toLatin1().constData());	// user-txt
    StdfFile.WriteString("");							// aux-file
    StdfFile.WriteString("");							// package-type
    StdfFile.WriteString(m_strProductID.toLatin1().constData());	// familyID
    StdfFile.WriteString("");							// Date-code
    StdfFile.WriteString("");							// Facility-ID
    StdfFile.WriteString("");							// FloorID
    StdfFile.WriteString("");							// ProcessID
    StdfFile.WriteString("");							// OperationFreq
    StdfFile.WriteString("");							// Spec-nam
    StdfFile.WriteString("");							// Spec-ver
    StdfFile.WriteString("");							// Flow-id
    StdfFile.WriteString((char *)m_strSetupId.left(255).toLatin1().constData());	// setup_id
    StdfFile.WriteRecord();

    // Write Test results for each line read.
    QStringList	lstTestName;
    QString		strSection;
    float		fValue;				// Used for reading floating point numbers.
    int			iTotalPart;
    int			iPartNumber;
    int			iIndex;
    BYTE		bData;

    // Reset counters
    iTotalPart = 0;

    // Write all Parameters read on this wafer.: WIR.PIR,PTR....PTR, PRR, PIR, PTR,PTR...PRR,   ... WRR

    //Top to Bottom
    //#;X displacement;Y displacement
    //1;-0.554;2.082
    //2;-0.544;2.093
    //3;-0.524;2.074
    //4;-0.529;2.045


    // Write WIR of new Wafer.
    RecordReadInfo.iRecordType = 2;
    RecordReadInfo.iRecordSubType = 10;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteByte(1);								// Test head
    StdfFile.WriteByte(255);							// Tester site (all)
    StdfFile.WriteDword(m_lStartTime);					// Start time
    StdfFile.WriteString(m_strWaferID.toLatin1().constData());	// WaferID
    StdfFile.WriteRecord();

    while(!hTesseraEvg40File->atEnd())
    {

        // Read line
        strString = ReadLine(*hTesseraEvg40File);

        if(strString.isEmpty())
            break;

        if(strString.startsWith("#"))
        {
            // List of test
            lstTestName = strString.split(m_strSeparator);
            continue;
        }

        // Part number
        iTotalPart++;
        iPartNumber = strString.section(m_strSeparator,0,0).simplified().toInt();

        // Write PIR for parts in this Wafer site
        RecordReadInfo.iRecordType = 5;
        RecordReadInfo.iRecordSubType = 10;
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte(1);					// Test head
        StdfFile.WriteByte(1);					// Tester site
        StdfFile.WriteRecord();

        // Extract Test value
        for(iIndex=1; iIndex<=strString.count(m_strSeparator); iIndex++)
        {
            // Read Parameter results for this record
            strSection = strString.section(m_strSeparator,iIndex,iIndex);
            fValue = strSection.simplified().toFloat();

            // One test result...write the PTR

            RecordReadInfo.iRecordType = 15;
            RecordReadInfo.iRecordSubType = 10;
            StdfFile.WriteHeader(&RecordReadInfo);
            StdfFile.WriteDword(iIndex);	// Test Number
            StdfFile.WriteByte(1);			// Test head
            StdfFile.WriteByte(1);			// Tester site#

            // No pass/fail information
            bData = 0x40;
            StdfFile.WriteByte(bData);							// TEST_FLG
            StdfFile.WriteByte(0x40|0x80);						// PARAM_FLG
            StdfFile.WriteFloat(fValue);						// Test result

            if(iTotalPart == 1)
            {
                // First record
                // Write Stat Header
                QString strTestName;
                if(lstTestName.count() >= iIndex)
                    strTestName = lstTestName.at(iIndex).simplified();

                StdfFile.WriteString(strTestName.toLatin1().constData());	// TEST_TXT
                StdfFile.WriteString("");					// ALARM_ID

                bData = BIT1|BIT2|BIT3|BIT4|BIT5;	// Valid data.
                StdfFile.WriteByte(bData);					// OPT_FLAG

                StdfFile.WriteByte(0);						// RES_SCALE
                StdfFile.WriteByte(0);						// LLM_SCALE
                StdfFile.WriteByte(0);						// HLM_SCALE
                StdfFile.WriteFloat(0);						// LOW Limit
                StdfFile.WriteFloat(0);						// HIGH Limit
                StdfFile.WriteString(m_strUnit.toLatin1().constData());	// Units
            }
            StdfFile.WriteRecord();
        }

        // Write PRR
        RecordReadInfo.iRecordType = 5;
        RecordReadInfo.iRecordSubType = 20;
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte(1);				// Test head
        StdfFile.WriteByte(1);				// Tester site#:1
        StdfFile.WriteByte(0x10);				// PART_FLG : PASSED
        StdfFile.WriteWord((WORD)iIndex-1);			// NUM_TEST
        StdfFile.WriteWord(1);					// HARD_BIN
        StdfFile.WriteWord(1);					// SOFT_BIN
        StdfFile.WriteWord((WORD)-32768);				// X_COORD
        StdfFile.WriteWord((WORD)-32768);				// Y_COORD
        StdfFile.WriteDword(0);					// No testing time known...
        StdfFile.WriteString(QString::number(iPartNumber).toLatin1().constData());		// PART_ID
        StdfFile.WriteString("");			// PART_TXT
        StdfFile.WriteString("");			// PART_FIX
        StdfFile.WriteRecord();
    };			// Read all lines with valid data records in file


    // Write WRR for last wafer inserted
    RecordReadInfo.iRecordType = 2;
    RecordReadInfo.iRecordSubType = 20;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteByte(1);						// Test head
    StdfFile.WriteByte(255);					// Tester site (all)
    StdfFile.WriteDword(m_lStopTime);			// Time of last part tested
    StdfFile.WriteDword(iTotalPart);			// Parts tested
    StdfFile.WriteDword(0);						// Parts retested
    StdfFile.WriteDword(0);						// Parts Aborted
    StdfFile.WriteDword(0);						// Good Parts
    StdfFile.WriteDword((DWORD)-1);				// Functionnal Parts
    StdfFile.WriteString(m_strWaferID.toLatin1().constData());	// WaferID
    StdfFile.WriteString("");					// FabId
    StdfFile.WriteString("");					// FrameId
    StdfFile.WriteString("");					// MaskId
    StdfFile.WriteString("");					// UserDesc
    StdfFile.WriteRecord();



    // Write WCR for last wafer inserted
    RecordReadInfo.iRecordType = 2;
    RecordReadInfo.iRecordSubType = 30;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteFloat((float)m_nWaferSize);	// Wafer size
    StdfFile.WriteFloat(0);						// Height of die
    StdfFile.WriteFloat(0);						// Width of die
    StdfFile.WriteByte(0);						// Units are in millimeters
    StdfFile.WriteByte((BYTE) ' ');				// Orientation of wafer flat
    StdfFile.WriteWord((WORD)-32768);					// CENTER_X
    StdfFile.WriteWord((WORD)-32768);					// CENTER_Y
    StdfFile.WriteByte((BYTE) ' ');				// POS_X
    StdfFile.WriteByte((BYTE) ' ');				// POS_Y
    StdfFile.WriteRecord();

    // Write MRR
    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 20;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteDword(m_lStopTime);			// File finish-time.
    StdfFile.WriteRecord();

    // Close STDF file.
    StdfFile.Close();

    // Success
    return true;
}

//////////////////////////////////////////////////////////////////////
// Convert 'FileName' TesseraEvg40 file, to STDF 'strFileNameSTDF' file
//////////////////////////////////////////////////////////////////////
bool CGTesseraEvg40toSTDF::Convert(const char *TesseraEvg40FileName, const char *strFileNameSTDF)
{
    // No erro (default)
    iLastError = errNoError;

    // If STDF file already exists...do not rebuild it...unless dates not matching!
    QFileInfo fInput(TesseraEvg40FileName);
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
        GexProgressBar->setMaximum(200);
        GexProgressBar->setTextVisible(true);
        GexProgressBar->setValue(0);
        GexProgressBar->show();
    }

    if(GexScriptStatusLabel != NULL)
    {
        if(GexScriptStatusLabel->isHidden())
        {
            bHideLabelAfter = true;
            GS::Gex::Engine::GetInstance().UpdateLabelStatus("Converting data from file "+QFileInfo(TesseraEvg40FileName).fileName()+"...");
            GexScriptStatusLabel->show();
        }
        GS::Gex::Engine::GetInstance().UpdateLabelStatus("Converting data from file "+QFileInfo(TesseraEvg40FileName).fileName()+"...");
        GexScriptStatusLabel->show();
    }
    QCoreApplication::processEvents();

    if(ReadTesseraEvg40File(TesseraEvg40FileName,strFileNameSTDF) != true)
    {
        //////////////////////////////////////////////////////////////////////
        // For ProgressBar
        if((GexProgressBar != NULL)
        && bHideProgressAfter)
            GexProgressBar->hide();

        if((GexScriptStatusLabel != NULL)
        && bHideLabelAfter)
            GexScriptStatusLabel->hide();
        return false;	// Error reading TesseraEvg40 file
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
QString CGTesseraEvg40toSTDF::ReadLine(QTextStream& hFile)
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
