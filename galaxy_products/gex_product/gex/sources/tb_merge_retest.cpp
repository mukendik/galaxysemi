#include "tb_merge_retest.h"
#include "gex_constants.h"
#include "gex_shared.h"
#include "import_all.h"
#include "import_constants.h"
#include "gqtl_log.h"
#include "engine.h"
#include "product_info.h"
#include "message.h"
#include "stdfrecords_v4.h"
#include "stdf_head_and_site_number_decipher.h"
#include "stdf_read_record.h"

#include <unistd.h>
#include <QDir>
#include <QMessageBox>
#include <QPushButton>

bool CompareStdFile(GexTbMergeSampleFileInfo *file1, GexTbMergeSampleFileInfo *file2 )
{
    return(file1->lStartTime < file2->lStartTime);
}

///////////////////////////////////////////////////////////
// Contructor: holds info found in input file (smart merge of test+retest)
///////////////////////////////////////////////////////////
GexTbMergeFileInfo::GexTbMergeFileInfo()
{
    lTotalSoftBins = 0;			// Total parts tested
    lTotalSoftBinsMerged = 0;	// Total parts tested
    lTotalHardBins = 0;			// Total parts tested
    lTotalHardBinsMerged = 0;	// Total parts tested
    bFileCreated = false;
    hStdf = NULL;				// Handle to STDF class
    m_lSetupTime = m_lStartTime = m_lFinishTime = 0;
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
GexTbMergeFileInfo::~GexTbMergeFileInfo()
{
    if(hStdf != NULL)
        delete hStdf;
    hStdf = NULL;
}

///////////////////////////////////////////////////////////
// Contructor: holds info found in input file (for merge of samples only)
///////////////////////////////////////////////////////////
GexTbMergeSampleFileInfo::GexTbMergeSampleFileInfo()
{
    lStartTime = 0;		// Lot Testing time & date
    hStdf = NULL;
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
GexTbMergeSampleFileInfo::~GexTbMergeSampleFileInfo()
{
    if(hStdf != NULL)
        delete hStdf;
    hStdf = NULL;
}

GexTbMergeRetest::GexTbMergeRetest(bool bGui): QObject(NULL)
{
    // Initializes variables
    mTestData = NULL;
    mReTestData = NULL;
    mGDFFileDetected = false;
    mCalledFromGui = bGui;

}

GexTbMergeRetest::~GexTbMergeRetest(){
    if(mTestData != NULL)
        delete mTestData;
    mTestData=0;
    if(mReTestData != NULL)
        delete mReTestData;
    mReTestData=0;
}

///////////////////////////////////////////////////////////
// Read TSR info
///////////////////////////////////////////////////////////
void	GexTbMergeRetest::OverloadTSR(GexTbMergeSampleFileInfo *pInDataFile)
{
    char	szString[257];
    BYTE	bData;
    long	lData,lTestNumber;

    GS::StdLib::StdfRecordReadInfo cStdfRecordHeader;
    cStdfRecordHeader.iRecordType = 10;
    cStdfRecordHeader.iRecordSubType = 30;
    mStdfFile.WriteHeader(&cStdfRecordHeader);

    if(pInDataFile->hStdf->ReadByte(&bData) != GS::StdLib::Stdf::NoError)			// head number: 255=Merged sites
        goto tsr_end;
    mStdfFile.WriteByte(bData);

    if(pInDataFile->hStdf->ReadByte(&bData) != GS::StdLib::Stdf::NoError)			// site number
        goto tsr_end;
    mStdfFile.WriteByte(bData);

    if(pInDataFile->hStdf->ReadByte(&bData) != GS::StdLib::Stdf::NoError)			// test type
        goto tsr_end;
    mStdfFile.WriteByte(bData);

    if(pInDataFile->hStdf->ReadDword(&lTestNumber) != GS::StdLib::Stdf::NoError)	// test_number
        goto tsr_end;
    mStdfFile.WriteDword(lTestNumber);

    if(pInDataFile->hStdf->ReadDword(&lData) != GS::StdLib::Stdf::NoError)			// exec_count
        goto tsr_end;
    mStdfFile.WriteDword(4294967295lu);

    if(pInDataFile->hStdf->ReadDword(&lData) != GS::StdLib::Stdf::NoError)			// fail_count
        goto tsr_end;
    mStdfFile.WriteDword(4294967295lu);

    if(pInDataFile->hStdf->ReadDword(&lData) != GS::StdLib::Stdf::NoError)			// alarm_count
        goto tsr_end;
    mStdfFile.WriteDword(4294967295lu);

    if(pInDataFile->hStdf->ReadString(szString) != GS::StdLib::Stdf::NoError)		// test_name
        goto tsr_end;
    mStdfFile.WriteString(szString);

tsr_end:
    mStdfFile.WriteRecord();

}

///////////////////////////////////////////////////////////
// Read file to build the Bin list (HardBin & SoftBin)
///////////////////////////////////////////////////////////
void	GexTbMergeRetest::ReadBinning(GexTbMergeFileInfo *pDataFile,bool bSoftBin)
{
    int		wData;
    long		lData;
    BYTE		bMergedSites;
    BYTE		bSites;
    BYTE		bType;
    char		szString[257];
    long		ltotalParts;

    switch(mStdfRecordHeader.iStdfVersion)
    {
        default :
            break;

        case GEX_STDFV4:
            pDataFile->hStdf->ReadByte(&bMergedSites);	// head number: 255=Merged sites
            pDataFile->hStdf->ReadByte(&bSites);  // site number
            pDataFile->hStdf->ReadWord(&wData);	// SBIN/HBIN #
            pDataFile->hStdf->ReadDword(&lData);	// SBIN/HBIN count
            if((unsigned)lData & 0xC0000000)
                break;	// Too high number to be valid...also fixes LTX-Fusion bug!
            ltotalParts = lData;
            if(bMergedSites == 255)
            {
                if(bSoftBin)
                {
                    if(pDataFile->map_SoftBinMerged.contains(wData))
                        pDataFile->map_SoftBinMerged[wData] += ltotalParts;
                    else
                        pDataFile->map_SoftBinMerged[wData] = ltotalParts;
                    pDataFile->lTotalSoftBinsMerged += ltotalParts;
                }
                else
                {
                    if(pDataFile->map_HardBinMerged.contains(wData))
                        pDataFile->map_HardBinMerged[wData] += ltotalParts;
                    else
                        pDataFile->map_HardBinMerged[wData] = ltotalParts;
                    pDataFile->lTotalHardBinsMerged += ltotalParts;
                }
            }
            else
            {
                // Check if entry already exists, and if so add count for given test site (unless we already have a negative number...meaning the merged site info takes precedence)
                if(bSoftBin)
                {
                    pDataFile->lTotalSoftBins += ltotalParts;
                    if(pDataFile->map_SoftBin.contains(wData))
                        pDataFile->map_SoftBin[wData] += ltotalParts;	// Entry already existed: then cumulate sites.
                    else
                        pDataFile->map_SoftBin[wData] = ltotalParts;	// Entry didn't exist: create it!
                }
                else
                {
                    pDataFile->lTotalHardBins += ltotalParts;
                    if(pDataFile->map_HardBin.contains(wData))
                        pDataFile->map_HardBin[wData] += ltotalParts;	// Entry already existed: then cumulate sites.
                    else
                        pDataFile->map_HardBin[wData] = ltotalParts;	// Entry didn't exist: create it!
                }
            }
            // Read Pass/Fail info
            // PAss/Fail type

            if (pDataFile->hStdf->ReadByte(&bType) == GS::StdLib::Stdf::NoError)
            {
                if(bSoftBin)
                {
                    if(pDataFile->map_SoftBinCat.contains(wData))
                    {
                        BYTE lOldCat    = pDataFile->map_SoftBinCat[wData];
                        if(lOldCat != bType)
                            pDataFile->map_SoftBinCat[wData] = 'F';
                    }
                    else if (bType != '\0')
                        pDataFile->map_SoftBinCat[wData] = bType;
                }
                else
                {
                    if(pDataFile->map_HardBinCat.contains(wData))
                    {
                        BYTE lOldCat    = pDataFile->map_HardBinCat[wData];
                        if(lOldCat != bType)
                            pDataFile->map_HardBinCat[wData] = 'F';
                    }
                    else if (bType != '\0')
                        pDataFile->map_HardBinCat[wData] = bType;
                }
            }

            // Read Bin name
            pDataFile->hStdf->ReadString(szString);	// BIN name label
            if(bSoftBin)
                pDataFile->map_SoftBinNames[wData] = szString;		// Save Soft bin name.
            else
                pDataFile->map_HardBinNames[wData] = szString;		// Save Hard bin name.
            break;
    }
}


///////////////////////////////////////////////////////////
// Read file to build the Bin list (HardBin & SoftBin)
///////////////////////////////////////////////////////////
int	GexTbMergeRetest::ReadInputFileSummaryInfo(GexTbMergeFileInfo *pDataFile)
{
    int	iStatus;

    // Set start/end times to current time (in case no MRR!)
    long lTime = time(NULL);
    pDataFile->SetStartTime(lTime);
    pDataFile->SetFinishTime(lTime);

    // Open STDF file
    pDataFile->hStdf = new GS::StdLib::Stdf;
    iStatus = pDataFile->hStdf->Open(pDataFile->strFileNameSTDF.toLatin1().constData(),STDF_READ,1000000L);
    if(iStatus != GS::StdLib::Stdf::NoError)
    {
        // Error. Can't open STDF file in read mode!
        mErrorMessage = "Failed opening file (probably corrupted):\n" + pDataFile->strFileName;
        pDataFile->hStdf->Close();	// Clean close.
        return ReadCorrupted;
    }

    // Read one record from STDF file.
    iStatus = pDataFile->hStdf->LoadRecord(&mStdfRecordHeader);
    while(iStatus == GS::StdLib::Stdf::NoError)
    {
        // Process STDF record read.
        switch(mStdfRecordHeader.iRecordType)
        {
        case 1:
            switch(mStdfRecordHeader.iRecordSubType)
            {
            case 10:// Process MIR
                pDataFile->hStdf->ReadDword(&lTime);
                pDataFile->SetSetupTime(lTime);
                pDataFile->hStdf->ReadDword(&lTime);
                pDataFile->SetStartTime(lTime);
                break;
            case 20:// Process MRR
                pDataFile->hStdf->ReadDword(&lTime);
                pDataFile->SetFinishTime(lTime);
                break;
            case 40:// Process HBR records... Type = 1:40
                ReadBinning(pDataFile,false);
                break;
            case 50:// Process SBR records... Type = 1:50
                ReadBinning(pDataFile,true);
                break;
            }
            break;
        }

        // Read one record from STDF file.
        iStatus = pDataFile->hStdf->LoadRecord(&mStdfRecordHeader);
    };

    // Close input STDF file
    pDataFile->hStdf->Close();

    ///////////////////////////////////////////////////////////
    // Check if Bin counts are correct (eg: total hardbin = total softbin, etc...)
    return CheckUpdateBinningTotals(pDataFile);
}

///////////////////////////////////////////////////////////
// Check if Bin counts are correct (eg: total hardbin = total softbin, etc...)
///////////////////////////////////////////////////////////
int	GexTbMergeRetest::CheckUpdateBinningTotals(GexTbMergeFileInfo *pDataFile)
{
    int	iSoftBin,iHardBin;
    bool bValidFile=true;

    if(pDataFile->lTotalSoftBins && pDataFile->lTotalSoftBinsMerged && (pDataFile->lTotalSoftBins != pDataFile->lTotalSoftBinsMerged))
        bValidFile = false;	// Both softBin & Merges softBin are valid but don't match...
    iSoftBin = gex_max(pDataFile->lTotalSoftBins , pDataFile->lTotalSoftBinsMerged);

    if(pDataFile->lTotalHardBins && pDataFile->lTotalHardBinsMerged && (pDataFile->lTotalHardBins != pDataFile->lTotalHardBinsMerged))
        bValidFile = false;	// Both softBin & Merges softBin are valid but don't match...
    iHardBin = gex_max(pDataFile->lTotalHardBins , pDataFile->lTotalHardBinsMerged);

    if(iSoftBin && iHardBin && (iSoftBin != iHardBin))
        bValidFile = false;	// Both softBin & Hardin are valid but don't match...

    // Warning: HBR/SBR mismatch...
    if(bValidFile == false)
        mErrorMessage = "*Warning* Binning tables are inaccurate (Bin mistmatchs)\nMerged binning table generated, but not accurate.";

    // Define which of the Soft Bin list we use (the one with most data)
    if(pDataFile->lTotalSoftBins > pDataFile->lTotalSoftBinsMerged)
        pDataFile->bUseSoftBinMerged = false;	// 'true' if we'll use the SoftBin merger list 'false' if we use the non-merged
    else
        pDataFile->bUseSoftBinMerged = true;

    // Define which of the Hard Bin list we use (the one with most data)
    if(pDataFile->lTotalHardBins > pDataFile->lTotalHardBinsMerged)
        pDataFile->bUseHardBinMerged = false;	// 'true' if we'll use the SoftBin merger list 'false' if we use the non-merged
    else
        pDataFile->bUseHardBinMerged = true;

    // Save total number of parts (Bins)
    pDataFile->lTotalParts = gex_max(iSoftBin , iHardBin);
    return NoError;
}

///////////////////////////////////////////////////////////
// Create HBR and SBR from merged binning results
///////////////////////////////////////////////////////////
int	GexTbMergeRetest::WriteMergedBinningTotal(GexTbMergeFileInfo *pDataFile)
{
    // Compute total of Bin1
    QMap <int, BinInfo> lBinList;

    if(pDataFile->bUseSoftBinMerged)
    {
        if (FillBinningList(pDataFile->map_SoftBinMerged, pDataFile->map_SoftBinCat,
                            pDataFile->map_SoftBinCatFromPRR, pDataFile->map_SoftBinNames, false, lBinList) == false)
        {
            return BinStatusMissing;
        }
    }
    else
    {
        if (FillBinningList(pDataFile->map_SoftBin, pDataFile->map_SoftBinCat,
                            pDataFile->map_SoftBinCatFromPRR, pDataFile->map_SoftBinNames, false, lBinList) == false)
        {
            return BinStatusMissing;
        }
    }

    // Write SBR record
    WriteBinList(50, lBinList);

    ///////////////////////////////////////////////////////////
    // Write HBR ONLY if no SBR flist (Bin#1 is sum of 'test' & 'retest', other bins are from 'retest'
    // Note: GDF Agilent files can have duplicated entries for
    // HBR (same Bin#, different name),...and this makes
    // computation corrupted. As such, the HBR records are built ONLY of no SBR are available!!!!
    if(IsGDFFile() && lBinList.count() > 0)
        return NoError;

    lBinList.clear();

    if(pDataFile->bUseHardBinMerged)
    {
        if (FillBinningList(pDataFile->map_HardBinMerged, pDataFile->map_HardBinCat,
                            pDataFile->map_HardBinCatFromPRR, pDataFile->map_HardBinNames, false, lBinList) == false)
        {
            return BinStatusMissing;
        }
    }
    else
    {
        if (FillBinningList(pDataFile->map_HardBin, pDataFile->map_HardBinCat,
                            pDataFile->map_HardBinCatFromPRR, pDataFile->map_HardBinNames, false, lBinList) == false)
        {
            return BinStatusMissing;
        }
    }

    // Write HBR record
    WriteBinList(40, lBinList);

    // Write PCR record
    WritePCR(lBinList);

    return NoError;
}

bool GexTbMergeRetest::FillBinningList(const QMap<int, int> &lBinCount, const QMap<int, BYTE> &lBinCat,
                                       const QMap<int, BYTE> &lBinCatFromPRR, const QMap<int, QString> &lBinName,
                                       bool lPassOnly, QMap<int, BinInfo> &lBinList)
{

    QList<int>  lKeys = lBinCount.keys();
    int         lBin;
    BYTE        lCat;

    for (int lIdx = 0; lIdx < lKeys.count(); ++lIdx)
    {
        lBin = lKeys.at(lIdx);

        //
        if (lBinCat.contains(lBin))
            lCat = lBinCat.value(lBin);
        else if (lBinCatFromPRR.contains(lBin))
            lCat = lBinCatFromPRR.value(lBin);
        else
        {
            mErrorMessage = QString("Missing Pass/Fail status information for binning %1").arg(lBin);

            GSLOG(SYSLOG_SEV_DEBUG, mErrorMessage.toLatin1().constData());
            return false;
        }

        if (lPassOnly == false || lCat == 'P')
        {
            if (lBinList.contains(lBin))
                lBinList[lBin].m_nBinCount += lBinCount.value(lBin);
            else
            {
                BinInfo lBinInfo;
                lBinInfo.m_nBinCount    = lBinCount.value(lBin);
                lBinInfo.m_cBinCat      = lCat;

                if (lBinName.contains(lBin))
                    lBinInfo.m_strBinName = lBinName.value(lBin);

                lBinList.insert(lBin, lBinInfo);
            }
        }
    }

    return true;
}

///////////////////////////////////////////////////////////
// Erase intermadiate files
///////////////////////////////////////////////////////////
void	GexTbMergeRetest::ExitCleanup()
{
    // Erase intermediate files if some where created
    QDir lDir;
    if(mTestData && (mTestData->bFileCreated == true))
        lDir.remove(mTestData->strFileNameSTDF);
    if(mReTestData && (mReTestData->bFileCreated == true))
        lDir.remove(mReTestData->strFileNameSTDF);
}

///////////////////////////////////////////////////////////
// Build SBR or HBR for all bins
void	GexTbMergeRetest::WriteBinList(int iStdfRecordSubType, const QMap<int, BinInfo> &cMabBinList)
{
    int     lBin;
    int     lCount;
    BYTE	lPassFail;
    QString	lBinName;
    QMap <int, BinInfo>::ConstIterator it;

    for (it  = cMabBinList.constBegin();
         it != cMabBinList.constEnd(); ++it)
    {
        lBin        = it.key();
        lCount      = it.value().m_nBinCount;
        lPassFail   = it.value().m_cBinCat.toLatin1();
        lBinName    = it.value().m_strBinName;

        mStdfRecordHeader.iRecordType = 1;
        mStdfRecordHeader.iRecordSubType = iStdfRecordSubType;
        mStdfFile.WriteHeader(&mStdfRecordHeader);
        mStdfFile.WriteByte(255);                    // Test Head = ALL
        mStdfFile.WriteByte(255);                    // Test sites = ALL
        mStdfFile.WriteWord(lBin);					// SBIN #
        mStdfFile.WriteDword(lCount);				// Total Bins
        mStdfFile.WriteByte(lPassFail);              // Pass / Fail info

        if(lBinName.isEmpty() == false)
            mStdfFile.WriteString(lBinName.toLatin1().constData());

        mStdfFile.WriteRecord();
    }
}

///////////////////////////////////////////////////////////
// Build PCR for all bins
void	GexTbMergeRetest::WritePCR(const QMap<int, BinInfo> &cMabBinList)
{
    int     lCount = 0;
    int     lCountPass = 0;
    QMap <int, BinInfo>::ConstIterator it;

    for (it  = cMabBinList.constBegin();
         it != cMabBinList.constEnd(); ++it)
    {
        lCount      += it.value().m_nBinCount;
        if(it.value().m_cBinCat.toLatin1() == 'P')
            lCountPass += it.value().m_nBinCount;
    }

    mStdfRecordHeader.iRecordType = 1;
    mStdfRecordHeader.iRecordSubType = 30;
    mStdfFile.WriteHeader(&mStdfRecordHeader);
    mStdfFile.WriteByte(255);                    // Test Head = ALL
    mStdfFile.WriteByte(255);                    // Test sites = ALL
    mStdfFile.WriteDword((DWORD)lCount);         // PART_CNT
    mStdfFile.WriteDword((DWORD)0);              // RTST_CNT
    mStdfFile.WriteDword((DWORD)0);              // ABRT_CNT
    mStdfFile.WriteDword((DWORD)lCountPass);     // GOOD_CNT
    mStdfFile.WriteRecord();
}

int	GexTbMergeRetest::loadFilesToMerge(QStringList strSources)
{
    mErrorMessage.clear();

    if(GS::LPPlugin::ProductInfo::getInstance()->isNotSupportedCapability(GS::LPPlugin::ProductInfo::toolbox))
    {
        // OEM mode, refuse to run this function!
        GS::Gex::Message::information(
                    "", "This function is disabled in teradyne mode\n\nContact " +
                    QString(GEX_EMAIL_SALES) + " for more information!");
        mErrorMessage = "This function is disabled in teradyne mode";
        return LicenseError;
    }

    QString strInput1 = strSources[0];
    QString strInput2 = strSources[1];

    ///////////////////////////////////////////////////////////
    // Check if the two input files exist
    ///////////////////////////////////////////////////////////
    if((QFile::exists(strInput1) == false))
    {
        mErrorMessage = "Files to merge do not exist: " + strInput1;
        return NoFile;
    }

    if (QFile::exists(strInput2) == false)
    {
        mErrorMessage = "Files to merge do not exist: " + strInput2;
        return NoFile;
    }

    ///////////////////////////////////////////////////////////
    // Convert input files to STDF if needed (call parser)
    ///////////////////////////////////////////////////////////
    mTestData = new GexTbMergeFileInfo;
    if(ConvertToSTDF(mTestData,strInput1)!= NoError)
        return ReadError;

    mReTestData = new GexTbMergeFileInfo;
    if(ConvertToSTDF(mReTestData,strInput2) != NoError)
        return ReadError;

    ///////////////////////////////////////////////////////////
    // Read each input file to detect which one is the retest (the most recent with fewer parts tested)
    ///////////////////////////////////////////////////////////
    if(ReadInputFileSummaryInfo(mTestData) != NoError)
        return ReadCorrupted;
    if(ReadInputFileSummaryInfo(mReTestData) != NoError)
        return ReadCorrupted;

    return NoError;
}

void GexTbMergeRetest::swapTestReTest()
{
    GexTbMergeFileInfo *poTemp = mTestData;
    mTestData = mReTestData;
    mReTestData = poTemp;
}

QString GexTbMergeRetest::GetErrorMessage() const
{
    return "Merge operation failed: " + mErrorMessage;
}

int	GexTbMergeRetest::MergeFiles(QStringList strSources,QString strOutput)
{
    mErrorMessage.clear();

    if(!mTestData || !mReTestData)
    {
        int lRet = loadFilesToMerge(strSources);
        if(lRet != GexTbMergeRetest::NoError)
            return lRet;
    }

    GexTbMergeFileInfo *pTest=mTestData;
    GexTbMergeFileInfo *pReTest=mReTestData;
    // Compute merged Setup, Start and Finish times
    long lMergedSetupTime, lMergedStartTime=0, lMergedFinishTime=0;
    lMergedSetupTime = pTest->GetSetupTime();
    lMergedStartTime = pTest->GetStartTime();
    lMergedFinishTime = lMergedStartTime + pTest->GetTestDuration() + pReTest->GetTestDuration();

    ///////////////////////////////////////////////////////////
    // Create STDF destination file.
    if(mStdfFile.Open(strOutput.toLatin1().constData(),STDF_WRITE) != GS::StdLib::Stdf::NoError)
    {
        // Failed importing CSV file into STDF database
        mErrorMessage = "Failed to create output file.\nDisk full or protection issue?:\n" + strOutput;

        // Exit cleanup: erase temporary files.
        ExitCleanup();

        return WriteSTDF;
    }

    ///////////////////////////////////////////////////////////
    // Write All from 'Test' data file except Binning & MRR records.
    int iStatus = pTest->hStdf->Open(pTest->strFileNameSTDF.toLatin1().constData(),STDF_READ,1000000L);
    if(iStatus != GS::StdLib::Stdf::NoError)
    {
        // Error. Can't open STDF file in read mode!
        mErrorMessage = "Failed opening file (probably corrupted):\n" + pTest->strFileName;
        mStdfFile.Close();
        pTest->hStdf->Close();	// Clean close.

        // Exit cleanup: erase temporary files.
        ExitCleanup();

        return ReadCorrupted;
    }

    // Make sure we recreate the STDF output in the same CPU format as it was (sothat records we dump and reecords we recreate are in the right and same CPU type!)
    mStdfFile.SetStdfCpuType(pTest->hStdf->GetStdfCpuType());

    // Write FAR
    mStdfRecordHeader.iRecordType = 0;
    mStdfRecordHeader.iRecordSubType = 10;
    mStdfFile.WriteHeader(&mStdfRecordHeader);
    mStdfFile.WriteByte(pTest->hStdf->GetStdfCpuType());	// CPU type (same as original file).
    mStdfFile.WriteByte(4);								// STDF V4
    mStdfFile.WriteRecord();

    GexTbMergeFileInfo oTempData;

    // Read one record from STDF file.
    bool	bWriteRecord;
    long	lCurrentPos;
    iStatus = pTest->hStdf->LoadRecord(&mStdfRecordHeader);
    while(iStatus == GS::StdLib::Stdf::NoError)
    {
        bWriteRecord = true;
        // Process STDF record read.
        switch(mStdfRecordHeader.iRecordType)
        {
        case 0:
            switch(mStdfRecordHeader.iRecordSubType)
            {
            case 10:	// FAR
                bWriteRecord = false;		// Don't write FAR, it has already been written
                break;
            }
            break;
        case 1:
            switch(mStdfRecordHeader.iRecordSubType)
            {
            case 10:	// MIR
                lCurrentPos = pTest->hStdf->GetReadRecordPos();			// Get current buffer position
                pTest->hStdf->OverwriteReadDword(lMergedSetupTime);		// Overwrite Setup_T
                pTest->hStdf->OverwriteReadDword(lMergedStartTime);		// Overwrite Start_T
                pTest->hStdf->SetReadRecordPos(lCurrentPos);			// Revert read-buffer offset position (for DumpRecord)
                break;
            case 20:	// MRR
                bWriteRecord = false;		// Don't write MRR, it will be written with re-test data
                break;
            case 30:	// PCR
            case 40:	// HBR
            case 50:	// SBR
                bWriteRecord = false;		// IGNORE these records
                break;
            }
            break;
        case 2:
            switch(mStdfRecordHeader.iRecordSubType)
            {
            case 20:        // WRR
            {
                GQTL_STDF::Stdf_WRR_V4 oWRRRecord;
                oTempData.hStdf = pTest->hStdf;
                int iError = 0;
                bool bRet = MergeWRR(oTempData, oWRRRecord, &iError);//Test
                if(iError == 1){
                    mStdfFile.Close();
                    pTest->hStdf->Close();	// Clean close.
                    oTempData.hStdf = NULL;
                    // Exit cleanup: erase temporary files.
                    ExitCleanup();

                    return WriteSTDF;
                }
                if(!bRet)
                    GSLOG (SYSLOG_SEV_WARNING," Can not Write WRR Record  ");
                bWriteRecord = false;
            }
                break;

            case 10: // Write only the first  WIR
                break;
            }
            break;

        case 5:
            switch (mStdfRecordHeader.iRecordSubType)
            {
            case 20:  // Process PRR records... Type = 5:20
            {
                long lInitPost = pTest->hStdf->GetReadRecordPos();

                iStatus = ReadPRR(*pTest, *pTest->hStdf);
                if (iStatus != NoError)
                {
                    mStdfFile.Close();
                    pTest->hStdf->Close();	// Clean close.

                    // Exit cleanup: erase temporary files.
                    ExitCleanup();

                    return ReadCorrupted;
                }

                pTest->hStdf->SetReadRecordPos(lInitPost);
                break;
            }
            default:
                break;
            }
            break;

        case 50:
        {
            // specific GDR that could lead to set a deciphering mode for test
            // number
            if( mStdfRecordHeader.iRecordSubType == 10 )
            {
                GQTL_STDF::Stdf_GDR_V4 gdr;
                gdr.Read( *pTest->hStdf );

                GQTL_STDF::HeadAndSiteNumberDecipher::SetDecipheringModeInStdf
                    ( gdr, *pTest->hStdf );
            }
            break;
        }
        }

        // Check if Write this Record to new STDF output file.
        if(bWriteRecord)
            mStdfFile.DumpRecord(&mStdfRecordHeader,pTest->hStdf);

        // Read one record from STDF file.
        iStatus = pTest->hStdf->LoadRecord(&mStdfRecordHeader);
    };

    // Close input 'Test' STDF file
    pTest->hStdf->Close();	// Clean close.

    ///////////////////////////////////////////////////////////
    // Write All from 'ReTest' data file except FAR, ATR, MIR, Binning, TSR, & MRR records.
    iStatus = pReTest->hStdf->Open(pReTest->strFileNameSTDF.toLatin1().constData(),STDF_READ,1000000L);
    if(iStatus != GS::StdLib::Stdf::NoError)
    {
        // Error. Can't open STDF file in read mode!
        mErrorMessage = "Failed opening file (probably corrupted):\n" + pReTest->strFileName;
        mStdfFile.Close();
        pReTest->hStdf->Close();	// Clean close.

        // Exit cleanup: erase temporary files.
        ExitCleanup();

        return ReadCorrupted;
    }

    // Read one record from STDF file.
    iStatus = pReTest->hStdf->LoadRecord(&mStdfRecordHeader);
    while(iStatus == GS::StdLib::Stdf::NoError)
    {
        // Process STDF record read.
        switch(mStdfRecordHeader.iRecordType)
        {
        default:
        case 2:		// WIR, WRR, WCR
            switch(mStdfRecordHeader.iRecordSubType)
            {
            case 20:        // WRR
            {
                oTempData.hStdf = pReTest->hStdf;
                GQTL_STDF::Stdf_WRR_V4 oWRRRecord;
                int iError = 0;
                bool bRet = MergeWRR(oTempData, oWRRRecord, &iError);//Retest
                if(iError == 1)
                {
                    mStdfFile.Close();
                    pReTest->hStdf->Close();	// Clean close.
                    oTempData.hStdf = NULL;
                    // Exit cleanup: erase temporary files.
                    ExitCleanup();

                    return WriteSTDF;
                }
                if(bRet)
                    bRet = WriteWRR(oTempData, oWRRRecord);
                oTempData.hStdf = NULL;

                if(!bRet)
                {
                    GSLOG (SYSLOG_SEV_WARNING," Can not Write WRR Record  ");
                    bWriteRecord = true;
                }else
                    bWriteRecord = false;
            }
                break;
            case 10: //No Need to write WIR already written from the first Test file
                bWriteRecord = false;
                break;
            case 30: //No Need to write WCR already written from the first Test file
                bWriteRecord = false;
                break;
            default:
                bWriteRecord = true;
            }
            break;

        case 5:
            switch (mStdfRecordHeader.iRecordSubType)
            {
            case 20:  // Process PRR records... Type = 5:20
            {
                long lInitPost = pReTest->hStdf->GetReadRecordPos();

                iStatus = ReadPRR(*pReTest, *pReTest->hStdf);
                if (iStatus != NoError)
                {
                    mStdfFile.Close();
                    pReTest->hStdf->Close();	// Clean close.

                    // Exit cleanup: erase temporary files.
                    ExitCleanup();

                    return ReadCorrupted;
                }

                pReTest->hStdf->SetReadRecordPos(lInitPost);
                break;
            }
            default:
                break;
            }
            bWriteRecord = true;	// Copy this record to the output file.
            break;

        case 15:	// PTR, MPR, FTR
            bWriteRecord = true;  // Copy this record to the output file
            break;
        case 20:	// BPS, EPS
            bWriteRecord = true;  // Copy this record to the output file
            break;
        case 50:	// GDR, DTR
        {
            // specific GDR that could lead to set a deciphering mode for test
            // number
            if( mStdfRecordHeader.iRecordSubType == 10 )
            {
                GQTL_STDF::Stdf_GDR_V4 gdr;
                gdr.Read( *pReTest->hStdf );

                GQTL_STDF::HeadAndSiteNumberDecipher::SetDecipheringModeInStdf
                    ( gdr, *pReTest->hStdf );
            }

            bWriteRecord = true;	// Copy this record to the output file.
            break;
        }

        case 0:
        case 1:
            bWriteRecord = false;	// IGNORE these records.
            break;
        }

        // Check if this record has to be written to the output file
        if(bWriteRecord)
            mStdfFile.DumpRecord(&mStdfRecordHeader,pReTest->hStdf);

        // Read one record from STDF file.
        iStatus = pReTest->hStdf->LoadRecord(&mStdfRecordHeader);
    };

    // Close input 'ReTest' STDF file
    pReTest->hStdf->Close();

    ///////////////////////////////////////////////////////////
    // Build SBR of bin pass (Soft Bin) record
    QMap <int, BinInfo> lBinList;
    if(pTest->bUseSoftBinMerged)
    {
        if (FillBinningList(pTest->map_SoftBinMerged, pTest->map_SoftBinCat,
                            pTest->map_SoftBinCatFromPRR, pTest->map_SoftBinNames, true, lBinList) == false)
        {
            return BinStatusMissing;
        }
    }
    else
    {
        if (FillBinningList(pTest->map_SoftBin, pTest->map_SoftBinCat,
                            pTest->map_SoftBinCatFromPRR, pTest->map_SoftBinNames, true, lBinList) == false)
        {
            return BinStatusMissing;
        }
    }

    // Add to this, all bins from ReTest...
    if(pReTest->bUseSoftBinMerged)
    {
        if (FillBinningList(pReTest->map_SoftBinMerged, pReTest->map_SoftBinCat,
                            pReTest->map_SoftBinCatFromPRR, pReTest->map_SoftBinNames, false, lBinList) == false)
        {
            return BinStatusMissing;
        }
    }
    else
    {
        if (FillBinningList(pReTest->map_SoftBin, pReTest->map_SoftBinCat,
                            pReTest->map_SoftBinCatFromPRR, pReTest->map_SoftBinNames, false, lBinList) == false)
        {
            return BinStatusMissing;
        }
    }

    ///////////////////////////////////////////////////////////
    // FIRST: Write SBR list (Bin#1 is sum of 'test' & 'retest', other bins are from 'retest'
    WriteBinList(50, lBinList);

    ///////////////////////////////////////////////////////////
    // Write HBR ONLY if no SBR flist (Bin#1 is sum of 'test' & 'retest', other bins are from 'retest'
    // Note: GDF Agilent files can have duplicated entries for
    // HBR (same Bin#, different name),...and this makes
    // computation corrupted. As such, the HBR records are built ONLY of no SBR are available!!!!
    if(lBinList.count() <= 0 || !IsGDFFile())
    {
        lBinList.clear();

        if(pTest->bUseHardBinMerged)
        {
            if (FillBinningList(pTest->map_HardBinMerged, pTest->map_HardBinCat,
                                pTest->map_HardBinCatFromPRR, pTest->map_HardBinNames, true, lBinList) == false)
            {
                return BinStatusMissing;
            }
        }
        else
        {
            if (FillBinningList(pTest->map_HardBin, pTest->map_HardBinCat,
                                pTest->map_HardBinCatFromPRR, pTest->map_HardBinNames, true, lBinList) == false)
            {
                return BinStatusMissing;
            }
        }

        // Add to this, the number of Bin1 from ReTest...
        if(pReTest->bUseHardBinMerged)
        {
            if (FillBinningList(pReTest->map_HardBinMerged, pReTest->map_HardBinCat,
                                pTest->map_HardBinCatFromPRR, pReTest->map_HardBinNames, false, lBinList) == false)
            {
                return BinStatusMissing;
            }
        }
        else
        {
            if (FillBinningList(pReTest->map_HardBin, pReTest->map_HardBinCat,
                                pTest->map_HardBinCatFromPRR, pReTest->map_HardBinNames, false, lBinList) == false)
            {
                return BinStatusMissing;
            }
        }

        WriteBinList(40, lBinList);

        // Write PCR record
        WritePCR(lBinList);

    }

    ///////////////////////////////////////////////////////////
    // Build MRR record
    mStdfRecordHeader.iRecordType = 1;
    mStdfRecordHeader.iRecordSubType = 20;
    mStdfFile.WriteHeader(&mStdfRecordHeader);
    mStdfFile.WriteDword(lMergedFinishTime);			// File finish-time.
    mStdfFile.WriteRecord();

    // Close STDF file
    mStdfFile.Close();

    // Exit cleanup: erase temporary files.
    ExitCleanup();

    return NoError;
}


///////////////////////////////////////////////////////////
// Read file to extract testing date & time (from MIR or WIR)
///////////////////////////////////////////////////////////
int	GexTbMergeRetest::ReadInputTestTimeInfo(GexTbMergeSampleFileInfo *pDataFile)
{
    int	iStatus;
    BYTE	bData;							// Temporary buffer when reading STDF object
    unsigned long	lTime,lData1,lData2;	// same as above

    pDataFile->hStdf = new GS::StdLib::Stdf;
    iStatus = pDataFile->hStdf->Open(pDataFile->strFileNameSTDF.toLatin1().constData(),STDF_READ,1000000L);
    if(iStatus != GS::StdLib::Stdf::NoError)
    {
        // Error. Can't open STDF file in read mode!
        mErrorMessage = "Failed opening file (probably corrupted):\n" + pDataFile->strFileName;
        pDataFile->hStdf->Close();	// Clean close.

        GSLOG(SYSLOG_SEV_ERROR, mErrorMessage.toLatin1().constData());
        return ReadCorrupted;
    }

    // Read one record from STDF file.
    iStatus = pDataFile->hStdf->LoadRecord(&mStdfRecordHeader);
    while(iStatus == GS::StdLib::Stdf::NoError)
    {
        // Process STDF record read.
        switch(mStdfRecordHeader.iRecordType)
        {
        case 1:
            switch(mStdfRecordHeader.iRecordSubType)
            {
            case 10:// Process MIR records... Type = 1:10
                switch(mStdfRecordHeader.iStdfVersion)
                {
                default :
                    break;

                case GEX_STDFV4:	// MIR STDF V4
                    pDataFile->hStdf->ReadDword((long *)&lData1);			// Setup_T
                    pDataFile->hStdf->ReadDword((long *)&lData2);			// Start_T
                    lTime = gex_max(lData1,lData2);	// Take the most recent of setup-time & start-time
                    if(lTime)
                    {
                        // If valid time detected, then return now!
                        pDataFile->lStartTime = lTime;
                        return NoError;
                    }
                    break;
                }
            }
            break;
        case 2:
            switch(mStdfRecordHeader.iRecordSubType)
            {
            case 10:// Process WIR records... Type = 2:10
                pDataFile->hStdf->ReadByte(&bData);	// Head
                pDataFile->hStdf->ReadByte(&bData);	// pad (stdf V3), test site (stdf V4)
                pDataFile->hStdf->ReadDword((long *)&lTime);	// START_T
                pDataFile->lStartTime = lTime;
                return NoError;
            }
            break;
        }

        // Read one record from STDF file.
        iStatus = pDataFile->hStdf->LoadRecord(&mStdfRecordHeader);
    };

    // Close input STDF file
    pDataFile->hStdf->Close();

    return NoError;
}

int GexTbMergeRetest::ReadPRR(GexTbMergeFileInfo& lTestFile, GS::StdLib::Stdf& lStdf)
{
    BYTE            lBinCat = 'F';
    unsigned short  lSoftBin;
    unsigned short  lHardBin;

    switch(mStdfRecordHeader.iStdfVersion)
    {
        case GEX_STDFV4:
        {
            GQTL_STDF::Stdf_PRR_V4 lPrrRecordV4;

            if (lPrrRecordV4.Read(lStdf) == false)
            {
                mErrorMessage = QString("Failed to read PRR at record %1 for file %2")
                                    .arg(lStdf.GetReadRecordNumber()).arg(lTestFile.strFileName);

                GSLOG(SYSLOG_SEV_ERROR, mErrorMessage.toLatin1().constData());
                return ReadCorrupted;
            }

            lBinCat     = (lPrrRecordV4.m_b1PART_FLG & 0x08) ? 'F' : 'P';
            lSoftBin    = lPrrRecordV4.m_u2SOFT_BIN;
            lHardBin    = lPrrRecordV4.m_u2HARD_BIN;
        }
            break;

        default:

            mErrorMessage = QString("Unsupported STDF version: %1 in file %2")
                                .arg(mStdfRecordHeader.iStdfVersion).arg(lTestFile.strFileName);
            GSLOG(SYSLOG_SEV_ERROR, mErrorMessage.toLatin1().constData());
            return ReadError;
    }

    if (lSoftBin <= 32767)
        lTestFile.map_SoftBinCatFromPRR[lSoftBin] = lBinCat;

    lTestFile.map_HardBinCatFromPRR[lHardBin] = lBinCat;

    return NoError;
}


int	GexTbMergeRetest::MergeSamplesFiles(QStringList strSources, QString strOutput,
                                        bool bRebuildHbrSbr, bool bSortByDate)
{
    ///////////////////////////////////////////////////////////
    // Check if the N*input files exist
    ///////////////////////////////////////////////////////////
    GexTbMergeSampleFileInfo *ptFile=0;
    bool lPIRHasBeenRead = false;

    QList <GexTbMergeSampleFileInfo*> cFilesList;
    int iStatus;
    int	iFileIndex=0;
#ifdef __sparc__
    int iCpuType = 1;
#else
    int iCpuType = 2;
#endif

    mErrorMessage.clear();

    // Structure used to compute merged Binnings.
    GexTbMergeFileInfo *pMergeBinning = new GexTbMergeFileInfo;

    for(QStringList::Iterator it = strSources.begin(); it != strSources.end(); ++it )
    {
        ptFile = new GexTbMergeSampleFileInfo;
        ptFile->strFileName = *it;
        cFilesList.append(ptFile);
        if(QFile::exists(ptFile->strFileName) == false)
        {
            mErrorMessage = "File to merge does not exist:\n" + ptFile->strFileName;
            iStatus = NoFile;
            goto end_sample_merge;
        }

        // Convert to STDF if need be.
        iStatus = ConvertToSTDF(ptFile,ptFile->strFileName);
        if(iStatus != NoError)
            goto end_sample_merge;

        // Read MIR record to extract the Time stamp info (so to sort files by date)
        iStatus = ReadInputTestTimeInfo(ptFile);
        if(iStatus != NoError)
        {
            goto end_sample_merge;
        }

        // Close input STDF file (will reopen later at time of WRITE-MERGE)
        ptFile->hStdf->Close();
    }

    //	// Sort list so first file is original test, then follow retests.
    if(bSortByDate)
        qSort(cFilesList.begin(), cFilesList.end(), CompareStdFile);

    // First, create the output file.
    // If it fails, return an error
    iStatus = mStdfFile.Open(strOutput.toLatin1().constData(),STDF_WRITE);
    if(iStatus != GS::StdLib::Stdf::NoError)
    {
        // Failed importing CSV file into STDF database
        mErrorMessage = "Failed to create output file.\nDisk full or protection issue?:\n" + strOutput;

        // Exit cleanup: erase temporary files.
        ExitCleanup();

        goto end_sample_merge;
    }

    // Dump (merge) all files, one at a time
    while(iFileIndex != cFilesList.count())
    {
        // Get input file to dump
        ptFile = cFilesList.at(iFileIndex);

        // Get Handle to STDF reader
        pMergeBinning->hStdf = ptFile->hStdf;

        iStatus = ptFile->hStdf->Open(ptFile->strFileNameSTDF.toLatin1().constData(),STDF_READ,1000000L);
        if(iStatus != GS::StdLib::Stdf::NoError)
        {
            // Error. Can't open STDF file in read mode!
            mErrorMessage = "Failed opening file (probably corrupted):\n" + ptFile->strFileName;
            mStdfFile.Close();
            ptFile->hStdf->Close();	// Clean close.

            goto end_sample_merge;
        }

        // If first file, let's write write FAR + MIR
        if(iFileIndex == 0)
        {
            // Make sure we recreate the STDF output in the same CPU format as it was (so that records we dump and records we recreate are in the right and same CPU type!)
            mStdfFile.SetStdfCpuType(ptFile->hStdf->GetStdfCpuType());

            // Write FAR
            mStdfRecordHeader.iRecordType = 0;
            mStdfRecordHeader.iRecordSubType = 10;
            mStdfFile.WriteHeader(&mStdfRecordHeader);
            mStdfFile.WriteByte(ptFile->hStdf->GetStdfCpuType());	// CPU type (same as original file).
            mStdfFile.WriteByte(4);					// STDF V4
            mStdfFile.WriteRecord();
            iCpuType = ptFile->hStdf->GetStdfCpuType();
        }

        // Read one record from STDF file.
        bool bWriteRecord;
        iStatus = ptFile->hStdf->LoadRecord(&mStdfRecordHeader);
        while(iStatus == GS::StdLib::Stdf::NoError)
        {
            bWriteRecord = true;
            // Process STDF record read.
            switch(mStdfRecordHeader.iRecordType)
            {
            case 0:
                switch(mStdfRecordHeader.iRecordSubType)
                {
                case 10:	// FAR
                    bWriteRecord = false;
                    break;
                }
                break;

            case 1:
                switch(mStdfRecordHeader.iRecordSubType)
                {
                case 10:	// MIR
                {
                    if(iFileIndex == 0){
                        long lInitPost = ptFile->hStdf->GetReadRecordPos();
                        SetGDFFile(IsGDFMIR(ptFile->hStdf));
                        ptFile->hStdf->SetReadRecordPos(lInitPost);
                    }

                }
                case 60:// Process PMR records... Type = 1:60
                case 62:// Process PGR records... Type = 1:62
                case 63:// Process PLR records... Type = 1:63
                case 80:// Process SDR records... Type = 1:80
                    if(iFileIndex == 0)
                        break;	// Write MIR/PMR/PGR/PLR/SDR if first input (sorted) STDF file
                    bWriteRecord = false;	// IGNORE these records
                    break;							break;

                case 20:	// MRR
                    // Read end-of-test time.
                    if(iFileIndex == cFilesList.count()-1)
                    {
                        // If we have to create updated HBR,SBR and PCR records, let's do it now just before the MRR!
                        if(bRebuildHbrSbr)
                        {
                            // Dump HBR & SBR from: pMergeBinning
                            iStatus = CheckUpdateBinningTotals(pMergeBinning);
                            if(iStatus != NoError)
                                goto end_sample_merge;

                            iStatus = WriteMergedBinningTotal(pMergeBinning);
                            if (iStatus != NoError)
                                goto end_sample_merge;

                            // Reload STDF header type to write (MRR)...because global variable overloeded in 'WriteMergedBinningTotal()'
                            mStdfRecordHeader.iRecordType = 1;
                            mStdfRecordHeader.iRecordSubType = 20;
                        }
                        // Write MRR of last input (sorted) STDF file
                        break;
                    }
                    bWriteRecord = false;	// IGNORE these records
                    break;							break;

                case 30:	// PCR
                    bWriteRecord = false;       // IGNORE these records
                    break;
                case 40:	// HBR
                    bWriteRecord = false;	// IGNORE these records

                    // If we have to create updated HBR,SBR and PCR records, let's collect data here...
                    if(bRebuildHbrSbr)
                    {
                        ReadBinning(pMergeBinning,false);
                    }
                    break;

                case 50:	// SBR
                    bWriteRecord = false;	// IGNORE these records

                    // If we have to create updated HBR,SBR and PCR records, let's collect data here...
                    if(bRebuildHbrSbr)
                    {
                        ReadBinning(pMergeBinning,true);
                    }
                    break;
                }
                break;

            case 2:
                switch(mStdfRecordHeader.iRecordSubType)
                {
                case 10:	// WIR
                    if(iFileIndex == 0)
                        break;	// Write WIR if first input (sorted) STDF file
                    bWriteRecord = false;	// IGNORE these records
                    break;							break;

                case 20:	// WRR
                {
                    //Merge WRR
                    GQTL_STDF::Stdf_WRR_V4 oWRRRecord;
                    int iError = 0;
                    bool bRet = MergeWRR(*pMergeBinning, oWRRRecord, &iError);
                    if(iError == 1)
                    {
                        mStdfFile.Close();
                        ptFile->hStdf->Close();	// Clean close.
                        pMergeBinning->hStdf = NULL;
                        return WriteSTDF;
                    }
                    if(iFileIndex == cFilesList.count()-1 && bRet)
                        bRet = WriteWRR(*pMergeBinning, oWRRRecord);//samples one type
                    if(!bRet)
                        GSLOG (SYSLOG_SEV_WARNING," Can not Write WRR Record  ");

                    bWriteRecord = false;       // IGNORE these records
                }
                    break;

                case 30:	// WCR
                    if(iFileIndex == 0)
                        break;	// Write WCR if first input (sorted) STDF file
                    bWriteRecord = false;	// IGNORE these records
                    break;
                }
                break;

            case 5:
                switch (mStdfRecordHeader.iRecordSubType)
                {
                case 20:  // Process PRR records... Type = 5:20
                {
                    long lInitPost = ptFile->hStdf->GetReadRecordPos();

                    iStatus = ReadPRR(*pMergeBinning, *ptFile->hStdf);
                    if (iStatus != NoError)
                        goto end_sample_merge;

                    ptFile->hStdf->SetReadRecordPos(lInitPost);
                    lPIRHasBeenRead = false;
                    break;
                }
                case 10:  // Process PIR records... Type = 5:10
                {
                    lPIRHasBeenRead = true;
                    break;
                }
                default:
                    break;
                }
                break;

            case 10:
                switch(mStdfRecordHeader.iRecordSubType)
                {
                case 30:	// TSR
                    // When reading LAST of STDF files, keep TSRs...only clear the counts (so to only keep test# and test name)
                    if(iFileIndex == cFilesList.count()-1)
                        OverloadTSR(ptFile);
                    bWriteRecord = false;	// IGNORE these records...but keep track of test list!
                    break;
                }
                case 15: // PTR, FTR, MPR
                {
                // Ignore the test definition in the biginning of the new files
                if (lPIRHasBeenRead == false && iFileIndex != 0)
                {
                    // Read record and check if the test_flg & 10 != 0 and the param_flg == 0
                    PTRFileRecord lPTRFileRecord;
                    // Save the original offset to put it after the reading of the record
                    int	lOffsetBeforeRead = ptFile->hStdf->GetReadRecordPos();
                    lPTRFileRecord.readPTR(*(ptFile->hStdf));
                    ptFile->hStdf->SetReadRecordPos(lOffsetBeforeRead);

                    // Check the test_flg and the param_flg
                    if (((lPTRFileRecord.testFlag & 0x10) != 0) && (lPTRFileRecord.paramFlg == 0x00))
                    {
                        bWriteRecord = false;
                    }
                }
                break;
                }
                break;
            }


            // Check if Write this Record to new STDF output file.
            if(bWriteRecord)
            {
                if(mStdfRecordHeader.iRecordType == 1 && mStdfRecordHeader.iRecordSubType == 20 && iCpuType != ptFile->hStdf->GetStdfCpuType())
                {
                    //ptFile->hStdf->SetStdfCpuType();
                    mStdfFile.SetStdfCpuType(ptFile->hStdf->GetStdfCpuType());
                    mStdfFile.DumpRecord(&mStdfRecordHeader,ptFile->hStdf);
                    mStdfFile.SetStdfCpuType(iCpuType);
                }
                else
                    mStdfFile.DumpRecord(&mStdfRecordHeader,ptFile->hStdf);
            }

            // Read one record from STDF file.
            iStatus = ptFile->hStdf->LoadRecord(&mStdfRecordHeader);
        };

        if (iStatus == GS::StdLib::Stdf::EndOfFile)
            iStatus = NoError;

        // Close input 'Test' STDF file
        ptFile->hStdf->Close();	// Clean close.

        // Increment File index
        iFileIndex++;
    };

end_sample_merge:
    // Close STDF file
    mStdfFile.Close();

    // Destroy all temporary structures allocated.
    qDeleteAll(cFilesList);

    // Delete structure holding all merged binnings (test + retest)
    pMergeBinning->hStdf = NULL;	// MANDATORY: because this was only a copy to STDF class deleted above!
    delete pMergeBinning;
    pMergeBinning=0;

    return iStatus;
}

//This method is used to merge multiple WRR
//with the assumption that : Final part count = Sum of all (part count)
//with the assumption that : Final good count = Sum of all (good count)
//Return true if the merge is done
bool GexTbMergeRetest::MergeWRR(GexTbMergeFileInfo& dataFile, GQTL_STDF::Stdf_WRR_V4& wrrRecord, int *piError)
{
    switch(mStdfRecordHeader.iStdfVersion)
    {
        case GEX_STDFV4:
        {
            if(!wrrRecord.Read(*dataFile.hStdf))
            {
                mErrorMessage = QString("Failed to read WRR record at record %1 in file %2")
                                    .arg(dataFile.hStdf->GetReadRecordNumber()).arg(dataFile.strFileName);

                GSLOG(SYSLOG_SEV_ERROR, mErrorMessage.toLatin1().constData());
                return false;
            }

            QString strWID = wrrRecord.m_cnWAFER_ID;

            if(dataFile.m_mapWRRMerge.isEmpty())
            {
                dataFile.m_mapWRRMerge.insert(strWID, GQTL_STDF::Stdf_WRR_V4());
                dataFile.m_mapWRRMerge[strWID].SetPART_CNT(0);
                dataFile.m_mapWRRMerge[strWID].SetGOOD_CNT(0);
                dataFile.m_mapWRRMerge[strWID].SetRTST_CNT(0);
                dataFile.m_mapWRRMerge[strWID].SetABRT_CNT(0);
                dataFile.m_mapWRRMerge[strWID].SetFUNC_CNT(0);
            }
            else
            {
                if(strWID != dataFile.m_mapWRRMerge.keys().first())
                {
                    if(mCalledFromGui)
                    {
                        QMessageBox oMergeBox;
                        oMergeBox.setWindowTitle(GS::Gex::Engine::GetInstance().Get("AppFullName").toString());
                        oMergeBox.setWindowIcon(QPixmap(":/gex/icons/gex_application_48x48.png"));
                        oMergeBox.setText("The data files to merge contains different WaferID, all data will be merged into a single WIR/WRR.");
                        oMergeBox.addButton("Continue", QMessageBox::YesRole);
                        QPushButton *poAbort = oMergeBox.addButton("Abort", QMessageBox::RejectRole);
                        oMergeBox.exec();

                        QAbstractButton *poUserChoice = oMergeBox.clickedButton();
                        if(!poUserChoice || poUserChoice == poAbort)
                        {
                            if(piError)
                                (*piError) = 1;

                            mErrorMessage = QString("Data files to merge contains different WaferID (%1 and %2)")
                                                .arg(strWID).arg(dataFile.m_mapWRRMerge.keys().first());

                            GSLOG(SYSLOG_SEV_ERROR, mErrorMessage.toLatin1().constData());
                            return false;
                        }
                    }
                    else
                    {
                        if(piError)
                            (*piError) = 1;

                        mErrorMessage = QString("Data files to merge contains different WaferID (%1 and %2)")
                                            .arg(strWID).arg(dataFile.m_mapWRRMerge.keys().first());

                        GSLOG(SYSLOG_SEV_ERROR, mErrorMessage.toLatin1().constData());
                        return false;
                    }
                }
            }

            strWID = (dataFile.m_mapWRRMerge.keys().isEmpty()) ? "" : dataFile.m_mapWRRMerge.keys().first();

            if(wrrRecord.IsFieldValid(GQTL_STDF::Stdf_WRR_V4::eposPART_CNT) &&
                    dataFile.m_mapWRRMerge[strWID].IsFieldValid(GQTL_STDF::Stdf_WRR_V4::eposPART_CNT))
            {
                dataFile.m_mapWRRMerge[strWID].m_u4PART_CNT += wrrRecord.m_u4PART_CNT;
            }
            else
            {
                dataFile.m_mapWRRMerge[strWID].invalidateField(GQTL_STDF::Stdf_WRR_V4::eposPART_CNT);
                dataFile.m_mapWRRMerge[strWID].m_u4PART_CNT = INVALID_INT;
            }

            if(wrrRecord.IsFieldValid(GQTL_STDF::Stdf_WRR_V4::eposGOOD_CNT) &&
                    dataFile.m_mapWRRMerge[strWID].IsFieldValid(GQTL_STDF::Stdf_WRR_V4::eposGOOD_CNT))
            {
                dataFile.m_mapWRRMerge[strWID].m_u4GOOD_CNT += wrrRecord.m_u4GOOD_CNT;
            }
            else
            {
                dataFile.m_mapWRRMerge[strWID].invalidateField(GQTL_STDF::Stdf_WRR_V4::eposGOOD_CNT);
                dataFile.m_mapWRRMerge[strWID].m_u4GOOD_CNT = INVALID_INT;
            }

            if(wrrRecord.IsFieldValid(GQTL_STDF::Stdf_WRR_V4::eposRTST_CNT) &&
                    dataFile.m_mapWRRMerge[strWID].IsFieldValid(GQTL_STDF::Stdf_WRR_V4::eposRTST_CNT))
            {
                dataFile.m_mapWRRMerge[strWID].m_u4RTST_CNT += wrrRecord.m_u4RTST_CNT;
            }
            else
            {
                dataFile.m_mapWRRMerge[strWID].invalidateField(GQTL_STDF::Stdf_WRR_V4::eposRTST_CNT);
                dataFile.m_mapWRRMerge[strWID].m_u4RTST_CNT = INVALID_INT;
            }

            if(wrrRecord.IsFieldValid(GQTL_STDF::Stdf_WRR_V4::eposABRT_CNT) &&
                    dataFile.m_mapWRRMerge[strWID].IsFieldValid(GQTL_STDF::Stdf_WRR_V4::eposABRT_CNT))
            {
                dataFile.m_mapWRRMerge[strWID].m_u4ABRT_CNT += wrrRecord.m_u4ABRT_CNT;
            }
            else
            {
                dataFile.m_mapWRRMerge[strWID].invalidateField(GQTL_STDF::Stdf_WRR_V4::eposABRT_CNT);
                dataFile.m_mapWRRMerge[strWID].m_u4ABRT_CNT = INVALID_INT;
            }

            if(wrrRecord.IsFieldValid(GQTL_STDF::Stdf_WRR_V4::eposFUNC_CNT) &&
                    dataFile.m_mapWRRMerge[strWID].IsFieldValid(GQTL_STDF::Stdf_WRR_V4::eposFUNC_CNT))
            {
                dataFile.m_mapWRRMerge[strWID].m_u4FUNC_CNT += wrrRecord.m_u4FUNC_CNT;
            }
            else
            {
                dataFile.m_mapWRRMerge[strWID].invalidateField(GQTL_STDF::Stdf_WRR_V4::eposFUNC_CNT);
                dataFile.m_mapWRRMerge[strWID].m_u4FUNC_CNT = INVALID_INT;
            }

            dataFile.m_u4FINISH_T = wrrRecord.m_u4FINISH_T;
        }
            break;

        default:
            //add an error message
            break;
    }

    return true;

}

bool GexTbMergeRetest::WriteWRR(GexTbMergeFileInfo &dataFile, GQTL_STDF::Stdf_WRR_V4 &wrrRecord)
{
    QString strWID = dataFile.m_mapWRRMerge.keys().first();

    if(dataFile.m_mapWRRMerge[strWID].IsFieldValid(GQTL_STDF::Stdf_WRR_V4::eposPART_CNT))
        wrrRecord.SetPART_CNT(dataFile.m_mapWRRMerge[strWID].m_u4PART_CNT);
    else
    {
        wrrRecord.invalidateField(GQTL_STDF::Stdf_WRR_V4::eposPART_CNT);
        wrrRecord.m_u4PART_CNT = INVALID_INT;
    }

    if(dataFile.m_mapWRRMerge[strWID].IsFieldValid(GQTL_STDF::Stdf_WRR_V4::eposGOOD_CNT))
        wrrRecord.SetGOOD_CNT(dataFile.m_mapWRRMerge[strWID].m_u4GOOD_CNT);
    else
    {
        wrrRecord.invalidateField(GQTL_STDF::Stdf_WRR_V4::eposGOOD_CNT);
        wrrRecord.m_u4GOOD_CNT = INVALID_INT;
    }

    if(dataFile.m_mapWRRMerge[strWID].IsFieldValid(GQTL_STDF::Stdf_WRR_V4::eposRTST_CNT))
        wrrRecord.SetRTST_CNT(dataFile.m_mapWRRMerge[strWID].m_u4RTST_CNT);
    else
    {
        wrrRecord.invalidateField(GQTL_STDF::Stdf_WRR_V4::eposRTST_CNT);
        wrrRecord.m_u4RTST_CNT = INVALID_INT;
    }

    if(dataFile.m_mapWRRMerge[strWID].IsFieldValid(GQTL_STDF::Stdf_WRR_V4::eposABRT_CNT))
        wrrRecord.SetABRT_CNT(dataFile.m_mapWRRMerge[strWID].m_u4ABRT_CNT);
    else
    {
        wrrRecord.invalidateField(GQTL_STDF::Stdf_WRR_V4::eposABRT_CNT);
        wrrRecord.m_u4ABRT_CNT = INVALID_INT;
    }

    if(dataFile.m_mapWRRMerge[strWID].IsFieldValid(GQTL_STDF::Stdf_WRR_V4::eposFUNC_CNT))
        wrrRecord.SetFUNC_CNT(dataFile.m_mapWRRMerge[strWID].m_u4FUNC_CNT);
    else
    {
        wrrRecord.invalidateField(GQTL_STDF::Stdf_WRR_V4::eposFUNC_CNT);
        wrrRecord.m_u4FUNC_CNT = INVALID_INT;
    }

    wrrRecord.SetFINISH_T(dataFile.m_u4FINISH_T);
    wrrRecord.Write(mStdfFile);
    return true;
}


int GexTbMergeRetest::merge (QStringList strTest, QStringList strReTest,
                             QString strOutputFile, bool bSort, QString strSwap)
{
    mErrorMessage.clear();

    if(GS::LPPlugin::ProductInfo::getInstance()->isNotSupportedCapability(GS::LPPlugin::ProductInfo::toolbox))
    {
        // OEM mode, refuse to run this function!
        GS::Gex::Message::information("", "This function is disabled in teradyne mode\n\nContact " +
                                      QString(GEX_EMAIL_SALES) + " for more information!");
        mErrorMessage = "This function is disabled in teradyne mode";
        return LicenseError;
    }

    if(strTest.isEmpty() || strReTest.isEmpty() )
        return GexTbMergeRetest::NoFile;

    if(strOutputFile.isEmpty())
    {
        strOutputFile = (strTest.isEmpty()) ? "" : strTest.first();
        strOutputFile = strOutputFile + "_merged.std";
    }

    bool bIsGDF = false;
    SetGDFFile(bIsGDF);

    QString strMergedTestFile = strTest[0] + "_test_galaxy.stdf";
    int lRet = MergeSamplesFiles(strTest,strMergedTestFile,true,bSort);
    if (lRet != GexTbMergeRetest::NoError)
    {
        return lRet;
    }

    bIsGDF = (bIsGDF || IsGDFFile());

    QString strMergedRetestFile = strReTest[0] + "_retest_galaxy.stdf";
    lRet = MergeSamplesFiles(strReTest,strMergedRetestFile,true,bSort);
    if (lRet != GexTbMergeRetest::NoError)
    {
        return lRet;
    }
    bIsGDF = (bIsGDF || IsGDFFile());

    QStringList strSources;
    strSources << strMergedTestFile << strMergedRetestFile;

    lRet = loadFilesToMerge(strSources);
    if(lRet != GexTbMergeRetest::NoError)
        return lRet;

    if(GetTestData()->lTotalParts < GetReTestData()->lTotalParts)
    {
        if(strSwap == "swap")
        {
            swapTestReTest();
        }
        else if(strSwap == "cancel")
        {
            mErrorMessage = "Merge Cancelled";
            ExitCleanup();
            return GexTbMergeRetest::NoFile;
        }
    }

    SetGDFFile(bIsGDF);
    lRet = MergeFiles(strSources,strOutputFile);

    unlink(strMergedTestFile.toLatin1().constData());		// File xx_test_galaxy.stdf
    unlink(strMergedRetestFile.toLatin1().constData());	// File xx_retest_galaxy.stdf

    // Check Status and display message accordingly.
    if(lRet == GexTbMergeRetest::NoError)
    {
        mErrorMessage = "Merge successful!\nFile created: " + strOutputFile;
    }

    return lRet;
}

bool GexTbMergeRetest::IsGDFMIR(GS::StdLib::Stdf *pStdf)
{
    BYTE bByte;
    int iWord;
    long lWord;
    char	szString[257];
    pStdf->ReadDword(&lWord);		// Overwrite Setup_T
    pStdf->ReadDword(&lWord);		// Overwrite Start_T
    pStdf->ReadByte(&bByte);
    pStdf->ReadByte(&bByte);
    pStdf->ReadByte(&bByte);
    pStdf->ReadByte(&bByte);
    pStdf->ReadWord(&iWord);
    pStdf->ReadByte(&bByte);
    pStdf->ReadString(szString);	// Lot ID
    pStdf->ReadString(szString);// Part Type / Product ID
    pStdf->ReadString(szString);// Node name
    pStdf->ReadString(szString);// Tester Type
    pStdf->ReadString(szString);// Job name
    pStdf->ReadString(szString);// job_rev
    pStdf->ReadString(szString);// sblot_id
    pStdf->ReadString(szString);// oper_nam
    pStdf->ReadString(szString);// exec_typ
    pStdf->ReadString(szString);// exec_ver
    pStdf->ReadString(szString);// test_cod
    pStdf->ReadString(szString);// tst_temp

    pStdf->ReadString(szString);// user-txt
    QString	strUserTxt = szString;

    // Construct custom Galaxy USER_TXT
    QString strGDFSignature = GEX_IMPORT_DATAORIGIN_LABEL;
    strGDFSignature += ":";
    strGDFSignature += GEX_IMPORT_DATAORIGIN_ATETEST;
    strGDFSignature += ":HP93K_HP83K";

    return strUserTxt.contains(strGDFSignature);
}

///////////////////////////////////////////////////////////
// Create intermediate input STDF file if needed
///////////////////////////////////////////////////////////
int	GexTbMergeRetest::ConvertToSTDF(GexTbMergeFileInfo *pDataFile,QString strInputFile)
{
    GS::Gex::ConvertToSTDF StdfConvert;
    pDataFile->strFileName = strInputFile;
    pDataFile->strFileNameSTDF = strInputFile + "_galaxy.std";
    int nConvertStatus = StdfConvert.Convert(false, true, false, true, strInputFile, pDataFile->strFileNameSTDF,"",
                                             pDataFile->bFileCreated, mErrorMessage);
    if(nConvertStatus == GS::Gex::ConvertToSTDF::eConvertError)
    {
        // Exit cleanup: erase temporary files.
        ExitCleanup();

        mErrorMessage = "Failed parsing data file. File may be corrupted:\n" + strInputFile;
        return ReadError;
    }

    // Check if Input file was already in STDF format!
    if(pDataFile->bFileCreated == false)
        pDataFile->strFileNameSTDF = strInputFile;

    return NoError;
}

///////////////////////////////////////////////////////////
// Create intermediate input STDF file if needed
///////////////////////////////////////////////////////////
int	GexTbMergeRetest::ConvertToSTDF(GexTbMergeSampleFileInfo *pDataFile,QString strInputFile)
{
    GS::Gex::ConvertToSTDF StdfConvert;
    pDataFile->strFileName = strInputFile;
    pDataFile->strFileNameSTDF = strInputFile + "_galaxy.std";
    int nConvertStatus = StdfConvert.Convert(false, true, false, true, strInputFile, pDataFile->strFileNameSTDF,"",pDataFile->bFileCreated, mErrorMessage);
    if(nConvertStatus == GS::Gex::ConvertToSTDF::eConvertError)
    {
        // Exit cleanup: erase temporary files.
        ExitCleanup();

        mErrorMessage = "Failed parsing data file (if archive file,\ncheck it only holds one file). File may be corrupted:\n" + strInputFile;
        return ReadError;
    }

    // Check if Input file was already in STDF format!
    if(pDataFile->bFileCreated == false)
        pDataFile->strFileNameSTDF = strInputFile;

    return NoError;
}
