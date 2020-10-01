
//////////////////////////////////////////////////////////////////////
// importSiTimeEtest.cpp: Convert a SiTimeEtest file to STDF V4.0
//////////////////////////////////////////////////////////////////////

#include <math.h>
#include <qmath.h>
#include <time.h>
#include <QFile>
#include <QDir>


#include "importSiTimeEtest.h"
//#include "import_constants.h"
//#include "gex_algorithms.h"
#include "gqtl_log.h"

// File format:
//Wafer	Site	LK_RT_EC_01_A	LK_RT_EC_01_B	LK_RT_EC_01_C	LK_RT_EC_01_D	LK_RT_EC_07_A	LK_RT_EC_07_B	LK_RT_EC_07_C	LK_RT_EC_07_D	MinOf_RT_EC_01	MaxZu_RT_EC_01	MinOf_RT_EC_01_07	MaxZu_RT_EC_01_07	Delta_RT_EC_01_07	LK_RT_EC_03_A	LK_RT_EC_03_B	LK_RT_EC_03_C	LK_RT_EC_03_D	MinOf_RT_EC_03	MaxZu_RT_EC_03	Delta_RT_EC_03	LK_RT_EC_02_A	LK_RT_EC_02_B	LK_RT_EC_02_C	LK_RT_EC_02_D	LK_RT_EC_08_A	LK_RT_EC_08_B	LK_RT_EC_08_C	LK_RT_EC_08_D	MinOf_RT_EC_02	MaxZu_RT_EC_02	MinOf_RT_EC_02_08	MaxZu_RT_EC_02_08	Delta_RT_EC_02_08	LK_RT_EC_04_A	LK_RT_EC_04_B	LK_RT_EC_04_C	LK_RT_EC_04_D	MinOf_RT_EC_04	MaxZu_RT_EC_04	Delta_RT_EC_04	V24_RT_WB_01	V13_RT_WB_01	Calc_CD_WB_1	V24_RT_WB_02	V13_RT_WB_02	Calc_CD_WB_2	RHO_RT_FP_02	RHO_RT_FP_04	LK_RT_FP_02	LK_RT_FP_04	BV_RT_PI_01	BV_RT_PI_02	BV_RT_PI_03	CR_C1_CT_01	CR_C1_CT_03	CR_C1_CT_04	CR_C1_CC_01	LK_C1_EC_01_A	LK_C1_EC_01_B	LK_C1_EC_01_C	RHO_IT_FP_02	LK_IT_EC_02_375	LK_IT_EC_02_400	LK_IT_EC_02_425	LK_IT_EC_02_450	LK_IT_EC_01_475	LK_IT_EC_01_500	LK_IT_EC_01_525	LK_IT_EC_01_550	MinOf_IT_EC_01	MaxZu_IT_EC_01	MinOf_IT_EC_375_550	MaxZu_IT_EC_375_550	Delta_IT_EC_375_550	CR_CM_CT_01	CR_CM_CT_03	CR_CM_CT_02	CR_CM_CT_04	CR_CM_CC_01	LK_CM_EC_01_A	LK_CM_EC_01_B	LK_CM_EC_01_C	RHO_ME_FP_01	RHO_ME_FP_02	LK_ME_IS_02	RHO_ME_SC_02	Frequenz	G�te	Winkel	Bandwidth
//1	1	 0.000	 0.001	 0.000	 2.000	 2.000	 2.000	 2.000	 2.000	 320.0	 300.0	 320.0	 300.0	 20.00	 0.000	 2.000	 2.000	 2.000	 340.0	 300.0	 40.00	 0.000	 0.000	 0.000	 2.000	 2.000	 2.000	 2.000	 2.000	 320.0	 300.0	 320.0	 300.0	 20.00	 0.000	 2.000	 2.000	 2.000	 340.0	 300.0	 40.00	-17.36	 469.0	 206.8	-83.16	 0.984	 216.9	 49.70	 47.08	-34.30	-54.47	 60.00	 95.50	 100.0	 82.41	 106.8	 24.63	 133.4	 2.000	 2.000	 2.000	 39.42	 2.000	 0.000	 0.000	 0.000	 0.001	 0.000	 0.000	 0.000	 475.0	Can'tC	 400.0	 375.0	 25.00	 31.13	 28.36	 55.84	 11.03	 73.22	 2.000	 2.000	 0.000	 41.25	 40.84	 0.098	 40.10	 523.8	 41.88	 30.62	 12.51
//1	2	 0.000	 0.000	 0.001	 2.000	 2.000	 2.000	 2.000	 2.000	 320.0	 300.0	 320.0	 300.0	 20.00	 0.000	 2.000	 2.000	 2.000	 340.0	 300.0	 40.00	 0.000	 0.000	 0.000	 2.000	 2.000	 2.000	 2.000	 2.000	 320.0	 300.0	 320.0	 300.0	 20.00	 0.000	 2.000	 2.000	 2.000	 340.0	 300.0	 40.00	-18.41	 467.5	 219.0	-88.86	 0.989	 228.5	 49.22	 46.89	-30.05	-38.02	 62.00	 100.0	 100.0	 82.62	 111.5	 24.97	 133.4	 2.000	 2.000	 2.000	 37.70	 2.000	 0.000	 0.000	 0.000	 0.001	 0.000	 0.000	 0.001	 475.0	Can'tC	 400.0	 375.0	 25.00	 27.31	 28.36	 55.81	 11.57	 74.33	 2.000	 2.000	 0.000	 41.99	 41.25	 0.094	 41.27	 523.8	 41.27	 28.50	 12.69
//1	3	 0.000	 0.000	 2.000	 2.000	 2.000	 2.000	 2.000	 2.000	 340.0	 320.0	 340.0	 320.0	 20.00	 2.000	 2.000	 2.000	 2.000	Can'tC	 340.0	Can'tC	 0.000	 0.000	 2.000	 2.000	 2.000	 2.000	 2.000	 2.000	 340.0	 320.0	 340.0	 320.0	 20.00	 2.000	 2.000	 2.000	 2.000	Can'tC	 340.0	Can'tC	-15.56	 458.8	 190.5	-73.73	 0.957	 200.3	 48.93	 46.44	-35.16	-50.30	 58.00	 93.50	 100.0	 82.80	 110.6	 24.70	 134.2	 2.000	 2.000	 2.000	 39.51	 2.000	 0.000	 0.000	 0.001	 0.001	 0.001	 0.001	 0.000	 475.0	Can'tC	 400.0	 375.0	 25.00	 30.09	 32.41	 49.76	 11.72	 73.96	 2.000	 2.000	 2.000	 42.47	 41.96	 0.056	 41.14	 524.3	 40.63	 33.97	 12.90
//1	4	 0.000	 0.001	 0.000	 0.000	 2.000	 2.000	 2.000	 2.000	 300.0	Can'tC	 300.0	 280.0	 20.00	 0.000	 2.000	 2.000	 2.000	 340.0	 300.0	 40.00	 0.001	 0.000	 0.000	 0.000	 2.000	 2.000	 2.000	 2.000	 300.0	Can'tC	 300.0	 280.0	 20.00	 0.000	 0.000	 2.000	 2.000	 300.0	 260.0	 40.00	-18.60	 474.7	 218.0	-118.4	 1.047	 276.8	 50.17	 47.49	-42.34	-32.95	 62.50	 100.0	 100.0	 82.42	 111.3	 24.90	 134.2	 2.000	 2.000	 2.000	 39.41	 2.000	 0.000	 0.000	 0.001	 0.000	 0.000	 0.000	 0.001	 475.0	Can'tC	 400.0	 375.0	 25.00	 29.84	 31.08	 50.86	 11.78	 74.70	 2.000	 2.000	 2.000	 42.38	 41.63	 0.068	 41.66	 523.4	 42.77	 27.48	 12.24


