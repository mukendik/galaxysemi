#include "QFile"

#include "importWatSmic.h"

// File format: (CSV delimited)
//########
//SMIC....,WAT
//PRODUCT.,0496E
//SMIC-LOT,B50950
//CUST-LOT,B50950
//TSTRNAME,F2WAT04
//STARTTIM,05/18/2006
//MANPRREV,0496_0
//OPERATOR,E007389
//RAWDATA.,T-WAFER,25
//RAWDATA.,WAFER, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25
//RAWDATA.,CHIP,5
//RAWDATA.,TOTAL-ITEM,49
//RAWDATA.,ITEM-NAME,WAFER,SITE,VTI_NAA18_10_D18,IDSAT_NAA18_10_D18,IOFF_NAA18_10_D18,BVDS_...
//RAWDATA.,UNIT,      ,,           V,       UA/UM,       PA/UM,           V,      OHM/SQ,  ...
//RAWDATA.,LOW-LIMIT, ,,   3.200E-01,   5.100E+02,   0.000E+00,   3.600E+00,   7.950E+02,  ...
//RAWDATA.,HIGH-LIMIT,,,   5.200E-01,   6.900E+02,   3.000E+02,   1.200E+01,   1.100E+03,  ...
//RAWDATA.,LCL,       ,,   3.200E-01,   5.100E+02,   0.000E+00,   3.600E+00,   7.950E+02,  ...
//RAWDATA.,UCL,       ,,   5.200E-01,   6.900E+02,   3.000E+02,   1.200E+01,   1.100E+03,  ...
//RAWDATA.,AVERAGE,   ,,   4.508E-01,   5.811E+02,   1.368E+01,   3.998E+00,   9.264E+02,  ...
//RAWDATA.,STD DEV,   ,,   3.651E-03,   1.105E+01,   2.091E+00,   1.393E-02,   5.258E+00,  ...
//RAWDATA.,DATA,  1,  1,   4.520E-01,   5.734E+02,   1.213E+01,   3.990E+00,   9.301E+02,  ...
//RAWDATA.,DATA,  1,  2,   4.510E-01,   5.736E+02,   1.264E+01,   3.990E+00,   9.302E+02,  ...
//

