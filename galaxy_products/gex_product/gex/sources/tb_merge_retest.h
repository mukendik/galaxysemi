#ifndef TB_MERGE_RETEST_H
#define TB_MERGE_RETEST_H

#include <QObject>
#include <QStringList>
#include <stdf.h>
#include <stdfrecords_v4.h>
#include "bin_info.h"

// For merging test + retest and update summary / HBR/SBR
// add new attributes to handle WRR Merging
class GexTbMergeFileInfo
{
public:
    GexTbMergeFileInfo();	// Constructor
    ~GexTbMergeFileInfo();	// Constructor

    // Set/Get
    void	SetSetupTime(long lSetupTime) { m_lSetupTime = lSetupTime; }
    void	SetStartTime(long lStartTime) { m_lStartTime = lStartTime; }
    void	SetFinishTime(long lFinishTime) { m_lFinishTime = lFinishTime; }
    long	GetSetupTime() { return m_lSetupTime; }
    long	GetStartTime() { return m_lStartTime; }
    long	GetFinishTime() { return m_lFinishTime; }
    long	GetTestDuration() { return (m_lFinishTime-m_lStartTime); }

    QString	strFileName;		 // Input file name
    QString	strFileNameSTDF;	 // STDF file name created if input file is not in STDF format
    bool	bFileCreated;		 // set to 'true' if an intermediate STDF file is created
    GS::StdLib::Stdf	*hStdf;				// Handle to STDF file to read
    QMap <int,int>	map_SoftBin;            // List of Software Binning
    QMap <int,BYTE> map_SoftBinCat;         // List of Software Binning Category from SBR
    QMap <int,BYTE>	map_SoftBinCatFromPRR;  // List of Software Binning Category from PRR
    QMap <int,int>	map_SoftBinMerged;      // List of Software Binning specified as Merged sites (site=255).
    QMap <int,QString>	map_SoftBinNames;   // List of Software Binning Names
    bool	bUseSoftBinMerged;	// 'true' if we'll use the SoftBin merger list 'false' if we use the non-merged
    QMap <int,int>	map_HardBin;            // List of Software Binning
    QMap <int,BYTE> map_HardBinCat;         // List of Software Binning Category from HBR
    QMap <int,BYTE>	map_HardBinCatFromPRR;  // List of Software Binning Category from PRR
    QMap <int,int>	map_HardBinMerged;      // List of Software Binning specified as Merged sites (site=255).
    QMap <int,QString>	map_HardBinNames;   // List of Hardware Binning Names
    bool	bUseHardBinMerged;	// 'true' if we'll use the SoftBin merger list 'false' if we use the non-merged
    // All following variables should be equal if the data file is correct...but reality is that they never!
    long	lTotalSoftBins;			// Total parts tested
    long	lTotalSoftBinsMerged;	// Total parts tested
    long	lTotalHardBins;			// Total parts tested
    long	lTotalHardBinsMerged;	// Total parts tested

    long	lTotalParts;			// total parts tested.

    QMap<QString , GQTL_STDF::Stdf_WRR_V4> m_mapWRRMerge;//Map reflecting the Pair(Parts Count, Good Count) for each wafer;
    unsigned long m_u4FINISH_T; // This value stores the last finish time of the test/retest record


private:
    // Start and end test times
    long	m_lSetupTime;
    long	m_lStartTime;
    long	m_lFinishTime;
};

// For merging N files (only merge sample records, ignore all summary and HBR/SBR)
class GexTbMergeSampleFileInfo
{
public:
    GexTbMergeSampleFileInfo();	// Constructor
    ~GexTbMergeSampleFileInfo();// Destructor
    QString	strFileName;		// Input file name
    QString	strFileNameSTDF;	// STDF file name created if input file is not in STDF format
    bool	bFileCreated;		// set to 'true' if an intermediate STDF file is created
    GS::StdLib::Stdf	*hStdf;				// Handle to STDF file to read
    unsigned long lStartTime;	// File (MIR start testing time)
};

class GexTbMergeRetest : public QObject
{
    Q_OBJECT

public:
    // Error codes returned by Stdf functions
    enum MergeErrorCode
    {
        NoError,		// No error
        NoFile,			// Input file doesn't exist!
        ReadError,		// Failed reading files to merge
        ReadCorrupted,	// Input file (in STDF format) didn't read well. Probably corrupted.
        WriteSTDF,		// Failed creating output STDF file
        BinStatusMissing,// Missing bin status
        LicenseError
    };

    GexTbMergeRetest(bool bGui);
    ~GexTbMergeRetest();

    // ?
    int     MergeFiles(QStringList inputs, QString output);
    // ?
    int     MergeSamplesFiles(QStringList strSources, QString strOutput,
                              bool bRebuildHbrSbr, bool bSortByDate);
    // ?
    int     merge(QStringList strTest, QStringList strReTest, QString strOutputFile,
                  bool bSort, QString strSwap);
    // ?
    int     loadFilesToMerge(QStringList strSources);

    void    swapTestReTest();


    QString                 GetErrorMessage() const;
    GexTbMergeFileInfo *    GetTestData()  const    { return mTestData; }
    GexTbMergeFileInfo *    GetReTestData() const   { return mReTestData; }
    bool                    IsGDFFile() const       { return mGDFFileDetected; }
    void                    SetGDFFile(bool bVal)   { mGDFFileDetected = bVal; }

    void                    ExitCleanup();

protected:

    bool    MergeWRR(GexTbMergeFileInfo &dataFile, GQTL_STDF::Stdf_WRR_V4 &wrrRecord, int *piError);
    bool    WriteWRR(GexTbMergeFileInfo &dataFile, GQTL_STDF::Stdf_WRR_V4 &wrrRecord);
    int		ConvertToSTDF(GexTbMergeFileInfo *pDataFile, QString strInputFile);
    int		ConvertToSTDF(GexTbMergeSampleFileInfo *pDataFile, QString strInputFile);
    int		ReadInputFileSummaryInfo(GexTbMergeFileInfo *pDataFile);
    int		CheckUpdateBinningTotals(GexTbMergeFileInfo *pDataFile);
    int     WriteMergedBinningTotal(GexTbMergeFileInfo *pDataFile);
    bool    FillBinningList(const QMap<int, int>& lBinCount, const QMap<int, BYTE>& lBinCat,
                            const QMap<int, BYTE> &lBinCatFromPRR, const QMap<int, QString>& lBinName,
                            bool lPassOnly, QMap<int, BinInfo>& lBinList);
    int     ReadInputTestTimeInfo(GexTbMergeSampleFileInfo *pDataFile);
    int     ReadPRR(GexTbMergeFileInfo &lTestFile, GS::StdLib::Stdf &lStdf);
    void    ReadBinning(GexTbMergeFileInfo *pDataFile,bool bSoftBin);
    void    OverloadTSR(GexTbMergeSampleFileInfo *pInDataFile);
    void    WriteBinList(int iStdfRecordSubType, const QMap<int, BinInfo> &cMabBinList);
    void    WritePCR(const QMap<int, BinInfo> &cMabBinList);
    bool    IsGDFMIR(GS::StdLib::Stdf *pStdf);

private:

    GexTbMergeFileInfo  *           mTestData;
    GexTbMergeFileInfo	*           mReTestData;
    GS::StdLib::Stdf                mStdfFile;
    GS::StdLib::StdfRecordReadInfo  mStdfRecordHeader;
    bool                            mGDFFileDetected;
    bool                            mCalledFromGui;
    QString                         mErrorMessage;
};

#endif // TB_MERGE_RETEST_H
