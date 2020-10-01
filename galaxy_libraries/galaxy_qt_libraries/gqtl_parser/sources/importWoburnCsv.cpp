//////////////////////////////////////////////////////////////////////
// importWoburnCsv.cpp: Convert a .CSV / Excel file to STDF V4.0
//////////////////////////////////////////////////////////////////////
#include "importWoburnCsv.h"
#include <gqtl_log.h>
#include <math.h>
#include <qfileinfo.h>

//#Test ID	Test Level	Mask	Lot	Operator	Wafer	Map ID	X	Y	DUT Number	Equipment	Temperature	Program_Rev	Limits_File	PC Number	Test_Date	Test_Time	Rfin_Pos	Ref1_Pos	Ref2_Pos	Rfout_Pos	Bias_Pos	Vc1_Pos	Rfin_Neg	Ref1_Neg	Ref2_Neg	Rfout_Neg	Bias_Neg	Vcl_Neg	Bias_Lkg	Vcl_Lkg	Rfout_Lkg	Vcl_7VLkg	Rfout_7VLkg	Total_Lkg	IRref	Vref1	Vref2	IBias1	IBias2	Icc1	Icc2	Trap_Lkg	IT_Lkg	ICQ	SW_BIN	BIN	Failed_Parameters	Prober_X	Prober_Y	Site	Variant
//#TestNum	Final Probe	93007b1ns	6929559	EA9999	6929559-8	Blank in NBP File	BLANK	BLANK	BLANK								PTR.TEST_NUM (may be blank)	PTR.TEST_NUM (may be blank)																												[initially put a default value here in sw_bin and bin columns]
//#TestHeader	FlatID	#CSVFormatRev	#Software	#TestPlan	#TestScreen	#ProbeMap	#WoburnDUTNum	#TestPlatform	#CreationDate	#DPAT	#Recipe							[columns A to R are inserted]																													[this and following columns are ADDED]						[number here, single char in map]
//#TestDetails	272151S190	1	ETS88_EG_Prober_RevG.31.1_Dual-io	13015b.tp1	13015b.ts1	13015_5mm_c.mp1	13015b	EagleDC	15-08-19-07:12:08	13051b.gxr	[Recipe .json file name]																																				[initially put a default value here in sw_bin and bin columns]					[for now make this a '1']
//#MapHeader	#WaferDiameter	#WaferUnits	#FlatLocation	#FlatType	#DimFieldX	#DimFieldY	#DieX	#DieY	#WaferX	#WaferY	#FieldX	#FieldY	#TotalDieCount	#PassCount	#DPATFails
//#MapDetails	"[ex. 101600]
//WCR.WAFR_SIZ"	um	East	N	21600	21840	720	560	7	7	39	30	37077	36819
//#Comments	[insert comments here]
//#HardBinName	"Pass
//HBR.HBIN_NAM"	"Continuity
//HBR.HBIN_NAM"	Leakage	Functional	DPAT	GPAT	[use as many columns as needed to define all hardware and software bins]
//#HardBinNumber	"1
//HBR.HBIN_NUM"	"5
//HBR.HBIN_NUM"	6	7	20	21
//#HardBinCategory	"P
//HBR.HBIN_PF"	"F
//HBR.HBIN_PF"	F	F	F	F
//#SoftBinName	"Pass
//SBR.SBIN_NAM"	Pass2	Continuity	Leakage	HVLeakage	LVLeakage	FuncFails	DPAT	GPAT
//#SoftBinNumber	"1
//SBR.SBIN_NUM"	2	3	4	5	6	7	144	145
//#SoftBinCategory	"P
//SBR.SBIN_PF"	P	F	F	F	F	F	F	F
//#LoLimit																	0.001	2.876	2.538	9.306	7.169	9.526	-0.009	-1.222	-1.222	-1.195	-1.222	-1.217	-0.32	-0.02	-0.45	0.57	1.09	-0.71	0.15	2.51	2.85	93.2	495.5	6	5	-0.1	-0.1	10
//#HiLimit																	0.01	3.044	2.61	9.474	7.301	9.694	-0.001	-1.196	-1.194	-1.169	-1.196	-1.191	0.45	0.08	0.67	1.2	2.5	1.11	0.2	2.59	3.05	132.8	645.5	10.1	20	0.1	0.2	30
//#LoGrossLimit																	-10	0.1	0.1	0.1	0.1	0.1	-999	-999	-999	-999	-999	-999	-10	-10	-10	-10	-10	-10	-10	-10	-10	-10	-10	-10	-999	-999	-999	-999
//#HiGrossLimit																	999	999	999	999	999	999	-0.001	-0.1	-0.1	-0.1	-0.1	-0.1	999	999	999	999	999	999	999	999	999	999	999	999	999	999	999	999
//#Units																	                V	                V	                V	                V	                V	                V	                V	                V	                V	                V	                V	                V	A	A	A	A	A	A	A	                V	                V	A	A	A	A	A	A	A
//#LimitScaling																	0	0	0	0	0	0	0	0	0	0	0	0	6	6	6	6	6	6	3	0	0	6	6	3	3	6	6	3
//	Wafer	93007B1NS	6929559	EA9999	6929559-8	151609	5	1	1609	DC_ETS88_03	25C	13015b.tp1	13015b.ts1	PCID139566805	15-08-19	5:43:13	0.005	3.396	2.7	9.358	7.207	9.577	-0.004	-1.203	-1.203	-1.178	-1.204	-1.199	0.00000002	0.000000019	-2.90E-08	9.63E-07	1.87E-06	1.00E-08	1.59E-04	2.62E+00	3.02	0.000106822	0.000436665	0.007063	1.33E-02	-6.00E-09	-3.00E-09	0.018981	2	2		164	15	1	1
//	Wafer	93007B1NS	6929559	EA9999	6929559-8	151608	5	1	1608	DC_ETS88_03	25C	13015b.tp1	13015b.ts1	PCID139566805	15-08-19	5:43:13	0.005	3.372	2.683	9.356	7.194	9.57	-0.004	-1.201	-1.2	-1.178	-1.202	-1.198	0.000000019	0.00000002	2.70E-08	9.71E-07	1.90E-06	6.60E-08	1.59E-04	2.61E+00	3.011	0.000107283	0.000444581	0.007122	1.33E-02	-3.00E-09	0.00E+00	0.01908	2	2		163	15	1	1
//	Wafer	93007B1NS	6929559	EA9999	6929559-8	151607	5	1	1607	DC_ETS88_03	25C	13015b.tp1	13015b.ts1	PCID139566805	15-08-19	5:43:13	0.005	3.342	2.672	9.345	7.196	9.567	-0.004	-1.201	-1.201	-1.176	-1.202	-1.198	0.000000017	0.000000024	4.30E-08	9.80E-07	1.96E-06	8.40E-08	1.59E-04	2.60E+00	3.009	0.000107363	0.000462309	0.007164	1.32E-02	0.00E+00	3.00E-09	0.019152	2	2		162	15	1	1
//	Wafer	93007B1NS	6929559	EA9999	6929559-8	151606	5	1	1606	DC_ETS88_03	25C	13015b.tp1	13015b.ts1	PCID139566805	15-08-19	5:43:13	0.005	3.295	2.66	9.348	7.206	9.572	-0.004	-1.203	-1.202	-1.177	-1.204	-1.198	0.000000021	0.000000025	5.10E-08	9.74E-07	1.95E-06	9.60E-08	1.60E-04	2.59E+00	3.004	0.000109888	0.000468761	0.007375	1.32E-02	-4.00E-09	4.00E-09	0.019615	2	2		161	15	1	1

