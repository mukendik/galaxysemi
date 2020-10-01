//////////////////////////////////////////////////////////////////////
// import_tsmc_wpr.cpp: Convert a file.wpr file to STDF V4.0
//////////////////////////////////////////////////////////////////////
#include <qfileinfo.h>
#include <qprogressbar.h>
#include <qapplication.h>
#include <qlabel.h>


#include "import_tsmc_wpr.h"
#include "time.h"
#include "import_constants.h"
#include "engine.h"

//TAIWAN SEMICONDUCTOR MANUFACTURING CO., LTD.
//		   WAFER PROBE RECORD
//
//PART NO: TML429C-EAC8                  CUSTOMER CODE: U470
//LOT NO: K0C909.1                       TESTER: SV3K0006
//P/N: 6510B03                                        TEST DATE: 22-MAY-2009
//
//1. LOT YIELD RESULT
//+------------------------------------------------------------------------+
//|  WAFNO                GOOD-DIE             OPEN               YIELD(%) |
//+------------------------------------------------------------------------+
//| K0C909.01             2521                    8                97.00   |
//| K0C909.02             2526                    3                97.19   |
//| K0C909.25             2492                    0                95.88   |
//+------------------------------------------------------------------------+
//|   25 PCS             62675                   49                96.46   |
//+------------------------------------------------------------------------+
//

// main.cpp
extern QLabel			*GexScriptStatusLabel;	// Handle to script status text in status bar
extern QProgressBar	*	GexProgressBar;		// Handle to progress bar in status bar


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGTsmcWprtoSTDF::CGTsmcWprtoSTDF()
{
    m_lStartTime = 0;
}

//////////////////////////////////////////////////////////////////////
// Destruction
//////////////////////////////////////////////////////////////////////
CGTsmcWprtoSTDF::~CGTsmcWprtoSTDF()
{
}

