//////////////////////////////////////////////////////////////////////
// import_fresco_sum.cpp: Convert a FrescoSummary .csv
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
#include "import_fresco_sum.h"
#include "time.h"
#include "import_constants.h"
#include "engine.h"

// Fresco Summary Format
//Filename: /usr/local/home/prod/DATA_LOCAL/FRE-FRE1031015AA000_FT-N2U045.00A---FM1151A1-UBX-46537-UTTC510-20101024165652.sum
//
//(1) General information
//--------------------------
//Start Date/Time          :Sun Oct 24 05:47:40 2010
//Stop Date/Time           :Sun Oct 24 16:56:52 2010
//
//Tester Station           :uttc510
//Tester Type              :D10
//Program Name             :spyder_1151_srm_rev6
//Device Name              :FM1151A1-UBX
//Job NO.                  :-
//MES Lot ID               :FRE1031015AA000_FT
//Lot Nbr                  :N2U045.00A
//Operator                 :CO1678
//Test OS Version          :v1.5.3_BLD14
//Robot Type               :TTL Test
//
//(2) Bin information
//--------------------------
// Hard Bin   Site0   Site1       Total   Yield
//------------------------------------------------
//        0       0       0           0        0%
//        1   21718   20111       41829    90.09%
//        2     129    2399        2528    5.445%
//        3    1003     224        1227    2.643%
//        4      33      46          79   0.1702%
//        5     402     363         765    1.648%
//------------------------------------------------
//  Tested:   23285   23143
//  Passed:   21718   20111
//  Failed:    1567    3032
//   Yield:   93.27%   86.9%
//
//     BinName         SoftBin HardBin   Site0   Site1       Total   Yield | P/F
//------------------------------------------------------------------------------------------------
//       CSC_UNDEFINED       0       0       0       0           0        0% |FAIL
//            pass_bin       1       1   21718   20111       41829    90.09% |PASS
//      continuity_bin       2       2      96    2370        2466    5.311% |FAIL
//     power_short_bin       3       2      33      29          62   0.1335% |FAIL
//        scan_stk_bin       4       3     976     202        1178    2.537% |FAIL
//        scan_tdf_bin       5       3      26      22          48   0.1034% |FAIL
//          agcout_bin       6       3       1       0           1 0.002154% |FAIL
//             vin_bin       7       4       0       1           1 0.002154% |FAIL
//------------------------------------------------------------------------------------------------
//
//(3) Summary
//--------------------------
//Total PASS:        41829( 90.09% )
//Total FAIL:         4599( 9.906% )
//Total TESTED:      46428
//------------------------------------------------------------------------------------------------

// main.cpp
extern QLabel			*GexScriptStatusLabel;	// Handle to script status text in status bar
extern QProgressBar	*	GexProgressBar;		// Handle to progress bar in status bar


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CGFrescoSummarytoSTDF::CGFrescoSummarytoSTDF()
{
    // Default: FrescoSummary parameter list on disk includes all known FrescoSummary parameters...
    m_lStartTime = m_lStopTime = 0;
    m_nPassParts = m_nFailParts = m_nTotalParts = 0;
}

//////////////////////////////////////////////////////////////////////
// Destruction
//////////////////////////////////////////////////////////////////////
CGFrescoSummarytoSTDF::~CGFrescoSummarytoSTDF()
{
}