namespace GS
{
namespace Parser
{

//////////////////////////////////////////////////////////////////////
// Construction
//////////////////////////////////////////////////////////////////////
WoburnCSVtoSTDF::WoburnCSVtoSTDF(): ParserBase(typeWoburnCsv, "typeWoburnCsv")
{
    mParameters.clear();
    mSwBinCell = 0;
    mTotalParameters = 0;
    mPassedTime = 0;
    mDTRMetaDataRecords.clear();
    mHardBins.clear();
    mSoftBins.clear();
    mWaferId = "";
    mParameterDirectory.SetFileName(GEX_WOBURN_PARAMETERS);
}

//////////////////////////////////////////////////////////////////////
// Destruction
//////////////////////////////////////////////////////////////////////
WoburnCSVtoSTDF::~WoburnCSVtoSTDF()
{
    // Destroy list of Parameters tables.
    mParameters.clear();
    mHardBins.clear();
    mSoftBins.clear();
    qDeleteAll(mDTRMetaDataRecords);
}


//////////////////////////////////////////////////////////////////////
// Check if File is compatible with TERADYNE_ASCII format
//////////////////////////////////////////////////////////////////////
bool WoburnCSVtoSTDF::IsCompatible(const QString &lFileName)
{
    bool lIsCompatible(false);

    QFile lFile(lFileName);
    if(!lFile.open(QIODevice::ReadOnly ))
    {
        // Convertion failed.
        return false;
    }
    // Assign file I/O stream
    QTextStream lCsvFile(&lFile);


    // Check the compatibility
    QString lStrString = lCsvFile.readLine();
    QStringList lStrCells = lStrString.split(",",QString::KeepEmptyParts);
    if (lStrCells.count() >= CSV_WOBURN_RAW_FIRST_DATA
        && lStrCells[0].contains("#Test ID", Qt::CaseInsensitive)
        && lStrCells[1].contains("Test Level", Qt::CaseInsensitive)
        && lStrCells[CSV_WOBURN_RAW_PART_TYP].contains("Mask", Qt::CaseInsensitive)
        && lStrCells[CSV_WOBURN_RAW_LOT_ID].contains("Lot", Qt::CaseInsensitive)
        && lStrCells[CSV_WOBURN_RAW_OPERATOR].contains("Operator", Qt::CaseInsensitive)
        && lStrCells[CSV_WOBURN_RAW_WAFER].contains("Wafer", Qt::CaseInsensitive)
        && lStrString.contains("sw_bin", Qt::CaseInsensitive))
        lIsCompatible = true;
    else
        return false;

    lStrString = lCsvFile.readLine();
    if (!lStrString.startsWith("#TestNum", Qt::CaseInsensitive))
        return false;

    // Read the next line to be sure that it is the expected format
    lStrString = lCsvFile.readLine();
    if (!lStrString.startsWith("#TestHeader", Qt::CaseInsensitive))
        return false;

    lStrString = lCsvFile.readLine();
    if (!lStrString.startsWith("#TestDetails", Qt::CaseInsensitive))
        return false;



    // Close file
    lFile.close();

    return lIsCompatible;
}

/**
 * @brief The WoburnHeaderParser class contains imlementation details to parse and write in-memory
 * all details related to headers in the input csv file
 */
class WoburnHeaderParser
{
public :
    struct TestHeaderIndexes
    {
        gsuint16 mProbIndex, mWoburnDUTNumIndex, mTestPlatform, mPassedTime, mFab, mLimitSetRev, mTestCategory, mTestPlan;

        TestHeaderIndexes() : mProbIndex(), mWoburnDUTNumIndex(), mTestPlatform(), mPassedTime(),
                              mFab(), mLimitSetRev(), mTestCategory(), mTestPlan() {}
    };
    struct MapHeaderInfos
    {
        gsuint16 mWafSizeIndex, mWFUnitIndex, mWaferPlatIndex, mDieWidIndex,
                 mDieHTIndex, mWaferXIndex, mWAferYIndex, mFieldXIndex,
                 mFieldYIndex, mDieCountIndex, mPassCountIndex;

        MapHeaderInfos() : mWafSizeIndex(), mWFUnitIndex(), mWaferPlatIndex(), mDieWidIndex(),
                           mDieHTIndex(), mWaferXIndex(), mWAferYIndex(), mFieldXIndex(),
                           mFieldYIndex(), mDieCountIndex(), mPassCountIndex() {}
    };

public :
    WoburnHeaderParser( QTextStream &aStream, WoburnCSVtoSTDF &aParser ) : mStream( aStream ), mParser( aParser ) {}

