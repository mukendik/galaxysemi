//////////////////////////////////////////////////////////////////////
// import_tessera_Diffractive.cpp: Convert a TesseraDiffractive .csv
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
#include "import_tessera_diffractive.h"
#include "time.h"
#include "import_constants.h"
#include "engine.h"

// Diffractive format 1
//X1-ppenilla-diffrac2-S-18538-12UV-200906121554-Sample
//18538-12UV
//X1
//0.730000	0.820000	389.000000	36.000000	51.000000
//1.000000
//830	Scribed	-1.071075E-2
//2.925500E+4	8.213800E+4	7.815628E-1	8.459900E+2	2.368640E-3	7.808024E-1	1.462750E-3

// Diffractive format 2
//0th-Carolyn-DT1A-S-18872-18UV-200906101024-FULL
//18872-18UV
//DT1A
//0.233000	0.333000	879.000000	24.000000	44.000000
//1
 //660	Scribed	3.370944E-1
//1.435200E+4	3.364400E+4	3.524132E-1

// main.cpp
extern QLabel			*GexScriptStatusLabel;	// Handle to script status text in status bar
extern QProgressBar	*	GexProgressBar;		// Handle to progress bar in status bar


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGTesseraDiffractivetoSTDF::CGTesseraDiffractivetoSTDF()
{
    m_lStartTime = 0;
    m_strTesterType = "Diffrative";
}

//////////////////////////////////////////////////////////////////////
// Destruction
//////////////////////////////////////////////////////////////////////
CGTesseraDiffractivetoSTDF::~CGTesseraDiffractivetoSTDF()
{
}


//////////////////////////////////////////////////////////////////////
// Get Error
//////////////////////////////////////////////////////////////////////
QString CGTesseraDiffractivetoSTDF::GetLastError()
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
// Check if File is compatible with TesseraDiffractive format
//////////////////////////////////////////////////////////////////////
bool CGTesseraDiffractivetoSTDF::IsCompatible(const char *szFileName)
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
    QTextStream hTesseraDiffractiveFile(&f);

    // Check if first line is the correct TesseraDiffractive header...
    //X1-ppenilla-diffrac2-S-18538-12UV-200906121554-Sample
    //18538-12UV
    //X1
    //0.730000	0.820000	389.000000	36.000000	51.000000
    //1.000000
    //830	Scribed	-1.071075E-2


    int nLine = 0;

    do
    {
        strString = hTesseraDiffractiveFile.readLine().simplified().toLower();
        nLine++;

        // Diffractive

        if(strString.section(" ",1,1) == "scribed")
        {
            // It's a Tessera diffraction
            f.close();
            return true;
        }
        if(nLine > 6)
            break;
    }
    while(!hTesseraDiffractiveFile.atEnd());


    f.close();

    return false;
}