namespace GS
{
namespace Parser
{
#define BIT0            0x01
#define BIT1            0x02
#define BIT2            0x04
#define BIT3            0x08
#define BIT4            0x10
#define BIT5            0x20
#define BIT6            0x40
#define BIT7            0x80

#define PARAMETER_OFFSET    2


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
SiTimeEtesttoSTDF::SiTimeEtesttoSTDF() : ParserBase(typeSiTimeEtest, "SiTimeEtest")
{
    mOutputFiles.clear();
    mParameterList.clear();
    mParameterDirectory.SetFileName(GEX_SITIME_ETEST_PARAMETERS);
    mTotalParameters = 0;
    mTotalColumns = 0;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
SiTimeEtesttoSTDF::~SiTimeEtesttoSTDF()
{
    mParameterList.clear();
    mOutputFiles.clear();
}


//std::list<std::string> SiTimeEtesttoSTDF::GetListStdfFiles() const
//{
//    std::list<std::string> lOutputFiles;

//    for (int lItem=0; lItem < mOutputFiles.count(); ++lItem)
//    {
//        lOutputFiles.push_back(mOutputFiles[lItem].toStdString());
//    }

//    return lOutputFiles;
//}

//bool SiTimeEtesttoSTDF::RemoveOutputFiles()
//{
//    for(unsigned short i=0; i<mOutputFiles.size(); ++i)
//    {
//        QString lFile = mOutputFiles[i];
//        if (!QFile::remove(lFile))
//        {
//            GSLOG(SYSLOG_SEV_ERROR, QString("Impossible to remove the output file %1")
//                  .arg(lFile).toLatin1().constData());
//            return false;
//        }
//    }
//    return true;
//}


//////////////////////////////////////////////////////////////////////
// Check if File is compatible with SiTime format
//////////////////////////////////////////////////////////////////////
bool SiTimeEtesttoSTDF::IsCompatible(const QString& fileName)
{
    QString lString;
    gsbool lCompatible(false);

    // Open hCsmFile file
    QFile lFile(fileName);
    if(!lFile.open( QIODevice::ReadOnly))
    {
        // Failed Opening the input file
        return false;
    }

    // First check the file name
    QString lBaseName = QFileInfo(fileName).fileName();
    if (fileName.endsWith(".csv")
            && lBaseName.split("_").size() == 2)
    {
        QTextStream lInputStreamFile(&lFile);

        // Check if first line is the correct SiTime header...
        // Wafer,Site,LK_RT_EC_01_A,LK_RT_EC_01_B,LK_RT_EC_01_C,LK_RT_EC_01_D,...,Frequenz,G�te,Winkel,Bandwidth
        do
            lString = lInputStreamFile.readLine().simplified();
        while(!lString.isNull() && lString.isEmpty());

        if(!lString.startsWith("Wafer,Site,", Qt::CaseInsensitive))
        {
            // Incorrect header...this is not a SiTime file!
            // Close file
            lFile.close();
            return false;
        }
        // Close file
        lFile.close();
        lCompatible = true;
    }

    return lCompatible;
}

//////////////////////////////////////////////////////////////////////
// Read and Parse the SiTime file
//////////////////////////////////////////////////////////////////////
bool SiTimeEtesttoSTDF::ReadSiTimeFile(const QString& SiTimeFileName,const QString &outputSTDF)
{
    QString lString;
    QString lSection;

    // Open SiTime file
    QFile lFile( SiTimeFileName );
    if(!lFile.open( QIODevice::ReadOnly ))
    {
        // Failed Opening SiTime file
        mLastError = errOpenFail;

        // Convertion failed.
        return false;
    }
    // Assign file I/O stream
    QTextStream lSiTimeFileStream(&lFile);
    QStringList lLstSections;


    // Check if first line is the correct SiTime header...
    // Wafer,Site,LK_RT_EC_01_A,LK_RT_EC_01_B,LK_RT_EC_01_C,LK_RT_EC_01_D,...,Frequenz,G�te,Winkel,Bandwidth
    //1	1	 0.000	 0.001	 0.000	 2.000	 2.000	 2.000	 2.000	 2.000	 320.0	 300.0	 320.0	 300.0	 20.00	 0.000	 2.000	 2.000	 2.000	 340.0	 300.0	 40.00	 0.000	 0.000	 0.000	 2.000	 2.000	 2.000	 2.000	 2.000	 320.0	 300.0	 320.0	 300.0	 20.00	 0.000	 2.000	 2.000	 2.000	 340.0	 300.0	 40.00	-17.36	 469.0	 206.8	-83.16	 0.984	 216.9	 49.70	 47.08	-34.30	-54.47	 60.00	 95.50	 100.0	 82.41	 106.8	 24.63	 133.4	 2.000	 2.000	 2.000	 39.42	 2.000	 0.000	 0.000	 0.000	 0.001	 0.000	 0.000	 0.000	 475.0	Can'tC	 400.0	 375.0	 25.00	 31.13	 28.36	 55.84	 11.03	 73.22	 2.000	 2.000	 0.000	 41.25	 40.84	 0.098	 40.10	 523.8	 41.88	 30.62	 12.51

    lString = ReadLine(lSiTimeFileStream).simplified();
    if(!lString.startsWith("Wafer,Site,", Qt::CaseInsensitive))
    {
        // Incorrect header...this is not a SiTime file!
        mLastError = errInvalidFormatParameter;

        // Convertion failed.
        // Close file
        lFile.close();
        return false;
    }

    // Read SiTime information
    lLstSections = lString.split(",", QString::KeepEmptyParts);

    // Count the number of parameters specified in the line
    // Do not count first fields
    mTotalColumns=lLstSections.count();
    mTotalParameters=mTotalColumns - PARAMETER_OFFSET;
    // If no parameter specified...ignore!
    if(mTotalParameters <= 0)
    {
        // Incorrect header...this is not a valid Exar - PCM file!
        mLastError = errInvalidFormatParameter;

        // Convertion failed.
        // Close file
        lFile.close();
        return false;
    }

    // Extract the N column names
    ParserParameter lParam;
    for (int lIndex=PARAMETER_OFFSET; lIndex<mTotalColumns; ++lIndex)
    {
        lSection = lLstSections[lIndex].trimmed();	// Remove spaces
        lParam.SetTestNumber(mParameterDirectory.UpdateParameterIndexTable(lSection));
        lParam.SetTestName(lSection);
        mParameterList.append(lParam);
    }

    // Extarct the lot and the product from the file name
    QString lBaseName = QFileInfo(outputSTDF).baseName();
    if(lBaseName.split("_").size() == 2)
    {
        mProductId= lBaseName.section(".", 0,0).section("_", 1,1);
        mLotId= lBaseName.section(".", 0,0).section("_", 0,0);
    }

    if(!WriteStdfFile(&lSiTimeFileStream,outputSTDF))
    {
//        RemoveOutputFiles();
        QFile::remove(outputSTDF);
        return false;
    }

    // Close file
    lFile.close();

    // All SiTime file read...check if need to update the SiTime Parameter list on disk?
    if(mParameterDirectory.GetNewParameterFound() == true)
        mParameterDirectory.DumpParameterIndexTable();

    // Success parsing SiTime file
    return true;
}

//////////////////////////////////////////////////////////////////////
// Create STDF file from SiTime data parsed
//////////////////////////////////////////////////////////////////////
bool SiTimeEtesttoSTDF::WriteStdfFile(QTextStream *SiTimeFilestream, const QString &outputSTDF)
{
    GQTL_STDF::Stdf_MIR_V4 lMIRRecord;
    QString     lLine;
    gsbool lFirstWafer(true);

    // Write Test results for each line read.
    int         lLastWafer(-1);
    QStringList lFields, lAllLines;

    if(mStdfParse.Open(outputSTDF.toLatin1().constData(),STDF_WRITE) == false)
    {
        // Failed importing TriQuintDC file into STDF database
        mLastError = errWriteSTDF;
//        RemoveOutputFiles();
        QFile::remove(outputSTDF);
        // Convertion failed.
        return false;
    }

    // Don't have a date, use the 1-1-1970
    mStartTime = 0;

    // Prepare the MIR
    lMIRRecord.SetSETUP_T(mStartTime);
    lMIRRecord.SetSTART_T(mStartTime);
    lMIRRecord.SetLOT_ID(mLotId);
    lMIRRecord.SetPART_TYP(mProductId);
    // Construct custom Galaxy USER_TXT
    QString	lUserTxt;
    lUserTxt = GEX_IMPORT_DATAORIGIN_LABEL;
    lUserTxt += ":";
    lUserTxt += GEX_IMPORT_DATAORIGIN_ETEST;
    lUserTxt += ":SiTime";
    lMIRRecord.SetUSER_TXT(lUserTxt);
    // Write MIR
    mStdfParse.WriteRecord(&lMIRRecord);

    GQTL_STDF::Stdf_WCR_V4 lWCRRecord;
    // Write WCR/WIR of new Wafer.
    lWCRRecord.SetWF_FLAT('D');
    lWCRRecord.SetPOS_X('R');
    lWCRRecord.SetPOS_Y('U');
    mStdfParse.WriteRecord(&lWCRRecord);

    // Wafer,Site,LK_RT_EC_01_A,LK_RT_EC_01_B,LK_RT_EC_01_C,LK_RT_EC_01_D,...,Frequenz,G�te,Winkel,Bandwidth
    //1	1	 0.000	 0.001	 0.000	 2.000	 2.000 ....	 0.098	 40.10	 523.8	 41.88	 30.62	 12.51

    // Write all Parameters read on this file : WIR,PIR,PTR...,PRR,WRR WIR,PIR,PTR..., PRR
    // Read SiTime result
    do
    {
        lLine = ReadLine(*SiTimeFilestream).simplified();
        lFields = lLine.split(",", QString::KeepEmptyParts);
        if(lFields.size() < mTotalColumns)
        {
            // Incorrect header...this is not a valid SiTime ETEST file!
            mLastError = errInvalidFormatParameter;

            // Convertion failed.
            return false;
        }

        if (lFirstWafer)
        {
            lFirstWafer = false;
            lLastWafer = lFields[0].toInt();
        }
        // New wafer
        if (lLastWafer != lFields[0].toInt() && !lAllLines.isEmpty())
        {
//            lOutputFile = QFileInfo(outputSTDF).absolutePath() + QDir::separator();
//            lOutputFile += QFileInfo(outputSTDF).baseName() + "_" + QString::number(lLastWafer);
//            lOutputFile += "." + QFileInfo(outputSTDF).completeSuffix();
//            mOutputFiles.append(lOutputFile);
            lLastWafer = lFields[0].toInt();

            if (!WriteStdfFile(lAllLines))
            {
                QFile::remove(outputSTDF);
            }
            lAllLines.clear();
        }
        lAllLines.append(lLine);
    }while (!SiTimeFilestream->atEnd());

    // Write the last wafer
//    lOutputFile = QFileInfo(outputSTDF).absolutePath() + QDir::separator();
//    lOutputFile += QFileInfo(outputSTDF).baseName() + "_" + QString::number(lLastWafer);
//    lOutputFile += "." + QFileInfo(outputSTDF).completeSuffix();
//    mOutputFiles.append(lOutputFile);
//    if(mStdfParse.Open(lOutputFile.toLatin1().constData(),STDF_WRITE) == false)
//    {
//        // Failed importing TriQuintDC file into STDF database
//        mLastError = errWriteSTDF;
//        RemoveOutputFiles();
//        // Convertion failed.
//        return false;
//    }
    if (!WriteStdfFile(lAllLines))
    {
//        RemoveOutputFiles();
        QFile::remove(outputSTDF);
    }

    GQTL_STDF::Stdf_MRR_V4 lMRRRecord;
    lMRRRecord.SetFINISH_T(mStartTime);
    mStdfParse.WriteRecord(&lMRRRecord);
    mStdfParse.Close();

    // Success
    return true;
}

//////////////////////////////////////////////////////////////////////
// Convert 'FileName' SiTime file, to STDF 'strFileNameSTDF' file
//////////////////////////////////////////////////////////////////////
bool SiTimeEtesttoSTDF::ConvertoStdf(const QString &CsvFileName, QString &StdfFileName)
{
    // No erro (default)
    mLastError = errNoError;

    if(ReadSiTimeFile(CsvFileName,StdfFileName) != true)
    {
        return false;	// Error reading SiTime file
    }

    // Convertion successful
    return true;
}


bool SiTimeEtesttoSTDF::WriteStdfFile(QStringList& lAllLines)
{
    BYTE        bData;
    int         iIndex;
    int         lSite;
    float       lValue;
    bool        lValidResult;
    QString     lWafer;
    gsuint16        lTotalTests(0), lPartNumber(0);
    GQTL_STDF::Stdf_WRR_V4 lWRRRecord;
    GQTL_STDF::Stdf_WIR_V4 lWIRRecord;
    GQTL_STDF::Stdf_PIR_V4 lPIRRecord;
    GQTL_STDF::Stdf_PRR_V4 lPRRRecord;
    GQTL_STDF::Stdf_PTR_V4 lPTRRecord;
    QStringList lFields;

    lWafer = lAllLines[0].section(",", 0, 0);

    lWIRRecord.SetHEAD_NUM(1);                          // Test head
    lWIRRecord.SetSITE_GRP(255);                        // Tester site (all)
    lWIRRecord.SetSTART_T(mStartTime);                  // Start time
    lWIRRecord.SetWAFER_ID(lWafer);
    mStdfParse.WriteRecord(&lWIRRecord);

    // For each wafer, have to write limit in the first PTR
    for(iIndex=0;iIndex<mTotalParameters;iIndex++)
        mParameterList[iIndex].SetStaticHeaderWritten(false);

    for(int i=0; i<lAllLines.size(); ++i)
    {
        lFields.clear();
        lFields = lAllLines[i].split(",", QString::KeepEmptyParts);

        lPartNumber++;
        lSite = lFields[1].toInt();

        // Write PIR
        // Write PIR for parts in this Wafer site
        lPIRRecord.SetHEAD_NUM(1);                     // Test head
        lPIRRecord.SetSITE_NUM(lSite);                 // Tester site
        mStdfParse.WriteRecord(&lPIRRecord);

        // Reset counters
        lTotalTests = 0;

        for (int lIndex=PARAMETER_OFFSET; lIndex<mTotalColumns; ++lIndex)
        {
            lValidResult = false;
            lPTRRecord.Reset();
            QString lValueField(lFields[lIndex].trimmed());
            lValue = 0.0;
            lValue = lValueField.toFloat(&lValidResult);
            lTotalTests++;
            // Compute Test# (add user-defined offset)
            // Write PTR
            lPTRRecord.SetTEST_NUM(mParameterList[lIndex-PARAMETER_OFFSET].GetTestNumber());
            lPTRRecord.SetHEAD_NUM(1);                             // Test head
            lPTRRecord.SetSITE_NUM(lSite);                         // Tester site:1,2,3,4 or 5, etc.
            bData = BIT6;	// Test completed with no pass/fail indication
            // Check if have valid value
            if (!lValidResult)
            {
                bData = bData|BIT5|BIT1;        // Test aborted
                lPTRRecord.SetTEST_FLG(bData);
                lPTRRecord.SetALARM_ID(lValueField);
            }
            else
            {
                lPTRRecord.SetTEST_FLG(bData);                          // TEST_FLG
            }
            lPTRRecord.SetPARM_FLG(0);                          // PARAM_FLG
            lPTRRecord.SetRESULT(lValue);                        // Test result
            if(!mParameterList[lIndex-PARAMETER_OFFSET].GetStaticHeaderWritten())
            {
                mParameterList[lIndex-PARAMETER_OFFSET].SetStaticHeaderWritten(true);

                // save Parameter name
                lPTRRecord.SetTEST_TXT(mParameterList[lIndex-PARAMETER_OFFSET].GetTestName());  // TEST_TXT
            }
            mStdfParse.WriteRecord(&lPTRRecord);
        }

        // Write PRR
        int lCoordX, lCoordY;
        lPRRRecord.SetHEAD_NUM(1);                  // Test head
        lPRRRecord.SetSITE_NUM(lSite);              // Tester site:1,2,3,4 or 5
        bData = BIT4;                               // Device completed testing with no pass/fail indication
        lPRRRecord.SetPART_FLG(bData);              // PART_FLG
        lPRRRecord.SetNUM_TEST(lTotalTests);        // NUM_TEST
        lPRRRecord.SetHARD_BIN(1);                     // HARD_BIN
        switch(lSite)
        {
        case 1:
            lCoordX = 0;
            lCoordY = 0;
            break;
        case 2:
            lCoordX = -2;
            lCoordY = 0;
            break;
        case 3:
            lCoordX = 0;
            lCoordY = 2;
            break;
        case 4:
            lCoordX = 2;
            lCoordY = 0;
            break;
        case 5:
            lCoordX = 0;
            lCoordY = -1;
            break;
        case 6:
            lCoordX = -2;
            lCoordY = -1;
            break;
        case 7:
            lCoordX = -1;
            lCoordY = 1;
            break;
        case 8:
            lCoordX = 2;
            lCoordY = 2;
            break;
        case 9:
            lCoordX = 1;
            lCoordY = -1;
            break;
        default: // More than 9 sites?....give 0,0 coordonates
            lCoordX = 0;
            lCoordY = 0;
            break;
        }
        lPRRRecord.SetX_COORD(lCoordX);
        lPRRRecord.SetY_COORD(lCoordY);
        mStdfParse.WriteRecord(&lPRRRecord);
    }

    // Write WRR for last wafer inserted
    lWRRRecord.SetHEAD_NUM(1);                      // Test head
    lWRRRecord.SetSITE_GRP(255);                    // Tester site (all)
    lWRRRecord.SetFINISH_T(mStartTime);            // Time of last part tested
    lWRRRecord.SetPART_CNT(lPartNumber);           // Parts tested
    lWRRRecord.SetWAFER_ID(lWafer);	// WaferID
    mStdfParse.WriteRecord(&lWRRRecord);


    return true;
}



}
}