    bool ParseHeaderLines()
    {
        QString lString = mParser.ReadLine(mStream);

        if( !IsTestIdLineType( lString ) )
        {
            mParser.mLastError = ParserBase::errInvalidFormatParameter;
            return false;
        }

        ProcessTestIdLineType( lString );

        for( unsigned char lLineTypeIndex = 0; lLineTypeIndex < 18; ++lLineTypeIndex )
        {
            lString = mParser.ReadLine(mStream);

            if( IsTestNumLineType( lString ) )
                ProcessTestNumLineType( lString );
            else if( IsTestHeaderLineType( lString ) )
                ProcessTestHeaderLineType( lString );
            else if( IsTestDetailsLineType( lString ) )
                ProcessTestDetailsLineType( lString );
            else if( IsMapHeaderLineType( lString ) )
                ProcessMapHeaderLineType( lString );
            else if( IsMapDetailsLineType( lString ) )
            {
                if( !ProcessMapDetailsLineType( lString ) )
                {
                    mParser.mLastError = ParserBase::errInvalidFormatParameter;
                    return false;
                }
            }
            else if( IsCommentLineType( lString ) )
                continue;
            else if( IsHardBinNameLineType( lString ) )
                ProcessHardBinNameLineType( lString );
            else if( IsHardBinNumberLineType( lString ) )
                ProcessHardBinNumberLineType( lString );
            else if( IsHardBinCategoryLineType( lString ) )
                ProcessHardBinCategoryLineType( lString );
            else if( IsSoftBinNameLineType( lString ) )
                ProcessSoftBinNameLineType( lString );
            else if( IsSoftBinNumberLineType( lString ) )
                ProcessSoftBinNumberLineType( lString );
            else if( IsSoftBinCategoryLineType( lString ) )
                ProcessSoftBinCategoryLineType( lString );
            else if( IsLoLimitLineType( lString ) )
                ProcessLoLimitLineType( lString );
            else if( IsHiLimitLineType( lString ) )
                ProcessHiLimitLineType( lString );
            else if( IsLoGrossLimitLineType( lString ) )
                ProcessLoGrossLimitLineType( lString );
            else if( IsHiGrossLimitLineType( lString ) )
                ProcessHiGrossLimitLineType( lString );
            else if( IsUnitsLineType( lString ) )
                ProcessUnitsLineType( lString );
            else if( IsLimitScalingLineType( lString ) )
                ProcessLimitScalingLineType( lString );
            else
            {
                mParser.mLastError = ParserBase::errInvalidFormatParameter;
                return false;
            }
        }

        // insert the list of SBin and HBin into the map
        for(int i=0; i<mSoftBins.size(); ++i)
            mParser.mSoftBins.insert(mSoftBins[i].GetBinNumber(), mSoftBins[i]);

        for(int i=0; i<mHardBins.size(); ++i)
            mParser.mHardBins.insert(mHardBins[i].GetBinNumber(), mHardBins[i]);

        return true;
    }

private :
    const QStringList &GetCellsFromLine( const QString &aLine )
    {
        if( mCells.empty() )
            mCells = aLine.split( ",", QString::KeepEmptyParts );

        return mCells;
    }
    bool IsTestIdLineType( const QString &aLine )
    {
        const QStringList &lCells = GetCellsFromLine( aLine );

        if( lCells.count() >= CSV_WOBURN_RAW_FIRST_DATA
            && lCells[ 0 ].contains( "#Test ID", Qt::CaseInsensitive )
            && lCells[ 1 ].contains( "Test Level", Qt::CaseInsensitive )
            && lCells[ CSV_WOBURN_RAW_PART_TYP ].contains( "Mask", Qt::CaseInsensitive )
            && lCells[ CSV_WOBURN_RAW_LOT_ID ].contains( "Lot", Qt::CaseInsensitive )
            && lCells[ CSV_WOBURN_RAW_OPERATOR ].contains( "Operator", Qt::CaseInsensitive )
            && lCells[ CSV_WOBURN_RAW_WAFER ].contains( "Wafer", Qt::CaseInsensitive )
            && aLine.contains( "sw_bin", Qt::CaseInsensitive ) )
            return true;

        return false;
    }
    void ProcessTestIdLineType( const QString &aLine )
    {
        const QStringList &lCells = GetCellsFromLine( aLine );
        mParser.mTotalParameters=0;
        mParser.mSwBinCell = 0;
        mParser.mTotalParameters = lCells.count() - CSV_WOBURN_RAW_FIRST_DATA;
        for (int i=0; i<lCells.count(); ++i)
            if (lCells[i].compare("sw_bin", Qt::CaseInsensitive) == 0)
            {
                mParser.mSwBinCell = i;
                mParser.mTotalParameters = i - CSV_WOBURN_RAW_FIRST_DATA;
            }

        GSLOG( SYSLOG_SEV_INFORMATIONAL, QString( "%1 Total Parameters" ).arg( mParser.mTotalParameters).toLatin1().constData() );

        // Set test names in the csv parameters
        ParserParameter lParam;
        for( gsuint16 i = 0; i < mParser.mTotalParameters; ++i)
        {
            lParam.SetTestName( lCells[ i + CSV_WOBURN_RAW_FIRST_DATA ] );

            int lTestNumber = mParser.mParameterDirectory.UpdateParameterIndexTable( lCells[i + CSV_WOBURN_RAW_FIRST_DATA ] );

            if (lTestNumber >= 0)
                lParam.SetTestNumber(lTestNumber);

            mParser.mParameters.append( lParam );
        }
    }
    bool IsTestNumLineType( const QString &aLine )
    {
        return aLine.startsWith( "#TestNum", Qt::CaseInsensitive );
    }
    void ProcessTestNumLineType( const QString &aLine )
    {
        mCells = aLine.split( ",", QString::KeepEmptyParts );
        if ( ( mCells.size() > CSV_WOBURN_RAW_FIRST_DATA ) && ( mCells[ CSV_WOBURN_RAW_FIRST_DATA ] != "" ) )
            for( gsuint16 i = 0; i < mParser.mTotalParameters; ++i )
                mParser.mParameters[i].SetTestNumber( mCells[ i + CSV_WOBURN_RAW_FIRST_DATA ].toFloat() );
    }
    bool IsTestHeaderLineType( const QString &aLine )
    {
        return aLine.startsWith("#TestHeader", Qt::CaseInsensitive);
    }
    void ProcessTestHeaderLineType( const QString &aLine )
    {
        if (aLine.startsWith("#TestHeader", Qt::CaseInsensitive))
        {
            mCells = aLine.split(",",QString::KeepEmptyParts);
            gsuint16 lSizeCell(mCells.size());
            for(gsuint16 i=1; i<lSizeCell; ++i)
            {
                if (mCells[i].isEmpty())
                    continue;
                else if (mCells[i].compare("#ProbeMap", Qt::CaseInsensitive) == 0)
                {
                    mTestHeaderIndexes.mProbIndex = i;
                }
                else if (mCells[i].compare("#WoburnDUTNum", Qt::CaseInsensitive) == 0)
                {
                    mTestHeaderIndexes.mWoburnDUTNumIndex = i;
                }
                else if (mCells[i].compare("#TestPlatform", Qt::CaseInsensitive) == 0)
                {
                     mTestHeaderIndexes.mTestPlatform = i;
                }
                else if (mCells[i].compare("#TotalTestTime", Qt::CaseInsensitive) == 0)
                {
                     mTestHeaderIndexes.mPassedTime = i;
                }
                else if (mCells[i].contains("#Fab", Qt::CaseInsensitive))
                {
                    mTestHeaderIndexes.mFab = i;
                }
                else if (mCells[i].contains("#LimitSetRev", Qt::CaseInsensitive))
                {
                    mTestHeaderIndexes.mLimitSetRev = i;
                }
                else if (mCells[i].contains("#TestCategory", Qt::CaseInsensitive))
                {
                    mTestHeaderIndexes.mTestCategory = i;
                }
                else if (mCells[i].contains("#TestPlanRev", Qt::CaseInsensitive))
                {
                    mTestHeaderIndexes.mTestPlan = i;
                }
            }
        }
    }
    bool IsTestDetailsLineType( const QString &aLine )
    {
        return aLine.startsWith("#TestDetails", Qt::CaseInsensitive);
    }
    void ProcessTestDetailsLineType( const QString &aLine )
    {
        mCells.clear();
        mCells = aLine.split(",",QString::KeepEmptyParts);
        if (mCells.size() < mTestHeaderIndexes.mPassedTime)
        {
            mParser.mLastError = ParserBase::errInvalidFormatParameter;
            GSLOG(SYSLOG_SEV_ERROR,
                  QString("Not enough fields from input #TestDetails lines (%1 instead of %2 expected)")
                  .arg(mCells.count()).arg(mTestHeaderIndexes.mPassedTime).toLatin1().constData());
        }
        mParser.AddMetaDataToDTR("ProbeMap", mCells[mTestHeaderIndexes.mProbIndex]);
        mParser.AddMetaDataToDTR("WoburnDUTNum", mCells[mTestHeaderIndexes.mWoburnDUTNumIndex]);
        mParser.mMIRRecord.SetTSTR_TYP(mCells[mTestHeaderIndexes.mTestPlatform]);
        mParser.mPassedTime = mCells[mTestHeaderIndexes.mPassedTime].toInt();
        mParser.mMIRRecord.SetUSER_TXT("Fab=" + mCells[mTestHeaderIndexes.mFab]);
        mParser.mMIRRecord.SetSPEC_VER(mCells[mTestHeaderIndexes.mLimitSetRev]);
        if (mCells[mTestHeaderIndexes.mTestCategory].size() > 1)
            mParser.mMIRRecord.SetMODE_COD(mCells[mTestHeaderIndexes.mTestCategory].at(0).toLatin1());
        mParser.mMIRRecord.SetJOB_REV(mCells[mTestHeaderIndexes.mTestPlan]);
        mParser.mWaferId = mCells[1];
    }
    bool IsMapHeaderLineType( const QString &aLine )
    {
        return aLine.startsWith("#MapHeader", Qt::CaseInsensitive);
    }
    void ProcessMapHeaderLineType( const QString &aLine )
    {
        mCells.clear();
        mCells = aLine.split(",",QString::KeepEmptyParts);
        gsuint16 lSizeCell(mCells.size());
        for(gsuint16 i=1; i<lSizeCell; ++i)
        {
            if (mCells[i].isEmpty())
                continue;
            else if (mCells[i].compare("#WaferDiameter", Qt::CaseInsensitive) == 0)
            {
                mMapHeaderInfos.mWafSizeIndex = i;
            }
            else if (mCells[i].compare("#WaferUnits", Qt::CaseInsensitive) == 0)
            {
                mMapHeaderInfos.mWFUnitIndex = i;
            }
            else if (mCells[i].compare("#FlatLocation", Qt::CaseInsensitive) == 0)
            {
                 mMapHeaderInfos.mWaferPlatIndex = i;
            }
            if (mCells[i].compare("#DieX", Qt::CaseInsensitive) == 0)
            {
                 mMapHeaderInfos.mDieWidIndex = i;
            }
            else if (mCells[i].compare("#DieY", Qt::CaseInsensitive) == 0)
            {
                 mMapHeaderInfos.mDieHTIndex = i;
            }
            else if (mCells[i].compare("#WaferX", Qt::CaseInsensitive) == 0)
            {
                 mMapHeaderInfos.mWaferXIndex = i;
            }
            else if (mCells[i].compare("#WaferY", Qt::CaseInsensitive) == 0)
            {
                 mMapHeaderInfos.mWAferYIndex = i;
            }
            else if (mCells[i].compare("#FieldX", Qt::CaseInsensitive) == 0)
            {
                 mMapHeaderInfos.mFieldXIndex = i;
            }
            else if (mCells[i].compare("#FieldY", Qt::CaseInsensitive) == 0)
            {
                 mMapHeaderInfos.mFieldYIndex = i;
            }
            else if (mCells[i].compare("#TotalDieCount", Qt::CaseInsensitive) == 0)
            {
                 mMapHeaderInfos.mDieCountIndex = i;
            }
            else if (mCells[i].compare("#PassCount", Qt::CaseInsensitive) == 0)
            {
                 mMapHeaderInfos.mPassCountIndex = i;
            }
        }
    }
    bool IsMapDetailsLineType( const QString &aLine )
    {
        return aLine.startsWith("#MapDetails", Qt::CaseInsensitive);
    }
    bool ProcessMapDetailsLineType( const QString &aLine )
    {
        mCells = aLine.split(",",QString::KeepEmptyParts);

        if (mCells.size() < mMapHeaderInfos.mPassCountIndex)
        {
            mParser.mLastError = ParserBase::errInvalidFormatParameter;
            return false;
        }

        gsint16 lCoeff(1);
        if (mCells[mMapHeaderInfos.mWFUnitIndex].size() > 0)
        {
            if (mCells[mMapHeaderInfos.mWFUnitIndex].compare("um", Qt::CaseInsensitive) == 0)
            {
                lCoeff = 1000;
                mParser.mWCRRecord.SetWF_UNITS(3);
            }
            else
                // if(mCells[lWFUnitIndex] == "mills") By default use this value
            {
                mParser.mWCRRecord.SetWF_UNITS(4);
            }
        }
        mParser.mWCRRecord.SetWAFR_SIZ(mCells[mMapHeaderInfos.mWafSizeIndex].toFloat() / lCoeff);
        if (mCells[mMapHeaderInfos.mWaferPlatIndex].size() > 0)
            mParser.mWCRRecord.SetWF_FLAT(mCells[mMapHeaderInfos.mWaferPlatIndex].at(0).toLatin1());
        mParser.mWCRRecord.SetDIE_WID(mCells[mMapHeaderInfos.mDieWidIndex].toFloat() / lCoeff);
        mParser.mWCRRecord.SetDIE_HT(mCells[mMapHeaderInfos.mDieHTIndex].toFloat() / lCoeff);
        mParser.AddMetaDataToDTR("WaferX", mCells[mMapHeaderInfos.mWaferXIndex]);
        mParser.AddMetaDataToDTR("WaferY", mCells[mMapHeaderInfos.mWAferYIndex]);
        mParser.AddMetaDataToDTR("FieldX", mCells[mMapHeaderInfos.mFieldXIndex]);
        mParser.AddMetaDataToDTR("FieldY", mCells[mMapHeaderInfos.mFieldYIndex]);
        mParser.mWRRRecord.SetPART_CNT(mCells[mMapHeaderInfos.mDieCountIndex].toULong());
        mParser.mWRRRecord.SetGOOD_CNT(mCells[mMapHeaderInfos.mPassCountIndex].toULong());
        mParser.mPCRRecord.SetHEAD_NUM(255);        // All sites
        mParser.mPCRRecord.SetPART_CNT(mCells[mMapHeaderInfos.mDieCountIndex].toULong());
        mParser.mPCRRecord.SetGOOD_CNT(mCells[mMapHeaderInfos.mPassCountIndex].toULong());

        return true;
    }
    bool IsCommentLineType( const QString &aLine )
    {
        return aLine.startsWith("#Comments", Qt::CaseInsensitive);
    }
    bool IsHardBinNameLineType( const QString &aLine )
    {
        return aLine.startsWith("#HardBinName", Qt::CaseInsensitive);
    }
    void ProcessHardBinNameLineType(const QString &aLine )
    {
        mCells.clear();
        mCells = aLine.split(",",QString::SkipEmptyParts);
        gsuint16 lSizeCell(mCells.size());
        for(gsuint16 i=1; i<lSizeCell; i++)
        {
            ParserBinning lBin;
            lBin.SetBinName(mCells[i]);
            mHardBins.append(lBin);
        }
    }
    bool IsHardBinNumberLineType( const QString &aLine )
    {
        return aLine.startsWith("#HardBinNumber", Qt::CaseInsensitive);
    }
    void ProcessHardBinNumberLineType(const QString &aLine )
    {
        mCells.clear();
        mCells = aLine.split(",",QString::SkipEmptyParts);
        gsuint16 lSizeCell(mCells.size());
        for(gsuint16 i=1; i<lSizeCell; i++)
        {
            QString lBinNumberString = mCells[i];
            gsuint16 lshort = lBinNumberString.toShort();
            mHardBins[i-1].SetBinNumber(lshort);
        }
    }
    bool IsHardBinCategoryLineType( const QString &aLine )
    {
        return aLine.startsWith("#HardBinCategory", Qt::CaseInsensitive);
    }
    void ProcessHardBinCategoryLineType(const QString &aLine )
    {
        mCells.clear();
        mCells = aLine.split(",",QString::SkipEmptyParts);
        gsuint16 lSizeCell(mCells.size());
        for(gsuint16 i=1; i<lSizeCell; ++i)
        {
            mHardBins[i-1].SetPassFail(mCells[i]=="P"?true:false);
        }
    }
    bool IsSoftBinNameLineType( const QString &aLine )
    {
        return aLine.startsWith("#SoftBinName", Qt::CaseInsensitive);
    }
    void ProcessSoftBinNameLineType(const QString &aLine )
    {
        mCells.clear();
        mCells = aLine.split(",",QString::SkipEmptyParts);
        gsuint16 lSizeCell(mCells.size());
        for(gsuint16 i=1; i<lSizeCell; ++i)
        {
            ParserBinning lBin;
            lBin.SetBinName(mCells[i]);
            mSoftBins.append(lBin);
        }
    }
    bool IsSoftBinNumberLineType( const QString &aLine )
    {
        return aLine.startsWith("#SoftBinNumber", Qt::CaseInsensitive);
    }
    void ProcessSoftBinNumberLineType(const QString &aLine )
    {
        mCells.clear();
        mCells = aLine.split(",",QString::SkipEmptyParts);
        gsuint16 lSizeCell(mCells.size());
        for(gsuint16 i=1; i<lSizeCell; ++i)
        {
            mSoftBins[i-1].SetBinNumber(mCells[i].toUShort());
        }
    }
    bool IsSoftBinCategoryLineType( const QString &aLine )
    {
        return aLine.startsWith("#SoftBinCategory", Qt::CaseInsensitive);
    }
    void ProcessSoftBinCategoryLineType(const QString &aLine )
    {
        mCells.clear();
        mCells = aLine.split(",",QString::SkipEmptyParts);
        gsuint16 lSizeCell(mCells.size());
        for(gsuint16 i=1; i<lSizeCell; ++i)
        {
            mSoftBins[i-1].SetPassFail(mCells[i]=="P"?true:false);
        }
    }
    bool IsLoLimitLineType( const QString &aLine )
    {
        return aLine.startsWith("#LoLimit", Qt::CaseInsensitive);
    }
    void ProcessLoLimitLineType( const QString &aLine )
    {
        mCells.clear();
        mCells = aLine.split(",");
        if ((mParser.mTotalParameters+CSV_WOBURN_RAW_FIRST_DATA) > mCells.size())
        {
            mParser.mLastError = ParserBase::errInvalidFormatParameter;
            GSLOG(SYSLOG_SEV_ERROR,
                  QString("Not enough fields from input #LoLimit lines (%1 instead of %2 expected)")
                  .arg(mCells.count()).arg(CSV_WOBURN_RAW_FIRST_DATA + mParser.mTotalParameters).toLatin1().constData());
        }
        for(gsuint16 i=CSV_WOBURN_RAW_FIRST_DATA; i<mParser.mTotalParameters+CSV_WOBURN_RAW_FIRST_DATA; ++i)
        {
            bool lOk(false);
            mParser.mParameters[i-CSV_WOBURN_RAW_FIRST_DATA].SetLowLimit(mCells[i].toDouble(&lOk));
            mParser.mParameters[i-CSV_WOBURN_RAW_FIRST_DATA].SetValidLowLimit(lOk);
        }
    }
    bool IsHiLimitLineType( const QString &aLine )
    {
        return aLine.startsWith("#HiLimit", Qt::CaseInsensitive);
    }
    void ProcessHiLimitLineType( const QString &aLine )
    {
        mCells.clear();
        mCells = aLine.split(",");
        if ((mParser.mTotalParameters+CSV_WOBURN_RAW_FIRST_DATA) > mCells.size())
        {
            mParser.mLastError = ParserBase::errInvalidFormatParameter;
            GSLOG(SYSLOG_SEV_ERROR,
                  QString("Not enough fields from input #HiLimit lines (%1 instead of %2 expected)")
                  .arg(mCells.count()).arg(CSV_WOBURN_RAW_FIRST_DATA + mParser.mTotalParameters).toLatin1().constData());
        }
        for(gsuint16 i=CSV_WOBURN_RAW_FIRST_DATA; i<mParser.mTotalParameters+CSV_WOBURN_RAW_FIRST_DATA; ++i)
        {
            bool lOk(false);
            mParser.mParameters[i-CSV_WOBURN_RAW_FIRST_DATA].SetHighLimit(mCells[i].toDouble(&lOk));
            mParser.mParameters[i-CSV_WOBURN_RAW_FIRST_DATA].SetValidHighLimit(lOk);
        }
    }
    bool IsLoGrossLimitLineType( const QString &aLine )
    {
        return aLine.startsWith("#LoGrossLimit", Qt::CaseInsensitive);
    }
    void ProcessLoGrossLimitLineType( const QString &aLine )
    {
        mCells.clear();
        mCells = aLine.split(",");
        if ((mParser.mTotalParameters+CSV_WOBURN_RAW_FIRST_DATA) > mCells.size())
        {
            mParser.mLastError = ParserBase::errInvalidFormatParameter;
            GSLOG(SYSLOG_SEV_ERROR,
                  QString("Not enough fields from input #LoGrossLimit lines (%1 instead of %2 expected)")
                  .arg(mCells.count()).arg(CSV_WOBURN_RAW_FIRST_DATA + mParser.mTotalParameters).toLatin1().constData());
        }
        for(gsuint16 i=CSV_WOBURN_RAW_FIRST_DATA; i<mParser.mTotalParameters+CSV_WOBURN_RAW_FIRST_DATA; ++i)
        {
            bool lOk(false);
            mParser.mParameters[i-CSV_WOBURN_RAW_FIRST_DATA].SetLowSpecLimit(mCells[i].toDouble(&lOk));
            mParser.mParameters[i-CSV_WOBURN_RAW_FIRST_DATA].SetValidLowSpecLimit(lOk);
        }
    }
    bool IsHiGrossLimitLineType( const QString &aLine )
    {
        return aLine.startsWith("#HiGrossLimit", Qt::CaseInsensitive);
    }
    void ProcessHiGrossLimitLineType( const QString &aLine )
    {
        mCells.clear();
        mCells = aLine.split(",");
        if ((mParser.mTotalParameters+CSV_WOBURN_RAW_FIRST_DATA) > mCells.size())
        {
            mParser.mLastError = ParserBase::errInvalidFormatParameter;
            GSLOG(SYSLOG_SEV_ERROR,
                  QString("Not enough fields from input #HiGrossLimit lines (%1 instead of %2 expected)")
                  .arg(mCells.count()).arg(CSV_WOBURN_RAW_FIRST_DATA + mParser.mTotalParameters).toLatin1().constData());
        }
        for(gsuint16 i=CSV_WOBURN_RAW_FIRST_DATA; i<mParser.mTotalParameters+CSV_WOBURN_RAW_FIRST_DATA; ++i)
        {
            bool lOk(false);
            mParser.mParameters[i-CSV_WOBURN_RAW_FIRST_DATA].SetHighSpecLimit(mCells[i].toDouble(&lOk));
            mParser.mParameters[i-CSV_WOBURN_RAW_FIRST_DATA].SetValidHighSpecLimit(lOk);
        }
    }
    bool IsUnitsLineType( const QString &aLine )
    {
        return aLine.startsWith("#Units", Qt::CaseInsensitive);
    }
    void ProcessUnitsLineType( const QString &aLine )
    {
        mCells.clear();
        mCells = aLine.split(",");
        if ((mParser.mTotalParameters+CSV_WOBURN_RAW_FIRST_DATA) > mCells.size())
        {
            mParser.mLastError = ParserBase::errInvalidFormatParameter;
            GSLOG(SYSLOG_SEV_ERROR,
                  QString("Not enough fields from input #Units lines (%1 instead of %2 expected)")
                  .arg(mCells.count()).arg(CSV_WOBURN_RAW_FIRST_DATA + mParser.mTotalParameters).toLatin1().constData());
        }
        for(gsuint16 i=CSV_WOBURN_RAW_FIRST_DATA; i<mParser.mTotalParameters+CSV_WOBURN_RAW_FIRST_DATA; ++i)
        {
            mParser.mParameters[i-CSV_WOBURN_RAW_FIRST_DATA].SetTestUnit(mCells[i]);
        }
    }
    bool IsLimitScalingLineType( const QString &aLine )
    {
        return aLine.startsWith("#LimitScaling", Qt::CaseInsensitive);
    }
    void ProcessLimitScalingLineType( const QString &aLine )
    {
        mCells.clear();
        mCells = aLine.split(",");
        if ((mParser.mTotalParameters+CSV_WOBURN_RAW_FIRST_DATA) > mCells.size())
        {
            mParser.mLastError = ParserBase::errInvalidFormatParameter;
            GSLOG(SYSLOG_SEV_ERROR,
                  QString("Not enough fields from input #LimitScaling lines (%1 instead of %2 expected)")
                  .arg(mCells.count()).arg(CSV_WOBURN_RAW_FIRST_DATA + mParser.mTotalParameters).toLatin1().constData());
        }
        for(gsuint16 i=CSV_WOBURN_RAW_FIRST_DATA; i<mParser.mTotalParameters+CSV_WOBURN_RAW_FIRST_DATA; ++i)
        {
            mParser.mParameters[i-CSV_WOBURN_RAW_FIRST_DATA].SetResultScale(mCells[i].trimmed().toInt());
        }
    }

private :
    QStringList mCells;
    TestHeaderIndexes mTestHeaderIndexes;
    MapHeaderInfos mMapHeaderInfos;
    QList<ParserBinning> mSoftBins, mHardBins;
    QTextStream &mStream;
    WoburnCSVtoSTDF &mParser;
};

//////////////////////////////////////////////////////////////////////
// Read and Parse the CSV file
//////////////////////////////////////////////////////////////////////
bool WoburnCSVtoSTDF::ConvertoStdf(const QString& lCsvFileName, QString& lFileNameSTDF)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString(" file %1").arg(lCsvFileName).toLatin1().constData());