namespace GS
{
namespace Parser
{

WatSmicToStdf::WatSmicToStdf():WatToStdf(typeWatSMic, "WatSmic")
{
    mParameterDirectory.SetFileName(GEX_WAT_SMIC_PARAMETERS);
}

WatSmicToStdf::~WatSmicToStdf()
{
}

//////////////////////////////////////////////////////////////////////
// Check if File is compatible with WAT_SMIC format
//////////////////////////////////////////////////////////////////////
bool WatSmicToStdf::IsCompatible(const QString &szFileName)
{
    QString strString;

    // Open hCsmFile file
    QFile f( szFileName );
    if(!f.open( QIODevice::ReadOnly ))
    {
        // Failed Opening ASL1000 file
        return false;
    }
    // Assign file I/O stream
    QTextStream hLWatSmicFile(&f);

    // Check if first line is the correct WAT_SMIC header...
    do
        strString = hLWatSmicFile.readLine();
    while(!strString.isNull() && strString.isEmpty());

    if(strString.startsWith("########") == false)
    {
        // Incorrect header...this is not a WAT_SMIC file!
        // Close file
        f.close();
        return false;
    }
    do
        strString = hLWatSmicFile.readLine();
    while(!strString.isNull() && strString.isEmpty());

    if(strString.startsWith("SMIC....,WAT", Qt::CaseInsensitive) == false)
    {
        // Incorrect header...this is not a WAT_SMIC file!
        // Close file
        f.close();
        return false;
    }

    f.close();

    return true;
}

//////////////////////////////////////////////////////////////////////
// Read and Parse the WAT_SMIC file
//////////////////////////////////////////////////////////////////////
bool WatSmicToStdf::ParseFile(const QString &fileName)
{
    QString strString;
    QString strSection;
    bool	bStatus = true;
    int		iIndex;				// Loop index

    // Open CSV file
    mFile.setFileName(fileName);
    if(!mFile.open( QIODevice::ReadOnly ))
    {
        mLastError = errOpenFail;
        return false;
    }

    mWatSmicFile.setDevice(&mFile);
    // Check if first line is the correct WAT_SMIC header...
    strString = ReadLine(mWatSmicFile);
    if(strString.startsWith("########") == false)
    {
        // Incorrect header...this is not a WAT_SMIC file!
        mLastError = errInvalidFormatParameter;
        mFile.close();
        return false;
    }
    strString = ReadLine(mWatSmicFile);
    if(strString.startsWith("SMIC....,WAT", Qt::CaseInsensitive) == false)
    {
        // Incorrect header...this is not a WAT_SMIC file!
        mLastError = errInvalidFormatParameter;
        mFile.close();
        return false;
    }

    // extract SMIC_WAT information
    //PRODUCT.,0496E
    //SMIC-LOT,B50950
    //CUST-LOT,B50950
    //TSTRNAME,F2WAT04
    //STARTTIM,05/18/2006
    //MANPRREV,0496_0
    //OPERATOR,E007389
    //RAWDATA.,T-WAFER,25
    while(mWatSmicFile.atEnd() == false)
    {
        strString = ReadLine(mWatSmicFile);

        strSection = strString.section(",",0,0).simplified().toUpper();
        strString = strString.section(",",1);

        if(strSection=="PRODUCT.")
        {
            mProductID = strString.simplified();
            strString = "";
        }
        else if(strSection=="SMIC-LOT")
        {
            mLotID = strString.simplified();
            strString = "";
        }
        else if(strSection=="CUST-LOT")
        {
            mLotID = strString.simplified();
            strString = "";
        }
        else if(strSection=="TSTRNAME")
        {
            mTesterType = strString.simplified();
            strString = "";
        }
        else if(strSection=="STARTTIM")
        {
            QDate clDate(strString.section("/",2,2).simplified().toInt(),strString.section("/",0,0).simplified().toInt(),strString.section("/",1,1).simplified().toInt());
            QTime clTime;
            QDateTime clDateTime(clDate,clTime);
            clDateTime.setTimeSpec(Qt::UTC);
            mStartTime  = clDateTime.toTime_t();
            strString   = "";
        }
        else if(strSection=="MANPRREV")
        {
            mProgRev = strString.simplified();
            strString = "";
        }
        else if(strSection=="OPERATOR")
        {
            mOperatorName = strString.simplified();
            strString = "";
        }
        else if(strSection=="RAWDATA.")
            break;
        else
            strString = "";
    }

    // Skip line
    //RAWDATA.,WAFER, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25
    //RAWDATA.,CHIP,5
    //RAWDATA.,TOTAL-ITEM,49
    // and goto
    //RAWDATA.,ITEM-NAME,WAFER,SITE,VTI_NAA18_10_D18,IDSAT_NAA18_10_D18,IOFF_NAA18_10...
    while(mWatSmicFile.atEnd() == false)
    {
        strString = ReadLine(mWatSmicFile);

        strSection = strString.section(",",0,0).simplified().toUpper();
        strString = strString.section(",",1);

        if(strSection!="RAWDATA.")
        {
            // Incorrect header...this is not a WAT_SMIC file!
            mLastError = errInvalidFormatParameter;
            mFile.close();
            return false;
        }
        strSection = strString.section(",",0,0).simplified().toUpper();
        if(strSection=="ITEM-NAME")
        {
            // have found the parameter description
            // skip the 2 other column description to have only parameter name
            strString = strString.section(",",3);
            // Count the number of parameters specified in the line
            mTotalParameters = strString.count(",")+1;
            break;
        }
    }

    // If no parameter specified...ignore!
    if(mTotalParameters <= 0)
    {
        // Incorrect header...this is not a valid WAT_SMIC file!
        mLastError = errInvalidFormatParameter;
        mFile.close();
        return false;
    }

    // Allocate the buffer to hold the N parameters & results.
    //m_pCGWatSmicParameter = new CGWatSmicParameter[m_iTotalParameters];	// List of parameters

    QStringList	lstSections = strString.split(",",QString::KeepEmptyParts);
    // Check if have the good count
    if(lstSections.count() < mTotalParameters)
    {
        mLastError = errParserSpecific;
        return false;
    }

    // Extract the N column names
    //RAWDATA.,ITEM-NAME,WAFER,SITE,VTI_NAA18_10_D18,IDSAT_NAA18_10_D18,IOFF_NAA18_10_D18,BVDS_NAA18_10_D18,RSFV_NWST_20_5,RSFV_NDF_D22_455,VTI_PAA18_10_D18,IDSAT_PAA18_10_D18,IOFF_PAA18_10_D18,BVDS_PAA18_10_D18,RSFV_PDF_D22_455,VTFM1_N_10_D28,VTI_NAA33_10_D35,IDSAT_NAA33_10_D35,IOFF_NAA33_10_D35,BVDS_NAA33_10_D35,VTFM1_P_10_D28,VTI_PAA33_10_D3,IDSAT_PAA33_10_D3,IOFF_PAA33_10_D3,BVDS_PAA33_10_D3,RSFV_NP_D18_556,RCFV_NDF_D22_902,RCFV_NP_D22_902,RCFV_V1_D26_3600,RCFV_V2_D26_3600,CONTI_NP_D18,CONTI_M1_D23,CONTI_M2_D28,CONTI_M3_D28,SPAFI_NP_D25,SPAFI_M1_D23,SPAFI_M2_D28,SPAFI_M3_D28,RSFV_PP_D18_556,RCFV_PDF_D22_902,RCFV_PP_D22_902,RCFV_V3_D26_3600,RCFV_V4_D36_3600,CONTI_PP_D18,CONTI_M4_D28,CONTI_M5_D44,SPAFI_PP_D25,SPAFI_M4_D28,SPAFI_M5_D46,VBG_NB18_70_100,VBG_NB33_70_100,VBG_PB18_70_100,VBG_PB33_70_100
    for(iIndex=0; iIndex<mTotalParameters; ++iIndex)
    {
        WatScaledParameter* lWatSmicParameter =  new WatScaledParameter();
        strSection      = lstSections[iIndex];
        strSection      = strSection.trimmed();	// Remove spaces
        lWatSmicParameter->SetTestName(strSection);
        mParameterDirectory.UpdateParameterIndexTable(strSection);		// Update Parameter master list if needed.
        lWatSmicParameter->SetStaticHeaderWritten (false);
        mSmicParameterList.insert(iIndex, lWatSmicParameter);
    }

    //RAWDATA.,UNIT,      ,,           V,       UA/UM,       PA/UM,           V,      OHM/SQ,      OHM/SQ,           V,       UA/UM,       PA/UM,           V,      OHM/SQ,           V,           V,       UA/UM,       PA/UM,           V,           V,           V,       UA/UM,       PA/UM,           V,      OHM/SQ,   OHM/COUNT,   OHM/COUNT,   OHM/COUNT,   OHM/COUNT,        KOHM,         OHM,         OHM,         OHM,           V,           V,           V,           V,      OHM/SQ,   OHM/COUNT,   OHM/COUNT,   OHM/COUNT,   OHM/COUNT,        KOHM,         OHM,         OHM,           V,           V,           V,           V,           V,           V,           V
    //RAWDATA.,LOW-LIMIT, ,,   3.200E-01,   5.100E+02,   0.000E+00,   3.600E+00,   7.950E+02,   1.000E+00,  -6.000E-01,  -3.000E+02,  -3.000E+02,  -1.200E+01,   1.000E+00,   3.600E+00,   6.200E-01,   5.100E+02,   0.000E+00,   6.000E+00,  -1.000E+01,  -7.800E-01,  -3.450E+02,  -3.000E+02,  -2.000E+01,   1.000E+00,   1.000E+00,   1.000E+00,   1.000E+00,   1.000E+00,   2.000E+01,   2.000E+02,   2.000E+02,   2.000E+02,   7.000E+00,   7.000E+00,   7.000E+00,   7.000E+00,   1.000E+00,   1.000E+00,   1.000E+00,   1.000E+00,   1.000E+00,   2.000E+01,   2.000E+02,   5.000E+01,   7.000E+00,   7.000E+00,   7.000E+00,  -1.000E+01,  -1.500E+01,   4.600E+00,   8.000E+00
    //RAWDATA.,HIGH-LIMIT,,,   5.200E-01,   6.900E+02,   3.000E+02,   1.200E+01,   1.100E+03,   1.000E+01,  -4.000E-01,  -2.200E+02,   0.000E+00,  -3.600E+00,   1.000E+01,   1.000E+01,   8.200E-01,   6.900E+02,   3.000E+02,   2.000E+01,  -3.600E+00,  -5.800E-01,  -2.550E+02,   0.000E+00,  -6.000E+00,   1.500E+01,   3.000E+01,   3.000E+01,   1.000E+01,   1.000E+01,   1.200E+02,   1.000E+03,   8.000E+02,   8.000E+02,   2.000E+01,   2.000E+01,   2.000E+01,   2.000E+01,   1.500E+01,   3.000E+01,   3.000E+01,   1.000E+01,   8.000E+00,   1.400E+02,   8.000E+02,   2.000E+02,   2.000E+01,   2.000E+01,   2.000E+01,  -4.600E+00,  -8.000E+00,   1.000E+01,   1.500E+01
    //RAWDATA.,LCL,       ,,   3.200E-01,   5.100E+02,   0.000E+00,   3.600E+00,   7.950E+02,   1.000E+00,  -6.000E-01,  -3.000E+02,  -3.000E+02,  -1.200E+01,   1.000E+00,   3.600E+00,   6.200E-01,   5.100E+02,   0.000E+00,   6.000E+00,  -1.000E+01,  -7.800E-01,  -3.450E+02,  -3.000E+02,  -2.000E+01,   1.000E+00,   1.000E+00,   1.000E+00,   1.000E+00,   1.000E+00,   2.000E+01,   2.000E+02,   2.000E+02,   2.000E+02,   7.000E+00,   7.000E+00,   7.000E+00,   7.000E+00,   1.000E+00,   1.000E+00,   1.000E+00,   1.000E+00,   1.000E+00,   2.000E+01,   2.000E+02,   5.000E+01,   7.000E+00,   7.000E+00,   7.000E+00,  -1.000E+01,  -1.500E+01,   4.600E+00,   8.000E+00
    //RAWDATA.,UCL,       ,,   5.200E-01,   6.900E+02,   3.000E+02,   1.200E+01,   1.100E+03,   1.000E+01,  -4.000E-01,  -2.200E+02,   0.000E+00,  -3.600E+00,   1.000E+01,   1.000E+01,   8.200E-01,   6.900E+02,   3.000E+02,   2.000E+01,  -3.600E+00,  -5.800E-01,  -2.550E+02,   0.000E+00,  -6.000E+00,   1.500E+01,   3.000E+01,   3.000E+01,   1.000E+01,   1.000E+01,   1.200E+02,   1.000E+03,   8.000E+02,   8.000E+02,   2.000E+01,   2.000E+01,   2.000E+01,   2.000E+01,   1.500E+01,   3.000E+01,   3.000E+01,   1.000E+01,   8.000E+00,   1.400E+02,   8.000E+02,   2.000E+02,   2.000E+01,   2.000E+01,   2.000E+01,  -4.600E+00,  -8.000E+00,   1.000E+01,   1.500E+01
    //RAWDATA.,AVERAGE,   ,,   4.508E-01,   5.811E+02,   1.368E+01,   3.998E+00,   9.264E+02,   4.850E+00,  -5.318E-01,  -2.352E+02,  -4.066E+00,  -5.243E+00,   5.124E+00,   1.000E+01,   6.764E-01,   6.153E+02,   1.126E+00,   8.943E+00,  -1.000E+01,  -6.268E-01,  -3.107E+02,  -2.173E+00,  -7.526E+00,   5.515E+00,   4.032E+00,   3.776E+00,   3.544E+00,   3.487E+00,   5.316E+01,   4.972E+02,   4.335E+02,   4.317E+02,   2.000E+01,   2.000E+01,   2.000E+01,   1.259E+01,   7.330E+00,   3.779E+00,   3.712E+00,   3.060E+00,   1.991E+00,   7.871E+01,   4.083E+02,   9.880E+01,   2.000E+01,   2.000E+01,   2.000E+01,  -5.846E+00,  -9.989E+00,   7.133E+00,   1.294E+01
    //RAWDATA.,STD DEV,   ,,   3.651E-03,   1.105E+01,   2.091E+00,   1.393E-02,   5.258E+00,   1.219E-01,   6.617E-03,   5.680E+00,   6.741E-01,   1.803E-02,   1.246E-01,   0.000E+00,   6.103E-03,   4.184E+00,   2.265E-01,   3.721E-02,   0.000E+00,   3.558E-03,   2.232E+00,   3.767E-01,   3.455E-02,   1.753E-01,   3.402E-01,   2.921E-01,   1.193E-01,   2.619E-01,   1.118E+00,   9.100E+00,   8.658E+00,   7.819E+00,   0.000E+00,   0.000E+00,   0.000E+00,   9.164E-02,   1.493E-01,   3.302E-01,   2.981E-01,   1.484E-01,   9.143E-02,   1.320E+00,   8.570E+00,   5.236E+00,   0.000E+00,   0.000E+00,   0.000E+00,   9.693E-02,   1.473E-01,   3.887E-01,   3.377E-01
    // With 2nd file handle: seek to the end of the file to read the Spec Limits.
    int	iLimits =0;
    while(mWatSmicFile.atEnd() == false)
    {
        strString = ReadLine(mWatSmicFile);
        if(strString.startsWith("RAWDATA.,UNIT", Qt::CaseInsensitive) == true)
        {
            // found the UNIT limits
            lstSections = strString.split(",",QString::KeepEmptyParts);
            // Extract the N column toUpper Limits
            for(iIndex=0; iIndex< mTotalParameters; ++iIndex)
            {
                strSection = "";
                if(lstSections.count() > iIndex+4)
                    strSection = lstSections[iIndex+4];
                mSmicParameterList[iIndex]->SetTestUnit(strSection.simplified());
            }
        }
        else if(strString.startsWith("RAWDATA.,HIGH-LIMIT", Qt::CaseInsensitive) == true)
        {
            // found the HIGH limits
            iLimits |= 1;

            lstSections = strString.split(",",QString::KeepEmptyParts);
            // Extract the N column toUpper Limits
            for(iIndex=0;iIndex<mTotalParameters;iIndex++)
            {
                strSection = "";
                if(lstSections.count() > iIndex+4)
                    strSection = lstSections[iIndex+4];
                mSmicParameterList[iIndex]->SetHighLimit(strSection.toFloat(&bStatus));
                mSmicParameterList[iIndex]->SetValidHighLimit(bStatus);
            }
        }
        else if(strString.startsWith("RAWDATA.,LOW-LIMIT", Qt::CaseInsensitive) == true)
        {
            // found the Low limits
            iLimits |= 2;

            lstSections = strString.split(",", QString::KeepEmptyParts);
            // Extract the N column Lower Limits
            for(iIndex=0;iIndex<mTotalParameters;iIndex++)
            {
                strSection = "";
                if(lstSections.count() > iIndex+4)
                    strSection = lstSections[iIndex+4];
                mSmicParameterList[iIndex]->SetLowLimit(strSection.toFloat(&bStatus));
                mSmicParameterList[iIndex]->SetValidLowLimit(bStatus);
            }
        }
        else if(strString.startsWith("RAWDATA.,LCL", Qt::CaseInsensitive) == true)
            continue;
        else if(strString.startsWith("RAWDATA.,UCL", Qt::CaseInsensitive) == true)
            continue;
        else if(strString.startsWith("RAWDATA.,AVERAGE", Qt::CaseInsensitive) == true)
            continue;
        else if(strString.startsWith("RAWDATA.,STD DEV", Qt::CaseInsensitive) == true)
            break;
    }

    if(iLimits != 3)
    {
        // Incorrect header...this is not a valid WAT_SMIC file!: we didn't find the limits!
        mLastError = errParserSpecific;

        // Convertion failed.
        return false;
    }

    // Normalize all Limits
    for(iIndex=0;iIndex<mTotalParameters;iIndex++)
    {
         NormalizeLimits(mSmicParameterList[iIndex]);
    }


    // All WAT_SMIC file read...check if need to update the WAT_SMIC Parameter list on disk?
    if(mParameterDirectory.GetNewParameterFound())
        mParameterDirectory.DumpParameterIndexTable();

    // Success parsing WAT_SMIC file
    return true;
}

//////////////////////////////////////////////////////////////////////
// Create STDF file from WAT_SMIC data parsed
//////////////////////////////////////////////////////////////////////
bool WatSmicToStdf::WriteStdfFile(const char *stdFileName)
{
    // now generate the STDF file...
    if(mStdfParse.Open(stdFileName ,STDF_WRITE) == false)
    {
        mLastError = errWriteSTDF;
        return false;
    }

    if(mStartTime <= 0)
        mStartTime = QDateTime::currentDateTime().toTime_t();

    // Write MIR
    GQTL_STDF::Stdf_MIR_V4 lMIRV4Record;
    mTestCode= "WAFER";
    mFamilyID = mProductID;
    SetMIRRecord(lMIRV4Record, ":WAT_SMIC");
    mStdfParse.WriteRecord(&lMIRV4Record);

    // Write Test results for each line read.
    QString strString;
    QString strSection;
    QString	strWaferID;
    float	fValue;				// Used for readng floating point numbers.
    int		value_scale;		// Scale factor for limots & results.
    int		iIndex;				// Loop index
    int		iSiteNumber;
    BYTE	bData;
    WORD	wSoftBin,wHardBin;
    long	iTotalGoodBin = 0,iTotalFailBin = 0;
    long	iTestNumber = 0,iTotalTests = 0,iPartNumber = 0;
    bool	bStatus,bPassStatus;

    GQTL_STDF::Stdf_WRR_V4 lWRRV4Record;
    GQTL_STDF::Stdf_WIR_V4 lWIRV4Record;
    GQTL_STDF::Stdf_PIR_V4 lPIRV4Record;
    // Write all Parameters read on this wafer.: WIR.PIR,PTR....PTR, PRR, PIR, PTR,PTR...PRR,   ... WRR
    while(mWatSmicFile.atEnd() == false)
    {
        // Part number
        ++iPartNumber;

        // Read line
        strString = ReadLine(mWatSmicFile);

        if(strString.startsWith("########"))
            break;	// Reach the end of valid data records...

        if(!strString.startsWith("RAWDATA.,DATA", Qt::CaseInsensitive))
            break;	// Reach the end of valid data records...

        strString = strString.section(",",1);

        if(!strString.startsWith("DATA", Qt::CaseInsensitive))
            break;	// Reach the end of valid data records...

        // Extract WaferID
        strSection = strString.section(',',1,1).simplified();
        if(strSection != strWaferID)
        {
            // Write WRR in case we have finished to write wafer records.
            if(strWaferID.isEmpty() == false)
            {
                // WRR
                SetWRRRecord(lWRRV4Record, strWaferID.toInt(), iTotalGoodBin+iTotalFailBin , iTotalGoodBin);
                mStdfParse.WriteRecord(&lWRRV4Record);
                lWRRV4Record.Reset();
            }

            iTotalGoodBin = 0;
            iTotalFailBin = 0;

            // For each wafer, have to write limit in the first PTR
            for(iIndex=0;iIndex<mTotalParameters;++iIndex)
                mSmicParameterList[iIndex]->SetStaticHeaderWritten (false);

            // Write WIR of new Wafer.
            strWaferID = strSection;

            SetWIRRecord(lWIRV4Record, strWaferID.toInt());
            mStdfParse.WriteRecord(&lWIRV4Record);
        }

        // Reset Pass/Fail flag.
        bPassStatus = true;

        // Reset counters
        iTotalTests = 0;

        // Extract Site
        iSiteNumber = strString.section(',',2,2).simplified().toInt();

        // Write PIR for parts in this Wafer site
        lPIRV4Record.SetHEAD_NUM(1);								// Test head
        lPIRV4Record.SetSITE_NUM(iSiteNumber);					// Tester site
        mStdfParse.WriteRecord(&lPIRV4Record);
        lPIRV4Record.Reset();

        QStringList lstSections = strString.split(",",QString::KeepEmptyParts);
        // Read Parameter results for this record
        GQTL_STDF::Stdf_PTR_V4 lPTRV4Record;

        for(iIndex=0;iIndex<mTotalParameters;++iIndex)
        {
            strSection = "";
            if(lstSections.count() > iIndex+4)
                strSection = lstSections[iIndex+3];
            fValue = strSection.toFloat(&bStatus);

            // Normalize result
            value_scale = mSmicParameterList[iIndex]->GetResultScale();
            fValue*= GS_POW(10.0,value_scale);

            if(bStatus == true)
            {
                // Valid test result...write the PTR
                ++iTotalTests;

                // Compute Test# (add user-defined offset)
                iTestNumber = (long) mParameterDirectory.GetFullParametersList().indexOf(mSmicParameterList[iIndex]->GetTestName());
                iTestNumber += GEX_TESTNBR_OFFSET_WAT_SMIC;		// Test# offset
                lPTRV4Record.SetTEST_NUM(iTestNumber);			// Test Number
                lPTRV4Record.SetHEAD_NUM(1);				    // Test head
                lPTRV4Record.SetSITE_NUM(iSiteNumber);			// Tester site#
                if(((mSmicParameterList[iIndex]->GetValidLowLimit()==true) && (fValue < mSmicParameterList[iIndex]->GetLowLimit())) ||
                   ((mSmicParameterList[iIndex]->GetValidHighLimit()==true) && (fValue > mSmicParameterList[iIndex]->GetHighLimit())))
                {
                    bData = 0200;	// Test Failed
                    bPassStatus = false;
                }
                else
                {
                    bData = 0;		// Test passed
                }
                lPTRV4Record.SetTEST_FLG(bData);                                // TEST_FLG
                bData = 0;
                if(mSmicParameterList[iIndex]->GetValidLowLimit()==true)
                    bData = 0x40;		// not strict limit
                if(mSmicParameterList[iIndex]->GetValidHighLimit()==true)
                    bData |= 0x80;		// not strict limit
                lPTRV4Record.SetPARM_FLG(bData);								// PARAM_FLG
                lPTRV4Record.SetRESULT(fValue);                                 // Test result
                if(mSmicParameterList[iIndex]->GetStaticHeaderWritten() == false)
                {
                    lPTRV4Record.SetTEST_TXT(mSmicParameterList[iIndex]->GetTestName().toLatin1().constData());	// TEST_TXT
                    lPTRV4Record.SetALARM_ID("");                               // ALARM_ID
                    bData = 2;	// Valid data.
                    if(mSmicParameterList[iIndex]->GetValidLowLimit()==false)
                        bData |=0x40;
                    if(mSmicParameterList[iIndex]->GetValidHighLimit()==false)
                        bData |=0x80;
                    lPTRV4Record.SetOPT_FLAG(bData);                                    // OPT_FLAG
                    lPTRV4Record.SetRES_SCAL(0);                                        // RES_SCALE
                    lPTRV4Record.SetLLM_SCAL(-value_scale);                             // LLM_SCALE
                    lPTRV4Record.SetHLM_SCAL(-value_scale);                             // HLM_SCALE
                    lPTRV4Record.SetLO_LIMIT(mSmicParameterList[iIndex]->GetLowLimit());    // LOW Limit
                    lPTRV4Record.SetHI_LIMIT(mSmicParameterList[iIndex]->GetHighLimit());	// HIGH Limit
                    lPTRV4Record.SetUNITS(mSmicParameterList[iIndex]->GetTestUnits().toLatin1().constData());// No Units
                    mSmicParameterList[iIndex]->SetStaticHeaderWritten(true);
                }
                mStdfParse.WriteRecord(&lPTRV4Record);
                lPTRV4Record.Reset();
            }	// Valid test result
        }		// Read all results on line

        // Write PRR
        GQTL_STDF::Stdf_PRR_V4 lPRRV4Record;
        lPRRV4Record.SetHEAD_NUM(1);					// Test head
        lPRRV4Record.SetSITE_NUM(iSiteNumber);

        if(bPassStatus == true)
        {
            lPRRV4Record.SetPART_FLG(0);				// PART_FLG : PASSED
            wSoftBin = wHardBin = 1;
            ++iTotalGoodBin;
        }
        else
        {
            lPRRV4Record.SetPART_FLG(8);				// PART_FLG : FAILED
            wSoftBin = wHardBin = 0;
            ++iTotalFailBin;
        }

        lPRRV4Record.SetNUM_TEST((WORD)iTotalTests);   // NUM_TEST
        lPRRV4Record.SetHARD_BIN(wHardBin);            // HARD_BIN
        lPRRV4Record.SetSOFT_BIN(wSoftBin);            // SOFT_BIN
        switch(iSiteNumber)
        {
            case 1:	// Center
                lPRRV4Record.SetX_COORD(1);			// X_COORD
                lPRRV4Record.SetY_COORD(1);			// Y_COORD
                break;
            case 2:	// Down
                lPRRV4Record.SetX_COORD(1);			// X_COORD
                lPRRV4Record.SetY_COORD(2);			// Y_COORD
                break;
            case 3:	// Left
                lPRRV4Record.SetX_COORD(0);			// X_COORD
                lPRRV4Record.SetY_COORD(1);			// Y_COORD
                break;
            case 4:	// Top
                lPRRV4Record.SetX_COORD(1);			// X_COORD
                lPRRV4Record.SetY_COORD(0);			// Y_COORD
                break;
            case 5:	// Right
                lPRRV4Record.SetX_COORD(2);			// X_COORD
                lPRRV4Record.SetY_COORD(1);			// Y_COORD
                break;
            default: // More than 5 sites?....give 0,0 coordonates
                lPRRV4Record.SetX_COORD(0);			// X_COORD
                lPRRV4Record.SetY_COORD(0);			// Y_COORD
                break;
        }
        lPRRV4Record.SetTEST_T(0);				// No testing time known...
        lPRRV4Record.SetPART_ID(QString::number(iPartNumber).toLatin1().constData());		// PART_ID
        lPRRV4Record.SetPART_TXT("");			// PART_TXT
        lPRRV4Record.SetPART_FIX();			    // PART_FIX
        mStdfParse.WriteRecord(&lPRRV4Record);
        lPRRV4Record.Reset();
    };			// Read all lines with valid data records in file

    // Write WRR for last wafer inserted
    SetWRRRecord(lWRRV4Record, strWaferID.toInt(), iTotalGoodBin+iTotalFailBin , iTotalGoodBin) ;
    mStdfParse.WriteRecord(&lWRRV4Record);
    lWRRV4Record.Reset();

    // Write MRR
    GQTL_STDF::Stdf_MRR_V4 lMRRV4Record;
    lMRRV4Record.SetFINISH_T(mStartTime);			// File finish-time.
    mStdfParse.WriteRecord(&lMRRV4Record);

    // Close STDF file.
    mStdfParse.Close();
    mFile.close();

    // Success
    return true;
}


}
}
