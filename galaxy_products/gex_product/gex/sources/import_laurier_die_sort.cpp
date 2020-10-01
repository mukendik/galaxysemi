//////////////////////////////////////////////////////////////////////
// import_LaurierDieSort.cpp: Convert a LaurierDieSort file to STDF V4.0
//////////////////////////////////////////////////////////////////////

#include <qfileinfo.h>
#include <qprogressbar.h>
#include <qapplication.h>
#include <qlabel.h>
#include <QMap>


#include "import_laurier_die_sort.h"
#include "time.h"
#include "import_constants.h"
#include "engine.h"

//# Version = b.6.0.3
//# Spec Rev = 1
//# Eng Fail Rev = 4
//# productid = 00062c400000f6f7
//#  date = 4/29/2009
//# Die Size X = 2554.0
//# Die Size Y = 2554.0
//         72          0        100
//         72          1        100


// main.cpp
extern QLabel			*GexScriptStatusLabel;	// Handle to script status text in status bar
extern QProgressBar	*	GexProgressBar;		// Handle to progress bar in status bar

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGLaurierDieSorttoSTDF::CGLaurierDieSorttoSTDF()
{
    m_lStartTime = 0;
}

//////////////////////////////////////////////////////////////////////
// Destruction
//////////////////////////////////////////////////////////////////////
CGLaurierDieSorttoSTDF::~CGLaurierDieSorttoSTDF()
{
}

//////////////////////////////////////////////////////////////////////
// Get Error
//////////////////////////////////////////////////////////////////////
QString CGLaurierDieSorttoSTDF::GetLastError()
{
    m_strLastError = "Import Laurier Die Sort 1D: ";

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
// Check if File is compatible with LaurierDieSort format
//////////////////////////////////////////////////////////////////////
bool CGLaurierDieSorttoSTDF::IsCompatible(const char *szFileName)
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
    QTextStream hLaurierDieSortFile(&f);

    // Check if first line is the correct LaurierDieSort header...
    do
        strString = hLaurierDieSortFile.readLine().remove(" ");
    while(!strString.isNull() && strString.isEmpty());

    // Close file
    f.close();

    //# Version = b.6.0.3
    strString = strString.toUpper();
    if(!strString.startsWith("#VERSION="))
    {
        // Incorrect header...this is not a LaurierDieSort file!
        return false;
    }

    return true;
}