    QString lString;

    // Open CSV file
    QFile lFile(lCsvFileName);
    if(!lFile.open( QIODevice::ReadOnly ))
    {
        // Failed Opening CSV file
        mLastError = errOpenFail;

        // Convertion failed.
        return false;
    }
    // Assign file I/O stream
    QTextStream lCsvFile(&lFile);

    // there are 19 line types
    // all must be read but can be unordered
    // read one line at a time and try each type one after another

    // GCORE-17610 - headers are now parsed independetly of their order in the input file
    WoburnHeaderParser lHeaderParser( lCsvFile, *this );
    lHeaderParser.ParseHeaderLines();

    // Write ML from HiLimit and LoLimit
    GS::Core::MultiLimitItem lMultiLimitItem;
    for(gsuint16 i=CSV_WOBURN_RAW_FIRST_DATA; i<mTotalParameters+CSV_WOBURN_RAW_FIRST_DATA; ++i)
    {
        bool lValidLowLimit = mParameters[i-CSV_WOBURN_RAW_FIRST_DATA].GetValidLowLimit();
        bool lValidHighLimit = mParameters[i-CSV_WOBURN_RAW_FIRST_DATA].GetValidHighLimit();
        if (lValidHighLimit || lValidLowLimit)
        {
            lMultiLimitItem.Clear();
            lMultiLimitItem.SetHardBin(1);
            lMultiLimitItem.SetSoftBin(1);
            if (lValidLowLimit)
                lMultiLimitItem.SetLowLimit(mParameters[i-CSV_WOBURN_RAW_FIRST_DATA].GetLowLimit());
            if (lValidHighLimit)
                lMultiLimitItem.SetHighLimit(mParameters[i-CSV_WOBURN_RAW_FIRST_DATA].GetHighLimit());
            mParameters[i-CSV_WOBURN_RAW_FIRST_DATA].AddMultiLimitItem(lMultiLimitItem,
                                                                       ParserParameter::KeepDuplicateMultiLimit);
        }
    }

