#ifdef _WIN32
#include <windows.h>
#include <time.h>
#endif

#include <QApplication>

// Galaxy modules includes
#include <gstdl_utils_c.h>

// includes from GEX
#include <converter_external_file.h>

// Local includes
#include "g-trigger_engine.h"
#include "profile.h"	// Handles .INI file (settings/options)
#include "cstdf.h"


#include <QFileInfo>
#include <QDir>
#include <QThread>

namespace GS
{
namespace GTrigger
{

///////////////////////////////////////////////////////////
// Check for FetTest NO-PAT summaries
///////////////////////////////////////////////////////////
bool GTriggerEngine::CheckForFetTestSummary_NoPatFlow(const QString &lPrnFolder)
{
    // Get list of .PRN files to convert
    QFileInfo				cFileInfo;
    QDir                    cDir;
    QString					lFullFileName;
    QDateTime				cDataTime;
    QStringList::Iterator	itFile;

    cDir.setPath(lPrnFolder);
    cDir.setFilter(QDir::Files);

    // Retrieve list of .PRN files found in folder
    QStringList strlFiles = cDir.entryList(QStringList(QString("*.PRN")));

    // Remove from list files that are not old enough (15 secs old or less)
    int iDeltaSec;
    for(itFile = strlFiles.begin(); itFile != strlFiles.end(); )
    {
        // Get .PRN file in list
        lFullFileName = lPrnFolder + "/" + *itFile;
        cFileInfo.setFile(lFullFileName);

        // Check when last modified
        cDataTime = cFileInfo.created();

        // Old enough?
        iDeltaSec = cDataTime.secsTo(QDateTime::currentDateTime());
        if(iDeltaSec <= mPrnDelay)
            itFile = strlFiles.erase(itFile);
        else
            itFile++;
    }

    // No Summary file to process...
    if(strlFiles.count() == 0)
        return true;

    // Load Mapping file (only used for NO-PAT FetTest summary files)
    if(!LoadBinMapFile(lPrnFolder))
        return false;

    // Do not keep files too recent (15 secs old or less)
    for(itFile = strlFiles.begin(); itFile != strlFiles.end(); itFile++)
    {
        // Get .PRN file in list
        lFullFileName = lPrnFolder + "/" + *itFile;

        // Convert file (+ rename it)
        if(!ConvertFetTestNoPat_ToStdf(lPrnFolder, lFullFileName))
            return false;
    }

    return true;
}

//////////////////////////////////////////////////////////////////////
// Read and Parse LVM TEST->BIN mapping file
//////////////////////////////////////////////////////////////////////
bool GTriggerEngine::ReadBinMapFile_LVM(const QString & lBinMapFile)
{
    try
    {
        mBinMapStore.reset( Qx::BinMapping::BinMapStoreFactory< Qx::BinMapping::lvm_ws >
                            ::MakeBinMapStore( lBinMapFile.toStdString() ) );
    }
    catch( const std::exception & )
    {
        return false;
    }

    return true;
}

//////////////////////////////////////////////////////////////////////
// Load TEST->BIN mapping file
//////////////////////////////////////////////////////////////////////
bool GTriggerEngine::LoadBinMapFile(const QString & lFolder)
{
    QString lString;

    // LOG
    lString = QString("Reading BinMap file in folder: %1").arg(lFolder);
    LogMessage_Debug(lString);

    // Clear variables
    mBinMapFileEnabled = false;

    // GET BINMAP FILE AND FORMAT
    QString strBinmapFile, strBinmapFormat, strErrorMsg;

    // IN: strPath contains the location where the converter_external_file.xml is stored
    // IN: strType must be "final" or "wafer"
    // OUT: strErrorMsg contains the error message is any
    // OUT: strFileName and strFileFormat contain file path and format information
    if(!ConverterExternalFile::GetBinmapFile(
                lFolder,"wafer",strBinmapFile,strBinmapFormat,strErrorMsg))
    {
        // LEGACY: support legacy env. variables
        // Check if 'GEX_FETTEST_BINMAP_FILE' environment variable is set
        char *ptChar = getenv("GEX_FETTEST_BINMAP_FILE");
        if(ptChar == NULL)
        {
            // Error message
            lString = "No GEX_FETTEST_BINMAP_FILE environment variable defined!";
            LogMessage(lString,eError);
            return false;	// Not set...error
        }
        strBinmapFile = QString(ptChar);
        strBinmapFormat=QString("vishay-lvm");
    }

    // Read TEST->BIN mappipng file and create map
    // BINMAP file in G-Trigger is used only for the FETTEST_NOPAT flow,
    // make sure format is the one we support
    if(strBinmapFormat.toLower() == "vishay-lvm")
    {
        mBinningMapFile = strBinmapFile;
        // Enable BinMap feature
        mBinMapFileEnabled = true;
        return ReadBinMapFile_LVM(mBinningMapFile);
    }

    // Error message
    lString = QString("Unsupported BINMAP format for FETEST_NOPAT flow: %1!").arg(strBinmapFormat);
    LogMessage(lString,eError);
    return false;
}

bool GTriggerEngine::TryGetBinMapItemFromBinMapStore(int aTestNumber, QString &aOutputTestName)
{
    try
    {
        const Qx::BinMapping::BinMapItemBase &lItem = mBinMapStore->GetBinMapItemByTestNumber( aTestNumber );
        aOutputTestName = QString::fromStdString( lItem.GetTestName() );
    }
    catch( const std::exception & )
    {
        return false;
    }

    return true;
}

//////////////////////////////////////////////////////////////////////
// Return the Test name (hardcoded) for a given test#
//////////////////////////////////////////////////////////////////////
bool GTriggerEngine::getFetTestName(int iTestNumber, QString & strTestName)
{
    // Check if FetTest Test->Binning mapping file is present.
    // If so, use this mapping file to get test names from test numbers
    if(mBinMapFileEnabled)
    {
        if( ! TryGetBinMapItemFromBinMapStore( iTestNumber, strTestName ) )
            return false;

        return true;
    }

    // No FetTest Test->Binning mapping file, use hard-coded test names
    switch(iTestNumber)
    {
        case 1:
            strTestName = "IGSS @ + 100% VGS";
            break;

        case 2:
            strTestName = "IGSS @ - 100% VGS";
            break;

        case 3:
            strTestName = "IGSS @ + 120% VGS";
            break;

        case 4:
            strTestName = "IGSS @ - 120% VGS";
            break;

        case 5:
            strTestName = "GS SHORT";
            break;

        case 11:
            strTestName = "IGDO";
            break;

        case 20:
            strTestName = "IR#2";
            break;

        case 21:
            strTestName = "VF @ + 100% IF";
            break;

        case 22:
            strTestName = "VF @ - 100% IF";
            break;

        case 23:
            strTestName = "VF @ + 50% IF";
            break;

        case 24:
            strTestName = "VF @ - 50% IF";
            break;

        case 25:
            strTestName = "D VF#1";
            break;

        case 26:
            strTestName = "Vth(OPEN)";
            break;

        case 27:
            strTestName = "Vth1(MIN~MAX)";
            break;

        case 28:
            strTestName = "Vth1(MIN)";
            break;

        case 29:
            strTestName = "Vth1(MAX)";
            break;

        case 30:
            strTestName = "VR";
            break;

        case 31:
            strTestName = "IGSS @ + High Voltage";
            break;

        case 32:
            strTestName = "IGSS @ - High Volatge";
            break;

        case 33:
            strTestName = "IR#1";
            break;

        case 34:
            strTestName = "D VF#2";
            break;

        case 50:
            strTestName = "IBEV";
            break;

        case 51:
            strTestName = "IDSX(100%VDS)";
            break;

        case 52:
            strTestName = "IDSX(50%VDS)";
            break;

        case 53:
            strTestName = "DS SHORT";
            break;

        case 54:
            strTestName = "IDSX  @High Voltage";
            break;

        case 76:
            strTestName = "BVDSX @ 250uA";
            break;

        case 77:
            strTestName = "BVDSX @ 5mA";
            break;

        case 78:
            strTestName = "BVDSX @ 10mA";
            break;

        case 79:
            strTestName = "D BV#1";
            break;

        case 80:
            strTestName = "D BV#2";
            break;

        case 100:
            strTestName = "VDSP_UIS";
            break;

        case 102:
            strTestName = "VDSP";
            break;

        case 103:
            strTestName = "UIS";
            break;

        case 104:
            strTestName = "IDSX(100%VDS after UIS)";
            break;

        case 126:
            strTestName = "rDSon #1 @ VGS= 10V";
            break;

        case 127:
            strTestName = "rDSon #2 @ VGS= 4.5V";
            break;

        case 128:
            strTestName = "rDSon #3 @ VGS= 2.5V";
            break;

        case 129:
            strTestName = "rDSon #4 @ VGS= 1.8V";
            break;

        case 130:
            strTestName = "rDSon #5 @ VGS= 1.5V";
            break;

        case 131:
            strTestName = "rDSon #6 @ VGS= 1.2V";
            break;

        case 133:
            strTestName = "rDSon #7 @ VGS= 2.7V";
            break;

        case 134:
            strTestName = "rDSon #8 @ VGS= 6V";
            break;

        case 201:
            strTestName = "VSDP";
            break;

        default:
            strTestName = "T"+QString::number(iTestNumber);
            break;
    }

    return true;
}

///////////////////////////////////////////////////////////
// Convert for FetTest NO-PAT summary to STDF files.
///////////////////////////////////////////////////////////
bool GTriggerEngine::ConvertFetTestNoPat_ToStdf(const QString & lPrnFolder, const QString & lFullFileName)
{
    int                 iStatus, nTotalWafers=0, nIndex;
    bool                bOk,bStatus;
    QDir                cDir;
    CInputFile          clInputFile(lFullFileName);
    QStringList         strlSplit, strlTestDefinition, strlTestFailures;
    QStringList         strlBinNames, strlBinNumbers, strlBinCounts, strlTempStdfFiles, strlTempLogFiles;
    QString             strFileName, strDateTime, strWaferID;
    CPromisTextFile     cPromis;
    unsigned            uTestNumber,uTotalParts,uTotalPass,uTotalFail,uBinNumber;
    CStdf               StdfFile;
    StdfRecordReadInfo	StdfRecordHeader;
    long                lStartTime,lEndTime,lWaferID;
    QString             strFullLotID="";	// <lot>.<split_lot>
    QString             strLotID="";		// <lot>
    QString             strBinName;
    QDateTime           cDateTime = QDateTime::currentDateTime();
    bool                bExtractingSummaryData = false;
    SummaryInfo         clSummaryInfo(mOutFolderLog, mProduction);
    QDateTime           clLastLogFile = QDateTime::currentDateTime().addSecs(0-mDelay_Fet_NoPAT_LogFiles);

    // LOG
    QString strString = QString("Converting FETTEST NO_PAT PRN file to STDF: %1").arg(lFullFileName);
    LogMessage_Debug(strString);

    // current time...
    lStartTime = time(NULL);

    // Fail reading file, keep for next trial.
    if(clInputFile.Open() == false)
        return true;

    // Read PRN File
    bool      bFileCorrupted=false;
    QString   strParameter,strPromisLine,strPromisFormat;
    int       nNbColumnsToParse=0;
    bStatus = clInputFile.NextLine(strString);
    WaferFilesList  lWaferList;
    while(bStatus && !bFileCorrupted)
    {
        // Check for LotID...
        if(strLotID.isEmpty() && strString.length() > 15)
        {
            // First line long enough to hold LotID.
            // Examples with a '.' between the LotID and the Split#:
            // LOTIDV52820.1                  03/27/08  21:50:32                          1
            // LOTIDY08145.1                  05/20/08  08:15:07                          1
            // Examples with no '.' between the LotID and the Split#:
            // LOTIDY33M1231                  05/19/03  07:17:37                          1
            // LOTIDY33M1238                  05/19/03  02:34:02                          1
            strString.replace("\275","");	// Remove printer control characters that may remain...
            strString.replace("\277","");	// Remove printer control characters that may remain...

            // Simplify whitespaces
            strString = strString.simplified();

            // Check if the line starts with "LOTID". If so, remove it, else error
            if(strString.toLower().startsWith("lotid"))
                strString = strString.mid(5);
            else
            {
                strString = "Failed extracting a valid LotID from PRN file";
                ParsingError(strString, &clInputFile, true);
                return false;
            }

            // Extract LotID and FullLotID (includes the splitlot)
            strFullLotID = strString.section(' ',0,0);	// Extract FullLotID string
            // If the Full LotID doesn't include the '.' between the LotID and the Split#, add it
            if(strFullLotID.indexOf(QString(".")) == -1)
            {
                QString strSplit = strFullLotID.right(1);
                int		nSplit = strSplit.toInt(&bOk,36);
                if(!bOk)
                {
                    strString = "Failed converting base 36 split# for Lot " + strFullLotID;
                    ParsingError(strString, &clInputFile, true);
                    return false;
                }
                strFullLotID = strFullLotID.left(strFullLotID.length()-1);
                strFullLotID += "." + QString::number(nSplit);
            }
            strLotID = strFullLotID.section('.',0,0);	// Extract LotID (remove split-lot#)

            // Get PROMIS data associated with this Lot
            cPromis.strPromisLotID = strFullLotID;
            if(getPROMIS_WafersInSplitLot(lPrnFolder,strFullLotID,cPromis,lWaferList,false,strPromisLine,strPromisFormat) == false)
            {
                // Entry not found in promis file...check in Engineering promis file
                if(getPROMIS_WafersInSplitLot(lPrnFolder,strFullLotID,cPromis,lWaferList,true,strPromisLine,strPromisFormat) == false)
                {
                    if(strPromisLine.isEmpty())
                    {
                        strString = "No info found in PROMIS file fot SplitLot " + strFullLotID;
                    }
                    else
                    {
                        strString = "Error parsing line in PROMIS file fot SplitLot " + strFullLotID;
                        strString += ". Line is \"" + strPromisLine + "\".";
                    }
                    ParsingError(strString, &clInputFile, false);
                    return false;
                }
            }

            // Extract Date/Time info
            // LOTIDY08145.1                  05/20/08  08:15:07                          1
            strDateTime = strString.section(' ', 1, 2);
            if(!extractDateTime_FetTest_NoPAT(cDateTime, strDateTime))
            {
                strString = "Failed parsing date " + strDateTime;
                ParsingError(strString, &clInputFile, true);
                return false;
            }
            lStartTime = cDateTime.toTime_t();

            // Set data for summary file
            clSummaryInfo.m_strLotID = strLotID;
            clSummaryInfo.m_strSplitLotID = strFullLotID;
            clSummaryInfo.m_strProduct = cPromis.strGeometryName;
            clSummaryInfo.m_strProgram = "FetTest program";
        }

        // Check if valid NO-PAT .PRN file
        if(bExtractingSummaryData || strString.contains(QString("WAFER PROBE FAIL COUNTER REPORT"),Qt::CaseInsensitive) > 0)
        {
            // If first time entering in loop...
            if(bExtractingSummaryData == false)
            {
                // Check if we found a valid LotID
                if(strLotID.isEmpty())
                {
                    strString = "Failed extracting a valid LotID";
                    ParsingError(strString, &clInputFile, true);
                    return false;
                }

                // There can be up to 3 test result lines

                // Example with 1 test result line:
                //
                //	WAFER PROBE FAIL COUNTER REPORT
                //	-------------------------------
                //	WAFER#  T#  1   T#  2   T# 26   T# 27   T# 51   T# 52   T# 76   T#126   T#127                            PASS   PASS%   TOTAL
                // ------- ------- ------- ------- ------- ------- ------- ------- ------- ------- ------- ------- ------- ------- ------- -------
                //	02           0       0      37       0       2       0       0       0       0                              84  68.29%     123
                //	01         144      13     828       0      48       0       0       0       0                            5545  84.30%    6578

                // Example with 2 test result lines:
                //
                //	WAFER PROBE FAIL COUNTER REPORT
                //	-------------------------------
                // WAFER#  T#  1   T#  2   T#  3   T#  5   T# 26   T# 27   T# 51   T# 52   T# 53   T# 76   T#100   T#102    PASS   PASS%   TOTAL
                //         T#103   T#104   T#127   T#128   T#129   T#201
                // ------- ------- ------- ------- ------- ------- ------- ------- ------- ------- ------- ------- ------- ------- ------- -------
                //  08           0       0       0       8       0       8       3       0       0       0       0       0    1928  92.92%    2075
                //               5       0       0       0     123       0
                //  09           0       4       3     152      65      48     176       7      62      36       2       0   12328  85.94%   14345
                //              30      14      46     213    1159       0

                // Skip '-----------------------------' line, and read next line: first test definition line
                bStatus = clInputFile.NextLine(strString);
                bStatus = clInputFile.NextLine(strString);
                if(!bStatus)
                {
                    strString = "Incomplete PRN file";
                    ParsingError(strString, &clInputFile, true);
                    return false;
                }

                // Insert ',' delimiters
                for(nIndex = 7; nIndex < strString.length(); nIndex += 8)
                    strString.replace(nIndex,1,",");	// Insert delimiters...
                strlSplit = strString.split(QString(","), QString::KeepEmptyParts);

                // Save nb. of columns to parse
                nNbColumnsToParse = strlSplit.count();
                // Save test numbers
                for(nIndex = 1; nIndex < strlSplit.count()-3; nIndex++)
                {
                    strParameter = strlSplit[nIndex].trimmed();

                    // Remove xxx# prefix if any
                    if(strParameter.contains(QString("#"))  > 0)
                        strParameter = strParameter.section(QString("#"),1).trimmed();

                    // Save extracter parameter
                    if(!strParameter.isEmpty())
                        strlTestDefinition.append(strParameter);
                }

                // Read all test definition lines until the "-----" separator line
                bStatus = clInputFile.NextLine(strString);
                while(bStatus && strString.trimmed().startsWith("T#"))
                {
                    // Insert ',' delimiters
                    for(nIndex = 7; nIndex < strString.length(); nIndex += 8)
                        strString.replace(nIndex,1,",");	// Insert delimiters...

                    strlSplit = strString.split(QString(","), QString::KeepEmptyParts);
                    for(nIndex = 1; nIndex < strlSplit.count(); nIndex++) // Ignore first cell, which is empty
                    {
                        strParameter = strlSplit[nIndex].trimmed();

                        // Remove xxx# prefix if any
                        if(strParameter.contains(QString("#"))  > 0)
                            strParameter = strParameter.section(QString("#"),1).trimmed();

                        // Save extracter parameter
                        if(!strParameter.isEmpty())
                            strlTestDefinition.append(strParameter);
                    }

                    // Read next line
                    bStatus = clInputFile.NextLine(strString);
                }

                // Reached separator line, ignore it
                // Read next line: first wafer data line
                bStatus = clInputFile.NextLine(strString);
                if(!bStatus)
                {
                    strString = "Incomplete PRN file";
                    ParsingError(strString, &clInputFile, true);
                    return false;
                }

                // Flag we need to loop until we've extracted ALL wafer lines.
                bExtractingSummaryData = true;
            }

            do
            {
                // Clear test failures list, binning lists
                strlTestFailures.clear();
                strlBinNames.clear();
                strlBinNumbers.clear();
                strlBinCounts.clear();

                // Read pass/fail count of test# for given wafer
                if(strString.startsWith(QString("SUMMARY"),Qt::CaseInsensitive))
                {
                    // Stop generating STDF files....
                    bExtractingSummaryData = false;
                    break;	// Done processing this .PRN file!
                }

                // Insert ',' delimiters
                for(nIndex = 7; nIndex < strString.length(); nIndex += 8)
                    strString.replace(nIndex,1,",");	// Insert delimiters...

                // Extract data...
                strlSplit = strString.split(QString(","), QString::SkipEmptyParts);

                // Make sure we have the expected nb. of columns
                if(strlSplit.count() != nNbColumnsToParse)
                {
                    // Stop generating STDF files....
                    bExtractingSummaryData = false;
                    bFileCorrupted = true;
                    break;	// Done processing this .PRN file!
                }

                // Save wafer details
                // [0] = Wafer Nb
                // [1] = PASS count
                // [2] = PASS %
                // [3] = TOTAL parts
                strWaferID = strlSplit[0].trimmed();
                uTotalPass = strlSplit[strlSplit.count()-3].trimmed().toUInt();
                uTotalParts = strlSplit[strlSplit.count()-1].trimmed().toUInt();
                lWaferID = strWaferID.toLong();

                // Save test results
                for(nIndex = 1; nIndex < strlSplit.count()-3; nIndex++)
                    strlTestFailures.append(strlSplit[nIndex].trimmed());

                // Read all test result lines
                while(strlTestDefinition.count() > strlTestFailures.count())
                {
                    // Read next line
                    bStatus = clInputFile.NextLine(strString);
                    if(!bStatus)
                    {
                        strString = "Incomplete PRN file";
                        ParsingError(strString, &clInputFile, true);
                        return false;
                    }

                    // Insert ',' delimiters
                    for(nIndex = 7; nIndex < strString.length(); nIndex += 8)
                        strString.replace(nIndex,1,",");	// Insert delimiters...

                    strlSplit = strString.split(QString(","), QString::KeepEmptyParts);
                    for(nIndex = 1; nIndex < strlSplit.count(); nIndex++) // Ignore first cell, which is empty
                    {
                        strParameter = strlSplit[nIndex].trimmed();
                        if(!strParameter.isEmpty())
                            strlTestFailures.append(strParameter);
                    }
                }

                // ALL RESULTS EXTRACTED FOR THIS WAFER: build STDF file

                // Build destination path
                QDir	clDir;
                int		nEndSection = 1;
                QString	strDest, strDir, strPartialDir;
                strDest = mOutFolderStdf + "/" + cPromis.strGeometryName + "/stdf/";
                if(!strDest.isEmpty() && !clDir.exists(strDest))
                {
                    // Try to create folder if it doesn't exist
                    clDir.setPath(strDest);
                    strDir = clDir.absolutePath();
                    strDir.replace('\\', '/');
                    strPartialDir = strDir.section('/', 0, nEndSection++, QString::SectionIncludeLeadingSep|QString::SectionIncludeTrailingSep);
                    while(strPartialDir.length() < strDir.length())
                    {
                        if(!clDir.exists(strPartialDir) && !clDir.mkdir(strPartialDir))
                            break;
                        strPartialDir = strDir.section('/', 0, nEndSection++, QString::SectionIncludeLeadingSep|QString::SectionIncludeTrailingSep);
                    }
                }

                strDest = QDir::toNativeSeparators(QDir::cleanPath(strDest));
                if(!clDir.exists(strDest))
                    QDir{}.mkpath(strDest);

                // Build STDF file name: <lot>.<split-lot>-Wafer_<date.time>_gexmo_nopat.std
                // eg: X32606.5-09_2008May07.195255_gexmo_nopat.std
                // SJM : GCORE-5450 - We actually want to change the format now to use the
                // numeric month so they sort properly. This is key for retests, etc.
                strFileName = strDest + "/" + strFullLotID + "-";	// <lot>.<split-lot>-
                strFileName += strWaferID + "_";	// Wafer#_
                strFileName += cDateTime.toString("yyyyMMdd.hhmmss") + "_gexmo_nopat";

                iStatus = StdfFile.Open(strFileName.toLatin1().data(),STDF_WRITE);
                if(iStatus != CStdf::NoError)
                {
                    // Failed creating STDF file....exception!
                    strString = "Failed creating STDF file " + strFileName;
                    ParsingError(strString, &clInputFile, false);
                    return false;
                }

                StdfFile.SetStdfCpuType(2);

                // Write FAR
                StdfRecordHeader.iRecordType = 0;
                StdfRecordHeader.iRecordSubType = 10;
                StdfFile.WriteHeader(&StdfRecordHeader);
                StdfFile.WriteByte(2);	// CPU type: PC
                StdfFile.WriteByte(4);	// STDF V4
                StdfFile.WriteRecord();

                // Write MIR
                StdfRecordHeader.iRecordType = 1;
                StdfRecordHeader.iRecordSubType = 10;
                StdfFile.WriteHeader(&StdfRecordHeader);
                StdfFile.WriteDword(lStartTime);			// Setup time
                StdfFile.WriteDword(lStartTime);			// Start time
                StdfFile.WriteByte(1);						// Station
                StdfFile.WriteByte((BYTE) 'P');				// Test Mode = PRODUCTION
                StdfFile.WriteByte((BYTE) ' ');				// rtst_cod
                StdfFile.WriteByte((BYTE) ' ');				// prot_cod
                StdfFile.WriteWord(65535);					// burn_tim
                StdfFile.WriteByte((BYTE) ' ');				// cmod_cod
                StdfFile.WriteString(strLotID.toLatin1().constData());		// Lot ID
                StdfFile.WriteString(cPromis.strGeometryName.toLatin1().constData());	// Part Type / Product ID
                StdfFile.WriteString("");								// Node name
                StdfFile.WriteString("FetTest");						// Tester Type
                StdfFile.WriteString("FetTest program");				// Job name
                StdfFile.WriteString("");	// Job rev
                StdfFile.WriteString(cPromis.strPromisLotID.toLatin1().constData());		// sublot-id
                StdfFile.WriteString("");					// operator
                StdfFile.WriteString("");					// exec-type
                StdfFile.WriteString("");					// exe-ver
                StdfFile.WriteString(cPromis.strSiteLocation.toLatin1().constData());	// test-cod
                StdfFile.WriteString("");					// test-temperature
                StdfFile.WriteString("");					// USER_TXT
                StdfFile.WriteString("");					// AUX_FILE
                StdfFile.WriteString("");					// PKG_TYP
                StdfFile.WriteString("");					// FAMLY_ID
                StdfFile.WriteString("");					// DATE_COD
                StdfFile.WriteString(cPromis.strFacilityID.toLatin1().constData());	// FACIL_ID
                StdfFile.WriteString("");					// FLOOR_ID
                StdfFile.WriteString(cPromis.strDsPartNumber.toLatin1().constData());	// PROC_ID
                StdfFile.WriteString("");					// OPER_FRQ
                StdfFile.WriteString("");					// SPEC_NAM
                StdfFile.WriteString("");					// SPEC_VER
                StdfFile.WriteString("");					// FLOW_ID
                StdfFile.WriteString("");					// SETUP_ID
                StdfFile.WriteString("");					// DSGN_REV
                StdfFile.WriteRecord();

                // Write DTR with Gross Die
                QString strUserTxt = "<cmd> gross_die=" + QString::number(cPromis.iGrossDie);
                StdfRecordHeader.iRecordType = 50;
                StdfRecordHeader.iRecordSubType = 30;
                StdfFile.WriteHeader(&StdfRecordHeader);
                StdfFile.WriteString(strUserTxt.toLatin1().constData());			// ASCII text string
                StdfFile.WriteRecord();

                // Write SDR
                StdfRecordHeader.iRecordType = 1;
                StdfRecordHeader.iRecordSubType = 80;
                StdfFile.WriteHeader(&StdfRecordHeader);
                StdfFile.WriteByte((BYTE)1);			// head#
                StdfFile.WriteByte((BYTE)1);			// Group#
                StdfFile.WriteByte((BYTE)1);			// site_count
                StdfFile.WriteByte((BYTE)1);			// array of test site# (dummy!)
                StdfFile.WriteString("");		// HAND_TYP: Handler/prober type
                StdfFile.WriteString("");		// HAND_ID: Handler/prober name
                StdfFile.WriteString("");								// CARD_TYP: Probe card type
                StdfFile.WriteString("");								// CARD_ID: Probe card name
                StdfFile.WriteString("");								// LOAD_TYP: Load board type
                StdfFile.WriteString("");								// LOAD_ID: Load board name
                StdfFile.WriteString("");								// DIB_TYP: DIB board type
                StdfFile.WriteString("");								// DIB_ID: DIB board name
                StdfFile.WriteString("");								// CABL_TYP: Interface cable type
                StdfFile.WriteString("");								// CABL_ID: Interface cable name
                StdfFile.WriteString("");								// CONT_TYP: Handler contactor type
                StdfFile.WriteString("");								// CONT_ID: Handler contactor name
                StdfFile.WriteString("");								// LASR_TYP: Laser type
                StdfFile.WriteString("");								// LASR_ID: Laser name
                StdfFile.WriteString("");								// EXTR_TYP: Extra equipment type
                StdfFile.WriteString(cPromis.strEquipmentID.toLatin1().constData());	// EXTR_ID: Extra equipment name
                StdfFile.WriteRecord();

                // Write WIR
                StdfRecordHeader.iRecordType = 2;
                StdfRecordHeader.iRecordSubType = 10;
                StdfFile.WriteHeader(&StdfRecordHeader);
                StdfFile.WriteByte(1);										// Test head
                StdfFile.WriteByte(255);									// Tester site (all)
                StdfFile.WriteDword(lStartTime);							// Start time
                StdfFile.WriteString(strWaferID.toLatin1().constData());	// WaferID
                StdfFile.WriteRecord();

                // Write WRR
                StdfRecordHeader.iRecordType = 2;
                StdfRecordHeader.iRecordSubType = 20;
                StdfFile.WriteHeader(&StdfRecordHeader);
                StdfFile.WriteByte(1);										// Test head
                StdfFile.WriteByte(255);									// Tester site (all)
                StdfFile.WriteDword(0);										// Time of last part tested
                StdfFile.WriteDword(uTotalParts);							// Parts tested
                StdfFile.WriteDword(0);										// Parts retested
                StdfFile.WriteDword(0);										// Parts Aborted
                StdfFile.WriteDword(uTotalPass);							// Good Parts
                StdfFile.WriteDword(4294967295UL);							// Functionnal Parts
                StdfFile.WriteString(strWaferID.toLatin1().constData());			// WaferID
                StdfFile.WriteRecord();

                // Write TSR
                for(nIndex = 0; nIndex < strlTestDefinition.count(); nIndex++)
                {
                    // Get test#
                    uTestNumber = strlTestDefinition[nIndex].toULong(&bOk);
                    if(!bOk)
                        break;	// We've read all valid entries...

                    // Get pass/fail info
                    uTotalFail = strlTestFailures[nIndex].toULong();			// Total failures for this test in wafer tested.

                    StdfRecordHeader.iRecordType = 10;
                    StdfRecordHeader.iRecordSubType = 30;
                    StdfFile.WriteHeader(&StdfRecordHeader);
                    StdfFile.WriteByte(1);						// Head
                    StdfFile.WriteByte(1);						// Site
                    StdfFile.WriteByte('P');					// Test type
                    StdfFile.WriteDword(uTestNumber);			// Test Number
                    StdfFile.WriteDword(uTotalParts);			// Exec count
                    StdfFile.WriteDword(uTotalFail);			// Fail count
                    StdfFile.WriteDword(0);						// Alarm count
                    getFetTestName(uTestNumber,strString);
                    StdfFile.WriteString(strString.toLatin1().constData());  // Test name
                    StdfFile.WriteRecord();
                }

                unsigned int uTotalFail_Cumul = 0;

                if(uTotalPass > 0)
                {
                    // Specify HBR - Bin1
                    StdfRecordHeader.iRecordType = 1;
                    StdfRecordHeader.iRecordSubType = 40;
                    StdfFile.WriteHeader(&StdfRecordHeader);
                    StdfFile.WriteByte(1);						// Head
                    StdfFile.WriteByte(1);						// Site
                    StdfFile.WriteWord(1);						// HBIN #
                    StdfFile.WriteDword(uTotalPass);			// Hbin count
                    StdfFile.WriteByte('P');					// Pass/Faill bin type
                    StdfFile.WriteString("PASS");				// Bin name
                    StdfFile.WriteRecord();

                    // Write SBR - Bin1
                    StdfRecordHeader.iRecordType = 1;
                    StdfRecordHeader.iRecordSubType = 50;
                    StdfFile.WriteHeader(&StdfRecordHeader);
                    StdfFile.WriteByte(1);						// Head
                    StdfFile.WriteByte(1);						// Site
                    StdfFile.WriteWord(1);						// SBIN #
                    StdfFile.WriteDword(uTotalPass);			// Sbin count
                    StdfFile.WriteByte('P');					// Pass/Faill bin type
                    StdfFile.WriteString("PASS");				// Bin name
                    StdfFile.WriteRecord();
                }

                // Add PASS binning info to list of binnings for wafer log file
                strlBinNames.append("PASS");
                strlBinNumbers.append("1");
                strlBinCounts.append(QString::number(uTotalPass));

                // Write HBR & SBR (same as test numbers with +10 offset !)
                for(nIndex = 0; nIndex < strlTestDefinition.count(); nIndex++)
                {
                    // Get Test#
                    uTestNumber = strlTestDefinition[nIndex].toULong(&bOk);		// Bin#
                    if(!bOk)
                        break;	// We've read all valid entries...

                    uTotalFail = strlTestFailures[nIndex].toULong();		// Total failures for this test in wafer tested.
                    if(uTotalFail > 0)
                    {
                        uTotalFail_Cumul += uTotalFail;

                        // Get Bin# from mapping
                        try
                        {
                            decltype( auto ) lBinMapItem = mBinMapStore->GetBinMapItemByTestNumber( uTestNumber );
                            uBinNumber = lBinMapItem.GetSoftBinNumber();
                            strBinName = QString::fromStdString( lBinMapItem.GetBinName() );
                        }
                        catch(const std::exception & )
                        {
                            strString = "Couldn't find test " + QString::number(uTestNumber);
                            strString += " in mapping file (" + mBinningMapFile + ")";
                            ParsingError(strString, &clInputFile, false);
                            StdfFile.Close();
                            return false;
                        }


                        // Write SBR
                        StdfRecordHeader.iRecordType = 1;
                        StdfRecordHeader.iRecordSubType = 50;
                        StdfFile.WriteHeader(&StdfRecordHeader);
                        StdfFile.WriteByte(1);								// Head
                        StdfFile.WriteByte(1);								// Site
                        StdfFile.WriteWord(uBinNumber);						// SBIN #
                        StdfFile.WriteDword(uTotalFail);					// Sbin count
                        StdfFile.WriteByte('F');							// Pass/Faill bin type
                        StdfFile.WriteString(strBinName.toLatin1().constData());	// Bin name
                        StdfFile.WriteRecord();

                        // Add binning info to list of binnings for wafer log file
                        strlBinNames.append(strBinName);
                        strlBinNumbers.append(QString::number(uBinNumber));
                        strlBinCounts.append(QString::number(uTotalFail));
                    }
                }

                if(uTotalFail_Cumul > 0)
                {
                    // Write HBR
                    StdfRecordHeader.iRecordType = 1;
                    StdfRecordHeader.iRecordSubType = 40;
                    StdfFile.WriteHeader(&StdfRecordHeader);
                    StdfFile.WriteByte(1);						// Head
                    StdfFile.WriteByte(1);						// Site
                    StdfFile.WriteWord(0);						// HBIN #
                    StdfFile.WriteDword(uTotalFail_Cumul);		// Hbin count
                    StdfFile.WriteByte('F');					// Pass/Faill bin type
                    StdfFile.WriteString("FAIL");				// Bin name
                    StdfFile.WriteRecord();
                }

                // Write PCR
                StdfRecordHeader.iRecordType = 1;
                StdfRecordHeader.iRecordSubType = 30;
                StdfFile.WriteHeader(&StdfRecordHeader);
                StdfFile.WriteByte(1);						// Head
                StdfFile.WriteByte(1);						// Site
                StdfFile.WriteDword(uTotalParts);			// Total tested parts
                StdfFile.WriteDword(0);						// Total re-tested parts
                StdfFile.WriteDword(0);						// Total aborted parts
                StdfFile.WriteDword(uTotalPass);			// Total good parts
                StdfFile.WriteRecord();

                // Write MRR record
                lEndTime = time(NULL);

                StdfRecordHeader.iRecordType = 1;
                StdfRecordHeader.iRecordSubType = 20;
                StdfFile.WriteHeader(&StdfRecordHeader);
                StdfFile.WriteDword(lEndTime);			// File finish-time.
                StdfFile.WriteRecord();

                // Close STDF file
                StdfFile.Close();
                strlTempStdfFiles.append(strFileName);

                nTotalWafers++;

                // Add Wafer info for production summary log file
                WaferInfo *pWafer = new WaferInfo;
                pWafer->eWaferType = GS::GTrigger::eNoPatWafer;
                pWafer->bValidWafer = true;
                pWafer->cDateTime = cDateTime;
                pWafer->lWaferID = lWaferID;
                pWafer->iGrossDie = cPromis.iGrossDie;
                pWafer->lTotalDies = uTotalParts;
                pWafer->lTotalGood_PrePAT = uTotalPass;
                // Compute yield info: see if use GrossDie count instead of PRN die count
                long lTotalDies;
                if(pWafer->iGrossDie > 0)
                    lTotalDies = pWafer->iGrossDie;
                else
                    lTotalDies = pWafer->lTotalDies;
                // Compute yield
                if(lTotalDies)
                    pWafer->lfYield_PrePAT  = (100.0*pWafer->lTotalGood_PrePAT)/lTotalDies;
                else
                    pWafer->bValidWafer = false;
                // Binning info
                pWafer->strlBinNames = strlBinNames;
                pWafer->strlBinNumbers = strlBinNumbers;
                pWafer->strlBinCounts = strlBinCounts;

                // Add wafer info to list...unless invalid entry, or more recent entry exists!
                if(!pWafer->bValidWafer || !clSummaryInfo.Add(pWafer))
                    delete pWafer;

                // Create wafer log file (without .log extension), but make sure we respect specified delay between log files creation
                QString		strWaferLogFile;
                clSummaryInfo.WriteWaferLogFile(mAppName ,lWaferID, strWaferLogFile,
                                                mValidWaferThreshold_GrossDieRatio, false);
                strlTempLogFiles.append(strWaferLogFile);

                break;
            }
            while(1);
        }
        else
            // Check if valid NO-PAT .PRN file
            if(strString.contains(QString("WAFER PROBE LOT SUMMARY REPORT"),Qt::CaseInsensitive) > 0)
            {
            }

        // Read next line
        bStatus = clInputFile.NextLine(strString);
    };

    // Close input file
    clInputFile.Close();

    // If file corrupted or no Lot found, delete all temp files
    if(bFileCorrupted || strLotID.isEmpty())
    {
        while(!strlTempStdfFiles.isEmpty())
        {
            // Delete temp STDF files
            strFileName = strlTempStdfFiles.takeFirst();
            cDir.remove(strFileName);
        }
        while(!strlTempLogFiles.isEmpty())
        {
            // Delete temp LOG files
            strFileName = strlTempLogFiles.takeFirst();
            cDir.remove(strFileName);
        }
    }

    // Check if file corrupted
    if(bFileCorrupted)
    {
        strString = "PRN file corrupted";
        ParsingError(strString, &clInputFile, true);
        return false;
    }

    // Check if we found a valid LotID
    if(strLotID.isEmpty())
    {
        strString = "Failed extracting a valid LotID";
        ParsingError(strString, &clInputFile, true);
        return false;
    }

    // Rename STDF files
    while(!strlTempStdfFiles.isEmpty())
    {
        // Rename temp file to .stdf
        strFileName = strlTempStdfFiles.takeFirst();
        strString = strFileName + ".std";
        if(cDir.exists(strString))
            cDir.remove(strString);
        cDir.rename(strFileName,strString);
    }

    // Rename log files + eventually execute shell command (make sure we respect delay between files)
    QDateTime	clNextLogFile;
    while(!strlTempLogFiles.isEmpty())
    {
        // Make sure we wait at least 200ms between execution of 2 scripts
        QThread::msleep(200);
        clNextLogFile = clLastLogFile.addSecs(mDelay_Fet_NoPAT_LogFiles);
        while(QDateTime::currentDateTime() <= clNextLogFile)
        {
            QThread::msleep(100);
            //Not needed anymore, we are running in a thread
            //qApplication->processEvents(QEventLoop::AllEvents, 400);
        }
        clLastLogFile = QDateTime::currentDateTime();

        // Rename temp file to .log
        strFileName = strlTempLogFiles.takeFirst();
        strString = strFileName + ".log";
        if(cDir.exists(strString))
            cDir.remove(strString);
        cDir.rename(strFileName,strString);

        // Call shell ?
        if((strPromisFormat == "vishay-lvm") && !mShell_Fet_NoPAT.isEmpty())
            ExecutePostWaferShell(mShell_Fet_NoPAT, strString);
        if((strPromisFormat == "vishay-hvm") && !mShell_Fet_NoPAT_HVM.isEmpty())
            ExecutePostWaferShell(mShell_Fet_NoPAT_HVM, strString);
    }

    // Check if replication required (if so, copy input PRN file to specified folder)
    char *ptChar = getenv("GEX_SERVER_INPUT_REPLICATION_FOLDER");
    if(ptChar != NULL)
    {
        // Copy file for replication
        QFileInfo	clFileInfo(lFullFileName);
        QString		strDestFile;
        QDir		clDir;
        strDestFile = ptChar;
        strDestFile += "/" + clFileInfo.dir().dirName();
        if(!clDir.exists(strDestFile))
            clDir.mkdir(strDestFile);
        strDestFile += "/" + clFileInfo.fileName();
        ut_CopyFile(lFullFileName.toLatin1().constData(), strDestFile.toLatin1().constData(), UT_COPY_USETEMPNAME);
    }

    // Report .PRN file processed
    LogMessage("FETTEST NoPAT Summary file: "+ lFullFileName +". Total of "+QString::number(nTotalWafers) +" STDF files created.");

    // Create production summary log file
    QString strLotSummary;
    if(clSummaryInfo.WriteSummaryLogFile(strLotSummary, mAppName))
        LogMessage("Summary log file created for splitlot " + clSummaryInfo.m_strSplitLotID + " (" + strLotSummary + ").");

    // Delete summary file if option in .INI set
    if(mDeletePrn)
    {
        // Delete summary
        cDir.remove(lFullFileName);
    }
    else
    {
        // Rename .PRN do .DONE...
        strString = lFullFileName + ".done";

        // Rename
        cDir.remove(strString);
        cDir.rename(lFullFileName,strString);
    }

    return true;
}

void GTriggerEngine::ExecutePostWaferShell(const QString & shell, const QString & waferLogFile)
{
    // Check if Post-Processing shell command line to launch
    if(shell.isEmpty())
        return;

    // Build argument: status, log file
    QString strCommandLine = "1 ";

    // If log file defined, add it to the argument list
    if(!waferLogFile.isEmpty())
    {
        // If file path includes space, we have to specify it within quotes
        if(waferLogFile.indexOf(" ") >= 0)
        {
            strCommandLine += "\"";
            strCommandLine += waferLogFile;
            strCommandLine += "\"";
        }
        else
            strCommandLine += waferLogFile;
    }

    // Launch Shell command (minimized)
#ifdef _WIN32
    ShellExecuteA(NULL,
                  "open",
                  shell.toLatin1().constData(),
                  strCommandLine.toLatin1().constData(),
                  NULL,
                  SW_SHOWMINIMIZED);
#else
    strCommandLine = shell + " " + strCommandLine;
    if ( system(strCommandLine.toLatin1().constData()) == -1)
    {
        //FIXME: sould returns an error or at least update a log
        return;
    }
#endif
}

} // namespace GS
} // namespace GTrigger
