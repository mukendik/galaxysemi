//////////////////////////////////////////////////////////////////////
// import_pcm_cypress.cpp: Convert a PcmCypress file to STDF V4.0
//////////////////////////////////////////////////////////////////////

#include <math.h>
#include <qmath.h>
#include <time.h>

#ifdef _WIN32
#include "windows.h"		// For 'GetWindowsDirectoryA' declaration
#endif

#include <qfileinfo.h>
#include <qmap.h>
#include <qprogressbar.h>
#include <qapplication.h>
#include <qlabel.h>
#include <QDir>

#include "engine.h"
#include "import_pcm_cypress.h"
#include "import_constants.h"
#include "gex_algorithms.h"

// File format:
//lot,date,process,device,wafer,site,modfile,probecard,program,ilfpsm,ilbpsm,vtlps,vtsps,gmmpl,vtxpl,idlpl,idspl,vtlpl,
//9327003,8/19/2013 0:00,L65155C1,7C1173K,1,1,-99,-99,-99,2.254199982,0.085699998,0.476999998,0.335000008,3.117E-06,0.3
//9327003,8/19/2013 0:00,L65155C1,7C1173K,1,2,-99,-99,-99,1.211400032,0.047600001,0.488000005,0.352999985,3.113E-06,0.3
//9327003,8/19/2013 0:00,L65155C1,7C1173K,1,3,-99,-99,-99,1.584499955,0.0704,0.481000006,0.344000012,3.112E-06,0.376599
//...
//9430017,9/21/2014 0:00,L65155C2,7C1553K,1,1,-99,-99,-99,1.510499954,0.071000002,0.483999997,0.344000012,3.1401E-06,0.
//9430017,9/21/2014 0:00,L65155C2,7C1553K,1,2,-99,-99,-99,1.988299966,0.134000003,0.476000011,0.331,3.1401E-06,0.371100




// main.cpp
extern QLabel           *GexScriptStatusLabel;  // Handle to script status text in status bar
extern QProgressBar     *GexProgressBar;        // Handle to progress bar in status bar

#define TEST_PASSFLAG_UNKNOWN 0
#define TEST_PASSFLAG_PASS 1
#define TEST_PASSFLAG_FAIL 2


#define BIT0            0x01
#define BIT1            0x02
#define BIT2            0x04
#define BIT3            0x08
#define BIT4            0x10
#define BIT5            0x20
#define BIT6            0x40
#define BIT7            0x80