    for(gsuint16 i=CSV_WOBURN_RAW_FIRST_DATA; i<mTotalParameters+CSV_WOBURN_RAW_FIRST_DATA; ++i)
    {
        bool lValidLowLimit = mParameters[i-CSV_WOBURN_RAW_FIRST_DATA].GetValidLowSpecLimit();
        bool lValidHighLimit = mParameters[i-CSV_WOBURN_RAW_FIRST_DATA].GetValidHighSpecLimit();
        if (lValidHighLimit || lValidLowLimit)
        {
            lMultiLimitItem.Clear();
            lMultiLimitItem.SetHardBin(2);
            lMultiLimitItem.SetSoftBin(2);
            if (lValidLowLimit)
                lMultiLimitItem.SetLowLimit(mParameters[i-CSV_WOBURN_RAW_FIRST_DATA].GetLowSpecLimit());
            if (lValidHighLimit)
                lMultiLimitItem.SetHighLimit(mParameters[i-CSV_WOBURN_RAW_FIRST_DATA].GetHighSpecLimit());
            mParameters[i-CSV_WOBURN_RAW_FIRST_DATA].AddMultiLimitItem(lMultiLimitItem,
                                                                       ParserParameter::KeepDuplicateMultiLimit);
        }
    }

    // All tests names read...check if need to update the csv Parameter list on disk
    if(mParameterDirectory.GetNewParameterFound() == true)
        mParameterDirectory.DumpParameterIndexTable();