//////////////////////////////////////////////////////////////////////
// Read and Parse the TesseraDiffractive file
//////////////////////////////////////////////////////////////////////
bool CGTesseraDiffractivetoSTDF::ReadTesseraDiffractiveFile(const char *TesseraDiffractiveFileName, const char *strFileNameSTDF)
{
    QString strString;
    QString strSection;
    bool	bStatus;

    // Open CSV file
    QFile f( TesseraDiffractiveFileName );
    if(!f.open( QIODevice::ReadOnly ))
    {
        // Failed Opening TesseraDiffractive file
        iLastError = errOpenFail;

        // Convertion failed.
        return false;
    }

    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    iNextFilePos = 0;
    iProgressStep = 0;
    iFileSize = f.size() + 1;

    m_nPass = 1;

    // Assign file I/O stream
    QTextStream hTesseraDiffractiveFile(&f);

    // Tessera Naming convention
    //
    //X1-ppenilla-diffrac2-S-18538-12UV-200906121554-Sample
    //18538-12UV
    //X1
    //0.730000	0.820000	389.000000	36.000000	51.000000
    //1.000000
    //830	Scribed	-1.071075E-2

    // TestName = X1
    // OperatorName = ppenilla
    // MachineName = diffrac2
    // Scribed or Unscribed = S ou U
    // ContainerName or WaferId = 18538-12UV
    // Date = 200906121554
    // Test Method = FULL or Sample

    m_strWaferID = "1";
    m_strTesterType = "Diffractive";
    m_lStartTime = m_lStopTime = 0;

    //X1-ppenilla-diffrac2-S-18538-12UV-200906121554-Sample
    strString = ReadLine(hTesseraDiffractiveFile);
    m_strJobName = strString.simplified();
    m_strSetupId = "LensSettings="+m_strJobName;

    if(strString.count("-") >= 7)
    {
        QDate	clDate;
        QTime	clTime;
        // Good naming convention
        // extract info
        m_strProductID = strString.section("-",0,0);
        m_strOperatorID = strString.section("-",1,1);
        m_strTesterID = strString.section("-",2,2);
        m_strWaferID = strString.section("-",4,5);
        m_strJobRev = strString.section("-",7,7);

        //Date Time:200906121554
        QString	strDate = strString.section("-",6,6);
        clDate = QDate(strDate.left(4).toInt(),strDate.mid(4,2).toInt(),strDate.mid(6,2).toInt());
        clTime.setHMS(strDate.mid(8,2).toInt(), strDate.right(2).toInt(), 0);

        QDateTime clDateTime(clDate,clTime);
        clDateTime.setTimeSpec(Qt::UTC);
        m_lStartTime = clDateTime.toTime_t();
    }

    strString = "";
    // goto to Data

    while(!hTesseraDiffractiveFile.atEnd())
    {
        // Reference Power has saved just before the "Scribed" line
        m_strProcessID = strString;

        strString = ReadLine(hTesseraDiffractiveFile).simplified();
        if(strString.indexOf("scribed",0,Qt::CaseInsensitive) > 0)
        {
            if(!m_strSetupId.isEmpty())
                m_strSetupId += ";";
            m_strSetupId += "Side="+strString.section(" ",1,1).simplified();

            // Data
            break;
        }
    }

    strString = strString.toLower();

    //Column,Row,Part ID,Wafer ID,# of pits
    if(strString.section(" ",1,1) != "scribed")
    {
        // Incorrect header...this is not a TesseraDiffractive file!
        iLastError = errInvalidFormat;
        f.close();
        return false;
    }


    // Have to find the XMin, YMin, XPitch, YPitch
    float fCurrentX, fCurrentY, fFloat;
    m_fXMin = m_fYMin = m_fXPitch = m_fYPitch = m_fXMax = m_fYMax = fCurrentX = fCurrentY = fFloat = 0.0;

    while(!hTesseraDiffractiveFile.atEnd())
    {
        strString = ReadLine(hTesseraDiffractiveFile).simplified();
        if(strString.isEmpty())
        {
            // no more Data
            break;
        }

        // X coordinate
        fFloat = strString.section(" ",0,0).toFloat();
        // XPitch
        if((fCurrentX > 0) && (fFloat != fCurrentX))
        {
            if(fFloat > fCurrentX)
            {
                if((m_fXPitch == 0) || ((fFloat - fCurrentX) < m_fXPitch))
                    m_fXPitch = (fFloat - fCurrentX);
            }
            else
            {
                if((m_fXPitch == 0) || ((fCurrentX - fFloat) < m_fXPitch))
                    m_fXPitch = (fCurrentX - fFloat);
            }
        }
        // XMin
        if((m_fXMin == 0) || (fFloat < m_fXMin))
            m_fXMin = fFloat;
        // XMax
        if((m_fXMax == 0) || (fFloat > m_fXMax))
            m_fXMax = fFloat;

        fCurrentX = fFloat;

        // Y coordinate
        fFloat = strString.section(" ",1,1).toFloat();
        // YPitch
        if((fCurrentY > 0) && (fFloat != fCurrentY))
        {
            if(fFloat > fCurrentY)
            {
                if((m_fYPitch == 0) || ((fFloat - fCurrentY) < m_fYPitch))
                    m_fYPitch = (fFloat - fCurrentY);
            }
            else
            {
                if((m_fYPitch == 0) || ((fCurrentY - fFloat) < m_fYPitch))
                    m_fYPitch = (fCurrentY - fFloat);
            }
        }
        // YMin
        if((m_fYMin == 0) || (fFloat < m_fYMin))
            m_fYMin = fFloat;
        // YMax
        if((m_fYMax == 0) || (fFloat > m_fYMax))
            m_fYMax = fFloat;

        fCurrentY = fFloat;

    }

    // Restart at the begining
    f.close();

    // Check if have correct Pitch
    if((m_fXPitch == 0) || (((fCurrentX-m_fXMin)/m_fXPitch) != (float)(int)((fCurrentX-m_fXMin)/m_fXPitch)))
    {
        // No Pitch or not integer convertion
        // Ignore this error, no convertion avalaible
        //iLastError = errInvalidFormatPitch;
        //return false;

        m_fXMin = m_fYMin = m_fXPitch = m_fYPitch = m_fXMax = m_fYMax = 0.0;

    }

    // Check if have correct Pitch
    if((m_fYPitch == 0) || (((fCurrentY-m_fYMin)/m_fYPitch) != (float)(int)((fCurrentY-m_fYMin)/m_fYPitch)))
    {
        // No Pitch or not integer convertion
        // Ignore this error, no convertion avalaible
        //iLastError = errInvalidFormatPitch;
        //return false;

        m_fXMin = m_fYMin = m_fXPitch = m_fYPitch = m_fXMax = m_fYMax = 0.0;
    }

    if(!f.open( QIODevice::ReadOnly ))
    {
        // Failed Opening TesseraDiffractive file
        iLastError = errOpenFail;

        // Convertion failed.
        return false;
    }

    // Loop reading file until end is reached & generate STDF file dynamically.
    bStatus = WriteStdfFile(&hTesseraDiffractiveFile,strFileNameSTDF);
    if(!bStatus)
        QFile::remove(strFileNameSTDF);

    // Success parsing TesseraDiffractive file
    f.close();
    return bStatus;
}