//////////////////////////////////////////////////////////////////////
// Get Error
//////////////////////////////////////////////////////////////////////
QString CGTsmcWprtoSTDF::GetLastError()
{
    m_strLastError = "Import TsmcWpr: ";

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
// Check if File is compatible with TsmcWpr format
//////////////////////////////////////////////////////////////////////
bool CGTsmcWprtoSTDF::IsCompatible(const char *szFileName)
{
    QString strString;
    QString strSection;
    QString strValue;
    QString	strSite;
    bool	bIsCompatible = false;

    // Open hCsmFile file
    QFile f( szFileName );
    if(!f.open( QIODevice::ReadOnly ))
    {
        // Failed Opening ASL1000 file
        return false;
    }
    // Assign file I/O stream

    //TAIWAN SEMICONDUCTOR MANUFACTURING
    //WAFER PROBE RECORD

    QTextStream hTsmcWprFile(&f);

    while(!hTsmcWprFile.atEnd())
    {
        do
            strSection = hTsmcWprFile.readLine().simplified().toUpper();
        while(!strSection.isNull() && strSection.isEmpty());

        if(strSection.left(34) == "TAIWAN SEMICONDUCTOR MANUFACTURING")
            continue;
        if(strSection == "WAFER PROBE RECORD")
        {
            bIsCompatible = true;
            break;
        }
        else
        {
            // Incorrect header...this is not a TsmcWpr file!
            bIsCompatible = false;
            break;
        }
    }

    f.close();

    return bIsCompatible;
}

//////////////////////////////////////////////////////////////////////
// Read and Parse the TsmcWpr file
//////////////////////////////////////////////////////////////////////
bool CGTsmcWprtoSTDF::ReadTsmcWprFile(const char* TsmcWprFileName,
                                      const char* strFileNameSTDF)
{
    QString strLine;
    QString strSection;
    QString strValue;

    // Open CSV file
    QFile f( TsmcWprFileName );
    if(!f.open( QIODevice::ReadOnly ))
    {
        // Failed Opening TsmcWpr file
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

    // Assign file I/O stream
    QTextStream hTsmcWprFile(&f);

    strLine = "";
    while(!hTsmcWprFile.atEnd())
    {
        if(strLine.isEmpty())
            strLine = ReadLine(hTsmcWprFile).simplified();

        strSection = strLine.section(":",0,0).trimmed().toUpper();
        strValue = strLine.section(":",1).trimmed();
        if(strValue.indexOf(' ') > 0)
        {
            strLine = strValue.section(' ',1);
            strValue = strValue.section(' ',0,0);
        }
        else
            strLine = "";

        if(strSection == "PART NO")
        {//PART NO: TML429C-EAC8
            m_strPartType=strValue;
        }
        else if(strSection == "LOT NO")
        {//LOT NO: K0C909.1
            m_strLotId=strValue;
        }
        else if(strSection == "P/N")
        {//P/N: 6510B03
            m_strPackageType=strValue;
        }
        else if(strSection == "CUSTOMER CODE")
        {//CUSTOMER CODE: U470
            m_strTestCode=strValue;
        }
        else if(strSection == "TESTER")
        {//TESTER: SV3K0006
            m_strTesterType=strValue;
        }
        else if(strSection == "TEST DATE")
        {//TEST DATE: 22-MAY-2009
            QString strMonth = strValue.mid(3,1).toUpper()+strValue.mid(4,2).toLower();
            int iDay = strValue.left(2).toInt();
            int iMonth = 0;
            int iYear = strValue.right(4).toInt();

            if(strMonth == "Jan") iMonth = 1;
            if(strMonth == "Feb") iMonth = 2;
            if(strMonth == "Mar") iMonth = 3;
            if(strMonth == "Apr") iMonth = 4;
            if(strMonth == "May") iMonth = 5;
            if(strMonth == "Jun") iMonth = 6;
            if(strMonth == "Jul") iMonth = 7;
            if(strMonth == "Aug") iMonth = 8;
            if(strMonth == "Sep") iMonth = 9;
            if(strMonth == "Oct") iMonth = 10;
            if(strMonth == "Nov") iMonth = 11;
            if(strMonth == "Dec") iMonth = 12;

            QDate clDate(iYear,iMonth,iDay);
            QDateTime clDateTime(clDate);
            clDateTime.setTimeSpec(Qt::UTC);
            m_lStartTime = clDateTime.toTime_t();
        }
        else if(strSection.indexOf("LOT YIELD RESULT") > 0)
        {
            //LOT YIELD RESULT
            break;
        }
    }


    if(strSection.indexOf("LOT YIELD RESULT") <= 0)
    {
        // Incorrect header...this is not a TsmcWpr file!
        m_iLastError = errInvalidFormat;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }

    // write STDF file
    if(!WriteStdfFile(&hTsmcWprFile,strFileNameSTDF))
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
// Create STDF file from TsmcWpr data parsed
//////////////////////////////////////////////////////////////////////
bool CGTsmcWprtoSTDF::WriteStdfFile(QTextStream *hTsmcWprFile, const char *strFileNameSTDF)
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
    StdfFile.WriteString(m_strPartType.toLatin1().constData());	// Part Type / Product ID
    StdfFile.WriteString("");					// Node name
    StdfFile.WriteString(m_strTesterType.toLatin1().constData());// Tester Type
    StdfFile.WriteString("");					// Job name
    StdfFile.WriteString("");					// Job rev
    StdfFile.WriteString("");					// sublot-id
    StdfFile.WriteString("");					// operator
    StdfFile.WriteString("");					// exec-type
    StdfFile.WriteString("");					// exe-ver
    StdfFile.WriteString(m_strTestCode.toLatin1().constData());// test-cod
    StdfFile.WriteString("");					// test-temperature
    // Construct custom Galaxy USER_TXT
    QString	strUserTxt;
    strUserTxt = GEX_IMPORT_DATAORIGIN_LABEL;
    strUserTxt += ":";
    strUserTxt += GEX_IMPORT_DATAORIGIN_ATETEST;
    strUserTxt += ":TSMC/WPR";
    StdfFile.WriteString(strUserTxt.toLatin1().constData());		// user-txt
    StdfFile.WriteString("");					// aux-file
    StdfFile.WriteString(m_strPackageType.toLatin1().constData());// package-type
    StdfFile.WriteString("");					// familyID
    StdfFile.WriteString("");					// Date-code
    StdfFile.WriteString("");					// Facility-ID
    StdfFile.WriteString("");					// FloorID
    StdfFile.WriteString("");					// ProcessID
    StdfFile.WriteRecord();

    // Write wafer results for each line read.
    QString		strString;
    QString		strGoodBin;
    int			iTotalPart=0;
    int			iTotalGoodPart=0;
    float		fYield, fTotalPart;

    QString		strBin;
    int			iTotalBin=0;

    //  WIR ... WRR
    while(hTsmcWprFile->atEnd() == false)
    {


        // Read line
        strString = ReadLine(*hTsmcWprFile).remove('|').simplified().toUpper();
        if(strString.left(10) == "+---------")
        {
            if(m_strWaferId.isEmpty())
                continue;
            else
            {
                // read the summary line
                strString = ReadLine(*hTsmcWprFile).remove('|').simplified().toUpper();
                //25 PCS             62675                   49                96.46
                iTotalGoodPart = strString.section(" ",2,2).toInt();
                iTotalBin = strString.section(" ",3,3).toInt();
                fYield = strString.section(" ",4,4).toFloat();
                if(fYield > 0.0)
                {
                    fTotalPart = (100.0 / fYield * (float)iTotalGoodPart);
                    iTotalPart = (int)fTotalPart;
                    if((int)(fTotalPart) < (int)(fTotalPart+0.5))
                        iTotalPart++;
                }

                break;
            }
        }
        if(strString.left(6) == "WAFNO ")
        {
            strGoodBin = strString.section(" ",1,1);
            strBin = strString.section(" ",2,2);
            continue;
        }

        // Reset counters
        iTotalGoodPart=iTotalPart=0;

        m_strWaferId = strString.section(" ",0,0);
        iTotalGoodPart = strString.section(" ",1,1).toInt();
        fYield = strString.section(" ",3,3).toFloat();
        if(fYield > 0.0)
        {
            fTotalPart = (100.0 / fYield * (float)iTotalGoodPart);
            iTotalPart = (int)fTotalPart;
            if((int)(fTotalPart) < (int)(fTotalPart+0.5))
                iTotalPart++;
        }

        // Write WIR of new Wafer.
        RecordReadInfo.iRecordType = 2;
        RecordReadInfo.iRecordSubType = 10;
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte(1);							// Test head
        StdfFile.WriteByte(255);						// Tester site (all)
        StdfFile.WriteDword(m_lStartTime);				// Start time
        StdfFile.WriteString(m_strWaferId.toLatin1().constData());	// WaferID
        StdfFile.WriteRecord();

        // Write WRR for last wafer inserted
        RecordReadInfo.iRecordType = 2;
        RecordReadInfo.iRecordSubType = 20;
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte(1);						// Test head
        StdfFile.WriteByte(255);					// Tester site (all)
        StdfFile.WriteDword(m_lStartTime);			// Time of last part tested
        StdfFile.WriteDword(iTotalPart);			// Parts tested
        StdfFile.WriteDword(0);						// Parts retested
        StdfFile.WriteDword(0);						// Parts Aborted
        StdfFile.WriteDword(iTotalGoodPart);			// Good Parts
        StdfFile.WriteDword(4294967295UL);			// Functionnal Parts
        StdfFile.WriteString(m_strWaferId.toLatin1().constData());	// WaferID
        StdfFile.WriteRecord();

    };			// Read all lines with valid data records in file


    if(!strBin.isEmpty() &&(iTotalBin>0))
    {
        RecordReadInfo.iRecordType = 1;
        RecordReadInfo.iRecordSubType = 40;
        // Write HBR
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte(1);						// Test Head
        StdfFile.WriteByte(255);					// Test sites
        StdfFile.WriteWord(0);						// HBIN
        StdfFile.WriteDword(iTotalBin);				// Total Bins
        StdfFile.WriteByte('F');
        StdfFile.WriteString( strBin.toLatin1().constData());
        StdfFile.WriteRecord();

        RecordReadInfo.iRecordType = 1;
        RecordReadInfo.iRecordSubType = 50;
        // Write SBR
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte(1);						// Test Head
        StdfFile.WriteByte(255);					// Test sites
        StdfFile.WriteWord(0);						// SBIN
        StdfFile.WriteDword(iTotalBin);				// Total Bins
        StdfFile.WriteByte('F');
        StdfFile.WriteString( strBin.toLatin1().constData());
        StdfFile.WriteRecord();

        RecordReadInfo.iRecordType = 1;
        RecordReadInfo.iRecordSubType = 40;
        // Write HBR
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte(1);						// Test Head
        StdfFile.WriteByte(255);					// Test sites
        StdfFile.WriteWord(2);						// HBIN
        StdfFile.WriteDword(iTotalPart-iTotalGoodPart-iTotalBin);				// Total Bins
        StdfFile.WriteByte('F');
        StdfFile.WriteString( "Others");
        StdfFile.WriteRecord();

        RecordReadInfo.iRecordType = 1;
        RecordReadInfo.iRecordSubType = 50;
        // Write SBR
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte(1);						// Test Head
        StdfFile.WriteByte(255);					// Test sites
        StdfFile.WriteWord(2);						// SBIN
        StdfFile.WriteDword(iTotalPart-iTotalGoodPart-iTotalBin);				// Total Bins
        StdfFile.WriteByte('F');
        StdfFile.WriteString( "Others");
        StdfFile.WriteRecord();
    }

    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 40;
    // Write HBR
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteByte(1);						// Test Head
    StdfFile.WriteByte(255);					// Test sites
    StdfFile.WriteWord(1);						// HBIN
    StdfFile.WriteDword(iTotalGoodPart);		// Total Bins
    StdfFile.WriteByte('P');
    StdfFile.WriteString( strGoodBin.toLatin1().constData());
    StdfFile.WriteRecord();

    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 50;
    // Write SBR
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteByte(1);						// Test Head
    StdfFile.WriteByte(255);					// Test sites
    StdfFile.WriteWord(1);						// SBIN
    StdfFile.WriteDword(iTotalGoodPart);		// Total Bins
    StdfFile.WriteByte('P');
    StdfFile.WriteString( strGoodBin.toLatin1().constData());
    StdfFile.WriteRecord();

    // Write PCR
    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 30;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteByte(255);					// Test Head = ALL
    StdfFile.WriteByte(255);					// Test sites = ALL
    StdfFile.WriteDword(iTotalPart);			// Total Parts tested
    StdfFile.WriteDword(0);						// Total Parts re-tested
    StdfFile.WriteDword(0);						// Total Parts aborted
    StdfFile.WriteDword(iTotalGoodPart);			// Total GOOD Parts
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
// Convert 'FileName' TsmcWpr file, to STDF 'strFileNameSTDF' file
//////////////////////////////////////////////////////////////////////
bool CGTsmcWprtoSTDF::Convert(const char *TsmcWprFileName, const char *strFileNameSTDF)
{
    // No erro (default)
    m_iLastError = errNoError;

    // If STDF file already exists...do not rebuild it...unless dates not matching!
    QFileInfo fInput(TsmcWprFileName);
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
            GS::Gex::Engine::GetInstance().UpdateLabelStatus("Converting data from file "+QFileInfo(TsmcWprFileName).fileName()+"...");
            GexScriptStatusLabel->show();
        }
    }
    QCoreApplication::processEvents();

    if(ReadTsmcWprFile(TsmcWprFileName,strFileNameSTDF) != true)
    {
        //////////////////////////////////////////////////////////////////////
        // For ProgressBar
        if((GexProgressBar != NULL)
        && bHideProgressAfter)
            GexProgressBar->hide();

        if((GexScriptStatusLabel != NULL)
        && bHideLabelAfter)
            GexScriptStatusLabel->hide();
        return false;	// Error reading TsmcWpr file
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
QString CGTsmcWprtoSTDF::ReadLine(QTextStream& hFile)
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