    // Write the stdf
    if(WriteStdfFile(lCsvFile, lFileNameSTDF, lCsvFileName) != true)
    {
        QFile::remove(lFileNameSTDF);
        // Close file
        lFile.close();
        return false;
    }

    mLastError = errNoError;
    // Success parsing CSV file
    return true;
}



//////////////////////////////////////////////////////////////////////
// Create STDF file from CSV data parsed
//////////////////////////////////////////////////////////////////////
bool WoburnCSVtoSTDF::WriteStdfFile(QTextStream& lCsvFile, const QString& fileNameSTDF, const QString &aCsvFilePath)
{
    GQTL_STDF::Stdf_WIR_V4 lWIRRecord;
    time_t lFinishTime;
    time_t lDate;
    bool mLastDie(false);

    // now generate the STDF file...
    if(mStdfParse.Open(fileNameSTDF.toStdString().c_str(), STDF_WRITE) == false)
    {
        mLastError = errWriteSTDF;
        GSLOG(SYSLOG_SEV_ERROR, "Can not open the stdf file")
        return false;
    }

    // GCORE-17610 - there is no data in this file, nothing to process, automatic failure and log
    if( lCsvFile.atEnd() )
    {
        mStdfParse.Close();

        mLastError = errMissingData;
        mLastErrorMessage = QString( "There is no data in the input file : %1" ).arg( aCsvFilePath );
        GSLOG( SYSLOG_SEV_ERROR, mLastErrorMessage.toStdString().c_str() );

        return false;
    }

    QStringList lFields;
    QString lLine = ReadLine(lCsvFile);

    while (lLine.simplified() == "")
        lLine = ReadLine(lCsvFile);

    lFields = lLine.split(",");
    // Write MIR
    mMIRRecord.SetPART_TYP(lFields[CSV_WOBURN_RAW_PART_TYP]);
    mMIRRecord.SetLOT_ID(lFields[CSV_WOBURN_RAW_LOT_ID]);
    mMIRRecord.SetSBLOT_ID(lFields[CSV_WOBURN_RAW_LOT_ID]);
    mMIRRecord.SetOPER_NAM(lFields[CSV_WOBURN_RAW_OPERATOR]);
    mMIRRecord.SetNODE_NAM(lFields[CSV_WOBURN_RAW_NODE_NAME]);
    mMIRRecord.SetTST_TEMP(lFields[CSV_WOBURN_RAW_TST_TEMP]);
    mMIRRecord.SetJOB_NAM(lFields[CSV_WOBURN_RAW_JOB_NAM]);
    mMIRRecord.SetSPEC_NAM(lFields[CSV_WOBURN_RAW_SPEC_NAM]);
    mMIRRecord.SetFACIL_ID("Woburn");
    GetDate(lFields[CSV_WOBURN_RAW_DATE], lFields[CSV_WOBURN_RAW_TIME], lDate);
    mStartTime = lDate - (mPassedTime*60); // mPassedTime is in minutes
    lFinishTime = lDate;
    mMIRRecord.SetSTART_T(mStartTime);
    lWIRRecord.SetSTART_T(mStartTime);

    // Add undefined values in the cvs and mandatory to the STDF
    mMIRRecord.SetSETUP_T(mStartTime);
    // Construct custom Galaxy USER_TXT
//    QString	strUserTxt;
//    strUserTxt = GEX_IMPORT_DATAORIGIN_LABEL;
//    strUserTxt += ":";
//    strUserTxt += GEX_IMPORT_DATAORIGIN_ATETEST;
//    strUserTxt += ":WoburnCSV";
//    mMIRRecord.SetUSER_TXT(strUserTxt.toLatin1().constData());
    mStdfParse.WriteRecord(&mMIRRecord);


    // Write the list of DTR
    for (int i=0; i<mDTRMetaDataRecords.size(); ++i)
    {
        mStdfParse.WriteRecord(mDTRMetaDataRecords[i]);
    }

//    // Write SDR : For the moment, we don't write the SDR until they check the spec because the new format doesn't match
    GQTL_STDF::Stdf_SDR_V4 lSDRRecord;
    lSDRRecord.SetCARD_ID(lFields[CSV_WOBURN_RAW_CARD_ID]);
    mStdfParse.WriteRecord(&lSDRRecord);

    // Write WIR
    int lSiteNum = lFields[mSwBinCell + 5].toUInt();
    lWIRRecord.SetWAFER_ID(mWaferId);
    lWIRRecord.SetSITE_GRP(lSiteNum);
    mStdfParse.WriteRecord(&lWIRRecord);

    // Write Test results for each line read.
    QString strString;
    BYTE	lValidLimit;

    // Write all Parameters read on this wafer.: PTR....PTR, PRR
    do
    {
        if ((mTotalParameters+CSV_WOBURN_RAW_FIRST_DATA) > lFields.size())
        {
            mLastError = errInvalidFormatParameter;
            GSLOG(SYSLOG_SEV_WARNING,
                  QString("Not enough fields from input line (%1 instead of %2 expected)")
                  .arg(lFields.count()).arg(CSV_WOBURN_RAW_FIRST_DATA + mTotalParameters).toLatin1().constData());

            // reading the next line
            strString = ReadLine(lCsvFile);

            while (strString.trimmed().isEmpty() && !lCsvFile.atEnd())
            {
                strString = ReadLine(lCsvFile);
            }

            if (!strString.trimmed().isEmpty())
            {    // Split line if not empty
                lFields = strString.split(",",QString::KeepEmptyParts);
            }

            if(lCsvFile.atEnd() == true)
            {
               break;
            }

            continue;
        }
        // Write PIR
        lSiteNum = lFields[mSwBinCell + 5].toUInt();
        GQTL_STDF::Stdf_PIR_V4 lPIRRecord;
        lPIRRecord.SetHEAD_NUM(1);
        lPIRRecord.SetSITE_NUM(lSiteNum);
        mStdfParse.WriteRecord(&lPIRRecord);

        // Read Parameter results for this record
        int lpar = mTotalParameters;
        for(unsigned lIndex=0; lIndex<(unsigned)(lpar); ++lIndex)
        {
            // If it's a PTR (pattern name empty)
            GQTL_STDF::Stdf_PTR_V4 lPTRRecord;
            ParserParameter& lTest = mParameters[lIndex];
            lPTRRecord.SetTEST_NUM(lTest.GetTestNumber());
            lPTRRecord.SetHEAD_NUM(1);
            lPTRRecord.SetSITE_NUM(lSiteNum);

            // GCORE-8570 : HTH
            // Limit must not be strict. Measurament equal to the low or high limit must be considered as
            // PASS
            lPTRRecord.SetPARM_FLG(static_cast<stdf_type_b1>(0x40|0x80)); // B*1 Parametric test flags (drift, etc.)

            double lScalVal(1);
            bool ok;
            float lValue = lFields[CSV_WOBURN_RAW_FIRST_DATA + lIndex].toFloat(&ok);
            if (!ok
                || (lValue == 99) ||  (lValue == 999) ||  (lValue == 9999)
                || (lValue == -99) || (lValue == -999) || (lValue == -9999))
            {
                lPTRRecord.SetTEST_FLG(0x42);       // No result, no Pass/fail indication
                lPTRRecord.SetRESULT(0);
            }
            else
            {
                lPTRRecord.SetTEST_FLG(0x40);       // No result
                lPTRRecord.SetRESULT(lValue);
            }
            if(lTest.GetStaticHeaderWritten() == false)
            {
                lPTRRecord.SetTEST_TXT(lTest.GetTestName());
                int lScale = lTest.GetResultScale();
                lPTRRecord.SetRES_SCAL(lScale);
                lPTRRecord.SetLLM_SCAL(lScale);
                lPTRRecord.SetHLM_SCAL(lScale);
                lValidLimit = 0x02;
                if(lTest.GetValidLowLimit() == false)
                    lValidLimit |=0x40;
                if(lTest.GetValidHighLimit() == false)
                    lValidLimit |=0x80;
                if(lTest.GetValidHighSpecLimit() == false)
                    lValidLimit |=0x08;
                if(lTest.GetValidLowSpecLimit() == false)
                    lValidLimit |=0x04;
                lPTRRecord.SetOPT_FLAG(lValidLimit);
                lScalVal = pow(10, -lScale);
                lPTRRecord.SetLO_LIMIT(lTest.GetLowLimit() * lScalVal);
                lPTRRecord.SetHI_LIMIT(lTest.GetHighLimit() * lScalVal);
                lPTRRecord.SetLO_SPEC(lTest.GetLowSpecLimit() * lScalVal);
                lPTRRecord.SetHI_SPEC(lTest.GetHighSpecLimit() * lScalVal);
                lPTRRecord.SetUNITS(lTest.GetTestUnits());
                lTest.SetStaticHeaderWritten(true);
            }
            mStdfParse.WriteRecord(&lPTRRecord);



            if(!lTest.GetStaticMultiLimitsWritten())
            {
                for (int lMLIndex = 0; lMLIndex < lTest.GetMultiLimitCount(); ++lMLIndex)
                {
                     GQTL_STDF::Stdf_DTR_V4 lDTRRecord;
                     QJsonObject            lMultiLimit;
                     GS::Core::MultiLimitItem&   lMLSet = lTest.GetMultiLimit(lMLIndex);
                     lMLSet.SetSite(lSiteNum);
                     lMLSet.SetHighLimit(lMLSet.GetHighLimit() * lScalVal);
                     lMLSet.SetLowLimit(lMLSet.GetLowLimit() * lScalVal);
                     lMLSet.CreateJsonFromMultiLimit(lMultiLimit, (int)lTest.GetTestNumber());

                     lDTRRecord.SetTEXT_DAT(lMultiLimit);
                     if (mStdfParse.WriteRecord(&lDTRRecord) == false)
                     {
                         GSLOG(SYSLOG_SEV_ERROR,
                               QString("Failed to write multi-limits DTR").toLatin1().constData());
                         mLastError = errWriteSTDF;
                         return false;
                     }
                }
                lTest.SetStaticMultiLimitsWritten(true);
            }
        };

        // Write
        GQTL_STDF::Stdf_PRR_V4 lPRRRecord;
        gsuint16 lHardBin = lFields[mSwBinCell+1].toUInt();
        GetDate(lFields[CSV_WOBURN_RAW_DATE], lFields[CSV_WOBURN_RAW_TIME], lDate);
        lPRRRecord.SetTEST_T(lDate - lFinishTime);
        lFinishTime = lDate;
        lPRRRecord.SetSITE_NUM(lSiteNum);
        lPRRRecord.SetHARD_BIN(lHardBin);
        if (mHardBins.find(lHardBin) != mHardBins.end())
        {
            ParserBinning& lHBin = mHardBins[lHardBin];
            lHBin.IncrementBinCount(1);
        }
        else
        {
            ParserBinning lParserBinning;
            lParserBinning.SetPassFail(false);
            lParserBinning.SetBinName(QString("Bin") + QString::number(lHardBin));
            lParserBinning.SetBinNumber(lHardBin);
            lParserBinning.SetBinCount(1);
            mHardBins.insert(lHardBin, lParserBinning);
        }

        gsint16 lXCoord = lFields[mSwBinCell + 3].toInt();
        gsint16 lYCoord = lFields[mSwBinCell + 4].toInt();
        lPRRRecord.SetX_COORD(lXCoord);
        lPRRRecord.SetY_COORD(lYCoord);
        gsuint16 lSoftBin = lFields[mSwBinCell].toUShort();
        lPRRRecord.SetSOFT_BIN(lSoftBin);
        if (mSoftBins.find(lSoftBin) != mSoftBins.end())
        {
            ParserBinning& lSBin = mSoftBins[lSoftBin];
            lSBin.IncrementBinCount(1);
        }
        else
        {
            ParserBinning lParserBinning;
            lParserBinning.SetPassFail(false);
            lParserBinning.SetBinName(QString("Bin") + QString::number(lSoftBin));
            lParserBinning.SetBinNumber(lSoftBin);
            lParserBinning.SetBinCount(1);
            mSoftBins.insert(lSoftBin, lParserBinning);
        }


        QMap<int,ParserBinning>::iterator lIter = mHardBins.find(lHardBin);
        if(lIter != mHardBins.end())
        {
            if(lIter.value().GetPassFail())
            {
                lPRRRecord.SetPART_FLG(0x00);
            }
            else
            {
                lPRRRecord.SetPART_FLG(0x08);
            }
        }
        else
        {
            lPRRRecord.SetPART_FLG(0x08);
        }



        // Write some requested fields by the customer for internal tools
        lPRRRecord.SetPART_ID(lFields[CSV_WOBURN_RAW_MAPID]);

        mStdfParse.WriteRecord(&lPRRRecord);

        if (WriteReticleDTR(lFields, lXCoord, lYCoord) == false)
            return false;

        if (mLastDie == true)
            break;
        // Read lines while empty
        strString = ReadLine(lCsvFile);
        while (strString.trimmed().isEmpty() && !lCsvFile.atEnd())
        {
            strString = ReadLine(lCsvFile);
        }

        if (!strString.trimmed().isEmpty())
        {    // Split line if not empty
            lFields = strString.split(",",QString::KeepEmptyParts);
        }
        if(lCsvFile.atEnd() == true)
            mLastDie = true;
    }
    while(true); // Read all lines in file

    // Write WRR and WCR
    mWRRRecord.SetSITE_GRP(lSiteNum);
    mWRRRecord.SetFINISH_T(lFinishTime);
    mStdfParse.WriteRecord(&mWRRRecord);
    mStdfParse.WriteRecord(&mWCRRecord);


    // Write HBR
    GQTL_STDF::Stdf_HBR_V4 lHBRRecord;
    lHBRRecord.SetHEAD_NUM(255);
    lHBRRecord.SetSITE_NUM(1);
    foreach(const int i, mHardBins.keys())
    {
        lHBRRecord.SetHBIN_NUM(mHardBins[i].GetBinNumber());
        lHBRRecord.SetHBIN_NAM(mHardBins[i].GetBinName());
        if(mHardBins[i].GetPassFail())
            lHBRRecord.SetHBIN_PF('P');
        else
            lHBRRecord.SetHBIN_PF('F');
        lHBRRecord.SetHBIN_CNT(mHardBins[i].GetBinCount());
        mStdfParse.WriteRecord(&lHBRRecord);
    }

    // Write SBR
    GQTL_STDF::Stdf_SBR_V4 lSBRRecord;
    lSBRRecord.SetHEAD_NUM(255);
    lSBRRecord.SetSITE_NUM(1);
    foreach(const int i, mSoftBins.keys())
    {
        lSBRRecord.SetSBIN_NUM(mSoftBins[i].GetBinNumber());
        lSBRRecord.SetSBIN_NAM(mSoftBins[i].GetBinName());
        if(mHardBins[i].GetPassFail())
            lSBRRecord.SetSBIN_PF('P');
        else
            lSBRRecord.SetSBIN_PF('F');
        lSBRRecord.SetSBIN_CNT(mSoftBins[i].GetBinCount());
        mStdfParse.WriteRecord(&lSBRRecord);
    }

    // Write TSR
    GQTL_STDF::Stdf_TSR_V4 lTSRRecord;
    lTSRRecord.SetHEAD_NUM(255);
    lTSRRecord.SetSITE_NUM(1);
    for (gsint16 lIndex=0; lIndex<mParameters.size(); ++lIndex)
    {
        ParserParameter lTest = mParameters[lIndex];
        lTSRRecord.SetTEST_TYP('P');
        lTSRRecord.SetTEST_NUM(lTest.GetTestNumber());
//        lTSRRecord.SetEXEC_CNT(lTest.GetExecCount());
//        lTSRRecord.SetFAIL_CNT(lTest.GetExecFail());
        lTSRRecord.SetTEST_NAM(lTest.GetTestName());
        mStdfParse.WriteRecord(&lTSRRecord);
    }

    // Write PCR
    mStdfParse.WriteRecord(&mPCRRecord);

    // Write MRR
    GQTL_STDF::Stdf_MRR_V4 lMRRRecord;
    // use the last date in the csv file
    lMRRRecord.SetFINISH_T(lFinishTime);
    lMRRRecord.SetDISP_COD(' ');
    mStdfParse.WriteRecord(&lMRRRecord);

    // Close STDF file.
    mStdfParse.Close();

    // Success
    return true;
}