//////////////////////////////////////////////////////////////////////
// Create STDF file from TesseraDiffractive data parsed
//////////////////////////////////////////////////////////////////////
bool CGTesseraDiffractivetoSTDF::WriteStdfFile(QTextStream *hTesseraDiffractiveFile, const char *strFileNameSTDF)
{
    m_nPass = 2;
    iNextFilePos = 0;

    QString strString;
    while(!hTesseraDiffractiveFile->atEnd())
    {
        strString = ReadLine(*hTesseraDiffractiveFile);
        if(strString.indexOf("scribed",0,Qt::CaseInsensitive) > 0)
        {
            // Data
            break;
        }
    }


    //Column,Row,Part ID,Wafer ID,# of pits
    if(strString.indexOf("scribed",0,Qt::CaseInsensitive) < 0)
    {
        // Incorrect header...this is not a TesseraDiffractive file!
        iLastError = errInvalidFormat;
        return false;
    }

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
    StdfFile.WriteString(m_strJobName.toLatin1().constData());	// Job name
    StdfFile.WriteString(m_strJobRev.toLatin1().constData());	// Job rev
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
    StdfFile.WriteString(m_strProcessID.toLatin1().constData());	// ProcessID
    StdfFile.WriteString("");							// OperationFreq
    StdfFile.WriteString("");							// Spec-nam
    StdfFile.WriteString("");							// Spec-ver
    StdfFile.WriteString("");							// Flow-id
    StdfFile.WriteString((char *)m_strSetupId.left(255).toLatin1().constData());	// setup_id
    StdfFile.WriteRecord();

    // Write Test results for each line read.
    QString strSection;
    float	fValue;				// Used for readng floating point numbers.
    int		iXpos, iYpos;
    int		iTotalPart;
    BYTE	bData;

    // Reset counters
    iTotalPart = 0;

    // Write all Parameters read on this wafer.: WIR.PIR,PTR....PTR, PRR, PIR, PTR,PTR...PRR,   ... WRR

    //830	Scribed	-1.071075E-2
    //2.925500E+4	8.213800E+4	7.815628E-1	8.459900E+2	2.368640E-3	7.808024E-1	1.462750E-3

    // Write WIR of new Wafer.
    RecordReadInfo.iRecordType = 2;
    RecordReadInfo.iRecordSubType = 10;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteByte(1);								// Test head
    StdfFile.WriteByte(255);							// Tester site (all)
    StdfFile.WriteDword(m_lStartTime);					// Start time
    StdfFile.WriteString(m_strWaferID.toLatin1().constData());	// WaferID
    StdfFile.WriteRecord();

    while(!hTesseraDiffractiveFile->atEnd())
    {

        // Part number
        iTotalPart++;

        // Read line
        strString = ReadLine(*hTesseraDiffractiveFile).simplified();

        if(strString.isEmpty())
            break;

        // Extract Column,Row,Pass
        // (XPos - XMin)/XPitch
        iXpos = iYpos = -32768;
        if(m_fXPitch > 0.0)
            iXpos = (int) ((strString.section(" ",0,0).toFloat() - m_fXMin)/m_fXPitch);
        if(m_fYPitch > 0.0)
            iYpos = (int) ((strString.section(" ",1,1).toFloat() - m_fYMin)/m_fYPitch);

        // Write PIR for parts in this Wafer site
        RecordReadInfo.iRecordType = 5;
        RecordReadInfo.iRecordSubType = 10;
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte(1);					// Test head
        StdfFile.WriteByte(1);					// Tester site
        StdfFile.WriteRecord();

        // Read Parameter results for this record
        strSection = strString.section(" ",2,2);
        fValue = strSection.toFloat();

        // One test result...write the PTR

        RecordReadInfo.iRecordType = 15;
        RecordReadInfo.iRecordSubType = 10;
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteDword(1);			// Test Number
        StdfFile.WriteByte(1);			// Test head
        StdfFile.WriteByte(1);			// Tester site#

        // No pass/fail information
        bData = 0x40;
        StdfFile.WriteByte(bData);							// TEST_FLG
        StdfFile.WriteByte(0x40|0x80);						// PARAM_FLG
        StdfFile.WriteFloat(fValue);						// Test result
        StdfFile.WriteRecord();


        // Write PRR
        RecordReadInfo.iRecordType = 5;
        RecordReadInfo.iRecordSubType = 20;
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte(1);				// Test head
        StdfFile.WriteByte(1);				// Tester site#:1
        StdfFile.WriteByte(0x10);				// PART_FLG : PASSED
        StdfFile.WriteWord((WORD)1);			// NUM_TEST
        StdfFile.WriteWord(1);					// HARD_BIN
        StdfFile.WriteWord(1);					// SOFT_BIN
        StdfFile.WriteWord(iXpos);				// X_COORD
        StdfFile.WriteWord(iYpos);				// Y_COORD
        StdfFile.WriteDword(0);					// No testing time known...
        StdfFile.WriteString(strString.section(" ",0,1).toLatin1().constData());		// PART_ID
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
    StdfFile.WriteFloat(0);				// Wafer size
    fValue = m_fXPitch;
    fValue /= 1000;
    StdfFile.WriteFloat(fValue);				// Height of die
    fValue = m_fYPitch;
    fValue /= 1000;
    StdfFile.WriteFloat(fValue);				// Width of die
    StdfFile.WriteByte(3);						// Units are in millimeters
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
    StdfFile.WriteDword(m_lStartTime);			// File finish-time.
    StdfFile.WriteRecord();

    // Close STDF file.
    StdfFile.Close();

    // Success
    return true;
}

//////////////////////////////////////////////////////////////////////
// Convert 'FileName' TesseraDiffractive file, to STDF 'strFileNameSTDF' file
//////////////////////////////////////////////////////////////////////
bool CGTesseraDiffractivetoSTDF::Convert(const char *TesseraDiffractiveFileName, const char *strFileNameSTDF)
{
    // No erro (default)
    iLastError = errNoError;

    // If STDF file already exists...do not rebuild it...unless dates not matching!
    QFileInfo fInput(TesseraDiffractiveFileName);
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
            GS::Gex::Engine::GetInstance().UpdateLabelStatus("Converting data from file "+QFileInfo(TesseraDiffractiveFileName).fileName()+"...");
        }
        GS::Gex::Engine::GetInstance().UpdateLabelStatus("Converting data from file "+QFileInfo(TesseraDiffractiveFileName).fileName()+"...");
    }

    QCoreApplication::processEvents();

    if(ReadTesseraDiffractiveFile(TesseraDiffractiveFileName,strFileNameSTDF) != true)
    {
        //////////////////////////////////////////////////////////////////////
        // For ProgressBar
        if((GexProgressBar != NULL)
        && bHideProgressAfter)
            GexProgressBar->hide();

        if((GexScriptStatusLabel != NULL)
        && bHideLabelAfter)
            GexScriptStatusLabel->hide();
        return false;	// Error reading TesseraDiffractive file
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
QString CGTesseraDiffractivetoSTDF::ReadLine(QTextStream& hFile)
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
            GexProgressBar->setValue(iProgressStep + ((m_nPass-1)*100));
        }
    }
    QCoreApplication::processEvents();

    do
        strString = hFile.readLine();
    while(!strString.isNull() && strString.isEmpty());

    return strString;

}