//////////////////////////////////////////////////////////////////////
// Get Error
//////////////////////////////////////////////////////////////////////
QString CGFrescoSummarytoSTDF::GetLastError()
{
    strLastError = "Import FrescoSummary: ";

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
// Check if File is compatible with FrescoSummary format
//////////////////////////////////////////////////////////////////////
bool CGFrescoSummarytoSTDF::IsCompatible(const char *szFileName)
{
    QString strString;
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
    QTextStream hFrescoSummaryFile(&f);

    //Filename: /usr/local/home/prod/DATA_LOCAL/FRE-FRE1031015AA000_FT-N2U045.00A---FM1151A1-UBX-46537-UTTC510-20101024165652.sum
    //
    //(1) General information

    while(!hFrescoSummaryFile.atEnd())
    {
        strString = hFrescoSummaryFile.readLine().remove(" ").toUpper();
        if(strString.isEmpty())
            continue;
        else if(strString.startsWith("FILENAME:"))
            continue;
        else if(strString.startsWith("(1)GENERALINFORMATION"))
        {
            f.close();
            return true;
        }
        else
            break;
    }

    // Incorrect header...this is not a FrescoSummary file!
    f.close();
    return false;
}

//////////////////////////////////////////////////////////////////////
// Read and Parse the FrescoSummary file
//////////////////////////////////////////////////////////////////////
bool CGFrescoSummarytoSTDF::ReadFrescoSummaryFile(const char *FrescoSummaryFileName,const char *strFileNameSTDF)
{
    QString strString;
    QString strSection;
    bool	bStatus;

    // Open CSV file
    QFile f( FrescoSummaryFileName );
    if(!f.open( QIODevice::ReadOnly ))
    {
        // Failed Opening FrescoSummary file
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
    QTextStream hFrescoSummaryFile(&f);
    QDate	clDate;
    QTime	clTime;

    while(!hFrescoSummaryFile.atEnd())
    {
        strString = hFrescoSummaryFile.readLine().remove(" ").toUpper();
        if(strString.isEmpty())
            continue;
        else if(strString.startsWith("FILENAME:"))
            continue;
        else if(strString == "(1)GENERALINFORMATION")
        {
            break;
        }
        else
            break;
    }

    if(!strString.startsWith("(1)GENERALINFORMATION"))
    {
        // Incorrect file date (greater than license expiration date)...refuse to convert file!
        iLastError = errInvalidFormat;

        // Convertion failed.
        f.close();
        return false;
    }

    //Filename: /usr/local/home/prod/DATA_LOCAL/FRE-FRE1031015AA000_FT-N2U045.00A---FM1151A1-UBX-46537-UTTC510-20101024165652.sum
    //
    //(1) General information
    //--------------------------
    //Start Date/Time          :Sun Oct 24 05:47:40 2010
    //Stop Date/Time           :Sun Oct 24 16:56:52 2010
    //
    //Tester Station           :uttc510
    //Tester Type              :D10
    //Program Name             :spyder_1151_srm_rev6
    //Device Name              :FM1151A1-UBX
    //Job NO.                  :-
    //MES Lot ID               :FRE1031015AA000_FT
    //Lot Nbr                  :N2U045.00A
    //Operator                 :CO1678
    //Test OS Version          :v1.5.3_BLD14
    //Robot Type               :TTL Test
    //
    //(2) Bin information
    //--------------------------

    // Read FrescoSummary information
    while(!hFrescoSummaryFile.atEnd())
    {
        strString = ReadLine(hFrescoSummaryFile);
        strSection = strString.section(":",0,0).remove(' ').toUpper();
        strString = strString.section(":",1).simplified();

        if(strSection == "(2)BININFORMATION")
        {
            //(2) Bin information
            //--------------------------
            // Hard Bin   Site0   Site1       Total   Yield
            //------------------------------------------------
            //        0       0       0           0        0%
            //        1   21718   20111       41829    90.09%
            //        2     129    2399        2528    5.445%
            //        3    1003     224        1227    2.643%
            //        4      33      46          79   0.1702%
            //        5     402     363         765    1.648%
            //------------------------------------------------
            //  Tested:   23285   23143
            //  Passed:   21718   20111
            //  Failed:    1567    3032
            //   Yield:   93.27%   86.9%
            //
            //     BinName         SoftBin HardBin   Site0   Site1       Total   Yield | P/F
            //------------------------------------------------------------------------------------------------
            //       CSC_UNDEFINED       0       0       0       0           0        0% |FAIL
            //            pass_bin       1       1   21718   20111       41829    90.09% |PASS
            //      continuity_bin       2       2      96    2370        2466    5.311% |FAIL
            //     power_short_bin       3       2      33      29          62   0.1335% |FAIL
            //        scan_stk_bin       4       3     976     202        1178    2.537% |FAIL
            //        scan_tdf_bin       5       3      26      22          48   0.1034% |FAIL
            //          agcout_bin       6       3       1       0           1 0.002154% |FAIL
            //             vin_bin       7       4       0       1           1 0.002154% |FAIL
            //------------------------------------------------------------------------------------------------

            int		nBinNum;
            int		nSiteNum;
            int		nValue;
            bool	bIsValidNum;
            QString strBinName;

            strString = ReadLine(hFrescoSummaryFile).trimmed().toUpper();
            if(strString.startsWith("-----"))
                strString = ReadLine(hFrescoSummaryFile).trimmed().toUpper();

            if(!strString.startsWith("HARD"))
            {
                // Incorrect data
                iLastError = errInvalidFormat;

                // Convertion failed.
                f.close();
                return false;
            }

            int nSiteCount;
            int	nIndex;
            QMap<int,int> mapSiteNumber;

            nSiteCount = strString.count("SITE");
            for(nIndex=1; nIndex<=nSiteCount; nIndex++)
            {
                strString = strString.section("SITE",1);
                nSiteNum = strString.section(" ",0,0).toInt(&bIsValidNum);
                if(!bIsValidNum)
                {
                    // Incorrect data
                    iLastError = errInvalidFormat;

                    // Convertion failed.
                    f.close();
                    return false;
                }
                mapSiteNumber[nIndex] = nSiteNum;
            }

            strString = ReadLine(hFrescoSummaryFile).simplified();
            if(strString.startsWith("-----"))
                strString = ReadLine(hFrescoSummaryFile).simplified();

            while(!strString.startsWith("-----"))
            {

                nBinNum  = strString.section(" ",0,0).toInt(&bIsValidNum);
                if(!bIsValidNum)
                {
                    // Incorrect data
                    iLastError = errInvalidFormat;

                    // Convertion failed.
                    f.close();
                    return false;
                }
                m_mapFrescoSummaryHardBinning[nBinNum].nNumber = nBinNum;
                // For each site
                for(nIndex=1; nIndex<=nSiteCount; nIndex++)
                {
                    nValue = strString.section(" ",nIndex,nIndex).toInt(&bIsValidNum);
                    m_mapFrescoSummaryHardBinning[nBinNum].mapCount[mapSiteNumber[nIndex]] = nValue;
                }
                // For all sites
                nValue = strString.section(" ",nIndex,nIndex).toInt(&bIsValidNum);
                m_mapFrescoSummaryHardBinning[nBinNum].mapCount[255] = nValue;

                strString = ReadLine(hFrescoSummaryFile).simplified();
            }

            strString = ReadLine(hFrescoSummaryFile).simplified();

            // Goto SoftBin
            while(!strString.startsWith("-----"))
                strString = ReadLine(hFrescoSummaryFile).simplified();

            strString = ReadLine(hFrescoSummaryFile).simplified();
            while(!strString.startsWith("-----"))
            {
                strBinName = strString.section(" ",0,0);
                nBinNum  = strString.section(" ",1,1).toInt(&bIsValidNum);
                strString = strString.section(" ",2);

                m_mapFrescoSummarySoftBinning[nBinNum].strName = strBinName;
                m_mapFrescoSummarySoftBinning[nBinNum].nNumber = nBinNum;
                // For each site
                for(nIndex=1; nIndex<=nSiteCount; nIndex++)
                {
                    nValue = strString.section(" ",nIndex,nIndex).toInt(&bIsValidNum);
                    m_mapFrescoSummarySoftBinning[nBinNum].mapCount[mapSiteNumber[nIndex]] = nValue;
                }
                // For all sites
                nValue = strString.section(" ",nIndex,nIndex).toInt(&bIsValidNum);
                m_mapFrescoSummarySoftBinning[nBinNum].mapCount[255] = nValue;
                // PassFlag
                m_mapFrescoSummarySoftBinning[nBinNum].bPass = (strString.section("|",1).simplified().toUpper() == "PASS");
                m_mapFrescoSummaryHardBinning[strString.section(" ",0,0).toInt()].bPass = (strString.section("|",1).simplified().toUpper() == "PASS");

                strString = ReadLine(hFrescoSummaryFile).simplified();
            }

        }
        else if(strSection == "(3)SUMMARY")
        {
            //(3) Summary
            //--------------------------
            //Total PASS:        41829( 90.09% )
            //Total FAIL:         4599( 9.906% )
            //Total TESTED:      46428

            strString = ReadLine(hFrescoSummaryFile).trimmed().toUpper();
            if(strString.startsWith("-----"))
                strString = ReadLine(hFrescoSummaryFile).trimmed().toUpper();

            if(!strString.startsWith("TOTAL"))
            {
                // Incorrect data
                iLastError = errInvalidFormat;

                // Convertion failed.
                f.close();
                return false;
            }

            m_nPassParts = strString.section(":",1).section("(",0,0).simplified().toInt();
            strString = ReadLine(hFrescoSummaryFile).trimmed().toUpper();
            m_nFailParts = strString.section(":",1).section("(",0,0).simplified().toInt();
            strString = ReadLine(hFrescoSummaryFile).trimmed().toUpper();
            m_nTotalParts = strString.section(":",1).simplified().toInt();

        }
        else if(strSection == "STARTDATE/TIME")
        {
            //Start Date/Time          :Sun Oct 24 05:47:40 2010
            QDate clDate = QDate::fromString(strString.section(" ",0,2)+" "+strString.section(" ",4));
            QTime clTime = QTime::fromString(strString.section(" ",3,3));
            QDateTime clDateTime(clDate,clTime);
            clDateTime.setTimeSpec(Qt::UTC);
            m_lStartTime = clDateTime.toTime_t();
        }
        else if(strSection == "STOPDATE/TIME")
        {
            //Start Date/Time          :Sun Oct 24 05:47:40 2010
            QDate clDate = QDate::fromString(strString.section(" ",0,2)+" "+strString.section(" ",4));
            QTime clTime = QTime::fromString(strString.section(" ",3,3));
            QDateTime clDateTime(clDate,clTime);
            clDateTime.setTimeSpec(Qt::UTC);
            m_lStopTime = clDateTime.toTime_t();
        }
        else if(strSection == "MESLOTID")
        {
            //MES Lot ID               :FRE1031015AA000_FT
            m_strLotID = strString;
        }
        else if(strSection == "LOTNBR")
        {
            //Lot Nbr                  :N2U045.00A
            m_strSubLotID = strString;
        }
        else if(strSection == "TESTERSTATION")
        {
            //Tester Station           :uttc510
            m_strTesterID = strString.simplified();
            strString = "";
        }
        else if(strSection == "TESTERTYPE")
        {
            //Tester Type              :D10
            m_strTesterType = strString.simplified();
            strString = "";
        }
        else if(strSection == "PROGRAMNAME")
        {
            //Program Name             :spyder_1151_srm_rev6
            m_strJobName = strString.simplified();
            strString = "";
        }
        else if(strSection == "DEVICENAME")
        {
            //Device Name              :FM1151A1-UBX
            m_strProductID = strString.simplified();
            strString = "";
        }
        else if(strSection == "JOBNO.")
        {
            //Job NO.                  :-
            m_strJobRev = strString.simplified();
            strString = "";
        }
        else if(strSection == "OPERATOR")
        {
            //Operator                 :CO1678
            m_strOperatorID = strString.simplified();
            strString = "";
        }
        else if(strSection == "TESTOSVERSION")
        {
            //Test OS Version          :v1.5.3_BLD14
            m_strExecVer = strString.simplified();
            strString = "";
        }
        else if(strSection == "ROBOTTYPE")
        {
            //Robot Type               :TTL Test
            m_strExecType = strString.simplified();
            strString = "";
        }
    }

    // Generate STDF file dynamically.
    bStatus = WriteStdfFile(strFileNameSTDF);
    if(!bStatus)
        QFile::remove(strFileNameSTDF);

    // Success parsing FrescoSummary file
    f.close();
    return bStatus;
}

//////////////////////////////////////////////////////////////////////
// Create STDF file from FrescoSummary data parsed
//////////////////////////////////////////////////////////////////////
bool CGFrescoSummarytoSTDF::WriteStdfFile(const char *strFileNameSTDF)
{
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
    StdfFile.WriteString(m_strTesterID.toLatin1().constData());		// Node name
    StdfFile.WriteString(m_strTesterType.toLatin1().constData());	// Tester Type
    StdfFile.WriteString(m_strJobName.toLatin1().constData());		// Job name
    StdfFile.WriteString(m_strJobRev.toLatin1().constData());		// Job rev
    StdfFile.WriteString(m_strSubLotID.toLatin1().constData());		// sublot-id
    StdfFile.WriteString(m_strOperatorID.toLatin1().constData());	// operator
    StdfFile.WriteString(m_strExecType.toLatin1().constData());		// exec-type
    StdfFile.WriteString(m_strExecVer.toLatin1().constData());		// exec-ver
    StdfFile.WriteString("");				// test-cod
    StdfFile.WriteString("");					// test-temperature
    // Construct custom Galaxy USER_TXT
    QString	strUserTxt;
    strUserTxt = GEX_IMPORT_DATAORIGIN_LABEL;
    strUserTxt += ":";
    strUserTxt += GEX_IMPORT_DATAORIGIN_ATETEST;
    strUserTxt += ":FRESCO_SUMMARY";
    StdfFile.WriteString((char *)strUserTxt.toLatin1().constData());	// user-txt
    StdfFile.WriteString("");							// aux-file
    StdfFile.WriteString("");							// package-type
    StdfFile.WriteString(m_strProductID.toLatin1().constData());	// familyID
    StdfFile.WriteRecord();


    QMap<int,CGFrescoSummaryBinning>::Iterator it;
    QMap<int,int>::Iterator itSite;

    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 40;
    for(it = m_mapFrescoSummaryHardBinning.begin(); it != m_mapFrescoSummaryHardBinning.end(); it++)
    {
        for(itSite = (*it).mapCount.begin(); itSite != (*it).mapCount.end(); itSite++)
        {
            // Write HBR/site
            StdfFile.WriteHeader(&RecordReadInfo);
            if(itSite.key() == 255)
                StdfFile.WriteByte(255);					// Test Head
            else
                StdfFile.WriteByte(1);						// Test Head
            StdfFile.WriteByte(itSite.key());			// Test sites
            StdfFile.WriteWord((*it).nNumber);			// HBIN
            StdfFile.WriteDword(itSite.value());			// Total Bins
            if((*it).bPass)
                StdfFile.WriteByte('P');
            else
                StdfFile.WriteByte('F');
            StdfFile.WriteString( (*it).strName.toLatin1().constData());
            StdfFile.WriteRecord();
        }
    }

    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 50;
    for(it = m_mapFrescoSummarySoftBinning.begin(); it != m_mapFrescoSummarySoftBinning.end(); it++)
    {
        for(itSite = (*it).mapCount.begin(); itSite != (*it).mapCount.end(); itSite++)
        {
            // Write SBR/site
            StdfFile.WriteHeader(&RecordReadInfo);
            if(itSite.key() == 255)
                StdfFile.WriteByte(255);					// Test Head
            else
                StdfFile.WriteByte(1);						// Test Head
            StdfFile.WriteByte(itSite.key());			// Test sites
            StdfFile.WriteWord((*it).nNumber);			// SBIN
            StdfFile.WriteDword(itSite.value());			// Total Bins
            if((*it).bPass)
                StdfFile.WriteByte('P');
            else
                StdfFile.WriteByte('F');
            StdfFile.WriteString( (*it).strName.toLatin1().constData());
            StdfFile.WriteRecord();
        }
    }

    // Write PCR
    RecordReadInfo.iRecordType = 1;
    RecordReadInfo.iRecordSubType = 30;
    StdfFile.WriteHeader(&RecordReadInfo);
    StdfFile.WriteByte(255);					// Test Head = ALL
    StdfFile.WriteByte(255);					// Test sites = ALL
    StdfFile.WriteDword(m_nTotalParts);			// Total Parts tested
    StdfFile.WriteDword(0);						// Total Parts re-tested
    StdfFile.WriteDword(0);						// Total Parts aborted
    StdfFile.WriteDword(m_nPassParts);			// Total GOOD Parts
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
// Convert 'FileName' FrescoSummary file, to STDF 'strFileNameSTDF' file
//////////////////////////////////////////////////////////////////////
bool CGFrescoSummarytoSTDF::Convert(const char *FrescoSummaryFileName, const char *strFileNameSTDF)
{
    // No erro (default)
    iLastError = errNoError;

    // If STDF file already exists...do not rebuild it...unless dates not matching!
    QFileInfo fInput(FrescoSummaryFileName);
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
            GS::Gex::Engine::GetInstance().UpdateLabelStatus("Converting data from file "+QFileInfo(FrescoSummaryFileName).fileName()+"...");
            GexScriptStatusLabel->show();
        }
        GS::Gex::Engine::GetInstance().UpdateLabelStatus("Converting data from file "+QFileInfo(FrescoSummaryFileName).fileName()+"...");
        GexScriptStatusLabel->show();
    }
    QCoreApplication::processEvents();

    if(ReadFrescoSummaryFile(FrescoSummaryFileName,strFileNameSTDF) != true)
    {
        //////////////////////////////////////////////////////////////////////
        // For ProgressBar
        if((GexProgressBar != NULL)
        && bHideProgressAfter)
            GexProgressBar->hide();

        if((GexScriptStatusLabel != NULL)
        && bHideLabelAfter)
            GexScriptStatusLabel->hide();
        return false;	// Error reading FrescoSummary file
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
QString CGFrescoSummarytoSTDF::ReadLine(QTextStream& hFile)
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