//////////////////////////////////////////////////////////////////////
// Skip empty line
//////////////////////////////////////////////////////////////////////
void  WoburnCSVtoSTDF::SpecificReadLine (QString &strString)
{
    if(strString.left(3) == ",,," && (strString.simplified().count(",")==strString.simplified().length()))
        strString = "";
}

//////////////////////////////////////////////////////////////////////
// Check if the line is empty or contains only ','
//////////////////////////////////////////////////////////////////////
bool WoburnCSVtoSTDF::EmptyLine(const QString& line)
{
    bool lEmpty(true);
    if (!line.isEmpty())
    {
        QStringList lCells = line.split(",", QString::KeepEmptyParts);
        for(int lIndexCell=0; lIndexCell<lCells.count(); ++lIndexCell)
        {
            if (!lCells[lIndexCell].isEmpty())
            {
                lEmpty = false;
                break;
            }
        }
    }
    return lEmpty;
}

gsbool WoburnCSVtoSTDF::AddMetaDataToDTR(QString key, QString fieldValue)
{
    GQTL_STDF::Stdf_DTR_V4* lDTRRecord = new GQTL_STDF::Stdf_DTR_V4();
    ParserBase::AddMetaDataToDTR(key,fieldValue,lDTRRecord);
    mDTRMetaDataRecords.append(lDTRRecord);
    return true;
}