CGPcmCypressParameter::CGPcmCypressParameter()
{
    mNumber = -1;
};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGPcmCypresstoSTDF::CGPcmCypresstoSTDF()
{
    // Default: PcmCypress parameter list on disk includes all known PcmCypress parameters...
    mNewParameterFound = false;
    mStartTime = 0;
    mParameterList = NULL;
    mFileLine = 0;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGPcmCypresstoSTDF::~CGPcmCypresstoSTDF()
{
    if(mParameterList)
        delete [] mParameterList;
}

//////////////////////////////////////////////////////////////////////
// Get Error
//////////////////////////////////////////////////////////////////////
QString CGPcmCypresstoSTDF::GetLastError()
{
    mLastError = "Import Cypress - E-test data : ";

    switch(mLastErrorCode)
    {
        default:
        case errNoError:
            mLastError += "No Error";
            break;
        case errOpenFail:
            mLastError += "Failed to open file";
            break;
        case errInvalidFormat:
            mLastError += "Invalid file format";
            break;
        case errWriteSTDF:
            mLastError += "Failed creating temporary file. Folder permission issue?";
            break;
        case errLicenceExpired:
            mLastError += "License has expired or Data file out of date...";
            break;
    }
    if(mFileLine > 0)
        mLastError += " at line " + QString::number(mFileLine);

    // Return Error Message
    return mLastError;
}

//////////////////////////////////////////////////////////////////////
// Load PcmCypress Parameter table from DISK
//////////////////////////////////////////////////////////////////////
void CGPcmCypresstoSTDF::LoadParameterIndexTable(void)
{
    QString	lPcmCypressTableFile;
    QString	lString;

    lPcmCypressTableFile  = GS::Gex::Engine::GetInstance().Get("UserFolder").toString();
    lPcmCypressTableFile += GEX_PCM_PARAMETERS;

    // Open PcmCypress Parameter table file
    QFile f( lPcmCypressTableFile );
    if(!f.open( QIODevice::ReadOnly ))
        return;

    // Assign file I/O stream
    QTextStream hPcmCypressTableFile(&f);

    // Skip comment or empty lines
    do
    {
      lString = hPcmCypressTableFile.readLine();
    }
    while((lString.indexOf("----------------------") < 0) && (!hPcmCypressTableFile.atEnd()));

    // Read lines
    mFullParametersList.clear();
    lString = hPcmCypressTableFile.readLine();
    while (lString.isNull() == false)
    {
        // Save Parameter name in list
        mFullParametersList.append(lString);
        // Read next line
        lString = hPcmCypressTableFile.readLine();
    };

    // Close file
    f.close();
}

//////////////////////////////////////////////////////////////////////
// Save PcmCypress Parameter table to DISK
//////////////////////////////////////////////////////////////////////
void CGPcmCypresstoSTDF::DumpParameterIndexTable(void)
{
    QString	lPcmCypressTableFile;

    lPcmCypressTableFile  = GS::Gex::Engine::GetInstance().Get("UserFolder").toString();
    lPcmCypressTableFile += GEX_PCM_PARAMETERS;

    // Open PcmCypress Parameter table file
    QFile f( lPcmCypressTableFile );
    if(!f.open( QIODevice::WriteOnly ))
        return;

    // Assign file I/O stream
    QTextStream hPcmCypressTableFile(&f);

    // First few lines are comments:
    hPcmCypressTableFile << "############################################################" << endl;
    hPcmCypressTableFile << "# DO NOT EDIT THIS FILE!" << endl;
    hPcmCypressTableFile << "# Quantix Examinator: PCM Parameters detected" << endl;
    hPcmCypressTableFile << "# www.mentor.com" << endl;
    hPcmCypressTableFile << "# Quantix Examinator reads and writes into this file..." << endl;
    hPcmCypressTableFile << "-----------------------------------------------------------" << endl;

    // Write lines
    // m_pFullPcmCypressParametersList.sort();
    for(int nIndex = 0; nIndex < mFullParametersList.count(); nIndex++)
    {
        // Write line
        hPcmCypressTableFile << mFullParametersList[nIndex] << endl;
    };

    // Close file
    f.close();
}

//////////////////////////////////////////////////////////////////////
// If Examinator doesn't have this PcmCypress parameter in his dictionnary, have it added.
//////////////////////////////////////////////////////////////////////
int CGPcmCypresstoSTDF::UpdateParameterIndexTable(QString strParamName)
{
    int iTestNumber;

    // Check if the table is empty...if so, load it from disk first!
    if(mFullParametersList.isEmpty() == true)
    {
        // Load PcmCypress parameter table from disk...
        LoadParameterIndexTable();
    }

    // Check if Parameter name already in table...if not, add it to the list
    // the new full list will be dumped to the disk at the end.

    iTestNumber = mFullParametersList.indexOf(strParamName);
    if(iTestNumber < 0)
    {
        // Update list
        mFullParametersList.append(strParamName);
        iTestNumber = mFullParametersList.indexOf(strParamName);

        // Set flag to force the current PcmCypress table to be updated on disk
        mNewParameterFound = true;
    }

    return iTestNumber;
}

//////////////////////////////////////////////////////////////////////
// Check if File is compatible with PcmCypress format
//////////////////////////////////////////////////////////////////////
bool CGPcmCypresstoSTDF::IsCompatible(const char *szFileName)
{
    QString lString;

    // Open hCsmFile file
    QFile f( szFileName );
    if(!f.open( QIODevice::ReadOnly ))
    {
        // Failed Opening ASL1000 file
        return false;
    }
    QTextStream hPcmCypressFile(&f);
    QStringList lstSections;

    // Check if first line is the correct PcmCypress header...
    //lot,date,process,device,wafer,site,modfile,probecard,program,ilfpsm,ilbpsm,vtlps,vt
    do
        lString = hPcmCypressFile.readLine().simplified();
    while(!lString.isNull() && lString.isEmpty());

    if(	!lString.startsWith("lot,date,process,device,wafer,site,modfile,probecard,program", Qt::CaseInsensitive))
    {
        // Incorrect header...this is not a PcmCypress file!
        // Close file
        f.close();
        return false;
    }
    // Close file
    f.close();

    return true;
}

//////////////////////////////////////////////////////////////////////
// Read and Parse the PcmCypress file
//////////////////////////////////////////////////////////////////////
bool CGPcmCypresstoSTDF::ReadPcmCypressFile(const char *PcmCypressFileName, QStringList &lstFileNameSTDF)
{
    QString lString;
    QString lSection;

    // Open PcmCypress file
    QFile f( PcmCypressFileName );
    if(!f.open( QIODevice::ReadOnly ))
    {
        // Failed Opening PcmCypress file
        mLastErrorCode = errOpenFail;

        // Convertion failed.
        return false;
    }
    // Assign file I/O stream
    QTextStream hPcmCypressFile(&f);
    QStringList lLstSections;

    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    mFileSize = f.size() + 1;

    // Check if first line is the correct PcmCypress header...
    //lot,date,process,device,wafer,site,modfile,probecard,program,ilfpsm,ilbpsm,vtlps,vt
    //9327003,8/19/2013 0:00,L65155C1,7C1173K,1,1,-99,-99,-99,2.254199982,0.085699998,0.476999998,0.335000008,3.117E-0

    lString = ReadLine(hPcmCypressFile).simplified();
    if(	!lString.startsWith("lot,date,process,device,wafer,site,modfile,probecard,program", Qt::CaseInsensitive))
    {
        // Incorrect header...this is not a PcmCypress file!
        mLastErrorCode = errInvalidFormat;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }

    // Good header

    //lot,date,process,device,wafer,site,modfile,probecard,program,ilfpsm,ilbpsm,vtlps,vt
    //9327003,8/19/2013 0:00,L65155C1,7C1173K,1,1,-99,-99,-99,2.254199982,0.085699998,0.476999998,0.335000008,3.117E-0

    // Read PcmCypress information
    lLstSections = lString.split(",",QString::KeepEmptyParts);
    mParametersOffset = 9;

    // Count the number of parameters specified in the line
    // Do not count first 4 fields
    mTotalParameters=lLstSections.count() - mParametersOffset;
    // If no parameter specified...ignore!
    if(mTotalParameters <= 0)
    {
        // Incorrect header...this is not a valid Exar - PCM file!
        mLastErrorCode = errInvalidFormat;

        // Convertion failed.
        // Close file
        f.close();
        return false;
    }

    // Allocate the buffer to hold the N parameters & results.
    mParameterList = new CGPcmCypressParameter[mTotalParameters];	// List of parameters

    // Extract the N column names
    int iIndex;
    CGPcmCypressParameter *ptParam;
    for(iIndex=0;iIndex<mTotalParameters;iIndex++)
    {
        lSection = lLstSections[iIndex+mParametersOffset].trimmed();	// Remove spaces
        ptParam = &mParameterList[iIndex];
        ptParam->mNumber = UpdateParameterIndexTable(lSection);		// Update Parameter master list if needed.
        ptParam->mName = lSection;
    }

    if(!WriteStdfFile(&hPcmCypressFile,lstFileNameSTDF))
    {
        foreach(QString lFile,lstFileNameSTDF)
            QFile::remove(lFile);
        // Close file
        f.close();
        return false;
    }

    // Close file
    f.close();

    // All PcmCypress file read...check if need to update the PcmCypress Parameter list on disk?
    if(mNewParameterFound == true)
        DumpParameterIndexTable();

    // Success parsing PcmCypress file
    return true;
}

//////////////////////////////////////////////////////////////////////
// Create STDF file from PcmCypress data parsed
//////////////////////////////////////////////////////////////////////
bool CGPcmCypresstoSTDF::WriteStdfFile(QTextStream *hPcmCypressFile,QStringList &lstFileNameSTDF)
{
    // now generate the STDF file...
    QString     lLine, lValue, lFileNameSDTF;
    QDateTime   lDateTime;
    GS::StdLib::Stdf lStdfFile;
    GS::StdLib::StdfRecordReadInfo lRecordReadInfo;

    // Write Test results for each line read.
    int         iTotalTests,iPartNumber;
    BYTE        bData;
    int         iIndex;
    int         iSite;
    float       fValue;
    bool        bValue;
    QString     lWafer, lLot;

    //lot,date,process,device,wafer,site,modfile,probecard,program,ilfpsm,ilbpsm,vtlps,vtsps,gmmpl,vtxpl,idlpl,idspl
    //9327003,8/19/2013 0:00,L65155C1,7C1173K,1,1,-99,-99,-99,2.254199982,0.085699998,0.476999998,0.335000008,3.117E

    // Reset counters
    iTotalTests=0;
    iPartNumber=0;

    // Write all Parameters read on this file : WIR,PIR,PTR...,PRR,WRR WIR,PIR,PTR..., PRR
    // Read PcmCypress result
    do
    {
        lLine = ReadLine(*hPcmCypressFile).simplified();
        if(lLine.count(",") <= mParametersOffset)
        {
            // Incorrect header...this is not a valid Exar - PCM file!
            mLastErrorCode = errInvalidFormat;

            // Convertion failed.
            return false;
        }

        // Invalid line
        lDateTime = QDateTime::fromString(lLine.section(",",1,1).trimmed(),"M/d/yyyy H:mm");
        if(!lDateTime.isValid() || lDateTime.date().year()==9999)
            continue;

        iSite = lLine.section(",",5,5).trimmed().toInt();
        if(iSite == 99)
            continue;

        // Extract TimeStamp, convert to UINT (nbr of seconds since 1-1-1970)
        lDateTime.setTimeSpec(Qt::UTC);
        mStartTime = lDateTime.toTime_t();

        lLot = lLine.section(",",0,0).trimmed();

        if(mLotId!=lLot)
        {
            // New lot
            // Generate a new STDF file
            // Close the last
            if(!mWaferId.isEmpty())
            {
                // Write WRR for last wafer inserted
                lRecordReadInfo.iRecordType = 2;
                lRecordReadInfo.iRecordSubType = 20;
                lStdfFile.WriteHeader(&lRecordReadInfo);
                lStdfFile.WriteByte(1);                     // Test head
                lStdfFile.WriteByte(255);                   // Tester site (all)
                lStdfFile.WriteDword(mStartTime);           // Time of last part tested
                lStdfFile.WriteDword(iPartNumber);          // Parts tested
                lStdfFile.WriteDword(0);                    // Parts retested
                lStdfFile.WriteDword(0);                    // Parts Aborted
                lStdfFile.WriteDword((DWORD)-1);            // Good Parts
                lStdfFile.WriteDword((DWORD)-1);            // Functionnal Parts
                lStdfFile.WriteString(mWaferId.toLatin1().constData());	// WaferID
                lStdfFile.WriteString("");                  // FabId
                lStdfFile.WriteString("");                  // FrameId
                lStdfFile.WriteString("");                  // MaskId
                lStdfFile.WriteString("");                  // UserDesc
                lStdfFile.WriteRecord();

            }

            if(!mLotId.isEmpty())
            {
                // Write MRR
                lRecordReadInfo.iRecordType = 1;
                lRecordReadInfo.iRecordSubType = 20;
                lStdfFile.WriteHeader(&lRecordReadInfo);
                lStdfFile.WriteDword(mStartTime);         // File finish-time.
                lStdfFile.WriteRecord();

            }

            if(!lFileNameSDTF.isEmpty())
                // Close STDF file.
                lStdfFile.Close();

            mLotId = "";
        }

        if(mLotId.isEmpty())
        {
            mLotId = lLot;
            lFileNameSDTF = mDataFilePath+QDir::separator();
            lFileNameSDTF +=QFileInfo(mStdfFileName).baseName()+"_"+mLotId;
            lFileNameSDTF += "."+QFileInfo(mStdfFileName).completeSuffix();
            QFile::remove(lFileNameSDTF);
            if(lStdfFile.Open((char*)lFileNameSDTF.toLatin1().constData(),STDF_WRITE) != GS::StdLib::Stdf::NoError)
            {
                // Failed importing PcmCypress file into STDF database
                mLastErrorCode = errWriteSTDF;

                // Convertion failed.
                return false;
            }

            lstFileNameSTDF << lFileNameSDTF;

            //lot,date,process,device,wafer,site,modfile,probecard,program,ilfpsm,ilbpsm,vtlps,vtsps,gmmpl,vtxpl,idlpl,idspl
            //9327003,8/19/2013 0:00,L65155C1,7C1173K,1,1,-99,-99,-99,2.254199982,0.085699998,0.476999998,0.335000008,3.117E
            mProcessId = lLine.section(",",2,2).trimmed();
            mProductId = lLine.section(",",3,3).trimmed();

            // Write FAR
            lRecordReadInfo.iRecordType = 0;
            lRecordReadInfo.iRecordSubType = 10;
            lStdfFile.WriteHeader(&lRecordReadInfo);
            lStdfFile.WriteByte(1);                 // SUN CPU type
            lStdfFile.WriteByte(4);                 // STDF V4
            lStdfFile.WriteRecord();

            if(mStartTime <= 0)
                mStartTime = QDateTime::currentDateTime().toTime_t();

            // Write MIR
            lRecordReadInfo.iRecordType = 1;
            lRecordReadInfo.iRecordSubType = 10;
            lStdfFile.WriteHeader(&lRecordReadInfo);
            lStdfFile.WriteDword(mStartTime);         // Setup time
            lStdfFile.WriteDword(mStartTime);         // Start time
            lStdfFile.WriteByte(1);                     // Station
            lStdfFile.WriteByte((BYTE) 'P');            // Test Mode = PRODUCTION
            lStdfFile.WriteByte((BYTE) ' ');            // rtst_cod
            lStdfFile.WriteByte((BYTE) ' ');            // prot_cod
            lStdfFile.WriteWord(65535);                 // burn_tim
            lStdfFile.WriteByte((BYTE) ' ');            // cmod_cod
            lStdfFile.WriteString(mLotId.toLatin1().constData());       // Lot ID
            lStdfFile.WriteString(mProductId.toLatin1().constData());   // Part Type / Product ID
            lStdfFile.WriteString("");                  // Node name
            lStdfFile.WriteString("Pcm Cypress");       // Tester Type
            lStdfFile.WriteString("");                  // Job name
            lStdfFile.WriteString("");                  // Job rev
            lStdfFile.WriteString("");                  // sublot-id
            lStdfFile.WriteString("");                  // operator
            lStdfFile.WriteString("");                  // exec-type
            lStdfFile.WriteString("");                  // exe-ver
            lStdfFile.WriteString("WAFER");             // test-cod
            lStdfFile.WriteString("");                  // test-temperature
            // Construct custom Galaxy USER_TXT
            QString	strUserTxt;
            strUserTxt = GEX_IMPORT_DATAORIGIN_LABEL;
            strUserTxt += ":";
            strUserTxt += GEX_IMPORT_DATAORIGIN_ETEST;
            strUserTxt += ":PCM_CYPRESS";
            lStdfFile.WriteString((char *)strUserTxt.toLatin1().constData());   // user-txt
            lStdfFile.WriteString("");                  // aux-file
            lStdfFile.WriteString("");                  // package-type
            lStdfFile.WriteString("");                  // familyID
            lStdfFile.WriteString("");                  // Date-code
            lStdfFile.WriteString("");                  // Facility-ID
            lStdfFile.WriteString("");                  // FloorID
            lStdfFile.WriteString(mProcessId.toLatin1().constData());   // ProcessID

            lStdfFile.WriteRecord();

            // Reset counters
            iTotalTests=0;
            iPartNumber=0;

            mWaferId = "";
        }

        lWafer = lLine.section(",",4,4).trimmed();

        if(mWaferId != lWafer)
        {
            // New wafer
            // Close the last
            if(!mWaferId.isEmpty())
            {
                // Write WRR for last wafer inserted
                lRecordReadInfo.iRecordType = 2;
                lRecordReadInfo.iRecordSubType = 20;
                lStdfFile.WriteHeader(&lRecordReadInfo);
                lStdfFile.WriteByte(1);                     // Test head
                lStdfFile.WriteByte(255);                   // Tester site (all)
                lStdfFile.WriteDword(mStartTime);           // Time of last part tested
                lStdfFile.WriteDword(iPartNumber);          // Parts tested
                lStdfFile.WriteDword(0);                    // Parts retested
                lStdfFile.WriteDword(0);                    // Parts Aborted
                lStdfFile.WriteDword((DWORD)-1);            // Good Parts
                lStdfFile.WriteDword((DWORD)-1);            // Functionnal Parts
                lStdfFile.WriteString(mWaferId.toLatin1().constData());	// WaferID
                lStdfFile.WriteString("");                  // FabId
                lStdfFile.WriteString("");                  // FrameId
                lStdfFile.WriteString("");                  // MaskId
                lStdfFile.WriteString("");                  // UserDesc
                lStdfFile.WriteRecord();

            }

            mWaferId = lWafer;

            if(!mWaferId.isEmpty())
            {
                // Write WIR of new Wafer.
                lRecordReadInfo.iRecordType = 2;
                lRecordReadInfo.iRecordSubType = 10;
                lStdfFile.WriteHeader(&lRecordReadInfo);
                lStdfFile.WriteByte(1);                         // Test head
                lStdfFile.WriteByte(255);                       // Tester site (all)
                lStdfFile.WriteDword(mStartTime);               // Start time
                lStdfFile.WriteString(mWaferId.toLatin1().constData());	// WaferID
                lStdfFile.WriteRecord();

                // For each wafer, have to write limit in the first PTR
                for(iIndex=0;iIndex<mTotalParameters;iIndex++)
                    mParameterList[iIndex].mStaticHeaderWritten = false;
            }

            iTotalTests = 0;
            iPartNumber = 0;
        }

        iPartNumber++;

        // Write PIR
        // Write PIR for parts in this Wafer site
        lRecordReadInfo.iRecordType = 5;
        lRecordReadInfo.iRecordSubType = 10;
        lStdfFile.WriteHeader(&lRecordReadInfo);
        lStdfFile.WriteByte(1);                     // Test head
        lStdfFile.WriteByte(iSite);                 // Tester site
        lStdfFile.WriteRecord();

        // Reset counters
        iTotalTests = 0;

        lLine = lLine.trimmed().remove(" ");

        for(iIndex=0;iIndex<mTotalParameters;iIndex++)
        {
            bValue = false;
            fValue = 0.0;
            lValue = lLine.section(",",iIndex+mParametersOffset,iIndex+mParametersOffset);
            if(lValue != "-99")
                fValue = lValue.toFloat(&bValue);

            // Check if have valid value
            if(!bValue)
                continue;

            iTotalTests++;
            // Compute Test# (add user-defined offset)
            // Write PTR
            lRecordReadInfo.iRecordType = 15;
            lRecordReadInfo.iRecordSubType = 10;

            lStdfFile.WriteHeader(&lRecordReadInfo);
            lStdfFile.WriteDword(mParameterList[iIndex].mNumber
                                + GEX_TESTNBR_OFFSET_PCM);      // Test Number
            lStdfFile.WriteByte(1);                             // Test head
            lStdfFile.WriteByte(iSite);                         // Tester site:1,2,3,4 or 5, etc.
            bData = BIT6;	// Test completed with no pass/fail indication
            lStdfFile.WriteByte(bData);                          // TEST_FLG
            bData = BIT6|BIT7;
            lStdfFile.WriteByte(bData);                          // PARAM_FLG
            lStdfFile.WriteFloat(fValue);                        // Test result
            if(!mParameterList[iIndex].mStaticHeaderWritten)
            {
                mParameterList[iIndex].mStaticHeaderWritten = true;

                // save Parameter name
                lStdfFile.WriteString(mParameterList[iIndex].mName
                                     .toLatin1().constData());  // TEST_TXT
            }
            lStdfFile.WriteRecord();
        }

        // Write PRR
        lRecordReadInfo.iRecordType = 5;
        lRecordReadInfo.iRecordSubType = 20;
        lStdfFile.WriteHeader(&lRecordReadInfo);
        lStdfFile.WriteByte(1);                  // Test head
        lStdfFile.WriteByte(iSite);              // Tester site:1,2,3,4 or 5
        bData = BIT4; // Device completed testing with no pass/fail indication
        lStdfFile.WriteByte(bData);              // PART_FLG
        lStdfFile.WriteWord((WORD)iTotalTests);  // NUM_TEST
        lStdfFile.WriteWord(1);                  // HARD_BIN
        lStdfFile.WriteWord(1);                  // SOFT_BIN
        switch(iSite)
        {
                    case 1:	// Center
                        lStdfFile.WriteWord(1);          // X_COORD
                        lStdfFile.WriteWord(1);          // Y_COORD
                        break;
                    case 2:	// Bottom
                        lStdfFile.WriteWord(1);          // X_COORD
                        lStdfFile.WriteWord(2);          // Y_COORD
                        break;
                    case 3:	// Right
                        lStdfFile.WriteWord(0);          // X_COORD
                        lStdfFile.WriteWord(1);          // Y_COORD
                        break;
                    case 4:	// Top
                        lStdfFile.WriteWord(1);          // X_COORD
                        lStdfFile.WriteWord(0);          // Y_COORD
                        break;
                    case 5:	// Left
                        lStdfFile.WriteWord(2);          // X_COORD
                        lStdfFile.WriteWord(1);          // Y_COORD
                        break;
                    case 6:	// Lower-Right corner
                        lStdfFile.WriteWord(2);          // X_COORD
                        lStdfFile.WriteWord(2);          // Y_COORD
                        break;
                    case 7:	// Lower left corner
                        lStdfFile.WriteWord(0);          // X_COORD
                        lStdfFile.WriteWord(2);          // Y_COORD
                        break;
                    case 8:	// toUpper-Left corner
                        lStdfFile.WriteWord(0);          // X_COORD
                        lStdfFile.WriteWord(0);          // Y_COORD
                        break;
                    case 9:	// toUpper-Right corner
                        lStdfFile.WriteWord(2);          // X_COORD
                        lStdfFile.WriteWord(0);          // Y_COORD
                        break;
                    default: // More than 9 sites?....give 0,0 coordonates
                        lStdfFile.WriteWord(0);          // X_COORD
                        lStdfFile.WriteWord(0);          // Y_COORD
                        break;
        }
        lStdfFile.WriteDword(0);                 // No testing time known...
        lStdfFile.WriteString(QString::number(iPartNumber).toLatin1().constData());// PART_ID
        lStdfFile.WriteString("");               // PART_TXT
        lStdfFile.WriteString("");               // PART_FIX
        lStdfFile.WriteRecord();
    }
    while(!hPcmCypressFile->atEnd());

    // Close the last
    if(!mWaferId.isEmpty())
    {
        // Write WRR for last wafer inserted
        lRecordReadInfo.iRecordType = 2;
        lRecordReadInfo.iRecordSubType = 20;
        lStdfFile.WriteHeader(&lRecordReadInfo);
        lStdfFile.WriteByte(1);                      // Test head
        lStdfFile.WriteByte(255);                    // Tester site (all)
        lStdfFile.WriteDword(mStartTime);            // Time of last part tested
        lStdfFile.WriteDword(iPartNumber);           // Parts tested
        lStdfFile.WriteDword(0);                     // Parts retested
        lStdfFile.WriteDword(0);                     // Parts Aborted
        lStdfFile.WriteDword((DWORD)-1);             // Good Parts
        lStdfFile.WriteDword((DWORD)-1);             // Functionnal Parts
        lStdfFile.WriteString(mWaferId.toLatin1().constData());	// WaferID
        lStdfFile.WriteString("");                   // FabId
        lStdfFile.WriteString("");                   // FrameId
        lStdfFile.WriteString("");                   // MaskId
        lStdfFile.WriteString("");                   // UserDesc
        lStdfFile.WriteRecord();

    }

    // Write MRR
    lRecordReadInfo.iRecordType = 1;
    lRecordReadInfo.iRecordSubType = 20;
    lStdfFile.WriteHeader(&lRecordReadInfo);
    lStdfFile.WriteDword(mStartTime);          // File finish-time.
    lStdfFile.WriteRecord();

    // Close STDF file.
    lStdfFile.Close();

    // Success
    return true;
}

//////////////////////////////////////////////////////////////////////
// Convert 'FileName' PcmCypress file, to STDF 'strFileNameSTDF' file
//////////////////////////////////////////////////////////////////////
bool CGPcmCypresstoSTDF::Convert(const char *PcmCypressFileName, QStringList &lstFileNameSTDF)
{
    // No erro (default)
    mLastErrorCode = errNoError;

    // If STDF file already exists...do not rebuild it...unless dates not matching!
    QFileInfo fInput(PcmCypressFileName);
    QFileInfo fOutput(lstFileNameSTDF.first());

    QFile f( lstFileNameSTDF.first() );
    if((f.exists() == true) && (fInput.lastModified() < fOutput.lastModified()))
        return true;

    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    mProgressStep = 0;
    mNextFilePos = 0;

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
            GS::Gex::Engine::GetInstance().UpdateLabelStatus("Converting data from file "+QFileInfo(PcmCypressFileName).fileName()+"...");
            GexScriptStatusLabel->show();
        }
        GS::Gex::Engine::GetInstance().UpdateLabelStatus("Converting data from file "+QFileInfo(PcmCypressFileName).fileName()+"...");
        GexScriptStatusLabel->show();
    }
    QCoreApplication::processEvents();

    mStdfFileName = lstFileNameSTDF.first();
    mDataFilePath = QFileInfo(mStdfFileName).absolutePath();
    if(mDataFilePath.isEmpty())
        mDataFilePath = QFileInfo(PcmCypressFileName).absolutePath();

    lstFileNameSTDF.clear();
    if(ReadPcmCypressFile(PcmCypressFileName,lstFileNameSTDF) != true)
    {
        //////////////////////////////////////////////////////////////////////
        // For ProgressBar
        if((GexProgressBar != NULL)
        && bHideProgressAfter)
            GexProgressBar->hide();

        if((GexScriptStatusLabel != NULL)
        && bHideLabelAfter)
            GexScriptStatusLabel->hide();
        return false;	// Error reading PcmCypress file
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
QString CGPcmCypresstoSTDF::ReadLine(QTextStream& hFile)
{
    QString lLine;

    //////////////////////////////////////////////////////////////////////
    // For ProgressBar
    if(GexProgressBar != NULL)
    {
        while((int) hFile.device()->pos() > mNextFilePos)
        {
            mProgressStep += 100/mFileSize + 1;
            mNextFilePos  += mFileSize/100 + 1;
            GexProgressBar->setValue(mProgressStep);
        }
    }
    QCoreApplication::processEvents();

    do
    {
        lLine = hFile.readLine();
        ++mFileLine;
    }
    while(!lLine.isNull() && lLine.isEmpty());

    return lLine;

}