//////////////////////////////////////////////////////////////////////
// Read and Parse the LaurierDieSort file
//////////////////////////////////////////////////////////////////////
bool CGLaurierDieSorttoSTDF::ReadLaurierDieSortFile(const char *LaurierDieSortFileName,const char *strFileNameSTDF)
{
    QString strString;
    QString strSection;

    // Open CSV file
    QFile f( LaurierDieSortFileName );
    if(!f.open( QIODevice::ReadOnly ))
    {
        // Failed Opening LaurierDieSort file
        m_iLastError = errOpenFail;

        // Convertion failed.
        return false;
    }

    // Assign file I/O stream
    QTextStream hLaurierDieSortFile(&f);

    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    iProgressStep = 0;
    iNextFilePos = 0;
    iFileSize = f.size() + 1;
    QCoreApplication::processEvents();


    // Check if first line is the correct LaurierDieSort header...
    strString = ReadLine(hLaurierDieSortFile);
    strSection = strString.section("=",0,0).remove(" ").toUpper();

    //# Version = b.6.0.3
    if(strSection != "#VERSION")
    {
        // Incorrect header...this is not a LaurierDieSort file!
        m_iLastError = errInvalidFormat;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }

    m_strJobRev = strString.section("=",1).simplified();

    while(!hLaurierDieSortFile.atEnd())
    {
        strString = ReadLine(hLaurierDieSortFile);
        strSection = strString.section("=",0,0).remove(" ").toUpper();

        if(strSection == "#SPECREV")
        {
            //# Spec Rev = 1
        }
        else
        if(strSection == "#ENGFAILREV")
        {
            //# Eng Fail Rev = 4
        }
        else
        if(strSection == "#PRODUCTID")
        {
            //# productid = 00062c400000f6f7
            m_strDeviceId = strString.section("=",1).simplified();
        }
        else
        if(strSection == "#DATE")
        {
            //#  date = 4/29/2009
            QDate clDate = QDate::fromString(strString.section("=",1).remove(" "),"M/d/yyyy");
            QDateTime clDateTime(clDate,QTime());
            clDateTime.setTimeSpec(Qt::UTC);
            m_lStartTime = clDateTime.toTime_t();
        }
        else
        if(strSection == "#DIESIZEX")
        {
            //# Die Size X = 2554.0
            m_fDieSizeX = strString.section("=",1).remove(" ").toFloat();
        }
        else
        if(strSection == "#DIESIZEY")
        {
            //# Die Size Y = 2554.0
            m_fDieSizeY = strString.section("=",1).remove(" ").toFloat();

            break;
        }
    }

    if(!WriteStdfFile(&hLaurierDieSortFile,strFileNameSTDF))
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
// Create STDF file from LaurierDieSort data parsed
//////////////////////////////////////////////////////////////////////
bool CGLaurierDieSorttoSTDF::WriteStdfFile(QTextStream *hLaurierDieSortFile, const char *strFileNameSTDF)
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
    StdfFile.WriteString("");					// Lot ID
    StdfFile.WriteString(m_strDeviceId.toLatin1().constData());	// Part Type / Product ID
    StdfFile.WriteString("");					// Node name
    StdfFile.WriteString("");					// Tester Type
    StdfFile.WriteString("");					// Job name
    StdfFile.WriteString(m_strJobRev.toLatin1().constData());	// Job rev
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
    strUserTxt += ":LAURIER_DIE_SORT";
    StdfFile.WriteString(strUserTxt.toLatin1().constData());	// user-txt
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
  //FIXME: not used ?
  //long iTotalGoodBin;
  //long iTotalFailBin;
    long		iPartNumber;

    int			iPosX, iPosY;
    int			iHBin;
    QMap<int,int> mapBinCnt;

    // Reset counters
  //FIXME: not used ?
  //iTotalGoodBin=iTotalFailBin=0;
    iPartNumber=0;

    // Write WIR of new Wafer.
    RecordReadInfo.iRecordType = 2;
    RecordReadInfo.iRecordSubType = 10;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteByte(1);							// Test head
    StdfFile.WriteByte(255);						// Tester site (all)
    StdfFile.WriteDword(m_lStartTime);				// Start time
    StdfFile.WriteString("1");						// WaferID
    StdfFile.WriteRecord();

    // Write all Parameters read on this wafer.: WIR.PIR,PRR, PIR,PRR,   ... WRR
    while(hLaurierDieSortFile->atEnd() == false)
    {

        // Read line
        strString = ReadLine(*hLaurierDieSortFile);
        if(strString.isEmpty())
            break;	// Reach the end of valid data records...

        iPosX = strString.left(11).remove(" ").toInt();
        iPosY = strString.mid(11,11).remove(" ").toInt();
        iHBin = strString.mid(22,11).remove(" ").toInt();
        if(!mapBinCnt.contains(iHBin))
            mapBinCnt[iHBin] = 0;
        mapBinCnt[iHBin]++;

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
        if(iHBin == 1)
            StdfFile.WriteByte(0);				// PART_FLG : PASSED
        else
            StdfFile.WriteByte(8);		// PART_FLG : FAILED

        StdfFile.WriteWord((WORD)0);			// NUM_TEST
        StdfFile.WriteWord((WORD)iHBin);		// HARD_BIN
        StdfFile.WriteWord((WORD)iHBin);        // SOFT_BIN
        StdfFile.WriteWord(iPosX);				// X_COORD
        StdfFile.WriteWord(iPosY);				// Y_COORD
        StdfFile.WriteDword(0);					// No testing time known...
        StdfFile.WriteString(QString::number(iPartNumber).toLatin1().constData());		// PART_ID
        StdfFile.WriteString("");				// PART_TXT
        StdfFile.WriteString("");				// PART_FIX
        StdfFile.WriteRecord();
    };			// Read all lines with valid data records in file

    // Write WRR for last wafer inserted
    RecordReadInfo.iRecordType = 2;
    RecordReadInfo.iRecordSubType = 20;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteByte(1);						// Test head
    StdfFile.WriteByte(255);					// Tester site (all)
    StdfFile.WriteDword(0);						// Time of last part tested
    StdfFile.WriteDword(iPartNumber);			// Parts tested
    StdfFile.WriteDword(0);						// Parts retested
    StdfFile.WriteDword(0);						// Parts Aborted
    StdfFile.WriteDword(mapBinCnt[1]);			// Good Parts
    StdfFile.WriteDword(4294967295UL);			// Functionnal Parts
    StdfFile.WriteString("1");					// WaferID
    StdfFile.WriteRecord();

    // Write WCR
    RecordReadInfo.iRecordType = 2;
    RecordReadInfo.iRecordSubType = 30;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteFloat(0);					// WAFR_SIZ
    StdfFile.WriteFloat(m_fDieSizeX);		// DIE_HT
    StdfFile.WriteFloat(m_fDieSizeY);		// DIE_WID
    StdfFile.WriteRecord();

    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 40;
    QMap<int,int>::Iterator itMapBin;
    for ( itMapBin = mapBinCnt.begin(); itMapBin != mapBinCnt.end(); ++itMapBin )
    {
        iHBin = itMapBin.key();
        // Write HBR
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte(255);						// Test Head = ALL
        StdfFile.WriteByte(255);						// Test sites = ALL
        StdfFile.WriteWord(iHBin);						// HBIN = 0
        StdfFile.WriteDword(mapBinCnt[iHBin]);			// Total Bins
        if(iHBin == 1)
            StdfFile.WriteByte('P');
        else
            StdfFile.WriteByte('F');
        StdfFile.WriteString("");
        StdfFile.WriteRecord();
    }

    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 50;

    for ( itMapBin = mapBinCnt.begin(); itMapBin != mapBinCnt.end(); ++itMapBin )
    {
        iHBin = itMapBin.key();
        // Write SBR
        StdfFile.WriteHeader(&RecordReadInfo);
        StdfFile.WriteByte(255);						// Test Head = ALL
        StdfFile.WriteByte(255);						// Test sites = ALL
        StdfFile.WriteWord(iHBin);						// SBIN = 0
        StdfFile.WriteDword(mapBinCnt[iHBin]);			// Total Bins
        if(iHBin == 1)
            StdfFile.WriteByte('P');
        else
            StdfFile.WriteByte('F');
        StdfFile.WriteString("");
        StdfFile.WriteRecord();
    }

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
// Convert 'FileName' LaurierDieSort file, to STDF 'strFileNameSTDF' file
//////////////////////////////////////////////////////////////////////
bool CGLaurierDieSorttoSTDF::Convert(const char *LaurierDieSortFileName, const char *strFileNameSTDF)
{
    // No erro (default)
    m_iLastError = errNoError;

    // If STDF file already exists...do not rebuild it...unless dates not matching!
    QFileInfo fInput(LaurierDieSortFileName);
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
            GS::Gex::Engine::GetInstance().UpdateLabelStatus("Converting data from file "+QFileInfo(LaurierDieSortFileName).fileName()+"...");
            GexScriptStatusLabel->show();
        }
    }
    QCoreApplication::processEvents();

    if(ReadLaurierDieSortFile(LaurierDieSortFileName,strFileNameSTDF) != true)
    {
        //////////////////////////////////////////////////////////////////////
        // For ProgressBar
        if((GexProgressBar != NULL)
        && bHideProgressAfter)
            GexProgressBar->hide();

        if((GexScriptStatusLabel != NULL)
        && bHideLabelAfter)
            GexScriptStatusLabel->hide();
        return false;	// Error reading LaurierDieSort file
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
QString CGLaurierDieSorttoSTDF::ReadLine(QTextStream& hFile)
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