gsbool WoburnCSVtoSTDF::GetDate(const QString& dateString, const QString& timeString, time_t& date)
{
    QStringList lDateList = dateString.split('-');
    QDate lDate;
    // 07-22-2015
    if (lDateList.count() != 3)
    {
         lDateList = dateString.split('/');
    }
    if (lDateList.count() == 3)
    {
        int lYear = lDateList[2].toInt();
        if (lYear < 70)
            lYear += 2000;
        else if (lYear < 99)
            lYear += 1900;

        lDate.setDate(lYear, lDateList[0].toInt(), lDateList[1].toInt());
    }
    else
    {
        return false;
    }
    QTime lTime;
    QString lTimeString = timeString.section(" ", 0, 0);
    if (lTimeString.size() == 8)
       lTime = QTime::fromString(lTimeString, "hh:mm:ss");
    else
        lTime = QTime::fromString(lTimeString, "h:mm:ss");
    date = QDateTime(lDate, lTime,Qt::UTC).toTime_t();
    return true;
}

// The Json syntaxe
//{
//"TYPE":"RETICLE",
//"DIEX":integer,      // Die X coordinate on the map
//"DIEY":integer,      // Die Y coordinate on the map
//"RETX":integer,     // X coordinate within the reticle
//"RETY": integer     // Y coordinate within the reticle
//}
bool WoburnCSVtoSTDF::WriteReticleDTR(const QStringList& fields,
                                       const gsint16 xCoord,
                                       const gsint16 yCoord)
{
    if (fields.count() < CSV_WOBURN_RAW_FIRST_DATA)
    {
        mLastError = errInvalidFormatParameter;
        return false;
    }

    gsbool lOk(false);

    // Extract Pos X and PosY of the reticle on the map from column X and Y
    gsint8 lPosX = fields.at(CSV_WOBURN_RAW_DIEX).toInt(&lOk);
    if (!lOk)
    {
        mLastError = errInvalidFormatParameter;

        return false;
    }

    gsint8 lPosY = fields.at(CSV_WOBURN_RAW_DIEY).toInt(&lOk);
    if (!lOk)
    {
        mLastError = errInvalidFormatParameter;
        return false;
    }

    // Extract X and Y coordinate inside the reticle from the DUT Number
    // DUT Number format is YYXX or YXX
    gsint8 lRetX(0), lRetY(0);
    gsint16 lDutNumber = fields.at(CSV_WOBURN_RAW_DUT).toInt(&lOk);
    if (lOk)
    {
        lRetY = lDutNumber / 100;
        lRetX = lDutNumber % 100;
    }
    else
    {
        mLastError = errInvalidFormatParameter;
        return false;
    }

    GQTL_STDF::Stdf_DTR_V4 lDTRRecord;
    QJsonObject            lReticleField;
    lReticleField.insert("TYPE", QJsonValue(QString("reticle")));
    lReticleField.insert("DIEX", QJsonValue(xCoord));
    lReticleField.insert("DIEY", QJsonValue((yCoord)));
    lReticleField.insert("RETX", QJsonValue((lRetX)));
    lReticleField.insert("RETY", QJsonValue((lRetY)));
    lReticleField.insert("RETPOSX", QJsonValue((lPosX)));
    lReticleField.insert("RETPOSY", QJsonValue((lPosY)));
    lDTRRecord.SetTEXT_DAT(lReticleField);
    mStdfParse.WriteRecord(&lDTRRecord);
    return true;
}

}
}
